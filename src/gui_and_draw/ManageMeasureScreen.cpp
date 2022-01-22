#include "ManageMeasureScreen.h"
#include "ScreenMgr.h"
#include "MeasureMgr.h"
#include "VSP_Geom_API.h"
#include "UnitConversion.h"

ManageMeasureScreen::ManageMeasureScreen( ScreenMgr * mgr ) : TabScreen( mgr, 300, 700+35, "Measure" )
{
    Fl_Group* ruler_tab = AddTab( "Rulers" );
    Fl_Group* ruler_group = AddSubGroup( ruler_tab, 5 );
    m_RulerLayout.SetGroupAndScreen( ruler_group, this );

    Fl_Group* probe_tab = AddTab( "Probe" );
    Fl_Group* probe_group = AddSubGroup( probe_tab, 5 );
    m_ProbeLayout.SetGroupAndScreen( probe_group, this );

    m_RulerLayout.SetInputWidth( 100 );
    m_RulerLayout.SetButtonWidth( 75 );

    m_RulerLengthUnitChoice.AddItem( "mm" );
    m_RulerLengthUnitChoice.AddItem( "cm" );
    m_RulerLengthUnitChoice.AddItem( "m" );
    m_RulerLengthUnitChoice.AddItem( "in" );
    m_RulerLengthUnitChoice.AddItem( "ft" );
    m_RulerLengthUnitChoice.AddItem( "yd" );
    m_RulerLengthUnitChoice.AddItem( "Unitless" );

    m_RulerLayout.AddChoice( m_RulerLengthUnitChoice, "Length Unit" );

    m_RulerLayout.SetSameLineFlag( true );
    m_RulerLayout.SetFitWidthFlag( false );

    m_RulerLayout.AddYGap();

    m_RulerLayout.AddInput( m_RulerNameInput, "Name:" );

    m_RulerLayout.SetFitWidthFlag( true );
    m_RulerLayout.AddButton( m_AddRulerButton, "Add", m_RulerLayout.GetX() );

    m_RulerLayout.ForceNewLine();
    m_RulerLayout.SetSameLineFlag( false );

    m_RulerBrowser = m_RulerLayout.AddFlBrowser( 75 );
    m_RulerBrowser->callback( staticScreenCB, this );


    m_RulerLayout.SetButtonWidth( m_RulerLayout.GetW() / 2.0 );
    m_RulerLayout.SetSameLineFlag( true );
    m_RulerLayout.SetFitWidthFlag( false );

    m_RulerLayout.AddButton( m_RemoveRulerButton, "Delete" );
    m_RulerLayout.AddButton( m_RemoveAllRulersButton, "Delete All" );

    m_RulerLayout.ForceNewLine();

    m_RulerLayout.AddButton( m_ShowAllRulersButton, "Show All" );
    m_RulerLayout.AddButton( m_HideAllRulersButton, "Hide All" );

    m_RulerLayout.ForceNewLine();
    m_RulerLayout.SetSameLineFlag( false );
    m_RulerLayout.SetFitWidthFlag( true );
    m_RulerLayout.SetButtonWidth( 75 );

    m_RulerLayout.AddYGap();

    m_RulerLayout.AddDividerBox( "Ruler Control" );

    m_RulerLayout.AddYGap();

    m_RulerDirectionChoice.AddItem( "All", vsp::ALL_DIR );
    m_RulerDirectionChoice.AddItem( "X", vsp::X_DIR );
    m_RulerDirectionChoice.AddItem( "Y", vsp::Y_DIR );
    m_RulerDirectionChoice.AddItem( "Z", vsp::Z_DIR );

    m_RulerLayout.AddChoice( m_RulerDirectionChoice, "Direction" );

    m_RulerLayout.AddYGap();

    m_StartGeom.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_EndGeom.AddExcludeType( PT_CLOUD_GEOM_TYPE );

    m_RulerLayout.AddGeomPicker( m_StartGeom, 0, "Start Geom" );
    m_RulerLayout.AddChoice( m_StartSurfChoice, "Surface" );
    m_RulerLayout.AddSlider( m_StartUSlider, "Start U", 1.0, "%5.4f" );
    m_RulerLayout.AddSlider( m_StartWSlider, "Start W", 1.0, "%5.4f" );

    m_RulerLayout.AddGeomPicker( m_EndGeom, 0, "End Geom" );
    m_RulerLayout.AddChoice( m_EndSurfChoice, "Surface" );
    m_RulerLayout.AddSlider( m_EndUSlider, "End U", 1.0, "%5.4f" );
    m_RulerLayout.AddSlider( m_EndWSlider, "End W", 1.0, "%5.4f" );

    m_RulerLayout.AddYGap();

    m_RulerLayout.AddButton( m_AttachRulerButton, "Re-Attach" );

    m_RulerLayout.AddYGap();

    m_RulerLayout.AddSlider( m_XOffsetSlider, "X Offset", 10.0, "%5.4f" );
    m_RulerLayout.AddSlider( m_YOffsetSlider, "Y Offset", 10.0, "%5.4f" );
    m_RulerLayout.AddSlider( m_ZOffsetSlider, "Z Offset", 10.0, "%5.4f" );

    m_RulerLayout.AddYGap();

    m_RulerLayout.AddSlider( m_PrecisionSlider, "Precision", 10.0, "%5.0f" );
    m_RulerLayout.AddButton( m_VisibleRulerButton, "Visible" );

    m_RulerLayout.AddYGap();

    m_RulerLayout.AddDividerBox( "Results" );

    m_RulerLayout.AddYGap();

    m_RulerLayout.AddDividerBox( "Displacement" );

    m_RulerLayout.AddOutput( m_DeltaXOutput, "Delta X", "%6.5g" );
    m_RulerLayout.AddOutput( m_DeltaYOutput, "Delta Y", "%6.5g" );
    m_RulerLayout.AddOutput( m_DeltaZOutput, "Delta Z", "%6.5g" );

    m_RulerLayout.AddYGap();

    m_RulerLayout.AddDividerBox( "Distance" );

    m_RulerLayout.AddOutput( m_DistanceOutput, "Distance", "%6.5g" );


    /* Probe Layout  ***********************************************/

    m_ProbeLayout.SetInputWidth( 100 );
    m_ProbeLayout.SetButtonWidth( 75 );

    m_ProbeLengthUnitChoice.AddItem( "mm" );
    m_ProbeLengthUnitChoice.AddItem( "cm" );
    m_ProbeLengthUnitChoice.AddItem( "m" );
    m_ProbeLengthUnitChoice.AddItem( "in" );
    m_ProbeLengthUnitChoice.AddItem( "ft" );
    m_ProbeLengthUnitChoice.AddItem( "yd" );
    m_ProbeLengthUnitChoice.AddItem( "Unitless" );

    m_ProbeLayout.AddChoice( m_ProbeLengthUnitChoice, "Length Unit" );

    m_ProbeLayout.SetSameLineFlag( true );
    m_ProbeLayout.SetFitWidthFlag( false );

    m_ProbeLayout.AddYGap();

    m_ProbeLayout.AddInput( m_ProbeNameInput, "Name:" );

    m_ProbeLayout.SetFitWidthFlag( true );
    m_ProbeLayout.AddButton( m_AddProbeButton, "Add", m_ProbeLayout.GetX() );

    m_ProbeLayout.ForceNewLine();
    m_ProbeLayout.SetSameLineFlag( false );

    m_ProbeBrowser = m_ProbeLayout.AddFlBrowser( 75 );
    m_ProbeBrowser->callback( staticScreenCB, this );


    m_ProbeLayout.SetButtonWidth( m_ProbeLayout.GetW() / 2.0 );
    m_ProbeLayout.SetSameLineFlag( true );
    m_ProbeLayout.SetFitWidthFlag( false );

    m_ProbeLayout.AddButton( m_RemoveProbeButton, "Delete" );
    m_ProbeLayout.AddButton( m_RemoveAllProbesButton, "Delete All" );

    m_ProbeLayout.ForceNewLine();

    m_ProbeLayout.AddButton( m_ShowAllProbesButton, "Show All" );
    m_ProbeLayout.AddButton( m_HideAllProbesButton, "Hide All" );

    m_ProbeLayout.ForceNewLine();
    m_ProbeLayout.SetSameLineFlag( false );
    m_ProbeLayout.SetFitWidthFlag( true );
    m_ProbeLayout.SetButtonWidth( 75 );

    m_ProbeLayout.AddYGap();

    m_ProbeLayout.AddDividerBox( "Probe Control" );

    m_ProbeLayout.AddYGap();

    m_StartGeom.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_EndGeom.AddExcludeType( PT_CLOUD_GEOM_TYPE );

    m_ProbeLayout.AddGeomPicker( m_ProbeGeom, 0, "Geom" );
    m_ProbeLayout.AddChoice( m_ProbeSurfChoice, "Surface" );
    m_ProbeLayout.AddSlider( m_ProbeUSlider, "U", 1.0, "%5.4f" );
    m_ProbeLayout.AddSlider( m_ProbeWSlider, "W", 1.0, "%5.4f" );
    m_ProbeLayout.AddSlider( m_ProbeLenSlider, "Len", 1.0, "%5.2f" );

    m_ProbeLayout.AddYGap();

    m_ProbeLayout.AddButton( m_AttachProbeButton, "Re-Attach" );

    m_ProbeLayout.AddYGap();

    m_ProbeLayout.AddSlider( m_ProbePrecisionSlider, "Precision", 10.0, "%5.0f" );
    m_ProbeLayout.AddButton( m_VisibleProbeButton, "Visible" );

    m_ProbeLayout.AddYGap();

    m_ProbeLayout.AddDividerBox( "Results" );

    m_ProbeLayout.AddYGap();

    m_ProbeLayout.AddDividerBox( "Position" );

    m_ProbeLayout.AddOutput( m_XOutput, "X", "%6.5g" );
    m_ProbeLayout.AddOutput( m_YOutput, "Y", "%6.5g" );
    m_ProbeLayout.AddOutput( m_ZOutput, "Z", "%6.5g" );

    m_ProbeLayout.AddYGap();

    m_ProbeLayout.AddDividerBox( "Normal" );

    m_ProbeLayout.AddOutput( m_NXOutput, "N_X", "%6.5g" );
    m_ProbeLayout.AddOutput( m_NYOutput, "N_Y", "%6.5g" );
    m_ProbeLayout.AddOutput( m_NZOutput, "N_Z", "%6.5g" );

    m_ProbeLayout.AddYGap();

    m_ProbeLayout.AddDividerBox( "Curvature" );

    m_ProbeLayout.AddOutput( m_K1Output, "K Max", "%6.5g" );
    m_ProbeLayout.AddOutput( m_K2Output, "K Min", "%6.5g" );
    m_ProbeLayout.AddOutput( m_KaOutput, "K Mean", "%6.5g" );
    m_ProbeLayout.AddOutput( m_KgOutput, "K Gaussian", "%6.5g" );

    ruler_tab->show();
}

