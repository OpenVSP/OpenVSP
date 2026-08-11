//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "WireGeom.h"
#include "Vehicle.h"
#include "StringUtil.h"
#include "VspUtil.h"

//==== Constructor ====//
WireGeom::WireGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "WireGeom";
    m_Type.m_Name = "WireFrame";
    m_Type.m_Type = WIRE_FRAME_GEOM_TYPE;

    // Disable Parameters that don't make sense for PtCloudGeom
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();
    m_Density.Deactivate();
    m_ShellFlag.Deactivate();
    m_MassArea.Deactivate();
    m_MassPrior.Deactivate();

    m_PointMass.Deactivate();
    m_CGx.Deactivate();
    m_CGy.Deactivate();
    m_CGz.Deactivate();
    m_Ixx.Deactivate();
    m_Iyy.Deactivate();
    m_Izz.Deactivate();
    m_Ixy.Deactivate();
    m_Ixz.Deactivate();
    m_Iyz.Deactivate();

    m_ScaleMatrix.loadIdentity();
    m_ScaleFromOrig.Init( "Scale_From_Original", "XForm", this, 1, 1.0e-5, 1.0e12 );

    m_WireType.Init( "WireType", "WireFrame", this, 0, 0, 1 );

    m_InvertFlag.Init( "InvertFlag", "Wireframe", this, false, false, true );

    m_ISkipStart.Init( "ISkipStart", "WireFrame", this, 0, 0, 1e6 );
    m_ISkipEnd.Init( "ISkipEnd", "WireFrame", this, 0, 0, 1e6 );
    m_JSkipStart.Init( "JSkipStart", "WireFrame", this, 0, 0, 1e6 );
    m_JSkipEnd.Init( "JSkipEnd", "WireFrame", this, 0, 0, 1e6 );

    m_SwapIJFlag.Init( "FlipIJFlag", "Wireframe", this, false, false, true );
    m_RevIFlag.Init( "RevIFlag", "Wireframe", this, false, false, true );
    m_RevJFlag.Init( "RevJFlag", "Wireframe", this, false, false, true );

    m_IStride.Init( "IStride", "WireFrame", this, 1, 1, 1e3 );
    m_JStride.Init( "JStride", "WireFrame", this, 1, 1, 1e3 );

    m_IStartPatchType.Init( "IStartPatchType", "WireFrame", this, vsp::PATCH_NONE, vsp::PATCH_NONE, vsp::PATCH_NUM_TYPES - 1 );
    m_IEndPatchType.Init( "IEndPatchType", "WireFrame", this, vsp::PATCH_NONE, vsp::PATCH_NONE, vsp::PATCH_NUM_TYPES - 1 );
    m_JStartPatchType.Init( "JStartPatchType", "WireFrame", this, vsp::PATCH_NONE, vsp::PATCH_NONE, vsp::PATCH_NUM_TYPES - 1 );
    m_JEndPatchType.Init( "JEndPatchType", "WireFrame", this, vsp::PATCH_NONE, vsp::PATCH_NONE, vsp::PATCH_NUM_TYPES - 1 );

    m_OtherInvertFlag = false;

    m_MainSurfIndxVec.resize( 1);
    m_MainSurfIndxVec[0] = 0;
    m_SurfCopyIndx.resize( 1 );
    m_SurfCopyIndx[0] = 0;

    m_MainSurfVec.clear();
}

//==== Destructor ====//
WireGeom::~WireGeom()
{
}

