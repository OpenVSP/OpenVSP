//
//  geom_degen.h
//  VSP
//
//  Created by Joel Belben on 8/5/12.
//

#ifndef VSP_geom_degen_h
#define VSP_geom_degen_h

#include "Defines.h"
#include <vector>
#include "Vec3d.h"
#include "Vec2d.h"
#include "Matrix4d.h"
#include "SubSurface.h"
#include "ResultsMgr.h"

using namespace std;

#ifdef _MSC_VER
#ifndef NAN
#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)
#endif
#endif

class Geom;

typedef struct
{
    vector< vector< vec3d > >   x;          //!
    vector< vector< vec3d > >   nvec;       //!
    vector< vector< double > >  area;       //!
    vector< vector< double > >  u;          //!
    vector< vector< double > >  w;          //!
} DegenSurface;

typedef struct
{
    vector< vector< vec3d > >   x;          //!
    vector< vector< double > >  zcamber;    //!
    vector< vector< vec3d > >   nCamber;    //!
    vector< vector< double > >  t;          //!
    vector< vec3d >             nPlate;     //!
    vector< vector< double > >  u;          //!
    vector< vector< double > >  wTop;       //!
    vector< vector< double > >  wBot;       //!
    vector< vector< vec3d > >   xCamber;    //!
} DegenPlate;

typedef struct
{
    vector< vec3d >             xle;        //!
    vector< vec3d >             xte;        //!
    vector< double >            toc;        //!
    vector< double >            tLoc;       //!
    vector< double >            chord;      //!
    vector< double >            sweeple;    //!
    vector< double >            sweepte;    //!
    vector< vector< double > >  transmat;   //!
    vector< vector< double > >  invtransmat;//!
    vector< vector< double > >  Ishell;     //!
    vector< vector< double > >  Isolid;     //!
    vector< vec3d >             xcgSolid;   //!
    vector< vec3d >             xcgShell;   //!
    vector< double >            sectarea;   //!
    vector< vec3d >             sectnvec;   //!
    vector< double >            perimTop;   //!
    vector< double >            perimBot;   //!
    vector< double >            areaTop;    //!
    vector< double >            areaBot;    //!
    vector< double >            u;          //!
    vector< double >            toc2;       //!
    vector< double >            tLoc2;      //!
    vector< double >            anglele;    //!
    vector< double >            anglete;    //!
    vector< double >            radleTop;   //!
    vector< double >            radleBot;   //!
} DegenStick;

typedef struct
{
    vector< double >            vol;        //!
    vector< double >            volWet;     //!
    vector< double >            area;       //!
    vector< double >            areaWet;    //!
    vector< vector< double > >  Ishell;     //! Multiply by rho*t to get inertias
    vector< vector< double > >  Isolid;     //! Multiply by rho to get inertias
    vector< vec3d >             xcgShell;   //!
    vector< vec3d >             xcgSolid;   //!
} DegenPoint;

typedef struct
{
    string                      name;       //! name displayed in the geom GUI
    string                      fullName;   //! fullName identifying parent geometry and parent surface index
    int                         testType;   //! "test" for subsurfaces to define which side of the boundary the subsurface is on (commonly used for LINE subsurface types)
    string                      typeName;   //! used to identify the design intent of the surface (see vsp::SUBSURF_TYPE in APIDefines.h and std::string SubSurface::GetTypeName( int type ) for more info)
    vsp::SUBSURF_TYPE           typeId;     //! enumeration for the typeName; vsp::SUBSURF_TYPE
    vector< double >            u;          //!
    vector< double >            w;          //!
    vector< vec3d >             x;          //!
} DegenSubSurf;

typedef struct
{
    string                      name;
    vector < double >           uStart;
    vector < double >           uEnd;
    vector < double >           wStart;
    vector < double >           wEnd;
    vector < vec3d >            xStart;
    vector < vec3d >            xEnd;
} DegenHingeLine;

