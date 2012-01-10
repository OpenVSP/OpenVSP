//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   MDuct Geometry Class
//
//
//   J.R. Gloudemans - 10/15/03
//
//******************************************************************************

#ifndef DUCT_GEOM_H
#define DUCT_GEOM_H

#include "parm.h"
#include "geom.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "xmlvsp.h"
#include "dyn_array.h"


class Af;

class DuctGeom : public Geom
{
   Xsec_surf surf;

   Af* foil;

   Parm length;
   Parm chord;
   Parm inlet_dia;
   Parm inlet_area;
   Parm outlet_dia;
   Parm outlet_area;
   Parm inlet_outlet;

public:

   DuctGeom(Aircraft* aptr);
   virtual ~DuctGeom();

   virtual void copy( Geom* fromGeom );

   Af* get_af_ptr()							{ return foil; }

   Parm* get_length()						{ return(&length); }
   Parm* get_chord()						{ return(&chord); }
   Parm* get_inlet_dia()					{ return(&inlet_dia); }
   Parm* get_inlet_area()                   { return(&inlet_area); }
   Parm* get_outlet_dia()					{ return(&outlet_dia); }
   Parm* get_outlet_area()					{ return(&outlet_area); }
   Parm* get_inlet_outlet()					{ return(&inlet_outlet); }
  
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

   virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   virtual vector< TMesh* > createTMeshVec();

   virtual int  get_num_bezier_comps();
   virtual void write_bezier_file( int id, FILE* file_id );

   virtual void dump_xsec_file(int geom_no, FILE* dump_file);

   virtual void computeCenter();
   virtual void parm_changed(Parm* chg_parm);

   virtual void drawAlpha(void);
   virtual void scale();		
   virtual void acceptScaleFactor();
   virtual void resetScaleFactor();

   virtual vec3d getAttachUVPos(double u, double v);

   //===== Felisa Functions =====   
   virtual int write_felisa_file(int geom_no, FILE* dump_file);
   virtual void write_bco_info(FILE*, int&, int&);
   virtual int get_num_felisa_comps();
   virtual int get_num_felisa_wings();
   virtual int get_num_felisa_line_sources();
   virtual int get_num_felisa_tri_sources();
   virtual void write_felisa_line_sources(FILE*);
   virtual void write_felisa_tri_sources(FILE*);

    virtual vec3d getVertex3d(int surfid, double x, double p, int r)			{ return surf.get_vertex(x, p, r); }
	virtual void  getVertexVec(vector< VertexID > *vertVec)		{ buildVertexVec(&surf, 0, vertVec); }

   virtual double getRefArea();
   virtual double getRefSpan();
   virtual double getRefCbar();
   virtual vec3d  getAeroCenter();

   virtual void LoadLinkableParms( vector< Parm* > & parmVec );
   virtual void GetInteriorPnts( vector< vec3d > & pVec );
};

#endif
