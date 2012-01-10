//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifndef _VOROBJ_H_
#define _VOROBJ_H_

/*
#ifdef WIN32
#include <windows.h>		
#endif
#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif
*/

#define NUM_CP_FREQ 50
#define MAX_CSF 20
#define MAX_CAMB 50
#define NSLICE_WIDTH 70
        
typedef struct{
        int ncomps, *ncross, *npts;
//        float ****herm, ****sherm;
		float ****herm;
		int ntraps, nslices;
//		float ***trap, ***strap;
		float ***trap;
        float ***ttrap;
		int ncamb;
//		float **camb, **scamb;
		float **camb;
		int nsubp, ncpv;
		float ***subp, ***ssubp;
		float *cpval;
        short **cpcol;
        float cp_surf_min;
        float cp_surf_max;
        float cp_surf_min_clamp;
        float cp_surf_max_clamp;
        int cp_freq[NUM_CP_FREQ];
        float cp_min, cp_max;
        float min_clamp, max_clamp;
        float scale, x_trans, y_trans;
        int ntotcsf, csfcheck, curr_csf;
        int *ncsftrap, **csftrap;
        int *csftype, *csfreflect, *csfsymmetry;
        float *csfdefangle, *csfchord, *csfhingepercent; 
        int flat_flag;
        int final_num_sub;
        float sref, cbar, xbar, zbar;
        float cltot, cdtot, cmtot, cltrf, cdtrf, ep, et, cdf;
        float final_alpha, final_mach;       
        char **csfname;
        char title1[255], title2[255];
        int iters;
}VOROBJ;

static VOROBJ *Vor;

/*
int readslice();
float fgetmousex();
float fgetmousey();
float flerp();
int isstrdigit();
int get_num_slc_camb();
int check_bbox();
int chk_zy_box();
int order_slices();

int no_slice;
int batch;
int numsub_batch;
int reset_Mach, reset_Re;
float new_Mach, new_Re;

int do_scaling;
float user_scale;

float ***falloc_3d();
float **falloc_2d();
int   **ialloc_2d();

int vorinp_();


int showherm, showslc, showcamb, showtitle; 
int showsubp, showcpvals, showhiddenline;
int askslice, showkeyslice, showrgb;
int mmclamp;

float zm_vor, zm_camb;

char aircraftName[80];
int lastNumSub;

VOROBJ *Vor;
int submenu1, submenu2;
int submenu3, submenu4;
int submenu5, menu;
int submenu6; 
int submenu7; 
int submenu3_1; 

int swap_xy;

#define MAIN_WIN 0
#define HELP_WIN 1
#define CONTROL_WIN 2
int currWin;

int mainWinID;
int helpWinID;
int controlWinID;

int viewMainSubMenu;
int colorMainSubMenu;
int exeMainSubMenu;
int keyMainSubMenu;
int quitMainSubMenu;
int mouseMainSubMenu;
int flagMainSubMenu;
int mainMenu;

int controlMenu;

#ifndef MIN
#define MIN(a,b) ( (a)<=(b) ? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a,b) ( (a)>=(b) ? (a) : (b) )
#endif

#define TRACK_STOP 0
#define TRACK_TRANSLATE 1
#define TRACK_ROTATE 2
#define TRACK_SCALE 3

#define INPUT_INTEGER 0
#define INPUT_FLOAT   1
#define INPUT_TEXT    2

int mouse_mode;
#define TRANSFORM        0
#define ADD_KEY_SLICE    1
#define MOVE_KEY_SLICE   2
#define MODIFY_TRAPEZOID 3
*/

#endif