void WireGeom::UpdateSurf()
{
    int num_j, num_i;

    m_OtherInvertFlag = false;

    num_i = m_WirePts.size();

    if ( num_i <= 0 )
    {
        m_MainPts.resize(0);
        return;
    }

    num_j = m_WirePts[0].size();

    if ( num_j <= 0 )
    {
        m_MainPts.resize(0);
        return;
    }

    // Everything below rearranges points and keeps them in this Geom's own frame.  Where the
    // Geom sits is applied afterwards, so moving it does not have to rebuild any of this.
    m_MainPts = m_WirePts;

    // Handle swapping I/J.
    if ( m_SwapIJFlag() )
    {
        m_OtherInvertFlag = !m_OtherInvertFlag;

        vector < vector < vec3d > > tmppts;

        tmppts.resize( num_j );
        for ( unsigned int j = 0 ; j < num_j ; j++ )
        {
            tmppts[j].resize( num_i );
        }

        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            for ( unsigned int j = 0 ; j < num_j ; j++ )
            {
                tmppts[j][i] = m_MainPts[i][j];
            }
        }

        double tmp = num_i;
        num_i = num_j;
        num_j = tmp;

        m_MainPts = tmppts;
    }

    // Handle reversing I
    if ( m_RevIFlag() )
    {
        m_OtherInvertFlag = !m_OtherInvertFlag;

        vector < vector < vec3d > > tmppts;

        tmppts.resize( num_i );
        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j );
        }


        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            unsigned int k = num_i - i - 1;
            for ( unsigned int j = 0 ; j < num_j ; j++ )
            {
                tmppts[k][j] = m_MainPts[i][j];
            }
        }

        m_MainPts = tmppts;
    }

    // Handle reversing J
    if ( m_RevJFlag() )
    {
        m_OtherInvertFlag = !m_OtherInvertFlag;

        vector < vector < vec3d > > tmppts;

        tmppts.resize( num_i );
        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j );
        }


        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            for ( unsigned int j = 0 ; j < num_j ; j++ )
            {
                unsigned int k = num_j - j - 1;
                tmppts[i][k] = m_MainPts[i][j];
            }
        }

        m_MainPts = tmppts;
    }

    // Handle skipping.
    if ( m_ISkipStart() != 0 || m_ISkipEnd() != 0 || m_JSkipStart() != 0 || m_JSkipEnd() != 0 )
    {
        num_i = num_i - m_ISkipStart() - m_ISkipEnd();
        num_j = num_j - m_JSkipStart() - m_JSkipEnd();

        if ( num_i <= 0 || num_j <= 0 ) // No surface left
        {
            m_MainPts.resize(0);
            return;
        }

        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i );
        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j );
            for ( unsigned int j = 0 ; j < num_j ; j++ )
            {
                tmppts[i][j] =  m_MainPts[i + m_ISkipStart()][j + m_JSkipStart()];
            }
        }
        m_MainPts = tmppts;
    }

    // Handle I Stride
    if ( m_IStride() != 1 )
    {
        vector < vector < vec3d > > tmppts;

        unsigned int ilast = 0;
        for ( unsigned int i = 0 ; i < num_i ; i = i + m_IStride() )
        {
            tmppts.push_back( m_MainPts[i] );
            ilast = i;
        }
        if ( ilast != num_i - 1 )
        {
            tmppts.push_back( m_MainPts[num_i - 1] );
        }

        m_MainPts = tmppts;
        num_i = m_MainPts.size();
    }

    // Handle J Stride
    if ( m_JStride() != 1 )
    {
        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i );

        unsigned int jlast = 0;
        for ( unsigned int j = 0 ; j < num_j ; j = j + m_JStride() )
        {
            for ( unsigned int i = 0 ; i < num_i ; i++ )
            {
                tmppts[i].push_back( m_MainPts[i][j] );
            }
            jlast = j;
        }
        if ( jlast != num_j - 1 )
        {
            for ( unsigned int i = 0 ; i < num_i ; i++ )
            {
                tmppts[i].push_back( m_MainPts[i][num_j - 1] );
            }
        }

        m_MainPts = tmppts;
        num_j = m_MainPts[0].size();
    }

    if ( m_IStartPatchType() != vsp::PATCH_NONE )
    {
        vector < vec3d > oldrow;
        vector < vec3d > oppositerow;

        vector < vec3d > newrow;

        oldrow = m_MainPts[0];
        oppositerow = m_MainPts[ m_MainPts.size() - 1 ];

        PatchRow( oldrow, oppositerow, m_IStartPatchType(), newrow );

        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i + 1 );
        tmppts[0] = newrow;
        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i+1] = m_MainPts[i];
        }

        m_MainPts = tmppts;
        num_i++;
    }

    if ( m_IEndPatchType() != vsp::PATCH_NONE )
    {
        vector < vec3d > oldrow;
        vector < vec3d > oppositerow;

        vector < vec3d > newrow;

        oldrow = m_MainPts[ m_MainPts.size() - 1 ];
        oppositerow = m_MainPts[0];

        PatchRow( oldrow, oppositerow, m_IEndPatchType(), newrow );

        m_MainPts.push_back( newrow );
        num_i++;
    }

    if ( m_JStartPatchType() != vsp::PATCH_NONE )
    {
        vector < vec3d > oldrow;
        vector < vec3d > oppositerow;

        vector < vec3d > newrow;

        oldrow.resize( num_i );
        oppositerow = oldrow;

        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            oldrow[i] = m_MainPts[i][0];
            oppositerow[i] = m_MainPts[i][num_j - 1];
        }

        PatchRow( oldrow, oppositerow, m_JStartPatchType(), newrow );

        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i );

        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j + 1 );
        }

        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i][0] = newrow[i];

            for ( unsigned int j = 0 ; j < num_j ; j++ )
            {
                tmppts[i][j+1] = m_MainPts[i][j];
            }
        }

        m_MainPts = tmppts;
        num_j++;
    }

    if ( m_JEndPatchType() != vsp::PATCH_NONE )
    {
        vector < vec3d > oldrow;
        vector < vec3d > oppositerow;

        vector < vec3d > newrow;

        oldrow.resize( num_i );
        oppositerow = oldrow;

        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            oldrow[i] = m_MainPts[i][num_j - 1];
            oppositerow[i] = m_MainPts[i][0];
        }

        PatchRow( oldrow, oppositerow, m_JEndPatchType(), newrow );

        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i );

        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j + 1 );
        }

        for ( unsigned int i = 0 ; i < num_i ; i++ )
        {
            for ( unsigned int j = 0 ; j < num_j ; j++ )
            {
                tmppts[i][j] = m_MainPts[i][j];
            }
            tmppts[i][num_j] = newrow[i];
        }

        m_MainPts = tmppts;
        num_j++;
    }

    // Calculate normal vectors.
}

