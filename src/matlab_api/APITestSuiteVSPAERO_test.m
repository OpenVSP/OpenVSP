% APITestSuiteVSPAERO_test
function tests = APITestSuiteVSPAERO_test()
    tests = functiontests(localfunctions);
end

%% Fresh Fixture Functions
function setup(~)  % do not change function name
    global VSPAERO_PATH
    
    vsp.VSPCheckSetup();
    vsp.VSPRenew();
    vsp.SetVSPAEROPath(VSPAERO_PATH);
    
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
end

function teardown(~)  % do not change function name
end

%% void APITestSuiteVSPAERO::TestVSPAeroCreateModel()
function TestVSPAeroCreateModel(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests

    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroCreateModel()\n' );

    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Add Wing Geom and set some parameters =====% //
    fprintf( '\tAdding WING (MainWing)...' );
    wing_id = vsp.AddGeom( 'WING' );
    vsp.SetGeomName( wing_id, 'MainWing' );
    assert( ~isempty(wing_id) );
    assert_delta( vsp.SetParmValUpdate( wing_id, 'TotalSpan', 'WingGeom', 17.0 ), 17.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( wing_id, 'Z_Rel_Location', 'XForm', 0.5 ), 0.5, TEST_TOL );
    % // Adjust chordwise tesselation
    assert_delta( vsp.SetParmValUpdate( wing_id, 'Tess_W', 'Shape', 32 ), calcTessWCheckVal( 32), TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( wing_id, 'LECluster', 'WingGeom', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( wing_id, 'TECluster', 'WingGeom', 2.0 ), 2.0, TEST_TOL );
    % // Adjust spanwise tesselation
    assert_delta( vsp.SetParmValUpdate( wing_id, 'SectTess_U', 'XSec_1', 15 ), 15, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( wing_id, 'InCluster', 'XSec_1', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( wing_id, 'OutCluster', 'XSec_1', 0.1 ), 0.1, TEST_TOL );
    % // Set to cambered airfoils
    xsec_surf_index = 0;
    xsec_surf_id = vsp.GetXSecSurf( wing_id, xsec_surf_index );

    % // Root airfoil - NACA 2412
    xsec_index = 0;
    vsp.ChangeXSecShape( xsec_surf_id, xsec_index, vsp.XS_FOUR_SERIES );
    xsec_id = vsp.GetXSec( xsec_surf_id, xsec_index );
    assert( vsp.XS_FOUR_SERIES == vsp.GetXSecShape( xsec_id ) );
    parm_id = vsp.GetXSecParm( xsec_id, 'Camber' );
    assert_delta( vsp.SetParmVal( parm_id, 0.02 ), 0.02, TEST_TOL );
    parm_id = vsp.GetXSecParm( xsec_id, 'CamberLoc' );
    assert_delta( vsp.SetParmVal( parm_id, 0.4 ), 0.4, TEST_TOL );
    parm_id = vsp.GetXSecParm( xsec_id, 'ThickChord' );
    assert_delta( vsp.SetParmVal( parm_id, 0.12 ), 0.12, TEST_TOL );
    % // Tip airfoil - NACA 2210
    xsec_index = 1;
    vsp.ChangeXSecShape( xsec_surf_id, xsec_index, vsp.XS_FOUR_SERIES );
    xsec_id = vsp.GetXSec( xsec_surf_id, xsec_index );
    assert( vsp.XS_FOUR_SERIES == vsp.GetXSecShape( xsec_id ) );
    parm_id = vsp.GetXSecParm( xsec_id, 'Camber' );
    assert_delta( vsp.SetParmVal( parm_id, 0.02 ), 0.02, TEST_TOL );
    parm_id = vsp.GetXSecParm( xsec_id, 'CamberLoc' );
    assert_delta( vsp.SetParmVal( parm_id, 0.2 ), 0.2, TEST_TOL );
    parm_id = vsp.GetXSecParm( xsec_id, 'ThickChord' );
    assert_delta( vsp.SetParmVal( parm_id, 0.10 ), 0.10, TEST_TOL );

    % // Add aileron control surface
    aileron_id = vsp.AddSubSurf( wing_id, double(vsp.SS_CONTROL) );
    vsp.SetSubSurfName( wing_id, aileron_id, 'Aileron' );
    assert( ~isempty(aileron_id) );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Add Vertical tail and set some parameters =====% //
    fprintf( '\tAdding WING (Vert)...' );
    vert_id = vsp.AddGeom( 'WING' );
    vsp.SetGeomName( vert_id, 'Vert' );
    assert( ~isempty(vert_id) );
    assert_delta( vsp.SetParmValUpdate( vert_id, 'TotalArea', 'WingGeom', 10.0 ), 10.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( vert_id, 'X_Rel_Location', 'XForm', 8.5 ), 8.5, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( vert_id, 'Z_Rel_Location', 'XForm', 0.2 ), 0.2, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( vert_id, 'X_Rel_Rotation', 'XForm', 90 ), 90, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( vert_id, 'Sym_Planar_Flag', 'Sym', 0 ), 0, TEST_TOL );
    % // Adjust chordwise tesselation
    assert_delta( vsp.SetParmValUpdate( vert_id, 'Tess_W', 'Shape', 20 ), calcTessWCheckVal( 20 ), TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( vert_id, 'LECluster', 'WingGeom', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( vert_id, 'TECluster', 'WingGeom', 2.0 ), 2.0, TEST_TOL );
    % // Adjust spanwise tesselation
    assert_delta( vsp.SetParmValUpdate( vert_id, 'SectTess_U', 'XSec_1', 8 ), 8, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( vert_id, 'InCluster', 'XSec_1', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( vert_id, 'OutCluster', 'XSec_1', 0.1 ), 0.1, TEST_TOL );
    % // Add rudder control surface
    rudder_id = vsp.AddSubSurf( vert_id, vsp.SS_CONTROL );
    vsp.SetSubSurfName( vert_id, rudder_id, 'Rudder' );
    assert( ~isempty(rudder_id) );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Add Horizontal tail and set some parameters =====% //
    fprintf( '\tAdding WING (Horiz)...' );
    horiz_id = vsp.AddGeom( 'WING' );
    vsp.SetGeomName( horiz_id, 'Horiz' );
    assert( ~isempty(horiz_id) );
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'TotalArea', 'WingGeom', 10.0 ), 10.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'X_Rel_Location', 'XForm', 8.5 ), 8.5, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'Z_Rel_Location', 'XForm', 0.2 ), 0.2, TEST_TOL );
    % // Adjust chordwise tesselation
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'Tess_W', 'Shape', 20 ), calcTessWCheckVal( 20 ), TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'LECluster', 'WingGeom', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'TECluster', 'WingGeom', 2.0 ), 2.0, TEST_TOL );
    % // Adjust spanwise tesselation
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'SectTess_U', 'XSec_1', 8 ), 8, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'InCluster', 'XSec_1', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'OutCluster', 'XSec_1', 0.1 ), 0.1, TEST_TOL );
    % // Add elevator control surface
    elevator_id = vsp.AddSubSurf( horiz_id, vsp.SS_CONTROL );
    vsp.SetSubSurfName( horiz_id, elevator_id, 'Elevator' );
    assert( ~isempty(elevator_id) );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Add Pod and set some parameters =====% //
    fprintf( '\tAdding POD...' );
    pod_id = vsp.AddGeom( 'POD' );
    assert( ~isempty(pod_id) );
    assert_delta( vsp.SetParmValUpdate( pod_id, 'Length', 'Design', 14.5 ), 14.5, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( pod_id, 'X_Rel_Location', 'XForm', -3.0 ), -3.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( wing_id, 'Tess_U', 'Shape', 15 ), 15, TEST_TOL ); % //lengthwise tesselation
    assert_delta( vsp.SetParmValUpdate( wing_id, 'Tess_W', 'Shape', 15 ), calcTessWCheckVal( 15 ), TEST_TOL ); % //radial tesselation
    fprintf( 'COMPLETE\n' );

    % //==== Set VSPAERO Reference lengths & areas ====% //
    fprintf( '\tSetting reference wing...' );
    % // Set as reference wing for VSPAERO
    assert( strcmp(wing_id,vsp.SetVSPAERORefWingID( wing_id )) );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Set VSPAERO Xcg position ====% //
    fprintf( '\tSetting reference position...' );
    vspaero_settings_container_id = vsp.FindContainer( 'VSPAEROSettings', 0 );
    xcg_id = vsp.FindParm( vspaero_settings_container_id, 'Xcg', 'VSPAERO' );
    assert_delta( vsp.SetParmValUpdate( xcg_id, 2 ), 2, TEST_TOL );
    fprintf( 'COMPLETE\n' );

    % //==== Auto Group Control Surfaces ====% //
    fprintf( '\tGrouping Control Surfaces...' );
    vsp.AutoGroupVSPAEROControlSurfaces();
    assert( vsp.GetNumControlSurfaceGroups() == 3 );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );
    control_group_settings_container_id = vsp.FindContainer( 'VSPAEROSettings', 0 );   % // auto grouping produces parm containers within VSPAEROSettings

    % //==== Set Control Surface Group Deflection Angle ====% //
    fprintf( '\tSetting control surface group deflection angles...' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % //  setup asymmetric deflection for aileron
    % // subsurfaces get added to groups with 'CSGQualities_[geom_name]_[control_surf_name]'
    % // subsurfaces gain parm name is 'Surf[surfndx]_Gain' starting from 0 to NumSymmetricCopies-1
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron_id '_0_Gain'], 'ControlSurfaceGroup_0' );
    assert_delta( vsp.SetParmValUpdate( deflection_gain_id, 0.8 ), 0.8, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron_id '_1_Gain'], 'ControlSurfaceGroup_0' );
    assert_delta( vsp.SetParmValUpdate( deflection_gain_id, 1.2 ), 1.2, TEST_TOL );
    % //  deflect aileron
    deflection_angle_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_0' );
    assert_delta( vsp.SetParmValUpdate( deflection_angle_id, 1.0 ), 1.0, TEST_TOL );
    fprintf( 'COMPLETE\n' );

    % //==== Setup export filenames ====% //
    % global m_vspfname_for_vspaerotests = 'apitest_TestVSPAero.vsp3';
    fprintf( '\tSetting export name: %s...', m_vspfname_for_vspaerotests );
    vsp.SetVSP3FileName( m_vspfname_for_vspaerotests );  % // this still needs to be done even if a call to WriteVSPFile is made
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Final vehicle update ====% //
    fprintf( '\tVehicle update...' );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Save Vehicle to File ====% //
    fprintf( '\tSaving vehicle file to: %s ...', m_vspfname_for_vspaerotests);
    vsp.WriteVSPFile( vsp.GetVSPFileName(), vsp.SET_ALL );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % Check that the file was mdae
    assert(exist(m_vspfname_for_vspaerotests)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',m_vspfname_for_vspaerotests));
    
    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroComputeGeom()
function TestVSPAeroComputeGeom_test(~)

%     global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests

    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroComputeGeom()\n' );

    % //open the file created in TestVSPAeroCreateModel
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Analysis: VSPAero Compute Geometry ====% //
    analysis_name = 'VSPAEROComputeGeometry';
    fprintf( '\t%s\n', analysis_name);

    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Change some input values
    % //    Analysis method - Vortex lattice
    analysis_method = vsp.VORTEX_LATTICE;
    vsp.SetIntAnalysisInput( analysis_name, 'AnalysisMethod', vsp.IntVector(analysis_method));
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\n\t\tExecuting VORTEX LATTICE GEOM...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    fprintf( 'COMPLETE\n\n' );

    % // Get & Display Results
    vsp.PrintResults( results_id );

    % Check that the files were mdae
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_DegenGeom.csv');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    
    % // Check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
    
end

%% void APITestSuiteVSPAERO::TestVSPAeroControlSurfaceDeflection()
function TestVSPAeroControlSurfaceDeflection_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests

    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroControlSurfaceDeflection()\n' );

    % //open the file created in TestVSPAeroCreateModel
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    fprintf( '\t\tExecuting Comp Geom...' );
    vsp.ExecAnalysis( 'VSPAEROComputeGeometry' );
    fprintf( 'COMPLETE\n\n' );

    % // Find All Control Surface IDs to be used in Gain Parm Names
    fprintf('\tFinding All Necessary Geometry IDs...\n');
    wing_id = vsp.FindGeom( 'MainWing', 0 );
    assert(~isempty(wing_id))
    aileron_id = vsp.GetSubSurf( wing_id, 0 );
    assert(~isempty(aileron_id))
    fprintf('\tCOMPLETE.\n');

    % /// ==== Test Asymmetric Deflection ==== % ///
    % // Edit Control Surface Group Angle and Contained Control Surface Gains
    fprintf('\tEditing Aileron Control Surface Deflection Gains and Angle...');
    csg_id = vsp.FindContainer( 'VSPAEROSettings', 0 );
    assert( ~isempty(csg_id) );
    deflection_angle_id = vsp.FindParm( csg_id, 'DeflectionAngle', 'ControlSurfaceGroup_0' );
    assert_delta( vsp.SetParmValUpdate( deflection_angle_id, 1.0 ), 1.0, TEST_TOL );
    surf0_gain_id = vsp.FindParm( csg_id, ['Surf_' aileron_id '_0_Gain'], 'ControlSurfaceGroup_0' );
    assert_delta( vsp.SetParmValUpdate( surf0_gain_id, 2.0 ), 2.0, TEST_TOL );
    surf1_gain_id = vsp.FindParm( csg_id, ['Surf_' aileron_id '_1_Gain'], 'ControlSurfaceGroup_0' );
    assert_delta( vsp.SetParmValUpdate( surf1_gain_id, 2.0 ), 2.0, TEST_TOL );
    fprintf('COMPLETE\n');

    % //==== Analysis: VSPAero Compute Geometry ====% //
    analysis_name = 'VSPAEROSinglePoint';
    fprintf( '\tAnalysis: %s\n', analysis_name);

    % // Set defaults
    fprintf('\tSetting Defaults...');
    vsp.SetAnalysisInputDefaults( analysis_name );
    fprintf('COMPLETE\n');

    % // Set Options
    num_wake_iter = vsp.IntVector(); num_wake_iter.push_back( 1 );
    vsp.SetIntAnalysisInput( analysis_name, 'WakeNumIter', num_wake_iter );
    
    % // Execute
    fprintf( '\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    assert(~isempty(results_id))
    fprintf( 'COMPLETE\n' );

    % // Check for within 5% of v3.13 Rolling Moment
    % // old cmx = -0.01010 % // Version Checked VSP 3.11 | VSPAERO 3.1
    % // current_cmx = -0.00882; % // Version Last Checked VSP 3.13 | VSPAERO 4.1
    current_cmx = 0.0093; % // Version Last Checked VSP 3.16.1 | VSPAERO 4.4.1
    history_id = vsp.FindLatestResultsID( 'VSPAERO_History' );
    roll_mom_tol = abs( 0.05 * cell2mat(vsp.GetDoubleResults( history_id, 'CMx')) );
    assert_delta( cell2mat(vsp.GetDoubleResults( history_id, 'CMx' )), current_cmx, roll_mom_tol );

    % /// ==== Test Symmetric Deflection ==== % ///
    % // Edit Control Surface Group Angle and Contained Control Surface Gains
    assert_delta( vsp.SetParmValUpdate( deflection_angle_id, 1.0 ), 1.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( surf0_gain_id, 1.0 ), 1.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( surf1_gain_id, -1.0 ), -1.0, TEST_TOL );

    % // Execute
    fprintf( '\n\t\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    assert(~isempty(results_id))
    fprintf( 'COMPLETE\n\n' );

    % Check that the files were mdae
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_DegenGeom.adb');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_DegenGeom.adb.cases');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_DegenGeom.fem');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_DegenGeom.history');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_DegenGeom.lod');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_DegenGeom.polar');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_DegenGeom.vspaero');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));

    % // Check for 0 rolling moment
    history_id = vsp.FindLatestResultsID( 'VSPAERO_History' );
    assert_delta( cell2mat(vsp.GetDoubleResults( history_id, 'CMx' )), 0.0, TEST_TOL );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end
    
