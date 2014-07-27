//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geomScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "CfdMeshScreen.h"
#include "GridDensity.h"
#include "CfdMeshMgr.h"
#include <FL/Fl_File_Chooser.H>
#include "StreamUtil.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CfdMeshScreen::CfdMeshScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    m_Vehicle = m_ScreenMgr->GetVehiclePtr();

    CFDMeshUI* ui = m_CfdMeshUI = new CFDMeshUI();

    VspScreen::SetFlWindow( ui->UIWindow );

    ui->UIWindow->position( 760, 30 );

    m_FLTK_Window = ui->UIWindow;

    //ui->intersectMeshButton->callback( staticCB, this );
    //ui->meshSingleButton->callback( staticCB, this );
    //ui->meshAllButton->callback( staticCB, this );
    ui->finalMeshButton->callback( staticCB, this );

    ui->rigorLimitButton->callback( staticCB, this );

    m_IntersectSubSurfsButton.Init( this, ui->intersectSubButton );

    m_DrawMeshButton.Init( this, ui->viewMeshButton );
    m_DrawSourceButton.Init( this, ui->viewSourceButton );
    m_DrawFarButton.Init( this, ui->viewFarMeshButton );
    m_DrawFarPreButton.Init( this, ui->viewFarPreButton );
    m_DrawBadButton.Init( this, ui->viewBadButton );
    m_DrawSymmButton.Init( this, ui->viewSymmButton );
    m_DrawWakeButton.Init( this, ui->viewWakeButton );
    m_DrawTagsButton.Init( this, ui->viewTags );

    m_BodyEdgeSizeSlider.Init( this, ui->bodyEdgeSizeSlider, ui->bodyEdgeSizeInput, 1.0, " %7.5f" );
    m_MinEdgeSizeSlider.Init( this, ui->minEdgeSizeSlider, ui->minEdgeSizeInput, 1.0, " %7.5f" );
    m_MaxGapSizeSlider.Init( this, ui->maxGapSizeSlider, ui->maxGapSizeInput, 1.0, " %7.5f" );
    m_NumCircSegmentSlider.Init( this, ui->numCircSegmentSlider, ui->numCircSegmentInput, 100.0, " %7.5f" );
    m_GrowRatioSlider.Init( this, ui->growRatioSlider, ui->growRatioInput, 2.0, " %7.5f" );

    m_FarEdgeLengthSlider.Init( this, ui->farEdgeSizeSlider, ui->farEdgeSizeInput, 1.0, " %7.5f" );
    m_FarGapSizeSlider.Init( this, ui->farGapSizeSlider, ui->farGapSizeInput, 1.0, " %7.5f" );
    m_FarCircSegmentSlider.Init( this, ui->farCircSegmentSlider, ui->farCircSegmentInput, 100.0, " %7.5f" );

    m_FarXScaleSlider.Init( this, ui->farXScaleSlider, ui->farXScaleInput, 10.0, " %7.5f" );
    m_FarYScaleSlider.Init( this, ui->farYScaleSlider, ui->farYScaleInput, 10.0, " %7.5f" );
    m_FarZScaleSlider.Init( this, ui->farZScaleSlider, ui->farZScaleInput, 10.0, " %7.5f" );

    m_FarXLocationSlider.Init( this, ui->farXLocSlider, ui->farXLocInput, 5.0, " %7.5f" );
    m_FarYLocationSlider.Init( this, ui->farYLocSlider, ui->farYLocInput, 5.0, " %7.5f" );
    m_FarZLocationSlider.Init( this, ui->farZLocSlider, ui->farZLocInput, 5.0, " %7.5f" );

    m_WakeScaleSlider.Init( this, ui->wakeScaleSlider, ui->wakeScaleInput, 10.0, " %7.5f" );
    m_WakeAngleSlider.Init( this, ui->wakeAngleSlider, ui->wakeAngleInput, 10.0, " %7.5f" );

    ui->compChoice->callback( staticCB, this );
    ui->sourceBrowser->callback( staticCB, this );
    ui->SourceNameInput->callback( staticCB, this );

    m_LengthSlider.Init( this, ui->lengthSlider, ui->lengthInput, 1.0, " %7.5f" );
    m_RadiusSlider.Init( this, ui->radiusSlider, ui->radiusInput, 1.0, " %7.5f" );

    m_Length2Slider.Init( this, ui->length2Slider, ui->length2Input, 1.0, " %7.5f" );
    m_Radius2Slider.Init( this, ui->radius2Slider, ui->radius2Input, 1.0, " %7.5f" );

    m_U1Slider.Init( this, ui->u1Slider, ui->u1Input, 1.0, " %7.5f" );
    m_W1Slider.Init( this, ui->w1Slider, ui->w1Input, 1.0, " %7.5f" );

    m_U2Slider.Init( this, ui->u2Slider, ui->u2Input, 1.0, " %7.5f" );
    m_W2Slider.Init( this, ui->w2Slider, ui->w2Input, 1.0, " %7.5f" );

    ui->addSourceButton->callback( staticCB, this );
    ui->deleteSourceButton->callback( staticCB, this );

    ui->addDefaultsButton->callback( staticCB, this );

    ui->adjLenDownButton->callback( staticCB, this );
    ui->adjLenDownDownButton->callback( staticCB, this );
    ui->adjLenUpButton->callback( staticCB, this );
    ui->adjLenUpUpButton->callback( staticCB, this );
    ui->adjRadDownButton->callback( staticCB, this );
    ui->adjRadDownDownButton->callback( staticCB, this );
    ui->adjRadUpButton->callback( staticCB, this );
    ui->adjRadUpUpButton->callback( staticCB, this );

    ui->outputText->buffer( &m_TextBuffer );

    m_DatToggleButton.Init( this, ui->datToggle );
    m_KeyToggleButton.Init( this, ui->keyToggle );
    m_ObjToggleButton.Init( this, ui->objToggle );
    m_PolyToggleButton.Init( this, ui->polyToggle );
    m_StlToggleButton.Init( this, ui->stlToggle );
    m_TriToggleButton.Init( this, ui->triToggle );
    m_GmshToggleButton.Init( this, ui->gmshToggle );
    m_SrfToggleButton.Init( this, ui->srfToggle );
    m_TkeyToggleButton.Init( this, ui->tkeyToggle );

    ui->datButton->callback( staticCB, this );
    ui->keyButton->callback( staticCB, this );
    ui->objButton->callback( staticCB, this );
    ui->polyButton->callback( staticCB, this );
    ui->stlButton->callback( staticCB, this );
    ui->triButton->callback( staticCB, this );
    ui->gmshButton->callback( staticCB, this );
    ui->srfButton->callback( staticCB, this );
    ui->tkeyButton->callback( staticCB, this );

    ui->addWakeButton->callback( staticCB, this );
    ui->addWakeButton->value( 0 );
    ui->wakeCompChoice->callback( staticCB, this );

    ui->halfMeshButton->callback( staticCB, this );

    ui->farMeshButton->callback( staticCB, this );
    ui->farCompChoice->callback( staticCB, this );
    ui->farCenLocButton->callback( staticCB, this );
    ui->farManLocButton->callback( staticCB, this );
    ui->farAbsSizeButton->callback( staticCB, this );
    ui->farRelSizeButton->callback( staticCB, this );
    ui->farBoxGenButton->callback( staticCB, this );
    ui->farComponentGenButton->callback( staticCB, this );
    ui->farXScaleAbsInput->callback( staticCB, this );
    ui->farYScaleAbsInput->callback( staticCB, this );
    ui->farZScaleAbsInput->callback( staticCB, this );
}

