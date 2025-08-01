
#define _USE_MATH_DEFINES
#include <cmath>

#include "DegenGeom.h"
#include "Geom.h"
#include "WriteMatlab.h"
#include "StringUtil.h"

#include "Vehicle.h"

void DegenGeom::build_trans_mat( vec3d x, vec3d y, vec3d z, const vec3d &p, Matrix4d &mat, Matrix4d &invmat )
{
    // Initialize transformation matrix as identity.
    mat.loadIdentity();

    // Input vectors are assumed to be unit vectors.
    // Their magnitude will equal one unless it is the zero vector.
    if ( x.mag() < 0.5 || y.mag() < 0.5 || z.mag() < 0.5 )  // At least one input vector is degenerate.
    {
        if( x.mag() > 0.5 && y.mag() < 0.5 ) // X is OK, Y is degenerate
        {
            int imin = x.minor_comp();
            y.v[imin] = 1.0;
            z = cross( x, y );
            z.normalize();
            y = cross( z, x );
            y.normalize();
        }
        else if( x.mag() < 0.5 && y.mag() > 0.5 ) // X is degenerate, Y is OK
        {
            int imin = y.minor_comp();
            x.v[imin] = 1.0;
            z = cross( x, y );
            z.normalize();
            x = cross( y, z );
            x.normalize();
        }
        else
        {
            x = vec3d( 1, 0, 0 );
            y = vec3d( 0, 1, 0 );
            z = vec3d( 0, 0, 1 );
        }
    }

    // Build rotation part of matrix.
    for( int i = 0; i < 3; i++ )
    {
        mat.data()[ 0 + ( i * 4 ) ] = x.v[i];
        mat.data()[ 1 + ( i * 4 ) ] = y.v[i];
        mat.data()[ 2 + ( i * 4 ) ] = z.v[i];

        invmat.data()[ i + ( 0 * 4 ) ] = x.v[i];
        invmat.data()[ i + ( 1 * 4 ) ] = y.v[i];
        invmat.data()[ i + ( 2 * 4 ) ] = z.v[i];
    }

    // Build translation part of matrix.
    vec3d offset = mat.xform( p );

    for( int i = 0; i < 3; i++ )
    {
        mat.data()[ i + ( 3 * 4 )  ] = -offset.v[i];
        invmat.data()[ i + ( 3 * 4 )  ] = p.v[i];
    }
}

void DegenGeom::build_basis( const int &startPnt, const vector < vec3d > &sect, vec3d &v1, vec3d &v2, vec3d &v3 )
{
    int n_halfpts = ( num_pnts + 1 ) / 2;
    int i_half = n_halfpts - 1;
    int i_quarter = ( ( num_pnts + 1 ) / 4 ) - 1;

    // Vector along chord from te to le.
    v1 = sect[ startPnt % ( num_pnts - 1 ) ] - sect[ ( startPnt + i_half ) % ( num_pnts - 1 ) ];
    v1.normalize();

    // Vector from 1/4 to 3/4 points around section.
    vec3d v = ( sect[ ( startPnt + i_quarter ) % ( num_pnts - 1 ) ] - sect[ ( startPnt + i_half + i_quarter ) % ( num_pnts - 1 ) ]  );
    v.normalize();

    v3 = cross( v1, v );
    v3.normalize();

    v2 = cross( v3, v1 );
    v2.normalize();
}


void DegenGeom::transform_section( const int &startPnt, vector < vec3d > &sect, Matrix4d &trans, Matrix4d &invtrans )
{
    vec3d chordVec;
    vec3d areaNormal;
    vec3d up;

    int n_halfpts = ( num_pnts + 1 ) / 2;
    int i_half = n_halfpts - 1;

    build_basis( startPnt, sect, chordVec, up, areaNormal );

    vec3d p = sect[ ( startPnt + i_half ) % ( num_pnts - 1 ) ];

    build_trans_mat( chordVec, up, areaNormal, p, trans, invtrans );

    for( int i = 0; i < num_pnts; i++ )
    {
        sect[i] = trans.xform( sect[i] );
    }
}

void DegenGeom::calculate_section_prop( const vector < vec3d > &sect, double &len, double &area, vec3d &xcgshell, vec3d &xcgsolid, vector < double > &Ishell, vector < double > &Isolid )
{
    double dl, da;
    double c11, c22, c12;
    double I22shell = 0, I11shell = 0, I12shell = 0;
    double I22solid = 0, I11solid = 0, I12solid = 0;

    vec3d c1;

    len = 0;
    area = 0;
    xcgshell = vec3d( 0, 0, 0 );
    xcgsolid = vec3d( 0, 0, 0 );
    Ishell.clear();
    Isolid.clear();

    int n = sect.size();

    // Cross section should be planar.  Assume Z=0.

    for( int j = 0; j < n; j++ )
    {
        int jnext = j + 1;
        if( jnext >= n ) // Final segment wrap to first segment.
        {
            jnext = 0;
        }

        dl = dist( sect[j], sect[jnext] );
        da = sect[j].x() * sect[jnext].y() - sect[jnext].x() * sect[j].y();

        len += dl;
        area += da;

        c1 = sect[j] + sect[jnext];
        xcgshell = xcgshell + c1 * dl;
        xcgsolid = xcgsolid + c1 * da;

        c11 = ( ( sect[j].y() * sect[j].y() ) + ( sect[j].y() * sect[jnext].y() ) + ( sect[jnext].y() * sect[jnext].y() ) );
        c22 = ( ( sect[j].x() * sect[j].x() ) + ( sect[j].x() * sect[jnext].x() ) + ( sect[jnext].x() * sect[jnext].x() ) );
        c12 = ( ( 2.0 * sect[j].x() * sect[j].y() ) + ( sect[j].x() * sect[jnext].y() ) + ( sect[jnext].x() * sect[j].y() ) + ( 2.0 * sect[jnext].x() * sect[jnext].y() ) );

        I11shell += c11 * dl;
        I22shell += c22 * dl;
        I12shell += c12 * dl;

        I11solid += c11 * da;
        I22solid += c22 * da;
        I12solid += c12 * da;
    }

    if( std::abs( len ) < 1e-6 )
    {
        xcgshell = vec3d( 0, 0, 0 );

        I11shell = 0.0;
        I22shell = 0.0;
        I12shell = 0.0;
    }
    else
    {
        xcgshell = xcgshell / ( 2.0 * len );

        I11shell /= ( 3.0 * len );
        I22shell /= ( 3.0 * len );
        I12shell /= ( 6.0 * len );
    }

    if( std::abs( area ) < 1e-6 )
    {
        area = 0.0;

        xcgsolid = vec3d( 0, 0, 0 );

        I11solid = 0.0;
        I22solid = 0.0;
        I12solid = 0.0;
    }
    else
    {
        area /= 2.0;

        xcgsolid = xcgsolid / ( 6.0 * area );

        I11solid /= ( 12.0 * area );
        I22solid /= ( 12.0 * area );
        I12solid /= ( 24.0 * area );
    }

    I11shell = ( I11shell - ( xcgshell.y() * xcgshell.y() ) ) * len;
    I22shell = ( I22shell - ( xcgshell.x() * xcgshell.x() ) ) * len;
    I12shell = ( I12shell - ( xcgshell.x() * xcgshell.y() ) ) * len;

    I11solid = ( I11solid - ( xcgsolid.y() * xcgsolid.y() ) ) * area;
    I22solid = ( I22solid - ( xcgsolid.x() * xcgsolid.x() ) ) * area;
    I12solid = ( I12solid - ( xcgsolid.x() * xcgsolid.y() ) ) * area;

    Ishell.push_back( I11shell );
    Ishell.push_back( I22shell );
    Ishell.push_back( I12shell );

    Isolid.push_back( I11solid );
    Isolid.push_back( I22solid );
    Isolid.push_back( I12solid );
}

void DegenGeom::createDegenSurface( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, bool flipnormal )
{
    int nLow = 0, nHigh = num_xsecs;

    vec3d nVec;

    unsigned int nxs = nHigh - nLow;
    degenSurface.x.resize( nxs );

    for ( int i = nLow; i < nHigh; i++ )
    {
        degenSurface.x[i - nLow].resize( num_pnts );
        for ( int j = 0; j < num_pnts; j++ )
        {
            degenSurface.x[i - nLow][j] = pntsarr[ i ][ j ];
        }
    }

    degenSurface.nvec.resize( nxs - 1 );
    degenSurface.area.resize( nxs - 1 );
    for ( int i = 0; i < nxs - 1; i++ )
    {
        degenSurface.nvec[i].resize( num_pnts - 1 );
        degenSurface.area[i].resize( num_pnts - 1 );
        for ( int j = 0; j < num_pnts - 1; j++ )
        {
            vec3d sVec1 = degenSurface.x[i + 1][j] - degenSurface.x[i][j];
            vec3d sVec2 = degenSurface.x[i][j + 1] - degenSurface.x[i][j];

            vec3d sVec3 = degenSurface.x[i + 1][j + 1] - degenSurface.x[i + 1][j];
            vec3d sVec4 = degenSurface.x[i + 1][j + 1] - degenSurface.x[i][j + 1];

            // Take areas of both triangles for quad.
            degenSurface.area[i][j] = 0.5 * ( cross( sVec1, sVec2 ).mag() + cross( sVec3, sVec4 ).mag() );

            vec3d v1, v2;

            if( sVec1.mag() > sVec4.mag() )
            {
                v1 = sVec1;
            }
            else
            {
                v1 = sVec4;
            }

            if( sVec2.mag() > sVec3.mag() )
            {
                v2 = sVec2;
            }
            else
            {
                v2 = sVec3;
            }

            // Approximate normal as normal for two longest sides.
            nVec = cross( v1, v2 );
            if( flipnormal )
            {
                nVec = nVec * -1.0;
            }

            nVec.normalize();

            degenSurface.nvec[i][j] = nVec;
        }
    }

    degenSurface.u.resize( nxs );
    degenSurface.w.resize( nxs );
    for ( int i = nLow; i < nHigh; i++ )
    {
        degenSurface.u[i - nLow].resize( num_pnts );
        degenSurface.w[i - nLow].resize( num_pnts );
        for ( int j = 0; j < num_pnts; j++ )
        {
            degenSurface.u[i - nLow][j] = uw_pnts[i][j].x();
            degenSurface.w[i - nLow][j] = uw_pnts[i][j].y();
        }
    }
}

