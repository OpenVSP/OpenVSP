"""A NumPy model of how GearGeom places a bogie's tires when it retracts by a mechanism, mirroring
Bogie::BuildRetractMatrix in GearGeom.cpp, and small helpers that make illustration gears through openvsp.

Points are in the gear's own frame: X aft, Y right, Z up from the nominal ground plane.
"""

import numpy as np
import openvsp as vsp


def rot( axis, deg ):
    """A right-handed turn by deg about axis, as a 4x4 matrix."""
    a = np.asarray( axis, float )
    a = a / np.linalg.norm( a )
    t = np.radians( deg )
    K = np.array( [ [ 0, -a[2], a[1] ], [ a[2], 0, -a[0] ], [ -a[1], a[0], 0 ] ] )
    M = np.eye( 4 )
    M[ :3, :3 ] = np.eye( 3 ) + np.sin( t ) * K + ( 1 - np.cos( t ) ) * K @ K
    return M


def trans( p ):
    M = np.eye( 4 )
    M[ :3, 3 ] = p
    return M


def about( p, axis, deg ):
    """A right-handed turn by deg about the line through p along axis."""
    return trans( p ) @ rot( axis, deg ) @ trans( -np.asarray( p, float ) )


def apply( M, P ):
    P = np.asarray( P, float )
    return P @ M[ :3, :3 ].T + M[ :3, 3 ]


def unit( v ):
    v = np.asarray( v, float )
    return v / np.linalg.norm( v )


class Bogie:
    """The parms of one bogie that set where its tires go, read from VSP."""

    def __init__( self, bogie_id ):
        def g( name, grp ):
            return vsp.GetParmVal( vsp.FindParm( bogie_id, name, grp ) )
        self.contact = np.array( [ g( "XContactPt", "Bogie" ), g( "YContactPt", "Bogie" ), g( "ZAboveGround", "Bogie" ) ] )
        self.slr = g( "StaticRadiusModel", "Tire" )
        self.travel_dir = unit( [ g( "TravelX", "Bogie" ), g( "TravelY", "Bogie" ), g( "TravelZ", "Bogie" ) ] )
        self.trunnion = np.array( [ g( "Mech_XLoc", "Retract" ), g( "Mech_YLoc", "Retract" ), g( "Mech_ZLoc", "Retract" ) ] )
        self.axis = unit( [ g( "Mech_X_Axis", "Retract" ), g( "Mech_Y_Axis", "Retract" ), g( "Mech_Z_Axis", "Retract" ) ] )
        self.ret = g( "Mech_RetAngle", "Retract" )
        self.knee_pos = g( "MechKneePos", "Retract" )
        self.knee_angle = g( "MechKneeAngle", "Retract" )
        self.knee_az = g( "MechKneeAzimuthAngle", "Retract" )
        self.knee_el = g( "MechKneeElevationAngle", "Retract" )
        self.knee_down = g( "MechKneeDownAngle", "Retract" )
        self.twist = g( "Mech_TwistAngle", "Retract" )
        self.roll = g( "m_MechRollAngle", "Retract" )
        self.bogie = g( "Mech_BogieAngle", "Retract" )
        self.strut_dl = g( "Mech_StrutDL", "Retract" )

    def pivot( self ):
        """The axle center with the gear down: SLR above the contact point."""
        return self.contact + np.array( [ 0, 0, self.slr ] )

    def knee_down_pt( self ):
        """The knee with the gear down."""
        pv = self.pivot()
        u = self.trunnion - pv
        alpha = np.radians( self.knee_down )
        beta = ( 1.0 - self.knee_pos ) * np.sin( alpha )
        gamma = np.pi - alpha - np.arctan2( beta, np.sqrt( 1 - beta * beta ) )
        return apply( about( self.trunnion, self.axis, np.degrees( gamma ) ), pv + u * self.knee_pos ), gamma

    def knee_axis( self ):
        """The axis the lower strut turns about at the knee: the part of the trunnion axis square to the
        straight strut, turned by the knee's Azimuth and Elevation, then by the knee bend with the upper strut."""
        pv = self.pivot()
        ud = unit( self.trunnion - pv )
        n = unit( np.cross( self.axis, ud ) )
        v = unit( np.cross( ud, n ) )
        R = rot( ud, -self.knee_az )[ :3, :3 ] @ rot( n, self.knee_el )[ :3, :3 ]
        k_pt, gamma = self.knee_down_pt()
        return rot( self.axis, np.degrees( gamma ) )[ :3, :3 ] @ R @ v

    def matrix( self, k ):
        """The placement of the bogie's center (its axle) at retraction fraction k (0 down, 1 up)."""
        k_pt, gamma = self.knee_down_pt()
        w = self.knee_axis()
        M = about( self.trunnion, self.axis, k * self.ret )
        knee = apply( M, k_pt )
        M = M @ about( k_pt, w, k * self.knee_angle )
        M = M @ trans( self.pivot() - k * self.strut_dl * self.travel_dir )
        M = M @ rot( [ 1, 0, 0 ], k * self.roll ) @ rot( [ 0, 1, 0 ], -k * self.bogie ) @ rot( [ 0, 0, 1 ], k * self.twist )
        return M, knee


def grid( gid, isurf, nu, nw, u0 = 0.0, u1 = 1.0, w0 = 0.0, w1 = 1.0 ):
    """VSP's surface isurf of gid sampled on an nu x nw grid, ( nu, nw, 3 )."""
    us = np.linspace( u0, u1, nu )
    ws = np.linspace( w0, w1, nw )
    U, W = np.meshgrid( us, ws, indexing = "ij" )
    pts = vsp.CompVecPnt01( gid, isurf, list( U.ravel() ), list( W.ravel() ) )
    return np.array( [ [ p.x(), p.y(), p.z() ] for p in pts ] ).reshape( nu, nw, 3 )


def tire_center( gid, isurf ):
    """A tire's axle center and axle direction, from the ends of its profile, which lie on the axle."""
    a = vsp.CompPnt01( gid, isurf, 0.0, 0.0 )
    b = vsp.CompPnt01( gid, isurf, 1.0, 0.0 )
    a = np.array( [ a.x(), a.y(), a.z() ] )
    b = np.array( [ b.x(), b.y(), b.z() ] )
    return 0.5 * ( a + b ), unit( a - b )