CfdMeshScreen::~CfdMeshScreen()
{
    delete m_CfdMeshUI;
}

void CfdMeshScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

bool CfdMeshScreen::Update()
{
    int i;

    CfdMeshMgr.UpdateDomain();

    //==== Base Len ====//

    m_BodyEdgeSizeSlider.Update( CfdMeshMgr.GetGridDensityPtr()->m_BaseLen.GetID() );
    m_MinEdgeSizeSlider.Update( CfdMeshMgr.GetGridDensityPtr()->m_MinLen.GetID() );
    m_MaxGapSizeSlider.Update( CfdMeshMgr.GetGridDensityPtr()->m_MaxGap.GetID() );
    m_NumCircSegmentSlider.Update( CfdMeshMgr.GetGridDensityPtr()->m_NCircSeg.GetID() );
    m_GrowRatioSlider.Update( CfdMeshMgr.GetGridDensityPtr()->m_GrowRatio.GetID() );
    m_IntersectSubSurfsButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_IntersectSubSurfs.GetID() );

    m_FarXScaleSlider.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_FarXScale.GetID() );
    m_FarYScaleSlider.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_FarYScale.GetID() );
    m_FarZScaleSlider.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_FarZScale.GetID() );


//  char xstr[255];
//  char ystr[255];
//  char zstr[255];
//  sprintf( xstr, "%0.4f", CfdMeshMgr.GetFarLength() );
//  sprintf( ystr, "%0.4f", CfdMeshMgr.GetFarWidth() );
//  sprintf( zstr, "%0.4f", CfdMeshMgr.GetFarHeight() );
//  m_CfdMeshUI->farXScaleAbsInput->value(xstr);
//  m_CfdMeshUI->farYScaleAbsInput->value(ystr);
//  m_CfdMeshUI->farZScaleAbsInput->value(zstr);


    m_FarXLocationSlider.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_FarXLocation.GetID() );
    m_FarYLocationSlider.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_FarYLocation.GetID() );
    m_FarZLocationSlider.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_FarZLocation.GetID() );

    m_FarEdgeLengthSlider.Update( CfdMeshMgr.GetGridDensityPtr()->m_FarMaxLen.GetID() );
    m_FarGapSizeSlider.Update( CfdMeshMgr.GetGridDensityPtr()->m_FarMaxGap.GetID() );
    m_FarCircSegmentSlider.Update( CfdMeshMgr.GetGridDensityPtr()->m_FarNCircSeg.GetID() );

    m_WakeScaleSlider.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_WakeScale.GetID() );
    m_WakeAngleSlider.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_WakeAngle.GetID() );

    //==== Load Geom Choice ====//
    m_GeomVec = m_Vehicle->GetGeomVec();

    m_CfdMeshUI->compChoice->clear();
    m_CfdMeshUI->wakeCompChoice->clear();
    m_CfdMeshUI->farCompChoice->clear();
    m_CompIDMap.clear();

    for ( i = 0 ; i < ( int )m_GeomVec.size() ; i++ )
    {
        char str[256];
        sprintf( str, "%d_%s", i, m_Vehicle->FindGeom( m_GeomVec[i] )->GetName().c_str() );
        m_CfdMeshUI->compChoice->add( str );
        m_CfdMeshUI->wakeCompChoice->add( str );
        m_CfdMeshUI->farCompChoice->add( str );
        m_CompIDMap[ m_GeomVec[i] ] = i;
    }

    string currGeomID = CfdMeshMgr.GetCurrGeomID();

    if( currGeomID.length() == 0 && m_GeomVec.size() > 0 )
    {
        // Handle case default case.
        currGeomID = m_GeomVec[0];
        CfdMeshMgr.SetCurrGeomID( currGeomID );
    }

    Geom* currGeom = m_Vehicle->FindGeom( currGeomID );

    m_CfdMeshUI->compChoice->value( m_CompIDMap[ currGeomID ] );
    m_CfdMeshUI->wakeCompChoice->value( m_CompIDMap[ currGeomID ] );

    string farGeomID = CfdMeshMgr.GetFarGeomID();
    m_CfdMeshUI->farCompChoice->value( m_CompIDMap[ farGeomID ] );

    BaseSource* source = CfdMeshMgr.GetCurrSource();

    if ( source )
    {
        m_LengthSlider.Activate();
        m_RadiusSlider.Activate();
        m_CfdMeshUI->SourceNameInput->activate();

        m_LengthSlider.Update( source->m_Len.GetID() );
        m_RadiusSlider.Update( source->m_Rad.GetID() );

        m_CfdMeshUI->SourceNameInput->value( source->GetName().c_str() );

        if ( source->GetType() == BaseSource::POINT_SOURCE )
        {
            m_U1Slider.Activate();
            m_W1Slider.Activate();

            PointSource* ps = ( PointSource* )source;

            m_U1Slider.Update( ps->m_ULoc.GetID() );
            m_W1Slider.Update( ps->m_WLoc.GetID() );

            m_CfdMeshUI->EditSourceTitle->label( "Edit Point Source" );

            m_Length2Slider.Deactivate();
            m_Radius2Slider.Deactivate();
            m_U2Slider.Deactivate();
            m_W2Slider.Deactivate();
        }
        else if ( source->GetType() == BaseSource::LINE_SOURCE )
        {
            m_Length2Slider.Activate();
            m_Radius2Slider.Activate();
            m_U1Slider.Activate();
            m_W1Slider.Activate();
            m_U2Slider.Activate();
            m_W2Slider.Activate();

            LineSource* ps = ( LineSource* )source;

            m_U1Slider.Update( ps->m_ULoc1.GetID() );
            m_W1Slider.Update( ps->m_WLoc1.GetID() );

            m_U2Slider.Update( ps->m_ULoc2.GetID() );
            m_W2Slider.Update( ps->m_WLoc2.GetID() );

            m_Length2Slider.Update( ps->m_Len2.GetID() );
            m_Radius2Slider.Update( ps->m_Rad2.GetID() );

            m_CfdMeshUI->EditSourceTitle->label( "Edit Line Source" );
        }
        else if ( source->GetType() == BaseSource::BOX_SOURCE )
        {
            m_U1Slider.Activate();
            m_W1Slider.Activate();
            m_U2Slider.Activate();
            m_W2Slider.Activate();

            BoxSource* ps = ( BoxSource* )source;

            m_U1Slider.Update( ps->m_ULoc1.GetID() );
            m_W1Slider.Update( ps->m_WLoc1.GetID() );

            m_U2Slider.Update( ps->m_ULoc2.GetID() );
            m_W2Slider.Update( ps->m_WLoc2.GetID() );

            m_CfdMeshUI->EditSourceTitle->label( "Edit Box Source" );

            m_Length2Slider.Deactivate();
            m_Radius2Slider.Deactivate();
        }
    }
    else
    {
        m_LengthSlider.Deactivate();
        m_RadiusSlider.Deactivate();
        m_Length2Slider.Deactivate();
        m_Radius2Slider.Deactivate();
        m_U1Slider.Deactivate();
        m_W1Slider.Deactivate();
        m_U2Slider.Deactivate();
        m_W2Slider.Deactivate();
        m_CfdMeshUI->SourceNameInput->deactivate();
        m_CfdMeshUI->EditSourceTitle->label( "" );
    }

    //==== Load Up Source Browser ====//
    int currSourceID = -1;

    m_CfdMeshUI->sourceBrowser->clear();

    if( currGeom )
    {
        vector< BaseSource* > sVec = currGeom->getCfdMeshSourceVec();
        for ( i = 0 ; i < ( int )sVec.size() ; i++ )
        {
            if ( source == sVec[i] )
            {
                currSourceID = i;
            }
            m_CfdMeshUI->sourceBrowser->add( sVec[i]->GetName().c_str() );
        }

        if ( currSourceID >= 0 && currSourceID < ( int )sVec.size() )
        {
            m_CfdMeshUI->sourceBrowser->select( currSourceID + 1 );
        }

    }

    m_DrawMeshButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_DrawMeshFlag.GetID() );
    m_DrawSourceButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_DrawSourceFlag.GetID() );
    m_DrawFarButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_DrawFarFlag.GetID() );
    m_DrawFarPreButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_DrawFarPreFlag.GetID() );
    m_DrawBadButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_DrawBadFlag.GetID() );
    m_DrawSymmButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_DrawSymmFlag.GetID() );
    m_DrawWakeButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_DrawWakeFlag.GetID() );
    m_DrawTagsButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->m_ColorTagsFlag.GetID() );

    if ( CfdMeshMgr.GetCfdSettingsPtr()->GetHalfMeshFlag() )
    {
        m_CfdMeshUI->halfMeshButton->value( 1 );
    }
    else
    {
        m_CfdMeshUI->halfMeshButton->value( 0 );
    }

    if ( CfdMeshMgr.GetGridDensityPtr()->GetRigorLimit() )
    {
        m_CfdMeshUI->rigorLimitButton->value( 1 );
    }
    else
    {
        m_CfdMeshUI->rigorLimitButton->value( 0 );
    }

    string datname = CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileName( CfdMeshSettings::DAT_FILE_NAME );
    m_CfdMeshUI->datName->value( truncateFileName( datname, 40 ).c_str() );
    string keyname = CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileName( CfdMeshSettings::KEY_FILE_NAME );
    m_CfdMeshUI->keyName->value( truncateFileName( keyname, 40 ).c_str() );
    string objname = CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileName( CfdMeshSettings::OBJ_FILE_NAME );
    m_CfdMeshUI->objName->value( truncateFileName( objname, 40 ).c_str() );
    string polyname = CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileName( CfdMeshSettings::POLY_FILE_NAME );
    m_CfdMeshUI->polyName->value( truncateFileName( polyname, 40 ).c_str() );
    string stlname = CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileName( CfdMeshSettings::STL_FILE_NAME );
    m_CfdMeshUI->stlName->value( truncateFileName( stlname, 40 ).c_str() );
    string triname = CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileName( CfdMeshSettings::TRI_FILE_NAME );
    m_CfdMeshUI->triName->value( truncateFileName( triname, 40 ).c_str() );
    string gmshname = CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileName( CfdMeshSettings::GMSH_FILE_NAME );
    m_CfdMeshUI->gmshName->value( truncateFileName( gmshname, 40 ).c_str() );
    string srfname = CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileName( CfdMeshSettings::SRF_FILE_NAME );
    m_CfdMeshUI->srfName->value( truncateFileName( srfname, 40 ).c_str() );
    string tkeyname = CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileName( CfdMeshSettings::TKEY_FILE_NAME );
    m_CfdMeshUI->tkeyName->value( truncateFileName( tkeyname, 40).c_str() );

    //==== Export Flags ====//

    m_DatToggleButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileFlag( CfdMeshSettings::DAT_FILE_NAME )->GetID() );
    m_KeyToggleButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileFlag( CfdMeshSettings::KEY_FILE_NAME )->GetID() );
    m_ObjToggleButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileFlag( CfdMeshSettings::OBJ_FILE_NAME )->GetID() );
    m_PolyToggleButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileFlag( CfdMeshSettings::POLY_FILE_NAME )->GetID() );
    m_StlToggleButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileFlag( CfdMeshSettings::STL_FILE_NAME )->GetID() );
    m_TriToggleButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileFlag( CfdMeshSettings::TRI_FILE_NAME )->GetID() );
    m_GmshToggleButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileFlag( CfdMeshSettings::GMSH_FILE_NAME )->GetID() );
    m_SrfToggleButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileFlag( CfdMeshSettings::SRF_FILE_NAME )->GetID() );
    m_TkeyToggleButton.Update( CfdMeshMgr.GetCfdSettingsPtr()->GetExportFileFlag( CfdMeshSettings::TKEY_FILE_NAME)->GetID() );

    //==== Wake Flag ====//
    if( currGeom )
    {
        if ( currGeom->GetWakeActiveFlag() )
        {
            m_CfdMeshUI->addWakeButton->value( 1 );
        }
        else
        {
            m_CfdMeshUI->addWakeButton->value( 0 );
        }
    }

    //=== Domain tab GUI active areas ===//
    if ( CfdMeshMgr.GetCfdSettingsPtr()->GetFarMeshFlag() )
    {
        m_CfdMeshUI->farParametersGroup->activate();

        if( CfdMeshMgr.GetCfdSettingsPtr()->GetFarCompFlag() )
        {
            m_CfdMeshUI->farBoxGroup->deactivate();
            m_CfdMeshUI->farCompGroup->activate();
        }
        else
        {
            m_CfdMeshUI->farBoxGroup->activate();
            m_CfdMeshUI->farCompGroup->deactivate();

            if( CfdMeshMgr.GetCfdSettingsPtr()->GetFarManLocFlag() )
            {
                m_CfdMeshUI->farXYZLocationGroup->activate();
            }
            else
            {
                m_CfdMeshUI->farXYZLocationGroup->deactivate();
            }
        }
    }
    else
    {
        m_CfdMeshUI->farParametersGroup->deactivate();
    }

    //=== Domain tab GUI radio & highlight buttons ===//
    if( CfdMeshMgr.GetCfdSettingsPtr()->GetFarMeshFlag() )
    {
        m_CfdMeshUI->farMeshButton->value( 1 );
    }
    else
    {
        m_CfdMeshUI->farMeshButton->value( 0 );
    }

    if( CfdMeshMgr.GetCfdSettingsPtr()->GetFarCompFlag() )
    {
        m_CfdMeshUI->farComponentGenButton->setonly();
    }
    else
    {
        m_CfdMeshUI->farBoxGenButton->setonly();
    }

    if( CfdMeshMgr.GetCfdSettingsPtr()->GetFarAbsSizeFlag() )
    {
        m_CfdMeshUI->farAbsSizeButton->value( 1 );
        m_CfdMeshUI->farRelSizeButton->value( 0 );
    }
    else
    {
        m_CfdMeshUI->farAbsSizeButton->value( 0 );
        m_CfdMeshUI->farRelSizeButton->value( 1 );
    }

    if( CfdMeshMgr.GetCfdSettingsPtr()->GetFarManLocFlag() )
    {
        m_CfdMeshUI->farManLocButton->value( 1 );
        m_CfdMeshUI->farCenLocButton->value( 0 );
    }
    else
    {
        m_CfdMeshUI->farManLocButton->value( 0 );
        m_CfdMeshUI->farCenLocButton->value( 1 );
    }

    return false;
}