void DegenGeom::createSurfDegenPlate( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts )
{
    int nLow = 0, nHigh = num_xsecs;
    int startPnt = 0;

    degenPlates.resize( 1 );
    createDegenPlate( degenPlates[0], pntsarr, uw_pnts, nLow, nHigh, startPnt );
}

void DegenGeom::createBodyDegenPlate( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts )
{
    int nLow = 0, nHigh = num_xsecs;

    degenPlates.resize( 2 );

    int startPnt = 0;
    createDegenPlate( degenPlates[0], pntsarr, uw_pnts, nLow, nHigh, startPnt );

    startPnt = ( num_pnts - 1 ) / 4;
    createDegenPlate( degenPlates[1], pntsarr, uw_pnts, nLow, nHigh, startPnt );
}

void DegenGeom::createDegenPlate( DegenPlate &degenPlate, const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, int nLow, int nHigh, int startPnt )
{
    unsigned int platePnts = ( num_pnts + 1 ) / 2;

    int nxsec = nHigh - nLow;

    degenPlate.x.resize( nxsec );
    degenPlate.xCamber.resize( nxsec );
    degenPlate.nCamber.resize( nxsec );
    degenPlate.t.resize( nxsec );
    degenPlate.zcamber.resize( nxsec );
    degenPlate.u.resize( nxsec );
    degenPlate.wTop.resize( nxsec );
    degenPlate.wBot.resize( nxsec );
    degenPlate.nPlate.resize( nxsec );

    vec3d  lePnt, tePnt, topPnt, botPnt, camberPnt, chordPnt, nPlate;

    int ixsec = 0;
    for ( int i = nLow; i < nHigh; i++ )
    {
        vector<vec3d> &xVec = degenPlate.x[ ixsec ];          xVec.resize( platePnts );
        vector<vec3d> &xCVec = degenPlate.xCamber[ ixsec ];   xCVec.resize( platePnts );
        vector<vec3d> &nCVec = degenPlate.nCamber[ ixsec ];   nCVec.resize( platePnts );
        vector<double> &tVec = degenPlate.t[ ixsec ];         tVec.resize( platePnts );
        vector<double> &zVec = degenPlate.zcamber[ ixsec ];   zVec.resize( platePnts );

        lePnt = pntsarr[ i ][ startPnt ];
        tePnt = pntsarr[ i ][ startPnt + platePnts - 1 ];

        // Set first point (trailing edge)
        xVec[0]  = lePnt;
        xCVec[0]  = lePnt;
        nCVec[0] = vec3d( 0, 0, 0 ); // on camber line
        tVec[0]  = 0;
        zVec[0]  = 0;

        // Set last point (leading edge)
        xVec[platePnts - 1]  = tePnt;
        xCVec[platePnts - 1]  = tePnt;
        nCVec[platePnts - 1] = vec3d( 0, 0, 0 ); // on camber line
        tVec[platePnts - 1]  = 0;
        zVec[platePnts - 1]  = 0;


        vector < vec3d > section( num_pnts );
        for ( int j = 0; j < num_pnts; j++ )
        {
            section[j] = pntsarr[ i ][ j ];
        }

        vec3d chordVec, anv, up;
        build_basis( startPnt, section, chordVec, up, anv );

        //== Compute plate normal ==//
        // rotated chord vector
        vec3d rcv = xVec[platePnts - 1] - xVec[0];
        // plate normal vector
        nPlate = cross( anv, rcv );
        nPlate.normalize();
        degenPlate.nPlate[ ixsec ] = nPlate;

        // normalized, unrotated chord vector (te->le)
        chordVec = tePnt - lePnt;
        chordVec.normalize();

        for ( int j = 1, k = startPnt + num_pnts - 2; j < platePnts - 1; j++, k-- )
        {
            botPnt = pntsarr[ i ][ startPnt + j ];
            topPnt = pntsarr[ i ][ k % ( num_pnts - 1 ) ];

            camberPnt = ( topPnt + botPnt ) / 2;
            xCVec[j] = camberPnt;

            nCVec[j] = topPnt - botPnt; // vector from bottom point to top point.
            nCVec[j].normalize();

            tVec[j]  = dist( topPnt, botPnt );

            // Try finding least-squares minimum distance point.
            double s, t;
            bool intflag = line_line_intersect( botPnt, topPnt, lePnt, tePnt, &s, &t );
            if( intflag )
            {
                chordPnt = lePnt + ( tePnt - lePnt ) * t;
            }
            else  // If it doesn't work, project.  Projection isn't quite right, but should be close.  This should never happen anyway.
            {
                chordPnt  = lePnt + chordVec * dot( camberPnt - lePnt, chordVec );
            }

            xVec[j]  = chordPnt;

            // This portion of the code implicitly assumes each section is flat.  For
            // bodies with extreme skinning, this is likely not the case and unexpected results
            // may occur.

            vec3d zv = camberPnt - chordPnt;
            zVec[j] = zv.mag();

            if ( dot( zv, nCVec[j] ) <= -1e-12 )
            {
                zVec[j] *= -1;
            }
        }

        ixsec++;
    }

    ixsec = 0;
    for ( int i = nLow; i < nHigh; i++ )
    {
        vector<double> &uVec = degenPlate.u[ixsec];           uVec.resize( platePnts );
        vector<double> &wTopVec = degenPlate.wTop[ixsec];     wTopVec.resize( platePnts );
        vector<double> &wBotVec = degenPlate.wBot[ixsec];     wBotVec.resize( platePnts );

        for ( int j = 0, k = startPnt + num_pnts - 1; j < platePnts; j++, k-- )
        {
            uVec[j] = uw_pnts[i][j].x();
            wBotVec[j] = uw_pnts[i][ startPnt + j ].y();
            if( k == ( num_pnts - 1 ) )
            {
                wTopVec[j] = uw_pnts[i][ num_pnts - 1 ].y();
            }
            else
            {
                wTopVec[j] = uw_pnts[i][ k % ( num_pnts - 1 ) ].y();
            }
        }

        ixsec++;
    }
}

void DegenGeom::createSurfDegenStick( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, const VspSurf *foilSurf, const bool &urootcap )
{
    int nLow = 0, nHigh = num_xsecs;

    degenSticks.resize( 1 );

    int startPnt = 0;
    createDegenStick( degenSticks[0], pntsarr, uw_pnts, nLow, nHigh, startPnt );

    if ( foilSurf ) // Calculate surface based exact values
    {
        augmentFoilSurfDegenStick( degenSticks[0], foilSurf, uw_pnts, urootcap );
    }
    else // Calculate discrete approximations
    {
        augmentFoilSurfDegenStick( degenSticks[0], pntsarr, uw_pnts, urootcap );
    }

}

void DegenGeom::createBodyDegenStick( const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts )
{
    int nLow = 0, nHigh = num_xsecs;

    degenSticks.resize( 2 );

    int startPnt = 0;
    createDegenStick( degenSticks[0], pntsarr, uw_pnts, nLow, nHigh, startPnt );

    startPnt = ( num_pnts - 1 ) / 4;
    createDegenStick( degenSticks[1], pntsarr, uw_pnts, nLow, nHigh, startPnt );
}


