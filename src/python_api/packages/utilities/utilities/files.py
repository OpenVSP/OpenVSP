# Copyright (c) 2018-2020 Uber Technologies, Inc.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


import tempfile
import shutil
import os
import six
import collections
import struct

class RunManager:
    def __init__(self,**kwargs):
        '''
        Builds a temporary folder for execution to take place
        :param kwargs: dict, looking for:
            cleanup_flag : whether or not to delete the directory once finished
            change_dir :  whether or not to change directories
            copyfiles : array of paths to files which will be copied into the temporary directory
        '''
        # variables that are settings go here,
        # these will get passed to the actual class object
        self.kwargs = kwargs
    def __enter__(self,**kwargs):
        # this is the class that we actually work with
        class RunDirectory:
            def __init__(self, cleanup_flag=True, change_dir=True, copyfiles=None, **kwargs):
                self.cleanup_flag = cleanup_flag
                self.change_dir = change_dir
                self.copyfiles = copyfiles

                self.__orig_wd = os.getcwd()
                self.wd = self.__orig_wd

                if self.change_dir:
                    self.wd = tempfile.mkdtemp()
                    self.copy()
                if not self.cleanup_flag:
                    print(self.wd)
                self.cd(self.wd)
            def copy(self):
                if self.copyfiles is not None:
                    # copy files from __old_wd to cwd
                    isIterable = isinstance(self.copyfiles, collections.Iterable) and not isinstance(self.copyfiles, six.string_types)
                    if isIterable:
                        for f in self.copyfiles:
                            shutil.copy(f, self.wd)
                    else:
                        f = self.copyfiles
                        shutil.copy(f, self.wd)

            def cd(self,folder):
                os.chdir(folder)

            def cleanup(self):
                self.cd(self.__orig_wd)
                if self.cleanup_flag and (self.wd is not self.__orig_wd):
                    shutil.rmtree(self.wd)

        # instantiate the subclass, return that so we work with it instead of the handler class
        # "self" in the function arguments refers to the RunManager class
        self.package_obj = RunDirectory(**self.kwargs)
        return self.package_obj

    def __exit__(self, exc_type, exc_value, traceback):
        self.package_obj.cleanup()

class BinaryFile():
    def __init__(self, filepath, description=None):
        '''
        Holder for binary file object
        :param file: path to file
        :param description: (optional) description of the file
        '''
        self.filepath = filepath
        self.description = description
        with open(filepath,"rb") as f:
            self.data = f.read()

    def writeFile(self, file):
        '''
        writes file to disk
        :param file: output file path
        :return: None
        '''
        with open(file,"wb") as f:
            f.write(self.data)

class AsciiFile():
    def __init__(self, filepath=None, description=None, lines=None):
        '''
        Holder for ascii file objects

        :param filepath: (optional) file path to load
        :param description: (optional) description of file
        :param lines: (optional) if the file is already loaded, can pass an array of strings instead
        '''
        self.description = description
        self.lines = lines

        self.filepath = filepath
        if self.filepath is not None:
            self.readFile()
    def readFile(self):
        '''
        Reads an ascii file
        :return: None
        '''
        with open(self.filepath,"r") as f:
            self.lines = f.readlines()

    def writeFile(self):
        '''
        Writes the ascii file
        :return: None
        '''
        with open(self.filepath,"w") as f:
            f.writelines(self.lines)

class StructUnpack():
    """
    Class to make unpacking binary data from a file easier
    """
    def __init__(self, endiness):
        """
        Class constructor

        :param endiness: endiness type to use when unpacking values, either '<' or '>'
        """
        self.endiness = endiness

    def unpack(self, fmt, file):
        """
        Unpacks values of format from file

        :param fmt: format to use for reading values
        :param file: binary file to read bytes from
        :return: returns tuple with values based on the input format (same as struct.unpack)
        """
        return struct.unpack(self.endiness + fmt, file.read(struct.calcsize(fmt)))

def split_filename(fname, full_path=False, period=True):
    '''
    Splits a filename into its name and its extension
    :param fname: file path
    :param full_path: (bool) whether or not to return the full path, or just the base name
    :param period: (bool) whether or not to include the period in the extension name
    :return: (filename without extension, extension)
    '''
    base = os.path.splitext(fname)
    name = base[0]
    ext = base[1]
    if not period:
        ext = ext[1:]
    if not full_path:
        name = os.path.split(name)[-1]

    return name, ext

if __name__ == "__main__":
    print("\n")

    print("cwd")
    print(os.getcwd())

    with RunManager(cleanup_flag=False, change_dir=True,copyfiles=["files.py"]) as r:
        print("\n")
        print("change dir flag")
        print(r.change_dir)
        print("cleanup_flag")
        print(r.cleanup_flag)
        print("cwd")
        print(os.getcwd())
        print("\n")
        with open("test.txt","w") as f:
            f.write("Hello, world")
        ls = os.listdir(r.wd)
        print(ls)
    print("cwd")
    print(os.getcwd())
    print("\n")
