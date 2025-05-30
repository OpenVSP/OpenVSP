#include "MeasureMgr.h"
#include "APIDefines.h"

MeasureMgrSingleton::MeasureMgrSingleton()
{
    m_CurrRulerIndex = 0;
    m_CurrProbeIndex = 0;
    m_CurrRSTProbeIndex = 0;
    m_CurrProtractorIndex = 0;
    Init();
}

void MeasureMgrSingleton::Init()
{
}

void MeasureMgrSingleton::Wype()
{
    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        delete m_Protractors[i];
    }
    m_Protractors.clear();

    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        delete m_Rulers[i];
    }
    m_Rulers.clear();

    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        delete m_Probes[i];
    }
    m_Probes.clear();

    for(int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        delete m_RSTProbes[i];
    }
    m_RSTProbes.clear();
}

void MeasureMgrSingleton::Renew()
{
    Wype();
    Init();
}

Protractor * MeasureMgrSingleton::CreateAndAddProtractor()
{
    Protractor * protractor = new Protractor();

    m_Protractors.push_back( protractor );

    m_CurrProtractorIndex = m_Protractors.size() - 1;
    return protractor;
}

string MeasureMgrSingleton::CreateAndAddProtractor( const string & startgeomid, int startsurfindx, double startu, double startw,
                                                    const string & midgeomid, int midsurfindx, double midu, double midw,
                                                    const string & endgeomid, int endsurfindx, double endu, double endw, const string & name )
{
    Protractor * protractor = CreateAndAddProtractor();

    protractor->m_OriginGeomID = startgeomid;
    protractor->m_OriginIndx = startsurfindx;
    protractor->m_OriginU = startu;
    protractor->m_OriginW = startw;

    protractor->m_MidGeomID = midgeomid;
    protractor->m_MidIndx = midsurfindx;
    protractor->m_MidU = midu;
    protractor->m_MidW = midw;

    protractor->m_EndGeomID = endgeomid;
    protractor->m_EndIndx = endsurfindx;
    protractor->m_EndU = endu;
    protractor->m_EndW = endw;

    protractor->SetName( name );
    protractor->m_Stage = STAGE_COMPLETE;

    return protractor->GetID();
}


Ruler * MeasureMgrSingleton::CreateAndAddRuler()
{
    Ruler * ruler = new Ruler();

    m_Rulers.push_back( ruler );

    m_CurrRulerIndex = m_Rulers.size() - 1;
    return ruler;
}

string MeasureMgrSingleton::CreateAndAddRuler( const string & startgeomid, int startsurfindx, double startu, double startw,
                          const string & endgeomid, int endsurfindx, double endu, double endw, const string & name )
{
    Ruler * ruler = CreateAndAddRuler();

    ruler->m_OriginGeomID = startgeomid;
    ruler->m_OriginIndx = startsurfindx;
    ruler->m_OriginU = startu;
    ruler->m_OriginW = startw;

    ruler->m_EndGeomID = endgeomid;
    ruler->m_EndIndx = endsurfindx;
    ruler->m_EndU = endu;
    ruler->m_EndW = endw;

    ruler->SetName( name );
    ruler->m_Stage = STAGE_COMPLETE;

    return ruler->GetID();
}

Probe * MeasureMgrSingleton::CreateAndAddProbe()
{
    Probe * probe = new Probe();

    m_Probes.push_back( probe );

    m_CurrProbeIndex = m_Probes.size() - 1;
    return probe;
}

string MeasureMgrSingleton::CreateAndAddProbe( const string & geomid, int surfindx, double u, double w, const string & name )
{
    Probe * probe = CreateAndAddProbe();

    probe->m_OriginGeomID = geomid;
    probe->m_OriginIndx = surfindx;
    probe->m_OriginU = u;
    probe->m_OriginW = w;

    probe->SetName( name );
    probe->m_Stage = STAGE_COMPLETE;

    return probe->GetID();
}

