//
//  geom_degen.h
//  VSP
//
//  Created by Joel Belben on 8/5/12.
//

#ifndef VSP_geom_degen_h
#define VSP_geom_degen_h

#include <vector>
#include "Vec3d.h"
#include "Vec2d.h"
#include "Matrix.h"

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
    int                         nblade;
    double                      d;
    vec3d                       x;
    vec3d                       nvec;
} DegenProp;

typedef struct
{
    string                      name;
    double                      mass;
    vec3d                       x;
} DegenPtMass;

class DegenGeom
{
public:
    enum { XY_PLANE, XZ_PLANE, YZ_PLANE };

    enum { SURFACE_TYPE, BODY_TYPE, DISK_TYPE };

    DegenGeom()
    {
        type = BODY_TYPE;
        num_pnts = 0;
        num_xsecs = 0;
        parentGeom = NULL;
        surfNum = -1;
    };
    virtual ~DegenGeom() {};

    DegenPoint   getDegenPoint()
    {
        return degenPoint;
    }
    DegenProp   getDegenProp()
    {
        return degenProp;
    }

    void setDegenPoint( DegenPoint degenPoint )
    {
        this->degenPoint = degenPoint;
    }
    void setDegenProp( DegenProp degenProp )
    {
        this->degenProp = degenProp;
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

    void setName( string namein )
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

    int getSurfNum()
    {
        return surfNum;
    }
    void setSurfNum( int n )
    {
        surfNum = n;
    }

    void build_trans_mat( vec3d x, vec3d y, vec3d z, const vec3d &p, Matrix4d &mat, Matrix4d &invmat );
    void build_basis( const int &startPnt, const vector < vec3d > &sect, vec3d &v1, vec3d &v2, vec3d &v3 );
    void transform_section( const int &startPnt, vector < vec3d > &sect, Matrix4d &trans, Matrix4d &invtrans );
    void calculate_section_prop( const vector < vec3d > &sect, double &len, double &area, vec3d &xcgshell, vec3d &xcgsolid, vector < double > &Ishell, vector < double > &Isolid );

    void createDegenSurface( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, bool flipnormal );
    void createSurfDegenPlate( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts );
    void createBodyDegenPlate( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts );
    void createDegenPlate( DegenPlate &degenPlate, const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, int nLow, int nHigh, int startPnt );
    void createSurfDegenStick( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts );
    void createBodyDegenStick( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts );
    void createDegenStick( DegenStick &degenStick, const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, int nLow, int nHigh, int startPnt );

    string makeCsvFmt( int n, bool newline = true );
    void write_degenGeomCsv_file( FILE* file_id );
    void write_degenGeomSurfCsv_file( FILE* file_id, int nxsecs );
    void write_degenGeomPlateCsv_file( FILE* file_id, int nxsecs, DegenPlate &degenPlate );
    void write_degenGeomStickCsv_file( FILE* file_id, int nxsecs, DegenStick &degenStick );
    void write_degenGeomPointCsv_file( FILE* file_id, int nxsecs );
    void write_degenGeomPropCsv_file( FILE* file_id );

    void write_degenGeomM_file( FILE* file_id );
    void write_degenGeomSurfM_file( FILE* file_id, int nxsecs );
    void write_degenGeomPlateM_file( FILE* file_id, int nxsecs, DegenPlate &degenPlate, int iplate );
    void write_degenGeomStickM_file( FILE* file_id, int nxsecs, DegenStick &degenStick, int istick );
    void write_degenGeomPointM_file( FILE* file_id, int nxsecs );
    void write_degenGeomPropM_file( FILE* file_id );

protected:

    DegenSurface degenSurface;
    vector< DegenPlate >   degenPlates;
    vector< DegenStick >   degenSticks;
    DegenPoint   degenPoint;
    DegenProp    degenProp;

    int num_xsecs;
    int num_pnts;

    string name;

    Geom* parentGeom;
    int   type;
    int   surfNum;
};



#endif