%% void APITestSuiteVSPAERO::TestVSPAeroComputeGeomPanel()
function TestVSPAeroComputeGeomPanel_test(~)

%     global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests
    
    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroComputeGeomPanel()\n' );

    % //open the file created in TestVSPAeroCreateModel
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Analysis: VSPAero Compute Geometry ====% //
    analysis_name = 'VSPAEROComputeGeometry';
    fprintf( '\t%s\n', analysis_name);

    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Change some input values
    % //    Analysis method
    analysis_method = vsp.IntVector(); analysis_method.push_back(vsp.PANEL);
    vsp.SetIntAnalysisInput( analysis_name, 'AnalysisMethod', analysis_method );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\n\t\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    fprintf( 'COMPLETE\n\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );

    % Check that the files were mdae
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','.tkey');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','.tri');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_CompGeom.csv');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));
    fileName = strrep(m_vspfname_for_vspaerotests,'.vsp3','_CompGeom.txt');
    assert(exist(fileName)~=0,sprintf('FILE NOT CREATED\n\tfile: %s',fileName));

    % Check that the each of the geoms has a non-zero volume
    resID = vsp.FindLatestResultsID( 'Comp_Geom' );
    dataNames = vsp.GetAllDataNames(resID);
    for iData = 1:numel(dataNames)
        if strcmp(dataNames{iData},'Total_Wet_Vol')
            geomVols = cell2mat(vsp.GetDoubleResults(resID,dataNames{30}));
        end
    end
    assert(all(geomVols),'Wetted volumes are bad')
    
    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroSinglePointPanel()
