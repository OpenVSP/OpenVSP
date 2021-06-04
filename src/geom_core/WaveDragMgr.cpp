//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// WaveDragMgr.cpp
//
// Michael Waddington
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "MeshGeom.h"
#include "WingGeom.h"

#include "wavedragEL.h"

WaveDragSingleton::WaveDragSingleton() : ParmContainer()
{
    m_Name = "WaveDragSettings";

    //==== Run Tab ====//
    m_NumSlices.Init( "NumSlices", "WaveDrag", this, 20, 3, 100 );
    m_NumSlices.SetDescript( "Number of Wave Drag Slices" );

    m_NumRotSects.Init( "NumRotSects", "WaveDrag", this, 10, 3, 30 );
    m_NumRotSects.SetDescript( "Number of Rotation Sections" );

    m_MachNumber.Init( "MachNumber", "WaveDrag", this, 1.5, 1, 5 );
    m_MachNumber.SetDescript( "Mach Number" );

    m_SelectedSetIndex.Init( "SelSetIndex", "WaveDrag", this, DEFAULT_SET, 0, NUM_SETS + 2 );
    m_SelectedSetIndex.SetDescript( "Selected Set Index" );

    m_RefFlag.Init( "RefFlag", "WaveDrag", this, vsp::MANUAL_REF, vsp::MANUAL_REF, vsp::COMPONENT_REF );
    m_RefFlag.SetDescript( "Reference quantity flag" );

    m_Sref.Init( "Sref", "WaveDrag", this, 100, 0.001, 1.0e12 );
    m_Sref.SetDescript( "Reference Area" );

    m_SymmFlag.Init( "SymmFlag", "WaveDrag", this, true, 0, 1 );

    //==== Plot Tab ====//
    m_PlaneFlag.Init( "PlaneFlag", "WaveDrag", this, false, 0, 1 );

    m_ThetaIndex.Init( "ThetaIndex", "WaveDrag", this, 1, 1, 1e4 );

    m_SlicingLoc.Init( "SlicingLoc", "WaveDrag", this, 0, -1e12, 1e12 );
    m_SlicingLoc.SetDescript( "View slice location" );

    m_IdealBodyType.Init( "SelBodyIndex", "WaveDrag", this, 0, 0, 1.0e12 );
    m_IdealBodyType.SetDescript( "Selected Body of Rev Index" );

    m_AreaPlotType.Init( "AreaPlotType", "WaveDrag", this, AREA_TOTAL, AREA_TOTAL, AREA_BUILDUP );
    m_AreaPlotType.SetDescript( "Area distribution plot type" );

    m_PointFlag.Init( "PointPlotFlag", "WaveDrag", this, true, 0, 1 );

    m_lastmeshgeomID = string();
    m_NumPtsFit = 100;

    m_XNormFit.resize( m_NumPtsFit );
    for ( int i = 0; i < m_NumPtsFit; i++ )
    {
        m_XNormFit[i] = ( double ) i / (double) ( m_NumPtsFit - 1 );
    }
}

xmlNodePtr WaveDragSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr WaveDragnode = xmlNewChild( node, NULL, BAD_CAST"WaveDragMgr", NULL );

    ParmContainer::EncodeXml( WaveDragnode );

    XmlUtil::AddStringNode( WaveDragnode, "ReferenceGeomID", m_RefGeomID );

    //===== Flow-Through Subsurfaces ====//
    xmlNodePtr flowSS_list_node = xmlNewChild( WaveDragnode, NULL, ( const xmlChar * )"FlowSS_List", NULL );
    for ( int i = 0 ; i < ( int )m_SSFlow_vec.size() ; i++ )
    {
        xmlNodePtr flowSS_node = xmlNewChild( flowSS_list_node, NULL, ( const xmlChar * )"FlowSS", NULL );
        XmlUtil::AddStringNode( flowSS_node, "flowSS_ID", m_SSFlow_vec[i] );
    }

    return WaveDragnode;
}