void CfdMeshScreen::AddOutputText( const string &text )
{
		m_TextBuffer.append( text.c_str() );
		m_CfdMeshUI->outputText->move_down();
		m_CfdMeshUI->outputText->show_insert_position();
		Fl::flush();
}

void CfdMeshScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    CfdMeshMgr.LoadDrawObjs( draw_obj_vec );
}

string CfdMeshScreen::truncateFileName( const string &fn, int len )
{
    string trunc( fn );
    if ( (int)trunc.length() > len )
    {
        trunc.erase( 0, trunc.length() - len );
        trunc.replace( 0, 3, "..." );
    }
    return trunc;
}

void CfdMeshScreen::CallBack( Fl_Widget* w )
{
    static bool intersectFlag = false;
    bool update_flag = true;

    if ( w == m_CfdMeshUI->rigorLimitButton )
    {
        if ( m_CfdMeshUI->rigorLimitButton->value() )
        {
            CfdMeshMgr.GetGridDensityPtr()->SetRigorLimit( true );
        }
        else
        {
            CfdMeshMgr.GetGridDensityPtr()->SetRigorLimit( false );
        }
    }
    else if ( w == m_CfdMeshUI->farMeshButton )
    {
        if ( m_CfdMeshUI->farMeshButton->value() )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetFarMeshFlag( true );
        }
        else
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetFarMeshFlag( false );
        }
    }
    else if ( w == m_CfdMeshUI->halfMeshButton )
    {
        if ( m_CfdMeshUI->halfMeshButton->value() )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetHalfMeshFlag( true );
        }
        else
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetHalfMeshFlag( false );
        }
    }
    else if ( w == m_CfdMeshUI->finalMeshButton )
    {
        redirecter redir( std::cout, CfdMeshMgr.m_OutStream );
        CfdMeshMgr.GenerateMesh();

        // Hide all geoms.
        Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
        vector<string> geomIds = veh->GetGeomVec();
        for( int i = 0; i < (int)geomIds.size(); i++ )
        {
            GeomBase* gPtr = veh->FindGeom( geomIds[i] );
            gPtr->m_GuiDraw.SetNoShowFlag( true );
        }
    }
