.PHONY: all mavlink dataflash pypackage clean test pywheel prepare

.IGNORE: clean

.SILENT: test

CFLAGS += -Wall -Wno-address-of-packed-member -g
LDFLAGS += -g

all: bin/dataflashreader bin/mavlinkreader pypackage

PY_DEPS := mavlink_introspect_gen.c mavlink_introspect_gen.h setup.py pyinterop.cpp

export CFLAGS
export LDFLAGS

prepare: $(PY_DEPS)

pypackage: prepare
	python setup.py build

pywheel: prepare
	python -m build

mavlink: bin/mavlinkreader

dataflash: bin/dataflashreader

bin/dataflashreader: dataflashreader.cpp dataflash.o c_introspect.o
	mkdir -p bin
	$(CXX) $(CFLAGS) $^ -o bin/dataflashreader


bin/mavlinkreader: mavlinkreader.cpp mavlink_introspect_gen.o c_introspect.o
	mkdir -p bin
	$(CXX) $(CFLAGS) $^ -o bin/mavlinkreader

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

%.cpp: %.pyx
	cython	--cplus $<

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
	@rm -rf pyinterop.cpp
	@rm -rf dist
	@rm -rf *.egg*

test: bin/mavlinkreader bin/dataflashreader
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