void DegenGeom::createDegenStick( DegenStick &degenStick, const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, int nLow, int nHigh, int startPnt )
{
    unsigned int platePnts = ( num_pnts + 1 ) / 2;
    vec3d camberPnt;

    for ( int i = nLow; i < nHigh; i++ )
    {
        vec3d topPnt, botPnt;
        double tempThickness = -1, perimTop = 0, perimBot = 0;
        int    maxThickIdx[2] = {0, 0};

        vector < vec3d > section( num_pnts );
        for ( int j = 0; j < num_pnts; j++ )
        {
            section[j] = pntsarr[ i ][ j ];
        }

        vec3d chordVec, areaNormal, up;
        build_basis( startPnt, section, chordVec, up, areaNormal );
        degenStick.sectnvec.push_back( areaNormal );

        // Transform section to XY plane, with LE at origin and TE on +X-axis.
        // Also return transformation matrix from 3D space to XY plane as sect_trans.
//        double sect_trans[4][4], sect_inv_trans[4][4];
        Matrix4d sect_trans, sect_inv_trans;
        transform_section( startPnt, section, sect_trans, sect_inv_trans );

        // Reshape matrix to vector and store in degenStick
        vector < double > tmatvec, invtmatvec;
        for ( int j = 0; j < 16; j++ )
        {
            tmatvec.push_back( sect_trans.data()[j] );
            invtmatvec.push_back( sect_inv_trans.data()[j] );
        }
        degenStick.transmat.push_back( tmatvec );
        degenStick.invtransmat.push_back( invtmatvec );

        // normalized, unrotated chord vector (te->le)
        chordVec = pntsarr[ i ][ startPnt + platePnts - 1 ] - pntsarr[ i ][ startPnt ];
        chordVec.normalize();

        degenStick.xle.push_back( pntsarr[ i ][ startPnt + platePnts - 1 ] );
        degenStick.xte.push_back( pntsarr[ i ][ startPnt ] );

        double chord = dist( pntsarr[ i ][ startPnt + platePnts - 1 ], pntsarr[ i ][ startPnt ] );
        degenStick.chord.push_back( chord );

        degenStick.u.push_back( uw_pnts[i][0].x() );

        double len, area;
        vec3d xcgshell, xcgsolid;
        vector < double > Ishell, Isolid;
        calculate_section_prop( section, len, area, xcgshell, xcgsolid, Ishell, Isolid );

        xcgshell = sect_inv_trans.xform( xcgshell );
        xcgsolid = sect_inv_trans.xform( xcgsolid );

        degenStick.Ishell.push_back( Ishell );
        degenStick.Isolid.push_back( Isolid );
        degenStick.xcgShell.push_back( xcgshell );
        degenStick.xcgSolid.push_back( xcgsolid );
        degenStick.sectarea.push_back( area );

        for ( int j = 1, k = startPnt + num_pnts - 2; j < platePnts - 1; j++, k-- )
        {
            botPnt = pntsarr[ i ][ startPnt + j ];
            topPnt = pntsarr[ i ][ k % ( num_pnts - 1 ) ];

            camberPnt = ( topPnt + botPnt ) / 2;

            if( dist( topPnt, botPnt ) > tempThickness )
            {
                tempThickness  = dist( topPnt, botPnt );
                maxThickIdx[0] = j + startPnt;
                maxThickIdx[1] = k % ( num_pnts - 1 );
            }
            perimBot += dist( pntsarr[ i][ startPnt + j ], pntsarr[ i ][ startPnt + j - 1 ] );
            perimTop += dist( pntsarr[ i][ k % ( num_pnts - 1 ) ], pntsarr[ i ][ k % ( num_pnts - 1 ) + 1  ] );
        }

        camberPnt = ( pntsarr[ i ][ maxThickIdx[0] ] + pntsarr[ i ][ maxThickIdx[1] ] ) / 2;

        if( chord > 0 )
        {
            degenStick.tLoc.push_back( 1 - ( dot( camberPnt - pntsarr[ i ][ startPnt], chordVec ) / chord ) );
            degenStick.toc.push_back( tempThickness / chord );
        }
        else
        {
            degenStick.tLoc.push_back( 0.0 );
            degenStick.toc.push_back( 0.0 );
        }

        perimBot += dist( pntsarr[ i ][ startPnt + platePnts - 1 ], pntsarr[ i ][ startPnt + platePnts - 2 ] );
        perimTop += dist( pntsarr[ i ][ startPnt + platePnts ], pntsarr[ i ][ startPnt + platePnts - 1 ] );
        degenStick.perimTop.push_back( perimTop );
        degenStick.perimBot.push_back( perimBot );

        // Pad values to be calculated later for wings using foilSurf.
        degenStick.toc2.push_back( 0.0 );
        degenStick.tLoc2.push_back( 0.0 );
        degenStick.anglele.push_back( 0.0 );
        degenStick.anglete.push_back( 0.0 );
        degenStick.radleTop.push_back( 0.0 );
        degenStick.radleBot.push_back( 0.0 );
    }

    // Calculate sweep angle
    for( int i = nLow; i < nHigh - 1; i++ )
    {
        vec3d xle0 = pntsarr[ i ][ startPnt + platePnts - 1 ];
        vec3d xte0 = pntsarr[ i ][ startPnt ];

        vec3d xle1 = pntsarr[ i + 1][ startPnt + platePnts - 1 ];
        vec3d xte1 = pntsarr[ i + 1][ startPnt ];

        vec3d vle = xle1 - xle0;
        vle.normalize();

        vec3d vte = xte1 - xte0;
        vte.normalize();

        vec3d vchd = xte0 - xle0;
        vchd.normalize();

        vec3d vdownstream( 1, 0, 0 );

        if( vchd.mag() > 0.5 )
        {
            vec3d n = cross( vchd, vle );
            n.normalize();

            vec3d downNormal = cross( n, vdownstream  );
            downNormal.normalize();

            if( downNormal.mag() > 0.5 && vle.mag() > 0.5 )
            {
                degenStick.sweeple.push_back( ( 180.0 / M_PI ) * signed_angle( downNormal, vle, n * -1.0 ) );
            }
            else
            {
                degenStick.sweeple.push_back( 0.0 );
            }

            if( downNormal.mag() > 0.5 && vte.mag() > 0.5 )
            {
                degenStick.sweepte.push_back( ( 180.0 / M_PI ) * signed_angle( downNormal, vte, n * -1.0 ) );
            }
            else
            {
                degenStick.sweepte.push_back( 0.0 );
            }
        }
        else
        {
            vec3d vref1 = cross( vdownstream, vle );
            vec3d vref2 = cross( vref1, vdownstream );
            degenStick.sweeple.push_back( ( 180.0 / M_PI ) * signed_angle( vref2, vle, vref1 * -1.0 ) );

            vref1 = cross( vdownstream, vte );
            vref2 = cross( vref1, vdownstream );
            degenStick.sweepte.push_back( ( 180.0 / M_PI ) * signed_angle( vref2, vte, vref1 * -1.0 ) );
        }



        double areaTop = 0, areaBot = 0;
        for ( int j = 0, k = startPnt + num_pnts - 2; j < platePnts - 1; j++, k-- )
        {
            areaBot += degenSurface.area[i - nLow][( startPnt + j ) % ( num_pnts - 1 )];
            areaTop += degenSurface.area[i - nLow][k % ( num_pnts - 1 )];
        }
        degenStick.areaTop.push_back( areaTop );
        degenStick.areaBot.push_back( areaBot );
    }
}

void DegenGeom::augmentFoilSurfDegenStick( DegenStick &degenStick, const VspSurf *foilSurf, const vector< vector< vec3d > > &uw_pnts, const bool &urootcap )
{

    for ( int i = 0; i < uw_pnts.size(); i++ )
    {
        double u = uw_pnts[i][0].x();

        if( urootcap ) // Shift u because foilSurf isn't capped.
        {
            u = u - 1.0;
        }

        VspCurve c;
        foilSurf->GetUConstCurve( c, u );

        double tloc;
        double t = c.CalculateThick( tloc );

        double te_angle = c.Angle( TMAGIC, VspCurve::AFTER, 4.0 - TMAGIC, VspCurve::BEFORE, true ) * 180.0 / M_PI;
        double le_angle = c.Angle( 2.0 + TMAGIC, VspCurve::AFTER, 2.0-TMAGIC, VspCurve::BEFORE, true ) * 180.0 / M_PI;

        double le_crv_low = c.CompCurve( 2.0 - TMAGIC, VspCurve::BEFORE );
        double le_crv_up = c.CompCurve( 2.0 + TMAGIC, VspCurve::AFTER );

        degenStick.toc2[i] = t;
        degenStick.tLoc2[i] =  tloc;
        degenStick.anglele[i] =  le_angle;
        degenStick.anglete[i] =  te_angle;
        degenStick.radleTop[i] =  1.0/le_crv_up;
        degenStick.radleBot[i] =  1.0/le_crv_low;
    }
}

void DegenGeom::augmentFoilSurfDegenStick( DegenStick &degenStick, const vector< vector< vec3d > > &pntsarr, const vector< vector< vec3d > > &uw_pnts, const bool &urootcap )
{
    int platePnts = ( num_pnts + 1 ) / 2;
    int jle = platePnts - 1;
    int jte0 = 0;
    int jte1 = num_pnts - 1;

    for ( int i = 0; i < uw_pnts.size(); i++ )
    {
        double u = uw_pnts[i][0].x();

        if( urootcap ) // Shift u because foilSurf isn't capped.
        {
            u = u - 1.0;
        }

        vector < vec3d > section( num_pnts );
        for ( int j = 0; j < num_pnts; j++ )
        {
            section[j] = pntsarr[ i ][ j ];
        }

        vec3d ute0 = pntsarr[ i ][ jte0 + 1 ] - pntsarr[ i ][ jte0 ];
        vec3d ute1 = pntsarr[ i ][ jte1 - 1 ] - pntsarr[ i ][ jte1 ];

        double te_angle = angle( ute0, ute1 ) * 180.0 / M_PI;

        vec3d ule0 = pntsarr[ i ][ jle + 1 ] - pntsarr[ i ][ jle ];
        vec3d ule1 = pntsarr[ i ][ jle - 1 ] - pntsarr[ i ][ jle ];

        double le_angle = angle( ule0, ule1 ) * 180.0 / M_PI;

        double le_rad = radius_of_circle( pntsarr[ i ][ jle + 1 ], pntsarr[ i ][ jle ], pntsarr[ i ][ jle - 1 ] );
        le_rad = le_rad / degenStick.chord[i];

        degenStick.toc2[i] = degenStick.toc[i];
        degenStick.tLoc2[i] = degenStick.tLoc[i];
        degenStick.anglele[i] = le_angle;
        degenStick.anglete[i] = te_angle;
        degenStick.radleTop[i] = le_rad;
        degenStick.radleBot[i] = le_rad;
    }
}


void DegenGeom::createDegenDisk(  const vector< vector< vec3d > > &pntsarr, bool flipnormal )
{
    vec3d origin = pntsarr[0][0];
    double r = 0;
    int imax = 0;
    for( int i = 1; i < num_xsecs; i++ )
    {
        vec3d p = pntsarr[i][0];
        double d = dist( origin, p );
        if( d > r )
        {
            r = d;
            imax = i;
        }
    }

    int j = ( num_pnts - 1 ) / 4;

    vec3d p = pntsarr[imax][0];
    vec3d q = pntsarr[imax][j];

    vec3d u = p - origin;
    vec3d v = q - origin;

    vec3d n = cross( u, v );
    n.normalize();

    if( flipnormal )
    {
        n = n * -1.0;
    }

    degenDisk.nvec = n;
    degenDisk.x = origin;
    degenDisk.d = 2.0 * r;
}

