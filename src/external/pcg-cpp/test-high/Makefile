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

TARGETS           = \
    check-pcg8_once_insecure check-pcg8_oneseq_once_insecure               \
    check-pcg16_once_insecure check-pcg16_oneseq_once_insecure check-pcg32 \
    check-pcg32_c1024 check-pcg32_c1024_fast check-pcg32_c64               \
    check-pcg32_c64_fast check-pcg32_c64_oneseq check-pcg32_fast           \
    check-pcg32_k1024 check-pcg32_k1024_fast check-pcg32_k16384            \
    check-pcg32_k16384_fast check-pcg32_k2 check-pcg32_k2_fast             \
    check-pcg32_k64 check-pcg32_k64_fast check-pcg32_k64_oneseq            \
    check-pcg32_once_insecure check-pcg32_oneseq                           \
    check-pcg32_oneseq_once_insecure check-pcg32_unique check-pcg64        \
    check-pcg64_c1024 check-pcg64_c1024_fast check-pcg64_c32               \
    check-pcg64_c32_fast check-pcg64_c32_oneseq check-pcg64_fast           \
    check-pcg64_k1024 check-pcg64_k1024_fast check-pcg64_k32               \
    check-pcg64_k32_fast check-pcg64_k32_oneseq check-pcg64_once_insecure  \
    check-pcg64_oneseq check-pcg64_oneseq_once_insecure check-pcg64_unique \
    check-pcg128_once_insecure check-pcg128_oneseq_once_insecure

# special flags for some compilers
CPPFLAGS_clang++ += \
	-Weverything \
	-Wno-unknown-warning-option \
	-Wno-c++98-compat \
	-Wno-c++98-compat-pedantic \
	-Wno-date-time \
	-Wno-undef \
	-Wno-header-hygiene \
	-Wno-unused-macros

CPPFLAGS += -I../include -Wall -Wextra $(CPPFLAGS_$(CXX))
CXXFLAGS += -std=c++11
CC       = $(CXX)       # Cheat so that linking uses the C++ compiler

all: $(TARGETS)

test: $(TARGETS)
	sh run-tests.sh

clean:
	rm -f *.o $(TARGETS)
	rm -rf actual

pcg-test-noadvance.o: pcg-test-noadvance.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
pcg-test.o: pcg-test.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
check-pcg8_once_insecure.o: check-pcg8_once_insecure.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg8_oneseq_once_insecure.o: check-pcg8_oneseq_once_insecure.cpp \
  pcg-test.cpp ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg16_once_insecure.o: check-pcg16_once_insecure.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg16_oneseq_once_insecure.o: check-pcg16_oneseq_once_insecure.cpp \
  pcg-test.cpp ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32.o: check-pcg32.cpp pcg-test.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
check-pcg32_c1024.o: check-pcg32_c1024.cpp pcg-test-noadvance.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_c1024_fast.o: check-pcg32_c1024_fast.cpp \
  pcg-test-noadvance.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
check-pcg32_c64.o: check-pcg32_c64.cpp pcg-test-noadvance.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_c64_fast.o: check-pcg32_c64_fast.cpp pcg-test-noadvance.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_c64_oneseq.o: check-pcg32_c64_oneseq.cpp \
  pcg-test-noadvance.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
check-pcg32_fast.o: check-pcg32_fast.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_k1024.o: check-pcg32_k1024.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_k1024_fast.o: check-pcg32_k1024_fast.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_k16384.o: check-pcg32_k16384.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_k16384_fast.o: check-pcg32_k16384_fast.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_k2.o: check-pcg32_k2.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_k2_fast.o: check-pcg32_k2_fast.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_k64.o: check-pcg32_k64.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_k64_fast.o: check-pcg32_k64_fast.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_k64_oneseq.o: check-pcg32_k64_oneseq.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_once_insecure.o: check-pcg32_once_insecure.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_oneseq.o: check-pcg32_oneseq.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_oneseq_once_insecure.o: check-pcg32_oneseq_once_insecure.cpp \
  pcg-test.cpp ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg32_unique.o: check-pcg32_unique.cpp pcg-test-noadvance.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64.o: check-pcg64.cpp pcg-test.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
check-pcg64_c1024.o: check-pcg64_c1024.cpp pcg-test-noadvance.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_c1024_fast.o: check-pcg64_c1024_fast.cpp \
  pcg-test-noadvance.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
check-pcg64_c32.o: check-pcg64_c32.cpp pcg-test-noadvance.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_c32_fast.o: check-pcg64_c32_fast.cpp pcg-test-noadvance.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_c32_oneseq.o: check-pcg64_c32_oneseq.cpp \
  pcg-test-noadvance.cpp ../include/pcg_random.hpp \
  ../include/pcg_extras.hpp ../include/pcg_uint128.hpp
check-pcg64_fast.o: check-pcg64_fast.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_k1024.o: check-pcg64_k1024.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_k1024_fast.o: check-pcg64_k1024_fast.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_k32.o: check-pcg64_k32.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_k32_fast.o: check-pcg64_k32_fast.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_k32_oneseq.o: check-pcg64_k32_oneseq.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_once_insecure.o: check-pcg64_once_insecure.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_oneseq.o: check-pcg64_oneseq.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_oneseq_once_insecure.o: check-pcg64_oneseq_once_insecure.cpp \
  pcg-test.cpp ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg64_unique.o: check-pcg64_unique.cpp pcg-test-noadvance.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg128_once_insecure.o: check-pcg128_once_insecure.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
check-pcg128_oneseq_once_insecure.o: \
  check-pcg128_oneseq_once_insecure.cpp pcg-test.cpp \
  ../include/pcg_random.hpp ../include/pcg_extras.hpp \
  ../include/pcg_uint128.hpp
