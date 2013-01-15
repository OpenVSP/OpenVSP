//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifndef CABINLAYOUTGEOM_H
#define CABINLAYOUTGEOM_H

#include "parm.h"
#include "geom.h"
#include "xsec_surf.h"
#include "herm_curve.h"
#include "xmlvsp.h"
#include "dyn_array.h"
#include "vec2d.h"
#include "editCurve.h"
#include "stringc.h"
#include "vec3d.h"

class CabinLayoutScreen;
class EditCurve;

struct CabinGeomData
{
		vector<vec3d> CabinPoints;
};

class CabinLayoutGeom : public Geom
{
	public:
		CabinLayoutGeom(Aircraft* aptr);
		virtual ~CabinLayoutGeom();

		virtual void write(xmlNodePtr node);
		virtual void read(xmlNodePtr node);
		virtual void read(FILE* file_id) {Geom::read(file_id);}

		virtual void copy( Geom* fromGeom );
		virtual void draw();
		virtual void drawAlpha();
		virtual void update_bbox();

		virtual void generate();
		virtual void gen_geom_surf();

		virtual void computeCenter();
		virtual void parm_changed(Parm* chg_parm);

		virtual void acceptScaleFactor();
		virtual void resetScaleFactor();

		virtual void scale();

//		virtual void write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes );
		virtual int  get_num_bezier_comps();
//		virtual void write_bezier_file( int id, FILE* file_id );
		virtual void dump_xsec_file(int geom_no, FILE* dump_file);

		virtual vec3d getVertex3d(int surf, double x, double p, int r)	{ return surfVec[surf]->get_vertex(x, p, r); }
		virtual void  getVertexVec(vector< VertexID > *vertVec);

		vector< TMesh* > createTMeshVec();

		virtual void AddDefaultSources(double base_len = 1.0);

		virtual double getRefArea();
		virtual double getRefSpan();
		virtual double getRefCbar();
		virtual vec3d  getAeroCenter();

		Stringc get_geom_data_file_name() const {return geom_data_file_name;}
		bool get_MirrorData() const {return MirrorData;}

		void set_geom_data_file_name(Stringc new_file);
		void set_MirrorData(bool val);

	protected:
		Stringc geom_data_file_name;
		vector<CabinGeomData> vCabinInfo;
		bool UpdateGeom;
		bool MirrorData;
};

#endif