void DegenGeom::addDegenSubSurf( SubSurface *ssurf, int surfIndx )
{
    ssurf->UpdatePolygonPnts();

    std::vector< std::vector< vec2d > > ppvec = ssurf->GetPolyPntsVec();

    Vehicle * veh;
    veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        string ssurfParentGeomId = ssurf->GetCompID();
        Geom *ssurfParentGeom = veh->FindGeom( ssurfParentGeomId );
        if ( ssurfParentGeom )
        {
            string ssurfParentName = ssurfParentGeom->GetName();

            const VspSurf *surf = ssurfParentGeom->GetSurfPtr( surfIndx );

            for ( int i = 0; i < ppvec.size(); i++ )
            {
                DegenSubSurf dgss;
                dgss.name = ssurf->GetName();
                dgss.fullName = ssurfParentName + "_Surf" + std::to_string( surfIndx ) + "_" + dgss.name;
                dgss.typeName = ssurf->GetTypeName(ssurf->GetType());
                dgss.typeId = (vsp::SUBSURF_TYPE)ssurf->GetType();
                dgss.testType = ssurf->m_TestType();

                int npt = ppvec[i].size();

                dgss.u.resize( npt );
                dgss.w.resize( npt );
                dgss.x.resize( npt );

                for ( int j = 0; j < ppvec[i].size(); j++ )
                {
                    if ( ppvec[i][j].x() > ssurfParentGeom->GetUMax( surfIndx ) )
                    {
                        dgss.u[j] = ssurfParentGeom->GetUMax( surfIndx );
                    }
                    else if ( ppvec[i][j].x() < 0 )
                    {
                        dgss.u[j] = 0;
                    }
                    else
                    {
                        dgss.u[j] = ppvec[i][j].x();
                    }

                    if ( ppvec[i][j].y() > ssurfParentGeom->GetWMax( surfIndx ) )
                    {
                        dgss.w[j] = ssurfParentGeom->GetWMax( surfIndx );
                    }
                    else if ( ppvec[i][j].y() < 0 )
                    {
                        dgss.w[j] = 0;
                    }
                    else
                    {
                        dgss.w[j] = ppvec[i][j].y();
                    }

                    dgss.x[j] = surf->CompPnt( dgss.u[j], dgss.w[j] );
                }

                degenSubSurfs.push_back( dgss );
            }
        }
    }
}

void DegenGeom::addDegenHingeLine( SSControlSurf *csurf, int surfIndx )
{
    Vehicle * veh;
    veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        string ssurfParentGeomId = csurf->GetCompID();
        Geom *ssurfParentGeom = veh->FindGeom( ssurfParentGeomId );
        if ( ssurfParentGeom )
        {
            const VspSurf *surf = ssurfParentGeom->GetSurfPtr( surfIndx );

            DegenHingeLine dghl;

            dghl.name = csurf->GetName();

            int npt = csurf->m_UWStart.size();

            dghl.uStart.resize( npt );
            dghl.uEnd.resize( npt );
            dghl.wStart.resize( npt );
            dghl.wEnd.resize( npt );
            dghl.xStart.resize( npt );
            dghl.xEnd.resize( npt );

            for ( int i = 0; i < npt; i++ )
            {
                dghl.uStart[i] = csurf->m_UWStart[i].x();
                dghl.uEnd[i] = csurf->m_UWEnd[i].x();
                dghl.wStart[i] = csurf->m_UWStart[i].y();
                dghl.wEnd[i] = csurf->m_UWEnd[i].y();
                dghl.xStart[i] = surf->CompPnt( dghl.uStart[i], dghl.wStart[i] );
                dghl.xEnd[i] = surf->CompPnt( dghl.uEnd[i], dghl.wEnd[i] );
            }

            degenHingeLines.push_back( dghl );
        }
    }
}

string DegenGeom::makeCsvFmt( int n, bool newline )
{
    char fmt[10];
    snprintf( fmt, sizeof( fmt ), "%%.%de", DBL_DIG + 3 );

    string fmtstring = "";
    for( int i = 0; i < n; ++i )
    {
        fmtstring.append( fmt );
        if( i < n - 1 )
        {
            fmtstring.append( ", " );
        }
        else
        {
            if( newline )
            {
                fmtstring.append( "\n" );
            }
        }
    }
    return fmtstring;
}

void DegenGeom::write_degenGeomSurfCsv_file( FILE* file_id, int nxsecs )
{
    fprintf( file_id, "# DegenGeom Type,nXsecs, nPnts/Xsec\n" );
    fprintf( file_id, "SURFACE_NODE,%d,%d\n", nxsecs, num_pnts );
    fprintf( file_id, "# x,y,z,u,w\n" );

    for ( int i = 0; i < nxsecs; i++ )
    {
        for ( int j = 0; j < num_pnts; j++ )
        {
            fprintf( file_id, makeCsvFmt( 5 ).c_str(),         \
                     degenSurface.x[i][j].x(),       \
                     degenSurface.x[i][j].y(),       \
                     degenSurface.x[i][j].z(),       \
                     degenSurface.u[i][j],               \
                     degenSurface.w[i][j]                );
        }
    }

    fprintf( file_id, "SURFACE_FACE,%d,%d\n", nxsecs - 1, num_pnts - 1 );
    fprintf( file_id, "# nx,ny,nz,area\n" );

    for ( int i = 0; i < nxsecs - 1; i++ )
    {
        for ( int j = 0; j < num_pnts - 1; j++ )
        {
            fprintf( file_id, makeCsvFmt( 4 ).c_str(),         \
                     degenSurface.nvec[i][j].x(),    \
                     degenSurface.nvec[i][j].y(),    \
                     degenSurface.nvec[i][j].z(),    \
                     degenSurface.area[i][j]         );
        }
    }
}

void DegenGeom::write_degenGeomPlateCsv_file( FILE* file_id, int nxsecs, const DegenPlate &degenPlate )
{
    fprintf( file_id, "# DegenGeom Type,nXsecs,nPnts/Xsec\n" );
    fprintf( file_id, "PLATE,%d,%d\n", nxsecs, ( num_pnts + 1 ) / 2 );
    fprintf( file_id, "# nx,ny,nz\n" );
    for ( int i = 0; i < nxsecs; i++ )
    {
        fprintf( file_id, makeCsvFmt( 3 ).c_str(), degenPlate.nPlate[i].x(), \
                 degenPlate.nPlate[i].y(), \
                 degenPlate.nPlate[i].z()  );
    }

    fprintf( file_id, "# x,y,z,zCamber,t,nCamberx,nCambery,nCamberz,u,wTop,wBot,xxCamber,xyCamber,xzCamber\n" );
    for ( int i = 0; i < nxsecs; i++ )
    {
        for ( int j = 0; j < ( num_pnts + 1 ) / 2; j++ )
        {
            fprintf( file_id, makeCsvFmt( 14 ).c_str(),    \
                     degenPlate.x[i][j].x(),             \
                     degenPlate.x[i][j].y(),             \
                     degenPlate.x[i][j].z(),             \
                     degenPlate.zcamber[i][j],           \
                     degenPlate.t[i][j],                 \
                     degenPlate.nCamber[i][j].x(),       \
                     degenPlate.nCamber[i][j].y(),       \
                     degenPlate.nCamber[i][j].z(),       \
                     degenPlate.u[i][j],                 \
                     degenPlate.wTop[i][j],              \
                     degenPlate.wBot[i][j],              \
                     degenPlate.xCamber[i][j].x(),       \
                     degenPlate.xCamber[i][j].y(),       \
                     degenPlate.xCamber[i][j].z()        );
        }
    }
}

void DegenGeom::write_degenGeomStickCsv_file( FILE* file_id, int nxsecs, const DegenStick &degenStick )
{

    fprintf( file_id, "# DegenGeom Type, nXsecs\n" );
    fprintf( file_id, "STICK_NODE, %d\n", nxsecs );
    fprintf( file_id, "# lex,ley,lez,tex,tey,tez,cgShellx,cgShelly,cgShellz,"
             "cgSolidx,cgSolidy,cgSolidz,toc,tLoc,chord,Ishell11,Ishell22,"
             "Ishell12,Isolid11,Isolid22,Isolid12,sectArea,sectNormalx,"
             "sectNormaly,sectNormalz,perimTop,perimBot,u," );
    fprintf( file_id, "t00,t01,t02,t03,t10,t11,t12,t13,t20,t21,t22,t23,t30,t31,t32,t33," );
    fprintf( file_id, "it00,it01,it02,it03,it10,it11,it12,it13,it20,it21,it22,it23,it30,it31,it32,it33," );
    fprintf( file_id, "toc2,tLoc2,anglele,anglete,radleTop,radleBot,\n" );

    for ( int i = 0; i < nxsecs; i++ )
    {
        fprintf( file_id, makeCsvFmt( 28, false ).c_str(), \
                 degenStick.xle[i].x(),                  \
                 degenStick.xle[i].y(),                  \
                 degenStick.xle[i].z(),                  \
                 degenStick.xte[i].x(),                  \
                 degenStick.xte[i].y(),                  \
                 degenStick.xte[i].z(),                  \
                 degenStick.xcgShell[i].x(),             \
                 degenStick.xcgShell[i].y(),             \
                 degenStick.xcgShell[i].z(),             \
                 degenStick.xcgSolid[i].x(),             \
                 degenStick.xcgSolid[i].y(),             \
                 degenStick.xcgSolid[i].z(),             \
                 degenStick.toc[i],                  \
                 degenStick.tLoc[i],                 \
                 degenStick.chord[i],                    \
                 degenStick.Ishell[i][0],                \
                 degenStick.Ishell[i][1],                \
                 degenStick.Ishell[i][2],                \
                 degenStick.Isolid[i][0],                \
                 degenStick.Isolid[i][1],                \
                 degenStick.Isolid[i][2],                \
                 degenStick.sectarea[i],                 \
                 degenStick.sectnvec[i].x(),             \
                 degenStick.sectnvec[i].y(),             \
                 degenStick.sectnvec[i].z(),             \
                 degenStick.perimTop[i],                 \
                 degenStick.perimBot[i],                 \
                 degenStick.u[i]                     );

        fprintf( file_id, ", " );

        for( int j = 0; j < 16; j ++ )
        {
            fprintf( file_id, makeCsvFmt( 1, false ).c_str(), degenStick.transmat[i][j] );
            fprintf( file_id, ", " );
        }


        for( int j = 0; j < 16; j ++ )
        {
            fprintf( file_id, makeCsvFmt( 1, false ).c_str(), degenStick.invtransmat[i][j] );
            fprintf( file_id, ", " );
        }

        fprintf( file_id, makeCsvFmt( 6, false ).c_str(), \
                 degenStick.toc2[i],                      \
                 degenStick.tLoc2[i],                     \
                 degenStick.anglele[i],                   \
                 degenStick.anglete[i],                   \
                 degenStick.radleTop[i],                  \
                 degenStick.radleBot[i]              );

        fprintf( file_id, "\n" );
    }


    fprintf( file_id, "# DegenGeom Type, nXsecs\n" );
    fprintf( file_id, "STICK_FACE, %d\n", nxsecs - 1 );
    fprintf( file_id, "# sweeple,sweepte,areaTop,areaBot\n" );

    for ( int i = 0; i < nxsecs - 1; i++ )
    {
        fprintf( file_id, makeCsvFmt( 4 ).c_str(), \
                 degenStick.sweeple[i],                  \
                 degenStick.sweepte[i],                  \
                 degenStick.areaTop[i],                  \
                 degenStick.areaBot[i]                   );
    }
}