typedef struct
{
    double                      d;
    vec3d                       x;
    vec3d                       nvec;
} DegenDisk;

typedef struct
{
    string                      name;
    double                      mass;
    vec3d                       x;
    string                      geom_id;
} DegenPtMass;

class DegenGeom
{
public:
    enum GEOM_PLANE { XY_PLANE, XZ_PLANE, YZ_PLANE };

    enum DEGEN_GEOM_TYPE { SURFACE_TYPE, BODY_TYPE, DISK_TYPE, MESH_TYPE };

    DegenGeom()
    {
        type = BODY_TYPE;
        num_pnts = 0;
        num_xsecs = 0;
        parentGeom = NULL;
        surfNum = -1;
        flip_normal = false;
        main_surf_ind = 0;
        sym_copy_ind = 0;
    };
    virtual ~DegenGeom() {};

    DegenSurface getDegenSurf()
    {
        return degenSurface;
    }
    vector < DegenPlate > getDegenPlates()
    {
        return degenPlates;
    }
    vector < DegenSubSurf > getDegenSubSurfs()
    {
        return degenSubSurfs;
    }

    DegenPoint   getDegenPoint()
    {
        return degenPoint;
    }
    DegenDisk   getDegenDisk()
    {
        return degenDisk;
    }
    vector <DegenStick> getDegenSticks()
    {
        return degenSticks;
    }

    void setDegenPoint( const DegenPoint & degenPoint )
    {
        this->degenPoint = degenPoint;
    }
    void setDegenDisk( DegenDisk degenDisk )
    {
        this->degenDisk = degenDisk;
    }

    int getNumXSecs()
    {
        return num_xsecs;
    };
    int getNumPnts()
    {
        return num_pnts;
    };

    void setNumXSecs( const int &nxss )
    {
        num_xsecs = nxss;
    }
    void setNumPnts( const int &npts )
    {
        num_pnts = npts;
    }

    void setName( const string & namein )
    {
        name = namein;
    }
    string getName()
    {
        return name;
    }

    Geom* getParentGeom()
    {
        return parentGeom;
    }
    void  setParentGeom( Geom* geom )
    {
        parentGeom = geom;
    }

    int getType()
    {
        return type;
    }
    void setType( int geomType )
    {
        type = geomType;
    }

    int getCfdSurfType()
    {
        return cfdsurftype;
    }
    void setCfdSurfType( int t )
    {
        cfdsurftype = t;
    }

    int getSurfNum()
    {
        return surfNum;
    }
    void setSurfNum( int n )
    {
        surfNum = n;
    }

    bool getFlipNormal()
    {
        return flip_normal;
    }
    void setFlipNormal( bool flag )
    {
        flip_normal = flag;
    }

    int getMainSurfInd()
    {
        return main_surf_ind;
    }
    void setMainSurfInd( int n )
    {
        main_surf_ind = n;
    }

    int getSymCopyInd()
    {
        return sym_copy_ind;
    }
    void setSymCopyInd( int n )
    {
        sym_copy_ind = n;
    }

    vector < double > getTransMat()
    {
        return transmat;
    }
    void setTransMat( const vector < double > & M )
    {
        transmat = M;
    }

    static void build_trans_mat( vec3d x, vec3d y, vec3d z, const vec3d &p, Matrix4d &mat, Matrix4d &invmat );
    void build_basis( const int &startPnt, const vector < vec3d > &sect, vec3d &v1, vec3d &v2, vec3d &v3 );
    void transform_section( const int &startPnt, vector < vec3d > &sect, Matrix4d &trans, Matrix4d &invtrans );
    static void calculate_section_prop( const vector < vec3d > &sect, double &len, double &area, vec3d &xcgshell, vec3d &xcgsolid, vector < double > &Ishell, vector < double > &Isolid );

