//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_DegenGeom.H"

/*##############################################################################
#                                                                              #
#                         VSP_DEGEN_GEOM constructor                           #
#                                                                              #
##############################################################################*/

VSP_DEGEN_GEOM::VSP_DEGEN_GEOM(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                             VSP_DEGEN_GEOM init                              #
#                                                                              #
##############################################################################*/

void VSP_DEGEN_GEOM::init(void)
{

    Verbose_ = 1;
    
}

/*##############################################################################
#                                                                              #
#                            VSP_DEGEN_GEOM Copy                               #
#                                                                              #
##############################################################################*/

VSP_DEGEN_GEOM::VSP_DEGEN_GEOM(const VSP_DEGEN_GEOM &VSP_DegenGeom)
{

    init();

    // Just * use the operator = code

    *this = VSP_DegenGeom;

}

/*##############################################################################
#                                                                              #
#                           VSP_DEGEN_GEOM operator=                           #
#                                                                              #
##############################################################################*/

VSP_DEGEN_GEOM& VSP_DEGEN_GEOM::operator=(const VSP_DEGEN_GEOM &VSP_DegenGeom)
{

    return *this;

}

/*##############################################################################
#                                                                              #
#                        VSP_DEGEN_GEOM destructor                             #
#                                                                              #
##############################################################################*/

VSP_DEGEN_GEOM::~VSP_DEGEN_GEOM(void)
{


}

/*##############################################################################
#                                                                              #
#                         VSP_DEGEN_GEOM ReadFile                              #
#                                                                              #
##############################################################################*/

void VSP_DEGEN_GEOM::ReadFile(char *FileName)
{
 
    int i, j, NumI, NumJ, Wing, Body, Done, Slice, NumberOfBodySets, BodySet;
    double x, y, z, u, v, Nx, Ny, Nz, DumFloat;
    char VSP_File_Name[2000], DumChar[2000], Stuff[2000], Type[2000], Name[2000];
    FILE *VSP_Degen_File, *MeshFile;
    
    // Open setup file

    sprintf(VSP_File_Name,"%s.csv",FileName);

    if ( (VSP_Degen_File = fopen(VSP_File_Name,"r")) == NULL ) {

       // No VSP degen file... exit

       printf("Could not load %s VSP Degen Geometry file... \n", VSP_File_Name);fflush(NULL);

       exit(1);

    }    
    
    // Now read in the data
    
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"%d\n",&NumberOfComponents_);
    
    if ( Verbose_ ) printf("NumberOfComponents: %d \n",NumberOfComponents_);
    
    // Now scan the file and determine how many wings
    
    NumberOfWings_ = 0;
    
    Done = 0;
    
    while ( !Done ) {
     
       if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
       
       if ( strncmp(DumChar,"LIFTING_SURFACE",15) == 0 ) NumberOfWings_++;
       
    }
    
    if ( Verbose_ ) printf("NumberOfWings_: %d \n",NumberOfWings_);
    
    VSP_Wing_ = new VSP_WING[NumberOfWings_ + 1];
    
    rewind(VSP_Degen_File);
    
    
    // Now scan the file and determine how many bodies
    
    NumberOfBodies_ = 0;
    
    Done = 0;
    
    while ( !Done ) {
     
       if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
       
       if ( strncmp(DumChar,"BODY",4) == 0 ) NumberOfBodies_++;
       
    }    
    
    // Split body data into 2 sets... vertical, and horizontal slices
    
    NumberOfBodySets = NumberOfBodies_;
    
    NumberOfBodies_ *= 2;

    if ( Verbose_ ) printf("NumberOfBodies_: %d \n",NumberOfBodies_);

    VSP_Body_ = new VSP_BODY[NumberOfBodies_ + 1];

    rewind(VSP_Degen_File);
            
    // Read in the wing data
    
    for ( Wing = 1 ; Wing <= NumberOfWings_ ; Wing++ ) {
    
       Done = 0;
       
       while ( !Done ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"LIFTING_SURFACE",15) == 0 ) Done = 1;
          
       }

       sscanf(DumChar,"%15s,%s",Type,Name);

       if ( Verbose_ ) printf("Working on reading wing: %d --> %s \n",Wing,Name);
  
       VSP_Wing(Wing).ReadFile(Name,VSP_Degen_File);
       
    }
        
    // Read in the body data
    
    Body = 0;
 
    for ( BodySet = 1 ; BodySet <= NumberOfBodySets ; BodySet++ ) {

       // Load in the vertical slices
    
       rewind(VSP_Degen_File);
    
       Body++;
            
       Done = 0;
       
       while ( Done < BodySet ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) Done++;
                    
       }

       sscanf(DumChar,"%4s,%s",Type,Name);

       if ( Verbose_ ) printf("Working on reading body: %d --> %s \n",Body,Name);
              
       VSP_Body(Body).ReadFile(Name,1,VSP_Degen_File);

       // Load in the horizontal slices
       
       rewind(VSP_Degen_File);
     
       Body++;
            
       Done = 0;
       
       while ( Done < BodySet ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) Done++;
                    
       }
       
       sscanf(DumChar,"%4s,%s",Type,Name);

       if ( Verbose_ ) printf("Working on reading body: %d --> %s \n",Body,Name);
              
       VSP_Body(Body).ReadFile(Name,2,VSP_Degen_File);     
       
    }
    
    fclose(VSP_Degen_File);
    
    // Create meshes for the VSP geometries
    
    MeshGeom();
    fflush(NULL);
}

