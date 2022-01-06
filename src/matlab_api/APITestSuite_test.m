% APITestSuiteTest
function tests = APITestSuite_test()
    tests = functiontests(localfunctions);
end

%% Fresh Fixture Functions
function setup(~)  % do not change function name
    vsp.VSPCheckSetup();
    vsp.VSPRenew();
end

function teardown(~)  % do not change function name
end

%% void APITestSuite::CheckSetup()
function CheckSetup_test(~)
    % fprintf( 'APITestSuite::CheckSetup()\n' );
    
    vsp.VSPCheckSetup();
    vsp.VSPRenew();

    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % assert( !vsp.ErrorMgr.PopErrorAndPrint( stdout ) );    % //PopErrorAndPrint returns TRUE if there is an error we want ASSERT to check that this is FALSE

end

%% void APITestSuite::CreateGeometry()
function CreateGeometry_test(~)

    % fprintf( 'APITestSuite::CreateGeometry()\n' );
    
    types = vsp.GetGeomTypes( );
    assert(numel(types)~=0)

    % fprintf( '\t[ndx]\t%20s\t[geom_id]\t%25s\t[num_geoms]\n','[geom_type]','[geom_type]' );
    n_extra_geoms = 0;
    for  i_geom_type = 1:numel(types)
        % % //=== Create geometry =====//
        % fprintf( '\t%d\t%20s', i_geom_type,types{i_geom_type} )
        if ~strcmp(types{i_geom_type},'CONFORMAL')
            geom_id = vsp.AddGeom( types{i_geom_type} );
        elseif strcmp(types{i_geom_type},'CONFORMAL')
            pod_id = vsp.AddGeom( 'POD' );    % add POD as parent
            n_extra_geoms = n_extra_geoms + 1;
            geom_id = vsp.AddGeom( types{i_geom_type}, pod_id );
        end
        % fprintf( '\t%s', geom_id );
        assert(~isempty(geom_id));
        assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

        % % //=== Set Name ====//
        geom_name = ['TestGeom_' types{i_geom_type}];
        vsp.SetGeomName( geom_id, geom_name );
        % fprintf( '\t%25s', geom_name );
        assert(strcmp(vsp.GetGeomName(geom_id), geom_name));
        assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

        % % //=== Check to make sure it got added to the list ====//
        geoms = vsp.FindGeoms();
        % fprintf( '\t%d', numel(geoms) );
        assert(numel(geoms) == i_geom_type + n_extra_geoms);
        assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
        
        % fprintf( '\n' );
    end
    % fprintf( '\n' );

    % % //=== Save Vehicle to File ====//
    fname = 'apitest_CreateGeometry.vsp3';
    vsp.WriteVSPFile( fname );
    assert(exist(fname,'file')~=0);
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    
end

