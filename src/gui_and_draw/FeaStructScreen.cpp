//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geomScreen.cpp: implementation of the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "FeaStructScreen.h"
//#include "groupScreen.h"
//#include "scriptMgr.h"
#include "FeaMeshMgr.h"


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FeaStructScreen::FeaStructScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    m_Vehicle = m_ScreenMgr->GetVehiclePtr();

    FEAStructUI* ui = m_FeaStructUI = new FEAStructUI();

    VspScreen::SetFlWindow( ui->UIWindow );

    m_FeaStructUI->UIWindow->position( 760, 30 );

    m_FLTK_Window = ui->UIWindow;

    ui->UIWindow->callback( staticCloseCB, this );

    m_DefEdgeSlider.Init( this, ui->elemSizeSlider, ui->elemSizeInput, 0.5, " %7.5f" );
    m_MinSizeSlider.Init( this, ui->minSizeSlider, ui->minSizeInput, 0.5, " %7.5f" );
    m_MaxGapSlider.Init( this, ui->maxGapSlider, ui->maxGapInput, 1.0, " %7.5f" );
    m_NumCircSegSlider.Init( this, ui->numCircSegSlider, ui->numCircSegInput, 100, " %7.5f" );
    m_GrowRatioSlider.Init( this, ui->growRatioSlider, ui->growRatioInput, 2.0, " %7.5f" );
    m_ThickScaleSlider.Init( this, ui->thickScaleSlider, ui->thickScaleInput, 0.5, " %7.5f" );


    ui->sectIDInput->callback( staticCB, this );
    ui->sectIDLeftButton->callback( staticCB, this );
    ui->sectIDRightButton->callback( staticCB, this );

    ui->editTabs->callback( staticCB, this );

    ui->sparIDInput->callback( staticCB, this );
    ui->sparIDDownButton->callback( staticCB, this );
    ui->sparIDUpButton->callback( staticCB, this );
    ui->addSparButton->callback( staticCB, this );
    ui->delSparButton->callback( staticCB, this );


    m_SparThickSlider.Init( this, ui->sparThickSlider, ui->sparThickInput, 0.5, " %7.5f" );
    m_SparDensitySlider.Init( this, ui->sparDensitySlider, ui->sparDensityInput, 1.0, " %7.5f" );
    m_SparPosSlider.Init( this, ui->sparPosSlider, ui->sparPosInput, 1.0, " %7.5f" );
    m_SparSweepSlider.Init( this, ui->sparSweepSlider, ui->sparSweepInput, 45, " %7.5f" );

    ui->sparSweepAbsButton->callback( staticCB, this );
    ui->sparSweepRelButton->callback( staticCB, this );
    ui->trimSparButton->callback( staticCB, this );

    ui->ribIDInput->callback( staticCB, this );
    ui->ribIDDownButton->callback( staticCB, this );
    ui->ribIDUpButton->callback( staticCB, this );
    ui->addRibButton->callback( staticCB, this );
    ui->delRibButton->callback( staticCB, this );


    m_RibThickSlider.Init( this, ui->ribThickSlider, ui->ribThickInput, 0.5, " %7.5f" );
    m_RibDensitySlider.Init( this, ui->ribDensitySlider, ui->ribDensityInput, 1.0, " %7.5f" );
    m_RibPosSlider.Init( this, ui->ribPosSlider, ui->ribPosInput, 1.0, " %7.5f" );
    m_RibSweepSlider.Init( this, ui->ribSweepSlider, ui->ribSweepInput, 45, " %7.5f" );


    ui->ribSweepAbsButton->callback( staticCB, this );
    ui->ribSweepRelButton->callback( staticCB, this );
    ui->trimRibButton->callback( staticCB, this );

    ui->exportFEAMeshButton->callback( staticCB, this );
    ui->computeFEAMeshButton->callback( staticCB, this );
    ui->drawMeshButton->callback( staticCB, this );

    //==== Upper Skin ====//

    m_UpThickSlider.Init( this, ui->upSkinThickSlider, ui->upSkinThickInput, 0.5, " %7.5f" );
    m_UpDensitySlider.Init( this, ui->upSkinDensitySlider, ui->upSkinDensityInput, 1.0, " %7.5f" );

    ui->upSkinAddSpliceLineButton->callback( staticCB, this );
    ui->upSkinDelSpliceLineButton->callback( staticCB, this );
    ui->upSpliceLineIDDownButton->callback( staticCB, this );
    ui->upSpliceLineIDUpButton->callback( staticCB, this );
    ui->upSpliceLineIDInput->callback( staticCB, this );

    m_UpSpliceLineLocSlider.Init( this, ui->upSkinSpliceLineLocSlider, ui->upSkinSpliceLineLocInput, 1.0, " %7.5f" );
    m_UpDefThickSlider.Init( this, ui->upSkinThickSlider, ui->upSkinThickInput, 0.1, " %7.5f" );

    ui->upSkinExportNoButton->callback( staticCB, this );
    ui->upSkinExportYesButton->callback( staticCB, this );
    ui->upSkinAddSpliceLineButton->callback( staticCB, this );
    ui->upSkinDelSpliceLineButton->callback( staticCB, this );

    ui->upSkinAddSpliceButton->callback( staticCB, this );
    ui->upSkinDelSpliceButton->callback( staticCB, this );

    //==== Add Gl Window to Main Window ====//
    Fl_Widget* w = ui->upSkinSpliceLineGLGroup;
    ui->upSkinSpliceLineGLGroup->begin();
