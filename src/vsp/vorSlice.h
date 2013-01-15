//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Vorview Slice Class
//  
// 
//   J.R. Gloudemans - 11/21/04
//
//    
//******************************************************************************

#ifndef VORSLICE_H
#define VORSLICE_H

#include <float.h>
#include <stdio.h>

#include "vorobj.h"

//==== Moved from VorObj ====//
#define NUM_CP_FREQ 50
#define MAX_CSF 20
#define MAX_CAMB 50
#define NSLICE_WIDTH 70
#define MAX_TRAP 200


#define INPUT_INTEGER 0
#define INPUT_FLOAT   1
#define INPUT_TEXT    2

#ifndef MIN
#define MIN(a,b) ( (a)<=(b) ? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a,b) ( (a)>=(b) ? (a) : (b) )
#endif

#define MINFLOAT      FLT_MIN

typedef struct Point {
   float x;
   float y;
   float z;
} Point;

typedef struct Key_point {
   int trap;         /* index into trapezoid array */
   int point;        /* index into trapezoid's point array */
   int init_flag;    /* flag which indicates whether or not we need to init */
		     /* presently unused... */
   int delete_flag;  /* used when key slices are deleted... */
		     /* presently unused... */
   Point orig_point; /* original x,y,z point */
   Point new_point;  /* new x,y,z point */
} Key_point;

typedef struct Key_point_list {
   int n_key_point;       /* number of key points */
   int current_key_point; /* index of current key point */
   Key_point *key_point;  /* array of key_points (sorted in x and z) */
} Key_point_list;

typedef struct trap_face {
   float *low_front;
   float *high_front;
   float *low_back;
   float *high_back;
} trap_face;

struct Key_slice {
   int nkey;        /* number of key slices */
   int current_key; /* current key slice */
   int *nslice;     /* number of slices in current slice region */
   int temp_flag;   /* flag for temporary key slice (used for adding) */
   int symmetry_flag; /* flag which indicates whether symmetry exists */
   float xmin;      /* min x for drawing... */
   float xmax;      /* max x for drawing... */
   float ymin;      /* min y */
   float ymax;      /* max y */
   float *y;        /* y value for current key slice */
   float ytemp;     /* y value for temporary key slice (used for adding) */
   float zmin;
   float zmax;
   float zavg;      /* average z value for model */
/* normalized versions of the above... */
   float sxmin;      /* min x for drawing... */
   float sxmax;      /* max x for drawing... */
   float symin;      /* min y */
   float symax;      /* max y */
   float *sy;        /* y value for current key slice */
   float sytemp;     /* y value for temporary key slice (used for adding) */
   float szavg;      /* average z value for model */
/* key point info */
   int nkeypt;            /* number of key point lists */
   int current_keypt;     /* index of current keypoint list */
   Key_point_list *keypt; /* array of key point lists */
};

typedef struct Trapezoid {
   int temp;      /* index of trapezoid being modified */
   int point;     /* index of current point in trapezoid being modified */
} Trapezoid;



class VorSlice
{
public:

	VorSlice();
	virtual ~VorSlice();


	//draw.c
	void drawherm();
	void drawslices();
	void draw_point(float xyz[],int pnt);
	void draw_main_point(float xyz[]);
	void draw_other_point(float xyz[]);
	void drawsubpols();
	void drawcpvals();
	void drawrgb();
	void drawcamber();
	float getcamberzoom()								{ return zm_camb; }
	void setcamberzoom(float z)							{ zm_camb = z; }
	void drawcsf();

	void get_trap_normal(int islc, float *normal);
	void vector_cross(float *vec1, float *vec2, float *cross);
	float vector_dot(float *vec1, float *vec2);

	//auto_slice.c
	void compute_area_ratio(float *ratio, float *ytemp);
	void eliminate_superfluous_keys();

	//batch.c
	void run_batch( char* filename, int numsub_batch );

	//camber.c
	void trap_camb(float **bbox);
	void min_max_z_val(int icomp,float xloc,float yloc, int* z_cnt, float*   z_min_max);
	void trail_z_vals(float* z_val, int* exist_flag);
	void normalize_z_vals(float* z_val, int* exist_flag);
	int chk_xy_box(float* bbox, float xloc, float yloc);
	int find_poly_z(float xloc, float yloc,float** tpts,float* zval);
	void find_y_int(float yloc,float** tpts, int ind1, int* num_int,   float* xint,float* zint);
	void lead_z_vals(float* z_val, int* exist_flag);

	//dump.c
	int dump_point(Point *pnt);
	int dump_key_point(Key_point *keypt);
	int dump_key_point_list(Key_point_list *keyptlst);
	int dump_key_points();
	int dump_trap();

	//hermgeom.c
	void normherm();
	void normslice();
	void normsubpol();
	int title_read(char* flname);
	void minmax_cp();
	void findcpcolors(int min_clamp_flag, int max_clamp_flag);
	void findrgbcolors(int min_clamp_flag, int max_clamp_flag);
	void initclamps();
	void find_rgb(float pcent, short cvec[3] );