// A lofted Geom keeps its surface in its own coordinates and is placed downstream of
// UpdateSurf, so moving it does not mean lofting it again -- which is why Geom::Update runs
// UpdateSurf only when the surface itself is dirty.  A wireframe has no surface to place:
// UpdateSurf is where the transform is worked into m_XFormPts, and everything downstream --
// the bounding box, the draw objects, the meshes handed to CompGeom and the exporters --
// reads those points.  So moving it does mean building it again, or it stays where it was
// last built.
void WireGeom::UpdateXForm()
{
    Geom::UpdateXForm();

    // Geom::Update calls UpdateSurf right after this when the surface is dirty too.
    if ( !m_SurfDirty )
    {
        UpdateSurf();
    }
}

void WireGeom::PatchRow( const vector < vec3d > &oldrow, const vector < vec3d > &oppositerow, int type, vector < vec3d > &newrow )
{
    if ( type == vsp::PATCH_POINT )
    {
        vec3d cen;
        for ( int i = 0; i < oldrow.size(); i++ )
        {
            cen = cen + oldrow[i];
        }
        cen = cen * 1.0 / oldrow.size();

        newrow.resize( oldrow.size(), cen );
    }
    else if ( type == vsp::PATCH_LINE )
    {
        newrow = oldrow;
        int n = ceil( newrow.size() * 0.5 );
        for ( int i = 0; i < n; i++ )
        {
            int j = newrow.size() - 1 - i;
            vec3d pt = ( newrow[i] + newrow[j] ) * 0.5;
            newrow[i] = pt;
            newrow[j] = pt;
        }
    }
    else if ( type == vsp::PATCH_COPY )
    {
        newrow = oppositerow;
    }
    else if ( type == vsp::PATCH_HALFWAY )
    {
        newrow.resize( oldrow.size() );
        for ( int i = 0; i < oldrow.size(); i++ )
        {
            newrow[i] = ( oldrow[i] + oppositerow[i] ) * 0.5;
        }
    }
}

