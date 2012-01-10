//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Havoc Geometry Class
//  
// 
//   J.R. Gloudemans - 11/16/94
//   ETP Software
//
//    
//******************************************************************************

#ifndef HAVOC_GEOM_H
#define HAVOC_GEOM_H


#include "vec2d.h"
#include "vec3d.h"
#include "parm.h"
#include "bezier_curve.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "bbox.h"
#include "xmlvsp.h"
#include "geom.h"
#include "fuse_xsec.h"

#include "xsec_surf.h"
#include "dl_list.h"

//jrg nuke class Havoc_screen;


class Havoc_geom : public Geom
{

   //==== Planform ====//
   Parm lpiovl;		// Break point
   Parm apm;
   Parm apn;
   Parm pera;

   Parm ar;
   Parm al;
   
   Parm pln;		// Percent Nose Length
   Parm ple;		// Percent Engine Length
   
   //==== Side ====//
   Parm aum;		// Upper exponent
   Parm aun;
   Parm bu;		// Height Upper
   
   Parm alm;		// Lower exponent
   Parm aln;
   Parm bl;		// Height lower
   
   Parm gum;		// Upper shape exponent
   Parm theta;		// Lower exit angle
   Parm ptas;		// Fraction of bu
      
   Parm bue;		// Exit Height Upper
   Parm ble;		// Exit Height Lower

   //==== X-Sec ====//
   Parm mu;		// Xsec Upper M Exp
   Parm nu;		// Xsec Upper M Exp

   Parm ml;		// Xsec Upper M Exp
   Parm nl;		// Xsec Upper M Exp

   Parm mexp1;		// Xsec Upper M Exp
   Parm nexp1;		// Xsec Upper M Exp

   Parm mexp2;		// Xsec Upper M Exp
   Parm nexp2;		// Xsec Upper M Exp

   Parm mexp3;		// Xsec Upper M Exp
   Parm nexp3;		// Xsec Upper M Exp

   Parm mexp4;		// Xsec Upper M Exp
   Parm nexp4;		// Xsec Upper M Exp
 
   int havoc_num_xsecs;
   int havoc_num_pnts;
   
   array< double > x_locs;
   array< double > left;
   array< double > right;
   array< double > upper;
   array< double > lower;
 
   Parm length;
   
   Xsec_surf body_surf;

public:

   Havoc_geom(Aircraft* aptr);
   virtual ~Havoc_geom();
      
   virtual void copy( Geom* fromGeom );
 
   void define_parms(); 
   virtual void computeCenter();
   void parm_changed(Parm* p);

   Parm* get_length()			{ return(&length); }

   Parm* get_lpiovl()			{ return(&lpiovl); }
   Parm* get_apm()			{ return(&apm); }
   Parm* get_apn()			{ return(&apn); }
   Parm* get_pera()			{ return(&pera); }
   Parm* get_pln()			{ return(&pln); }
   Parm* get_ple()			{ return(&ple); }

   Parm* get_ar()			{ return(&ar); }
   Parm* get_al()			{ return(&al); }

   Parm* get_aum()			{ return(&aum); }
   Parm* get_aun()			{ return(&aun); }
   Parm* get_bu()			{ return(&bu); }

   Parm* get_alm()			{ return(&alm); }
   Parm* get_aln()			{ return(&aln); }
   Parm* get_bl()			{ return(&bl); }

   Parm* get_gum()			{ return(&gum); }
   Parm* get_theta()			{ return(&theta); }
   Parm* get_ptas()			{ return(&ptas); }

   Parm* get_bue()			{ return(&bue); }
   Parm* get_ble()			{ return(&ble); }

   Parm* get_mu()			{ return(&mu); }
   Parm* get_nu()			{ return(&nu); }

   Parm* get_ml()			{ return(&ml); }
   Parm* get_nl()			{ return(&nl); }

   Parm* get_mexp1()			{ return(&mexp1); }
   Parm* get_nexp1()			{ return(&nexp1); }

   Parm* get_mexp2()			{ return(&mexp2); }
   Parm* get_nexp2()			{ return(&nexp2); }

   Parm* get_mexp3()			{ return(&mexp3); }
   Parm* get_nexp3()			{ return(&nexp3); }

   Parm* get_mexp4()			{ return(&mexp4); }
   Parm* get_nexp4()			{ return(&nexp4); }

   void generate_planform_curves();

   void gen_body_surf();
  
   void load_hidden_surf();
   void load_normals();
   void update_bbox();
   
   double compute_z( int quad, double x_fract, double y_fract );
   double compute_nose_z( int quad, double x_fract, double y_fract );
   double compute_tail_z( int quad, double x_fract, double y_fract );
   
   enum { UPPER_RIGHT, UPPER_LEFT, LOWER_LEFT, LOWER_RIGHT };
  
   void draw();
   void generate();
   void regenerate();
   void gen_surf();
   void write(FILE* dump_file);
   void write(xmlNodePtr node);
   void read(FILE* dump_file);
   void read(xmlNodePtr node);
   virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
   virtual vector< TMesh* > createTMeshVec();

   virtual int  get_num_bezier_comps();
   virtual void write_bezier_file( int id, FILE* file_id );

   void dump_xsec_file(int, FILE* );


   double get_area();
   double get_span();
   double get_cbar();

   void drawAlpha(void);
   virtual vec3d getAttachUVPos(double u, double v);
   void scale();		
   void acceptScaleFactor();
   void resetScaleFactor();


    virtual vec3d getVertex3d(int surf, double x, double p, int r)			{ return body_surf.get_vertex(x, p, r); }
	virtual void  getVertexVec(vector< VertexID > *vertVec)		{ buildVertexVec(&body_surf, 0, vertVec); }

   virtual double getRefArea();
   virtual double getRefSpan();
   virtual double getRefCbar();
   virtual vec3d  getAeroCenter();

   virtual void LoadLinkableParms( vector< Parm* > & parmVec );
   virtual void GetInteriorPnts( vector< vec3d > & pVec );

   virtual void  AddDefaultSources(double base_len = 1.0)				{}

};

#endif
