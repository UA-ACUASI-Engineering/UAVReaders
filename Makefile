.phony: all clean

.ignore: clean

.intermediate:

CFLAGS += -Wno-address-of-packed-member -g
LDFLAGS += -g

all: bin/dataflashreader bin/mavlinkreader


bin/dataflashreader: dataflashreader.cpp dataflash.o c_introspect.o
	mkdir -p bin
	$(CXX) $(CFLAGS) --std=gnu++23 $^ -o bin/dataflashreader


bin/mavlinkreader: mavlinkreader.cpp mavlink_introspect_gen.o c_introspect.o
	mkdir -p bin
	$(CXX) $(CFLAGS) --std=gnu++23 $^ -o bin/mavlinkreader

MAVLINK_DEFS := mavlink_definitions
MAVLINK_VER := 2.0
MAVLINK_SCHEMA := v1.0/all.xml

export PYTHONPATH=$(shell pwd)/$(MAVLINK_DEFS)
mavlink/:
	if [ ! -d $(MAVLINK_DEFS) ] ; then \
	git clone 'https://github.com/mavlink/mavlink.git' $(MAVLINK_DEFS) --recursive ; \
	fi
	python3 -m pymavlink.tools.mavgen --lang=C --wire-protocol=$(MAVLINK_VER) --output=mavlink $(MAVLINK_DEFS)/message_definitions/$(MAVLINK_SCHEMA)

mavlink_introspect_gen.c mavlink_introspect_gen.h &: mavlink/ makestructs.py
	@find mavlink/ \
	-type f \
	-regex ".*mavlink_msg_.*\.h" \
	-exec ./makestructs.py \
	-r mavlink_introspect_gen.h \
	-p mavlink_introspect_gen.c '{}' +

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp %.h
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf mavlink_definitions mavlink
	@rm -rf *.o
	@rm mavlinkreader
	@rm dataflashreader
	@rm *_gen.*
