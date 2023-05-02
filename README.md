# UAVReaders
Parsers for UAV-related formats like DataFlash and Mavlink

## Usage from Python

Right now, as this module has not yet been uploaded to PyPI or anything like 
that, usage is a bit more complicated (hopefully not too complicated, 
though)! There are a few build-time dependencies that you need to install 
before building the package.
 - `regex`, install with `conda install regex`
 - `future`, install with `conda install future`
 - `cython`, also available through conda with `conda install cython`
 - A working Linux-ish C++ toolchain with `make`. GCC is what I have been 
   using. It should technically be possible to build natively on windows,
   but I would just use WSL.

Now, you have to make sure that Git has pulled all the submodules this 
project requires by running `git submodule update --init --recursive`.

After that, just `cd` into the directory and run `make pypackage`. Import 
`uavreaders.UAVReaders` to load it into your python. This provides access
to `UAVReaders.data_flash_reader`, `UAVReaders.mavlink_reader`, and a few
auxiliary functions not needed during normal usage.

`mavlink_reader` and `data_flash_reader` have the same interface, so I 
will describe just `data_flash_reader`. Create a `data_flash_reader` object 
by passing either a file name to read or a Python function thet `yields` bytes. After that, just read packets out of the reader in a loop.

Here is an example.

```python
import uavreaders.UAVReaders as u

dfreader = u.data_flash_reader("test_data/dataflash_test.bin")

for packet in dfreader():
    # do something with the packet, which is a dict containing 
	# the values of each field in the original packet.
```

## Commandline Usage

Running `make` alone will generate both the python package and 
two binaries that can be used to transform DataFlash or Mavlink 
files to JSON. They are called `dataflashreader` and `mavlinkreader` 
and are written to `/bin` under the project root.

They both have the same interface. They take data through stdin
write to stdout and stderr, so you should use them through input
redirection. To use `dataflashreader` under Bash, run something 
like this: 

```bash
$ ./bin/dataflashreader <test_data/dataflash_test.bin | gzip >output.json.gz
```

This reads dataflash packets from `testdata/dataflash_test.bin` and 
compresses the output JSON, storing it in `output.json.gz`. JSON 
representations of MavLink and Dataflash packets are very large; you will
almost always want to compress the output. A one-gigabyte DataFlash
.bin file might expand by as much as 50 times on parsing to JSON. 

## Testing

Run `make test` to test the functionality of the DataFlash and Mavlink
parsers. This compiles the command-line parsing utilities and compares
their output against a known-good state.