function  TestVSPAeroSinglePointPanel_test(~)

%     global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests
    
    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroSinglePointPanel()\n' );

    % //open the file created in TestVSPAeroCreateModel
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Analysis: VSPAERO Single Point ====% //
    analysis_name = 'VSPAEROSinglePoint';
    fprintf( '\t%s\n', analysis_name);
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Change some input values
    % //    Analysis method
    analysis_method = vsp.PANEL;
    vsp.SetIntAnalysisInput( analysis_name, 'AnalysisMethod', vsp.IntVector(analysis_method), 0 );
    % //    Reference geometry set
    geom_set = vsp.IntVector(); geom_set.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'GeomSet', geom_set );
    % //    Reference areas, lengths
    sref = vsp.DoubleVector(); sref.push_back(10);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Sref', sref );
    bref = vsp.DoubleVector(); bref.push_back(17);
    vsp.SetDoubleAnalysisInput( analysis_name, 'bref', bref );
    cref = vsp.DoubleVector(); cref.push_back(3);
    vsp.SetDoubleAnalysisInput( analysis_name, 'cref', cref );
    ref_flag = vsp.IntVector(); ref_flag.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'RefFlag', ref_flag );
    % //    freestream parameters
    alpha = vsp.DoubleVector(); alpha.push_back(5);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Alpha', vsp.DoubleVector(alpha) );
    beta = vsp.DoubleVector(); beta.push_back(2.5);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Beta', beta );
    mach = vsp.DoubleVector(); mach.push_back(0.1);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Mach', mach );

    % // Reduce wake iteration for speed (force new setup file ensures wake iter setting is re-generated for this test)
    wakeNumIter = vsp.IntVector(); wakeNumIter.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'WakeNumIter', wakeNumIter );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\n\t\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    fprintf( 'COMPLETE\n\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );
    
    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end

%%void APITestSuiteVSPAERO::TestVSPAeroSinglePoint()
function TestVSPAeroSinglePoint_test(~)

%     global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests
    
    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroSinglePoint()\n' );

    % //open the file created in TestVSPAeroCreateModel
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Analysis: VSPAERO Single Point ====% //
    analysis_name = 'VSPAEROSinglePoint';
    fprintf( '\t%s\n', analysis_name);
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Change some input values
    % //    Analysis method
    analysis_method = vsp.VORTEX_LATTICE;
    vsp.SetIntAnalysisInput( analysis_name, 'AnalysisMethod', vsp.IntVector(analysis_method), 0 );
    % //    Reference geometry set
    geom_set = vsp.IntVector(); geom_set.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'GeomSet', geom_set );
    % //    Reference areas, lengths
    sref = vsp.DoubleVector(); sref.push_back(10);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Sref', sref );
    bref = vsp.DoubleVector(); bref.push_back(17);
    vsp.SetDoubleAnalysisInput( analysis_name, 'bref', bref );
    cref = vsp.DoubleVector(); cref.push_back(3);
    vsp.SetDoubleAnalysisInput( analysis_name, 'cref', cref );
    ref_flag = vsp.IntVector(); ref_flag.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'RefFlag', ref_flag );
    % //    freestream parameters
    alpha = vsp.DoubleVector(); alpha.push_back(5);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Alpha', alpha );
    beta = vsp.DoubleVector(); beta.push_back(2.5);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Beta', beta );
    mach = vsp.DoubleVector(); mach.push_back(0.1);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Mach', mach );

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\n\t\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    fprintf( 'COMPLETE\n\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroSinglePointStab()
function TestVSPAeroSinglePointStab_test(~)

%     global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests
    
    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroSinglePointStab()\n' );

    % //open the file created in TestVSPAeroCreateModel
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Analysis: VSPAERO Single Point  stabilityFlag = TRUE ====% //
    analysis_name = 'VSPAEROSinglePoint';
    fprintf( '\t%s\n', analysis_name);
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Change some input values
    % //    Analysis method
    analysis_method = vsp.VORTEX_LATTICE;
    vsp.SetIntAnalysisInput( analysis_name, 'AnalysisMethod', vsp.IntVector(analysis_method), 0 );
    % //    Reference geometry set
    geom_set = vsp.IntVector(); geom_set.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'GeomSet', geom_set );
    % //    Reference areas, lengths
    sref = vsp.DoubleVector(); sref.push_back(10);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Sref', sref );
    bref = vsp.DoubleVector(); bref.push_back(17);
    vsp.SetDoubleAnalysisInput( analysis_name, 'bref', bref );
    cref = vsp.DoubleVector(); cref.push_back(3);
    vsp.SetDoubleAnalysisInput( analysis_name, 'cref', cref );
    ref_flag = vsp.IntVector(); ref_flag.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'RefFlag', ref_flag );
    % //    freestream parameters
    alpha = vsp.DoubleVector(); alpha.push_back(4);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Alpha', alpha );
    beta = vsp.DoubleVector(); beta.push_back(-3.0);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Beta', beta );
    mach = vsp.DoubleVector(); mach.push_back(0.4);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Mach', mach );
    % //     Case Setup
    wakeNumIter = vsp.IntVector(); wakeNumIter.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'WakeNumIter', wakeNumIter );
    stabilityCalcFlag = vsp.IntVector(); stabilityCalcFlag.push_back(vsp.STABILITY_DEFAULT);
    vsp.SetIntAnalysisInput( analysis_name, 'UnsteadyType', stabilityCalcFlag );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\n\t\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    fprintf( 'COMPLETE\n\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
    
end

%% void APITestSuiteVSPAERO::TestVSPAeroSinglePointUnsteady()
function TestVSPAeroSinglePointUnsteady_test(~)

%     global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests
    
    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroSinglePointUnsteady()\n' );

    % //open the file created in TestVSPAeroCreateModel
    fprintf('\tReading in file...');
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf('COMPLETE\n');

    % //==== Analysis: VSPAERO Single Point  stabilityFlag = TRUE ====% //
    analysis_name = 'VSPAEROSinglePoint';
    fprintf( '\tAnalysis Type: %s\n', analysis_name);
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    fprintf('\tChanging Analysis Inputs...');
    % // Change some input values
    % //    Analysis method
    analysis_method = vsp.IntVector(); analysis_method.push_back(vsp.VORTEX_LATTICE);
    vsp.SetIntAnalysisInput( analysis_name, 'AnalysisMethod', analysis_method, 0 );
    % //    Reference geometry set
    geom_set = vsp.IntVector(); geom_set.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'GeomSet', geom_set );
    % //    Reference areas, lengths
    sref = vsp.DoubleVector(); sref.push_back(10);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Sref', sref );
    bref = vsp.DoubleVector(); bref.push_back(17);
    vsp.SetDoubleAnalysisInput( analysis_name, 'bref', bref );
    cref = vsp.DoubleVector(); cref.push_back(3);
    vsp.SetDoubleAnalysisInput( analysis_name, 'cref', cref );
    ref_flag = vsp.IntVector(); ref_flag.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'RefFlag', ref_flag );
    % //    freestream parameters
    alpha = vsp.DoubleVector(); alpha.push_back(4);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Alpha', alpha );
    beta = vsp.DoubleVector(); beta.push_back(-3.0);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Beta', beta );
    mach = vsp.DoubleVector(); mach.push_back(0.4);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Mach', mach );
    % //     Case Setup
    wakeNumIter = vsp.IntVector(); wakeNumIter.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'WakeNumIter', wakeNumIter );
    
    stabilityCalcType = vsp.STABILITY_P_ANALYSIS;
    vsp.SetIntAnalysisInput( analysis_name, 'UnsteadyType', vsp.IntVector(stabilityCalcType) );
    jacobiPrecondition = vsp.PRECON_JACOBI;
    vsp.SetIntAnalysisInput( analysis_name, 'Precondition', vsp.IntVector(jacobiPrecondition) );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf('COMPLETE\n');

    % // Execute
    fprintf( '\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    assert(~isempty(results_id))
    fprintf( 'COMPLETE\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroSweep()
function TestVSPAeroSweep_test(~)

%     global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests
    
    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroSweep()\n' );

    % //open the file created in TestVSPAeroCreateModel
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Analysis: VSPAERO Sweep ====% //
    analysis_name = 'VSPAEROSweep';
    fprintf( '\t%s\n', analysis_name);
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Change some input values
    % //    Analysis method
    analysis_method = vsp.IntVector(); analysis_method.push_back(vsp.VORTEX_LATTICE);
    vsp.SetIntAnalysisInput( analysis_name, 'AnalysisMethod', vsp.IntVector(analysis_method), 0 );
    % //    Reference geometry set
    geom_set = vsp.IntVector(); geom_set.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'GeomSet', geom_set );
    % //    Reference areas, lengths
    sref = vsp.DoubleVector(); sref.push_back(10);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Sref', sref );
    bref = vsp.DoubleVector(); bref.push_back(17);
    vsp.SetDoubleAnalysisInput( analysis_name, 'bref', bref );
    cref = vsp.DoubleVector(); cref.push_back(3);
    vsp.SetDoubleAnalysisInput( analysis_name, 'cref', cref );
    ref_flag = vsp.IntVector(); ref_flag.push_back(vsp.MANUAL_REF);
    vsp.SetIntAnalysisInput( analysis_name, 'RefFlag', ref_flag );
    % //    freestream parameters
    % //        Alpha
    alpha_start = vsp.DoubleVector(); alpha_start.push_back(1);
    vsp.SetDoubleAnalysisInput( analysis_name, 'AlphaStart', alpha_start );
    alpha_end = vsp.DoubleVector(); alpha_end.push_back(10);
    vsp.SetDoubleAnalysisInput( analysis_name, 'AlphaEnd', alpha_end );
    alpha_npts = vsp.IntVector(); alpha_npts.push_back(4);
    vsp.SetIntAnalysisInput( analysis_name, 'AlphaNpts', alpha_npts );
    % //        Beta
    beta_start = vsp.DoubleVector(); beta_start.push_back(0);
    vsp.SetDoubleAnalysisInput( analysis_name, 'BetaStart', beta_start );
    beta_end = vsp.DoubleVector(); beta_end.push_back(5);
    vsp.SetDoubleAnalysisInput( analysis_name, 'BetaEnd', beta_end );
    beta_npts = vsp.IntVector(); beta_npts.push_back(4);
    vsp.SetIntAnalysisInput( analysis_name, 'BetaNpts', beta_npts );
    % //        Mach
    mach_start = vsp.DoubleVector(); mach_start.push_back(0.05);
    vsp.SetDoubleAnalysisInput( analysis_name, 'MachStart', mach_start );
    mach_end = vsp.DoubleVector(); mach_end.push_back(0.15);
    vsp.SetDoubleAnalysisInput( analysis_name, 'MachEnd', mach_end );
    mach_npts = vsp.IntVector(); mach_npts.push_back(2);
    vsp.SetIntAnalysisInput( analysis_name, 'MachNpts', mach_npts );
    vsp.Update();
    % //     Case Setup
    wakeNumIter = vsp.IntVector(); wakeNumIter.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'WakeNumIter', wakeNumIter );
    batch_mode_flag = vsp.IntVector(); batch_mode_flag.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'BatchModeFlag', batch_mode_flag );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\n\t\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    fprintf( 'COMPLETE\n\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );

end

%% void APITestSuiteVSPAERO::TestVSPAeroSweepBatch()
function  TestVSPAeroSweepBatch_test(~)

%     global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests
    
    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroSweepBatch()\n' );

    % //open the file created in TestVSPAeroCreateModel
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Analysis: VSPAERO Sweep ====% //
    analysis_name = 'VSPAEROSweep';
    fprintf( '\t%s\n', analysis_name);
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Change some input values
    % //    Analysis method
    analysis_method = vsp.IntVector(); analysis_method.push_back(vsp.VORTEX_LATTICE);
    vsp.SetIntAnalysisInput( analysis_name, 'AnalysisMethod', vsp.IntVector(analysis_method), 0 );
    % //    Reference geometry set
    geom_set = vsp.IntVector(); geom_set.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'GeomSet', geom_set );
    % //    Reference areas, lengths
    sref = vsp.DoubleVector(); sref.push_back(10);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Sref', sref );
    bref = vsp.DoubleVector(); bref.push_back(17);
    vsp.SetDoubleAnalysisInput( analysis_name, 'bref', bref );
    cref = vsp.DoubleVector(); cref.push_back(3);
    vsp.SetDoubleAnalysisInput( analysis_name, 'cref', cref );
    ref_flag = vsp.IntVector(); ref_flag.push_back(vsp.MANUAL_REF);
    vsp.SetIntAnalysisInput( analysis_name, 'RefFlag', ref_flag );
    % //    freestream parameters
    % //        Alpha
    alpha_start = vsp.DoubleVector(); alpha_start.push_back(1);
    vsp.SetDoubleAnalysisInput( analysis_name, 'AlphaStart', alpha_start );
    alpha_end = vsp.DoubleVector(); alpha_end.push_back(10);
    vsp.SetDoubleAnalysisInput( analysis_name, 'AlphaEnd', alpha_end );
    alpha_npts = vsp.IntVector(); alpha_npts.push_back(4);
    vsp.SetIntAnalysisInput( analysis_name, 'AlphaNpts', alpha_npts );
    % //        Beta
    beta_start = vsp.DoubleVector(); beta_start.push_back(0);
    vsp.SetDoubleAnalysisInput( analysis_name, 'BetaStart', beta_start );
    beta_end = vsp.DoubleVector(); beta_end.push_back(5);
    vsp.SetDoubleAnalysisInput( analysis_name, 'BetaEnd', beta_end );
    beta_npts = vsp.IntVector(); beta_npts.push_back(4);
    vsp.SetIntAnalysisInput( analysis_name, 'BetaNpts', beta_npts );
    % //        Mach
    mach_start = vsp.DoubleVector(); mach_start.push_back(0.05);
    vsp.SetDoubleAnalysisInput( analysis_name, 'MachStart', mach_start );
    mach_end = vsp.DoubleVector(); mach_end.push_back(0.15);
    vsp.SetDoubleAnalysisInput( analysis_name, 'MachEnd', mach_end );
    mach_npts = vsp.IntVector(); mach_npts.push_back(2);
    vsp.SetIntAnalysisInput( analysis_name, 'MachNpts', mach_npts );
    vsp.Update();
    % //     Case Setup
    wakeNumIter = vsp.IntVector(); wakeNumIter.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'WakeNumIter', wakeNumIter );
    batch_mode_flag = vsp.IntVector(); batch_mode_flag.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'BatchModeFlag', batch_mode_flag );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\n\t\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    fprintf( 'COMPLETE\n\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroSharpTrailingEdge()
function TestVSPAeroSharpTrailingEdge_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
%     global m_vspfname_for_vspaerotests
    
    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroSharpTrailingEdge()\n' );

    % //==== Analysis: VSPAero Single Point ====% //
    analysis_name = 'VSPAEROSinglePoint';
    fprintf( '\t%s\n', analysis_name);

    % //==== Create Symmetric Airfoil Wing Geometry ====% //
    fprintf( '\tGenerating Geometries\n' );

    wing_id = vsp.AddGeom( 'WING' );
    assert( ~isempty(wing_id) );

    % // Get Wing Section IDs
    wingxsurf_id = vsp.GetXSecSurf( wing_id, 0 );
    xsec_id0 = vsp.GetXSec( wingxsurf_id, 0 );
    assert(~isempty(xsec_id0))
    xsec_id1 = vsp.GetXSec( wingxsurf_id, 1 );
    assert(~isempty(xsec_id1))

    % //  Set Root and Tip Chord to 3 and Area to 25:
   vsp.SetDriverGroup( wing_id, 1, vsp.AREA_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER );
    xtipchord_id1 = vsp.GetXSecParm( xsec_id1, 'Tip_Chord' );
    assert_delta( vsp.SetParmVal( xtipchord_id1, 3 ), 3, TEST_TOL );
    xrootchord_id1 = vsp.GetXSecParm( xsec_id1, 'Root_Chord' );
    assert_delta( vsp.SetParmVal( xrootchord_id1, 3 ), 3, TEST_TOL );
    assert_delta( vsp.SetParmVal( wing_id, 'Area', 'XSec_1', 25.0 ), 25.0, TEST_TOL );

    % //  Set Sweep to 0:
    xsweep_id1 = vsp.GetXSecParm( xsec_id1, 'Sweep' );
    assert_delta( vsp.SetParmVal( xsweep_id1, 0.0 ), 0.0, TEST_TOL );

    % //  Increase W Tesselation:
    assert_delta( vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', 69 ), calcTessWCheckVal( 69 ), TEST_TOL );

    % //  Increase U Tesselation
    xutess_id1 = vsp.GetXSecParm( xsec_id1, 'SectTess_U' );
    assert_delta( vsp.SetParmVal( xutess_id1, 16 ), 16, TEST_TOL );
    xrtCMxuster_id1 = vsp.GetXSecParm( xsec_id1, 'InCluster' );
    assert_delta( vsp.SetParmVal( xrtCMxuster_id1, 0.1 ), 0.1, TEST_TOL );
    xtipCMxuster_id1 = vsp.GetXSecParm( xsec_id1, 'OutCluster' );
    assert_delta( vsp.SetParmVal( xtipCMxuster_id1, 0.1 ), 0.1, TEST_TOL );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Setup export filenames ====% //
    % // Execution of one of these methods is required to properly set the export filenames for creation of vspaero input files and execution commands
    fname_sharptrailingedge = 'apitest_VSPAeroSharpTrailingEdge.vsp3';
    fprintf( '\tSetting export name: %s\n', fname_sharptrailingedge );
    vsp.SetVSP3FileName( fname_sharptrailingedge );  % // this still needs to be done even if a call to WriteVSPFile is made
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Save Vehicle to File ====% //
    fprintf( '\tSaving vehicle file to: %s ...\n', fname_sharptrailingedge );
    vsp.WriteVSPFile( vsp.GetVSPFileName(), vsp.SET_ALL );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====% //
    compgeom_name = 'VSPAEROComputeGeometry';
    fprintf( '\t%s\n', compgeom_name);

    % // Set defaults
    vsp.SetAnalysisInputDefaults( compgeom_name );

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( compgeom_name );

    % // Execute
    fprintf( '\tExecuting...\n' );
    compgeom_resid = vsp.ExecAnalysis( compgeom_name );
    fprintf( 'COMPLETE\n' );

    % // Get & Display Results
    vsp.PrintResults( compgeom_resid );

    % //==== Analysis: VSPAero Single Point ====% //
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Reference geometry set
    geom_set = vsp.IntVector(); geom_set.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'GeomSet', geom_set );
    ref_flag = vsp.IntVector(); ref_flag.push_back(vsp.COMPONENT_REF);
    vsp.SetIntAnalysisInput( analysis_name, 'RefFlag', ref_flag );
    wid = vsp.StringVector(); wid.push_back(char(vsp.FindGeomsWithName( 'WingGeom' )));
    vsp.SetStringAnalysisInput( analysis_name, 'WingID', wid);

    % // Freestream Parameters
    alpha = vsp.DoubleVector(); alpha.push_back(0.0);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Alpha', alpha, 0 );
    mach = vsp.DoubleVector(); mach.push_back(0.1);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Mach', mach, 0 );

    % // Reduce wake iteration for speed (force new setup file ensures wake iter setting is re-generated for this test)
    wakeNumIter = vsp.IntVector(); wakeNumIter.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'WakeNumIter', wakeNumIter );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\tExecuting...\n' );
    results_id = vsp.ExecAnalysis( analysis_name  );
    fprintf( 'COMPLETE\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );
    res_id = vsp.GetStringResults( results_id, 'ResultsVec' , 0 );

    CL = cell2mat(vsp.GetDoubleResults( res_id{1}, 'CL', 0 ));
    fprintf( '   CL: ' );
    for i = 1:numel(CL)
        assert_delta( CL(i), 0.0, TEST_TOL );
        fprintf( '%7.3f', CL(i) );
    end
    fprintf( '\n' );
    CMx = cell2mat(vsp.GetDoubleResults( res_id{1}, 'CMx', 0 ));
    fprintf( '   CMx: ' );
    for i=1:numel(CMx)
        assert_delta( CMx(i), 0.0, TEST_TOL );
        fprintf( '%7.3f', CMx(i) );
    end
    fprintf( '\n' );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroBluntTrailingEdge()
function TestVSPAeroBluntTrailingEdge_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
%     global m_vspfname_for_vspaerotests

    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroBluntTrailingEdge()\n' );

    % //==== Analysis: VSPAero Single Point ====% //
    analysis_name = 'VSPAEROSinglePoint';
    fprintf( '\t%s\n', analysis_name);

    % //==== Create Symmetric Airfoil Wing Geometry ====% //
    fprintf( '\tGenerating Geometries\n' );

    wing_id = vsp.AddGeom( 'WING' );
    assert( ~isempty(wing_id) );

    % // Get Wing Section IDs
    wingxsurf_id = vsp.GetXSecSurf( wing_id, 0 );
    xsec_id0 = vsp.GetXSec( wingxsurf_id, 0 );
    assert(~isempty(xsec_id0))
    xsec_id1 = vsp.GetXSec( wingxsurf_id, 1 );
    assert(~isempty(xsec_id1))

    % //  Set Root and Tip Chord to 3 and Area to 25:
   vsp.SetDriverGroup( wing_id, 1, vsp.AREA_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER );
    xtipchord_id1 = vsp.GetXSecParm( xsec_id1, 'Tip_Chord' );
    assert_delta( vsp.SetParmVal( xtipchord_id1, 3 ), 3, TEST_TOL );
    xrootchord_id1 = vsp.GetXSecParm( xsec_id1, 'Root_Chord' );
    assert_delta( vsp.SetParmVal( xrootchord_id1, 3 ), 3, TEST_TOL );
    assert_delta( vsp.SetParmVal( wing_id, 'Area', 'XSec_1', 25.0 ), 25.0, TEST_TOL );

    % //  Set Sweep to 0:
    xsweep_id1 = vsp.GetXSecParm( xsec_id1, 'Sweep' );
    assert_delta( vsp.SetParmVal( xsweep_id1, 0.0 ), 0.0, TEST_TOL );

    % //  Set Trailing Edge to Blunt:
    xtrimtype_id0 = vsp.GetXSecParm( xsec_id0, 'TE_Trim_Type' );
    assert_delta( vsp.SetParmVal( xtrimtype_id0, 1 ), 1, TEST_TOL );
    xtrimval_id0 = vsp.GetXSecParm( xsec_id0, 'TE_Trim_X' );
    assert_delta( vsp.SetParmVal( xtrimval_id0, 0.5 ), 0.5, TEST_TOL );
    xtrimtype_id1 = vsp.GetXSecParm( xsec_id1, 'TE_Trim_Type' );
    assert_delta( vsp.SetParmVal( xtrimtype_id1, 1 ), 1, TEST_TOL );
    xtrimval_id1 = vsp.GetXSecParm( xsec_id1, 'TE_Trim_X' );
    assert_delta( vsp.SetParmVal( xtrimval_id1, 0.5 ), 0.5, TEST_TOL );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //  Increase W Tesselation:
    assert_delta( vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', 69 ), calcTessWCheckVal( 69 ), TEST_TOL );

    % //  Increase U Tesselation
    xutess_id1 = vsp.GetXSecParm( xsec_id1, 'SectTess_U' );
    assert_delta( vsp.SetParmVal( xutess_id1, 16 ), 16, TEST_TOL );
    xrtCMxuster_id1 = vsp.GetXSecParm( xsec_id1, 'InCluster' );
    assert_delta( vsp.SetParmVal( xrtCMxuster_id1, 0.1 ), 0.1, TEST_TOL );
    xtipCMxuster_id1 = vsp.GetXSecParm( xsec_id1, 'OutCluster' );
    assert_delta( vsp.SetParmVal( xtipCMxuster_id1, 0.1 ), 0.1, TEST_TOL );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Setup export filenames ====% //
    % // Execution of one of these methods is required to properly set the export filenames for creation of vspaero input files and execution commands
    fname_blunttrailingedge = 'apitest_VSPAeroBluntTrailingEdge.vsp3';
    fprintf( '\tSetting export name: %s\n', fname_blunttrailingedge );
    vsp.SetVSP3FileName( fname_blunttrailingedge );  % // this still needs to be done even if a call to WriteVSPFile is made
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Save Vehicle to File ====% //
    fprintf( '\tSaving vehicle file to: %s ...\n', fname_blunttrailingedge );
    vsp.WriteVSPFile( vsp.GetVSPFileName(), vsp.SET_ALL );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====% //
    compgeom_name = 'VSPAEROComputeGeometry';
    fprintf( '\t%s\n', compgeom_name);

    % // Set defaults
    vsp.SetAnalysisInputDefaults( compgeom_name );

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( compgeom_name );

    % // Execute
    fprintf( '\tExecuting...\n' );
    compgeom_resid = vsp.ExecAnalysis( compgeom_name );
    fprintf( 'COMPLETE\n' );

    % // Get & Display Results
    vsp.PrintResults( compgeom_resid );

    % //==== Analysis: VSPAero Single Point ====% //
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Reference geometry set
    geom_set = vsp.IntVector(); geom_set.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'GeomSet', geom_set, 0 );
    ref_flag = vsp.IntVector(); ref_flag.push_back(vsp.COMPONENT_REF);
    vsp.SetIntAnalysisInput( analysis_name, 'RefFlag', ref_flag, 0 );
    wid = vsp.StringVector(); wid.push_back(char(vsp.FindGeomsWithName( 'WingGeom' )));
    vsp.SetStringAnalysisInput( analysis_name, 'WingID', wid, 0 );

    % // Freestream Parameters
    alpha = vsp.DoubleVector(); alpha.push_back(0.0);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Alpha', alpha, 0 );
    mach = vsp.DoubleVector(); mach.push_back(0.1);
    vsp.SetDoubleAnalysisInput( analysis_name, 'Mach', mach, 0 );

    % // Reduce wake iteration for speed (force new setup file ensures wake iter setting is re-generated for this test)
    wakeNumIter = vsp.IntVector(); wakeNumIter.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'WakeNumIter', vsp.IntVector(wakeNumIter) );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\tExecuting...\n' );
    results_id = vsp.ExecAnalysis( analysis_name  );
    fprintf( 'COMPLETE\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );
    res_id = vsp.GetStringResults( results_id, 'ResultsVec', 0 );

    CL = cell2mat(vsp.GetDoubleResults( res_id{1}, 'CL', 0 ));
    fprintf( '   CL: ' );
    for i=1:numel(CL)
        assert_delta( CL(i), 0.0, TEST_TOL );
        fprintf( '%7.3f', CL(i) );
    end
    fprintf( '\n' );
    CMx = cell2mat(vsp.GetDoubleResults( res_id{1}, 'CMx', 0 ));
    fprintf( '   CMx: ' );
    for i=1:numel(CMx)
        assert_delta( CMx(i), 0.0, TEST_TOL );
        fprintf( '%7.3f', CMx(i) );
    end
    fprintf( '\n' );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroSupersonicDeltaWing()
function TestVSPAeroSupersonicDeltaWing_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
%     global m_vspfname_for_vspaerotests

    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroSupersonicDeltaWing()\n' );

    % //==== Analysis: VSPAero Sweep ====% //
    analysis_name = 'VSPAEROSweep';
    fprintf( '\t%s\n', analysis_name);

    % //==== Create Delta Wing Geometry ====% //
    fprintf( '\tGenerating Geometry\n' );

    wing_id = vsp.AddGeom( 'WING' );
    assert( ~isempty(wing_id) );

    % // Get Wing Section IDs
    wingxsurf_id = vsp.GetXSecSurf( wing_id, 0 );
    xsec_id0 = vsp.GetXSec( wingxsurf_id, 0 );
    assert(~isempty(xsec_id0))
    xsec_id1 = vsp.GetXSec( wingxsurf_id, 1 );
    assert(~isempty(xsec_id1))

    % //  Set Root and Tip Chord to 3 and Area to 25:
    vsp.SetDriverGroup( wing_id, 1, double(vsp.SPAN_WSECT_DRIVER), double(vsp.ROOTC_WSECT_DRIVER), double(vsp.TIPC_WSECT_DRIVER) );
    xtipchord_id1 = vsp.GetXSecParm( xsec_id1, 'Tip_Chord' );
    assert_delta( vsp.SetParmVal( xtipchord_id1, 0.01 ), 0.01, TEST_TOL );
    xrootchord_id1 = vsp.GetXSecParm( xsec_id1, 'Root_Chord' );
    assert_delta( vsp.SetParmVal( xrootchord_id1, 8 ), 8, TEST_TOL );
    assert_delta( vsp.SetParmVal( wing_id, 'Span', 'XSec_1', 8 ), 8, TEST_TOL );

    % //  Set Sweep to 45:
    xsweep_id1 = vsp.GetXSecParm( xsec_id1, 'Sweep' );
    assert_delta( vsp.SetParmVal( xsweep_id1, 45 ), 45, TEST_TOL );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //  Increase W Tesselation:
    assert_delta( vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', 69 ), calcTessWCheckVal( 69 ), TEST_TOL );
    assert_delta( vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 0.1 ), 0.1, TEST_TOL );

    % //  Increase U Tesselation
    xutess_id1 = vsp.GetXSecParm( xsec_id1, 'SectTess_U' );
    assert_delta( vsp.SetParmVal( xutess_id1, 16 ), 16, TEST_TOL );
    xrtCMxuster_id1 = vsp.GetXSecParm( xsec_id1, 'InCluster' );
    assert_delta( vsp.SetParmVal( xrtCMxuster_id1, 0.1 ), 0.1, TEST_TOL );
    xtipCMxuster_id1 = vsp.GetXSecParm( xsec_id1, 'OutCluster' );
    assert_delta( vsp.SetParmVal( xtipCMxuster_id1, 0.1 ), 0.1, TEST_TOL );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Setup export filenames ====% //
    % // Execution of one of these methods is required to properly set the export filenames for creation of vspaero input files and execution commands
    fname_deltawing = 'apitest_VSPAeroSupersonicDeltaWing.vsp3';
    fprintf( '\tSetting export name: %s\n', fname_deltawing );
    vsp.SetVSP3FileName( fname_deltawing );  % // this still needs to be done even if a call to WriteVSPFile is made
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Save Vehicle to File ====% //
    fprintf( '\tSaving vehicle file to: %s ...\n', fname_deltawing );
    vsp.WriteVSPFile( vsp.GetVSPFileName(), vsp.SET_ALL );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====% //
    compgeom_name = 'VSPAEROComputeGeometry';
    fprintf( '\t%s\n', compgeom_name);

    % // Set defaults
    vsp.SetAnalysisInputDefaults( compgeom_name );

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( compgeom_name );

    % // Execute
    fprintf( '\tExecuting...\n' );
    compgeom_resid = vsp.ExecAnalysis( compgeom_name );
    fprintf( 'COMPLETE\n' );

    % // Get & Display Results
    vsp.PrintResults( compgeom_resid );

    % //==== Analysis: VSPAero Single Point ====% //
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Reference geometry set
    geom_set = vsp.IntVector(); geom_set.push_back(0);
    vsp.SetIntAnalysisInput( analysis_name, 'GeomSet', geom_set, 0 );
    ref_flag = vsp.IntVector(); ref_flag.push_back(vsp.COMPONENT_REF);
    vsp.SetIntAnalysisInput( analysis_name, 'RefFlag', ref_flag, 0 );
    wid = vsp.StringVector(); wid.push_back(char(vsp.FindGeomsWithName( 'WingGeom' )));
    vsp.SetStringAnalysisInput( analysis_name, 'WingID', wid, 0 );

    % // Freestream Parameters
    alpha_start = vsp.DoubleVector(); alpha_start.push_back(1);
    vsp.SetDoubleAnalysisInput( analysis_name, 'AlphaStart', alpha_start, 0 );
    alpha_npts = vsp.IntVector(); alpha_npts.push_back(1);
    vsp.SetIntAnalysisInput( analysis_name, 'AlphaNpts', alpha_npts, 0 );
    mach_start = vsp.DoubleVector(); mach_start.push_back(1.2);
    vsp.SetDoubleAnalysisInput( analysis_name, 'MachStart', mach_start, 0 );
    mach_npts = vsp.IntVector(); mach_npts.push_back(8);
    vsp.SetIntAnalysisInput( analysis_name, 'MachNpts', mach_npts, 0 );
    mach_end = vsp.DoubleVector; mach_end.push_back(4.4);
    vsp.SetDoubleAnalysisInput( analysis_name, 'MachEnd', mach_end, 0 );

    % // Reduce wake iteration for speed (force new setup file ensures wake iter setting is re-generated for this test)
    wakeNumIter = vsp.IntVector(); wakeNumIter.push_back(3);
    vsp.SetIntAnalysisInput( analysis_name, 'WakeNumIter', wakeNumIter );

    % // Set Batch Mode
    batch_mode_flag = vsp.IntVector(); batch_mode_flag.push_back(1);
    vsp.SetIntAnalysisInput( analysis_name, 'BatchModeFlag', batch_mode_flag, 0 );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\tExecuting...\n' );
    results_id = vsp.ExecAnalysis( analysis_name  );
    fprintf( 'COMPLETE\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroCreateFunctionalityModel()
function TestVSPAeroCreateFunctionalityModel_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
%     global m_vspfname_for_vspaerotests
    global m_vspfname_for_vspaerofunctionalitytests

    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroCreateFunctionalityModel()\n' );

    % //==== Add Wing Geom and set some parameters =====% //
    fprintf( '\tAdding WING (MainWing)...' );
    wing_id = vsp.AddGeom( 'WING' );
    vsp.SetGeomName( wing_id, 'MainWing' );
    assert( ~isempty(wing_id) );

    % // Add aileron control surfaces
    aileron1_id = vsp.AddSubSurf( wing_id, vsp.SS_CONTROL );
    vsp.SetSubSurfName( wing_id, aileron1_id, 'Inner Aileron' );
    assert( ~isempty(aileron1_id) );
    tessstart = vsp.GetParm( wing_id, 'UStart', 'SS_Control_1');
    assert_delta( vsp.SetParmValUpdate( tessstart, 0.35 ), 0.35, TEST_TOL );
    tessend = vsp.GetParm( wing_id, 'UEnd', 'SS_Control_1' );
    assert_delta( vsp.SetParmValUpdate( tessend, 0.45 ), 0.45, TEST_TOL );

    aileron2_id = vsp.AddSubSurf( wing_id, vsp.SS_CONTROL );
    vsp.SetSubSurfName( wing_id, aileron2_id, 'Outer Aileron' );
    assert( ~isempty(aileron2_id) );
    tessstart = vsp.GetParm( wing_id, 'UStart', 'SS_Control_2');
    assert_delta( vsp.SetParmValUpdate( tessstart, 0.47 ), 0.47, TEST_TOL );
    tessend = vsp.GetParm( wing_id, 'UEnd', 'SS_Control_2');
    assert_delta( vsp.SetParmValUpdate( tessend, 0.6 ), 0.6, TEST_TOL );

    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Add Horizontal tail and set some parameters =====% //
    fprintf( '\tAdding WING (Horiz)...' );
    horiz_id = vsp.AddGeom( 'WING' );
    vsp.SetGeomName( horiz_id, 'Tail' );
    assert( ~isempty(horiz_id) );
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'TotalArea', 'WingGeom', 10.0 ), 10.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( horiz_id, 'X_Rel_Location', 'XForm', 8.5 ), 8.5, TEST_TOL );

    % // Add elevator control surface
    elevator_id = vsp.AddSubSurf( horiz_id, vsp.SS_CONTROL );
    vsp.SetSubSurfName( horiz_id, elevator_id, 'Elevator' );
    assert( ~isempty(elevator_id) );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % /// TODO
    % //==== Add Inner Disk and set some parameters ====% //
    % //==== Add Outer Disk and set some parameters ====% //

    % //==== Auto Group Control Surfaces ====% //
    fprintf( '\tGrouping Control Surfaces...' );
    vsp.AutoGroupVSPAEROControlSurfaces();
    assert( vsp.GetNumControlSurfaceGroups() == 3 );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );
    control_group_settings_container_id = vsp.FindContainer( 'VSPAEROSettings', 0 );   % // auto grouping produces parm containers within VSPAEROSettings

    % //==== Set Control Surface Group Deflection Angle ====% //
    fprintf( '\tSetting control surface group deflection angles...' );
    % // subsurfaces get added to groups with 'ControlSurfaceGroup_[index]'
    % // subsurfaces gain parm name is 'Surf_[surfid]_[surfndx]_Gain' starting from 0 to NumSymmetricCopies-1
    
    gain0 = 0.1;
    angle0 = 1.0;
    gain1 = 0.2;
    angle1 = 2.0;
    gain2 = 0.3;
    angle2 = 3.0;

    % // ControlSurfaceGroup_0
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron1_id '_0_Gain'], 'ControlSurfaceGroup_0' );
    assert_delta( vsp.SetParmValUpdate( deflection_gain_id, gain0 ), gain0, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron1_id '_1_Gain'], 'ControlSurfaceGroup_0' );
    assert_delta( vsp.SetParmValUpdate( deflection_gain_id, -gain0 ), -gain0, TEST_TOL );
    % //  deflect inside aileron
    deflection_angle_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_0' );
    assert_delta( vsp.SetParmValUpdate( deflection_angle_id, angle0 ), angle0, TEST_TOL );

    % // ControlSurfaceGroup_1
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron2_id '_0_Gain'], 'ControlSurfaceGroup_1' );
    assert_delta( vsp.SetParmValUpdate( deflection_gain_id, gain1 ), gain1, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron2_id '_1_Gain'], 'ControlSurfaceGroup_1' );
    assert_delta( vsp.SetParmValUpdate( deflection_gain_id, -gain1 ), -gain1, TEST_TOL );
    % //  deflect inside aileron
    deflection_angle_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_1' );
    assert_delta( vsp.SetParmValUpdate( deflection_angle_id, angle1 ), angle1, TEST_TOL );

    % // ControlSurfaceGroup_2
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' elevator_id '_0_Gain'], 'ControlSurfaceGroup_2' );
    assert_delta( vsp.SetParmValUpdate( deflection_gain_id, gain2 ), gain2, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' elevator_id '_1_Gain'], 'ControlSurfaceGroup_2' );
    assert_delta( vsp.SetParmValUpdate( deflection_gain_id, -gain2 ), -gain2, TEST_TOL );
    % //  deflect inside aileron
    deflection_angle_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_2' );
    assert_delta( vsp.SetParmValUpdate( deflection_angle_id, angle2 ), angle2, TEST_TOL );

    fprintf('COMPLETE. \n');

    % //==== Setup export filenames ====% //
    fprintf( '\tSetting export name: %s...', m_vspfname_for_vspaerofunctionalitytests);
    vsp.SetVSP3FileName( m_vspfname_for_vspaerofunctionalitytests );  % // this still needs to be done even if a call to WriteVSPFile is made
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Final vehicle update ====% //
    fprintf( '\tVehicle update...' );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Save Vehicle to File ====% //
    fprintf( '\tSaving vehicle file to: %s ...', m_vspfname_for_vspaerofunctionalitytests);
    vsp.WriteVSPFile( vsp.GetVSPFileName(), vsp.SET_ALL );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    fprintf( 'COMPLETE\n\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroReadControlSurfaceGroupsFromFile()
function TestVSPAeroReadControlSurfaceGroupsFromFile_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
%     global m_vspfname_for_vspaerotests
    global m_vspfname_for_vspaerofunctionalitytests;

    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroReadControlSurfaceGroupsFromFile()\n' );

    % //open the file created in TestVSPAeroCreateModel
    fprintf('\tLoading File...');
    vsp.ReadVSPFile( m_vspfname_for_vspaerofunctionalitytests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Find Control Surface Group Parm Containers ====% //
    control_group_settings_container_id = vsp.FindContainer( 'VSPAEROSettings', 0 );   % // auto grouping produces parm containers within VSPAEROSettings

    % // Find All Control Surface IDs to be used in Gain Parm Names
    fprintf('\tFinding All Geometry IDs...\n');
    % find wing
    wing_id = vsp.FindGeom( 'MainWing', 0 );
    assert(~isempty(wing_id))
    % find control surfaces
    aileron1_id = vsp.GetSubSurf( wing_id, 0 );
    assert(~isempty(aileron1_id))
    aileron2_id = vsp.GetSubSurf( wing_id, 1 );
    assert(~isempty(aileron2_id))
    
    % find tail
    horiz_id = vsp.FindGeom( 'Tail', 0 );
    assert(~isempty(horiz_id))
    % find control surfaces
    elevator_id = vsp.GetSubSurf( horiz_id, 0 );
    assert(~isempty(elevator_id))

    gain0 = 0.1;
    angle0 = 1.0;
    gain1 = 0.2;
    angle1 = 2.0;
    gain2 = 0.3;
    angle2 = 3.0;

    % // ControlSurfaceGroup_0
    fprintf('\tChecking All Parms for ControlSurfaceGroup_0...');
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron1_id '_0_Gain'], 'ControlSurfaceGroup_0');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), gain0, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron1_id '_1_Gain'], 'ControlSurfaceGroup_0');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), -gain0, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_0');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), angle0, TEST_TOL );
    fprintf('COMPLETE.\n');

    % // ControlSurfaceGroup_1
    fprintf('\tChecking All Parms for ControlSurfaceGroup_1...');
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron2_id '_0_Gain'], 'ControlSurfaceGroup_1');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), gain1, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron2_id '_1_Gain'], 'ControlSurfaceGroup_1');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), -gain1, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_1');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), angle1, TEST_TOL );
    fprintf('COMPLETE.\n');

    % // ControlSurfaceGroup_2
    fprintf('\tChecking All Parms for ControlSurfaceGroup_2...');
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' elevator_id '_0_Gain'], 'ControlSurfaceGroup_2');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), gain2, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' elevator_id '_1_Gain'], 'ControlSurfaceGroup_2');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), -gain2, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_2');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), angle2, TEST_TOL );
    fprintf('COMPLETE.\n');

    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    fprintf( 'COMPLETE\n\n' );