//  m_UpSkinGLWin = new XSecGlWindow(w->x(), w->y(), w->w(), w->h());
    ui->upSkinSpliceLineGLGroup->end();

    m_UpSpliceLocSlider.Init( this, ui->upSplicePosSlider, ui->upSplicePosInput, 1.0, " %7.5f" );
    m_UpSpliceThickSlider.Init( this, ui->upSpliceThickSlider, ui->upSpliceThickInput, 0.1, " %7.5f" );

    ui->upSpliceIDLeftButton->callback( staticCB, this );
    ui->upSpliceIDRightButton->callback( staticCB, this );

    //==== Lower Skin ====//
    m_LowDensitySlider.Init( this, ui->lowSkinThickSlider, ui->lowSkinThickInput, 0.1, " %7.5f" );
    m_LowThickSlider.Init( this, ui->lowSkinDensitySlider, ui->lowSkinDensityInput, 0.1, " %7.5f" );

    ui->lowSkinAddSpliceLineButton->callback( staticCB, this );
    ui->lowSkinDelSpliceLineButton->callback( staticCB, this );
    ui->lowSpliceLineIDDownButton->callback( staticCB, this );
    ui->lowSpliceLineIDUpButton->callback( staticCB, this );
    ui->lowSpliceLineIDInput->callback( staticCB, this );

    m_LowSpliceLineLocSlider.Init( this, ui->lowSkinSpliceLineLocSlider, ui->lowSkinSpliceLineLocInput, 1.0, " %7.5f" );
    m_LowDefThickSlider.Init( this, ui->lowSkinThickSlider, ui->lowSkinThickInput, 0.1, " %7.5f" );

    ui->lowSkinExportNoButton->callback( staticCB, this );
    ui->lowSkinExportYesButton->callback( staticCB, this );
    ui->lowSkinAddSpliceLineButton->callback( staticCB, this );
    ui->lowSkinDelSpliceLineButton->callback( staticCB, this );

    ui->lowSkinAddSpliceButton->callback( staticCB, this );
    ui->lowSkinDelSpliceButton->callback( staticCB, this );

    //==== Add Gl Window to Main Window ====//
    w = ui->lowSkinSpliceLineGLGroup;
    ui->lowSkinSpliceLineGLGroup->begin();
//  m_LowSkinGLWin = new XSecGlWindow(w->x(), w->y(), w->w(), w->h());
    ui->lowSkinSpliceLineGLGroup->end();

    m_LowSpliceLocSlider.Init( this, ui->lowSplicePosSlider, ui->lowSplicePosInput, 1.0, " %7.5f" );
    m_LowSpliceThickSlider.Init( this, ui->lowSpliceThickSlider, ui->lowSpliceThickInput, 0.1, " %7.5f" );

    ui->lowSpliceIDLeftButton->callback( staticCB, this );
    ui->lowSpliceIDRightButton->callback( staticCB, this );

    ui->outputText->buffer( &m_TextBuffer );

    //==== Point Mass =====//
    ui->ptMassIDInput->callback( staticCB, this );
    ui->ptMassIDDownButton->callback( staticCB, this );
    ui->ptMassIDUpButton->callback( staticCB, this );
    ui->addPtMassButton->callback( staticCB, this );
    ui->delPtMassButton->callback( staticCB, this );

    m_pmXPosSlider.Init( this, ui->pmXLocSlider, ui->pmXLocInput, 10.0, " %7.5f" );
    m_pmYPosSlider.Init( this, ui->pmYLocSlider, ui->pmYLocInput, 10.0, " %7.5f" );
    m_pmZPosSlider.Init( this, ui->pmZLocSlider, ui->pmZLocInput, 10.0, " %7.5f" );

    ui->moveAttachPointButton->callback( staticCB, this );

    ui->geomButton->callback( staticCB, this );
    ui->massButton->callback( staticCB, this );
    ui->nastranButton->callback( staticCB, this );
    ui->thickButton->callback( staticCB, this );
    ui->stlButton->callback( staticCB, this );
    ui->geomToggle->value( 1 );
    ui->massToggle->value( 1 );
    ui->nastranToggle->value( 1 );
    ui->thickToggle->value( 1 );
    ui->stlToggle->value( 0 );
}