xmlNodePtr WaveDragSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr WaveDragnode = XmlUtil::GetNode( node, "WaveDragMgr", 0 );

    if ( WaveDragnode )
    {
        ParmContainer::DecodeXml( WaveDragnode );
        m_RefGeomID   = XmlUtil::FindString( WaveDragnode, "ReferenceGeomID", m_RefGeomID );

        //==== Flow-Through Subsurfaces ====//
        xmlNodePtr flowSS_list_node = XmlUtil::GetNode( WaveDragnode, "FlowSS_List", 0 );

        if ( flowSS_list_node )
        {
            int num_flowSS =  XmlUtil::GetNumNames( flowSS_list_node, "FlowSS" );
            for ( int i = 0 ; i < num_flowSS ; i++ )
            {
                xmlNodePtr flowSS_node = XmlUtil::GetNode( flowSS_list_node, "FlowSS", i );
                string new_flowSS = XmlUtil::FindString( flowSS_node, "flowSS_ID", string() );
                m_SSFlow_vec.push_back( new_flowSS );
            }
        }
    }

    return WaveDragnode;
}

void WaveDragSingleton::Update()
{
    if( m_RefFlag() == vsp::MANUAL_REF )
    {
        m_Sref.Activate();
    }
    else
    {
        Geom* refgeom = VehicleMgr.GetVehicle()->FindGeom( m_RefGeomID );

        if( refgeom )
        {
            if( refgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
            {
                WingGeom* refwing = (WingGeom*) refgeom;
                m_Sref.Set( refwing->m_TotalArea() );

                m_Sref.Deactivate();
            }
        }
        else
        {
            m_RefGeomID = string();
        }
    }
}

void WaveDragSingleton::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        assert( false );
        return;
    }

    // Clear point and normal vectors
    m_ShadeMeshViewPlane.m_PntVec.clear();
    m_ShadeMeshViewPlane.m_NormVec.clear();
    m_ViewPlaneLine.m_PntVec.clear();

    // Get bounding box size for cutting plane visualizer dimensions
    veh->UpdateBBox();
    BndBox BBox;
    BBox = veh->GetBndBox();

    double maxdel = 1.02 * BBox.GetLargestDist();
    // Get y, z length of bounding box
    double ydel = BBox.GetMax( 1 ) - BBox.GetMin( 1 );
    double ycenter = BBox.GetMin( 1 ) + ydel/2;
    double zdel = BBox.GetMax( 2 ) - BBox.GetMin( 2 );
    double zcenter = BBox.GetMin( 2 ) + zdel/2;

    // Establish four basic corners of cutting plane visuaizer, with norm in positive x-direction
    int nq = 4;
    vector< vec3d > quads(nq);
    quads[0].set_xyz( 0,  maxdel,  maxdel );
    quads[1].set_xyz( 0, -maxdel,  maxdel );
    quads[2].set_xyz( 0, -maxdel, -maxdel );
    quads[3].set_xyz( 0,  maxdel, -maxdel );

    // Get Mach angle and theta values
    double MAngle = asin( 1 / m_MachNumber.Get() );

    int itheta = m_ThetaIndex() - 1;
    double theta = m_ThetaRad[ itheta ];

    for ( int i = 0; i < nq; i++ )
    {
        // Rotate cutting plane visualizer to Mach angle
        quads[i].rotate_y( cos(-( 0.5*PI-MAngle)), sin(-( 0.5*PI-MAngle)) );
        // Rotate cutting plane visualizer to current theta
        quads[i].rotate_x( cos(theta), sin(theta) );
        // Move cutting plane visualizer to current slice section location
        quads[i].offset_x( m_SlicingLoc.Get() );
        quads[i].offset_y( ycenter );
        quads[i].offset_z( zcenter );
    }

    // Get new normal
    vec3d quadnorm = cross( quads[3] - quads[2], quads[1] - quads[2] );
    quadnorm.normalize();

    // Establish OpenGL shaded plane
    m_ShadeMeshViewPlane.m_GeomID = "IDFORVIEWPLANE";
    m_ShadeMeshViewPlane.m_Type = DrawObj::VSP_SHADED_TRIS;

    // Create the triangles
    m_ShadeMeshViewPlane.m_PntVec.push_back(quads[0]);
    m_ShadeMeshViewPlane.m_PntVec.push_back(quads[1]);
    m_ShadeMeshViewPlane.m_PntVec.push_back(quads[3]);
    m_ShadeMeshViewPlane.m_PntVec.push_back(quads[1]);
    m_ShadeMeshViewPlane.m_PntVec.push_back(quads[2]);
    m_ShadeMeshViewPlane.m_PntVec.push_back(quads[3]);

    for ( int i = 0; i < 6; i++ )
    {
        m_ShadeMeshViewPlane.m_NormVec.push_back(quadnorm);
    }

    // Set plane color to medium glass
    for ( int i = 0; i < 4; i++ )
    {
        m_ShadeMeshViewPlane.m_MaterialInfo.Ambient[i] = 0.2f;
        m_ShadeMeshViewPlane.m_MaterialInfo.Diffuse[i] = 0.1f;
        m_ShadeMeshViewPlane.m_MaterialInfo.Specular[i] = 0.7f;
        m_ShadeMeshViewPlane.m_MaterialInfo.Emission[i] = 0.0f;
    }
    m_ShadeMeshViewPlane.m_MaterialInfo.Diffuse[3] = 0.5f;
    m_ShadeMeshViewPlane.m_MaterialInfo.Shininess = 5.0f;

    // Push draw object for cutting plane visualizer
    draw_obj_vec.push_back( &m_ShadeMeshViewPlane );

    // Establish OpenGL line loop for cutting plane visuaizer edge
    m_ViewPlaneLine.m_GeomID = "IDFORVIEWPLANELINE";
    m_ViewPlaneLine.m_Type = DrawObj::VSP_LINE_LOOP;
    m_ViewPlaneLine.m_LineColor = vec3d( 0, 0, 0 );
    m_ViewPlaneLine.m_LineWidth = 2.0;

    for ( int i = 0; i < nq; i++ )
    {
        m_ViewPlaneLine.m_PntVec.push_back(quads[i]);
    }

    // Push draw object for line loop
    draw_obj_vec.push_back( &m_ViewPlaneLine );
}