//  else if ( w == m_CfdMeshUI->addDefaultsButton )
//  {
//      vector<string> geomVec = m_Vehicle->GetGeomVec();
//
//      int currGeomID = CfdMeshMgr.GetCurrGeomID();
//      if ( currGeomID >= 0 && currGeomID < (int)geomVec.size() )
//      {
//          double base_len = CfdMeshMgr.GetGridDensityPtr()->GetBaseLen();
//          geomVec[currGeomID]->AddDefaultSources(base_len);
//      }
//  }

//  else if ( m_FarXScaleSlider->GuiChanged( w ) )
//  {
//      double val = m_FarXScaleSlider->GetVal();
//      bool change = false;
//
//      if ( CfdMeshMgr.GetFarAbsSizeFlag() )
//      {
//          CfdMeshMgr.SetFarAbsSizeFlag( false );
//          change = true;
//      }
//
//      CfdMeshMgr.SetFarXScale( val );
//      CfdMeshMgr.UpdateDomain();
//      char xstr[255];
//      sprintf( xstr, "%0.4f", CfdMeshMgr.GetFarLength() );
//      cfdMeshUI->farXScaleAbsInput->value(xstr);
//
//      if ( change )
//          CfdMeshMgr.SetFarAbsSizeFlag( true );
//
//      update_flag = false;
//  }
//  else if ( m_FarYScaleSlider->GuiChanged( w ) )
//  {
//      double val = m_FarYScaleSlider->GetVal();
//      bool change = false;
//
//      if ( CfdMeshMgr.GetFarAbsSizeFlag() )
//      {
//          CfdMeshMgr.SetFarAbsSizeFlag( false );
//          change = true;
//      }
//
//      CfdMeshMgr.SetFarYScale( val );
//      CfdMeshMgr.UpdateDomain();
//      char ystr[255];
//      sprintf( ystr, "%0.4f", CfdMeshMgr.GetFarWidth() );
//      cfdMeshUI->farYScaleAbsInput->value(ystr);
//
//      if ( change )
//          CfdMeshMgr.SetFarAbsSizeFlag( true );
//
//      update_flag = false;
//  }
//  else if ( m_FarZScaleSlider->GuiChanged( w ) )
//  {
//      double val = m_FarZScaleSlider->GetVal();
//      bool change = false;
//
//      if ( CfdMeshMgr.GetFarAbsSizeFlag() )
//      {
//          CfdMeshMgr.SetFarAbsSizeFlag( false );
//          change = true;
//      }
//
//      CfdMeshMgr.SetFarZScale( val );
//      CfdMeshMgr.UpdateDomain();
//      char zstr[255];
//      sprintf( zstr, "%0.4f", CfdMeshMgr.GetFarHeight() );
//      cfdMeshUI->farZScaleAbsInput->value(zstr);
//
//      if ( change )
//          CfdMeshMgr.SetFarAbsSizeFlag( true );
//
//      update_flag = false;
//  }