void WireGeom::UpdateDrawObj()
{
    UpdateXFormPts();

    m_LineDO.m_PntVec.clear();
    m_LineDO.m_GeomChanged = true;

    // A wireframe that is a single row or column of points -- a Plot3D file of curves reads
    // in that way, one block per curve with a j dimension of one -- has no quads to make a
    // mesh out of, and drawing it as one shows nothing.  Draw the polyline instead, and
    // leave no shaded DrawObj for Geom to make a mesh of.
    int nrow = ( int ) m_XFormPts.size();
    int ncol = 0;
    if ( nrow > 0 )
    {
        ncol = ( int ) m_XFormPts[0].size();
    }

    if ( ( nrow == 1 && ncol > 1 ) || ( ncol == 1 && nrow > 1 ) )
    {
        m_WireShadeDrawObj_vec.clear();

        vector < vec3d > line_pnts;
        if ( ncol == 1 )
        {
            line_pnts.reserve( nrow );
            for ( int i = 0; i < nrow; i++ )
            {
                line_pnts.push_back( m_XFormPts[i][0] );
            }
        }
        else
        {
            line_pnts = m_XFormPts[0];
        }

        // VSP_LINES takes a point pair per segment.
        m_LineDO.m_PntVec.reserve( 2 * ( line_pnts.size() - 1 ) );
        for ( int i = 0; i < ( int ) line_pnts.size() - 1; i++ )
        {
            m_LineDO.m_PntVec.push_back( line_pnts[i] );
            m_LineDO.m_PntVec.push_back( line_pnts[i + 1] );
        }
    }
    else
    {
        // Keep the existing DrawObj alive across updates -- assigning the meshes in place
        // reuses their heap allocations from the previous update.
        if ( m_WireShadeDrawObj_vec.size() != 1 )
        {
            m_WireShadeDrawObj_vec.clear();
            m_WireShadeDrawObj_vec.resize( 1 );
        }
        m_WireShadeDrawObj_vec[0].m_FlipNormals = false;
        m_WireShadeDrawObj_vec[0].m_GeomChanged = true;

        m_WireShadeDrawObj_vec[0].m_PntMesh.resize( 1 );
        m_WireShadeDrawObj_vec[0].m_PntMesh[0] = m_XFormPts;
        m_WireShadeDrawObj_vec[0].m_NormMesh.resize( 1 );
        m_WireShadeDrawObj_vec[0].m_NormMesh[0] = m_XFormNorm;

        // Dummy texture coordinates matching the point mesh shape.
        m_WireShadeDrawObj_vec[0].m_uTexMesh.resize( 1 );
        m_WireShadeDrawObj_vec[0].m_uTexMesh[0].resize( m_XFormPts.size() );
        for ( int i = 0; i < m_XFormPts.size(); i++ )
        {
            m_WireShadeDrawObj_vec[0].m_uTexMesh[0][i].assign( m_XFormPts[0].size(), 0.0 );
        }
        m_WireShadeDrawObj_vec[0].m_vTexMesh = m_WireShadeDrawObj_vec[0].m_uTexMesh;
    }

    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
    m_HighlightDrawObj.m_GeomChanged = true;

    //=== Axis ===//
    if ( m_AxisDrawObj_vec.size() != 3 )
    {
        m_AxisDrawObj_vec.clear();
        m_AxisDrawObj_vec.resize( 3 );
    }
    for ( int i = 0; i < 3; i++ )
    {
        m_AxisDrawObj_vec[i].m_PntVec.clear();
        MakeDashedLine( m_AttachOrigin,  m_AttachAxis[i], 4, m_AxisDrawObj_vec[i].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        m_AxisDrawObj_vec[i].m_LineColor = c;
        m_AxisDrawObj_vec[i].m_GeomChanged = true;
    }

    int num_cross = ( int ) m_XFormPts.size();
    if ( num_cross > 0 )
    {
        int ilim = floor( num_cross * 0.5 );

        int num_pnts = ( int ) m_XFormPts[0].size();

        if ( num_pnts > 0 )
        {
            int jlim = floor( num_pnts * 0.5 );

            if ( m_FeatureDrawObj_vec.size() != 2 )
            {
                m_FeatureDrawObj_vec.clear();
                m_FeatureDrawObj_vec.resize( 2 );
            }
            m_FeatureDrawObj_vec[0].m_PntVec.clear();
            m_FeatureDrawObj_vec[1].m_PntVec.clear();

            for ( int i = 0; i < ilim - 1; i++ )
            {
                m_FeatureDrawObj_vec[0].m_PntVec.push_back( m_XFormPts[ i ][0] );
                m_FeatureDrawObj_vec[0].m_PntVec.push_back( m_XFormPts[ i + 1 ][0] );
            }
            m_FeatureDrawObj_vec[0].m_LineColor = vec3d( 1, 0, 0 );
            m_FeatureDrawObj_vec[0].m_LineWidth = 3.0;
            m_FeatureDrawObj_vec[0].m_GeomChanged = true;

            for ( int j = 0; j < jlim - 1; j++ )
            {
                m_FeatureDrawObj_vec[1].m_PntVec.push_back( m_XFormPts[0][ j ] );
                m_FeatureDrawObj_vec[1].m_PntVec.push_back( m_XFormPts[0][ j + 1 ] );
            }
            m_FeatureDrawObj_vec[1].m_LineColor = vec3d( 0, 1, 0 );
            m_FeatureDrawObj_vec[1].m_LineWidth = 3.0;
            m_FeatureDrawObj_vec[1].m_GeomChanged = true;
        }
    }
}

void WireGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Geom::LoadDrawObjs( draw_obj_vec );

    // Empty unless the wireframe is one point wide, in which case it holds the polyline and
    // there is no shaded DrawObj beside it.
    m_LineDO.m_GeomID = m_ID + "Line";
    m_LineDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LineDO.m_Type = DrawObj::VSP_LINES;
    m_LineDO.m_LineWidth = 2.0;
    m_LineDO.m_LineColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
                                  m_GuiDraw.GetWireColor().y() / 255.0,
                                  m_GuiDraw.GetWireColor().z() / 255.0 );
    m_LineDO.m_Visible = GetSetFlag( vsp::SET_SHOWN ) &&
                         m_GuiDraw.GetDisplayType() == vsp::DISPLAY_TYPE::DISPLAY_BEZIER &&
                         m_GuiDraw.GetDrawType() != vsp::DRAW_TYPE::GEOM_DRAW_NONE;

    draw_obj_vec.push_back( &m_LineDO );
}

