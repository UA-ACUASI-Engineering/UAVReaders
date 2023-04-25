#!/usr/bin/env python3
import argparse as args
import regex as re
import logging as lo

# floats, ints, uints, char arrays.
# typedef struct __struct_naem {/* lines */} /* stuff */


class CStructMember:
    member_template = '{{ "{member}", &(parsed.{member}), {num}, {c_type}}}'

    def __init__(self, name, c_type, count):
        self.name = name
        self.c_type = c_type
        self.count = count

    def generate(self):
        return CStructMember.member_template.format(
            member=self.name,
            c_type=self.c_type.upper().replace(" ", "_"),
            num=self.count
        )


class CStruct:
    decl_template = "cStruct* unbox_{name}(const mavlink_message_t* message)"

    body_template = """
{{
    static mavlink_{name}_t parsed;

    static cMember members[] = {{
        {records}
    }};
    static cStruct record = {{"{name}", members, {num_members}, {mav_type}}};

    mavlink_msg_{name}_decode(message, &parsed);
    return &record;
}}
"""

    struct_extract_regex = re.compile(
        r"^typedef\s+struct\s+(\S+)\s+\{([^}]*)\}",
        re.MULTILINE
    )

    type_and_name_regex = re.compile(
        r"^\s*(\S+)\s+([^;[]+)(?:|\[(\d+)\]);"
    )

    mavlink_id_regex = re.compile(
        r'#define MAVLINK_MSG_ID_(\w+)\s+(\d+)'
    )

    def __init__(self, text):
        self.members = CStruct.get_struct_members(text)
        self.name, self.packet_id = CStruct.get_mavlink_packet_details(text)

    def get_struct_members(text):
        struct = CStruct.struct_extract_regex.search(text)
        name = struct.group(1).strip("\n")
        body = struct.group(2).strip("\n")
        lines = body.split("\n")
        members = []
        for line in lines:
            match = CStruct.type_and_name_regex.match(line)
            if match is not None:
                c_type = match.group(1)
                name = match.group(2)
                count = match.group(3)
                if count is None:
                    count = 1
                else:
                    try:
                        count = int(count)
                    except ValueError:
                        lo.warn("failed to parse {count} as int")
                        continue

                member = CStructMember(name, c_type, count)
                members.append(member)

        return members

    def get_mavlink_packet_details(text):
        match = CStruct.mavlink_id_regex.search(text)
        if match is not None:
            name = match.group(1).lower()
            packet_id = match.group(2)
            try:
                int(packet_id)
            except ValueError:
                lo.warn(f"could not parse {packet_id} as mavlink packet id")

            return name, int(packet_id)

    def generate_function_body(self):
        members = ",\n        ".join([x.generate() for x in self.members])

        body = self.body_template.format(
            name=self.name,
            records=members,
            num_members=len(self.members),
            mav_type=self.packet_id
        )
        decl = self.decl_template.format(name=self.name)
        return decl + body

    def generate_decl(self):
        return CStruct.decl_template.format(
            name=self.name)


class CProgram:
    c_program_template = """
"""
    body_template = "{include}\n\n{functions}\n\n"

    function_table_entry_template = "unbox_{name}"

    function_table_template = """
unboxer unboxers[] = {{
    {pointers}
}};
"""
    incl_template = "#include \"{header}\""


    def __init__(self):
        self.children = []
        self.unboxer_registry = {}

    def generate_unboxer_table(self):
        null = "0,\n    "
        pointers = ""
        i = 0
        keys = list(self.unboxer_registry.keys())
        keys.sort()
        for k in keys:
            if k > i:
                pointers += null * (k - i)
            i = k+1
            pointers += CProgram.function_table_entry_template.format(
                name=self.unboxer_registry[k])
            pointers += ",\n    "

        return CProgram.function_table_template.format(pointers=pointers)

    def generate_body(self, include=[]):
        incl = "\n".join([CProgram.incl_template.format(header=x) for x in include])

        functions = "\n".join(
            [x.generate_function_body() for x in self.children])
        return self.body_template.format(
            include=incl,
            functions=functions)

    def generate(self, include=[]):
        body = self.generate_body(include=include)
        function_table = self.generate_unboxer_table()
        return body + "\n" + function_table

    def append(self, child):
        self.children.append(child)
        print(child.packet_id, child.name)
        if child.packet_id in self.unboxer_registry.keys():
            lo.warn("Duplicate packet id found -- replacing!")
        self.unboxer_registry[child.packet_id] = child.name


class CHeader:
    c_header_template = """
/* This file defines some tables and functions for introspecting
 * Mavlink header files. This is a generated file; do not edit.
 ****************************************************************/

#pragma once

#include "mavlink/mavlink_types.h"
#include "table.h"

typedef cStruct*(*unboxer)(const mavlink_message_t*);

extern unboxer unboxers[];

{decls}
"""

    def __init__(self):
        self.children = []

    def append(self, child):
        self.children.append(child)

    def generate(self):
        decls = ";\n".join([
            x.generate_decl() for x in self.children
        ])
        decls += ";\n"

        return CHeader.c_header_template.format(
            decls=decls
        )


def main(args):
    mav_headers = set({})
    for header in args.headers:
        mavlink_header=header.split('/')
        mavlink_header[-1]="mavlink.h"
        mav_headers.add('/'.join(mavlink_header))

    includes = list(mav_headers) + args.headers
    header = CHeader()
    program = CProgram()
    for h in args.headers:
        with open(h, 'r') as f:
            struct = CStruct(f.read())
            header.append(struct)
            program.append(struct)

    header_text = header.generate()
    includes.append(args.out_header)
    program_text = program.generate(include=includes)

    with open(args.out_header, "w") as f:
        f.write(header_text)

    with open(args.out_program, "w") as f:
        f.write(program_text)


if __name__ == "__main__":
    arg_parser = args.ArgumentParser()
    arg_parser.add_argument(
        "--out-header",
        "-r",
        help="The header to write into"
    )
    arg_parser.add_argument(
        "--out-program",
        "-p",
        help="The c file to write into"
    )

    arg_parser.add_argument(
        "headers",
        type=str,
        nargs=args.REMAINDER,
        help="List of header files to read"
    )
    args = arg_parser.parse_args()

    main(args)
