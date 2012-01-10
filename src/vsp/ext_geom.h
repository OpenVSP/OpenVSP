//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   External Geometry Class
//  
// 
//   J.R. Gloudemans - 11/7/94
//   Sterling Software
//
//******************************************************************************

#ifndef EXT_GEOM_H
#define EXT_GEOM_H


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

#include "af.h"



#define BOMB_TYPE	0 //!@# why don't these match gui
#define MISSLE_TYPE	1 //!@#
#define TANK_TYPE	2
#define FIXED_TANK_TYPE	3

class Ext_geom : public Geom
{
	enum { BODY_SURF, PYLON_SURF, FIN1_SURF, FIN2_SURF };
   int ext_type;
   int pylon_flag;
   Parm length;
   Parm fine_ratio;
   Parm pylon_height;

   Parm drag;
   Parm pylon_drag;

   fuse_xsec circle;
   Af* pylon_af; 
   Af* fin_af; 
   Bezier_curve body_curve;
   Xsec_surf body_surf;
   Xsec_surf pylon_surf;
   Xsec_surf fin1_surf;
   Xsec_surf fin2_surf;

   int numPntsBody;
   int numXsecsBody;
   int numPntsPylon;
   int numXsecsPylon;
   int numPntsFin;
   int numXsecsFin;

public:

   Ext_geom(Aircraft* aptr);
   ~Ext_geom();
 
   virtual void copy( Geom* fromGeom );
   virtual void define_parms(); 

   virtual void set_ext_type(int type_in);
   virtual int get_ext_type()				{ return(ext_type);    }

   virtual void set_pylon_flag(int flag_in);
   virtual int get_pylon_flag()				{ return(pylon_flag); }

   Parm* get_length()				{ return(&length); }
   Parm* get_fine_ratio()			{ return(&fine_ratio); }
   Parm* get_pylon_height()			{ return(&pylon_height); }

   Parm* get_drag()					{ return(&drag); }
   Parm* get_pylon_drag()			{ return(&pylon_drag); }

   virtual void setNumPntsXsecs();

   virtual void generate_body_curve();
   virtual void gen_body_surf();
   virtual void generate_pylon();
   virtual void generate_fins();
  
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

   virtual void dump_xsec_file(int, FILE* )	{}

   virtual float get_area();
   virtual float get_span();
   virtual float get_cbar();

   virtual void computeCenter();
   virtual void parm_changed(Parm* chg_parm);

   virtual void drawAlpha(void);
   virtual vec3d getAttachUVPos(double u, double v);
   virtual void scale();		
   virtual void acceptScaleFactor();
   virtual void resetScaleFactor();

   virtual int getNumXSecSurfs()				{ return 0; }

   //===== Aero Functions =====
   virtual void	write_aero_file(FILE* aero_file, int aero_id_num);

    virtual vec3d getVertex3d(int surfid, double x, double p, int r);
	virtual void  getVertexVec(vector< VertexID > *vertVec);

	virtual void LoadLinkableParms( vector< Parm* > & parmVec );


};

#endif
