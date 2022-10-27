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

    int m_FeaPartIndex;
    int m_FeaSubSurfIndex;

    bool m_XLTFlag;
    bool m_XGTFlag;

    bool m_YLTFlag;
    bool m_YGTFlag;

    bool m_ZLTFlag;
    bool m_ZGTFlag;

    double m_XLTVal;
    double m_XGTVal;

    double m_YLTVal;
    double m_YGTVal;

    double m_ZLTVal;
    double m_ZGTVal;
};

#endif //VSP_TOP_SIMPLEBC_H
