//
// Created by Rob McDonald on 10/12/22.
//

#include "SimpleBC.h"
#include "FeaStructure.h"
#include "FeaMeshMgr.h"
#include "StlHelper.h"
#include "FeaElement.h"
#include "APIDefines.h"

SimpleBC::SimpleBC()
{
    m_BCType = -1;

    m_Val = 0;
    m_FeaPartIndex = -1;
    m_FeaSubSurfIndex = -1;
}

void SimpleBC::CopyFrom( FeaBC* fea_bc )
{
    m_Constraints = fea_bc->GetAsBitMask();
    m_BCType = fea_bc->m_FeaBCType();

    m_Val = fea_bc->m_Yval();
    m_FeaPartIndex = vector_find_val( FeaMeshMgr.GetMeshPtr()->m_FeaPartIDVec, fea_bc->GetPartID() );
    m_FeaSubSurfIndex = FeaMeshMgr.GetSimpSubSurfIndex( fea_bc->GetSubSurfID() );
}

void SimpleBC::ApplyTo( FeaNode* node )
{
    if ( m_BCType == vsp::FEA_BC_Y_LESS_THAN )
    {
        if ( node->m_Pnt[1] < m_Val )
        {
            node->m_BCs.Or( m_Constraints );
        }
    }
    else if ( m_BCType == vsp::FEA_BC_PART )
    {
        if ( node->HasTag( m_FeaPartIndex ) )
        {
            node->m_BCs.Or( m_Constraints );
        }
    }
    else if ( m_BCType == vsp::FEA_BC_SUBSURF )
    {
        if ( FeaMeshMgr.GetMeshPtr() )
        {
            if ( node->HasTag( m_FeaSubSurfIndex + FeaMeshMgr.GetMeshPtr()->m_NumFeaParts ) )
            {
                node->m_BCs.Or( m_Constraints );
            }
        }
    }
}