FeaStructScreen::~FeaStructScreen()
{
    delete m_FeaStructUI;
}

void FeaStructScreen::closeCB( Fl_Widget* w )
{
    m_FeaStructUI->drawMeshButton->value( 0 );
    FeaMeshMgr.SetDrawMeshFlag( false );
    FeaMeshMgr.SetDrawFlag( false );

    FeaMeshMgr.SaveData();

    vector< string > geomVec = m_Vehicle->GetGeomVec();
    for ( int i = 0 ; i < ( int )geomVec.size() ; i++ )
    {
        Geom *g = m_Vehicle->FindGeom( geomVec[i] );

        g->SetSetFlag( SET_SHOWN, false );
        g->SetSetFlag( SET_NOT_SHOWN, true );
    }

    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );

}

void FeaStructScreen::Show()
{
    FeaMeshMgr.SetDrawFlag( true );

    FeaMeshMgr.LoadSurfaces();
    Update();

    m_FLTK_Window->show();

    vector< string > geomVec = m_Vehicle->GetGeomVec();
    for ( int i = 0 ; i < ( int )geomVec.size() ; i++ )
    {
        Geom *g = m_Vehicle->FindGeom( geomVec[i] );

        g->SetSetFlag( SET_SHOWN, true );
        g->SetSetFlag( SET_NOT_SHOWN, false );
    }
    m_ScreenMgr->SetUpdateFlag( true );

}