void DegenGeom::write_degenGeomPointCsv_file( FILE* file_id )
{
    fprintf( file_id, "# DegenGeom Type\n" );
    fprintf( file_id, "POINT\n" );
    fprintf( file_id, "# vol,volWet,area,areaWet,Ishellxx,Ishellyy,Ishellzz,Ishellxy," );
    fprintf( file_id, "Ishellxz,Ishellyz,Isolidxx,Isolidyy,Isolidzz,Isolidxy,Isolidxz," );
    fprintf( file_id, "Isolidyz,cgShellx,cgShelly,cgShellz,cgSolidx,cgSolidy,cgSolidz\n" );
    fprintf( file_id, makeCsvFmt( 22 ).c_str(), \
             degenPoint.vol[0],          \
             degenPoint.volWet[0],       \
             degenPoint.area[0],         \
             degenPoint.areaWet[0],      \
             degenPoint.Ishell[0][0],    \
             degenPoint.Ishell[0][1],    \
             degenPoint.Ishell[0][2],    \
             degenPoint.Ishell[0][3],    \
             degenPoint.Ishell[0][4],    \
             degenPoint.Ishell[0][5],    \
             degenPoint.Isolid[0][0],    \
             degenPoint.Isolid[0][1],    \
             degenPoint.Isolid[0][2],    \
             degenPoint.Isolid[0][3],    \
             degenPoint.Isolid[0][4],    \
             degenPoint.Isolid[0][5],    \
             degenPoint.xcgShell[0].x(), \
             degenPoint.xcgShell[0].y(), \
             degenPoint.xcgShell[0].z(), \
             degenPoint.xcgSolid[0].x(), \
             degenPoint.xcgSolid[0].y(), \
             degenPoint.xcgSolid[0].z()  );
}

void DegenGeom::write_degenGeomDiskCsv_file( FILE* file_id )
{
    char fmtstr[255];
    fmtstr[0] = '\0';
    strcat( fmtstr, makeCsvFmt( 7 ).c_str() );
    fprintf( file_id, "# DegenGeom Type\n" );
    fprintf( file_id, "PROP\n" );
    fprintf( file_id, "# diameter,x,y,z,nx,ny,nz\n" );
    fprintf( file_id, fmtstr, \
             degenDisk.d,        \
             degenDisk.x.x(),    \
             degenDisk.x.y(),    \
             degenDisk.x.z(),    \
             degenDisk.nvec.x(), \
             degenDisk.nvec.y(), \
             degenDisk.nvec.z()  );
}

void DegenGeom::write_degenSubSurfCsv_file( FILE* file_id, int isubsurf )
{
    string nospacename = degenSubSurfs[isubsurf].fullName;
    StringUtil::change_space_to_underscore( nospacename );
    fprintf( file_id, "# DegenGeom Type, name, typeName, typeId, fullname\n" );
    fprintf( file_id, "SUBSURF,%s,%s,%d,%s\n", degenSubSurfs[isubsurf].name.c_str(),
                                               degenSubSurfs[isubsurf].typeName.c_str(),
                                               degenSubSurfs[isubsurf].typeId,
                                               nospacename.c_str() );

    fprintf( file_id, "# testType\n" );
    fprintf( file_id, "%d\n", \
            degenSubSurfs[isubsurf].testType );

    int n = degenSubSurfs[isubsurf].u.size();

    fprintf( file_id, "# DegenGeom Type, nPts\n" );
    fprintf( file_id, "SUBSURF_BNDY, %d\n", n );
    fprintf( file_id, "# u,w,x,y,z\n" );
    for ( int i = 0; i < n; i++ )
    {
        fprintf( file_id, makeCsvFmt( 5 ).c_str(), \
                 degenSubSurfs[isubsurf].u[i],                  \
                 degenSubSurfs[isubsurf].w[i],                  \
                 degenSubSurfs[isubsurf].x[i].x(),              \
                 degenSubSurfs[isubsurf].x[i].y(),              \
                 degenSubSurfs[isubsurf].x[i].z() );
    }
}

void DegenGeom::write_degenHingeLineCsv_file( FILE* file_id, int ihingeline )
{
    int n = degenHingeLines[ihingeline].uStart.size();

    fprintf( file_id, "# DegenGeom Type, name, nPts\n" );
    fprintf( file_id, "HINGELINE,%s, %d\n", degenHingeLines[ihingeline].name.c_str(), n );

    fprintf( file_id, "# uStart,uEnd,wStart,wEnd,xStart,yStart,zStart,xEnd,yEnd,zEnd\n" );
    for ( int i = 0; i < n; i++ )
    {
        fprintf( file_id, makeCsvFmt( 10 ).c_str(), \
                degenHingeLines[ihingeline].uStart[i], \
                degenHingeLines[ihingeline].uEnd[i], \
                degenHingeLines[ihingeline].wStart[i], \
                degenHingeLines[ihingeline].wEnd[i], \
                degenHingeLines[ihingeline].xStart[i].x(), \
                degenHingeLines[ihingeline].xStart[i].y(), \
                degenHingeLines[ihingeline].xStart[i].z(), \
                degenHingeLines[ihingeline].xEnd[i].x(), \
                degenHingeLines[ihingeline].xEnd[i].y(), \
                degenHingeLines[ihingeline].xEnd[i].z() );
    }
}

void DegenGeom::write_degenGeomCsv_file( FILE* file_id )
{
    int nxsecs = num_xsecs;

    string typestr;

    if( type == SURFACE_TYPE )
    {
        typestr = "LIFTING_SURFACE";
    }
    else if( type == DISK_TYPE )
    {
        typestr = "DISK";
    }
    else if( type == MESH_TYPE )
    {
        typestr = "MESH";
    }
    else
    {
        typestr = "BODY";
    }

    fprintf( file_id, "\n# DegenGeom Type, Name, SurfNdx, GeomID, MainSurfNdx, SymCopyNdx, FlipNormal," );
    fprintf( file_id, "t00,t01,t02,t03,t10,t11,t12,t13,t20,t21,t22,t23,t30,t31,t32,t33" );
    fprintf( file_id, "\n%s,%s,%d,%s,%d,%d,%d,", typestr.c_str(), name.c_str(), getSurfNum(),
            this->parentGeom->GetID().c_str(), getMainSurfInd(), getSymCopyInd(), getFlipNormal() );

    for( int j = 0; j < 16; j ++ )
    {
        fprintf( file_id, makeCsvFmt( 1, false ).c_str(), transmat[j] );

        if( j < 16 - 1 )
        {
            fprintf( file_id, ", " );
        }
        else
        {
            fprintf( file_id, "\n" );
        }
    }

    if( type == DISK_TYPE )
    {
        write_degenGeomDiskCsv_file( file_id );
    }

    if( type != MESH_TYPE )
    {
        write_degenGeomSurfCsv_file( file_id, nxsecs );
    }

    if( type == DISK_TYPE )
    {
        return;
    }

    if( degenPlates.size() > 0 )
    {
        write_degenGeomPlateCsv_file( file_id, nxsecs, degenPlates[0] );
    }

    if ( type == DegenGeom::BODY_TYPE && degenPlates.size() > 1 )
    {
        write_degenGeomPlateCsv_file( file_id, nxsecs, degenPlates[1] );
    }

    if ( degenSticks.size() > 0 )
    {
        write_degenGeomStickCsv_file( file_id, nxsecs, degenSticks[0] );
    }

    if ( type == DegenGeom::BODY_TYPE && degenSticks.size() > 1 )
    {
        write_degenGeomStickCsv_file( file_id, nxsecs, degenSticks[1] );
    }

    write_degenGeomPointCsv_file( file_id );

    for ( int i = 0; i < degenSubSurfs.size(); i++ )
    {
        write_degenSubSurfCsv_file( file_id, i );
    }

    for ( int i = 0; i < degenHingeLines.size(); i++ )
    {
        write_degenHingeLineCsv_file( file_id, i );
    }
}

void DegenGeom::write_degenGeomSurfM_file( FILE* file_id, int nxsecs )
{
    string basename = string( "degenGeom(end).surf." );

    WriteVecDoubleM writeVecDouble;
    WriteMatVec3dM writeMatVec3d;
    WriteMatDoubleM writeMatDouble;

    fprintf( file_id, "degenGeom(end).surf.nxsecs = %d;\n", nxsecs );
    fprintf( file_id, "degenGeom(end).surf.num_pnts = %d;\n", num_pnts );

    writeMatVec3d.write(  file_id, degenSurface.x,    basename, nxsecs, num_pnts );
    writeMatDouble.write( file_id, degenSurface.u,    basename + "u",   nxsecs,      num_pnts );
    writeMatDouble.write( file_id, degenSurface.w,    basename + "w",   nxsecs,      num_pnts );
    writeMatVec3d.write(  file_id, degenSurface.nvec, basename + "n",   nxsecs - 1,    num_pnts - 1 );
    writeMatDouble.write( file_id, degenSurface.area, basename + "area", nxsecs - 1,    num_pnts - 1 );
}