ManageMeasureScreen::~ManageMeasureScreen()
{
}

void ManageMeasureScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void ManageMeasureScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ManageMeasureScreen::Update()
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    std::vector < Ruler * > rulers = MeasureMgr.GetRulerVec();

    m_RulerLengthUnitChoice.Update( vPtr->m_MeasureLenUnit.GetID() );
    m_ProbeLengthUnitChoice.Update( vPtr->m_MeasureLenUnit.GetID() );

    m_RulerBrowser->clear();
    // Add rulers to browser.
    for( int i = 0; i < ( int )rulers.size(); i++ )
    {
        m_RulerBrowser->add( rulers[i]->GetName().c_str() );
    }

    int index = MeasureMgr.GetCurrRulerIndex();
    if ( index >= 0 && index < ( int )rulers.size() )
    {
        m_RulerBrowser->select( index + 1 );
    }


    m_StartSurfChoice.ClearItems();
    m_EndSurfChoice.ClearItems();

    // Ruler / Text Panel.
    Ruler * ruler = MeasureMgr.GetCurrentRuler();

    if ( ruler )
    {
        m_RulerDirectionChoice.Update( ruler->m_Component.GetID() );

        m_PrecisionSlider.Update( ruler->m_Precision.GetID() );

        m_StartGeom.SetGeomChoice( ruler->m_OriginGeomID );

        m_RulerNameInput.Update( ruler->GetName() );

        m_StartUSlider.Update( ruler->m_OriginU.GetID() );
        m_StartWSlider.Update( ruler->m_OriginW.GetID() );

        m_VisibleRulerButton.Update( ruler->m_Visible.GetID() );

        m_XOffsetSlider.Update( ruler->m_XOffset.GetID() );
        m_YOffsetSlider.Update( ruler->m_YOffset.GetID() );
        m_ZOffsetSlider.Update( ruler->m_ZOffset.GetID() );

        Geom* geom_start = vPtr->FindGeom( ruler->m_OriginGeomID );

        if ( geom_start )
        {
            int nsurf = geom_start->GetNumTotalSurfs();
            char str[256];
            for ( int i = 0; i < nsurf; ++i )
            {
                sprintf( str, "Surf_%d", i );
                m_StartSurfChoice.AddItem( str );
            }
            m_StartSurfChoice.UpdateItems();


            if( ruler->m_OriginIndx() < 0 || ruler->m_OriginIndx() >= nsurf )
            {
                ruler->m_OriginIndx = 0;
            }
            m_StartSurfChoice.SetVal( ruler->m_OriginIndx() );
        }

        m_EndGeom.SetGeomChoice( ruler->m_EndGeomID );

        m_EndUSlider.Update( ruler->m_EndU.GetID() );
        m_EndWSlider.Update( ruler->m_EndW.GetID() );

        char str[255];
        sprintf( str, "%%.%df", ruler->m_Precision() );

        m_DeltaXOutput.SetFormat( str );
        m_DeltaYOutput.SetFormat( str );
        m_DeltaZOutput.SetFormat( str );
        m_DistanceOutput.SetFormat( str );

        string lenunit = LenUnitName( vPtr->m_MeasureLenUnit() );
        m_DeltaXOutput.SetSuffix( lenunit );
        m_DeltaYOutput.SetSuffix( lenunit );
        m_DeltaZOutput.SetSuffix( lenunit );
        m_DistanceOutput.SetSuffix( lenunit );

        m_DeltaXOutput.Update( ruler->m_DeltaX.GetID() );
        m_DeltaYOutput.Update( ruler->m_DeltaY.GetID() );
        m_DeltaZOutput.Update( ruler->m_DeltaZ.GetID() );
        m_DistanceOutput.Update( ruler->m_Distance.GetID() );

        Geom* geom_end = vPtr->FindGeom( ruler->m_EndGeomID );

        if ( geom_end )
        {
            int nsurf = geom_end->GetNumTotalSurfs();
            char str[256];
            for ( int i = 0; i < nsurf; ++i )
            {
                sprintf( str, "Surf_%d", i );
                m_EndSurfChoice.AddItem( str );
            }
            m_EndSurfChoice.UpdateItems();

            if( ruler->m_EndIndx() < 0 || ruler->m_EndIndx() >= nsurf )
            {
                ruler->m_EndIndx = 0;
            }
            m_EndSurfChoice.SetVal( ruler->m_EndIndx() );
        }
    }
    else
    {
        m_StartGeom.SetGeomChoice( "" );
        m_EndGeom.SetGeomChoice( "" );

        m_StartUSlider.Update( "" );
        m_StartWSlider.Update( "" );
        m_EndUSlider.Update( "" );
        m_EndWSlider.Update( "" );

        m_XOffsetSlider.Update( "" );
        m_YOffsetSlider.Update( "" );
        m_ZOffsetSlider.Update( "" );

        m_RulerNameInput.Update( "" );

        m_DeltaXOutput.Update( "" );
        m_DeltaYOutput.Update( "" );
        m_DeltaZOutput.Update( "" );

        m_DistanceOutput.Update( "" );
    }

    m_StartGeom.Update();
    m_EndGeom.Update();

    std::vector < Probe * > probes = MeasureMgr.GetProbeVec();

    m_ProbeBrowser->clear();
    // Add probes to browser.
    for( int i = 0; i < ( int )probes.size(); i++ )
    {
        m_ProbeBrowser->add( probes[i]->GetName().c_str() );
    }

    index = MeasureMgr.GetCurrProbeIndex();
    if ( index >= 0 && index < ( int )probes.size() )
    {
        m_ProbeBrowser->select( index + 1 );
    }


    m_ProbeSurfChoice.ClearItems();

    // Ruler / Text Panel.
    Probe * probe = MeasureMgr.GetCurrentProbe();

    if ( probe )
    {
        m_ProbeGeom.SetGeomChoice( probe->m_OriginGeomID );

        m_ProbeNameInput.Update( probe->GetName() );

        m_ProbeUSlider.Update( probe->m_OriginU.GetID() );
        m_ProbeWSlider.Update( probe->m_OriginW.GetID() );
        m_ProbeLenSlider.Update( probe->m_Len.GetID() );

        m_ProbePrecisionSlider.Update( probe->m_Precision.GetID() );

        m_VisibleProbeButton.Update( probe->m_Visible.GetID() );

        Geom* geom_start = vPtr->FindGeom( probe->m_OriginGeomID );

        if ( geom_start )
        {
            int nsurf = geom_start->GetNumTotalSurfs();
            char str[256];
            for ( int i = 0; i < nsurf; ++i )
            {
                sprintf( str, "Surf_%d", i );
                m_ProbeSurfChoice.AddItem( str );
            }
            m_ProbeSurfChoice.UpdateItems();


            if( probe->m_OriginIndx() < 0 || probe->m_OriginIndx() >= nsurf )
            {
                probe->m_OriginIndx = 0;
            }
            m_ProbeSurfChoice.SetVal( probe->m_OriginIndx() );
        }

        char str[255];
        sprintf( str, "%%.%df", probe->m_Precision() );

        m_XOutput.SetFormat( str );
        m_YOutput.SetFormat( str );
        m_ZOutput.SetFormat( str );

        m_NXOutput.SetFormat( str );
        m_NYOutput.SetFormat( str );
        m_NZOutput.SetFormat( str );

        m_K1Output.SetFormat( str );
        m_K2Output.SetFormat( str );
        m_KaOutput.SetFormat( str );
        m_KgOutput.SetFormat( str );

        string lenunit = LenUnitName( vPtr->m_MeasureLenUnit() );
        m_XOutput.SetSuffix( lenunit );
        m_YOutput.SetSuffix( lenunit );
        m_ZOutput.SetSuffix( lenunit );

        char curvunit[255];
        sprintf( curvunit, "1/%s", lenunit.c_str() );

        m_K1Output.SetSuffix( string( curvunit ) );
        m_K2Output.SetSuffix( string( curvunit ) );
        m_KaOutput.SetSuffix( string( curvunit ) );
        m_KgOutput.SetSuffix( string( curvunit ) );

        m_XOutput.Update( probe->m_X.GetID() );
        m_YOutput.Update( probe->m_Y.GetID() );
        m_ZOutput.Update( probe->m_Z.GetID() );

        m_NXOutput.Update( probe->m_NX.GetID() );
        m_NYOutput.Update( probe->m_NY.GetID() );
        m_NZOutput.Update( probe->m_NZ.GetID() );

        m_K1Output.Update( probe->m_K1.GetID() );
        m_K2Output.Update( probe->m_K2.GetID() );
        m_KaOutput.Update( probe->m_Ka.GetID() );
        m_KgOutput.Update( probe->m_Kg.GetID() );
    }
    else
    {
        m_ProbeGeom.SetGeomChoice( "" );

        m_ProbeUSlider.Update( "" );
        m_ProbeWSlider.Update( "" );
        m_ProbeLenSlider.Update( "" );

        m_ProbeNameInput.Update( "" );

        m_XOutput.Update( "" );
        m_YOutput.Update( "" );
        m_ZOutput.Update( "" );

        m_NXOutput.Update( "" );
        m_NYOutput.Update( "" );
        m_NZOutput.Update( "" );

        m_K1Output.Update( "" );
        m_K2Output.Update( "" );
        m_KaOutput.Update( "" );
        m_KgOutput.Update( "" );
    }

    m_ProbeGeom.Update();

    m_FLTK_Window->redraw();
    return true;
}