//==== Get Total Transformation Matrix from Original Points ====//
Matrix4d WireGeom::GetTotalTransMat() const
{
    Matrix4d retMat;
    retMat.initMat( m_ScaleMatrix );
    retMat.postMult( m_ModelMatrix );

    return retMat;
}

void WireGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_ScaleFromOrig *= currentScale;
    m_ScaleMatrix.loadIdentity();
    m_ScaleMatrix.scale( m_ScaleFromOrig() );
    m_LastScale = m_Scale();
}


//==== Placing the rearranged grid, and drawing it ====//

void WireGeom::BuildWireXFormPts( const vector < vector < vec3d > > &main_pts, const Matrix4d &trans,
                                      bool invert,
                                      vector < vector < vec3d > > &xform_pts,
                                      vector < vector < vec3d > > &xform_norm )
{
    unsigned int num_i = main_pts.size();

    if ( num_i <= 0 )
    {
        xform_pts.resize( 0 );
        xform_norm.resize( 0 );
        return;
    }

    unsigned int num_j = main_pts[0].size();

    xform_pts.resize( num_i );
    for ( unsigned int i = 0 ; i < num_i ; i++ )
    {
        xform_pts[i].resize( num_j );
        for ( unsigned int j = 0 ; j < num_j ; j++ )
        {
            xform_pts[i][j] = trans.xform( main_pts[i][j] );
        }
    }

    // Worked out where the points have been placed, so a scale in the transform is accounted for.
    xform_norm.resize( num_i );
    for ( unsigned int i = 0 ; i < num_i ; i++ )
    {
        xform_norm[i].resize( num_j );
        for ( unsigned int j = 0 ; j < num_j ; j++ )
        {
            int inext = clamp( (int)i + 1, 0, (int)num_i - 1 );
            int iprev = clamp( (int)i - 1, 0, (int)num_i - 1 );
            int jnext = clamp( (int)j + 1, 0, (int)num_j - 1 );
            int jprev = clamp( (int)j - 1, 0, (int)num_j - 1 );

            vec3d di = xform_pts[inext][j] - xform_pts[iprev][j];
            vec3d dj = xform_pts[i][jnext] - xform_pts[i][jprev];

            vec3d n = cross( di, dj );
            n.normalize();

            if ( invert )
            {
                n = -1.0 * n;
            }

            xform_norm[i][j] = n;
        }
    }
}

void WireGeom::BuildWireBndBox( const vector < vector < vec3d > > &xform_pts, BndBox &bbox )
{
    bbox.Reset();

    for ( int i = 0 ; i < ( int )xform_pts.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )xform_pts[i].size() ; j++ )
        {
            bbox.Update( xform_pts[i][j] );
        }
    }
}

