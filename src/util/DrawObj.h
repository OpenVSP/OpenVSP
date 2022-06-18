//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DrawObj.h: 
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_DRAWOBJ_H
#define VSP_DRAWOBJ_H

#include "Vec3d.h"

#include <vector>
#include <string>

using namespace std;

#define PICKGEOMHEADER "GPICKING_"
#define PICKVERTEXHEADER "VPICKING_"
#define PICKLOCHEADER "LPICKING_"
#define BBOXHEADER "BBOX_"
#define XSECHEADER "XSEC_"

class DrawObj
{
public:
    /*
    * Constructor.
    */
    DrawObj();
    /*
    * Destructor.
    */
    virtual ~DrawObj();

    /*
    * Render Type Enum.
    *
    * VSP_POINTS - Render to points.
    * VSP_LINES - Render to lines.
    * VSP_LINE_STRIP - Render to line strip. 
    * VSP_LINE_LOOP - Render to line loop.
    * VSP_WIRE_MESH - Render to wire frame.
    * VSP_HIDDEN_MESH - Render to solid wire frame.
    * VSP_SHADED_MESH - Render to mesh with lighting.
    * VSP_TEXTURED_MESH - Render to texture mapped mesh with lighting.
    * VSP_WIRE_TRIS - Render Triangles to wire frame.
    * VSP_HIDDEN_TRIS - Render Triangles to solid wire frame.
    * VSP_SHADED_TRIS - Render Triangles to mesh with lighting.
    * VSP_WIRE_SHADED_TRIS - Render Triangles to wire frame mesh with lighting
    * VSP_SETTING - This drawObj provides Global Setting Info(lighting, etc...).  Does not render anything.
    * VSP_RULER - Render Ruler.
    * VSP_PICK_VERTEX - This type drawObj enables vertex picking for a specific geometry.
    * VSP_PICK_GEOM - This type drawObj enables geometry picking.
    * VSP_PICK_LOCATION - This type drawObj enables location picking.
    * VSP_CLIP - Add clipping plane.
    * VSP_HIDDEN_QUADS - Render Quads to solid wire frame.
    * VSP_SHADED_QUADS - Render Quads to mesh with lighting.
    * VSP_WIRE_QUADS - Render Quads to wire frame.
    * VSP_WIRE_SHADED_QUADS - Render Quads to wire frame mesh with lighting
    */

    static vec3d ColorWheel( double angle ); // Returns rgb for an angle given in degrees

    enum TypeEnum
    {
        VSP_POINTS,
        VSP_LINES,
        VSP_LINE_STRIP,
        VSP_LINE_LOOP,
        VSP_WIRE_MESH,
        VSP_HIDDEN_MESH,
        VSP_SHADED_MESH,
        VSP_TEXTURED_MESH,
        VSP_WIRE_TRIS,
        VSP_HIDDEN_TRIS,
        VSP_SHADED_TRIS,
        VSP_WIRE_SHADED_TRIS,
        VSP_HIDDEN_TRIS_CFD,
        VSP_HIDDEN_QUADS,
        VSP_HIDDEN_QUADS_CFD,
        VSP_SETTING,
        VSP_RULER,
        VSP_PICK_VERTEX,
        VSP_PICK_GEOM,
        VSP_PICK_LOCATION,
        VSP_PROBE,
        VSP_CLIP,
        VSP_SHADED_QUADS,
        VSP_WIRE_QUADS,
        VSP_WIRE_SHADED_QUADS,
    };

    /*
    * Target Screen Enum.
    * This is not Screen Type, each screen represent an actual screen
    * in GUI.
    *
    * VSP_MAIN_SCREEN - Main VSP display window.
    * VSP_XSEC_SCREEN - 2D display window in XSec Panel.
    * VSP_FEA_XEC_SCREEN - 2D dimensioned drawing of FEA beam element XSec
    */
    enum ScreenEnum
    {
        VSP_MAIN_SCREEN,
        VSP_XSEC_SCREEN,
        VSP_TEX_PREVIEW,
        VSP_FEA_XSEC_SCREEN,
        VSP_EDIT_CURVE_SCREEN
    };

    /*
    * Probe step enum.
    * A probe requires two steps to complete.  On first
    * step, only start point is given.  Probe label is drawn
    * between start point and mouse location.
    */
    enum ProbeEnum
    {
        VSP_PROBE_STEP_ZERO,
        VSP_PROBE_STEP_ONE,
        VSP_PROBE_STEP_TWO,
        VSP_PROBE_STEP_COMPLETE,
    };

