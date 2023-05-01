# distutils: language=c++
"""
Pyinterop.pyx

Contributors: Christian Clifford cjclifford@alaska.edu

Cython wrappers and objects to allow using readers.h
from Python.
""" 

from libcpp.string cimport string
from libcpp.vector cimport vector


import cython
from libcpp.string cimport string

cdef cppToPyStr(string cpp_string):
    # Get a pointer to the underlying C string using the c_str() method
    cdef const char* c_string_ptr = cpp_string.c_str()

    # Convert the C string to a Python string by decoding the byte string using UTF-8 encoding
    cdef bytes c_bytes = <bytes>c_string_ptr
    a = c_bytes.decode('utf-8')
    return a

cimport libc.string

#from libcpp cimport bool

cdef extern from *:
    """
    #include <stdbool.h>
    #include <stdint.h>
    """
    ctypedef unsigned char uint8_t
    ctypedef unsigned short uint16_t
    ctypedef unsigned int uint32_t
    ctypedef unsigned long long uint64_t

    ctypedef signed char int8_t
    ctypedef short int16_t
    ctypedef int int32_t
    ctypedef long long int64_t


cdef extern from "table.h":
    cdef struct cStruct:
        pass

    cdef struct cMember:
        pass


    ctypedef enum cType:
        BOOL = 1
        CHAR = 2
        SIGNED_CHAR = 3
        UNSIGNED_CHAR = 4
        SHORT = 5
        UNSIGNED_SHORT = 6
        INT = 7
        UNSIGNED_INT = 8
        LONG = 9
        UNSIGNED_LONG = 10
        LONG_LONG = 11
        UNSIGNED_LONG_LONG = 12
        INT8_T = 13
        UINT8_T = 14
        INT16_T = 15
        UINT16_T = 16
        INT32_T = 17
        UINT32_T = 18
        INT64_T = 19
        UINT64_T = 20
        FLOAT = 21
        DOUBLE = 22
        NONE = 23


cdef extern from "c_introspect.h" namespace "introspect":
    cdef cppclass abstractToJson:
        pass

    cdef cppclass abstractStructMember (abstractToJson):
        string& getName()
        size_t getNumElements()
        cType getTypeAsEnum()
        void* getInnerPointer()

    cdef cppclass struct:
        pass

    cdef cppclass Struct (abstractToJson):
        vector[abstractStructMember *] elements
        int getType()
        string& getName()
        int getNumChildren()
        Struct(cStruct *) except +
        vector[abstractStructMember*] innerVector()
        vector[abstractStructMember*].iterator begin()
        vector[abstractStructMember*].iterator end()

        string to_json() except +

cdef extern from "readers.h" namespace "UAVFormatReaders":

    cdef cppclass dataFlashReader:
        void parseByte(const uint8_t) except +
        void parseBuffer(const uint8_t *, uint64_t) except +
        int numAvailable()
        int bytesSeen()
        int packetsSeen()
        Struct* getPacket() except +

    cdef cppclass mavLinkReader:
        void parseByte(uint8_t) except +
        void parseBuffer(uint8_t *, uint64_t) except +
        int numAvailable()
        int bytesSeen()
        int packetsSeen()
        Struct* getPacket() except +

    void deleteStruct(Struct *)

# Yes this is terrible; thank you for pointing it out.
cdef object voidToPython(void* ptr, cType t, size_t num):
    l = []
    if t == cType.BOOL:
        for i in range(num):
            l.append(bool((<unsigned char*>ptr)[i]))
    elif t == cType.CHAR:
        for i in range(num):
            l = ""
            l = str(<char*>ptr, encoding="utf-8")
    elif t == cType.SIGNED_CHAR:
        for i in range(num):
            l.append(int((<char*>ptr)[i]))
    elif t == cType.UNSIGNED_CHAR:
        for i in range(num):
            l.append(int((<unsigned char*>ptr)[i]))
    elif t == cType.SHORT:
        for i in range(num):
            l.append(int((<short*>ptr)[i]))
    elif t == cType.UNSIGNED_SHORT:
        for i in range(num):
            l.append(int((<unsigned short*>ptr)[i]))
    elif t == cType.INT:
        for i in range(num):
            l.append(int((<int*>ptr)[i]))
    elif t == cType.UNSIGNED_INT:
        for i in range(num):
            l.append(int((<unsigned int*>ptr)[i]))
    elif t == cType.LONG:
        for i in range(num):
            l.append(int((<long*>ptr)[i]))
    elif t == cType.UNSIGNED_LONG:
        for i in range(num):
            l.append(int((<unsigned long*>ptr)[i]))
    elif t == cType.LONG_LONG:
        for i in range(num):
            l.append(int((<long long*>ptr)[i]))
    elif t == cType.UNSIGNED_LONG_LONG:
        for i in range(num):
            l.append(int((<unsigned long long*>ptr)[i]))
    elif t == cType.INT8_T:
        for i in range(num):
            l.append(int((<int8_t*>ptr)[i]))
    elif t == cType.UINT8_T:
        for i in range(num):
            l.append(int((<uint8_t*>ptr)[i]))
    elif t == cType.INT16_T:
        for i in range(num):
            l.append(int((<int16_t*>ptr)[i]))
    elif t == cType.UINT16_T:
        for i in range(num):
            l.append(int((<uint16_t*>ptr)[i]))
    elif t == cType.INT32_T:
        for i in range(num):
            l.append(int((<int32_t*>ptr)[i]))
    elif t == cType.UINT32_T:
        for i in range(num):
            l.append(int((<uint32_t*>ptr)[i]))
    elif t == cType.INT64_T:
        for i in range(num):
            l.append(int((<int64_t*>ptr)[i]))
    elif t == cType.UINT64_T:
        for i in range(num):
            l.append(int((<uint64_t*>ptr)[i]))
    elif t == cType.FLOAT:
        for i in range(num):
            l.append(float((<float*>ptr)[i]))
    elif t == cType.DOUBLE:
        for i in range(num):
            l.append(float((<double*>ptr)[i]))
    if len(l) == 1:
        return l[0]
    else:
        return l