RSTProbe * MeasureMgrSingleton::CreateAndAddRSTProbe()
{
    RSTProbe * RSTprobe = new RSTProbe();

    m_RSTProbes.push_back(RSTprobe );

    m_CurrRSTProbeIndex = m_RSTProbes.size() - 1;
    return RSTprobe;
}

string MeasureMgrSingleton::CreateAndAddRSTProbe(const string & geomid, int surfindx, double r, double s, double t, const string & name )
{
    RSTProbe * RSTprobe = CreateAndAddRSTProbe();

    RSTprobe->m_OriginGeomID = geomid;
    RSTprobe->m_OriginIndx = surfindx;
    RSTprobe->m_OriginR = r;
    RSTprobe->m_OriginS = s;
    RSTprobe->m_OriginT = t;

    RSTprobe->SetName( name );
    RSTprobe->m_Stage = STAGE_COMPLETE;

    return RSTprobe->GetID();
}

Protractor * MeasureMgrSingleton::GetCurrentProtractor()
{
    if ( m_CurrProtractorIndex < 0 || m_CurrProtractorIndex >= ( int )m_Protractors.size() )
    {
        return nullptr;
    }

    return m_Protractors[ m_CurrProtractorIndex ];
}

Ruler * MeasureMgrSingleton::GetCurrentRuler()
{
    if ( m_CurrRulerIndex < 0 || m_CurrRulerIndex >= ( int )m_Rulers.size() )
    {
        return nullptr;
    }

    return m_Rulers[ m_CurrRulerIndex ];
}

Probe * MeasureMgrSingleton::GetCurrentProbe()
{
    if ( m_CurrProbeIndex < 0 || m_CurrProbeIndex >= ( int )m_Probes.size() )
    {
        return nullptr;
    }

    return m_Probes[ m_CurrProbeIndex ];
}

RSTProbe * MeasureMgrSingleton::GetCurrentRSTProbe()
{
    if (m_CurrRSTProbeIndex < 0 || m_CurrRSTProbeIndex >= ( int )m_RSTProbes.size() )
    {
        return nullptr;
    }

    return m_RSTProbes[ m_CurrRSTProbeIndex ];
}

std::vector < Protractor * > MeasureMgrSingleton::GetProtractorVec()
{
    return m_Protractors;
}

std::vector < Ruler * > MeasureMgrSingleton::GetRulerVec()
{
    return m_Rulers;
}

std::vector < Probe * > MeasureMgrSingleton::GetProbeVec()
{
    return m_Probes;
}

std::vector < RSTProbe * > MeasureMgrSingleton::GetRSTProbeVec()
{
    return m_RSTProbes;
}

xmlNodePtr MeasureMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    char labelName[256];

    xmlNodePtr child_node = xmlNewChild( node, nullptr, BAD_CAST "Measure", nullptr );

    XmlUtil::AddIntNode( child_node, "Num_of_Protractors", m_Protractors.size() );

    for ( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Protractor_%d", i );
        xmlNodePtr label_node = xmlNewChild( child_node, nullptr, BAD_CAST labelName, nullptr );
        m_Protractors[i]->EncodeXml( label_node );
    }

    XmlUtil::AddIntNode( child_node, "Num_of_Rulers", m_Rulers.size() );

    for ( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Ruler_%d", i );
        xmlNodePtr label_node = xmlNewChild( child_node, nullptr, BAD_CAST labelName, nullptr );
        m_Rulers[i]->EncodeXml( label_node );
    }

    XmlUtil::AddIntNode( child_node, "Num_of_Probes", m_Probes.size() );

    for ( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Probe_%d", i );
        xmlNodePtr label_node = xmlNewChild( child_node, nullptr, BAD_CAST labelName, nullptr );
        m_Probes[i]->EncodeXml( label_node );
    }

    XmlUtil::AddIntNode(child_node, "Num_of_RSTprobes", m_RSTProbes.size() );

    for (int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        snprintf( labelName, sizeof( labelName ), "RSTprobe_%d", i );
        xmlNodePtr label_node = xmlNewChild( child_node, nullptr, BAD_CAST labelName, nullptr );
        m_RSTProbes[i]->EncodeXml(label_node );
    }

    return child_node;
}

