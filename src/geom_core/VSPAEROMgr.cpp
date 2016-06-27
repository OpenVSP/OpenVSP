//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROMgr.cpp: VSPAERO Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "VSPAEROMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "StlHelper.h"
#include "APIDefines.h"
#include "WingGeom.h"

//==== Constructor ====//
VSPAEROMgrSingleton::VSPAEROMgrSingleton()
{
    m_Name = "VSPAEROSettings";

    m_DegenGeomSet.Init( "GeomSet", "VSPAERO", this, 0, 0, 12 );
    m_DegenGeomSet.SetDescript( "Geometry set" );

    m_Sref.Init( "Sref", "VSPAERO", this, 100.0, 0.0, 1e12 );
    m_Sref.SetDescript( "Reference area" );

    m_bref.Init( "bref", "VSPAERO", this, 1.0, 0.0, 1e6 );
    m_bref.SetDescript( "Reference span" );

    m_cref.Init( "cref", "VSPAERO", this, 1.0, 0.0, 1e6 );
    m_cref.SetDescript( "Reference chord" );

    m_RefFlag.Init( "RefFlag", "VSPAERO", this, MANUAL_REF, MANUAL_REF, COMPONENT_REF );
    m_RefFlag.SetDescript( "Reference quantity flag" );

    m_CGGeomSet.Init( "MassSet", "VSPAERO", this, 0, 0, 12 );
    m_CGGeomSet.SetDescript( "Mass property set" );

    m_NumMassSlice.Init( "NumMassSlice", "VSPAERO", this, 10, 10, 200 );
    m_NumMassSlice.SetDescript( "Number of mass property slices" );

    m_Xcg.Init( "Xcg", "VSPAERO", this, 0.0, -1.0e12, 1.0e12 );
    m_Xcg.SetDescript( "X Center of Gravity" );

    m_Ycg.Init( "Ycg", "VSPAERO", this, 0.0, -1.0e12, 1.0e12 );
    m_Ycg.SetDescript( "Y Center of Gravity" );

    m_Zcg.Init( "Zcg", "VSPAERO", this, 0.0, -1.0e12, 1.0e12 );
    m_Zcg.SetDescript( "Z Center of Gravity" );

    m_Alpha.Init( "Alpha", "VSPAERO", this, 5.0, -180, 180 );
    m_Alpha.SetDescript( "Angle of attack" );

    m_Beta.Init( "Beta", "VSPAERO", this, 0.0, -180, 180 );
    m_Beta.SetDescript( "Angle of sideslip" );

    m_Mach.Init( "Mach", "VSPAERO", this, 0.3, 0.0, 5.0 );
    m_Mach.SetDescript( "Freestream Mach number" );


    m_NCPU.Init( "NCPU", "VSPAERO", this, 2, 1, 255 );
    m_NCPU.SetDescript( "Number of processors to use" );
}

void VSPAEROMgrSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}


xmlNodePtr VSPAEROMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr VSPAEROsetnode = xmlNewChild( node, NULL, BAD_CAST"VSPAEROSettings", NULL );

    ParmContainer::EncodeXml( VSPAEROsetnode );

    XmlUtil::AddStringNode( VSPAEROsetnode, "ReferenceGeomID", m_RefGeomID );

    return VSPAEROsetnode;
}

xmlNodePtr VSPAEROMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr VSPAEROsetnode = XmlUtil::GetNode( node, "VSPAEROSettings", 0 );
    if ( VSPAEROsetnode )
    {
        ParmContainer::DecodeXml( VSPAEROsetnode );
        m_RefGeomID   = XmlUtil::FindString( VSPAEROsetnode, "ReferenceGeomID", m_RefGeomID );
    }

    return VSPAEROsetnode;
}


