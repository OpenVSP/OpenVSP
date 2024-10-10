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

    m_FeaPartIndex = -1;
    m_FeaSubSurfIndex = -1;

    m_XLTFlag = false;
    m_XGTFlag = false;

    m_YLTFlag = false;
    m_YGTFlag = false;

    m_ZLTFlag = false;
    m_ZGTFlag = false;

    m_XLTVal = 0;
    m_XGTVal = 0;

    m_YLTVal = 0;
    m_YGTVal = 0;

    m_ZLTVal = 0;
    m_ZGTVal = 0;
}

void SimpleBC::CopyFrom( FeaBC* fea_bc )
{
    m_Constraints = fea_bc->GetAsBitMask();
    m_BCType = fea_bc->m_FeaBCType();

    if ( FeaMeshMgr.GetMeshPtr() )
    {
        m_FeaPartIndex = vector_find_val( FeaMeshMgr.GetMeshPtr()->m_FeaPartIDVec, fea_bc->GetPartID() );
    }
    m_FeaSubSurfIndex = FeaMeshMgr.GetSimpSubSurfIndex( fea_bc->GetSubSurfID() );

    m_XLTFlag = fea_bc->m_XLTFlag();
    m_XGTFlag = fea_bc->m_XGTFlag();

    m_YLTFlag = fea_bc->m_YLTFlag();
    m_YGTFlag = fea_bc->m_YGTFlag();

    m_ZLTFlag = fea_bc->m_ZLTFlag();
    m_ZGTFlag = fea_bc->m_ZGTFlag();

    m_XLTVal = fea_bc->m_XLTVal();
    m_XGTVal = fea_bc->m_XGTVal();

    m_YLTVal = fea_bc->m_YLTVal();
    m_YGTVal = fea_bc->m_YGTVal();

    m_ZLTVal = fea_bc->m_ZLTVal();
    m_ZGTVal = fea_bc->m_ZGTVal();

}

void SimpleBC::ApplyTo( FeaNode* node )
{
    if ( m_BCType == vsp::FEA_BC_PART )
    {
        if ( node->HasTag( m_FeaPartIndex ) )
        {

        }
        else
        {
            return;
        }
    }
    else if ( m_BCType == vsp::FEA_BC_SUBSURF )
    {
        if ( FeaMeshMgr.GetMeshPtr() )
        {
            if ( node->HasTag( m_FeaSubSurfIndex + FeaMeshMgr.GetMeshPtr()->m_NumFeaParts ) )
            {

            }
            else
            {
                return;
            }
        }
    }

    if ( m_XLTFlag )
    {
        if ( node->m_Pnt.x() > m_XLTVal )
        {
            return;
        }
    }

    if ( m_XGTFlag )
    {
        if ( node->m_Pnt.x() < m_XGTVal )
        {
            return;
        }
    }

    if ( m_YLTFlag )
    {
        if ( node->m_Pnt.y() > m_YLTVal )
        {
            return;
        }
    }

    if ( m_YGTFlag )
    {
        if ( node->m_Pnt.y() < m_YGTVal )
        {
            return;
        }
    }

    if ( m_ZLTFlag )
    {
        if ( node->m_Pnt.z() > m_ZLTVal )
        {
            return;
        }
    }

    if ( m_ZGTFlag )
    {
        if ( node->m_Pnt.z() < m_ZGTVal )
        {
            return;
        }
    }

    // Apply other conditions.
    node->m_BCs.Or( m_Constraints );

}