bool FeaStructScreen::Update()
{
    char str[256];

    //==== Default Elem Size ====//
    m_DefEdgeSlider.Update( FeaMeshMgr.GetGridDensityPtr()->m_BaseLen.GetID() );
    m_MinSizeSlider.Update( FeaMeshMgr.GetGridDensityPtr()->m_MinLen.GetID() );
    m_MaxGapSlider.Update( FeaMeshMgr.GetGridDensityPtr()->m_MaxGap.GetID() );
    m_NumCircSegSlider.Update( FeaMeshMgr.GetGridDensityPtr()->m_NCircSeg.GetID() );
    m_GrowRatioSlider.Update( FeaMeshMgr.GetGridDensityPtr()->m_GrowRatio.GetID() );
    m_ThickScaleSlider.Update( FeaMeshMgr.m_ThickScale.GetID() );

    //==== SectID ====//
    sprintf( str, "        %d ", FeaMeshMgr.GetCurrSectID() );
    m_FeaStructUI->sectIDInput->value( str );

    //==== Spar Rib ID ====//
    sprintf( str, "        %d ", FeaMeshMgr.GetCurrSparID() );
    if ( FeaMeshMgr.GetNumSpars() == 0 )
    {
        sprintf( str, " " );
    }
    m_FeaStructUI->sparIDInput->value( str );

    sprintf( str, "        %d ", FeaMeshMgr.GetCurrRibID() );
    if ( FeaMeshMgr.GetNumRibs() == 0 )
    {
        sprintf( str, " " );
    }
    m_FeaStructUI->ribIDInput->value( str );

    FeaRib* rib = FeaMeshMgr.GetCurrRib();
    if ( rib )
    {
        m_FeaStructUI->ribEditGroup->show();

        m_RibThickSlider.Update( rib->m_Thick.GetID() );
        m_RibDensitySlider.Update( rib->m_Density.GetID() );
        m_RibPosSlider.Update( rib->m_PerSpan.GetID()  );
        m_RibSweepSlider.Update( rib->m_Sweep.GetID()  );

        if ( rib->m_AbsSweepFlag )
        {
            m_FeaStructUI->ribSweepAbsButton->value( 1 );
            m_FeaStructUI->ribSweepRelButton->value( 0 );
        }
        else
        {
            m_FeaStructUI->ribSweepAbsButton->value( 0 );
            m_FeaStructUI->ribSweepRelButton->value( 1 );
        }
        if ( rib->m_TrimFlag )
        {
            m_FeaStructUI->trimRibButton->value( 1 );
        }
        else
        {
            m_FeaStructUI->trimRibButton->value( 0 );
        }
    }
    else
    {
        m_FeaStructUI->ribEditGroup->hide();
    }


    FeaSpar* spar = FeaMeshMgr.GetCurrSpar();
    if ( spar )
    {
        m_FeaStructUI->sparEditGroup->show();
        m_SparThickSlider.Update( spar->m_Thick.GetID() );

        m_SparDensitySlider.Update( spar->m_Density.GetID() );

        m_SparPosSlider.Update( spar->m_PerChord.GetID() );

        m_SparSweepSlider.Update( spar->m_Sweep.GetID() );

        if ( spar->m_AbsSweepFlag )
        {
            m_FeaStructUI->sparSweepAbsButton->value( 1 );
            m_FeaStructUI->sparSweepRelButton->value( 0 );
        }
        else
        {
            m_FeaStructUI->sparSweepAbsButton->value( 0 );
            m_FeaStructUI->sparSweepRelButton->value( 1 );
        }
        if ( spar->m_TrimFlag )
        {
            m_FeaStructUI->trimSparButton->value( 1 );
        }
        else
        {
            m_FeaStructUI->trimSparButton->value( 0 );
        }
    }
    else
    {
        m_FeaStructUI->sparEditGroup->hide();
    }

    //===== Upper Skin ====//
    FeaSkin* upskin = FeaMeshMgr.GetCurrUpperSkin();
    if ( upskin && upskin->GetExportFlag() )
    {
        m_FeaStructUI->upperSkinEditGroup->show();
        m_FeaStructUI->upSkinExportNoButton->value( 0 );
        m_FeaStructUI->upSkinExportYesButton->value( 1 );

        m_UpDefThickSlider.Update( upskin->m_DefaultThick.GetID() );

        m_UpDensitySlider.Update( upskin->m_Density.GetID() );

        FeaSpliceLine* sl = upskin->GetCurrSpliceLine();
        if ( sl )
        {
            m_UpSpliceLineLocSlider.Update( sl->m_PerSpan.GetID() );

            FeaSplice* splice = sl->GetEditSplice();
            if ( splice )
            {
                m_UpSpliceLocSlider.Update( splice->m_Pos.GetID() );
                m_UpSpliceThickSlider.Update( splice->m_Thick.GetID() );
            }
        }

        //==== Splice Line ID ====//
        sprintf( str, "        %d ", upskin->GetCurrSpliceLineID() );
        if ( ( int )upskin->m_SpliceLineVec.size() == 0 )
        {
            sprintf( str, " " );
        }
        m_FeaStructUI->upSpliceLineIDInput->value( str );
    }
    else
    {
        m_FeaStructUI->upperSkinEditGroup->hide();
        m_FeaStructUI->upSkinExportNoButton->value( 1 );
        m_FeaStructUI->upSkinExportYesButton->value( 0 );
        m_FeaStructUI->upSpliceLineIDInput->value( "" );
    }

//  m_UpSkinGLWin->setDrawBase( NULL );
    if ( upskin )
    {
        FeaSpliceLine* sl = upskin->GetCurrSpliceLine();
        if ( sl )
        {
//          m_UpSkinGLWin->setDrawBase( sl );

            if ( sl->GetMode() == FeaSpliceLine::ADD_MODE )
            {
                m_FeaStructUI->upSkinAddSpliceButton->value( 1 );
            }
            else
            {
                m_FeaStructUI->upSkinAddSpliceButton->value( 0 );
            }

            if ( sl->GetMode() == FeaSpliceLine::DEL_MODE )
            {
                m_FeaStructUI->upSkinDelSpliceButton->value( 1 );
            }
            else
            {
                m_FeaStructUI->upSkinDelSpliceButton->value( 0 );
            }
        }
    }
//  m_UpSkinGLWin->redraw();

    //===== Lower Skin ====//
    FeaSkin* lowskin = FeaMeshMgr.GetCurrLowerSkin();
    if ( lowskin && lowskin->GetExportFlag() )
    {
        m_FeaStructUI->lowerSkinEditGroup->show();
        m_FeaStructUI->lowSkinExportNoButton->value( 0 );
        m_FeaStructUI->lowSkinExportYesButton->value( 1 );

        m_LowDefThickSlider.Update( lowskin->m_DefaultThick.GetID() );

        m_LowDensitySlider.Update( lowskin->m_Density.GetID() );

        FeaSpliceLine* sl = lowskin->GetCurrSpliceLine();
        if ( sl )
        {
            m_LowSpliceLineLocSlider.Update( sl->m_PerSpan.GetID() );

            FeaSplice* splice = sl->GetEditSplice();
            if ( splice )
            {
                m_LowSpliceLocSlider.Update( splice->m_Pos.GetID() );
                m_LowSpliceThickSlider.Update( splice->m_Thick.GetID() );
            }
        }

        //==== Splice Line ID ====//
        sprintf( str, "        %d ", lowskin->GetCurrSpliceLineID() );
        if ( ( int )lowskin->m_SpliceLineVec.size() == 0 )
        {
            sprintf( str, " " );
        }
        m_FeaStructUI->lowSpliceLineIDInput->value( str );
    }
    else
    {
        m_FeaStructUI->lowerSkinEditGroup->hide();
        m_FeaStructUI->lowSkinExportNoButton->value( 1 );
        m_FeaStructUI->lowSkinExportYesButton->value( 0 );
        m_FeaStructUI->lowSpliceLineIDInput->value( "" );
    }

//  m_LowSkinGLWin->setDrawBase( NULL );
    if ( lowskin )
    {
        FeaSpliceLine* sl = lowskin->GetCurrSpliceLine();
        if ( sl )
        {
//          m_LowSkinGLWin->setDrawBase( sl );

            if ( sl->GetMode() == FeaSpliceLine::ADD_MODE )
            {
                m_FeaStructUI->lowSkinAddSpliceButton->value( 1 );
            }
            else
            {
                m_FeaStructUI->lowSkinAddSpliceButton->value( 0 );
            }

            if ( sl->GetMode() == FeaSpliceLine::DEL_MODE )
            {
                m_FeaStructUI->lowSkinDelSpliceButton->value( 1 );
            }
            else
            {
                m_FeaStructUI->lowSkinDelSpliceButton->value( 0 );
            }
        }
    }
//  m_LowSkinGLWin->redraw();


    //==== Point Mass ====//
    FeaPointMass* pmass = FeaMeshMgr.GetCurrPointMass();
    if ( pmass )
    {
        m_FeaStructUI->ptMassEditGroup->show();
        m_pmXPosSlider.Update( pmass->m_PosX.GetID() );
        m_pmYPosSlider.Update( pmass->m_PosY.GetID() );
        m_pmZPosSlider.Update( pmass->m_PosZ.GetID() );

        //==== Point Mass ID ====//
        sprintf( str, "        %d ", FeaMeshMgr.GetCurrPointMassID() );
        if ( FeaMeshMgr.GetNumPointMasses() == 0 )
        {
            sprintf( str, " " );
        }
        m_FeaStructUI->ptMassIDInput->value( str );

        sprintf( str, "%f", pmass->m_AttachPos.x() );
        m_FeaStructUI->XAttachInput->value( str );
        sprintf( str, "%f", pmass->m_AttachPos.y() );
        m_FeaStructUI->YAttachInput->value( str );
        sprintf( str, "%f", pmass->m_AttachPos.z() );
        m_FeaStructUI->ZAttachInput->value( str );
    }
    else
    {
        m_FeaStructUI->ptMassEditGroup->hide();
        m_FeaStructUI->ptMassIDInput->value( "" );
    }

    if ( FeaMeshMgr.GetDrawAttachPointsFlag() )
    {
        m_FeaStructUI->moveAttachPointButton->value( 1 );
    }
    else
    {
        m_FeaStructUI->moveAttachPointButton->value( 0 );
    }

    string massname = FeaMeshMgr.GetFeaExportFileName( FeaMeshMgrSingleton::MASS_FILE_NAME );
    m_FeaStructUI->massName->value( truncateFileName( massname, 40 ).c_str() );
    string nastranname = FeaMeshMgr.GetFeaExportFileName( FeaMeshMgrSingleton::NASTRAN_FILE_NAME );
    m_FeaStructUI->nastranName->value( truncateFileName( nastranname, 40 ).c_str() );
    string geomname = FeaMeshMgr.GetFeaExportFileName( FeaMeshMgrSingleton::GEOM_FILE_NAME );
    m_FeaStructUI->geomName->value( truncateFileName( geomname, 40 ).c_str() );
    string thickname = FeaMeshMgr.GetFeaExportFileName( FeaMeshMgrSingleton::THICK_FILE_NAME );
    m_FeaStructUI->thickName->value( truncateFileName( thickname, 40 ).c_str() );
    string stlname = FeaMeshMgr.GetFeaExportFileName( FeaMeshMgrSingleton::STL_FEA_NAME );
    m_FeaStructUI->stlName->value( truncateFileName( stlname, 40 ).c_str() );

    return true;
}