%% void APITTestSuite::ChangePodParams()
function ChangePodParams_test(~)
    global TEST_TOL     % read in TEST_TOL from global workspace
    
    % fprintf( 'APITestSuite::ChangePodParams()\n' );
    
    % % //==== Add Pod Geom =====% //
    pod_id = vsp.AddGeom( 'POD' );
    assert( ~isempty(pod_id) );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Set Name ====% //
    pod_name = 'Pod';
    vsp.SetGeomName( pod_id, pod_name );
    assert( strcmp(vsp.GetGeomName( pod_id ),pod_name) )
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Change Length with TWO step method: step 1 - GetParm(), step 2 - SetParmValUpdate()
    len_val = 7.0;
    len_id = vsp.GetParm( pod_id, 'Length', 'Design' );
    assert_delta( vsp.SetParmValUpdate( len_id, len_val ), len_val, TEST_TOL );    % //tests SetParmValUpdate)
    assert_delta( vsp.GetParmVal( len_id ), len_val, TEST_TOL );                % //tests GetParmVal
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Change Finess Ratio with ONE step method: SetParmValUpdate()
    finess_val = 10;
    assert_delta( vsp.SetParmValUpdate( pod_id, 'FineRatio', 'Design', finess_val ), finess_val, TEST_TOL ) ;
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Change X Location  with ONE step method: SetParmValUpdate()
    x_loc_val = 3.0;
    assert_delta( vsp.SetParmValUpdate( pod_id, 'X_Rel_Location', 'XForm', x_loc_val ), x_loc_val, TEST_TOL ) ;
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Change Y Location  with ONE step method: SetParmValUpdate()
    y_loc_val = 1.0;
    assert_delta( vsp.SetParmValUpdate( pod_id, 'Y_Rel_Location', 'XForm', y_loc_val ), y_loc_val, TEST_TOL ) ;
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Change Z Location  with ONE step method: SetParmValUpdate()
    z_loc_val = 4.2;
    assert_delta( vsp.SetParmValUpdate( pod_id, 'Z_Rel_Location', 'XForm', z_loc_val ), z_loc_val, TEST_TOL ) ;
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Change Symmetry =====% //
    sym_flag_id = vsp.GetParm( pod_id, 'Sym_Planar_Flag', 'Sym' );
    assert_delta(vsp.SetParmValUpdate( sym_flag_id, double(vsp.SYM_XZ)), double(vsp.SYM_XZ), TEST_TOL );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Save Vehicle to File ====% //
    fname = 'apitest_ChangePodParams.vsp3';
    vsp.WriteVSPFile( fname );
    assert(exist(fname,'file')~=0);
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    
end

%% void APITestSuite::CopyPasteGeometry()
function CopyPasteGeometry_test(~)
    global TEST_TOL     % read in TEST_TOL from global workspace
    
    % fprintf( 'APITestSuite::CopyPasteGeometry()\n' );
    
    % % //==== Add Fuselage Geom =====//
    fuse_id = vsp.AddGeom( 'FUSELAGE' );
    assert( ~isempty(fuse_id) );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Add Pod Geom as a child of the fuse =====//
    first_pod_id = vsp.AddGeom( 'POD', fuse_id );
    assert( ~isempty(first_pod_id) );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Change First Pod Parameters (name, length, finess ratio, y location, x location, symmetry) ====//
    pod_name = 'Pod';
    vsp.SetGeomName( first_pod_id, pod_name );
    % //    test that the parameters got set within the TEST_TOL tolerance
    assert_delta( vsp.SetParmValUpdate(  first_pod_id, 'Length', 'Design', 7.0 ), 7.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( first_pod_id, 'FineRatio', 'Design', 10.0 ), 10.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( first_pod_id, 'X_Rel_Location', 'XForm', 3.0 ), 3.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( first_pod_id, 'Y_Rel_Location', 'XForm', 1.0 ), 1.0, TEST_TOL );
    vsp.SetParmValUpdate( first_pod_id, 'Sym_Planar_Flag', 'Sym', double(vsp.SYM_XZ)  );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Copy/Paste Pod Geom =====//
    vsp.CopyGeomToClipboard( first_pod_id );                % // copy pod to clipboard
    vsp.PasteGeomClipboard( first_pod_id );                    % // Make fuse_id parent
    vsp.SetGeomName( first_pod_id, 'Original_Pod' );            % // change name of first pod so that the newly paasted pod can be found by searching for the name 'Pod'
    second_pod_id = vsp.FindGeom( 'Pod', 0 );    % // search for the copied pod
    assert( ~isempty(second_pod_id) );                % // assert if the 2nd pod was not found (copy/paste operation FAILED)
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % // set name of second pod to something unique
    vsp.SetGeomName( second_pod_id, 'Second_Pod' );    % // change the name of the second pod to something more descriptive
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Change Second Pod Parameters (name, y location, z location, symmetry) ====//
    assert_delta( vsp.SetParmVal( second_pod_id, 'Y_Rel_Location', 'XForm', 0.0 ), 0.0, TEST_TOL );
    assert_delta( vsp.SetParmVal( second_pod_id, 'Z_Rel_Location', 'XForm', 1.0 ), 1.0, TEST_TOL );
    vsp.SetParmVal( second_pod_id, 'Sym_Planar_Flag', 'Sym', 0 );    % // no symmetry
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Check Second pod has the same length, finess ratio, and x location as the first ====//
    assert_delta( vsp.GetParmVal( first_pod_id, 'Length', 'Design'  ), vsp.GetParmVal( second_pod_id, 'Length', 'Design' ), TEST_TOL );
    assert_delta( vsp.GetParmVal( first_pod_id, 'FineRatio', 'Design' ), vsp.GetParmVal( second_pod_id, 'FineRatio', 'Design' ), TEST_TOL );
    assert_delta( vsp.GetParmVal( first_pod_id, 'X_Rel_Location', 'XForm' ), vsp.GetParmVal( second_pod_id, 'X_Rel_Location', 'XForm' ), TEST_TOL );

    % % //==== Save Vehicle to File ====//
    fname = 'apitest_CopyPasteGeometry.vsp3';
    vsp.WriteVSPFile( fname );
    assert(exist(fname,'file')~=0);
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

