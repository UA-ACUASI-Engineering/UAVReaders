from distutils.core import setup, Extension
from Cython.Build import cythonize
from glob import glob

cppfiles = glob("./*.cpp")
cfiles = glob("./*.c")

exclude = {"./mavlinkreader.cpp", "./dataflashreader.cpp", "./pyinterop.cpp"}

extrafiles = [x for x in cppfiles + cfiles if x not in exclude]

ext_modules = [
    Extension (
        "UAVReaders",
        sources=["pyinterop.cpp"] + extrafiles
    )
]

setup(
    ext_modules = cythonize(ext_modules),
    name="UAVReaders",
    version="0.5.5",
    author="Christian Clifford",
    author_email="cjclifford@alaska.edu",
    description="Parsers for UAV/Aerial Drone-related formats like MavLink and DataFlash",
    py_modules=[]
)