string WaveDragSingleton::SliceAndAnalyze()
{
    return SliceAndAnalyze( m_SelectedSetIndex.Get(), m_NumSlices.Get(),
            m_NumRotSects.Get(), m_MachNumber.Get(),
            m_SSFlow_vec, m_SymmFlag.Get() );
}

string WaveDragSingleton::SliceAndAnalyze( int set, int numSlices, int numRots, double Mach,
        const vector <string> & Flow_vec, bool Symm )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        assert( false );
        return string();
    }

    // Terminates slicing routine if no Geoms in desired set
    vector<string> geomlist = veh->GetGeomSet( set );
    if ( geomlist.empty() )
    {
        return string();
    }

    // Delete MeshGeom from previous wave drag tool run
    veh->DeleteGeom( m_lastmeshgeomID );

    // Run slicing routine
    m_lastmeshgeomID = WaveDragSlice( set, numSlices, numRots, Mach, Flow_vec, Symm );

    // Send ambiguous flow face message if necessary
    if ( m_AmbigSubSurf )
    {
        MessageData errMsgData;
        errMsgData.m_String = "Error";
        errMsgData.m_IntVec.push_back( vsp::VSP_AMBIGUOUS_SUBSURF );
        char buf[255];
        sprintf( buf, "Error: Ambiguous flow-through subsurface detected" );
        errMsgData.m_StringVec.push_back( string( buf ) );
        MessageMgr::getInstance().SendAll( errMsgData );
    }

    //==== Create Results ====//
    Results* res = ResultsMgr.CreateResults( "WaveDrag" );

    res->Add( NameValData( "Mach", Mach ) );

    res->Add( NameValData( "Mesh_GeomID", m_lastmeshgeomID ) );

    PushSliceResults( res );

    CalcDrag();

    PushDragResults( res );

    FitBuildup();

    string filename = veh->getExportFileName( vsp::WAVE_DRAG_TXT_TYPE );

    string str_mach = "M" + std::to_string( res->Find( "Mach" ).GetDouble( 0 ) );

    vector< string > set_name_vec = veh->GetSetNameVec();

    int set_index = m_SelectedSetIndex.Get();
    string set_name = set_name_vec[set_index];
    res->Add( NameValData( "Set_Name", set_name ) );

    string to_insert = "_" + str_mach + "_" + set_name;
    filename.insert( filename.find( ".txt" ), to_insert );

    res->WriteWaveDragFile( filename );

    return res->GetID();
}

void WaveDragSingleton::SetupTheta( int ntheta )
{
    m_NTheta = ntheta;

    m_ThetaIndex.SetUpperLimit( m_NTheta );

    m_ThetaRad.resize( m_NTheta );
    m_StartX.resize( m_NTheta );
    m_EndX.resize( m_NTheta );

    for ( int i = 0; i < m_NTheta; i++ )
    {
        m_ThetaRad[i] = ( 2.0 * PI * (double)i ) / ( m_NTheta );
        if ( m_SymmFlag() )
        {
            m_ThetaRad[i] = ( PI * (double)i ) / ( m_NTheta - 1 );
        }
    }
}

