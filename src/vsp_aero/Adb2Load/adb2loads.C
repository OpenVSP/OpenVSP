//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include "ADBSlicer.H"

int DoSlice            = 0;
int Interpolate        = 0;
int GnuPlot            = 0;
int NodeOffSet         = 0;
int ElementOffSet      = 0;
int AddLabel           = 0;
int ParseCalculixFile  = 0;
int CalculateOffSets   = 0;
int MergeCalculixFiles = 0;
int FindClosestNode    = 0;
int ParseCalculixFrd   = 0;
int CleanCalculixInip  = 0;
int CalculixOpt        = 0;
int ScalePressures     = 0;
int RenumberINPFile    = 0;

float DynamicPressure = 1.;
float BoundaryTolerance = 1.e-4;
float ScaleFactor = 1.;

float MaxStress = 12960000.;
float MinThick  = 0.01333333;
float MaxThick  = 0.05;
float OptFact   = 1.00;

float xyz_find[3];

char FileName_1[10000], FileName_2[10000], FileName_3[10000], Label[10000];
char OptElementListName[10000];

int main(int argc, char **argv);
void ParseInput(int argc, char *argv[]);

/*##############################################################################
#                                                                              #
#                                 viewer main                                  #
#                                                                              #
##############################################################################*/

int main(int argc, char **argv) {
   
    ADBSLICER Slicer;
    
    ParseInput(argc, argv);
    
    Slicer.GnuPlot() = GnuPlot;

    if ( DoSlice ) {
       
       Slicer.LoadFile(FileName_1);

       Slicer.SliceGeometry(FileName_1);
       
    }

    else if ( CalculateOffSets ) {
       
       Slicer.CalculateCalulixOffSets(FileName_2);
       
    }
        
    else if ( ParseCalculixFile ) {
       
       Slicer.ParseCalculixFile(FileName_2);
       
    }
    
    else if ( FindClosestNode ) {
       
       Slicer.FindNearestNodeInCalculixFile(FileName_2,xyz_find);
       
    }    
 
    else if ( MergeCalculixFiles ) {
       
       Slicer.MergeCalculixFiles(FileName_1, FileName_2, FileName_3);
       
    }
    
    else if ( CleanCalculixInip ) {
       
       Slicer.CleanCalculixInpFile(FileName_1, FileName_2);
       
    }
    
    else if ( ScalePressures ) {
       
       Slicer.ScaleCalulixInpPressureLoads(FileName_1, FileName_2, ScaleFactor);
        
    }
    
    else if ( CalculixOpt ) {
       
       Slicer.MinSkinThickness() = MinThick;
       Slicer.MaxSkinThickness() = MaxThick;
       Slicer.MaxStress()        = MaxStress;
       Slicer.OptFact()          = OptFact;
       
       Slicer.SetElementName(OptElementListName);
       
       Slicer.OptimizationCalculixInpFile(FileName_1, FileName_2, FileName_3);
       
    }
    
    else if ( Interpolate ) {
       
       printf("Loading VSP file... \n");fflush(NULL);
       
       Slicer.LoadFile(FileName_1);       
    
       Slicer.DynamicPressure() = DynamicPressure;
       
       Slicer.BoundaryTolerance() = BoundaryTolerance;
       
       Slicer.NodeOffSet() = NodeOffSet;
       
       Slicer.ElementOffSet() = ElementOffSet;
       
       if ( AddLabel > 0 ) {
          
          Slicer.AddLabel() = AddLabel;
          
          sprintf(Slicer.Label(),"%s",Label);
          
       }
       
       printf("Interpolating to FEM File... \n");fflush(NULL);

       Slicer.InterpolateSolutionToCalculix(FileName_2);
  
    }
    
    else if ( ParseCalculixFrd ) {
       
       Slicer.LoadCalculixData(FileName_1);
       
    }
    
    else if ( RenumberINPFile ) {
       
       Slicer.WriteOutRenumberedCalculixINPFile(FileName_1);
       
    }
    
    return 0;

}

/*##############################################################################
#                                                                              #
#                              ParseInput                                      #
#                                                                              #
##############################################################################*/