end

%% void APITestSuite::CheckAnalysisMgr()
function CheckAnalysisMgr_test(~)
    % fprintf( 'APITestSuite::CheckAnalysisMgr()\n' );
    n_analysis = vsp.GetNumAnalysis();
    analysis_names = vsp.ListAnalysis();
    % fprintf( '    Analyses found: %d\n', n_analysis );
    % fprintf( '\t[analysis_name]\n' );
    % fprintf( '\t\t%-20s%s\t%s\n', '[input_name]', '[type]', '[#]' );
    for  i_analysis = 1:n_analysis 
        
        % print out name
        % fprintf( '\t%s\n', analysis_names{i_analysis} );

        % get input names
        input_names = vsp.GetAnalysisInputNames( analysis_names{i_analysis} );
        for i_input_name = 1:numel(input_names)

            current_input_type = vsp.GetAnalysisInputType( analysis_names{i_analysis}, input_names{i_input_name} );
            %current_input_num_data = vsp.GetNumAnalysisInputData( analysis_names{i_analysis}, input_names{i_input_name} );

            % print out name and type enumeration
            % fprintf( '\t\t%-20s%d\t\t%d', input_names{i_input_name}, current_input_type, current_input_num_data );

            % ASSERT if an invalid type is found
            assert( current_input_type ~= vsp.INVALID_TYPE );

            % fprintf( '\n' );
            
        end
        
    end 
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % fprintf( '\n' );
end

%%void APITestSuite::TestAnalysesWithPod()
function TestAnalysesWithPod_test(~)
    global TEST_TOL     % read in TEST_TOL from global workspace

    % fprintf( 'APITestSuite::TestAnalysesWithPod()\n' );

    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Add Pod Geom and set some parameters =====//
    pod_id = vsp.AddGeom( 'POD' );
    assert( ~isempty(pod_id) );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Change Pod Parameters (name, length, finess ratio, y location, x location, symmetry) ====//
    pod_name = 'Pod_Test';
    vsp.SetGeomName( pod_id, pod_name );
    % //    test that the parameters got set within the TEST_TOL tolerance
    assert_delta( vsp.SetParmValUpdate(  pod_id, 'Length', 'Design', 7.0 ), 7.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( pod_id, 'FineRatio', 'Design', 10.0 ), 10.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( pod_id, 'X_Rel_Location', 'XForm', 3.0 ), 3.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( pod_id, 'Y_Rel_Location', 'XForm', 1.0 ), 1.0, TEST_TOL );
    vsp.SetParmValUpdate( pod_id, 'Sym_Planar_Flag', 'Sym', double(vsp.SYM_XZ)  );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Save Vehicle to File ====//
    fname = 'apitest_TestAnalysesWithPod.vsp3';
    vsp.WriteVSPFile( fname );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % % //==== Analysis: CompGeom ====//
    analysis_name = 'CompGeom';
    % fprintf( '\t%s\n', analysis_name );

    % // Set defaults
    vsp.SetAnalysisInputDefaults( analysis_name );

    % // list inputs, type, and current values
    vsp.PrintAnalysisInputs( analysis_name );

    % // Execute
    % fprintf( '\n\t\tExecuting...' );
    results_id = vsp.ExecAnalysis( analysis_name );
    % fprintf( 'COMPLETE\n\n' );

    % // Get & Display Results

    vsp.PrintResults( results_id );

    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % fprintf( '\n' );