void WaveDragSingleton::Setup( int nslice, int ncomp )
{
    m_NSlice = nslice;
    m_NComp = ncomp;

    m_CompSliceAreaDist.resize( m_NTheta );
    m_SliceAreaDist.resize( m_NTheta );

    for ( int i = 0; i < m_NTheta; i++ )
    {
        m_CompSliceAreaDist[i].resize( m_NComp );
        m_SliceAreaDist[i].resize( m_NSlice );

        for ( int j = 0; j < m_NComp; j++ )
        {
            m_CompSliceAreaDist[i][j].resize( m_NSlice );
        }
    }
}

string WaveDragSingleton::WaveDragSlice( int set, int numSlices, int numRots, double Mach,
                const vector< string > & SSFlow_vec, bool Symm )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        assert( false );
        return string( "None" );
    }

    string id = veh->AddMeshGeom( set );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    veh->HideAllExcept( id );

    MeshGeom* mesh_ptr = ( MeshGeom* ) veh->FindGeom( id );
    if ( !mesh_ptr )
    {
        return id;
    }

    if ( mesh_ptr->m_TMeshVec.size() )
    {
        // Compute Mach angle
        double MachAngle = asin( 1.0 / Mach );

        // Execute slicing routine in MeshGeom
        mesh_ptr->WaveDragSlice( numSlices, MachAngle, numRots, SSFlow_vec, Symm );

        mesh_ptr->FlattenTMeshVec();
        mesh_ptr->FlattenSliceVec();
        mesh_ptr->m_SurfDirty = true;
        mesh_ptr->Update();
    }
    else
    {
        veh->CutActiveGeomVec();
        veh->DeleteClipBoard();
        id = "NONE";
    }

    return id;
}

double WaveDragSingleton::WaveDrag( int r )
{
    unsigned int n = m_XNorm.size();
    vector<double> x( n );
    m_SliceAreaDistFlow[r].resize( n );
    for ( int i = 0; i < n; i++ )
    {
        x[i] = m_StartX[r] + m_XNorm[i] * ( m_EndX[r] - m_StartX[r] );
        m_SliceAreaDistFlow[r][i] = m_SliceAreaDist[r][i] - m_InletArea * (1.0 - m_XNorm[i]) - m_ExitArea * m_XNorm[i];
    }

    vector < double > ELControlPoints;
    ELControlPoints.resize( m_NSlice - 2 );

    // Run WaveDrag
    double Donq = WaveDrag( x, m_SliceAreaDistFlow[r], ELControlPoints );

    EvalELCurve( ELControlPoints, m_SliceAreaDistFlow[r], m_XNormFit, m_FitAreaDistFlow[r], m_XMaxDrag[r] );

    m_FitAreaDist[r].resize( m_NumPtsFit );
    for ( int i = 0; i < m_NumPtsFit; i++ )
    {
        m_FitAreaDist[r][i] = m_FitAreaDistFlow[r][i] + m_InletArea * (1.0 - m_XNormFit[i]) + m_ExitArea * m_XNormFit[i];
    }
    return Donq;
}

double WaveDragSingleton::WaveDrag( const vector <double> & x_vec, const vector <double> & area_vec, vector <double> &coeff )
{
    int n = x_vec.size();
    vector < double > slicex = x_vec;
    vector < double > sliceS = area_vec;

    double Donq = emlord( n, &slicex[0], &sliceS[0], &coeff[0] );
    return Donq;
}

void WaveDragSingleton::CalcDrag( )
{
    // Calculate drag around control cylinder and integrate.
    double D = 0;
    double dtheta = 1.0 / m_NTheta;
    if ( m_SymmFlag() )
    {
        dtheta = 1.0 / ( m_NTheta - 1 );
    }

    m_Volume.resize( m_NTheta );
    m_MaxArea.resize( m_NTheta );
    m_Length.resize( m_NTheta );
    m_MaxMaxArea = m_SliceAreaDist[0][0];

    m_FitAreaDist.resize( m_NTheta );
    m_XMaxDrag.resize( m_NTheta );

    m_SliceAreaDistFlow.resize( m_NTheta );
    m_FitAreaDistFlow.resize( m_NTheta );

    int numpts = 100;

    vector< double > XNorm_locs;
    for ( int i = 0; i < numpts; i++ )
    {
        double xnorm = ( double )i / ( (double)numpts - 1.0 );
        XNorm_locs.push_back( xnorm );
    }

    m_iMaxDrag = -1;
    double dmax = -1.0;

    for ( int i = 0; i < m_NTheta; i++ )
    {
        double k = 1.0;
        if ( m_SymmFlag() && ( i == 0 || i == m_NTheta - 1 ) )
        {
            k = 0.5;
        }
        double dd = WaveDrag( i );
        D += dd * dtheta * k;

        if ( dd > dmax )
        {
            dmax = dd;
            m_iMaxDrag = i;
        }

        m_Length[i] = m_EndX[i] - m_StartX[i];

        CalcVolArea( m_XNormFit, m_FitAreaDistFlow[i], m_Length[i], m_Volume[i], m_MaxArea[i] );
        m_MaxMaxArea  = std::max( m_MaxMaxArea, m_MaxArea[i] );

    }
    m_CDWave = D / m_Sref();
}