    void createDegenSurface( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, bool flipnormal );
    void createSurfDegenPlate( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts );
    void createBodyDegenPlate( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts );
    void createDegenPlate( DegenPlate &degenPlate, const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, int nLow, int nHigh, int startPnt );
    void createSurfDegenStick( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, const VspSurf *foilSurf, const bool &urootcap );
    void createBodyDegenStick( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts );
    void createDegenStick( DegenStick &degenStick, const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, int nLow, int nHigh, int startPnt );
    static void augmentFoilSurfDegenStick( DegenStick &degenStick, const VspSurf *foilSurf, const vector< vector< vec3d > > &uw_pnts, const bool &urootcap );
    void augmentFoilSurfDegenStick( DegenStick &degenStick, const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, const bool &urootcap );
    void createDegenDisk(  const vector< vector< vec3d > > &pntsarr, bool flipnormal );
    void addDegenSubSurf( SubSurface *ssurf, int surfIndx );
    void addDegenHingeLine( SSControlSurf *csurf, int surfIndx );

    static string makeCsvFmt( int n, bool newline = true );
    void write_degenGeomCsv_file( FILE* file_id );
    void write_degenGeomSurfCsv_file( FILE* file_id, int nxsecs );
    void write_degenGeomPlateCsv_file( FILE* file_id, int nxsecs, DegenPlate &degenPlate );
    void write_degenGeomStickCsv_file( FILE* file_id, int nxsecs, DegenStick &degenStick );
    void write_degenGeomPointCsv_file( FILE* file_id );
    void write_degenGeomDiskCsv_file( FILE* file_id );
    void write_degenSubSurfCsv_file( FILE* file_id, int isubsurf );
    void write_degenHingeLineCsv_file( FILE* file_id, int ihingeline );

    void write_degenGeomM_file( FILE* file_id );
    void write_degenGeomSurfM_file( FILE* file_id, int nxsecs );
    void write_degenGeomPlateM_file( FILE* file_id, int nxsecs, DegenPlate &degenPlate, int iplate );
    static void write_degenGeomStickM_file( FILE* file_id, int nxsecs, DegenStick &degenStick, int istick );
    void write_degenGeomPointM_file( FILE* file_id );
    void write_degenGeomDiskM_file( FILE* file_id );
    void write_degenSubSurfM_file( FILE* file_id, int isubsurf );
    void write_degenHingeLineM_file( FILE* file_id, int ihingeline );

    void write_degenGeomResultsManager( vector< string> &degen_results_ids );
    void write_degenGeomDiskResultsManager( Results * res );
    void write_degenGeomSurfResultsManager( Results * res );
    void write_degenGeomPlateResultsManager( vector< string > &plate_ids, const DegenPlate &degenPlate );
    void write_degenGeomStickResultsManager( vector< string > &stick_ids, const DegenStick &degenStick );
    void write_degenGeomPointResultsManager( Results * res );
    static void write_degenSubSurfResultsManager( vector< string > &subsurf_ids, const DegenSubSurf &degenSubSurf );
    static void write_degenHingeLineResultsManager( vector< string > &hinge_ids, const DegenHingeLine &degenHingeLine );

    void createTMeshVec( Geom * geom, vector< TMesh* > &tMeshVec );

    bool GetFlipNormal() const { return flip_normal; }
    void FlipNormal() { flip_normal = !flip_normal; }
    void ResetFlipNormal( ) { flip_normal = false; }

    void Transform( const Matrix4d & mat );

protected:

    DegenSurface degenSurface;
    vector< DegenPlate >   degenPlates;
    vector< DegenStick >   degenSticks;
    DegenPoint   degenPoint;
    DegenDisk    degenDisk;
    vector< DegenSubSurf > degenSubSurfs;
    vector< DegenHingeLine > degenHingeLines;

    int num_xsecs;
    int num_pnts;

    string name;

    Geom* parentGeom;
    int   type;
    int   cfdsurftype;
    int   surfNum;
    bool flip_normal;
    int main_surf_ind;
    int sym_copy_ind;
    vector< double > transmat;
};



#endif