/*##############################################################################
#                                                                              #
#                         VSP_DEGEN_GEOM MeshGeom                              #
#                                                                              #
##############################################################################*/

void VSP_DEGEN_GEOM::MeshGeom(void)
{
 
    int Wing, Body;
    char DumChar[2000];
    
    // Loop over the wings and create a mesh

    for ( Wing = 1 ; Wing <= NumberOfWings_ ; Wing++ ) {

       VSP_Wing(Wing).CreateMesh(Wing);

//       sprintf(DumChar,"Wing.%d.msh",Wing);
       
//       VSP_Wing(Wing).Grid().WriteMesh(DumChar);
       
    }   
    
    // Loop over the bodies and create a mesh

    printf("Body meshing... \n");fflush(NULL);
    
    for ( Body = 1 ; Body <= NumberOfBodies_ ; Body++ ) {

       VSP_Body(Body).CreateMesh(Body);

//       sprintf(DumChar,"Body.%d.msh",Body);
       
//       VSP_Body(Body).Grid().WriteMesh(DumChar);
       
    }   
     
}

/*##############################################################################
#                                                                              #
#                         VSP_DEGEN_GEOM WriteMeshFile                         #
#                                                                              #
##############################################################################*/

void VSP_DEGEN_GEOM::WriteMeshFile(char *FileName)
{

    FILE *adb_file_;
    char adb_file_name[2000], DumChar[2000];
    int i, j, Node1, Node2, Node3, NumberOfTrailingVortexEdges, NumberOfRotors;
    int m, i_size, c_size, f_size, DumInt, number_of_nodes, number_of_tris;
    int SurfaceType, SurfaceID, TotalNodes, MaxLevels;
    float Mach, AngleOfAttack, AngleOfBeta, Sref, Cref, Bref, X_cg, Y_cg, Z_cg, Area;

    int num_Mach = 1;
    int num_Alpha = 1;
    
    float Sref_ = 1.;
    float Cref_ = 1.;
    float Bref_ = 1.;
    float X_cg_ = 0.;
    float Y_cg_ = 0.;
    float Z_cg_ = 0.;
       
    int stagnation_tri               = 1;
    
    float FreeStreamPressure         = 1000;
    float DynamicPressure            = 100000.;

    float DumFloat;
    
    float Cp;

    // Sizeof int and float

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    sprintf(adb_file_name,"%s.adb",FileName);
    
    if ( (adb_file_ = fopen(adb_file_name, "wb")) == NULL ) {

       printf("Could not open the aerothermal data base file for binary output! \n");fflush(NULL);

       exit(1);

    }
    
    // Write out coded id to allow us to determine endiannes of files

    DumInt = -123789456; // Version 1 of the ADB file

    fwrite(&DumInt, i_size, 1, adb_file_);

    // Write out header to aerothermal file

    number_of_tris = number_of_nodes = 0;
    
    for ( i = 1 ; i <= NumberOfWings_ ; i++ ) { 

       number_of_tris += VSP_Wing(i).Grid().NumberOfTris();
       number_of_nodes += VSP_Wing(i).Grid().NumberOfNodes();
       
    }
    
    for ( i = 1 ; i <= NumberOfBodies_ ; i++ ) { 

       number_of_tris += VSP_Body(i).Grid().NumberOfTris();
       number_of_nodes += VSP_Body(i).Grid().NumberOfNodes();
       
    }    
    
    printf("number_of_nodes: %d \n",number_of_nodes);

    fwrite(&number_of_nodes, i_size, 1, adb_file_);
    fwrite(&number_of_tris,  i_size, 1, adb_file_);
    fwrite(&num_Mach,        i_size, 1, adb_file_);
    fwrite(&num_Alpha,       i_size, 1, adb_file_);
    fwrite(&Sref,           f_size, 1, adb_file_);
    fwrite(&Cref,           f_size, 1, adb_file_);
    fwrite(&Bref,           f_size, 1, adb_file_);
    fwrite(&X_cg,           f_size, 1, adb_file_);
    fwrite(&Y_cg,           f_size, 1, adb_file_);
    fwrite(&Z_cg,           f_size, 1, adb_file_);

    Mach = 0.;

    fwrite(&Mach, f_size, 1, adb_file_);

    AngleOfAttack = 0.;

    fwrite(&AngleOfAttack, f_size, 1, adb_file_);

    AngleOfBeta = 0.;

    fwrite(&AngleOfBeta, f_size, 1, adb_file_);
    
    // Write out wing ID flags, names...

    fwrite(&NumberOfWings_, i_size, 1, adb_file_);
    
    for ( i = 1 ; i <= NumberOfWings_ ; i++ ) { 
     
       fwrite(&(i), i_size, 1, adb_file_);
 
       sprintf(DumChar,"%s",VSP_Wing(i).ComponentName());
       
       fwrite(DumChar, c_size, 100, adb_file_);
     
    }
    
    // Write out body ID flags, names...
 
    fwrite(&NumberOfBodies_, i_size, 1, adb_file_);
    
    for ( i = 1 ; i <= NumberOfBodies_ ; i++ ) { 
     
       fwrite(&(i), i_size, 1, adb_file_);
       
       sprintf(DumChar,"%s",VSP_Body(i).ComponentName());

       fwrite(DumChar, c_size, 100, adb_file_);
     
    } 
    
    // Write out geometry and surface data to aerothermal file

    TotalNodes = 0;
    
    // Wings
    
    for ( i = 1 ; i <= NumberOfWings_ ; i++ ) { 

       for ( j = 1 ; j <= VSP_Wing(i).Grid().NumberOfTris() ; j++ ) {

          Node1 = VSP_Wing(i).Grid().TriList(j).Node1() + TotalNodes;
          Node2 = VSP_Wing(i).Grid().TriList(j).Node2() + TotalNodes;
          Node3 = VSP_Wing(i).Grid().TriList(j).Node3() + TotalNodes;
          
          SurfaceType = WING_SURFACE;
          SurfaceID   = i;
          
          Area = VSP_Wing(i).Grid().TriList(j).Area();
          
          fwrite(&(Node1),       i_size, 1, adb_file_);
          fwrite(&(Node2),       i_size, 1, adb_file_);
          fwrite(&(Node3),       i_size, 1, adb_file_);
          fwrite(&(SurfaceType), i_size, 1, adb_file_);
          fwrite(&(SurfaceID),   i_size, 1, adb_file_);
          fwrite(&Area,          f_size, 1, adb_file_);

       }
       
       TotalNodes +=  VSP_Wing(i).Grid().NumberOfNodes();
       
    }
    
    // Bodies

    for ( i = 1 ; i <= NumberOfBodies_ ; i++ ) { 

       for ( j = 1 ; j <= VSP_Body(i).Grid().NumberOfTris() ; j++ ) {

          Node1 = VSP_Body(i).Grid().TriList(j).Node1() + TotalNodes;
          Node2 = VSP_Body(i).Grid().TriList(j).Node2() + TotalNodes;
          Node3 = VSP_Body(i).Grid().TriList(j).Node3() + TotalNodes;
          
          SurfaceType = BODY_SURFACE;
          SurfaceID   = i;
          
          Area = VSP_Wing(i).Grid().TriList(j).Area();
          
          fwrite(&(Node1),       i_size, 1, adb_file_);
          fwrite(&(Node2),       i_size, 1, adb_file_);
          fwrite(&(Node3),       i_size, 1, adb_file_);
          fwrite(&(SurfaceType), i_size, 1, adb_file_);
          fwrite(&(SurfaceID),   i_size, 1, adb_file_);
          fwrite(&Area,          f_size, 1, adb_file_);

       }
       
       TotalNodes +=  VSP_Body(i).Grid().NumberOfNodes();
       
    }    
    
    printf("Total Grid Nodes: %d \n",TotalNodes);

    float x, y, z;
    
    TotalNodes = 0;
    
    // Wings
    
    for ( i = 1 ; i <= NumberOfWings_ ; i++ ) { 

       for ( j = 1 ; j <=VSP_Wing(i).Grid().NumberOfNodes() ; j++ ) {

          x = VSP_Wing(i).Grid().NodeList(j).x();
          y = VSP_Wing(i).Grid().NodeList(j).y();
          z = VSP_Wing(i).Grid().NodeList(j).z();
          
          fwrite(&(x), f_size, 1, adb_file_);
          fwrite(&(y), f_size, 1, adb_file_);
          fwrite(&(z), f_size, 1, adb_file_);
          
       }
       
       TotalNodes += VSP_Wing(i).Grid().NumberOfNodes();
       
    }
    
    // Bodies
    
    for ( i = 1 ; i <= NumberOfBodies_ ; i++ ) { 

       for ( j = 1 ; j <= VSP_Body(i).Grid().NumberOfNodes() ; j++ ) {

          x = VSP_Body(i).Grid().NodeList(j).x();
          y = VSP_Body(i).Grid().NodeList(j).y();
          z = VSP_Body(i).Grid().NodeList(j).z();
          
          fwrite(&(x), f_size, 1, adb_file_);
          fwrite(&(y), f_size, 1, adb_file_);
          fwrite(&(z), f_size, 1, adb_file_);
          
       }
       
       TotalNodes += VSP_Body(i).Grid().NumberOfNodes();
       
    }    
    
    printf("Total Surface Nodes: %d \n",TotalNodes);

    // Write out aerothermal data base file
 
    fwrite(&(FreeStreamPressure), f_size, 1, adb_file_); // Freestream static pressure
    fwrite(&(DynamicPressure),    f_size, 1, adb_file_); // Freestream dynamic pressure
    
    // Wings
    
    for ( i = 1 ; i <= NumberOfWings_ ; i++ ) { 

       for ( j = 1 ; j <= VSP_Wing(i).Grid().NumberOfTris() ; j++ ) {

          // Write out the data base
            
          Cp = 0.;

          fwrite(&Cp, f_size, 1, adb_file_); // Wall or Edge Pressure, Pa
 
       }
       
    }
    
    // Bodies
    
    for ( i = 1 ; i <= NumberOfBodies_ ; i++ ) { 

       for ( j = 1 ; j <= VSP_Body(i).Grid().NumberOfTris() ; j++ ) {

          Cp = 0.;

          fwrite(&Cp, f_size, 1, adb_file_); // Wall or Edge Pressure, Pa

       }

    }      
    
    // Write out wake shape
    
    NumberOfTrailingVortexEdges = 0;
    
    fwrite(&NumberOfTrailingVortexEdges, i_size, 1, adb_file_);   
    
    // Write out the rotor data
    
    NumberOfRotors = 0;
    
    fwrite(&NumberOfRotors, i_size, 1, adb_file_);
    
    /*
    for ( i = 1 ; i <= NumberOfRotors_ ; i++ ) {
     
       RotorDisk(i).Write_Binary_STP_Data(adb_file_);
     
    }
   
   */
    
   MaxLevels = 1;
   
   fwrite(&MaxLevels, i_size, 1, adb_file_); 

    
    fclose(adb_file_);
    
    printf("Total surface tris: %d \n",number_of_tris);
    fflush(NULL);
}