void WaveDragSingleton::PushDragResults( Results* res )
{
    res->Add( NameValData( "CDWave", m_CDWave ) );

    res->Add( NameValData( "Length", m_Length ) );
    res->Add( NameValData( "Volume", m_Volume ) );
    res->Add( NameValData( "MaxArea", m_MaxArea ) );
}

void WaveDragSingleton::CalcVolArea( const vector < double > &x, const vector < double > &S, double length, double &volume, double &maxArea ) const
{
    volume = 0;

    int n = x.size();

    maxArea = S[0];
    for ( int j = 1; j < n; j++ )
    {
        maxArea = std::max( maxArea, S[j] );
        volume += 0.5 * ( S[j-1] + S[j] ) * ( x[j] - x[j-1] ) * length;
    }
}

void WaveDragSingleton::FitBuildup()
{
    m_CompFitAreaDist.resize( m_NTheta );
    m_BuildupFitAreaDist.resize( m_NTheta );
    m_BuildupAreaDist.resize( m_NTheta );

    for ( int itheta = 0; itheta < m_NTheta; itheta++ )
    {
        m_CompFitAreaDist[itheta].resize( m_NComp );
        m_BuildupFitAreaDist[itheta].resize( m_NComp );
        m_BuildupAreaDist[itheta].resize( m_NComp );

        vector < double > xptsdim( m_NSlice );

        for ( int i = 0; i < m_NSlice; i++ )
        {
            xptsdim[i] = m_StartX[itheta] + m_XNorm[i] * ( m_EndX[itheta] - m_StartX[itheta] );
        }

        vector < double > Svec = m_SliceAreaDist[itheta];
        // Components
        for ( int icomp = 0; icomp < m_NComp; icomp++ )
        {
            m_BuildupAreaDist[itheta][icomp] = Svec;

            double xdum;
            vector < double > conpnts( m_NSlice - 2 );
            WaveDrag( xptsdim, Svec, conpnts );
            EvalELCurve( conpnts, Svec, m_XNormFit, m_BuildupFitAreaDist[itheta][icomp], xdum );

            WaveDrag( xptsdim, m_CompSliceAreaDist[itheta][icomp], conpnts );
            EvalELCurve( conpnts, m_CompSliceAreaDist[itheta][icomp], m_XNormFit, m_CompFitAreaDist[itheta][icomp], xdum );

            for ( int i = 0; i < m_NSlice; i++ )
            {
                Svec[i] -= m_CompSliceAreaDist[itheta][icomp][i];
            }
        }
    }
}

void WaveDragSingleton::PushSliceResults( Results* res )
{
    //==== Add Mesh Results after component and subsurface intersection ====//
    res->Add( NameValData( "Ambiguous_Subsurf_Flag", m_AmbigSubSurf ) );

    res->Add( NameValData( "Num_Slices", m_NSlice ) );

    vector <double> theta_deg( m_NTheta );
    for ( int i = 0; i < m_NTheta; i++ )
    {
        theta_deg[i] = m_ThetaRad[i] * 180.0 / PI;

        res->Add( NameValData( "Slice_Area", m_SliceAreaDist[i] ) );
    }

    res->Add( NameValData( "Theta", theta_deg ) );
    res->Add( NameValData( "Num_Cone_Sections", m_NTheta ) );

    res->Add( NameValData( "Inlet_Area", m_InletArea ) );
    res->Add( NameValData( "Exit_Area", m_ExitArea ) );

    res->Add( NameValData( "X_Norm", m_XNorm ) );
    res->Add( NameValData( "Start_X", m_StartX ) );
    res->Add( NameValData( "End_X", m_EndX ) );
}