//  else if ( w == cfdMeshUI->SourceNameInput )
//  {
//      CfdMeshMgr.GUI_Val( "SourceName", cfdMeshUI->SourceNameInput->value() );
//  }

    else if ( w == m_CfdMeshUI->compChoice )
    {
        //==== Load List of Parts for Comp ====//
        int id = m_CfdMeshUI->compChoice->value();
        CfdMeshMgr.SetCurrGeomID( m_GeomVec[ id ] );
    }
    else if ( w == m_CfdMeshUI->wakeCompChoice )
    {
        //==== Load List of Parts for Comp ====//
        int id = m_CfdMeshUI->wakeCompChoice->value();
        CfdMeshMgr.SetCurrGeomID( m_GeomVec[ id ] );
    }
    else if ( w == m_CfdMeshUI->farCompChoice )
    {
        //==== Load List of Parts for Comp ====//
        int id = m_CfdMeshUI->farCompChoice->value();
        CfdMeshMgr.SetFarGeomID( m_GeomVec[ id ] );
    }
    else if ( w == m_CfdMeshUI->sourceBrowser )
    {
        CfdMeshMgr.GUI_Val( "SourceID", m_CfdMeshUI->sourceBrowser->value() - 1 );
    }

//  else if ( w == m_CfdMeshUI->u1Slider )
//      CfdMeshMgr.GUI_Val( "U1", cfdMeshUI->u1Slider->value() );
//  else if ( w == m_CfdMeshUI->w1Slider )
//      CfdMeshMgr.GUI_Val( "W1", cfdMeshUI->w1Slider->value() );
//  else if ( w == m_CfdMeshUI->u1Input )
//      CfdMeshMgr.GUI_Val( "U1", atof( cfdMeshUI->u1Input->value() ) );
//  else if ( w == m_CfdMeshUI->w1Input )
//      CfdMeshMgr.GUI_Val( "W1", atof( cfdMeshUI->w1Input->value() ) );
//
//  else if ( w == m_CfdMeshUI->u2Slider )
//      CfdMeshMgr.GUI_Val( "U2", cfdMeshUI->u2Slider->value() );
//  else if ( w == m_CfdMeshUI->w2Slider )
//      CfdMeshMgr.GUI_Val( "W2", cfdMeshUI->w2Slider->value() );
//  else if ( w == m_CfdMeshUI->u2Input )
//      CfdMeshMgr.GUI_Val( "U2", atof( cfdMeshUI->u2Input->value() ) );
//  else if ( w == m_CfdMeshUI->w2Input )
//      CfdMeshMgr.GUI_Val( "W2", atof( cfdMeshUI->w2Input->value() ) );

    else if ( w == m_CfdMeshUI->addSourceButton )
    {
        int type = m_CfdMeshUI->sourceTypeChoice->value();
        if ( type >= 0 && type < BaseSource::NUM_SOURCE_TYPES )
        {
            CfdMeshMgr.AddSource( type );
        }
    }
    else if ( w == m_CfdMeshUI->deleteSourceButton )
    {
        CfdMeshMgr.DeleteCurrSource();
    }
    else if ( w == m_CfdMeshUI->adjLenDownButton )
    {
        CfdMeshMgr.AdjustAllSourceLen( 1.0 / 1.1 );
    }
    else if ( w == m_CfdMeshUI->adjLenUpButton )
    {
        CfdMeshMgr.AdjustAllSourceLen( 1.1 );
    }
    else if ( w == m_CfdMeshUI->adjLenDownDownButton )
    {
        CfdMeshMgr.AdjustAllSourceLen( 1.0 / 1.5 );
    }
    else if ( w == m_CfdMeshUI->adjLenUpUpButton )
    {
        CfdMeshMgr.AdjustAllSourceLen( 1.5 );
    }
    else if ( w == m_CfdMeshUI->adjRadDownButton )
    {
        CfdMeshMgr.AdjustAllSourceRad( 1.0 / 1.1 );
    }
    else if ( w == m_CfdMeshUI->adjRadUpButton )
    {
        CfdMeshMgr.AdjustAllSourceRad( 1.1 );
    }
    else if ( w == m_CfdMeshUI->adjRadDownDownButton )
    {
        CfdMeshMgr.AdjustAllSourceRad( 1.0 / 1.5 );
    }
    else if ( w == m_CfdMeshUI->adjRadUpUpButton )
    {
        CfdMeshMgr.AdjustAllSourceRad( 1.5 );
    }

    else if ( w == m_CfdMeshUI->datButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select NASCART .dat file.", "*.dat" );
        if ( newfile.compare( "" ) != 0 )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( newfile, CfdMeshSettings::DAT_FILE_NAME );
        }
    }
    else if ( w == m_CfdMeshUI->keyButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select NASCART .key file.", "*.key" );
        if ( newfile.compare( "" ) != 0 )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( newfile, CfdMeshSettings::KEY_FILE_NAME );
        }
    }
    else if ( w == m_CfdMeshUI->objButton  )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .obj file.", "*.obj" );
        if ( newfile.compare( "" ) != 0 )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( newfile, CfdMeshSettings::OBJ_FILE_NAME );
        }
    }
    else if ( w == m_CfdMeshUI->polyButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .poly file.", "*.poly" );
        if ( newfile.compare( "" ) != 0 )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( newfile, CfdMeshSettings::POLY_FILE_NAME );
        }
    }
    else if ( w == m_CfdMeshUI->stlButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .stl file.", "*.stl" );
        if ( newfile.compare( "" ) != 0 )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( newfile, CfdMeshSettings::STL_FILE_NAME );
        }
    }
    else if ( w == m_CfdMeshUI->triButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .tri file.", "*.tri" );
        if ( newfile.compare( "" ) != 0 )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( newfile, CfdMeshSettings::TRI_FILE_NAME );
        }
    }
    else if ( w == m_CfdMeshUI->gmshButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .msh file.", "*.msh" );
        if ( newfile.compare( "" ) != 0 )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( newfile, CfdMeshSettings::GMSH_FILE_NAME );
        }
    }
    else if ( w == m_CfdMeshUI->srfButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .srf file.", "*.srf" );
        if ( newfile.compare( "" ) != 0 )
        {
            CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( newfile, CfdMeshSettings::SRF_FILE_NAME );
        }
    }
    else if ( w == m_CfdMeshUI->tkeyButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .tkey file.", "*.tkey" );
        if ( newfile.compare( "" ) != 0 )
	    {
            CfdMeshMgr.GetCfdSettingsPtr()->SetExportFileName( newfile, CfdMeshSettings::TKEY_FILE_NAME );
	    }
    }

    else if ( w == m_CfdMeshUI->addWakeButton )
    {
        bool flag = !!( m_CfdMeshUI->addWakeButton->value() );

        vector<string> geomVec = m_Vehicle->GetGeomVec();
        string currGeomID = CfdMeshMgr.GetCurrGeomID();
        m_Vehicle->FindGeom( currGeomID )->SetWakeActiveFlag( flag );
    }
    else if ( w == m_CfdMeshUI->farBoxGenButton )
    {
        CfdMeshMgr.GetCfdSettingsPtr()->SetFarCompFlag( false );
    }
    else if ( w == m_CfdMeshUI->farComponentGenButton )
    {
        CfdMeshMgr.GetCfdSettingsPtr()->SetFarCompFlag( true );
    }
    else if ( w == m_CfdMeshUI->farCenLocButton )
    {
        CfdMeshMgr.GetCfdSettingsPtr()->SetFarManLocFlag( false );
    }
    else if ( w == m_CfdMeshUI->farManLocButton )
    {
        CfdMeshMgr.GetCfdSettingsPtr()->SetFarManLocFlag( true );
    }
    else if ( w == m_CfdMeshUI->farRelSizeButton )
    {
        CfdMeshMgr.GetCfdSettingsPtr()->SetFarAbsSizeFlag( false );
    }
    else if ( w == m_CfdMeshUI->farAbsSizeButton )
    {
        CfdMeshMgr.GetCfdSettingsPtr()->SetFarAbsSizeFlag( true );
    }