void WireGeom::BuildWireDrawObjs( const vector < vector < vec3d > > &xform_pts,
                                      const vector < vector < vec3d > > &xform_norm,
                                      vector < DrawObj > &draw_obj_vec )
{
    // Keep the existing DrawObj alive across updates -- assigning the meshes in place reuses
    // their heap allocations from the previous update.
    if ( draw_obj_vec.size() != 1 )
    {
        draw_obj_vec.clear();
        draw_obj_vec.resize( 1 );
    }

    draw_obj_vec[0].m_FlipNormals = false;
    draw_obj_vec[0].m_GeomChanged = true;

    draw_obj_vec[0].m_PntMesh.resize( 1 );
    draw_obj_vec[0].m_PntMesh[0] = xform_pts;
    draw_obj_vec[0].m_NormMesh.resize( 1 );
    draw_obj_vec[0].m_NormMesh[0] = xform_norm;

    // Dummy texture coordinates matching the point mesh shape.
    draw_obj_vec[0].m_uTexMesh.resize( 1 );
    draw_obj_vec[0].m_vTexMesh.resize( 1 );
    draw_obj_vec[0].m_uTexMesh[0].resize( xform_pts.size() );
    draw_obj_vec[0].m_vTexMesh[0].resize( xform_pts.size() );
    for ( int i = 0; i < ( int )xform_pts.size(); i++ )
    {
        draw_obj_vec[0].m_uTexMesh[0][i].resize( xform_pts[i].size(), 0.0 );
        draw_obj_vec[0].m_vTexMesh[0][i].resize( xform_pts[i].size(), 0.0 );
    }
}

void WireGeom::UpdateXFormPts()
{
    m_TransMatVec.resize( 1 );
    m_TransMatVec[0] = GetWireTransMat();

    BuildWireXFormPts( m_MainPts, GetWireTransMat(), GetWireInvert(), m_XFormPts, m_XFormNorm );
}

void WireGeom::UpdateBBox()
{
    UpdateXFormPts();

    BndBox new_box;

    int num_pnts, num_cross;

    num_cross = ( int ) m_XFormPts.size();

    if ( num_cross == 0 )
    {
        new_box.Update( vec3d( 0.0, 0.0, 0.0 ) );
    }

    for ( int i = 0 ; i < num_cross ; i++ )
    {
        num_pnts = ( int ) m_XFormPts[0].size();

        if ( num_pnts == 0 )
        {
            new_box.Update( vec3d( 0.0, 0.0, 0.0 ) );
        }

        for ( int j = 0 ; j < num_pnts ; j++ )
        {
            new_box.Update( m_XFormPts[i][j] );
        }
    }

    if ( new_box != m_BBox )
    {
        m_BbXLen = new_box.GetMax( 0 ) - new_box.GetMin( 0 );
        m_BbYLen = new_box.GetMax( 1 ) - new_box.GetMin( 1 );
        m_BbZLen = new_box.GetMax( 2 ) - new_box.GetMin( 2 );

        m_BbXMin = new_box.GetMin( 0 );
        m_BbYMin = new_box.GetMin( 1 );
        m_BbZMin = new_box.GetMin( 2 );

        m_BBox = new_box;
        m_ScaleIndependentBBox = m_BBox;
    }
}

void WireGeom::ReadP3D( FILE* fp, int ni, int nj, int nk )
{
    m_WirePts.resize( ni );
    for ( int i = 0 ; i < ni ; i++ )
    {
        m_WirePts[i].resize(nj);
    }

    for ( int ix = 0; ix < 3; ix++ )
    {
        for ( int k = 0; k < nk; k++ )
        {
            for ( int j = 0 ; j < nj ; j++ )
            {
                for ( int i = 0 ; i < ni ; i++ )
                {
                    double xi;
                    fscanf( fp, "%lf ", &xi );
                    if ( k == 0 )  // Only store k=0 surface
                    {
                        m_WirePts[i][j].v[ix] = xi;
                    }
                }
            }
        }
    }

    m_InvertFlag = CheckInverted();

    SetDirtyFlag( GeomBase::SURF );
    Update();
}

void WireGeom::ReadXSec( FILE* fp )
{
    char name_str[256];
    float x, y, z;
    int group_num, type;
    int num_pnts, num_cross;

    fgets( name_str, 256, fp );
    sscanf( name_str, "%s", name_str );
    fscanf( fp, " GROUP NUMBER = %d\n", &group_num );
    fscanf( fp, " TYPE = %d\n", &type );
    fscanf( fp, " CROSS SECTIONS = %d\n", &( num_cross ) );
    fscanf( fp, " PTS/CROSS SECTION = %d\n", &( num_pnts ) );

    m_Name = string( name_str );
    StringUtil::change_space_to_underscore( m_Name );
    m_WireType = type;

    //===== Size Cross Vec ====//
    m_WirePts.resize( num_cross );
    for ( int i = 0 ; i < num_cross ; i++ )
    {
        m_WirePts[i].resize( num_pnts );
        for ( int j = 0 ; j < num_pnts ; j++ )
        {
            fscanf( fp, "%f %f %f\n", &x, &y, &z );
            m_WirePts[i][j].set_xyz( x, y, z );
        }
    }

    m_InvertFlag = CheckInverted();

    SetDirtyFlag( GeomBase::SURF );
    Update();
}