void WaveDragSingleton::EvalELCurve( const vector < double > &Spts, const vector < double > &x, vector < double > &S, double &xMaxDrag, int itheta )
{
    vector < double > xptsdim( m_NSlice );

    for ( int i = 0; i < m_NSlice; i++ )
    {
        xptsdim[i] = m_StartX[itheta] + m_XNorm[i] * ( m_EndX[itheta] - m_StartX[itheta] );
    }

    vector < double > conpnts( m_NSlice - 2 );
    WaveDrag( xptsdim, Spts, conpnts );

    EvalELCurve( conpnts, Spts, x, S, xMaxDrag );
}

void WaveDragSingleton::EvalELCurve( const vector < double > & conpnts, const vector < double > &Spts, const vector < double > &x, vector < double > &S, double &xMaxDrag )
{
    S.clear();

    double S2max = -1;
    xMaxDrag = 0.0;

    int n = x.size();

    // Create Fourier coefficients
    vector<double> a;
    a.push_back( 0 );
    for ( int r2 = 2; r2 < n + 1; r2++ )
    {
        double asum = 0;
        for ( int i = 0; i < m_NSlice - 2; i++ )
        {
            double kap = acos( 1.0 - 2.0 * m_XNorm[ i + 1 ] );
            asum += conpnts[i] * ( sin( ((double)r2 - 1.0 ) * kap )/((double)r2 - 1.0 ) - sin( ((double)r2 + 1.0 ) * kap ) / ( (double)r2 + 1.0 ) ) * (1.0 / (double)r2 );
        }
        a.push_back( asum );
    }

    double Sstart = Spts[0];
    double Send = Spts.back();

    // Create S values for Fourier curve
    S.push_back( Sstart );
    for ( int i=1; i<n-1; i++ )
    {
        double S2sum = 0;
        double theta = acos( 1 - 2*x[i] );
        double Ssum = Sstart + 0.25 * (4.0 / PI) * ( Send - Sstart ) * (theta - sin( theta ) * cos( theta ) );
        for ( int r2=2; r2<n+1; r2++ )
        {
            Ssum += 0.25 * ( sin( ((double)r2 - 1.0 ) * theta ) / ((double)r2 - 1.0 ) - sin( ((double)r2 + 1.0 ) * theta ) / ((double)r2 + 1.0 ) ) * a[r2-1];
            S2sum += cos( (double)r2*theta ) * (double)r2 * ( 1 / sqrt( -(x[i]-1)*x[i] ) ) * a[r2-1];
        }
        S.push_back( Ssum );

        if ( std::abs(S2sum) > S2max )
        {
            S2max = std::abs(S2sum);
            xMaxDrag = x[i];
        }
    }
    S.push_back( Send );
}

// Sears Haack body
void WaveDragSingleton::SearsHaack( const vector < double > &xvec, vector < double > & Svec, double Vol, double len ) const
{
    // Sears-Haack curve
    unsigned int n = xvec.size();
    Svec.resize( n );
    for ( unsigned int i = 0; i < n; i++ )
    {
        double x = xvec[i];
        Svec[i] = ( 16.0 * Vol / ( 3.0 * PI * len ) ) * pow( ( ( 4.0 * x ) * ( 1.0 - x ) ), 1.5 );
    }
}

// von Karman Ogive
void WaveDragSingleton::vonKarman( const vector < double > &xvec, vector < double > & Svec, double Sbase ) const
{
    unsigned int n = xvec.size();
    Svec.resize( n );
    for ( unsigned int i = 0; i < n; i++ )
    {
        double x = xvec[i];
        Svec[i] = ( 2.0 / PI ) * Sbase * ( asin( sqrt( x ) ) - ( 1.0 - 2.0 * x ) * sqrt( x * ( 1.0 - x ) ) );
    }
}

// Lighthill body
void WaveDragSingleton::Lighthill( const vector < double > &xvec, vector < double > & Svec, double Smax ) const
{
    // Sears-Haack curve
    unsigned int n = xvec.size();
    Svec.resize( n );
    for ( unsigned int i = 0; i < n; i++ )
    {
        double x = xvec[i];
        Svec[i] = Smax * ( 2.0 * sqrt( x * ( 1 - x ) ) - ( 1.0 - 2.0 * x ) * ( 1.0 - 2.0 * x ) * acosh( std::abs( 1.0 / ( 1.0 - 2.0 * x ) ) ) );
    }
}
