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

#ifndef DEBUGGING_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define DEBUGGING_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include "mathutils.h"

namespace Pinocchio {

class Debugging {
  public:
    static std::ostream &out() { return *outStream; }
    static void PINOCCHIO_API setOutStream(std::ostream &os) { outStream = &os; }

  private:
    static std::ostream *outStream;
};

} // namespace Pinocchio

#endif // DEBUGGING_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
