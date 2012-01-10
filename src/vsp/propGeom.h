//******************************************************************************
//
//  Prop Geometry Class
//
//
//   J.R. Gloudemans - 10/15/03
//
//******************************************************************************

#ifndef PROP_GEOM_H
#define PROP_GEOM_H

#include "parm.h"
#include "geom.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "xmlvsp.h"
#include "dyn_array.h"

class Af;

class Section
{ 
public:

	Section();

	Af* foil;
	Parm x_off;			// XOff/Radius
	Parm y_off;			// YOff/Radius
	Parm chord;			// Chord/Radius
	Parm twist;			// Twsist Degrees

	virtual void SetGeomPtr( Geom* geomPtr );

};


class PropGeom : public Geom
{
	int numBlades;

	Parm diameter;
	Parm cone_angle;
	Parm pitch;

	Parm loc;
	Parm offset;
	Parm chord;
	Parm twist;

	int currSectID;
	int smoothFlag;
	int numU;
	int numW;

	vector< Section > sectVec;				// One Blade - Section Vec
	vector< Xsec_surf > bladeVec;			// Blade surfaces

public:

   PropGeom(Aircraft* aptr);
   virtual ~PropGeom();

   virtual void copy( Geom* fromGeom );

   vector< Section > & getSectVec()			{ return sectVec; }
   Section* get_curr_section()				{ return &sectVec[currSectID]; }

   Af* get_af_ptr()							{ return sectVec[currSectID].foil; }

   void addStation();
   void delStation();
   int  getNumStations()					{ return sectVec.size(); }

   void setNumBlades( int b );
   int  getNumBlades()						{ return numBlades; }
   void setCurrSectID( int id );
   int  getCurrSectID()						{ return currSectID; }

   void setSmoothFlag( int f );
   int  getSmoothFlag()						{ return smoothFlag; }
   void setNumU( int n );
   int  getNumU()							{ return numU; }
   void setNumW( int n );
   int  getNumW()							{ return numW; }

   Parm* get_diameter()						{ return(&diameter); }
   Parm* get_cone_angle()					{ return(&cone_angle); }
   Parm* get_pitch()						{ return(&pitch); }

   Parm* get_chord()						{ return(&chord); }
   Parm* get_offset()						{ return(&offset); }
   Parm* get_loc()							{ return(&loc); }
   Parm* get_twist()						{ return(&twist); }
  
   virtual void load_hidden_surf();
   virtual void load_normals();
   virtual void update_bbox();
  
   virtual void draw();
   virtual void generate();
   virtual void regenerate();

   virtual void write(FILE* dump_file);
   virtual void write(xmlNodePtr node);
   virtual void read(FILE* dump_file);
   virtual void read(xmlNodePtr node);

   virtual void dump_xsec_file(int, FILE* )	{}

   virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   virtual vector< TMesh* > createTMeshVec();
   virtual int getNumXSecSurfs()				{ return 0; }

   virtual int  get_num_bezier_comps();
   virtual void write_bezier_file( int id, FILE* file_id );

   virtual void computeCenter();
   virtual void parm_changed(Parm* chg_parm);

   virtual void drawAlpha(void);
   virtual void scale();		
   virtual void acceptScaleFactor();
   virtual void resetScaleFactor();

   virtual vec3d getAttachUVPos(double u, double v);

    virtual vec3d getVertex3d(int surfid, double x, double p, int r);
	virtual void  getVertexVec(vector< VertexID > *vertVec);

	virtual void LoadLinkableParms( vector< Parm* > & parmVec );
	virtual void RemoveFoilParmReferences( Af* foil );
	virtual void RemoveSectParmReferences( int sect_id );
	virtual void DeleteAllFoilsAndSects();
	virtual void GetInteriorPnts( vector< vec3d > & pVec );
 
};

#endif