end

%% void APITestSuiteVSPAERO::TestVSPAeroReadRotorDisksFromFile()
function TestVSPAeroReadRotorDisksFromFile_test(~)
    % // TODO - Need to Figure Out how to put Disks into basic files
end

%% void APITestSuiteVSPAERO::TestVSPAeroParmContainersAccessibleAfterSave()
function TestVSPAeroParmContainersAccessibleAfterSave_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
%     global m_vspfname_for_vspaerotests
    global m_vspfname_for_vspaerofunctionalitytests
    
    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroParmContainersAccessibleAfterSave()\n' );

    % //open the file created in TestVSPAeroCreateFunctionalityModel
    fprintf('\tLoading File...');
    vsp.ReadVSPFile( m_vspfname_for_vspaerofunctionalitytests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Save Vehicle to File ====% //
    fprintf( '\tSaving vehicle file to: %s ...', m_vspfname_for_vspaerofunctionalitytests);
    vsp.WriteVSPFile( vsp.GetVSPFileName(), vsp.SET_ALL );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( 'COMPLETE\n' );

    % //==== Check if ParmContainers are still accessible ====% //
    control_group_settings_container_id = vsp.FindContainer( 'VSPAEROSettings', 0 );   % // auto grouping produces parm containers within VSPAEROSettings

    % // Find All Control Surface IDs to be used in Gain Parm Names
    fprintf('\tFinding All Geometry IDs...\n');
    % find wing
    wing_id = vsp.FindGeom( 'MainWing', 0 );
    assert(~isempty(wing_id))
    % find control surfaces
    aileron1_id = vsp.GetSubSurf( wing_id, 0 );
    assert(~isempty(aileron1_id))
    aileron2_id = vsp.GetSubSurf( wing_id, 1 );
    assert(~isempty(aileron2_id))
    
    % find tail
    horiz_id = vsp.FindGeom( 'Tail', 0 );
    assert(~isempty(horiz_id))
    % find control surfaces
    elevator_id = vsp.GetSubSurf( horiz_id, 0 );
    assert(~isempty(elevator_id))

    gain0 = 0.1;
    angle0 = 1.0;
    gain1 = 0.2;
    angle1 = 2.0;
    gain2 = 0.3;
    angle2 = 3.0;

    % // ControlSurfaceGroup_0
    fprintf('\tChecking All Parms for ControlSurfaceGroup_0...');
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron1_id '_0_Gain'], 'ControlSurfaceGroup_0');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), gain0, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron1_id '_1_Gain'], 'ControlSurfaceGroup_0');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), -gain0, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_0');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), angle0, TEST_TOL );
    fprintf('COMPLETE.\n');

    % // ControlSurfaceGroup_1
    fprintf('\tChecking All Parms for ControlSurfaceGroup_1...');
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron2_id '_0_Gain'], 'ControlSurfaceGroup_1');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), gain1, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' aileron2_id '_1_Gain'], 'ControlSurfaceGroup_1');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), -gain1, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_1');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), angle1, TEST_TOL );
    fprintf('COMPLETE.\n');

    % // ControlSurfaceGroup_2
    fprintf('\tChecking All Parms for ControlSurfaceGroup_2...');
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' elevator_id '_0_Gain'], 'ControlSurfaceGroup_2');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), gain2, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, ['Surf_' elevator_id '_1_Gain'], 'ControlSurfaceGroup_2');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), -gain2, TEST_TOL );
    deflection_gain_id = vsp.FindParm( control_group_settings_container_id, 'DeflectionAngle', 'ControlSurfaceGroup_2');
    assert_delta( vsp.GetParmVal( deflection_gain_id ), angle2, TEST_TOL );
    fprintf('COMPLETE.\n');