xmlNodePtr MeasureMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    char labelName[256];

    xmlNodePtr label_root_node = XmlUtil::GetNode( node, "Measure", 0 );

    int numofProtractors = XmlUtil::FindInt( label_root_node, "Num_of_Protractors", 0 );
    for ( int i = 0; i < numofProtractors; i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Protractor_%d", i );
        xmlNodePtr label_node = XmlUtil::GetNode( label_root_node, labelName, 0 );
        if( label_node )
        {
            xmlNodePtr parmcontain_node = XmlUtil::GetNode( label_node, "ParmContainer", 0 );
            if ( parmcontain_node )
            {
                CreateAndAddProtractor();
                GetCurrentProtractor()->DecodeXml( label_node );
            }
        }
    }

    int numofLabels = XmlUtil::FindInt( label_root_node, "Num_of_Rulers", 0 );
    for ( int i = 0; i < numofLabels; i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Ruler_%d", i );
        xmlNodePtr label_node = XmlUtil::GetNode( label_root_node, labelName, 0 );
        if( label_node )
        {
            xmlNodePtr parmcontain_node = XmlUtil::GetNode( label_node, "ParmContainer", 0 );
            if ( parmcontain_node )
            {
                CreateAndAddRuler();
                GetCurrentRuler()->DecodeXml( label_node );
            }
        }
    }

    int numofProbes = XmlUtil::FindInt( label_root_node, "Num_of_Probes", 0 );
    for ( int i = 0; i < numofProbes; i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Probe_%d", i );
        xmlNodePtr label_node = XmlUtil::GetNode( label_root_node, labelName, 0 );
        if( label_node )
        {
            xmlNodePtr parmcontain_node = XmlUtil::GetNode( label_node, "ParmContainer", 0 );
            if ( parmcontain_node )
            {
                CreateAndAddProbe();
                GetCurrentProbe()->DecodeXml( label_node );
            }
        }
    }

    int numofRSTprobes = XmlUtil::FindInt( label_root_node, "Num_of_RSTprobes", 0 );
    for ( int i = 0; i < numofRSTprobes; i++ )
    {
        snprintf( labelName, sizeof( labelName ), "RSTprobe_%d", i );
        xmlNodePtr label_node = XmlUtil::GetNode( label_root_node, labelName, 0 );
        if( label_node )
        {
            xmlNodePtr parmcontain_node = XmlUtil::GetNode( label_node, "ParmContainer", 0 );
            if ( parmcontain_node )
            {
                CreateAndAddRSTProbe();
                GetCurrentRSTProbe()->DecodeXml(label_node );
            }
        }
    }

    return label_root_node;
}

void MeasureMgrSingleton::DeleteInvalid()
{
    std::vector < Protractor * > valid_protractors;
    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        if ( m_Protractors[i]->Valid() )
        {
            valid_protractors.push_back( m_Protractors[i] );
        }
        else
        {
            delete m_Protractors[i];
        }
    }
    m_Protractors = valid_protractors;

    std::vector < Ruler * > valid_rulers;
    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        if ( m_Rulers[i]->Valid() )
        {
            valid_rulers.push_back( m_Rulers[i] );
        }
        else
        {
            delete m_Rulers[i];
        }
    }
    m_Rulers = valid_rulers;

    std::vector < Probe * > valid_probes;
    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        if ( m_Probes[i]->Valid() )
        {
            valid_probes.push_back( m_Probes[i] );
        }
        else
        {
            delete m_Probes[i];
        }
    }
    m_Probes = valid_probes;

    std::vector < RSTProbe * > valid_RSTprobes;
    for(int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        if ( m_RSTProbes[i]->Valid() )
        {
            valid_RSTprobes.push_back(m_RSTProbes[i] );
        }
        else
        {
            delete m_RSTProbes[i];
        }
    }
    m_RSTProbes = valid_RSTprobes;
}

void MeasureMgrSingleton::Update()
{
    DeleteInvalid();

    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        m_Protractors[i]->Update();
    }

    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        m_Rulers[i]->Update();
    }

    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        m_Probes[i]->Update();
    }

    for(int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        m_RSTProbes[i]->Update();
    }
}