//==== Encode XML ====//
xmlNodePtr WireGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );

    int num_cross = m_WirePts.size();
    if ( num_cross > 0 )
    {
        int num_pnts = m_WirePts[0].size();
        if ( num_pnts > 0 )
        {
            xmlNodePtr wire_node = xmlNewChild( node, nullptr, BAD_CAST "WireGeom", nullptr );

            if ( wire_node )
            {
                XmlUtil::AddIntNode( wire_node, "Num_Cross", num_cross );
                XmlUtil::AddIntNode( wire_node, "Num_Pnts", num_pnts );

                xmlNodePtr pt_list_node = xmlNewChild( wire_node, nullptr, BAD_CAST "Pt_List", nullptr );

                for ( int i = 0 ; i < num_cross ; i++ )
                {
                    for ( int j = 0 ; j < num_pnts ; j++ )
                    {
                        XmlUtil::AddVec3dNode( pt_list_node, "Pt", m_WirePts[i][j] );
                    }
                }
            }
        }
    }

    return node;
}

//==== Decode XML ====//
xmlNodePtr WireGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr wire_node = XmlUtil::GetNode( node, "WireGeom", 0 );
    if ( wire_node )
    {
        unsigned int num_cross = XmlUtil::FindInt( wire_node, "Num_Cross", 0 );
        unsigned int num_pnts = XmlUtil::FindInt( wire_node, "Num_Pnts", 0 );

        xmlNodePtr pt_list_node = XmlUtil::GetNode( wire_node, "Pt_List", 0 );
        if ( pt_list_node )
        {
            unsigned int num_pts = XmlUtil::GetNumNames( pt_list_node, "Pt" );

            if ( num_pts == num_cross * num_pnts )
            {
                vector<vec3d> tmppts;
                tmppts.reserve( num_pts );

                xmlNodePtr iter_node = pt_list_node->xmlChildrenNode;
                while ( iter_node != nullptr )
                {
                    if ( !xmlStrcmp(iter_node->name, (const xmlChar *) "Pt") )
                    {
                        vec3d pt = XmlUtil::GetVec3dNode( iter_node );
                        tmppts.push_back( pt );
                    }
                    iter_node = iter_node->next;
                }


                // Transfer vector of points to matrix storage.
                unsigned int k = 0;
                m_WirePts.resize( num_cross );
                for ( unsigned int i = 0; i < num_cross; i++ )
                {
                    m_WirePts[i].resize( num_pnts );

                    for ( unsigned int j = 0; j < num_pnts; j++ )
                    {
                        m_WirePts[i][j] = tmppts[k];
                        k++;
                    }
                }
            }
        }
    }

    return wire_node;
}

bool WireGeom::CheckInverted()
{
    int num_pnts, num_cross;

    num_cross = ( int ) m_WirePts.size();

    if ( num_cross == 0 )
    {
        return false;
    }

    num_pnts = ( int ) m_WirePts[0].size();

    if ( num_pnts == 0 )
    {
        return false;
    }

    // Find approximate center point to improve volume calculation.
    vec3d cen;
    for ( int i = 0 ; i < num_cross; i++ )
    {
        for ( int j = 0 ; j < num_pnts; j++ )
        {
            cen = cen + m_WirePts[i][j];
        }
    }
    cen = cen / ( num_cross * num_pnts );

    // Find approximate volume.  Since bodies are not required to be
    // watertight, this may not be perfectly accurate.  However, we're
    // only interested in the sign of the result.
    double vol = 0;
    for ( int i = 0 ; i < num_cross - 1; i++ )
    {
        for ( int j = 0 ; j < num_pnts - 1; j++ )
        {
            vec3d a = m_WirePts[i][j] - cen;
            vec3d b = m_WirePts[i+1][j] - cen;
            vec3d c = m_WirePts[i+1][j+1] - cen;
            vec3d d = m_WirePts[i][j+1] - cen;

            vol = vol + tetra_volume( a, b, c )
                      + tetra_volume( a, c, d );
        }
    }

    // Check volume sign as indication of orientation.
    if ( vol < 0 )
    {
        return true;
    }
    return false;
}

