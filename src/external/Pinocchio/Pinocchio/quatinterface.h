/*
 * =====================================================================================
 *
 *  Filename:  quatinterface.h
 *
 *  Description: Header file for quatinterface.cpp. See the source file for details
 *               on different functions defined here.
 *
 *  Version:  1.0
 *  Created:  03/14/2015 07:01:53 PM
 *  Revision:  none
 *  Compiler:  gcc
 *
 *  Author:  Group F, CMPUT414 Winter 2015
 *
 * =====================================================================================
 */

#ifndef QUATINTERFACE_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
#define QUATINTERFACE_H_BFCF2002_4190_11E9_AA8F_EFB66606E782

#include "vector.h"
#include "transfo.h"
#include "transform.h"
#include "quat_cu.h"
#include "dual_quat_cu.h"

namespace Pinocchio {

Tbx::Dual_quat_cu getQuatFromMat(Transform<> matrix);
Vector3 transformPoint(Vector3 vpos, Tbx::Dual_quat_cu &dquat_blend);

} // namespace Pinocchio

#endif // QUATINTERFACE_H_BFCF2002_4190_11E9_AA8F_EFB66606E782