end

%% void APITestSuite::TestDXFExport()
function TestDXFExport_test(~)
    global TEST_TOL     % read in TEST_TOL from global workspace
    
    % fprintf( 'APITestSuite::TestDXFExport()\n' );

    % //==== Add Wing Geom and set some parameters =====//
    wing_id = vsp.AddGeom( 'WING' );
    assert(~isempty(wing_id) );
    assert_delta( vsp.SetParmValUpdate(  wing_id, 'TotalSpan', 'WingGeom', 30.0 ), 30.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate(  wing_id, 'LECluster', 'WingGeom', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate(  wing_id, 'TECluster', 'WingGeom', 2.0 ), 2.0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Add Fuselage Geom and set some parameters =====//
    fus_id = vsp.AddGeom( 'FUSELAGE' );
    assert( ~isempty(fus_id) );
    assert_delta( vsp.SetParmValUpdate(  fus_id, 'X_Rel_Location', 'XForm', -9.0 ), -9.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate(  fus_id, 'Z_Rel_Location', 'XForm', -1.0 ), -1.0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Test Default 3D DXF Export =====//
    vsp.ExportFile( 'TestDXF_3D_API.dxf', vsp.SET_ALL, vsp.EXPORT_DXF );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    geom_id = vsp.FindContainer( 'Vehicle', 0 );

    % //==== Test Default 2D 4 View DXF Export =====//
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'DimFlag', 'DXFSettings' ), double(vsp.SET_2D) ), vsp.SET_2D, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    vsp.ExportFile( 'TestDXF_2D_4View_API.dxf', vsp.SET_ALL, vsp.EXPORT_DXF );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== 2D 1 View DXF Export ====//
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'ViewType', 'DXFSettings' ), double(vsp.VIEW_1) ), vsp.VIEW_1, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'TopLeftView', 'DXFSettings' ), double(vsp.VIEW_BOTTOM) ), vsp.VIEW_BOTTOM, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'TopLeftRotation', 'DXFSettings' ), double(vsp.ROT_90) ), vsp.ROT_90, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    vsp.ExportFile( 'TestDXF_2D_1View_API.dxf', vsp.SET_ALL, vsp.EXPORT_DXF );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== 2D 2 Horizontal View DXF Export ====//
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'ViewType', 'DXFSettings' ), double(vsp.VIEW_2HOR) ), vsp.VIEW_2HOR, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'TopRightView', 'DXFSettings' ), double(vsp.VIEW_RIGHT) ), vsp.VIEW_RIGHT, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'TopRightRotation', 'DXFSettings' ), double(vsp.ROT_270) ), vsp.ROT_270, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    vsp.ExportFile( 'TestDXF_2D_2HView_API.dxf', vsp.SET_ALL, vsp.EXPORT_DXF );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== 2D 2 Vertical View DXF Export ====//
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'ViewType', 'DXFSettings' ), double(vsp.VIEW_2VER) ), vsp.VIEW_2VER, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'BottomLeftView', 'DXFSettings' ), double(vsp.VIEW_REAR) ), vsp.VIEW_REAR, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'BottomLeftRotation', 'DXFSettings' ), double(vsp.ROT_0) ), vsp.ROT_0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    vsp.ExportFile( 'TestDXF_2D_2VView_API.dxf', vsp.SET_ALL, vsp.EXPORT_DXF );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Open Each DXF File In A Viewer To Verify ====//
    % fprintf( '-> COMPLETE: Open Each DXF File In A DXF Viewer To Verify \n' );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % fprintf( '\n' );
end

