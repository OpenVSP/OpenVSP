# Copyright (c) 2023 Rob McDonald

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

import numpy as np
import code
import os
import shutil
import glob



def find_nearest(array, value):
    array = np.asarray(array)
    idx = (np.abs(array - value)).argmin()
    return idx



def keyboard():
    code.interact(local=locals())



def remove_files( match ):
    files = glob.glob(match)

    for f in files:
        try:
            os.remove(f)
        except OSError:
            pass



def remove_file( file ):
    try:
        os.remove(file)
    except:
        pass



def archive( path, vstring ):

    vfname = os.path.join( path, 'ver' )
    with open( vfname, "w") as ver_file:
        ver_file.write( vstring )

    dname = os.path.join( path, vstring )
    try:
        os.mkdir( dname )
    except:
        pass

    files = glob.glob( os.path.join( path, '*' ) )

    for f in files:
        if os.path.isfile( f ):
            shutil.copy( f, dname )


def archive_file( fname, vstring ):
    fbase, ext = os.path.splitext( fname )
    vfname = fbase + '_' + vstring + ext
    shutil.copy( fname, vfname )
    return vfname


def merge_files( finlist, fout ):
    with open( fout, 'w') as outfile:
        for fname in finlist:
            with open(fname) as infile:
                for line in infile:
                    outfile.write(line)
