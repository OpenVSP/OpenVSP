//
// Created by Rob McDonald on 10/12/22.
//

#ifndef VSP_TOP_SIMPLEBC_H
#define VSP_TOP_SIMPLEBC_H

#include "BitMask.h"
#include "FeaStructure.h"
#include "FeaElement.h"

class SimpleBC
{
public:
    SimpleBC();

    void CopyFrom( FeaBC* fea_bc );

    void ApplyTo( FeaNode* node );

    int m_BCType;
    BitMask m_Constraints;

    double m_Val;
    int m_FeaPartIndex;
    int m_FeaSubSurfIndex;
};

#endif //VSP_TOP_SIMPLEBC_H