%%void APITestSuite::TestSVGExport()
function TestSVGExport_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
    
    % fprintf( 'APITestSuite::TestSVGExport()\n' );

    % fprintf( '->Generating geometries...\n' );

    % //==== Add Wing Geom and set some parameters =====//
    wing_id = vsp.AddGeom( 'WING' );
    assert( ~isempty( wing_id) );
    assert_delta( vsp.SetParmValUpdate(  wing_id, 'TotalSpan', 'WingGeom', 30.0 ), 30.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate(  wing_id, 'LECluster', 'WingGeom', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate(  wing_id, 'TECluster', 'WingGeom', 2.0 ), 2.0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    % //==== Add Fuselage Geom and set some parameters =====//
    fus_id = vsp.AddGeom( 'FUSELAGE' );
    assert( ~isempty(fus_id) );
    assert_delta( vsp.SetParmValUpdate(  fus_id, 'X_Rel_Location', 'XForm', -9.0 ), -9.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate(  fus_id, 'Z_Rel_Location', 'XForm', -1.0 ), -1.0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    geom_id = vsp.FindContainer( 'Vehicle', 0 );

    % //==== Manually Add Scale Bar ====//
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'LenUnit', 'SVGSettings' ), double(vsp.LEN_IN) ), vsp.LEN_IN, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'Scale', 'SVGSettings' ), 30.0 ), 30.0, TEST_TOL );

    % //==== Test Default 4 View SVG Export =====//
    vsp.ExportFile( 'TestSVG_4View_API.svg', vsp.SET_ALL, vsp.EXPORT_SVG );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % fprintf( '--> 4 View SVG Export Saved To: TestSVG_4View_API.svg \n' );

    % //==== 1 View SVG Export ====//
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'ViewType', 'SVGSettings' ), double(vsp.VIEW_1) ), vsp.VIEW_1, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'TopLeftView', 'SVGSettings' ), double(vsp.VIEW_BOTTOM) ), vsp.VIEW_BOTTOM, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'TopLeftRotation', 'SVGSettings' ), double(vsp.ROT_0) ), vsp.ROT_0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    vsp.ExportFile( 'TestSVG_1View_API.svg', vsp.SET_ALL, vsp.EXPORT_SVG );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % fprintf( '--> 1 View SVG Export Saved To: TestSVG_1View_API.svg \n' );

    % //==== 2 Horizontal View SVG Export ====//
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'ViewType', 'SVGSettings' ), double(vsp.VIEW_2HOR) ), vsp.VIEW_2HOR, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'TopRightView', 'SVGSettings' ), double(vsp.VIEW_RIGHT) ), vsp.VIEW_RIGHT, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'TopRightRotation', 'SVGSettings' ), double(vsp.ROT_0) ), vsp.ROT_0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    vsp.ExportFile( 'TestSVG_2HView_API.svg', vsp.SET_ALL, vsp.EXPORT_SVG );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % fprintf( '--> 2 Horizontal View SVG Export Saved To: TestSVG_2HView_API.svg \n' );

    % //==== 2 Vertical View SVG Export ====//
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'ViewType', 'SVGSettings' ), double(vsp.VIEW_2VER) ), vsp.VIEW_2VER, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'BottomLeftView', 'SVGSettings' ), double(vsp.VIEW_FRONT) ), vsp.VIEW_FRONT, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( geom_id, 'BottomLeftRotation', 'SVGSettings' ), double(vsp.ROT_0) ), vsp.ROT_0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());

    vsp.ExportFile( 'TestSVG_2VView_API.svg', vsp.SET_ALL, vsp.EXPORT_SVG );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % fprintf( '--> 2 Vertical View SVG Export Saved To: TestSVG_2VView_API.svg \n' );

    % //==== Open Each SVG File In A Viewer To Verify ====//
    % fprintf( '-> COMPLETE: Open Each SVG File In A SVG Viewer To Verify \n' );

    %// Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString());
    % fprintf( '\n' );
end