void DegenGeom::write_degenGeomPlateM_file( FILE* file_id, int nxsecs, const DegenPlate &degenPlate, int iplate )
{
    char num[80];
    snprintf( num, sizeof( num ), "degenGeom(end).plate(%d).", iplate );
    string basename = string( num );

    WriteVecDoubleM writeVecDouble;
    WriteVecVec3dM writeVecVec3d;
    WriteMatDoubleM writeMatDouble;
    WriteMatVec3dM writeMatVec3d;

    fprintf( file_id, "degenGeom(end).plate(%d).nxsecs = %d;\n", iplate, nxsecs );
    fprintf( file_id, "degenGeom(end).plate(%d).num_pnts = %d;\n", iplate, ( num_pnts + 1 ) / 2 );

    writeVecVec3d.write(  file_id, degenPlate.nPlate,  basename + "n",       nxsecs );
    writeMatVec3d.write(  file_id, degenPlate.x,       basename,             nxsecs,    ( num_pnts + 1 ) / 2 );
    writeMatVec3d.write(  file_id, degenPlate.xCamber,       basename + "xCamber", nxsecs,    ( num_pnts + 1 ) / 2 );
    writeMatDouble.write( file_id, degenPlate.zcamber, basename + "zCamber", nxsecs,    ( num_pnts + 1 ) / 2 );
    writeMatDouble.write( file_id, degenPlate.t,       basename + "t",       nxsecs,    ( num_pnts + 1 ) / 2 );
    writeMatVec3d.write(  file_id, degenPlate.nCamber, basename + "nCamber", nxsecs,    ( num_pnts + 1 ) / 2 );
    writeMatDouble.write( file_id, degenPlate.u,       basename + "u",       nxsecs,    ( num_pnts + 1 ) / 2 );
    writeMatDouble.write( file_id, degenPlate.wTop,    basename + "wTop",    nxsecs,    ( num_pnts + 1 ) / 2 );
    writeMatDouble.write( file_id, degenPlate.wBot,    basename + "wBot",    nxsecs,    ( num_pnts + 1 ) / 2 );
}

void DegenGeom::write_degenGeomStickM_file( FILE* file_id, int nxsecs, const DegenStick &degenStick, int istick )
{
    char num[80];
    snprintf( num, sizeof( num ), "degenGeom(end).stick(%d).", istick );
    string basename = string( num );

    WriteVecDoubleM writeVecDouble;
    WriteVecVec3dM writeVecVec3d;
    WriteMatDoubleM writeMatDouble;

    fprintf( file_id, "degenGeom(end).stick(%d).nxsecs = %d;\n", istick, nxsecs );

    writeVecVec3d.write(  file_id, degenStick.xle,        basename + "le",         nxsecs );
    writeVecVec3d.write(  file_id, degenStick.xte,        basename + "te",         nxsecs );
    writeVecVec3d.write(  file_id, degenStick.xcgShell,   basename + "cgShell",    nxsecs );
    writeVecVec3d.write(  file_id, degenStick.xcgSolid,   basename + "cgSolid",    nxsecs );
    writeVecDouble.write( file_id, degenStick.toc,        basename + "toc",        nxsecs );
    writeVecDouble.write( file_id, degenStick.tLoc,       basename + "tLoc",       nxsecs );
    writeVecDouble.write( file_id, degenStick.chord,      basename + "chord",      nxsecs );
    writeMatDouble.write( file_id, degenStick.Ishell,     basename + "Ishell",     nxsecs,        3 );
    writeMatDouble.write( file_id, degenStick.Isolid,     basename + "Isolid",     nxsecs,        3 );
    writeVecDouble.write( file_id, degenStick.sectarea,   basename + "sectArea",   nxsecs );
    writeVecVec3d.write(  file_id, degenStick.sectnvec,   basename + "sectNormal", nxsecs );
    writeVecDouble.write( file_id, degenStick.perimTop,   basename + "perimTop",   nxsecs );
    writeVecDouble.write( file_id, degenStick.perimBot,   basename + "perimBot",   nxsecs );
    writeVecDouble.write( file_id, degenStick.u,          basename + "u",          nxsecs );
    writeMatDouble.write( file_id, degenStick.transmat,   basename + "transmat",   nxsecs,        16 );
    writeMatDouble.write( file_id, degenStick.invtransmat, basename + "invtransmat", nxsecs,        16 );
    writeVecDouble.write( file_id, degenStick.toc2,       basename + "toc2",       nxsecs );
    writeVecDouble.write( file_id, degenStick.tLoc2,      basename + "tLoc2",      nxsecs );
    writeVecDouble.write( file_id, degenStick.anglele,    basename + "anglele",    nxsecs );
    writeVecDouble.write( file_id, degenStick.anglete,    basename + "anglete",    nxsecs );
    writeVecDouble.write( file_id, degenStick.radleTop,   basename + "radleTop",   nxsecs );
    writeVecDouble.write( file_id, degenStick.radleBot,   basename + "radleBot",   nxsecs );

    writeVecDouble.write( file_id, degenStick.sweeple,    basename + "sweeple",    nxsecs - 1 );
    writeVecDouble.write( file_id, degenStick.sweepte,    basename + "sweepte",    nxsecs - 1 );
    writeVecDouble.write( file_id, degenStick.areaTop,    basename + "areaTop",    nxsecs - 1 );
    writeVecDouble.write( file_id, degenStick.areaBot,    basename + "areaBot",    nxsecs - 1 );

}

void DegenGeom::write_degenGeomPointM_file( FILE* file_id )
{
    string basename = string( "degenGeom(end).point." );

    WriteDoubleM writeDouble;
    WriteVec3dM writeVec3d;
    WriteVecDoubleM writeVecDouble;

    writeDouble.write(    file_id, degenPoint.vol[0],      basename + "vol" );
    writeDouble.write(    file_id, degenPoint.volWet[0],   basename + "volWet" );
    writeDouble.write(    file_id, degenPoint.area[0],     basename + "area" );
    writeDouble.write(    file_id, degenPoint.areaWet[0],  basename + "areaWet" );
    writeVecDouble.write( file_id, degenPoint.Ishell[0],   basename + "Ishell",     6 );
    writeVecDouble.write( file_id, degenPoint.Isolid[0],   basename + "Isolid",     6 );
    writeVec3d.write(     file_id, degenPoint.xcgShell[0], basename + "cgShell" );
    writeVec3d.write(     file_id, degenPoint.xcgSolid[0], basename + "cgSolid" );
}

void DegenGeom::write_degenGeomDiskM_file( FILE* file_id )
{
    string basename = string( "degenGeom(end).disk." );

    WriteDoubleM writeDouble;
    WriteVec3dM writeVec3d;

    writeDouble.write( file_id, degenDisk.d,    basename + "diameter" );
    writeVec3d.write(  file_id, degenDisk.x,    basename );
    writeVec3d.write(  file_id, degenDisk.nvec, basename + "n" );
}

void DegenGeom::write_degenSubSurfM_file( FILE* file_id, int isubsurf )
{
    char num[80];
    snprintf( num, sizeof( num ), "degenGeom(end).subsurf(%d).", isubsurf + 1 );
    string basename = string( num );

    WriteVecDoubleM writeVecDouble;
    WriteVecVec3dM writeVecVec3d;

    fprintf( file_id, "\ndegenGeom(end).subsurf(%d).name = '%s';\n", isubsurf + 1, degenSubSurfs[isubsurf].name.c_str() );
    fprintf( file_id, "\ndegenGeom(end).subsurf(%d).typeName = %d;\n", isubsurf + 1, degenSubSurfs[isubsurf].testType );
    fprintf( file_id, "\ndegenGeom(end).subsurf(%d).typeId = %d;\n", isubsurf + 1, degenSubSurfs[isubsurf].testType );
    fprintf( file_id, "\ndegenGeom(end).subsurf(%d).fullName = '%s';\n", isubsurf + 1, degenSubSurfs[isubsurf].fullName.c_str() );
    fprintf( file_id, "\ndegenGeom(end).subsurf(%d).testType = %d;\n", isubsurf + 1, degenSubSurfs[isubsurf].testType );

    int n = degenSubSurfs[isubsurf].u.size();

    writeVecDouble.write( file_id, degenSubSurfs[isubsurf].u,        basename + "u",        n );
    writeVecDouble.write( file_id, degenSubSurfs[isubsurf].w,        basename + "w",        n );
    writeVecVec3d.write( file_id, degenSubSurfs[isubsurf].x,         basename + "x",        n );
}

void DegenGeom::write_degenHingeLineM_file( FILE* file_id, int ihingeline )
{
    char num[80];
    snprintf( num, sizeof( num ), "degenGeom(end).hingeline(%d).", ihingeline + 1 );
    string basename = string( num );

    WriteVecDoubleM writeVecDouble;
    WriteVecVec3dM writeVecVec3d;

    fprintf( file_id, "\ndegenGeom(end).hingeline(%d).name = '%s';\n", ihingeline + 1, degenHingeLines[ihingeline].name.c_str() );

    int n = degenHingeLines[ihingeline].uStart.size();

    writeVecDouble.write( file_id, degenHingeLines[ihingeline].uStart,        basename + "uStart",        n );
    writeVecDouble.write( file_id, degenHingeLines[ihingeline].uEnd,          basename + "uEnd",          n );
    writeVecDouble.write( file_id, degenHingeLines[ihingeline].wStart,        basename + "wStart",        n );
    writeVecDouble.write( file_id, degenHingeLines[ihingeline].wEnd,          basename + "wEnd",          n );
    writeVecVec3d.write( file_id, degenHingeLines[ihingeline].xStart,         basename + "xStart",        n );
    writeVecVec3d.write( file_id, degenHingeLines[ihingeline].xEnd,           basename + "xEnd",        n );
}