void ParseInput(int argc, char *argv[])
{

    int i;

    // Parse the input

    i = 1;

    while ( i <= argc - 2 ) {

       if ( strcmp(argv[i],"-slice") == 0 ) {
        
          DoSlice = 1;
          
          sprintf(FileName_1,"%s",argv[++i]);
          
       }  
       
       else if ( strcmp(argv[i],"-interp") == 0 ) {
        
          Interpolate = 1;
          
          sprintf(FileName_1,"%s",argv[++i]);

          sprintf(FileName_2,"%s",argv[++i]);
          
       }  

       else if ( strcmp(argv[i],"-getoffsets") == 0 ) {
        
          CalculateOffSets = 1;
          
          sprintf(FileName_2,"%s",argv[++i]);
          
       }  
       
       else if ( strcmp(argv[i],"-parse") == 0 ) {
        
          ParseCalculixFile = 1;
          
          sprintf(FileName_2,"%s",argv[++i]);
          
       }  
       
       else if ( strcmp(argv[i],"-merge") == 0 ) {
          
          MergeCalculixFiles = 1;
          
          sprintf(FileName_1,"%s",argv[++i]);

          sprintf(FileName_2,"%s",argv[++i]);

          sprintf(FileName_3,"%s",argv[++i]);
          
       }
       
       else if ( strcmp(argv[i],"-clean") == 0 ) {
          
          CleanCalculixInip = 1;
          
          sprintf(FileName_1,"%s",argv[++i]);

          sprintf(FileName_2,"%s",argv[++i]);
          
       }
       
       else if ( strcmp(argv[i],"-scalepressure") == 0 ) {
          
          ScalePressures = 1;
          
          ScaleFactor = atof(argv[++i]);
          
          sprintf(FileName_1,"%s",argv[++i]);

          sprintf(FileName_2,"%s",argv[++i]);
                    
       }

       else if ( strcmp(argv[i],"-maxstress") == 0 ) {

          MaxStress = atof(argv[++i]);
          
       }

       else if ( strcmp(argv[i],"-minthick") == 0 ) {

          MinThick = atof(argv[++i]);
          
       }
       
       else if ( strcmp(argv[i],"-maxthick") == 0 ) {

          MaxThick = atof(argv[++i]);
          
       }

       else if ( strcmp(argv[i],"-optfact") == 0 ) {

          OptFact = atof(argv[++i]);
          
       }

       else if ( strcmp(argv[i],"-elementlist") == 0 ) {

          sprintf(OptElementListName,"%s",argv[++i]);
   
       }       
                            
       else if ( strcmp(argv[i],"-opt") == 0 ) {
          
          CalculixOpt = 1;
          
          sprintf(FileName_1,"%s",argv[++i]);

          sprintf(FileName_2,"%s",argv[++i]);
          
          sprintf(FileName_3,"%s",argv[++i]);
                    
       }
       
       else if ( strcmp(argv[i],"-find") == 0 ) {
      
          FindClosestNode = 1;
          
          xyz_find[0] = atof(argv[++i]);
          xyz_find[1] = atof(argv[++i]);
          xyz_find[2] = atof(argv[++i]);

          sprintf(FileName_2,"%s",argv[++i]);
          
          printf("FileName_2: %s \n",FileName_2);

       }         
                        
       else if ( strcmp(argv[i],"-dynp") == 0 ) {
      
          DynamicPressure = atof(argv[++i]);

       }  
       
       else if ( strcmp(argv[i],"-boundarycutoff") == 0 ) {
      
          BoundaryTolerance = atof(argv[++i]);

       }         
       
       else if ( strcmp(argv[i],"-offsets") == 0 ) {
      
          NodeOffSet = atoi(argv[++i]);

          ElementOffSet = atoi(argv[++i]);

       }  

       else if ( strcmp(argv[i],"-label") == 0 ) {
      
          AddLabel = 1;
          
          sprintf(Label,"%s",argv[++i]);
          
       }  
       
       else if ( strcmp(argv[i],"-frd") == 0 ) {
       
          ParseCalculixFrd = 1;
          
          sprintf(FileName_1,"%s",argv[++i]);
          
       }
       
       else if ( strcmp(argv[i],"-renumber") == 0 ) {
          
          RenumberINPFile = 1;
        
          sprintf(FileName_1,"%s",argv[++i]);

       }
                                          
       else if ( strcmp(argv[i],"-gnu") == 0 ) {
        
          GnuPlot = 1;
          
       }  
       
       i++;    
       
    }
    
}