end

%% void APITestSuiteVSPAERO::TestVSPAeroCpSlicer()
function TestVSPAeroCpSlicer_test(~)

%     global TEST_TOL     % read in TEST_TOL from global workspace
    global m_vspfname_for_vspaerotests

    fprintf( 'APITestSuiteVSPAERO::TestVSPAeroCpSlicer()\n' );

    % //open the file created in TestVSPAeroCreateModel
    vsp.ReadVSPFile( m_vspfname_for_vspaerotests );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Analysis: CpSlicer ====% //
    analysis_name = 'CpSlicer';
    fprintf( '\t%s\n', analysis_name);
    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // Setup cuts
    ycuts = vsp.DoubleVector();
    ycuts.push_back(2.0);
    ycuts.push_back(4.5);
    ycuts.push_back(8.0);

    vsp.SetDoubleAnalysisInput( analysis_name, 'YSlicePosVec', ycuts, 0 );

    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    fprintf( '\n\t\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    fprintf( 'COMPLETE\n\n' );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % // Get & Display Results
    vsp.PrintResults( results_id );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    fprintf( '\n' );
end

function t_tess_w_result = calcTessWCheckVal( t_tess_w )

    t_mult = 4;
    t_shift = 1;

    t_tess_w_result = t_mult * ceil( ( t_tess_w - t_shift ) / t_mult ) + t_shift;
    
end