    /*
    * Ruler step enum.
    * A ruler requires three steps to complete.  On first
    * step, only start point is given.  Ruler is drawn
    * between start point and mouse location.  On second
    * step, both start and end point are given.  Ruler is
    * drawn between those points.  Lastly, offset is set and
    * ruler is set to appropriate height.
    */
    enum RulerEnum
    {
        VSP_RULER_STEP_ZERO,
        VSP_RULER_STEP_ONE,
        VSP_RULER_STEP_TWO,
        VSP_RULER_STEP_COMPLETE,
    };

    /*
    * Probe Information.
    * Step - Current step of building a probe.
    * Pt - Probe surface point.
    * Norm - Surface normal at probe point.
    * Len - Length of label from point.
    */
    struct Probe
    {
        ProbeEnum Step;
        vec3d Pt;
        vec3d Norm;
        double Len;
        string Label;
    };

    /*
    * Ruler Information.
    * Step - Current step of building a ruler.
    * Start - Vertex Information of starting point.
    * End - Vertex Information of ending point.
    * Offset - placement of the ruler.
    */
    struct Ruler
    {
        RulerEnum Step;
        vec3d Start;
        vec3d End;
        vec3d Offset;
        string Label;
        int Dir;
    };

    /*
    * Information needed to map a single texture.
    *
    * FileName - Texture File Path + File Name.
    * ID - Texture ID, this ID is only unique to this drawObj.
    *
    * U - Texture's position on U direction.
    * W - Texture's position on W direction.
    *
    * UScale - Texture's scale on U direction.
    * WScale - Texture's scale on W direction.
    *
    * Transparency - Texture's Alpha value.
    *
    * UFlip - Flag for if U coordinate is flipped.
    * WFlip - Flag for if W coordinate is flipped.
    */
    struct TextureInfo
    {
        string FileName;
        std::string ID;

        float U;
        float W;

        float UScale;
        float WScale;

        float Transparency;

        bool UFlip;
        bool WFlip;
    };

    /*
    * Information needed to define a material.
    *
    * Ambient - Ambient value for this material.
    * Diffuse - Diffuse value for this material.
    * Specular - Specular value for this material.
    * Emission - Emission value for this material.
    * Shininess - Shine value, the value is between 0 and 128.
    */
    struct MaterialInfo
    {
        float Ambient[4];
        float Diffuse[4];
        float Specular[4];
        float Emission[4];

        float Shininess;
    };

    /*
    * Light Source Information.
    *
    * Active - True if Light Source is active.
    *
    * X - Light Source's position on X-axis.
    * Y - Light Source's position on Y-axis.
    * Z - Light Source's position on Z-axis.
    *
    * Amb - Ambient Light Value.
    * Diff - Diffuse Light Value.
    * Spec - Specular Light Value.
    */
    struct LightSourceInfo
    {
        bool Active;

        float X;
        float Y;
        float Z;

        float Amb;
        float Diff;
        float Spec;
    };

    /*
    * Unique Geom ID.
    * Default ID = "Default".
    */
    string m_GeomID;

    /*
    * If m_Type is a picking type, m_PickSourceID stores GeomID of 
    * the "Host Geometry".  
    * Picking DrawObjs do not store any data, its job is to set a target
    * geometry to become selectable. The target geometry is the host,
    * and its id goes here.
    */
    string m_PickSourceID;

    /*
    * If m_Type is a picking type, m_FeedbackGroup identifies where the
    * picking feedback is going.  With this, we can have multiple picking
    * sessions active at the same time.
    */
    string m_FeedbackGroup;

    /*
    * Visibility of Geom.  
    * Geometry is visible on true and invisible on false.
    * Visibility is set to true by default.
    */
    bool m_Visible;

    /*
    * Geometry changed flag.  
    * This flag is used for optimization.  Flag true if geometry data are changed and require 
    * graphic buffer update.
    */
    bool m_GeomChanged;

    /*
    * Flip Normals flag.  If true, Normals need to flip to the opposite direction.
    */
    bool m_FlipNormals;

    /*
    * Desired Render type.
    * m_Type is set to VSP_POINTS by default.
    */
    TypeEnum m_Type;

