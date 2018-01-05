# Copyright (c) 2018 Uber Technologies, Inc.

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


class RunManager:
    def __init__(self,**kwargs):
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

                self.__old_wd = os.getcwd()
                self.cwd = self.__old_wd

                if self.change_dir:
                    self.cwd = tempfile.mkdtemp()
                    self.copy()
                if not self.cleanup_flag:
                    print(self.cwd)
                self.cd(self.cwd)
            def copy(self):
                if self.copyfiles is not None:
                    # copy files from __old_wd to cwd
                    isIterable = isinstance(self.copyfiles, collections.Iterable) and not isinstance(self.copyfiles, six.string_types)
                    if isIterable:
                        for f in self.copyfiles:
                            shutil.copy(f,self.cwd)
                    else:
                        f = self.copyfiles
                        shutil.copy(f,self.cwd)

            def cd(self,folder):
                os.chdir(folder)

            def cleanup(self):
                if self.cleanup_flag and (self.cwd is not self.__old_wd):
                    shutil.rmtree(self.cwd)
                self.cd(self.__old_wd)

        # instantiate the subclass, return that so we work with it instead of the handler class
        # "self" in the function arguments refers to the RunManager class
        self.package_obj = RunDirectory(**self.kwargs)
        return self.package_obj

    def __exit__(self, exc_type, exc_value, traceback):
        self.package_obj.cleanup()
