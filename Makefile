.phony: all clean
.intermediate: mavlink_introspect_gen.c mavlink_introspect_gen.h *.o

CFLAGS += -Wno-address-of-packed-member

all: mavlink_introspect_gen.o c_introspect.h main.cpp table.h
	$(CXX) $(CFLAGS) --std=gnu++23 main.cpp *.o -o mavlinkreader

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

#mavlink_introspect_gen.o: mavlink_introspect_gen.c mavlink_introspect_gen.h table.h
#	$(CC) $(CFLAGS) -c mavlink_introspect_gen.c $@

clean:
	@rm -rf mavlink_definitions mavlink
	@rm *_gen.*
	@rm mavlinkreader