//==== Create TMesh Vector ====//
vector< TMesh* > WireGeom::CreateTMeshVec( bool skipnegflipnormal, const int & n_ref ) const
{
    vector < TMesh* > tmeshvec;

    int num_pnts, num_cross;

    num_cross = ( int ) m_XFormPts.size();

    if ( num_cross == 0 )
    {
        return tmeshvec;
    }

    num_pnts = ( int ) m_XFormPts[0].size();

    if ( num_pnts == 0 )
    {
        return tmeshvec;
    }

    TMesh*  tMesh = new TMesh();
    int iQuad = 0;
    //==== Convert CrossSections to Triangles ====//
    for ( int i = 1 ; i < num_cross ; i++ )
    {
        for ( int j = 1; j < num_pnts; j++ )
        {
            if ( m_InvertFlag() ^ m_OtherInvertFlag ) // Bitwise XOR
            {
                tMesh->AddTri( m_XFormPts[ i - 1 ][ j - 1 ], m_XFormPts[ i ][ j ], m_XFormPts[ i ][ j - 1 ], iQuad );
                tMesh->AddTri( m_XFormPts[ i - 1 ][ j - 1 ], m_XFormPts[ i - 1 ][ j ], m_XFormPts[ i ][ j ], iQuad );
            }
            else
            {
                tMesh->AddTri( m_XFormPts[ i - 1 ][ j - 1 ], m_XFormPts[ i ][ j - 1 ], m_XFormPts[ i ][ j ], iQuad );
                tMesh->AddTri( m_XFormPts[ i - 1 ][ j - 1 ], m_XFormPts[ i ][ j ], m_XFormPts[ i - 1 ][ j ], iQuad );
            }
            iQuad++;
        }
    }
    tMesh->LoadGeomAttributes( this );

    tmeshvec.push_back( tMesh );
    return tmeshvec;
}

//==== Create Degenerate Geometry ====//
// When preview = true, this simplifies to generate only the
// required degen plate,surface, and subsurface for updating the preview DrawObj vectors
void WireGeom::CreateDegenGeom( vector<DegenGeom> &dgs, bool preview, const int & n_ref )
{
    unsigned int num_pnts, num_cross;

    num_cross = ( int ) m_XFormPts.size();

    if ( num_cross == 0 )
    {
        return;
    }

    num_pnts = ( int ) m_XFormPts[0].size();

    if ( num_pnts == 0 )
    {
        return;
    }

    vector< vector< vec3d > > uwpnts;
    uwpnts.resize( num_cross );

    for ( unsigned int i = 0; i < num_cross; i++ )
    {
        uwpnts[i].resize( num_pnts );
        for ( unsigned int j = 0; j < num_pnts; j++ )
        {
            uwpnts[i][j] = vec3d( i * 1.0 / num_cross, j * 1.0 / num_pnts, 0.0 );
        }
    }

    int surftype = DegenGeom::SURFACE_TYPE;
    if ( m_WireType() == 1 )
    {
        surftype = DegenGeom::BODY_TYPE;
    }

    int cfdsurftype = vsp::CFD_NORMAL;
    if ( m_NegativeVolumeFlag() )
    {
        cfdsurftype = vsp::CFD_NEGATIVE;
    }

    dgs.resize( 1 );
    Geom::CreateDegenGeom( dgs[0], m_XFormPts, m_XFormNorm, uwpnts, false, 0, preview, m_InvertFlag(), surftype, cfdsurftype, nullptr );
}

int WireGeom::GetNumTotalHrmSurfs() const
{
    return 1;
}

void WireGeom::WriteXSecFile( int geom_no, FILE* dump_file )
{
    //==== Write XSec Header ====//
    fprintf( dump_file, "\n" );
    fprintf( dump_file, "%s \n", ( char* ) m_Name.c_str() );
    fprintf( dump_file, " GROUP NUMBER      = %d \n", geom_no );
    fprintf( dump_file, " TYPE              = %d  \n", m_WireType() );         // 1 -- Non Lifting, 0 -- Lifting
    fprintf( dump_file, " CROSS SECTIONS    = %d \n", static_cast<int>( m_XFormPts.size() ) );
    fprintf( dump_file, " PTS/CROSS SECTION = %d \n", static_cast<int>( m_XFormPts[0].size() ) );

    //==== Write XSec Data ====//
    for ( int j = 0 ; j < ( int )m_XFormPts.size() ; j++ )
    {
        for ( int k = 0 ; k < ( int )m_XFormPts[j].size() ; k++ )
        {
            fprintf( dump_file, "%25.17e  %25.17e  %25.17e\n", m_XFormPts[j][k].x(), m_XFormPts[j][k].y(), m_XFormPts[j][k].z() );
        }
    }
}
