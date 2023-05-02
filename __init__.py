"""
UAVReaders.py

Contributors: Christian Clifford cjclifford1669@gmail.com

collection of parsers for UAV related formats
"""

from glob import glob
import sys
import os.path


class _add_path():
    """
    add_path
    Context manager to temporararily add directories to the Python path
    """
    def __init__(self, path):
        self.path = path


    def __enter__(self):
        for p in self.path:
            sys.path.insert(0, p)
            

    def __exit__(self, exc_type, exc_value, traceback):
        try:
            for p in self.path:
                sys.path.remove(p)

        except ValueError:
            pass

def _import_uavreaders():
    global mavlink_reader
    global data_flash_reader
    directory = os.path.dirname(__file__)
    library_directories = glob(directory+"/build/lib*")
    with _add_path(library_directories):
        import UAVReaders as u
        mavlink_reader = u.mavlink_reader
        data_flash_reader = u.data_flash_reader

_import_uavreaders()