void MeasureMgrSingleton::DelAllProtractors()
{
    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        delete m_Protractors[i];
    }
    m_Protractors.clear();

    m_CurrProtractorIndex = -1;
}

void MeasureMgrSingleton::ShowAllProtractors()
{
    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        m_Protractors[i]->m_Visible = true;
    }
}

void MeasureMgrSingleton::HideAllProtractors()
{
    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        m_Protractors[i]->m_Visible = false;
    }
}

Protractor * MeasureMgrSingleton::GetProtractor( const string &id )
{
    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        if ( m_Protractors[i]->GetID() == id )
        {
            return m_Protractors[i];
        }
    }

    return nullptr;
}

vector < string > MeasureMgrSingleton::GetAllProtractors()
{
    vector < string > protractorList( m_Protractors.size() );

    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        protractorList[i] = m_Protractors[i]->GetID();
    }

    return protractorList;
}

void MeasureMgrSingleton::DelProtractor( const int & i )
{
    if ( i < 0 || i >= ( int )m_Protractors.size() )
    {
        return;
    }

    Protractor* protractor = m_Protractors[i];

    m_Protractors.erase( m_Protractors.begin() +  i );

    delete protractor;
}

void MeasureMgrSingleton::DelProtractor( const string &id )
{
    int idel = -1;
    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        if ( m_Protractors[i]->GetID() == id )
        {
            idel = i;
            break;
        }
    }

    DelProtractor( idel );
}

void MeasureMgrSingleton::DelAllRulers()
{
    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        delete m_Rulers[i];
    }
    m_Rulers.clear();

    m_CurrRulerIndex = -1;
}

void MeasureMgrSingleton::ShowAllRulers()
{
    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        m_Rulers[i]->m_Visible = true;
    }
}

void MeasureMgrSingleton::HideAllRulers()
{
    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        m_Rulers[i]->m_Visible = false;
    }
}

Ruler * MeasureMgrSingleton::GetRuler( const string &id )
{
    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        if ( m_Rulers[i]->GetID() == id )
        {
            return m_Rulers[i];
        }
    }

    return nullptr;
}

vector < string > MeasureMgrSingleton::GetAllRulers()
{
    vector < string > rulerList( m_Rulers.size() );

    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        rulerList[i] = m_Rulers[i]->GetID();
    }

    return rulerList;
}

void MeasureMgrSingleton::DelRuler( const int & i )
{
    if ( i < 0 || i >= ( int )m_Rulers.size() )
    {
        return;
    }

    Ruler* ruler = m_Rulers[i];

    m_Rulers.erase( m_Rulers.begin() +  i );

    delete ruler;
}

void MeasureMgrSingleton::DelRuler( const string &id )
{
    int idel = -1;
    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        if ( m_Rulers[i]->GetID() == id )
        {
            idel = i;
            break;
        }
    }

    DelRuler( idel );
}

void MeasureMgrSingleton::DelAllProbes()
{
    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        delete m_Probes[i];
    }
    m_Probes.clear();

    m_CurrProbeIndex = -1;
}

void MeasureMgrSingleton::ShowAllProbes()
{
    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        m_Probes[i]->m_Visible = true;
    }
}

void MeasureMgrSingleton::HideAllProbes()
{
    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        m_Probes[i]->m_Visible = false;
    }
}

Probe * MeasureMgrSingleton::GetProbe( const string &id )
{
    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        if ( m_Probes[i]->GetID() == id )
        {
            return m_Probes[i];
        }
    }

    return nullptr;
}

vector < string > MeasureMgrSingleton::GetAllProbes()
{
    vector < string > probeList( m_Probes.size() );

    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        probeList[i] = m_Probes[i]->GetID();
    }

    return probeList;
}

void MeasureMgrSingleton::DelProbe( const int & i )
{
    if ( i < 0 || i >= ( int )m_Probes.size() )
    {
        return;
    }

    Probe* probe = m_Probes[i];

    m_Probes.erase( m_Probes.begin() +  i );

    delete probe;
}