void ManageMeasureScreen::CallBack( Fl_Widget * w )
{
    if ( w == m_RulerBrowser )
    {
        int sel = m_RulerBrowser->value();
        MeasureMgr.SetCurrRulerIndex( sel - 1 );
    }
    else if ( w == m_ProbeBrowser )
    {
        int sel = m_ProbeBrowser->value();
        MeasureMgr.SetCurrProbeIndex( sel - 1 );
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageMeasureScreen::GuiDeviceCallBack( GuiDevice* device )
{
    if ( device == &m_AddRulerButton )
    {
        MeasureMgr.CreateAndAddRuler();
    }
    else if ( device == &m_RemoveRulerButton )
    {
        MeasureMgr.DelRuler( MeasureMgr.GetCurrRulerIndex() );
    }
    else if ( device == &m_RemoveAllRulersButton )
    {
        MeasureMgr.DelAllRulers();
    }
    else if ( device == &m_ShowAllRulersButton )
    {
        MeasureMgr.ShowAllRulers();
    }
    else if ( device == &m_HideAllRulersButton )
    {
        MeasureMgr.HideAllRulers();
    }
    else if ( device == &m_RulerNameInput )
    {
        Ruler * ruler = MeasureMgr.GetCurrentRuler();
        if ( ruler )
        {
            ruler->SetName( m_RulerNameInput.GetString() );
        }
    }
    else if ( device == &m_AttachRulerButton )
    {
        Ruler * ruler = MeasureMgr.GetCurrentRuler();
        if ( ruler )
        {
            ruler->Reset();
        }
    }
    else if ( device == & m_StartGeom )
    {
        Ruler * ruler = MeasureMgr.GetCurrentRuler();
        if ( ruler )
        {
            ruler->m_OriginGeomID = m_StartGeom.GetGeomChoice();
        }
    }
    else if ( device == & m_EndGeom )
    {
        Ruler * ruler = MeasureMgr.GetCurrentRuler();
        if ( ruler )
        {
            ruler->m_EndGeomID = m_EndGeom.GetGeomChoice();
        }
    }
    else if ( device == &m_StartSurfChoice )
    {
        Ruler * ruler = MeasureMgr.GetCurrentRuler();
        if ( ruler )
        {
            ruler->m_OriginIndx = m_StartSurfChoice.GetVal();
        }
    }
    else if ( device == &m_EndSurfChoice )
    {
        Ruler * ruler = MeasureMgr.GetCurrentRuler();
        if ( ruler )
        {
            ruler->m_EndIndx = m_EndSurfChoice.GetVal();
        }
    }
    else if ( device == &m_AddProbeButton )
    {
        MeasureMgr.CreateAndAddProbe();
    }
    else if ( device == &m_RemoveProbeButton )
    {
        MeasureMgr.DelProbe( MeasureMgr.GetCurrProbeIndex() );
    }
    else if ( device == &m_RemoveAllProbesButton )
    {
        MeasureMgr.DelAllProbes();
    }
    else if ( device == &m_ShowAllProbesButton )
    {
        MeasureMgr.ShowAllProbes();
    }
    else if ( device == &m_HideAllProbesButton )
    {
        MeasureMgr.HideAllProbes();
    }
    else if ( device == &m_ProbeNameInput )
    {
        Probe * probe = MeasureMgr.GetCurrentProbe();
        if ( probe )
        {
            probe->SetName( m_ProbeNameInput.GetString() );
        }
    }
    else if ( device == &m_AttachProbeButton )
    {
        Probe * probe = MeasureMgr.GetCurrentProbe();
        if ( probe )
        {
            probe->Reset();
        }
    }
    else if ( device == & m_ProbeGeom )
    {
        Probe * probe = MeasureMgr.GetCurrentProbe();
        if ( probe )
        {
            probe->m_OriginGeomID = m_ProbeGeom.GetGeomChoice();
        }
    }
    else if ( device == &m_ProbeSurfChoice )
    {
        Probe * probe = MeasureMgr.GetCurrentProbe();
        if ( probe )
        {
            probe->m_OriginIndx = m_ProbeSurfChoice.GetVal();
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageMeasureScreen::Set( vec3d placement, std::string targetGeomId )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    Ruler * ruler = MeasureMgr.GetCurrentRuler();
    if( ruler && veh )
    {
        if( ruler->m_Stage == STAGE_ZERO )
        {
            ruler->m_Stage = STAGE_ONE;
            ruler->m_OriginGeomID = targetGeomId;

            int index;
            double u, w;
            veh->ProjPnt01I(targetGeomId, placement, index, u, w);

            ruler->m_OriginU = u;
            ruler->m_OriginW = w;
            ruler->m_OriginIndx = index;
        }
        else if( ruler->m_Stage == STAGE_ONE )
        {
            ruler->m_Stage = STAGE_TWO;
            ruler->m_EndGeomID = targetGeomId;

            int index;
            double u, w;
            veh->ProjPnt01I(targetGeomId, placement, index, u, w);

            ruler->m_EndU = u;
            ruler->m_EndW = w;
            ruler->m_EndIndx = index;
        }
        else if( ruler->m_Stage == STAGE_TWO )
        {
            ruler->m_Stage = STAGE_COMPLETE;

            ruler->m_XOffset = placement.x();
            ruler->m_YOffset = placement.y();
            ruler->m_ZOffset = placement.z();
        }
    }

    Probe * probe = MeasureMgr.GetCurrentProbe();
    if( probe && veh )
    {
        if( probe->m_Stage == STAGE_ZERO )
        {
            probe->m_Stage = STAGE_ONE;
            probe->m_OriginGeomID = targetGeomId;

            int index;
            double u, w;
            veh->ProjPnt01I(targetGeomId, placement, index, u, w);

            probe->m_OriginU = u;
            probe->m_OriginW = w;
            probe->m_OriginIndx = index;
        }
        else if( probe->m_Stage == STAGE_ONE )
        {
            probe->m_Stage = STAGE_COMPLETE;

            probe->SetLenFromPlacement( placement );
        }
    }


}

std::string ManageMeasureScreen::getFeedbackGroupName()
{
    return std::string( "MeasureGUIGroup" );
}

void ManageMeasureScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    MeasureMgr.LoadDrawObjs( draw_obj_vec );

    UpdatePickList();

    for( int i = 0; i < ( int )m_PickList.size(); i++ )
    {
        draw_obj_vec.push_back( &m_PickList[i] );
    }
}

void ManageMeasureScreen::UpdatePickList()
{
    // Load all geom.

    Vehicle* veh = VehicleMgr.GetVehicle();
    vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );

    m_PickList.clear();

    Ruler * ruler = MeasureMgr.GetCurrentRuler();

    if ( ruler )
    {
        vector < DrawObj *> drawobj;
        ruler->LoadDrawObjs( drawobj );

        if( drawobj.size() > 0 )
        {
            DrawObj * currDrawObj = drawobj[0];
            if( currDrawObj && currDrawObj->m_Ruler.Step == DrawObj::VSP_RULER_STEP_COMPLETE )
            {
                // Do nothing.
            }
            else if( currDrawObj && currDrawObj->m_Ruler.Step == DrawObj::VSP_RULER_STEP_TWO )
            {
                DrawObj pickDO;
                pickDO.m_Type = DrawObj::VSP_PICK_LOCATION;
                pickDO.m_GeomID = PICKLOCHEADER + currDrawObj->m_GeomID;
                pickDO.m_PickSourceID = "";
                pickDO.m_FeedbackGroup = getFeedbackGroupName();

                m_PickList.push_back( pickDO );
            }
            else if( currDrawObj )
            {
                for ( int i = 0; i < ( int ) geom_vec.size(); i++ )
                {
                    vector<DrawObj *> geom_drawObj_vec;
                    geom_vec[i]->LoadMainDrawObjs( geom_drawObj_vec );

                    for ( int j = 0; j < ( int ) geom_drawObj_vec.size(); j++ )
                    {
                        if ( geom_drawObj_vec[j]->m_Visible )
                        {
                            // Ignore bounding boxes.
                            if ( geom_drawObj_vec[j]->m_GeomID.compare( 0, string( BBOXHEADER ).size(), BBOXHEADER ) != 0 )
                            {
                                DrawObj pickDO;
                                pickDO.m_Type = DrawObj::VSP_PICK_VERTEX;
                                pickDO.m_GeomID = PICKVERTEXHEADER + geom_drawObj_vec[j]->m_GeomID;
                                pickDO.m_PickSourceID = geom_drawObj_vec[j]->m_GeomID;
                                pickDO.m_FeedbackGroup = getFeedbackGroupName();

                                m_PickList.push_back( pickDO );
                            }
                        }
                    }
                }
            }
        }
    }

    Probe * probe = MeasureMgr.GetCurrentProbe();

    if ( probe )
    {
        vector < DrawObj *> drawobj;
        probe->LoadDrawObjs( drawobj );

        if( drawobj.size() > 0 )
        {
            DrawObj * currDrawObj = drawobj[0];
            if( currDrawObj && currDrawObj->m_Probe.Step == DrawObj::VSP_PROBE_STEP_COMPLETE )
            {
                // Do nothing.
            }
            else if( currDrawObj && currDrawObj->m_Probe.Step == DrawObj::VSP_PROBE_STEP_ONE )
            {
                DrawObj pickDO;
                pickDO.m_Type = DrawObj::VSP_PICK_LOCATION;
                pickDO.m_GeomID = PICKLOCHEADER + currDrawObj->m_GeomID;
                pickDO.m_PickSourceID = "";
                pickDO.m_FeedbackGroup = getFeedbackGroupName();

                m_PickList.push_back( pickDO );
            }
            else if( currDrawObj )
            {
                for ( int i = 0; i < ( int ) geom_vec.size(); i++ )
                {
                    vector<DrawObj *> geom_drawObj_vec;
                    geom_vec[i]->LoadMainDrawObjs( geom_drawObj_vec );

                    for ( int j = 0; j < ( int ) geom_drawObj_vec.size(); j++ )
                    {
                        if ( geom_drawObj_vec[j]->m_Visible )
                        {
                            // Ignore bounding boxes.
                            if ( geom_drawObj_vec[j]->m_GeomID.compare( 0, string( BBOXHEADER ).size(), BBOXHEADER ) != 0 )
                            {
                                DrawObj pickDO;
                                pickDO.m_Type = DrawObj::VSP_PICK_VERTEX;
                                pickDO.m_GeomID = PICKVERTEXHEADER + geom_drawObj_vec[j]->m_GeomID;
                                pickDO.m_PickSourceID = geom_drawObj_vec[j]->m_GeomID;
                                pickDO.m_FeedbackGroup = getFeedbackGroupName();

                                m_PickList.push_back( pickDO );
                            }
                        }
                    }
                }
            }
        }
    }

}