string FeaStructScreen::truncateFileName( const string &fn, int len )
{
    string trunc( fn );
    if ( trunc.length() > len )
    {
        trunc.erase( 0, trunc.length() - len );
        trunc.replace( 0, 3, "..." );
    }
    return trunc;
}

void FeaStructScreen::addOutputText( const char* text )
{
    m_TextBuffer.append( text );
    m_FeaStructUI->outputText->move_down();
    m_FeaStructUI->outputText->show_insert_position();
    Fl::flush();
}

void FeaStructScreen::CallBack( Fl_Widget* w )
{
    if ( w == m_FeaStructUI->sectIDInput )      // Sect ID
    {
        int id = atoi( m_FeaStructUI->sectIDInput->value() );
        FeaMeshMgr.SetCurrSectID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->sectIDLeftButton )
    {
        int id = FeaMeshMgr.GetCurrSectID() - 1;
        FeaMeshMgr.SetCurrSectID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->sectIDRightButton )
    {
        int id = FeaMeshMgr.GetCurrSectID() + 1;
        FeaMeshMgr.SetCurrSectID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->sparIDInput )     // Spar ID
    {
        int id = atoi( m_FeaStructUI->sparIDInput->value() );
        FeaMeshMgr.SetCurrSparID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->sparIDDownButton )
    {
        int id = FeaMeshMgr.GetCurrSparID() - 1;
        FeaMeshMgr.SetCurrSparID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->sparIDUpButton )
    {
        int id = FeaMeshMgr.GetCurrSparID() + 1;
        FeaMeshMgr.SetCurrSparID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->ribIDInput )      // Rib ID
    {
        int id = atoi( m_FeaStructUI->ribIDInput->value() );
        FeaMeshMgr.SetCurrRibID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->ribIDDownButton )
    {
        int id = FeaMeshMgr.GetCurrRibID() - 1;
        FeaMeshMgr.SetCurrRibID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->ribIDUpButton )
    {
        int id = FeaMeshMgr.GetCurrRibID() + 1;
        FeaMeshMgr.SetCurrRibID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->addRibButton )        // Add Ribs
    {
        FeaMeshMgr.AddRib();
        Update();
    }
    else if ( w == m_FeaStructUI->delRibButton )
    {
        FeaMeshMgr.DelCurrRib();
    }
    else if ( w == m_FeaStructUI->ribSweepAbsButton )
    {
        FeaRib* rib = FeaMeshMgr.GetCurrRib();
        if ( rib )
        {
            rib->m_AbsSweepFlag = true;
            rib->ComputeEndPoints();
            Update();
        }
    }
    else if ( w == m_FeaStructUI->ribSweepRelButton )
    {
        FeaRib* rib = FeaMeshMgr.GetCurrRib();
        if ( rib )
        {
            rib->m_AbsSweepFlag = false;
            rib->ComputeEndPoints();
            Update();
        }
    }
    else if ( w == m_FeaStructUI->trimRibButton )
    {
        FeaRib* rib = FeaMeshMgr.GetCurrRib();
        if ( rib )
        {
            if ( m_FeaStructUI->trimRibButton->value() )
            {
                rib->m_TrimFlag = true;
            }
            else
            {
                rib->m_TrimFlag = false;
            }
            rib->ComputeEndPoints();
        }
    }
    else if ( w == m_FeaStructUI->sparSweepAbsButton )
    {
        FeaSpar* spar = FeaMeshMgr.GetCurrSpar();
        if ( spar )
        {
            spar->m_AbsSweepFlag = true;
            spar->ComputeEndPoints();
            Update();
        }
    }
    else if ( w == m_FeaStructUI->sparSweepRelButton )
    {
        FeaSpar* spar = FeaMeshMgr.GetCurrSpar();
        if ( spar )
        {
            spar->m_AbsSweepFlag = false;
            spar->ComputeEndPoints();
            Update();
        }
    }
    else if ( w == m_FeaStructUI->trimSparButton )
    {
        FeaSpar* spar = FeaMeshMgr.GetCurrSpar();
        if ( spar )
        {
            if ( m_FeaStructUI->trimSparButton->value() )
            {
                spar->m_TrimFlag = true;
            }
            else
            {
                spar->m_TrimFlag = false;
            }
            spar->ComputeEndPoints();
        }
    }
    else if ( w == m_FeaStructUI->addSparButton )
    {
        FeaMeshMgr.AddSpar();
        Update();
    }
    else if ( w == m_FeaStructUI->delSparButton )
    {
        FeaMeshMgr.DelCurrSpar();
    }
    else if ( w == m_FeaStructUI->exportFEAMeshButton )
    {
        FeaMeshMgr.Build();
        FeaMeshMgr.Export();
        FeaMeshMgr.SetDrawMeshFlag( true );
        m_FeaStructUI->drawMeshButton->value( 1 );
    }
    else if ( w == m_FeaStructUI->computeFEAMeshButton )
    {
        FeaMeshMgr.Build();
        FeaMeshMgr.SetDrawMeshFlag( true );
        m_FeaStructUI->drawMeshButton->value( 1 );
    }
    else if ( w == m_FeaStructUI->drawMeshButton )
    {
        if ( m_FeaStructUI->drawMeshButton->value() )
        {
            FeaMeshMgr.SetDrawMeshFlag( true );
        }
        else
        {
            FeaMeshMgr.SetDrawMeshFlag( false );
        }
    }
    else if ( w == m_FeaStructUI->editTabs )
    {
        Fl_Widget* tw = m_FeaStructUI->editTabs->value();
        if ( tw == m_FeaStructUI->upperSkinGroup )
        {
            FeaMeshMgr.SetCurrEditType( FeaMeshMgrSingleton::UP_SKIN_EDIT );
        }
        else if ( tw == m_FeaStructUI->lowerSkinGroup  )
        {
            FeaMeshMgr.SetCurrEditType( FeaMeshMgrSingleton::LOW_SKIN_EDIT );
        }
        else if ( tw == m_FeaStructUI->ribGroup  )
        {
            FeaMeshMgr.SetCurrEditType( FeaMeshMgrSingleton::RIB_EDIT );
        }
        else if ( tw == m_FeaStructUI->sparGroup  )
        {
            FeaMeshMgr.SetCurrEditType( FeaMeshMgrSingleton::SPAR_EDIT );
        }
        else if ( tw == m_FeaStructUI->pointMassGroup  )
        {
            FeaMeshMgr.SetCurrEditType( FeaMeshMgrSingleton::POINT_MASS_EDIT );
        }

        Update();
    }
    //==== Upper Skin ====//
    else if ( w == m_FeaStructUI->upSkinExportNoButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrUpperSkin();
        if ( skin )
        {
            skin->SetExportFlag( false );
        }
        Update();
    }
    else if ( w == m_FeaStructUI->upSkinExportYesButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrUpperSkin();
        if ( skin )
        {
            skin->SetExportFlag( true );
        }
        Update();
    }
    else if ( w == m_FeaStructUI->upSkinAddSpliceLineButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrUpperSkin();
        if ( skin )
        {
            skin->AddSpliceLine();
            skin->ComputeSpliceLineEndPoints();
        }
        Update();
    }
    else if ( w == m_FeaStructUI->upSkinDelSpliceLineButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrUpperSkin();
        if ( skin )
        {
            skin->DelCurrSpliceLine();
        }
        Update();
    }
    else if ( w == m_FeaStructUI->upSpliceLineIDInput )     // Splice Line ID
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrUpperSkin();
        if ( skin )
        {
            int id = atoi( m_FeaStructUI->upSpliceLineIDInput->value() );
            skin->SetCurrSpliceLineID( id );
            Update();
        }
    }
    else if ( w == m_FeaStructUI->upSpliceLineIDDownButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrUpperSkin();
        if ( skin )
        {
            int id = skin->GetCurrSpliceLineID() - 1;
            skin->SetCurrSpliceLineID( id );
            Update();
        }
    }
    else if ( w == m_FeaStructUI->upSpliceLineIDUpButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrUpperSkin();
        if ( skin )
        {
            int id = skin->GetCurrSpliceLineID() + 1;
            skin->SetCurrSpliceLineID( id );
            Update();
        }
    }
    else if ( w == m_FeaStructUI->upSkinAddSpliceButton )
    {
        FeaSkin*  upskin = FeaMeshMgr.GetCurrUpperSkin();
        if ( upskin )
        {
            FeaSpliceLine* sl = upskin->GetCurrSpliceLine();
            if ( sl )
            {
                sl->SetMode( FeaSpliceLine::ADD_MODE );
            }
        }
    }
    else if ( w == m_FeaStructUI->upSkinDelSpliceButton )
    {
        if ( FeaMeshMgr.GetCurrUpperSkin() )
            if ( FeaMeshMgr.GetCurrUpperSkin()->GetCurrSpliceLine() )
            {
                FeaMeshMgr.GetCurrUpperSkin()->GetCurrSpliceLine()->DelEditSplice();
                Update();
            }
    }
    else if ( w == m_FeaStructUI->upSpliceIDLeftButton )
    {
        if ( FeaMeshMgr.GetCurrUpperSkin() )
            if ( FeaMeshMgr.GetCurrUpperSkin()->GetCurrSpliceLine() )
            {
                FeaMeshMgr.GetCurrUpperSkin()->GetCurrSpliceLine()->ChangeEditSplice( -1 );
                Update();
            }
    }
    else if ( w == m_FeaStructUI->upSpliceIDRightButton )
    {
        if ( FeaMeshMgr.GetCurrUpperSkin() )
            if ( FeaMeshMgr.GetCurrUpperSkin()->GetCurrSpliceLine() )
            {
                FeaMeshMgr.GetCurrUpperSkin()->GetCurrSpliceLine()->ChangeEditSplice( 1 );
                Update();
            }
    }
    //==== Lower Skin ====//
    else if ( w == m_FeaStructUI->lowSkinExportNoButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrLowerSkin();
        if ( skin )
        {
            skin->SetExportFlag( false );
        }
        Update();
    }
    else if ( w == m_FeaStructUI->lowSkinExportYesButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrLowerSkin();
        if ( skin )
        {
            skin->SetExportFlag( true );
        }
        Update();
    }
    else if ( w == m_FeaStructUI->lowSkinAddSpliceLineButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrLowerSkin();
        if ( skin )
        {
            skin->AddSpliceLine();
            skin->ComputeSpliceLineEndPoints();
        }
        Update();
    }
    else if ( w == m_FeaStructUI->lowSkinDelSpliceLineButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrLowerSkin();
        if ( skin )
        {
            skin->DelCurrSpliceLine();
        }
        Update();
    }
    else if ( w == m_FeaStructUI->lowSpliceLineIDInput )        // Splice Line ID
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrLowerSkin();
        if ( skin )
        {
            int id = atoi( m_FeaStructUI->lowSpliceLineIDInput->value() );
            skin->SetCurrSpliceLineID( id );
            Update();
        }
    }
    else if ( w == m_FeaStructUI->lowSpliceLineIDDownButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrLowerSkin();
        if ( skin )
        {
            int id = skin->GetCurrSpliceLineID() - 1;
            skin->SetCurrSpliceLineID( id );
            Update();
        }
    }
    else if ( w == m_FeaStructUI->lowSpliceLineIDUpButton )
    {
        FeaSkin* skin = FeaMeshMgr.GetCurrLowerSkin();
        if ( skin )
        {
            int id = skin->GetCurrSpliceLineID() + 1;
            skin->SetCurrSpliceLineID( id );
            Update();
        }
    }
    else if ( w == m_FeaStructUI->lowSkinAddSpliceButton )
    {
        FeaSkin*  skin = FeaMeshMgr.GetCurrLowerSkin();
        if ( skin )
        {
            FeaSpliceLine* sl = skin->GetCurrSpliceLine();
            if ( sl )
            {
                sl->SetMode( FeaSpliceLine::ADD_MODE );
            }
        }
    }
    else if ( w == m_FeaStructUI->lowSkinDelSpliceButton )
    {
        if ( FeaMeshMgr.GetCurrLowerSkin() )
            if ( FeaMeshMgr.GetCurrLowerSkin()->GetCurrSpliceLine() )
            {
                FeaMeshMgr.GetCurrLowerSkin()->GetCurrSpliceLine()->DelEditSplice();
                Update();
            }
    }
    else if ( w == m_FeaStructUI->lowSpliceIDLeftButton )
    {
        if ( FeaMeshMgr.GetCurrLowerSkin() )
            if ( FeaMeshMgr.GetCurrLowerSkin()->GetCurrSpliceLine() )
            {
                FeaMeshMgr.GetCurrLowerSkin()->GetCurrSpliceLine()->ChangeEditSplice( -1 );
                Update();
            }
    }
    else if ( w == m_FeaStructUI->lowSpliceIDRightButton )
    {
        if ( FeaMeshMgr.GetCurrLowerSkin() )
            if ( FeaMeshMgr.GetCurrLowerSkin()->GetCurrSpliceLine() )
            {
                FeaMeshMgr.GetCurrLowerSkin()->GetCurrSpliceLine()->ChangeEditSplice( 1 );
                Update();
            }
    }
    else if ( w == m_FeaStructUI->moveAttachPointButton )
    {
        int val = m_FeaStructUI->moveAttachPointButton->value();
        FeaMeshMgr.SetDrawAttachPointsFlag( !!val );
        Update();
    }
    else if ( w == m_FeaStructUI->addPtMassButton )
    {
        FeaMeshMgr.AddPointMass();
        Update();
    }
    else if ( w == m_FeaStructUI->delPtMassButton )
    {
        FeaMeshMgr.DelCurrPointMass();
        Update();
    }
    else if ( w == m_FeaStructUI->ptMassIDInput )
    {
        int id = atoi( m_FeaStructUI->ptMassIDInput->value() );
        FeaMeshMgr.SetCurrPointMassID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->ptMassIDUpButton )
    {
        int id = FeaMeshMgr.GetCurrPointMassID() + 1;
        FeaMeshMgr.SetCurrPointMassID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->ptMassIDDownButton )
    {
        int id = FeaMeshMgr.GetCurrPointMassID() - 1;
        FeaMeshMgr.SetCurrPointMassID( id );
        Update();
    }
    else if ( w == m_FeaStructUI->massButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Mass .dat file.", "*.dat" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.SetFeaExportFileName( newfile, FeaMeshMgrSingleton::MASS_FILE_NAME );
        }
        Update();
    }
    else if ( w == m_FeaStructUI->nastranButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select NASTRAN .dat file.", "*.dat" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.SetFeaExportFileName( newfile, FeaMeshMgrSingleton::NASTRAN_FILE_NAME );
        }
        Update();
    }
    else if ( w == m_FeaStructUI->geomButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Calculix Geom .dat file.", "*.dat" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.SetFeaExportFileName( newfile, FeaMeshMgrSingleton::GEOM_FILE_NAME );
        }
        Update();
    }
    else if ( w == m_FeaStructUI->thickButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select Calculix Thickness .dat file.", "*.dat" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.SetFeaExportFileName( newfile, FeaMeshMgrSingleton::THICK_FILE_NAME );
        }
        Update();
    }
    else if ( w == m_FeaStructUI->stlButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select .stl file.", "*.stl" );
        if ( newfile.compare( "" ) != 0 )
        {
            FeaMeshMgr.SetFeaExportFileName( newfile, FeaMeshMgrSingleton::STL_FEA_NAME );
        }
        Update();
    }

    m_ScreenMgr->SetUpdateFlag( true );

//  m_Vehicle->triggerDraw();

}

