from distutils.core import setup, Extension
from Cython.Build import cythonize
from glob import glob
import sys
plt = sys.platform

cppfiles = glob("./*.cpp")
cfiles = glob("./*.c")

exclude = {"./mavlinkreader.cpp", "./dataflashreader.cpp", "./pyinterop.cpp"}

extrafiles = [x for x in cppfiles + cfiles if x not in exclude]

ext_modules = [
    Extension (
        "UAVReaders",
        sources=["pyinterop.cpp"] + extrafiles,
        extra_compile_args=['/std:c++20' if plt=='win32' else '-std=c++20']
    )
]

setup(
    ext_modules = cythonize(ext_modules),
    name="UAVReaders",
    version="1.0.5",
    author="Christian Clifford",
    author_email="cjclifford@alaska.edu",
    description="Parsers for UAV/Aerial Drone-related formats like MavLink and DataFlash",
    py_modules=[]
)