void VSPAEROMgrSingleton::Update()
{
    if( m_RefFlag() == MANUAL_REF )
    {
        m_Sref.Activate();
        m_bref.Activate();
        m_cref.Activate();
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
                m_bref.Set( refwing->m_TotalSpan() );
                m_cref.Set( refwing->m_TotalChord() );

                m_Sref.Deactivate();
                m_bref.Deactivate();
                m_cref.Deactivate();
            }
        }
        else
        {
            m_RefGeomID = string();
        }
    }


}

std::string VSPAEROMgrSingleton::ReplaceAddNameValue(std::string contents, std::string name, std::string value_str)
{
    std::string new_contents = contents;
    std::regex replacement_expression(name + " = (.*)");
    std::smatch match;
    std::string replacement_string = name + " = " + value_str + " ";
    if (std::regex_search(contents, match, replacement_expression) && match.size() > 1) 
    {
        new_contents = std::regex_replace (contents,replacement_expression,replacement_string);
    } 
    else 
    {
        //TODO throw error because a match wasn't found
    }
    //std::cout<<new_contents;
    return new_contents;
}
// helper thread functions for VSPAERO GUI interface and multi-threaded impleentation
bool VSPAEROMgrSingleton::IsSolverRunning()
{
    return m_SolverProcess.IsRunning();
}
void VSPAEROMgrSingleton::KillSolver()
{
    return m_SolverProcess.Kill();
}
ProcessUtil* VSPAEROMgrSingleton::GetSolverProcess()
{
    return &m_SolverProcess;
}