%% void APITestSuite::TestFacetExport()
function TestFacetExport_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
    
    % fprintf( 'APITestSuite::TestFacetExport()\n' );

    % //==== Add Pod Geom and set some parameters =====//
    pod_id = vsp.AddGeom( 'POD' );
    assert( ~isempty(pod_id) );


    % //==== Add SubSurfaces and set some parameters ====/
    subsurf_ellipse_id = vsp.AddSubSurf( pod_id, vsp.SS_ELLIPSE, 0 );
    assert( ~isempty(subsurf_ellipse_id) );

    subsurf_rectangle_id = vsp.AddSubSurf( pod_id, vsp.SS_RECTANGLE, 0 );
    assert( ~isempty(subsurf_rectangle_id) );

    assert_delta( vsp.SetParmVal( vsp.FindParm( subsurf_rectangle_id, 'Center_U', 'SS_Rectangle' ), 0.6 ), 0.6, TEST_TOL );


    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % //==== CFDMesh Method Facet Export =====//
    vsp.SetComputationFileName( vsp.CFD_FACET_TYPE, 'TestCFDMeshFacet_API.facet' );

    % fprintf( '\tComputing CFDMesh...' );

    vsp.ComputeCFDMesh( vsp.SET_ALL, vsp.CFD_FACET_TYPE );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % fprintf( 'COMPLETE\n' );

    % //==== MeshGeom Method Facet Export =====//
    % fprintf( '\tComputing MeshGeom...' );

    vsp.ExportFile( 'TestMeshGeomFacet_API.facet', vsp.SET_ALL, vsp.EXPORT_FACET );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % fprintf( 'COMPLETE\n' );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 
    % fprintf( '\n' );
end