//  else if ( w == m_CfdMeshUI->farXScaleAbsInput )
//  {
//      bool change = false;
//
//      if ( !CfdMeshMgr.GetFarAbsSizeFlag() )
//      {
//          CfdMeshMgr.SetFarAbsSizeFlag( true );
//          change = true;
//      }
//
//      double val = atof( m_CfdMeshUI->farXScaleAbsInput->value() );
//      CfdMeshMgr.SetFarLength( val );
//      CfdMeshMgr.UpdateDomain();
//      double scale = CfdMeshMgr.GetFarXScale();
//      m_FarXScaleSlider.SetVal( scale );
//      m_FarXScaleSlider.UpdateGui();
//
//      update_flag = false;
//
//      if ( change )
//          CfdMeshMgr.SetFarAbsSizeFlag( false );
//  }
//
//  else if ( w == m_CfdMeshUI->farYScaleAbsInput )
//  {
//      bool change = false;
//
//      if ( !CfdMeshMgr.GetFarAbsSizeFlag() )
//      {
//          CfdMeshMgr.SetFarAbsSizeFlag( true );
//          change = true;
//      }
//
//      double val = atof( m_CfdMeshUI->farYScaleAbsInput->value() );
//      CfdMeshMgr.SetFarWidth( val );
//      CfdMeshMgr.UpdateDomain();
//      double scale = CfdMeshMgr.GetFarYScale();
//      m_FarYScaleSlider.SetVal( scale );
//      m_FarYScaleSlider.UpdateGui();
//
//      update_flag = false;
//
//      if ( change )
//          CfdMeshMgr.SetFarAbsSizeFlag( false );
//  }
//
//  else if ( w == m_CfdMeshUI->farZScaleAbsInput )
//  {
//      bool change = false;
//
//      if ( !CfdMeshMgr.GetFarAbsSizeFlag() )
//      {
//          CfdMeshMgr.SetFarAbsSizeFlag( true );
//          change = true;
//      }
//
//      double val = atof( m_CfdMeshUI->farZScaleAbsInput->value() );
//      CfdMeshMgr.SetFarHeight( val );
//      CfdMeshMgr.UpdateDomain();
//      double scale = CfdMeshMgr.GetFarZScale();
//      m_FarZScaleSlider.SetVal( scale );
//      m_FarZScaleSlider.UpdateGui();
//
//      update_flag = false;
//
//      if ( change )
//          CfdMeshMgr.SetFarAbsSizeFlag( false );
//  }

    if ( update_flag )
    {
        Update();
    }

    m_ScreenMgr->SetUpdateFlag( true );

//  m_Vehicle->triggerDraw();

}