// function is used to wait for the result to show up on the file system
void VSPAEROMgrSingleton::WaitForFile(string filename)
{
    // Wait until the results show up on the file system
    int n_wait = 0;
    // wait no more than 5 seconds = (50*100)/1000
    while ( (!FileExist(filename)) & (n_wait<50) )
    {
        n_wait++;
        SleepForMilliseconds(100);
    }
    SleepForMilliseconds(100);    //additional wait for file
}
/*******************************************************
Read .HISTORY file output from VSPAERO
See: VSP_Solver.C in vspaero project
line 4351 - void VSP_SOLVER::OutputStatusFile(int Type)
line 4407 - void VSP_SOLVER::OutputZeroLiftDragToStatusFile(void)
*******************************************************/
string VSPAEROMgrSingleton::ReadHistoryFile()
{
    string res_id;
    //TODO return success or failure
    FILE *fp = NULL;
    size_t result;
    bool read_success = false;

    //Read setup file to get number of wakeiterations
    fp = fopen( m_SetupFile.c_str(), "r" );
      if (fp==NULL) 
    {
        fputs ("VSPAEROMgrSingleton::ReadHistoryFile() - File open error\n",stderr);
    }
    else
    {
        // find the 'WakeIters' token
        char key[] = "WakeIters";
        char param[30];
        float param_val=0;
        int n_wakeiters = -1;
        bool found = false;
        while (!feof(fp) & !found)
        {
            if( fscanf(fp,"%s = %f\n",param,&param_val)==2 )
            {
                if( strcmp(key,param)==0 )
                {
                    n_wakeiters = (int)param_val;
                    found = true;
                }
            }
        } 
        fclose(fp);

        //HISTORY file
        WaitForFile(m_HistoryFile);
        fp = fopen( m_HistoryFile.c_str(), "r" );
          if (fp==NULL) 
        {
            fputs ("VSPAEROMgrSingleton::ReadHistoryFile() - File open error\n",stderr);
        }
        else
        {
            // Read header line - we don't ever use this it's just a way to move the file pointer
            // TODO - use the fields in the header string as the parameter names in the results manager
            char headerstr [256];
            fgets(headerstr,255,fp);
            // split headerstr into fieldnames
            std::vector<string>fieldnames;
            int n_fields=0;
            char * pch;
            pch = strtok (headerstr," ");
            while (pch != NULL)
            {
                n_fields++;
                fieldnames.push_back(pch);
                pch = strtok (NULL, " ");
            }
        

            // Read wake iter data
            std::vector<int> i;                 i.assign(n_wakeiters,0);
            std::vector<double> Mach;           Mach.assign(n_wakeiters,0);
            std::vector<double> Alpha;          Alpha.assign(n_wakeiters,0);
            std::vector<double> Beta;           Beta.assign(n_wakeiters,0);
            std::vector<double> CL;             CL.assign(n_wakeiters,0);
            std::vector<double> CDo;            CDo.assign(n_wakeiters,0);
            std::vector<double> CDi;            CDi.assign(n_wakeiters,0);
            std::vector<double> CDtot;          CDtot.assign(n_wakeiters,0);
            std::vector<double> CS;             CS.assign(n_wakeiters,0);
            std::vector<double> CFx;            CFx.assign(n_wakeiters,0);
            std::vector<double> CFy;            CFy.assign(n_wakeiters,0);
            std::vector<double> CFz;            CFz.assign(n_wakeiters,0);
            std::vector<double> CMx;            CMx.assign(n_wakeiters,0);
            std::vector<double> CMy;            CMy.assign(n_wakeiters,0);
            std::vector<double> CMz;            CMz.assign(n_wakeiters,0);
            std::vector<double> LoD;            LoD.assign(n_wakeiters,0);
            std::vector<double> E;              E.assign(n_wakeiters,0);
            std::vector<double> ToQS;           ToQS.assign(n_wakeiters,0);

            // Read in all of the wake data first before adding to the results manager
            for( int i_wake=0; i_wake<n_wakeiters; i_wake++ )
            {
                //TODO - add results to results manager based on header string
                //for (int i_field=0; i_field<n_fields; i_field++)
                //{}
                result = fscanf(fp,"%d",&i[i_wake]);
                result = fscanf(fp,"%lf",&Mach[i_wake]);
                result = fscanf(fp,"%lf",&Alpha[i_wake]);
                result = fscanf(fp,"%lf",&Beta[i_wake]);
                result = fscanf(fp,"%lf",&CL[i_wake]);
                result = fscanf(fp,"%lf",&CDo[i_wake]);
                result = fscanf(fp,"%lf",&CDi[i_wake]);
                result = fscanf(fp,"%lf",&CDtot[i_wake]);
                result = fscanf(fp,"%lf",&CS[i_wake]);
                result = fscanf(fp,"%lf",&LoD[i_wake]);
                result = fscanf(fp,"%lf",&E[i_wake]);
                result = fscanf(fp,"%lf",&CFx[i_wake]);
                result = fscanf(fp,"%lf",&CFy[i_wake]);
                result = fscanf(fp,"%lf",&CFz[i_wake]);
                result = fscanf(fp,"%lf",&CMx[i_wake]);
                result = fscanf(fp,"%lf",&CMy[i_wake]);
                result = fscanf(fp,"%lf",&CMz[i_wake]);
                result = fscanf(fp,"%lf",&ToQS[i_wake]);
            }
            fclose (fp);

            //add to the results manager
            Results* res = ResultsMgr.CreateResults( "VSPAERO_History" );
            res->Add( NameValData( "WakeIter", i ));
            res->Add( NameValData( "Mach", Mach ));
            res->Add( NameValData( "Alpha", Alpha ));
            res->Add( NameValData( "Beta", Beta ));
            res->Add( NameValData( "CL", CL ));
            res->Add( NameValData( "CDo", CDo ));
            res->Add( NameValData( "CDi", CDi ));
            res->Add( NameValData( "CDtot", CDtot ));
            res->Add( NameValData( "CS", CS ));
            res->Add( NameValData( "L/D", LoD ));
            res->Add( NameValData( "E", E ));
            res->Add( NameValData( "CFx", CFx ));
            res->Add( NameValData( "CFy", CFy ));
            res->Add( NameValData( "CFz", CFz ));
            res->Add( NameValData( "CMx", CMx ));
            res->Add( NameValData( "CMy", CMy ));
            res->Add( NameValData( "CMz", CMz ));
            res->Add( NameValData( "T/QS", ToQS ));

            res_id = res->GetID();
        }
    }    

    return res_id;
}