void DegenGeom::write_degenGeomM_file( FILE* file_id )
{
    int nxsecs = num_xsecs;

    WriteVecDoubleM writeVecDouble;

    if( type == SURFACE_TYPE )
    {
        fprintf( file_id, "\ndegenGeom(end+1).type = 'LIFTING_SURFACE';" );
    }
    else if( type == DISK_TYPE )
    {
        fprintf( file_id, "\ndegenGeom(end+1).type = 'DISK';" );
    }
    else if( type == MESH_TYPE )
    {
        fprintf( file_id, "\ndegenGeom(end+1).type = 'MESH';" );
    }
    else
    {
        fprintf( file_id, "\ndegenGeom(end+1).type = 'BODY';" );
    }

    fprintf( file_id, "\ndegenGeom(end).name = '%s';", name.c_str() );
    fprintf( file_id, "\ndegenGeom(end).geom_id = '%s';", parentGeom->GetID().c_str() );
    fprintf( file_id, "\ndegenGeom(end).surf_index = %d;", getSurfNum() );
    fprintf( file_id, "\ndegenGeom(end).main_surf_index = %d;", getMainSurfInd() );
    fprintf( file_id, "\ndegenGeom(end).sym_copy_index = %d;", getSymCopyInd() );
    fprintf( file_id, "\ndegenGeom(end).flip_normal = %d;\n", getFlipNormal() );

    writeVecDouble.write( file_id, transmat, "degenGeom(end).transmat",    16 );

    if( type == DISK_TYPE )
    {
        write_degenGeomDiskM_file(file_id);
    }

    if ( type != MESH_TYPE )
    {
        write_degenGeomSurfM_file( file_id, nxsecs );
    }

    if( type == DISK_TYPE )
    {
        return;
    }

    if ( degenPlates.size() > 0 )
        write_degenGeomPlateM_file( file_id, nxsecs, degenPlates[0], 1 );

    if ( type == DegenGeom::BODY_TYPE && degenPlates.size() > 1 )
    {
        write_degenGeomPlateM_file( file_id, nxsecs, degenPlates[1], 2 );
    }

    if ( degenSticks.size() > 0 )
        write_degenGeomStickM_file( file_id, nxsecs, degenSticks[0], 1 );

    if ( type == DegenGeom::BODY_TYPE && degenSticks.size() > 1 )
    {
        write_degenGeomStickM_file( file_id, nxsecs, degenSticks[1], 2 );
    }

    write_degenGeomPointM_file( file_id );

    for ( int i = 0; i < degenSubSurfs.size(); i++ )
    {
        write_degenSubSurfM_file( file_id, i );
    }

    for ( int i = 0; i < degenHingeLines.size(); i++ )
    {
        write_degenHingeLineM_file( file_id, i );
    }
}

void DegenGeom::write_degenGeomResultsManager( vector< string> &degen_results_ids )
{
    Results *res = ResultsMgr.CreateResults( "Degen_DegenGeom", "Degen geom results." );
    degen_results_ids.push_back( res->GetID() );
    string degentype;
    if ( type == SURFACE_TYPE )
    {
        degentype = "LIFTING_SURFACE";
    }
    else if ( type == DISK_TYPE )
    {
        degentype = "DISK";
    }
    else if( type == MESH_TYPE )
    {
        degentype = "MESH";
    }
    else
    {
        degentype = "BODY";
    }
    res->Add( new NameValData( "type", degentype, "Geom type classification (LIFTING_SURFACE, DISK, MESH, or BODY)." ) );

    res->Add( new NameValData( "name", name, "Geom name." ) );
    res->Add( new NameValData( "surf_index", getSurfNum(), "Surface index." ) );
    res->Add( new NameValData( "geom_id", parentGeom->GetID(), "GeomID." ) );
    res->Add( new NameValData( "main_surf_index", getMainSurfInd(), "Index of main surface copied for symmetry." ) );
    res->Add( new NameValData( "sym_copy_index", getSymCopyInd(), "Symmetrical copy index." ) );
    res->Add( new NameValData( "flip_normal", getFlipNormal(), "Flag to flip surface orientation." ) );

    res->Add( new NameValData( "transmat", transmat, "Transformation matrix from origin to surface location." ) );

    if ( type == DISK_TYPE )
    {
        write_degenGeomDiskResultsManager( res );
    }

    if( type != MESH_TYPE )
    {
        write_degenGeomSurfResultsManager( res );
    }

    if ( type == DISK_TYPE )
    {
        return;
    }

    vector< string > plate_ids;
    for ( unsigned int i = 0; i < degenPlates.size(); i++ )
    {
        write_degenGeomPlateResultsManager( plate_ids, degenPlates[i] );
    }
    res->Add( new NameValData( "plates", plate_ids, "ID's of degen plate results." ) );

    vector< string > stick_ids;
    for ( unsigned int i = 0; i < degenSticks.size(); i++ )
    {
        write_degenGeomStickResultsManager( stick_ids, degenSticks[i] );
    }
    res->Add( new NameValData( "sticks", stick_ids, "ID's of degen stick results." ) );

    write_degenGeomPointResultsManager( res );

    vector< string > subsurf_ids;
    for ( unsigned int i = 0; i < degenSubSurfs.size(); i++ )
    {
        write_degenSubSurfResultsManager( subsurf_ids, degenSubSurfs[i] );
    }
    res->Add( new NameValData( "subsurfs", subsurf_ids, "ID's of degen subsurface results." ) );

    vector< string > hinge_ids;
    for ( unsigned int i = 0; i < degenHingeLines.size(); i++ )
    {
        write_degenHingeLineResultsManager( hinge_ids, degenHingeLines[i] );
    }
    res->Add( new NameValData( "hinges", hinge_ids, "ID's of degen hinge line results." ) );
}

void DegenGeom::write_degenGeomDiskResultsManager( Results *res )
{
    if ( !res ) { return; }

    Results *disk_res = ResultsMgr.CreateResults( "Degen_disk", "Degen geom actuator disk results." );
    res->Add( new NameValData( "disk", disk_res->GetID(), "ID of degen disk result." ) );

    disk_res->Add( new NameValData( "diameter", degenDisk.d, "Diameter." ) );
    disk_res->Add( new NameValData( "pos", degenDisk.x, "Center coordinate." ) );
    disk_res->Add( new NameValData( "n", degenDisk.nvec, "Axis of rotation vector." ) );

}

void DegenGeom::write_degenGeomSurfResultsManager( Results *res )
{
    if ( !res ) { return; }

    Results *surf_res = ResultsMgr.CreateResults( "Degen_surf", "Degen surface representation results." );
    res->Add( new NameValData ( "surf", surf_res->GetID(), "ID of degen surf result." ) );

    surf_res->Add( new NameValData( "nxsecs", num_xsecs, "Number of cross sections." ) );
    surf_res->Add( new NameValData( "num_pnts", num_pnts, "Number of points per cross section." ) );
    surf_res->Add( degenSurface.x, "", "Node coordinates." );
    surf_res->Add( new NameValData( "u", degenSurface.u, "U surface parameters." ) );
    surf_res->Add( new NameValData( "w", degenSurface.w, "W surface parameters." ) );
    surf_res->Add( degenSurface.nvec, "n", "Face normal vectors." );
    surf_res->Add( new NameValData( "area", degenSurface.area, "Face areas." ) );
}

void DegenGeom::write_degenGeomPlateResultsManager( vector< string > &plate_ids, const DegenPlate &degenPlate )
{
    Results *plate_res = ResultsMgr.CreateResults( "Degen_plate", "Degen geom plate representation results." );
    plate_ids.push_back( plate_res->GetID() );

    plate_res->Add( new NameValData( "nxsecs", num_xsecs, "Number of cross sections." ) );
    plate_res->Add( new NameValData( "num_pnts", num_pnts, "Number of points per cross section." ) );

    plate_res->Add( new NameValData( "n", degenPlate.nPlate, "Normal vector." ) );
    plate_res->Add( degenPlate.x, "", "Plate surface coordinates." );
    plate_res->Add( degenPlate.xCamber, "xCamber", "Camber surface coordinates." );
    plate_res->Add( new NameValData( "zCamber", degenPlate.zcamber, "Camber offset distance." ) );
    plate_res->Add( new NameValData( "t", degenPlate.t, "Surface thickness." ) );
    plate_res->Add( degenPlate.nCamber, "nCamber_", "Camber surface normal vector." );
    plate_res->Add( new NameValData( "u", degenPlate.u, "U surface parameters." ) );
    plate_res->Add( new NameValData( "wTop", degenPlate.wTop, "W surface parameter of top points." ) );
    plate_res->Add( new NameValData( "wBot", degenPlate.wBot, "W surface parameter of bottom points." ) );
}

