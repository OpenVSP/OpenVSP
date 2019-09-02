/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "indexer.h"

namespace Pinocchio {

unsigned int interLeaveLookupTable[32768];

class LookupTable
{
  public:
    LookupTable()
    {
      for(int i = 0; i < 32768; ++i)
      {
        interLeaveLookupTable[i] = 0;
        for(int k = 0; k < 15; ++k)
          if(i & (1 << k))
            interLeaveLookupTable[i] += (1 << (28 - 2 * k));
      }
    }
};

static LookupTable lt;

unsigned int interLeave3LookupTable[1024];

class LookupTable3
{
  public:
    LookupTable3()
    {
      for(int i = 0; i < 1024; ++i)
      {
        interLeave3LookupTable[i] = 0;
        for(int k = 0; k < 10; ++k)
          if(i & (1 << k))
            interLeave3LookupTable[i] += (1 << (27 - 3 * k));
      }
    }
};

static LookupTable3 lt3;

} // namespace Pinocchio
