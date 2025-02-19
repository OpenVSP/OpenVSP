//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GearGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPGEARGEOM__INCLUDED_)
#define VSPGEARGEOM__INCLUDED_

#include "Geom.h"
#include "XSec.h"

class Bogie : public ParmContainer
{
public:
    Bogie();

    // virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    // virtual xmlNodePtr DecodeXml( xmlNodePtr & node );
    virtual void ParmChanged( Parm* parm_ptr, int type );

    int GetNumSurf() const;
    void UpdateParms();
    void UpdateTireCurve();
    void Update();
    void AppendMainSurf( vector < VspSurf > &surfvec ) const;

    bool m_Visible;

    VspCurve m_TireProfile;
    VspSurf m_TireSurface;


    BoolParm m_Symmetrical;

    IntParm m_NAcross;
    IntParm m_NTandem;

    IntParm m_SpacingType;
    Parm m_Spacing;
    Parm m_SpacingFrac;
    Parm m_SpacingGap;
    Parm m_SpacingGapFrac;

    IntParm m_PitchType;
    Parm m_Pitch;
    Parm m_PitchFrac;
    Parm m_PitchGap;
    Parm m_PitchGapFrac;

    Parm m_XContactPt;
    Parm m_YContactPt;
    Parm m_ZAboveGround;

    Parm m_Width;
    Parm m_Diameter;

    BoolParm m_SLRFlag;
    Parm m_DeflectionPct;
    Parm m_StaticRadius;

    BoolParm m_DrimFlag;
    Parm m_DrimFrac;
    Parm m_Drim;

    BoolParm m_WrimFlag;
    Parm m_WrimFrac;
    Parm m_Wrim;

    BoolParm m_WsFlag;
    Parm m_WsFrac;
    Parm m_Ws;

    BoolParm m_HsFlag;
    Parm m_HsFrac;
    Parm m_Hs;


};


//==== Pod Geom ====//
class GearGeom : public Geom
{
public:
    GearGeom( Vehicle* vehicle_ptr );
    virtual ~GearGeom();

    virtual void ComputeCenter();
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    Bogie * CreateAndAddBogie();
    string CreateAndAddBogie( int foo );

    Bogie * GetCurrentBogie();
    std::vector < Bogie * > GetBogieVec();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void SetCurrBogieIndex( int i )             { m_CurrBogieIndex = i; }
    int GetCurrBogieIndex()                     { return m_CurrBogieIndex; }
    void DelAllBogies();
    void ShowAllBogies();
    void HideAllBogies();
    Bogie *  GetBogie( const string &id );
    vector < string > GetAllBogies();
    void DelBogie( const int &i );
    void DelBogie( const string &id );

    virtual void UpdateBBox();
    virtual bool IsModelScaleSensitive()        { return true; }


protected:
    virtual void UpdateSurf();

    std::vector < Bogie * > m_Bogies;
    int m_CurrBogieIndex;

};


#endif // !defined(VSPGEARGEOM__INCLUDED_)