void MeasureMgrSingleton::DelProbe( const string &id )
{
    int idel = -1;
    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        if ( m_Probes[i]->GetID() == id )
        {
            idel = i;
            break;
        }
    }

    DelProbe( idel );
}

void MeasureMgrSingleton::DelAllRSTProbes()
{
    for( int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        delete m_RSTProbes[i];
    }
    m_RSTProbes.clear();

    m_CurrRSTProbeIndex = -1;
}

void MeasureMgrSingleton::ShowAllRSTProbes()
{
    for( int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        m_RSTProbes[i]->m_Visible = true;
    }
}

void MeasureMgrSingleton::HideAllRSTProbes()
{
    for( int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        m_RSTProbes[i]->m_Visible = false;
    }
}

RSTProbe * MeasureMgrSingleton::GetRSTProbe(const string &id )
{
    for( int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        if (m_RSTProbes[i]->GetID() == id )
        {
            return m_RSTProbes[i];
        }
    }

    return nullptr;
}

vector < string > MeasureMgrSingleton::GetAllRSTProbes()
{
    vector < string > RSTprobeList(m_RSTProbes.size() );

    for( int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        RSTprobeList[i] = m_RSTProbes[i]->GetID();
    }

    return RSTprobeList;
}

void MeasureMgrSingleton::DelRSTProbe(const int & i )
{
    if ( i < 0 || i >= ( int )m_RSTProbes.size() )
    {
        return;
    }

    RSTProbe* RSTprobe = m_RSTProbes[i];

    m_RSTProbes.erase(m_RSTProbes.begin() + i );

    delete RSTprobe;
}

void MeasureMgrSingleton::DelRSTProbe(const string &id )
{
    int idel = -1;
    for( int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        if (m_RSTProbes[i]->GetID() == id )
        {
            idel = i;
            break;
        }
    }

    DelRSTProbe(idel);
}

void MeasureMgrSingleton::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    UpdateDrawObjs();

    for( int i = 0; i < ( int )m_Protractors.size(); i++ )
    {
        m_Protractors[i]->LoadDrawObjs( draw_obj_vec );
    }

    for( int i = 0; i < ( int )m_Rulers.size(); i++ )
    {
        m_Rulers[i]->LoadDrawObjs( draw_obj_vec );
    }

    for( int i = 0; i < ( int )m_Probes.size(); i++ )
    {
        m_Probes[i]->LoadDrawObjs( draw_obj_vec );
    }

    for( int i = 0; i < ( int )m_RSTProbes.size(); i++ )
    {
        m_RSTProbes[i]->LoadDrawObjs(draw_obj_vec );
    }
}