void DegenGeom::write_degenGeomStickResultsManager( vector<string> &stick_ids, const DegenStick &degenStick )
{
    Results *stick_res = ResultsMgr.CreateResults( "Degen_stick", "Degen stick representation results." );
    stick_ids.push_back( stick_res->GetID() );

    stick_res->Add( new NameValData( "nxsecs", num_xsecs, "Number of cross sections." ) );
    stick_res->Add( new NameValData( "le", degenStick.xle, "Leading edge coordinates." ) );
    stick_res->Add( new NameValData( "te", degenStick.xte, "Trailing edge coordinates." ) );
    stick_res->Add( new NameValData( "cgShell", degenStick.xcgShell, "Center of gravity of section treated as thin shell." ) );
    stick_res->Add( new NameValData( "cgSolid", degenStick.xcgSolid, "Center of gravity of section treated as solid area." ) );
    stick_res->Add( new NameValData( "toc", degenStick.toc, "Maximum thickness to chord calculated from mesh points." ) );
    stick_res->Add( new NameValData( "tLoc", degenStick.tLoc, "Location of max t/c calculated from mesh points." ) );
    stick_res->Add( new NameValData( "chord", degenStick.chord, "Section chord." ) );
    stick_res->Add( new NameValData( "Ishell", degenStick.Ishell, "Moment of inertia of section treated as thin shell I11 I22 I12, per unit thickness." ) );
    stick_res->Add( new NameValData( "Isolid", degenStick.Isolid, "Moment of inertia of section treated as solid area I11 I22 I12." ) );
    stick_res->Add( new NameValData( "sectArea", degenStick.sectarea, "Cross section area." ) );
    stick_res->Add( new NameValData( "sectNormal", degenStick.sectnvec, "Section normal vector." ) );
    stick_res->Add( new NameValData( "perimTop", degenStick.perimTop, "Perimeter of section top surface." ) );
    stick_res->Add( new NameValData( "perimBot", degenStick.perimBot, "Perimeter of section bottom surface." ) );
    stick_res->Add( new NameValData( "u", degenStick.u, "U surface parameter of section." ) );
    stick_res->Add( new NameValData( "transmat", degenStick.transmat, "Transformation matrix from origin to section." ) );
    stick_res->Add( new NameValData( "invtransmat", degenStick.invtransmat, "Inverse transformation matrix." ) );
    stick_res->Add( new NameValData( "toc2", degenStick.toc2, "Maximum thickness to chord calculated from Bezier curves." ) );
    stick_res->Add( new NameValData( "tLoc2", degenStick.tLoc2, "Location of max t/c calculated from Bezier curves." ) );
    stick_res->Add( new NameValData( "anglele", degenStick.anglele, "Leading edge angle." ) );
    stick_res->Add( new NameValData( "anglete", degenStick.anglete, "Trailing edge angle." ) );
    stick_res->Add( new NameValData( "radleTop", degenStick.radleTop, "Upper surface leading edge radius." ) );
    stick_res->Add( new NameValData( "radleBot", degenStick.radleBot, "Lower surface leading edge radius." ) );

    stick_res->Add( new NameValData( "sweeple", degenStick.sweeple, "Leading edge sweep angle." ) );
    stick_res->Add( new NameValData( "sweepte", degenStick.sweepte, "Trailing edge sweep angle." ) );
    stick_res->Add( new NameValData( "areaTop", degenStick.areaTop, "Area of top surface strip." ) );
    stick_res->Add( new NameValData( "areaBot", degenStick.areaBot, "Area of bottom surface strip." ) );
}

void DegenGeom::write_degenGeomPointResultsManager( Results *res )
{
    if ( !res ) { return; }

    Results *point_res = ResultsMgr.CreateResults( "point", "Degen geom point representation results." );
    res->Add( new NameValData( "point", point_res->GetID(), "ID of degen point result." ) );

    point_res->Add( new NameValData( "vol", degenPoint.vol[0], "Volume." ) );
    point_res->Add( new NameValData( "volWet", degenPoint.volWet[0], "Contribution to trimmed volume of Set." ) );
    point_res->Add( new NameValData( "area", degenPoint.area[0], "Surface area." ) );
    point_res->Add( new NameValData( "areaWet", degenPoint.areaWet[0], "Contribution to wetted surface area of Set." ) );
    point_res->Add( new NameValData( "Ishell", degenPoint.Ishell[0], "Moment of inertia treated as thin shell Ixx Iyy Izz Ixy Ixz Iyz, per unit thickness." ) );
    point_res->Add( new NameValData( "Isolid", degenPoint.Isolid[0], "Moment of inertia treated as solid body Ixx Iyy Izz Ixy Ixz Iyz." ) );
    point_res->Add( new NameValData( "cgShell", degenPoint.xcgShell, "Center of gravity treated as thin shell." ) );
    point_res->Add( new NameValData( "cgSolid", degenPoint.xcgSolid, "Center of gravity treated as solid." ) );
}

void DegenGeom::write_degenSubSurfResultsManager( vector<string> &subsurf_ids, const DegenSubSurf &degenSubSurf )
{
    Results* subsurf_res = ResultsMgr.CreateResults( "Degen_subsurf", "Degen subsurface results." );
    subsurf_ids.push_back( subsurf_res->GetID() );

    subsurf_res->Add( new NameValData( "name", degenSubSurf.name, "Sub surface name." ) );
    subsurf_res->Add( new NameValData( "typeName", degenSubSurf.typeName, "Name of type." ) );
    subsurf_res->Add( new NameValData( "typeId", degenSubSurf.typeId, "ID of type." ) );
    subsurf_res->Add( new NameValData( "fullName", degenSubSurf.fullName, "ParentGeomName_Surf#_SubSurfName." ) );
    subsurf_res->Add( new NameValData( "testType", degenSubSurf.testType, "Test type ID, inside, outside, none." ) );

    subsurf_res->Add( new NameValData( "u", degenSubSurf.u, "U surface parameter polyline." ) );
    subsurf_res->Add( new NameValData( "w", degenSubSurf.w, "W surface parameter polyline." ) );
    subsurf_res->Add( new NameValData( "x", degenSubSurf.x, "Coordinate polyline." ) );
}

void DegenGeom::write_degenHingeLineResultsManager( vector<string> &hinge_ids, const DegenHingeLine &degenHingeLine )
{
    Results *hinge_res = ResultsMgr.CreateResults( "Degen_hinge", "Degen hinge results." );
    hinge_ids.push_back( hinge_res->GetID() );

    hinge_res->Add( new NameValData( "name", degenHingeLine.name, "Control surface name." ) );
    hinge_res->Add( new NameValData( "uStart", degenHingeLine.uStart, "U surface parameter of start point." ) );
    hinge_res->Add( new NameValData( "uEnd", degenHingeLine.uEnd, "U surface parameter of end point." ) );
    hinge_res->Add( new NameValData( "wStart", degenHingeLine.wStart, "W surface parameter of start point." ) );
    hinge_res->Add( new NameValData( "wEnd", degenHingeLine.wEnd, "W surface parameter of end point." ) );
    hinge_res->Add( new NameValData( "xStart", degenHingeLine.xStart, "Coordinate of start point." ) );
    hinge_res->Add( new NameValData( "xEnd", degenHingeLine.xEnd, "Coordinate of end point." ) );
}

void DegenGeom::createTMeshVec( Geom * geom, vector< TMesh* > &tMeshVec, int skipnegflipnormal, int n_ref, bool checkFlat )
{
    int surftype = getType();
    bool thicksurf = false;
    int iQuad = 0;

    for (int i = 0; i < degenPlates.size(); i++ )
    {
        int nl = degenPlates[i].u.size();
        int nm = degenPlates[i].u[0].size();
        vector < vector < vec3d > > uw_pnts;
        uw_pnts.resize( nl );
        for (int j = 0; j < nl; j++ )
        {
            uw_pnts[j].resize(nm );
            for (int k = 0; k < nm; k++ )
            {
                uw_pnts[j][k] = vec3d(degenPlates[i].u[j][k],
                                      degenPlates[i].wBot[j][k],
                                      0.0 );
            }
        }


        if ( surftype == DegenGeom::SURFACE_TYPE )
        {
            if ( checkFlat )
            {
                CreateTMeshVecFromPtsCheckFlat( geom,
                                                tMeshVec,
                                                degenPlates[i].xCamber,
                                                uw_pnts,
                                                n_ref,
                                                getSurfNum(),
                                                i,
                                                vsp::WING_SURF,
                                                getCfdSurfType(),
                                                thicksurf,
                                                getFlipNormal(),
                                                skipnegflipnormal, iQuad );
            }
            else
            {
                CreateTMeshVecFromPts( geom,
                                       tMeshVec,
                                       degenPlates[i].xCamber,
                                       uw_pnts,
                                       n_ref,
                                       getSurfNum(),
                                       i,
                                       vsp::WING_SURF,
                                       getCfdSurfType(),
                                       thicksurf,
                                       getFlipNormal(),
                                       skipnegflipnormal, iQuad );
            }
        }
        else if ( surftype == DegenGeom::DISK_TYPE)
        {
            int vspsurftype = vsp::DISK_SURF;
        }
        else
        {
            if ( checkFlat )
            {
                CreateTMeshVecFromPtsCheckFlat( geom,
                                                tMeshVec,
                                                degenPlates[i].x,
                                                uw_pnts,
                                                n_ref,
                                                getSurfNum(),
                                                i,
                                                vsp::NORMAL_SURF,
                                                getCfdSurfType(),
                                                thicksurf,
                                                getFlipNormal(),
                                                skipnegflipnormal, iQuad );
            }
            else
            {
                CreateTMeshVecFromPts( geom,
                                       tMeshVec,
                                       degenPlates[i].x,
                                       uw_pnts,
                                       n_ref,
                                       getSurfNum(),
                                       i,
                                       vsp::NORMAL_SURF,
                                       getCfdSurfType(),
                                       thicksurf,
                                       getFlipNormal(),
                                       skipnegflipnormal, iQuad );
            }
        }
    }
}

// This does not attempt to transform everything in a DegenGeom.  This is limited to Surface and Plate representations
// as this is only meant to work with the preview visualization.
void DegenGeom::Transform( const Matrix4d & mat )
{
    for ( int i = 0; i < degenSurface.x.size(); i++ )
    {
        mat.xformvec( degenSurface.x[i] );
    }

    for ( int i = 0; i < degenSurface.nvec.size(); i++ )
    {
        mat.xformnormvec( degenSurface.nvec[i] );
    }

    for ( int i = 0; i < degenPlates.size(); i++ )
    {
        for ( int j = 0; j < degenPlates[i].x.size(); j++ )
        {
            mat.xformvec( degenPlates[i].x[j] );
        }

        for ( int j = 0; j < degenPlates[i].nCamber.size(); j++ )
        {
            mat.xformnormvec( degenPlates[i].nCamber[j] );
        }

        mat.xformnormvec( degenPlates[i].nPlate );
    }
}