cdef tuple memberToPy(abstractStructMember* member):
    cdef cType t = member.getTypeAsEnum()
    cdef void * p = member.getInnerPointer()
    cdef size_t num = member.getNumElements()
    cdef object pyObj = voidToPython(p, t, num)
    name = cppToPyStr(member.getName())
    return name, pyObj

cdef class packet(dict):
    """
    A class representing a packet. 
    Basically a dict, but with three additional properties:

     - `.name` name of the packet
     - `.packet_type` ID representing the type of packet within the 
       context of the packet's source protocol
     - `.protocol` The source protocol of the packet as a string

    These packets are not meant to be created by users at this time.
    """
    cdef string name
    cdef string protocol
    cdef int packet_type

    def __cinit__(self):
        super(packet, self).__init__()
        self.name = ""
        self.protocol = ""
        self.packet_type = 0
        
    cdef secret_real_init(self, Struct* ptr, string proto):
        cdef vector[abstractStructMember*] vec = ptr.innerVector()
        for i in vec:
            k, v = memberToPy(i)
            self[k] = v

        self.protocol = proto
        self.packet_type = ptr.getType()
        print("hi")
        self["__name__"] = cppToPyStr(ptr.getName())
        self["__packet_type__"] = self.packet_type
        self["__protocol__"] = cppToPyStr(proto)

        deleteStruct(ptr)

    @property
    def protocol(self):
        return self.protocol

    @property
    def name(self):
        return self.name

    @property
    def packet_type(self):
        return self.packet_type

def thing_to_iterable(thing):
    """
    Converts certain python objects into
    generators returning bytes.
    Currently accepts strings contining filenames and
    things that are already generator functions, not touching them.

    Both readers use this internally, but it can be useful to have access to 
    on its own.

    :param thing: A python object to attemt to convert
    """
    if (callable(thing)):
        return thing
    elif type(thing) == str:
        def readFile():
            with open(thing, "rb") as f:
                data = f.read(512)
                while len(data) > 0:
                    yield data
                    data = f.read(512)

        return readFile

cdef class mavlink_reader:
    """
    Parser for DataFlash .bin files.
    
    To use:
     - Load UAVReaders
     - create a MavlinkReader
       by passing the initializer a string filname or a 
       yielding function that returns bytes to parse
     - Read from the Reader by calling it to obtain a 
       generator.
    
    for example,


    ```python
    from UAVReaders import mavlinkReader as mlr
    reader = mlr("file.tlog")
    for packet in reader():
        # do stuff
    
    # done!
    ```
    :param src: Source of bytes to parse; generator function or filename
    """
    cdef mavLinkReader* reader
    cdef object gen

    def __cinit__(self, src):
        self.reader = new mavLinkReader()
        self.gen = thing_to_iterable(src)

    def __dealloc__ (self):
        del self.reader

    def parseByte(self, const uint8_t byte):
        self.reader.parseByte(byte)

    def parseBuffer(self, const uint8_t[:] bytes):
        cdef uint64_t l = len(bytes)
        self.reader.parseBuffer(&bytes[0], l)

    def get_packet(self) -> packet:
        cdef int n = self.reader.numAvailable()
        if (n > 0):
            s = self.reader.getPacket()
            p = packet()
            p.secret_real_init(s, "mavlink")
            return p
        else:
            return None

    def __call__(self):
        for thing in self.gen(): 
            self.parseBuffer(thing)
            while self.reader.numAvailable() > 0:
                s = self.reader.getPacket()
                p = packet()
                p.secret_real_init(s, "mavlink")
                yield p

cdef class data_flash_reader:
    """
    Parser for DataFlash .bin files.
    
    To use:
     - Load UAVReaders
     - create a dataFlashReader or MavlinkReader
       by passing the initializer a string filname or a 
       yielding function that returns bytes to parse
     - Read from the Reader by calling it to obtain a 
       generator.
    
    for example,


    ```python
    from UAVReaders import dataFlashReader as dfr
    reader = dfr("file.bin")
    for packet in reader():
        # do stuff
    
    # done!
    ```
    :param src: Source of bytes to parse; generator function or filename
    """
    cdef dataFlashReader* reader
    cdef object gen

    def __cinit__(self, src):
        self.reader = new dataFlashReader()
        self.gen = thing_to_iterable(src)

    def __dealloc__ (self):
        del self.reader

    def parseByte(self, const uint8_t byte):
        self.reader.parseByte(byte)

    def parseBuffer(self, const uint8_t[:] bytes):
        cdef uint64_t l = len(bytes)
        self.reader.parseBuffer(&bytes[0], l)

    def get_packet(self) -> packet:
        cdef int n = self.reader.numAvailable()
        if (n > 0):
            s = self.reader.getPacket()
            p = packet()
            p.secret_real_init(s, "data_flash")
            return p
        else:
            return None

    def __call__(self):
        for thing in self.gen():
            self.parseBuffer(thing)
            while self.reader.numAvailable() > 0:
                s = self.reader.getPacket()
                p = packet()
                p.secret_real_init(s, "data_flash")
                yield p