void MeasureMgrSingleton::UpdateDrawObjs()
{
    std::vector < Protractor * > protractorList = MeasureMgr.GetProtractorVec();
    for( int i = 0; i < ( int )protractorList.size(); i++ )
    {
        // Find out label type.
        Protractor * protractor = protractorList[i];

        vector < DrawObj * > draw_obj_vec;
        protractor->LoadDrawObjs( draw_obj_vec );


        if ( draw_obj_vec.size() > 0 )
        {
            DrawObj * match = draw_obj_vec[0];
            if( !match )
            {
                return;
            }

            // Set label base on type.
            if( protractor )
            {
                match->m_Type = DrawObj::VSP_PROTRACTOR;

                // Set label stage.  Load stage data.
                match->m_Protractor.Step = DrawObj::VSP_PROTRACTOR_STEP_ZERO;

                if ( protractor->m_Stage == STAGE_ONE )
                {
                    match->m_Protractor.Step = DrawObj::VSP_PROTRACTOR_STEP_ONE;
                }
                else if ( protractor->m_Stage == STAGE_TWO )
                {
                    match->m_Protractor.Step = DrawObj::VSP_PROTRACTOR_STEP_TWO;
                }
                else if ( protractor->m_Stage == STAGE_THREE )
                {
                    match->m_Protractor.Step = DrawObj::VSP_PROTRACTOR_STEP_THREE;
                }
                else if ( protractor->m_Stage == STAGE_COMPLETE )
                {
                    match->m_Protractor.Step = DrawObj::VSP_PROTRACTOR_STEP_COMPLETE;

                    // Load placement info to DrawObj.
                    match->m_Protractor.Offset = protractor->m_Offset.Get();
                }
            }
        }
    }

    std::vector < Ruler * > rulerList = MeasureMgr.GetRulerVec();
    for( int i = 0; i < ( int )rulerList.size(); i++ )
    {
        // Find out label type.
        Ruler * ruler = rulerList[i];

        vector < DrawObj * > draw_obj_vec;
        ruler->LoadDrawObjs( draw_obj_vec );


        if ( draw_obj_vec.size() > 0 )
        {
            DrawObj * match = draw_obj_vec[0];
            if( !match )
            {
                return;
            }

            // Set label base on type.
            if( ruler )
            {
                match->m_Type = DrawObj::VSP_RULER;

                // Set label stage.  Load stage data.
                match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_ZERO;

                if ( ruler->m_Stage == STAGE_ONE )
                {
                    match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_ONE;
                }
                else if ( ruler->m_Stage == STAGE_TWO )
                {
                    match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_TWO;
                }
                else if ( ruler->m_Stage == STAGE_COMPLETE )
                {
                    match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_COMPLETE;

                    // Load placement info to DrawObj.
                    match->m_Ruler.Offset = vec3d( ruler->m_XOffset.Get(),
                                                   ruler->m_YOffset.Get(),
                                                   ruler->m_ZOffset.Get() );
                }
            }
        }
    }

    std::vector < Probe * > probeList = MeasureMgr.GetProbeVec();
    for( int i = 0; i < ( int )probeList.size(); i++ )
    {
        // Find out label type.
        Probe * probe = probeList[i];

        vector < DrawObj * > draw_obj_vec;
        probe->LoadDrawObjs( draw_obj_vec );


        if ( draw_obj_vec.size() > 0 )
        {
            DrawObj * match = draw_obj_vec[0];
            if( !match )
            {
                return;
            }

            // Set label base on type.
            if( probe )
            {
                match->m_Type = DrawObj::VSP_PROBE;

                // Set label stage.  Load stage data.
                match->m_Probe.Step = DrawObj::VSP_PROBE_STEP_ZERO;

                if ( probe->m_Stage == STAGE_ONE )
                {
                    match->m_Probe.Step = DrawObj::VSP_PROBE_STEP_ONE;
                }
                else if ( probe->m_Stage == STAGE_COMPLETE )
                {
                    match->m_Probe.Step = DrawObj::VSP_PROBE_STEP_COMPLETE;

                    // Load placement info to DrawObj.
                    match->m_Probe.Len = probe->m_Len();
                }
            }
        }
    }

    std::vector < RSTProbe * > RSTprobeList = MeasureMgr.GetRSTProbeVec();
    for( int i = 0; i < ( int )RSTprobeList.size(); i++ )
    {
        // Find out label type.
        RSTProbe * RSTprobe = RSTprobeList[i];

        vector < DrawObj * > draw_obj_vec;
        RSTprobe->LoadDrawObjs( draw_obj_vec );


        if ( draw_obj_vec.size() > 0 )
        {
            DrawObj * match = draw_obj_vec[0];
            if( !match )
            {
                return;
            }

            // Set label base on type.
            if( RSTprobe )
            {
                match->m_Type = DrawObj::VSP_PROBE;

                // Set label stage.  Load stage data.
                match->m_Probe.Step = DrawObj::VSP_PROBE_STEP_ZERO;

                if ( RSTprobe->m_Stage == STAGE_ONE )
                {
                    match->m_Probe.Step = DrawObj::VSP_PROBE_STEP_ONE;
                }
                else if ( RSTprobe->m_Stage == STAGE_COMPLETE )
                {
                    match->m_Probe.Step = DrawObj::VSP_PROBE_STEP_COMPLETE;

                    // Load placement info to DrawObj.
                    match->m_Probe.Len = RSTprobe->m_Len();
                }
            }
        }
    }
}