	//init_vorobj.c
	void zero_camber(void);
	void unzero_camber(void);

	//moved.c
	void zoomvor(float zfactor);
	void zoomcamb(float zfactor);
	void title_write(char* flname);
	int info_message(const char *message, int pause_time);
	void drawit();
	int vorinp_();
	int srfprs_();
	int clsstb_();
	void setInfoMessage( const char* msg, int msecs );
	void setInpCBPtr(  void (*fptr)(int, char*, int) );
	void setInpType( int t );
	void setInpID( int id );
	void setInpMessage( const char* msg );
	void setFractDone( float f );
	void setFractDoneOff();
	void setFractDoneOn();
	int getFractDoneActive();

	//read_file.c
	void clear();
	int readherm(char* flname);
	int readslice(char* flname);
	int readsubpol(char* flname);
	int readcpvals(char* flname);
	int readcsf(char* flname);
	int correct_file(FILE* in_file, const char* header);
	int write_flags(char* flname);
	int read_flags(char* flname);

	//slice.c
	void compute_slice();
	void comp_bound_box(float** bbox);
	void slice_locations(float* slice);
	  int check_bbox(float* box, float slc);
	void comp_slice(int comp_num, float slc, float* comp_slc);
	void slc_int(float slc, float x1, float x2, float y1, float y2, 
	  float z1, float z2, float* xint, float* zint);
	int order_slices(int comp_slc_cnt, float** comp_slc, float** all_s);
	void assemble_slices(float* slice,  int* num_all_slc, 
	  float*** all_slc);
	void load_trap(int ntrap,float x1, float x2, float x3, float x4, 
	  float y1, float y2, float z1, float z2, float z3, float z4);
	void flip_trap_order(int s_ind, int e_ind);
	void write_trap(char* flname);
	void write_csf_trap(char* flname);
	void flap_slat_camb(int trap, int num_temp_camb, float* temp_camb, 
	  int csf, float camb_len);
	void getFirstTitleString( int num_char, char* str, int id );
	void getSecondTitleString( int num_char, char* str, int id );

	void reslice( int num_char, char* str, int id );
	void recamber( int num_char, char* str, int id );
	
	//trapezoid.c
	void init_trapezoid();
	void set_trapezoid(int);
	void unset_trapezoid();
	void default_trapezoid_point();
	void up_trapezoid_point();
	void down_trapezoid_point();
	void move_trapezoid_point(float);
	int picked_trapezoid(int);
	int picked_trapezoid_point(int);
	int get_trapezoid();

	//utility.c
	float fabsf( float x );
	float sqrtf( float x );
	float ceilf( float x );
	float floorf( float x );
	int isstrdigit(char* in_string);
	void info_mess_(char* message, int* pause_time);
	void time_keep_(float* fract_done);
	float **falloc_2d(int d1, int d2);
	int **ialloc_2d(int d1, int d2);
	float ***falloc_3d(int d1,int d2,int d3);
	void ffree_2d(float **farray, int d1);
	void ifree_2d(int **iarray, int d1);
	void ffree_3d(float*** farray, int d1, int d2);
	void find_trap_y_minmax(float **trap,float *ymin,float *ymax);
	int angles_match(const float *v1s, const float *v1e, const float *v2s, 
			 const float *v2e, const float cos_limit);
	int ranges_overlap(float x1low, float x1high, float x2low, 
	  float x2high);
	float nearest_int(float x);

	//point.c
	void set_point(Point *p,float x,float y,float z);
	void get_point(Point *p,float *x,float *y,float *z);
	void copy_point(Point *pfrom,Point *pto);
	int equal_point(Point *p,float xyz[3]);
	int greater_than_point(Point *p,float xyz[3]);

	//keyPoint.c
	int Num_key_points(Key_point_list*);
	Key_point_list* alloc_key_point_list(Key_point_list **keypt, int n);
	Key_point_list* realloc_key_point_list(Key_point_list* keypt,int old_n,
				       int new_n,int new_index);
	void free_key_point_list(Key_point_list*);
	void insert_key_point(Key_point_list *keyptlst,int itrap,int ipoint);
	int Key_point_index(float xyz[3],Key_point_list *keypt,int *element);
	void Set_current_key_point(Key_point_list *keypt,int element);
	void Move_current_key_point(Key_point_list *keypt,float xyz[3]);
	void Move_opposite_key_point(Key_point_list *ckeypt,
				    Key_point_list *okeypt,float x);
	void Reset_current_key_point(Key_point_list*);
	void Reset_opposite_key_point(Key_point_list*,Key_point_list*);
	int Get_key_point_trap(Key_point_list*,int);
	int Locate_key_point_by_trap(Key_point_list*, int);
	int Locate_key_point_pair(Key_point_list*, int);
	void Get_key_point_new_xyz(Key_point_list*, int, float*, float*, float*);
	void Set_key_point_new_xyz(Key_point_list*, int, float, float, float);
	int Read_key_point(FILE *fp,Key_point_list *keyptlst);
	int Write_key_point(FILE *fp,Key_point_list *keyptlst,int low_list);

