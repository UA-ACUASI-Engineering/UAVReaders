# distutils: language=c++

from libcpp.string cimport string
from libcpp.vector cimport vector

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

#    cdef cppclass abstractStructMember:
#        string getName()
#        int getNumElements()
#        cType getTypeAsEnum()
#        void* getInnerPointer()

    cdef cppclass struct:
        pass

    cdef cppclass Struct (abstractToJson):
        vector[abstractStructMember *] elements
        int getType()
        string getName()
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


cdef object voidToPython(void* ptr, cType t):
    if t == cType.BOOL:
        return bool((<unsigned char*>ptr)[0])
    elif t == cType.CHAR:
        return chr((<char*>ptr)[0])
    elif t == cType.SIGNED_CHAR:
        return int((<char*>ptr)[0])
    elif t == cType.UNSIGNED_CHAR:
        return int((<unsigned char*>ptr)[0])
    elif t == cType.SHORT:
        return int((<short*>ptr)[0])
    elif t == cType.UNSIGNED_SHORT:
        return int((<unsigned short*>ptr)[0])
    elif t == cType.INT:
        return int((<int*>ptr)[0])
    elif t == cType.UNSIGNED_INT:
        return int((<unsigned int*>ptr)[0])
    elif t == cType.LONG:
        return int((<long*>ptr)[0])
    elif t == cType.UNSIGNED_LONG:
        return int((<unsigned long*>ptr)[0])
    elif t == cType.LONG_LONG:
        return int((<long long*>ptr)[0])
    elif t == cType.UNSIGNED_LONG_LONG:
        return int((<unsigned long long*>ptr)[0])
    elif t == cType.INT8_T:
        return int((<int8_t*>ptr)[0])
    elif t == cType.UINT8_T:
        return int((<uint8_t*>ptr)[0])
    elif t == cType.INT16_T:
        return int((<int16_t*>ptr)[0])
    elif t == cType.UINT16_T:
        return int((<uint16_t*>ptr)[0])
    elif t == cType.INT32_T:
        return int((<int32_t*>ptr)[0])
    elif t == cType.UINT32_T:
        return int((<uint32_t*>ptr)[0])
    elif t == cType.INT64_T:
        return int((<int64_t*>ptr)[0])
    elif t == cType.UINT64_T:
        return int((<uint64_t*>ptr)[0])
    elif t == cType.FLOAT:
        return float((<float*>ptr)[0])
    elif t == cType.DOUBLE:
        return float((<double*>ptr)[0])
    else:
        return None

cdef tuple memberToPy(abstractStructMember* member):
    cdef cType t = member.getTypeAsEnum()
    cdef void * p = member.getInnerPointer()
    cdef object pyObj = voidToPython(p, t)
    cdef string name = member.getName()
    return name, pyObj


cdef class packet(dict):
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
        self.name = ptr.getName()
        self["__name__"] = self.name
        self["__packet_type__"] = self.packet_type
        self["__protocol__"] = proto

        deleteStruct(ptr)

    @property
    def type(self):
        return self.packet_type

    @property
    def name(self):
        return self.name

    @property
    def packet_type(self):
        return self.packet_type
        
cdef class data_flash_reader:
    cdef dataFlashReader* reader

    def __cinit__(self):
        self.reader = new dataFlashReader()

    def __dealloc__ (self):
        del self.reader

    def parseByte(self, const uint8_t byte):
        self.reader.parseByte(byte)

    def parseBuffer(self, const uint8_t[:] bytes):
        cdef uint64_t l = len(bytes)
        self.reader.parseBuffer(&bytes[0], l)

    def get_packet(self) -> packet:
        cdef Struct* s = self.reader.getPacket()
        cdef packet p = packet()
        p.secret_real_init(s, "mavlink")
        return p

cdef class mavlink_reader:
    cdef mavLinkReader* reader

    def __cinit__(self):
        self.reader = new mavLinkReader()

    def __dealloc__ (self):
        del self.reader

    def parseByte(self, const uint8_t byte):
        self.reader.parseByte(byte)

    def parseBuffer(self, const uint8_t[:] bytes):
        cdef uint64_t l = len(bytes)
        self.reader.parseBuffer(&bytes[0], l)

    def get_packet(self) -> packet:
        cdef Struct* s = self.reader.getPacket()
        cdef packet p = packet()
        p.secret_real_init(s, "mavlink")
        return p

    
    