%% void APITestSuite::TestSaveLoad()
function TestSaveLoad_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace

    % fprintf( 'APITestSuite::TestSaveLoad()\n' );

    % //==== Add Wing Geom and set some parameters =====//
    wing_id = vsp.AddGeom( 'WING' );
    assert( ~isempty(wing_id) );
    assert_delta( vsp.SetParmValUpdate(  wing_id, 'TotalSpan', 'WingGeom', 30.0 ), 30.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate(  wing_id, 'LECluster', 'WingGeom', 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate(  wing_id, 'TECluster', 'WingGeom', 2.0 ), 2.0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % //==== Add Fuselage Geom and set some parameters =====//
    fus_id = vsp.AddGeom( 'FUSELAGE' );
    assert( ~isempty(fus_id) );
    assert_delta( vsp.SetParmValUpdate(  fus_id, 'X_Rel_Location', 'XForm', -9.0 ), -9.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate(  fus_id, 'Z_Rel_Location', 'XForm', -1.0 ), -1.0, TEST_TOL );
    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % //==== Record original geoms ====//
    geoms_saved = vsp.FindGeoms();
    
    % //==== Save Vehicle to File ====//
    % fprintf( 'Saving VSP model\n' );
    fname = 'apitest_SaveLoad.vsp3';
    vsp.WriteVSPFile( fname );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % //==== Reset Geometry ====//
    % fprintf( 'Resetting VSP model to blank slate\n' );
    vsp.VSPRenew();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % //==== Read Geometry From File ====//
    % fprintf( 'Reading model from: %s\n', fname );
    vsp.ReadVSPFile( fname );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % //==== Check that all geoms saves were loaded ====//
    % fprintf( 'All geoms in Vehicle:\n' );
    geoms_loaded = vsp.FindGeoms();
    assert(numel(geoms_saved)==numel(geoms_saved));
    for i=1:numel(geoms_loaded)
        assert(strcmp(geoms_loaded{i},geoms_saved{i}));
        % fprintf( 'Geom id: %s name: %s \n', geoms[i], vsp.GetGeomName( geoms[i] ) );
    end

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 
    % fprintf( '\n' );
end


%% void APITestSuite::TestFEAMesh()
function TestFEAMesh_test(~)

    global TEST_TOL     % read in TEST_TOL from global workspace
    
    % fprintf( 'APITestSuite::TestFEAMesh()\n' );

    % //==== Add Pod Geometry ====//
    % fprintf( '\tAdding Geometry and Creating FeaStructure\n' );
    pod_id = vsp.AddGeom( 'POD' );
    assert( ~isempty(pod_id) );

    length = 15.0;

    assert_delta( vsp.SetParmValUpdate( pod_id, 'X_Rel_Location', 'XForm', 5.0 ), 5.0, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( pod_id, 'X_Rel_Rotation', 'XForm', 90 ), 90, TEST_TOL );
    assert_delta( vsp.SetParmValUpdate( pod_id, 'Length', 'Design', length ), length, TEST_TOL );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % //==== Set Structure Units ====//
    veh_id = vsp.FindContainer( 'Vehicle', 0 );
    assert_delta( vsp.SetParmVal( vsp.FindParm( veh_id, 'StructUnit', 'FeaStructure' ), double(vsp.BFT_UNIT) ), double(vsp.BFT_UNIT), TEST_TOL );

    % //==== Add FeaStructure to Pod ====//
    struct_ind = vsp.AddFeaStruct( pod_id );
    assert( struct_ind ~= -1 );

    % //==== Create FeaMaterial ====//
    mat_id = vsp.AddFeaMaterial();
    assert( ~isempty(mat_id) );

    assert_delta( vsp.SetParmVal( vsp.FindParm( mat_id, 'MassDensity', 'FeaMaterial' ), 0.016 ), 0.016, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( mat_id, 'ElasticModulus', 'FeaMaterial' ), 2.0e6 ), 2.0e6, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( mat_id, 'PoissonRatio', 'FeaMaterial' ), 0.4 ), 0.4, TEST_TOL );

    % //==== Create FeaProperty ====//
    prop_id = vsp.AddFeaProperty();
    assert( ~isempty(prop_id) );

    assert_delta( vsp.SetParmVal( vsp.FindParm( prop_id, 'FeaMaterialIndex', 'FeaProperty' ), 4 ), 4, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( prop_id, 'Thickness', 'FeaProperty' ), 0.01 ), 0.01, TEST_TOL );

    % //==== Adjust FeaMeshSettings ====//
    vsp.SetFeaMeshVal( pod_id, struct_ind, vsp.CFD_MAX_EDGE_LEN, 0.75 );
    vsp.SetFeaMeshVal( pod_id, struct_ind, vsp.CFD_MIN_EDGE_LEN, 0.2 );

    % //==== Add Floor ====//
    floor_id = vsp.AddFeaPart( pod_id, struct_ind, vsp.FEA_SLICE );
    assert( ~isempty(floor_id) );

    assert_delta( vsp.SetParmVal( vsp.FindParm( floor_id, 'IncludedElements', 'FeaPart' ), double(vsp.FEA_SHELL_AND_BEAM) ), double(vsp.FEA_SHELL_AND_BEAM), TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( floor_id, 'RelCenterLocation', 'FeaPart' ), 0.34 ), 0.34, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( floor_id, 'OrientationPlane', 'FeaSlice' ), double(vsp.XZ_BODY) ), double(vsp.XZ_BODY), TEST_TOL ); %// XZ_BODY

    assert_delta( vsp.SetParmVal( vsp.FindParm( floor_id, 'FeaPropertyIndex', 'FeaPart' ), 2 ), 2, TEST_TOL );

    % //==== Add Bulkead ====//
    bulkhead_id = vsp.AddFeaPart( pod_id, struct_ind, vsp.FEA_SLICE );
    assert( ~isempty(bulkhead_id) );

    assert_delta( vsp.SetParmVal( vsp.FindParm( bulkhead_id, 'IncludedElements', 'FeaPart' ), double(vsp.FEA_SHELL_AND_BEAM) ), double(vsp.FEA_SHELL_AND_BEAM), TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( bulkhead_id, 'RelCenterLocation', 'FeaPart' ), 0.15 ), 0.15, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( bulkhead_id, 'OrientationPlane', 'FeaSlice' ), double(vsp.SPINE_NORMAL) ), double(vsp.SPINE_NORMAL), TEST_TOL );

    % //==== Add Dome ====//
    dome_id = vsp.AddFeaPart( pod_id, struct_ind, vsp.FEA_DOME );
    assert( ~isempty(dome_id) );

    assert_delta( vsp.SetParmVal( vsp.FindParm( dome_id, 'IncludedElements', 'FeaPart' ), double(vsp.FEA_SHELL) ), double(vsp.FEA_SHELL), TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( dome_id, 'X_Location', 'FeaDome' ), 0.7 * length ), 0.7 * length, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( dome_id, 'A_Radius', 'FeaDome' ), 1.5 ), 1.5, TEST_TOL );

    % //==== Add Stiffener ====//
    stiffener_id = vsp.AddFeaPart( pod_id, struct_ind, vsp.FEA_SLICE );
    assert( ~isempty(stiffener_id) );

    assert_delta( vsp.SetParmVal( vsp.FindParm( stiffener_id, 'IncludedElements', 'FeaPart' ), double(vsp.FEA_BEAM) ), double(vsp.FEA_BEAM), TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( stiffener_id, 'RelCenterLocation', 'FeaPart' ), 0.45 ), 0.45, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( stiffener_id, 'OrientationPlane', 'FeaSlice' ), double(vsp.SPINE_NORMAL) ), double(vsp.SPINE_NORMAL), TEST_TOL );

    % //==== Add LineArray ====//
    line_array_id = vsp.AddFeaSubSurf( pod_id, struct_ind, vsp.SS_LINE_ARRAY );
    assert( ~isempty(line_array_id) );

    assert_delta( vsp.SetParmVal( vsp.FindParm( line_array_id, 'ConstLineType', 'SS_LineArray' ), 1 ), 1, TEST_TOL ); % // Constant W
    assert_delta( vsp.SetParmVal( vsp.FindParm( line_array_id, 'Spacing', 'SS_LineArray' ), 0.25 ), 0.25, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( line_array_id, 'StartLocation', 'SS_LineArray' ), 0.125 ), 0.125, TEST_TOL );

    % //==== Add Hole ====//
    hole_id = vsp.AddFeaSubSurf( pod_id, struct_ind, vsp.SS_RECTANGLE );
    assert( ~isempty(hole_id) );

    assert_delta( vsp.SetParmVal( vsp.FindParm( hole_id, 'IncludedElements', 'SubSurface' ), double(vsp.FEA_BEAM) ), double(vsp.FEA_BEAM), TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( hole_id, 'Center_U', 'SS_Rectangle' ), 0.65 ), 0.65, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( hole_id, 'Center_W', 'SS_Rectangle' ), 0.5 ), 0.5, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( hole_id, 'U_Length', 'SS_Rectangle' ), 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( hole_id, 'W_Length', 'SS_Rectangle' ), 0.1 ), 0.1, TEST_TOL );
    assert_delta( vsp.SetParmVal( vsp.FindParm( hole_id, 'Test_Type', 'SS_Rectangle' ), double(vsp.INSIDE) ), double(vsp.INSIDE), TEST_TOL );

    vsp.Update();
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 

    % //==== Save Vehicle to File ====//
    fname = 'apitest_FEAMesh.vsp3';
    vsp.WriteVSPFile( fname );
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 
    % print_str = '\tVehicle Saved to ' + fname + ' \n';
    % fprintf( print_str );

    % //=== Set Export File Name ===//
    export_name = 'apitest_FEAMesh_calculix.dat';
    vsp.SetFeaMeshFileName( pod_id, struct_ind, vsp.FEA_CALCULIX_FILE_NAME, export_name );
    % print_str = '\tExport File Name Set to ' + export_name + ' \n';
    % fprintf( print_str );

    % //==== Generate FEA Mesh and Export ====//
    % fprintf( '\tGenerating FEA Mesh\n' );
    vsp.ComputeFeaMesh( pod_id, struct_ind, vsp.FEA_CALCULIX_FILE_NAME );

    % // Final check for errors
    assert(strcmp(vsp.ErrorMgrSingleton.getInstance.GetLastError().GetErrorString(),'No Error'),vsp.ErrorMgrSingleton.getInstance.PopLastError().GetErrorString()); 
    % fprintf( '\n' );

    % fprintf( 'COMPLETE\n' );
end