	//vor_execute
	void subdivide_trap( int num_char, char* instr, int id );
	void execute_vor(int num_char, char* instr, int id );
	int run_vorlax(char* flname,char* csfname,int* cur_csf,
	  int* icalc,int numsub,float* delta,int* stab);

	int write_surf_press_pnts(char* srfname);
	int calc_surf_press(char* srfname);

	//auto_slice
	float trap_area(const trap_face* const);
	float area_between(float ylow,float yhigh);
	void build_trap_face_array();
	int find_low_key_slices(const int itrap, const float cos_limit, 
			       float *ylow);
	int find_high_key_slices(const int itrap, const float cos_limit, 
			        float *yhigh);
	int find_key_slices();
	void eliminate_redundant_keys();
	void merge_close_keys();
	void free_trap_face_array();
	void auto_find(void);
	void auto_eliminate(void);
	void auto_merge(void);
	void auto_slice(void);

	//control.c
	void add_del_csf_trap(int curr, int trap);
	void add_csf();
	void del_csf();
	void flap_write(char* flname);

	void set_curr_csf( int id );
	int  get_curr_csf();
	void draw_csf();
	void get_csf_color(int index, int* r, int* g, int* b);



	//key_slice
	void init_key_slice();
	void reset_key_slice();
	int get_num_key_slices();
	float get_current_key_slice_y();
	void  set_current_key_slice_y(int id, float y);
	int get_current_nslices();
	void set_current_nslices(int n);
	int get_total_nslices();
	int get_current_region();
	int up_current_key_slice();
	int down_current_key_slice();
	int up_nslice();
	int down_nslice();
	int set_current_key_slice(int);
	int get_current_key_slice();
	int reset_current_key_slice();
	void set_temp_key_slice_y(float);
	void set_temp_key_slice(float);
	void adjust_temp_key_slice_y(float);
	float get_temp_key_slice_y();
	int key_slice_exists();
	void set_close_key_slice_y( float );
	void add_key_slice();
	void delete_key_slice();
	void delete_key_slice_end();
	void copy_key_slice();
	int get_opposite_region();
	int get_opposite_keyslice();
	void set_nslice(int);
	int set_symmetry_flag(int);
	int get_symmetry_flag();
	int toggle_symmetry_flag();
	int read_key_slice(char*);
	int write_key_slice(char*);
	int get_key_point_list(int);
	int get_num_key_point_list(int);
	void find_key_points();
	int key_point_index(float xyz[3],int *ilist,  int *ielement,
						int *ilist2, int *ielement2);
	int is_key_point(float xyz[3]);
	void set_current_key_point(int ilist,int ielement);
	void move_current_key_point(float xyz[3]);
	void move_opposite_key_point(float x);
	void free_key_point_list_array();
	void reset_current_key_point();
	int get_key_point_trap(int,int);
	int locate_key_point_by_trap(int, int);
	int locate_key_point_pair(int, int);
	int num_key_points(int);
	void get_key_point_new_xyz(int, int, float*, float*, float*);
	void set_key_point_new_xyz(int, int, float, float, float);
	int read_key_point(char* basename);
	int write_key_point(char* basename);
	int get_key_slice_temp_flag();
	int set_key_slice_temp_flag(int);
	int find_centermost_key_slice();
	int small_key_slice_gap(int, int);
	void merge_key_slices(int, int);
	void get_key_slice_percent_bounds(int base_on_current, float *pmin, 
							      float *pmax);
	void _delete_key_slice_(int);
	void uninit_key_slice(void);
	void add_middle_key_slice(void);

	void set_curr_trap( int );
	int  get_curr_trap();

	int is_key_slice_y( float y_val );

	float key_slice_ymin()				{ return key_slice.ymin; }
	float key_slice_ymax()				{ return key_slice.ymax; }



	VOROBJ *Vor;


protected:

	int swap_xy;
	int showherm, showslc, showcamb, showtitle; 
	int showsubp, showcpvals, showhiddenline;
	int askslice, showkeyslice, showrgb;

	float zm_vor, zm_camb;
	int do_scaling;
	float user_scale;
	char aircraftName[255];

	int reset_Mach, reset_Re;
	float new_Mach, new_Re;


	trap_face* Trap_face; /* array of trapezoid face information, one 
                                element per trapezoid... */

	float key_slice_tolerance;	
	struct Key_slice key_slice;

	int saved_ncamber;
	int default_nslice;

	Trapezoid trapezoid;

	int initKeyFlag;

	int curr_trap;


};

#endif

