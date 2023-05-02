.PHONY: all mavlink dataflash pypackage clean test

.IGNORE: clean

.INTERMEDIATE: pyinterop.cpp  pyinterop.cpp

.SILENT: test

CFLAGS += -Wall -Wno-address-of-packed-member -g
LDFLAGS += -g

all: bin/dataflashreader bin/mavlinkreader pypackage

export CFLAGS
export LDFLAGS
pypackage pyinterop.cpp&: mavlink_introspect_gen.c mavlink_introspect_gen.h setup.py pyinterop.pyx
	python setup.py build

mavlink: bin/mavlinkreader

dataflash: bin/dataflashreader

bin/dataflashreader: dataflashreader.cpp dataflash.o c_introspect.o
	mkdir -p bin
	$(CXX) $(CFLAGS) --std=gnu++23 $^ -o bin/dataflashreader


bin/mavlinkreader: mavlinkreader.cpp mavlink_introspect_gen.o c_introspect.o
	mkdir -p bin
	$(CXX) $(CFLAGS) --std=gnu++23 $^ -o bin/mavlinkreader

MAVLINK_DEFS := mavlink_project
PYMAVLINK := $(MAVLINK_DEFS)/pymavlink
MAVLINK_VER := 2.0
MAVLINK_SCHEMA := v1.0/all.xml

export PYTHONPATH=$(shell pwd)/$(MAVLINK_DEFS)
mavlink/:
	echo $(PYMAVLINK)
	python -c "import os; print(os.environ[\"PYTHONPATH\"])"
	python3 -m pymavlink.tools.mavgen --lang=C --wire-protocol=$(MAVLINK_VER) --output=mavlink $(MAVLINK_DEFS)/message_definitions/$(MAVLINK_SCHEMA)

mavlink_introspect_gen.c mavlink_introspect_gen.h &: mavlink/ makestructs.py
	@find mavlink/ \
	-type f \
	-regex ".*mavlink_msg_.*\.h" \
	-exec ./makestructs.py \
	-r mavlink_introspect_gen.h \
	-p mavlink_introspect_gen.c '{}' + >/dev/null

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp %.h
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf mavlink
	@rm -rf *.o
	@rm -rf bin
	@rm -rf *_gen.c
	@rm -rf *_gen.h
	@rm -rf build

test: bin/mavlinkreader
	./bin/mavlinkreader <./test_data/mavlink_test.bin > __test_mav.out \
	2>/dev/null
	if cmp -s __test_mav.out ./test_data/mavlink_test.out ; then \
	echo "MAVLINK_OK" ; \
	else echo "MAVLINK BROKEN; DIFF SAVED" ; \
	diff __test_mav.out ./test_data/mavlink_test.out ; \
    fi
	rm __test_mav.out

	./bin/dataflashreader <./test_data/dataflash_test.bin > __test_DF.out \
	2>/dev/null
	if cmp -s __test_DF.out ./test_data/dataflash_test.out ; then \
	echo "DATAFLASH_OK" ; \
	else echo "DATAFLASH BROKEN; DIFF SAVED" ; \
	diff __test_mav.out ./test_data/dataflash_test.out ; \
    fi
	rm __test_DF.out
