# 
# PCG Random Number Generation for C++.
# 
# Copyright 2014-2017 Melissa O'Neill <oneill@pcg-random.org>,
#                     and the PCG Project contributors.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#
# Licensed under the Apache License, Version 2.0 (provided in
# LICENSE-APACHE.txt and at http://www.apache.org/licenses/LICENSE-2.0)
# or under the MIT license (provided in LICENSE-MIT.txt and at
# http://opensource.org/licenses/MIT), at your option. This file may not
# be copied, modified, or distributed except according to those terms.
#
# Distributed on an "AS IS" BASIS, WITHOUT WARRANTY OF ANY KIND, either
# express or implied.  See your chosen license for details.
#
# For additional information about the PCG random number generation scheme,
# visit http://www.pcg-random.org/.
#

TARGETS           = pcg-demo codebook cppref-sample make-partytrick
BINARYOUT_TARGETS = spew use-partytrick

CPPFLAGS += -I../include 
CXXFLAGS += -std=c++11 -O2
CC       = $(CXX)       # Cheat so that linking uses the C++ compiler

all: $(TARGETS)

posix-binaryoutput: spew use-partytrick

clean:
	rm -f *.o $(TARGETS) $(BINARYOUT_TARGETS)

codebook.o: codebook.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
cppref-sample.o: cppref-sample.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
make-partytrick.o: make-partytrick.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
pcg-demo.o: pcg-demo.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
spew.o: spew.cpp ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
use-partytrick.o: use-partytrick.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