    /*
    * Screen to display this drawObj.
    * m_Screen is set to VSP_MAIN_SCREEN by default.
    */
    ScreenEnum m_Screen;

    /*
     * Holds probe information.
     */
    Probe m_Probe;

    /*
    * Holds ruler information.
    * A ruler requires three steps to complete.  On first
    * step, only start point is given.  Ruler is drawn
    * between start point and mouse location.  On second
    * step, both start and end point are given.  Ruler is
    * drawn between those points.  Lastly, offset is set and
    * ruler is set to appropriate height.
    */
    Ruler m_Ruler;

    /*
    * Line thickness.
    * LineWidth is set to 1.0 by default.
    */
    double m_LineWidth;
    /*
    * RGB Line Color.
    * LineColor is set to blue(0, 0, 1) by default.
    */
    vec3d m_LineColor;

    /*
    * Point Size.
    * PointSize is set to 5.0 by default.
    */
    double m_PointSize;
    /*
    * RGB Point Color.
    * PointColor is set to red(1, 0, 0) by default.
    */
    vec3d m_PointColor;

    /*
    * Text size for label.
    */
    double m_TextSize;
    /*
    * RGB Text Color.
    */
    vec3d m_TextColor;

    /*
    * Array of vertex data.
    * m_PntVec is available if m_Type is one of the following:
    * VSP_POINTS, VSP_LINES, VSP_WIRE_TRIS, VSP_HIDDEN_TRIS,
    * VSP_SHADED_TRIS.
    *
    * The data format is based on m_Type.
    * On VSP_POINTS, data are stored as v0 v1 v2...
    * On VSP_LINES, data are stored as v0 v1 v1 v2...
    * On VSP_WIRE_TRIS, VSP_HIDDEN_TRIS, and VSP_SHADED_TRIS,
    * data are stored as v0, v1, v2...
    */
    vector< vec3d > m_PntVec;
    /*
    * XSec data.
    * m_PntMesh is available if m_Type is one of the following:
    * VSP_WIRE_MESH, VSP_HIDDEN_MESH, VSP_SHADED_MESH, VSP_TEXTURED_MESH
    *
    * Data format:
    * m_PntMesh[pnts on xsec][xsec index]
    */
    vector< vector< vector< vec3d > > > m_PntMesh;
    /*
    * XSec normals.
    * m_NormMesh is available if m_Type is one of the following:
    * VSP_WIRE_MESH, VSP_HIDDEN_MESH, VSP_SHADED_MESH, VSP_TEXTURED_MESH
    *
    * Data format:
    * m_NormMesh[pnts on xsec][xsec index]
    */ 
    vector< vector< vector< vec3d > > > m_NormMesh;
    vector< vec3d > m_NormVec; // For triangles

    vector< vector< vector< double > > > m_uTexMesh;
    vector< vector< vector< double > > > m_vTexMesh;

    /*
    * List of attached textures to this drawobj.  Default is empty.
    */
    vector<TextureInfo> m_TextureInfos;

    /*
    * List of Light Source Information.  Lighting is global, there will be
    * only one drawObj that has m_LightingInfos to avoid redundant processes.
    * Otherwise m_LightingInfos is empty.
    */
    vector<LightSourceInfo> m_LightingInfos;

    /*
    * Material information of this DrawObj.
    */
    MaterialInfo m_MaterialInfo;

    vector< double > m_ClipLoc;
    vector< bool > m_ClipFlag;

protected:

};

void MakeArrowhead( const vec3d &ptip, const vec3d &uref, double len, vector < vec3d > &pts );
void MakeArrowhead( const vec3d &ptip, const vec3d &uref, double len, DrawObj &dobj );

void MakeCircle( const vec3d &pcen, const vec3d &norm, double rad, vector < vec3d > &pts, int nseg = 48 );
void MakeCircle( const vec3d &pcen, const vec3d &norm, const vec3d &pstart, vector < vec3d > &pts, int nseg = 48 );
void MakeCircle( const vec3d &pcen, const vec3d &norm, double rad, DrawObj &dobj );

void MakeCircleArrow( const vec3d &pcen, const vec3d &norm, double rad, DrawObj &dobj, DrawObj &arrow );

void MakeDashedLine( const vec3d &pstart, const vec3d &pend, int ndash, vector < vec3d > &dashpts );

#endif


