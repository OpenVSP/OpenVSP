#!/usr/bin/env python3
"""Convert a DATCOM input file to an OpenVSP *.vsp3 file.

    datcom2vsp.py for005_Figure07.dat                 -> for005_Figure07.vsp3
    datcom2vsp.py in.dat -o out.vsp3                  -> out.vsp3
    datcom2vsp.py in.dat --case 3                     -> only case 3
    datcom2vsp.py in.dat --dump                       -> print what was read, build nothing
    datcom2vsp.py in.dat --ref geometry.stl           -> also compare to the STL DATCOM wrote

Each NAMELIST becomes a built-in component:

    AXIBOD, ELLBOD  ->  MDC_BOD  (NAMELIST = AXIBOD or ELLBOD; Option 1 inputs only)
    PROTUB          ->  one MDC_PROTUB component per protuberance set
    FINSETn         ->  one MDC_FIN component per fin set
    INLET           ->  one MDC_INLET component

A file may hold several cases.  Namelists are kept from one case to the next when the case has a SAVE card, a
DELETE card purges saved namelists, and a new value of a saved variable replaces the old.  Every case that has
geometry and differs from those already written becomes a file: name.vsp3 for a lone case, name_caseN.vsp3 for
more.  Namelists that carry no geometry (FLTCON, TRIM, DEFLCT, EXPR, ...) and the output control cards are read
and ignored, except REFQ's SCALE, which scales every length, and the NACA card, which gives a NACA fin its section.

Lengths stay in the units of the DIM card: the model is not rescaled.

With --ref, the model is compared to a reference STL, such as the one DATCOM writes of its geometry: the STL is
imported, turned into a point cloud, and each point projected onto the nearest surface of the model.  DATCOM writes
its STL in feet whatever the DIM card, so the points are taken from feet to the DIM card's units; --ref-units names
other units.  The report gives
the distances, overall and for each component the points lie nearest, and the points farthest off.  With --ref-tol, the
script exits with an error where any point is farther than that.  OpenVSP reads standard STL; DATCOM's binary STL
files are not standard (each solid is a binary block of its own, and each facet ends in four bytes, not two), so use
the ASCII file.

Run it with an interpreter that can import openvsp:  python datcom2vsp.py in.dat
"""

import argparse
import os
import re
import sys


# ---------------------------------------------------------------------------------------------------------------
# Reading the file
# ---------------------------------------------------------------------------------------------------------------

GEOM_NAMELISTS = ( "AXIBOD", "ELLBOD", "PROTUB", "INLET" )


def is_geom_namelist( name ):
    return name in GEOM_NAMELISTS or re.fullmatch( r"FINSET[1-9]", name ) is not None


def fortran_value( tok ):
    """One value of a namelist list: a number, a logical or a name."""
    t = tok.strip().strip( "'\"" )
    u = t.upper()
    if u in ( ".TRUE.", ".T.", "TRUE", "T" ):
        return True
    if u in ( ".FALSE.", ".F.", "FALSE", "F" ):
        return False
    try:
        return float( u.replace( "D", "E" ) )
    except ValueError:
        return u


def parse_namelist_body( text ):
    """The assignments of one namelist as { VARIABLE: [values] }.  Index 1 is element 0; a gap is None."""
    out = {}
    hits = list( re.finditer( r"([A-Za-z_]\w*)\s*(?:\(\s*(\d+)\s*\))?\s*=", text ) )
    for k, m in enumerate( hits ):
        end = len( text )
        if k + 1 < len( hits ):
            end = hits[k + 1].start()
        name = m.group( 1 ).upper()
        if name == "X0":
            name = "XO"
        start = 0
        if m.group( 2 ):
            start = int( m.group( 2 ) ) - 1
        vals = []
        for item in text[m.end():end].split( "," ):
            item = item.strip()
            if not item:
                continue
            rep = re.fullmatch( r"(\d+)\s*\*\s*(.+)", item )
            if rep:
                vals.extend( [ fortran_value( rep.group( 2 ) ) ] * int( rep.group( 1 ) ) )
            else:
                vals.append( fortran_value( item ) )
        cur = out.setdefault( name, [] )
        while len( cur ) < start:
            cur.append( None )
        for i, v in enumerate( vals ):
            if start + i < len( cur ):
                cur[start + i] = v
            else:
                cur.append( v )
    return out


def overlay( dst, src ):
    """Put the variables of src over those of dst, element by element."""
    for name, vals in src.items():
        cur = dst.setdefault( name, [] )
        for i, v in enumerate( vals ):
            if v is None:
                continue
            while len( cur ) <= i:
                cur.append( None )
            cur[i] = v


class Case:
    def __init__( self, number ):
        self.number = number
        self.caseid = ""
        self.namelists = {}      # name -> { variable: [values] }, saved ones included
        self.naca = {}           # fin set number -> ( type, designation )
        self.dim = "FT"


NACA_CARD = re.compile( r"^NACA[\s,=\-.:]*(\d)[\s,=\-.:]+([1456S])[\s,=\-.:]+(.+)$", re.I )


def read_cases( path ):
    saved = {}
    saved_naca = {}
    dim = "FT"
    cases = []

    cur = Case( 1 )
    cur.namelists = {}
    new_case = True
    open_name = None
    open_text = []
    keep = False
    delete = []
    local = {}
    local_naca = {}

    def close_namelist():
        nonlocal open_name, open_text
        if open_name is not None:
            overlay( local.setdefault( open_name, {} ), parse_namelist_body( " ".join( open_text ) ) )
        open_name = None
        open_text = []

    def end_case():
        nonlocal cur, saved, saved_naca, keep, delete, local, local_naca, new_case
        close_namelist()
        for n in delete:
            saved.pop( n, None )
            if n.startswith( "NACA" ):
                saved_naca.clear()
        merged = { n: { v: list( x ) for v, x in d.items() } for n, d in saved.items() }
        for n, d in local.items():
            overlay( merged.setdefault( n, {} ), d )
        naca = dict( saved_naca )
        naca.update( local_naca )
        cur.namelists = merged
        cur.naca = naca
        cur.dim = dim
        cases.append( cur )
        if keep:
            saved = merged
            saved_naca = naca
        else:
            saved = {}
            saved_naca = {}
        cur = Case( len( cases ) + 1 )
        keep = False
        delete = []
        local = {}
        local_naca = {}
        new_case = True

    with open( path, "r", errors="replace" ) as f:
        lines = f.read().splitlines()

    for raw in lines:
        line = raw.rstrip()
        if open_name is None:
            s = line.strip()
            if not s:
                continue
            if "$" not in s:
                u = s.upper()
                new_case = False
                if s.startswith( "*" ):
                    continue
                if u.startswith( "NEXT CASE" ):
                    end_case()
                elif u.startswith( "CASEID" ):
                    cur.caseid = s[6:].strip()
                elif u.startswith( "SAVE" ):
                    keep = True
                elif u.startswith( "DELETE" ):
                    delete.extend( t.strip().upper() for t in re.split( r"[,\s]+", s[6:] ) if t.strip() )
                elif u.startswith( "DIM" ):
                    toks = u.split()
                    if len( toks ) > 1 and toks[1] in ( "IN", "FT", "CM", "M" ):
                        dim = toks[1]
                elif u.startswith( "NACA" ):
                    m = NACA_CARD.match( s )
                    if m:
                        local_naca[int( m.group( 1 ) )] = ( m.group( 2 ).upper(), m.group( 3 ).strip() )
                    else:
                        warn( "unreadable NACA card: " + s )
                continue

        # In a namelist, or a line that opens one.
        parts = line.split( "$" )
        if open_name is not None:
            open_text.append( parts[0] )
        elif parts[0].strip():
            warn( "text before $ outside a namelist ignored: " + parts[0].strip() )
        for p in parts[1:]:
            m = re.match( r"\s*(\w+)?(.*)$", p )
            name = m.group( 1 )
            rest = m.group( 2 )
            if name is None or name.upper() == "END":
                close_namelist()
                continue
            close_namelist()
            open_name = name.upper()
            open_text = [ rest ]

    if open_name is not None or local or local_naca:
        end_case()
    return cases


def warn( msg ):
    print( "datcom2vsp: warning: " + msg, file=sys.stderr )


def onoff( flag ):
    """A flag's value as an OpenVSP parm takes it."""
    if flag:
        return 1
    return 0


# ---------------------------------------------------------------------------------------------------------------
# Reading values out of a namelist
# ---------------------------------------------------------------------------------------------------------------

class NL:
    """A namelist's variables, with the DATCOM names and defaults."""

    def __init__( self, name, data, scale=1.0 ):
        self.name = name
        self.d = data
        self.scale = scale

    def has( self, var ):
        v = self.d.get( var )
        return v is not None and len( v ) > 0 and v[0] is not None

    def has_at( self, var, i ):
        v = self.d.get( var )
        return v is not None and i < len( v ) and v[i] is not None

    def count( self, var ):
        v = self.d.get( var )
        if not v:
            return 0
        n = len( v )
        while n > 0 and v[n - 1] is None:
            n -= 1
        return n

    def raw( self, var, i=0, default=None ):
        if self.has_at( var, i ):
            return self.d[var][i]
        return default

    def num( self, var, i=0, default=None, length=False ):
        v = self.raw( var, i, None )
        if v is None:
            return default
        if isinstance( v, bool ) or isinstance( v, str ):
            warn( "%s: %s is not a number: %r" % ( self.name, var, v ) )
            return default
        if length:
            return v * self.scale
        return v

    def flag( self, var, default=False ):
        v = self.raw( var, 0, None )
        if v is None:
            return default
        if isinstance( v, bool ):
            return v
        if isinstance( v, float ):
            return v != 0.0
        return default

    def enum( self, var, table, default, i=0 ):
        v = self.raw( var, i, None )
        if v is None:
            return default
        if isinstance( v, float ):
            n = int( round( v ) )
            if n in table.values():
                return n
            warn( "%s: %s = %s is not a known value" % ( self.name, var, v ) )
            return default
        if isinstance( v, str ) and v in table:
            return table[v]
        warn( "%s: %s = %s is not a known value" % ( self.name, var, v ) )
        return default


NOSE_TYPES = { "CONICAL": 0, "CONE": 0, "OGIVE": 1, "POWER": 2, "HAACK": 3, "KARMAN": 4 }
AFT_TYPES = { "CONICAL": 0, "CONE": 0, "OGIVE": 1 }
PROTUB_TYPES = { "VCYL": 1, "HCYL": 2, "LUG": 3, "SHOE": 4, "BLOCK": 5, "FAIRING": 6 }
PROTUB_MEMBERS = { 1: 1, 2: 1, 3: 4, 4: 3, 5: 1, 6: 1 }
INLET_TYPES = { "2DTOP": 1, "2DSIDE": 2, "AXI": 3 }
SECTYPS = { "HEX": 0, "NACA": 1, "ARC": 2, "USER": 3 }
HEX, NACA, ARC, USER = 0, 1, 2, 3

NACA_SERIES = { "4": 0, "4M": 1, "5": 2, "5M": 3, "1": 4, "6": 5 }


# ---------------------------------------------------------------------------------------------------------------
# Building the model
# ---------------------------------------------------------------------------------------------------------------

class Builder:
    def __init__( self, vsp, case ):
        self.vsp = vsp
        self.case = case
        self.names = case.namelists
        self.scale = 1.0
        if "REFQ" in self.names:
            self.scale = NL( "REFQ", self.names["REFQ"] ).num( "SCALE", 0, 1.0 )
        self.body = None            # the MDC_BOD's Geom ID, if there is one

    # -- helpers --------------------------------------------------------------------------------------------

    def nl( self, name ):
        return NL( name, self.names[name], self.scale )

    def setp( self, geom, name, val, group="Design" ):
        vsp = self.vsp
        pid = vsp.GetParm( geom, name, group )
        if not pid:
            warn( "no parm %s in %s" % ( name, vsp.GetGeomName( geom ) ) )
            return
        got = vsp.SetParmVal( pid, float( val ) )
        if abs( got - float( val ) ) > 1.0e-9 * max( 1.0, abs( val ) ):
            warn( "%s of %s limited to %g from %g" % ( name, vsp.GetGeomName( geom ), got, val ) )

    def setx( self, geom, x ):
        """Places a component at x along the axis: its XForm X location, which its XO, XPROT, XINLT or XLE(1) is."""
        self.setp( geom, "X_Rel_Location", x, "XForm" )

    def add_geom( self, kind, name ):
        """A new component, a child of the body where there is one, so it takes its RBODY from the body."""
        parent = ""
        if self.body is not None:
            parent = self.body
        geom = self.vsp.AddGeom( kind, parent )
        self.vsp.SetGeomName( geom, name )
        return geom

    # -- the body -------------------------------------------------------------------------------------------

    def build_body( self ):
        have_axi = "AXIBOD" in self.names
        have_ell = "ELLBOD" in self.names
        if not have_axi and not have_ell:
            return
        if have_axi and have_ell:
            warn( "both AXIBOD and ELLBOD given; building the ELLBOD" )
        ell = have_ell
        name = "ELLBOD"
        if not ell:
            name = "AXIBOD"
        b = self.nl( name )

        if b.has( "NX" ) or b.has( "X" ) or b.has( "R" ):
            warn( "%s Option 2 inputs (NX, X, R) are not supported; the body is left out" % name )
            return

        vsp = self.vsp
        g = self.add_geom( "MDC_BOD", "MDC_BOD" )
        self.body = g
        self.setp( g, "NAMELIST", onoff( ell ) )

        self.setx( g, b.num( "XO", 0, 0.0, True ) )
        self.setp( g, "TNOSE", b.enum( "TNOSE", NOSE_TYPES, 1 ) )
        self.setp( g, "POWER", b.num( "POWER", 0, 1.0 ) )
        if not b.has( "LNOSE" ):
            warn( "%s has no LNOSE" % name )
        self.setp( g, "LNOSE", b.num( "LNOSE", 0, 1.0, True ) )
        self.setp( g, "BNOSE", b.num( "BNOSE", 0, 0.0, True ) )
        self.setp( g, "TRUNC", onoff( b.flag( "TRUNC", False ) ) )
        self.setp( g, "ZTIP", b.num( "ZTIP", 0, 0.0, True ) )
        self.setp( g, "ZNOSE", b.num( "ZNOSE", 0, 0.0, True ) )
        self.setp( g, "SLPNOS", b.num( "SLPNOS", 0, 0.0 ) )

        self.setp( g, "LCENTR", b.num( "LCENTR", 0, 0.0, True ) )
        self.setp( g, "ZCENTR", b.num( "ZCENTR", 0, 0.0, True ) )
        self.setp( g, "TAFT", b.enum( "TAFT", AFT_TYPES, 0 ) )
        self.setp( g, "LAFT", b.num( "LAFT", 0, 0.0, True ) )
        self.setp( g, "ZAFT", b.num( "ZAFT", 0, 0.0, True ) )
        self.setp( g, "SLPTAL", b.num( "SLPTAL", 0, 0.0 ) )

        # The size across a station: a diameter for an AXIBOD, a width for an ELLBOD.  The nose's defaults to the
        # centerbody's, and the centerbody's to the nose's.
        across = "DNOSE", "DCENTR", "DAFT"
        if ell:
            across = "WNOSE", "WCENTR", "WAFT"
        an, ac, aa = across
        nose = b.num( an, 0, None, True )
        cen = b.num( ac, 0, None, True )
        if nose is None:
            nose = cen
        if nose is None:
            nose = 1.0 * self.scale
        self.setp( g, an, nose )
        if cen is None:
            self.setp( g, "CENTRDefault", 1 )
        else:
            self.setp( g, "CENTRDefault", 0 )
            self.setp( g, ac, cen )
        aft = b.num( aa, 0, None, True )
        if aft is None:
            if b.num( "LAFT", 0, 0.0 ) > 0.0:
                warn( "%s has an aft body but no %s" % ( name, aa ) )
        else:
            self.setp( g, aa, aft )

        # A nozzle exit is drawn into the base.
        if b.has( "DEXIT" ):
            self.setp( g, "DEXIT", b.num( "DEXIT", 0, 0.0, True ) )

        if ell:
            self.setp( g, "ENOSE", b.num( "ENOSE", 0, 1.0 ) )
            self.setp( g, "ECENTR", b.num( "ECENTR", 0, 1.0 ) )
            self.setp( g, "EAFT", b.num( "EAFT", 0, 1.0 ) )

        vsp.Update()

    # -- protuberances --------------------------------------------------------------------------------------

    def build_protub( self ):
        if "PROTUB" not in self.names:
            return
        p = self.nl( "PROTUB" )
        nprot = int( round( p.num( "NPROT", 0, 0.0 ) ) )
        if nprot == 0:
            nprot = max( p.count( "PTYPE" ), p.count( "XPROT" ), p.count( "NLOC" ) )
        if nprot > 20:
            warn( "PROTUB has more than 20 sets; the first 20 are built" )
            nprot = 20

        member = 0
        loc = 0
        for s in range( nprot ):
            ptype = p.enum( "PTYPE", PROTUB_TYPES, 1, s )
            nmem = PROTUB_MEMBERS[ptype]
            nloc = int( round( p.num( "NLOC", s, 1.0 ) ) )
            nloc = max( 1, min( nloc, 20 ) )
            xprot = p.num( "XPROT", s, 0.0, True )

            phis = []
            for k in range( nloc ):
                phis.append( p.num( "PHIPRO", loc + k, None ) )
            loc += nloc
            given = all( v is not None for v in phis )
            if not given:
                if any( v is not None for v in phis ):
                    warn( "PROTUB set %d: PHIPRO is not given for every protuberance; evenly spaced instead" % ( s + 1 ) )
                phis = [ 360.0 * k / nloc for k in range( nloc ) ]

            g = self.add_geom( "MDC_PROTUB", "MDC_PROTUB_%d" % ( s + 1 ) )
            self.setp( g, "PTYPE", ptype )
            self.setx( g, xprot )
            self.setp( g, "NLOC", nloc )
            # The Parms of each protuberance and member are made by the update.
            self.vsp.Update()
            self.setp( g, "PHIPROFlag", onoff( given ) )
            for k in range( nloc ):
                self.setp( g, "PHIPRO_%d" % ( k + 1 ), phis[k] )

            for m in range( nmem ):
                self.setp( g, "LPROT_%d" % ( m + 1 ), p.num( "LPROT", member + m, 1.0, True ) )
                self.setp( g, "WPROT_%d" % ( m + 1 ), p.num( "WPROT", member + m, 1.0, True ) )
                self.setp( g, "HPROT_%d" % ( m + 1 ), p.num( "HPROT", member + m, 1.0, True ) )
                if ptype == 6:
                    self.setp( g, "OPROT_%d" % ( m + 1 ), 0.0 )
                else:
                    self.setp( g, "OPROT_%d" % ( m + 1 ), p.num( "OPROT", member + m, 0.0, True ) )
            member += nmem

            if self.body is None:
                self.setp( g, "RBODY", 1.0e-6 )

    # -- inlets ---------------------------------------------------------------------------------------------

    def build_inlet( self ):
        if "INLET" not in self.names:
            return
        n = self.nl( "INLET" )
        nin = int( round( n.num( "NIN", 0, 1.0 ) ) )
        nin = max( 1, min( nin, 20 ) )
        intype = n.enum( "INTYPE", INLET_TYPES, 1 )
        xinlt = n.num( "XINLT", 0, 0.0, True )

        phis = [ n.num( "PHI", k, None ) for k in range( nin ) ]
        given = all( v is not None for v in phis )
        if not given:
            if any( v is not None for v in phis ):
                warn( "INLET: PHI is not given for every inlet; evenly spaced instead" )
            phis = [ 360.0 * k / nin for k in range( nin ) ]

        g = self.add_geom( "MDC_INLET", "MDC_INLET" )
        self.setp( g, "INTYPE", intype )
        self.setx( g, xinlt )
        self.setp( g, "XDIV", n.num( "XDIV", 0, 0.0, True ) )
        self.setp( g, "HDIV", n.num( "HDIV", 0, 0.0, True ) )
        self.setp( g, "LDIV", n.num( "LDIV", 0, 0.0, True ) )
        self.setp( g, "NIN", nin )
        # The Parms of each inlet are made by the update.
        self.vsp.Update()
        self.setp( g, "PHIFlag", onoff( given ) )
        for k in range( nin ):
            self.setp( g, "PHI_%d" % ( k + 1 ), phis[k] )

        # Stations 2 to 5 have an X and an H; X(1) and H(1) are zero by definition.
        for i in range( 1, 5 ):
            self.setp( g, "X%d" % ( i + 1 ), n.num( "X", i, 0.0, True ) )
            if intype != 3:
                self.setp( g, "H%d" % ( i + 1 ), n.num( "H", i, 0.0, True ) )
        for i in range( 5 ):
            self.setp( g, "W%d" % ( i + 1 ), n.num( "W", i, 0.0, True ) )

        self.setp( g, "COVER", onoff( n.flag( "COVER", False ) ) )
        if n.has( "RAMP" ):
            self.setp( g, "RAMP", n.num( "RAMP", 0, 15.0 ) )

        if self.body is None:
            self.setp( g, "RBODY", 1.0e-6 )

    # -- fins -----------------------------------------------------------------------------------------------

    def build_fins( self ):
        for k in range( 1, 10 ):
            name = "FINSET%d" % k
            if name in self.names:
                self.build_fin( k, self.nl( name ) )

    def naca_card( self, g, k, f ):
        """Sets the NACA section parms from the NACA card of fin set k."""
        vsp = self.vsp
        card = self.case.naca.get( k )
        if card is None:
            warn( "FINSET%d is SECTYP = NACA but there is no NACA card for it; a NACA 0012 is used" % k )
            self.setp( g, "NACASER", 0 )
            self.setp( g, "NACADES", 12.0 )
            return
        kind, des = card
        des = re.sub( r"\s+", "", des.upper() )

        a = None
        m = re.search( r"A\s*=\s*([0-9.]+)", des )
        if m:
            a = float( m.group( 1 ) )
            des = des[:m.start()]
        des = des.strip( ",=" )

        def num( s ):
            try:
                return float( s )
            except ValueError:
                warn( "FINSET%d: cannot read NACA designation %s" % ( k, card[1] ) )
                return 12.0

        if kind == "4" or kind == "5":
            body, _, mod = des.partition( "-" )
            ser = kind
            if mod:
                ser = kind + "M"
                self.setp( g, "NACAMOD", int( num( mod ) ) )
            self.setp( g, "NACASER", NACA_SERIES[ser] )
            self.setp( g, "NACADES", num( body ) )
        elif kind == "1":
            # NACA-1-1-16-212: series and position of minimum pressure, then Cl in tenths and thickness.
            parts = des.split( "-" )
            tail = parts[-1]
            if len( parts ) != 2 or not parts[0] or not tail:
                warn( "FINSET%d: cannot read NACA designation %s" % ( k, card[1] ) )
            self.setp( g, "NACASER", NACA_SERIES["1"] )
            self.setp( g, "NACADES", num( tail ) )
        elif kind == "6":
            # NACA-1-6-64-005, 65A010: series and position of minimum pressure, a dash or A, then Cl and thickness.
            m = re.fullmatch( r"6?(\d)(A|-)?-?(\d+(?:\.\d+)?)", des )
            if not m:
                warn( "FINSET%d: cannot read NACA designation %s" % ( k, card[1] ) )
            else:
                pos = m.group( 1 )
                self.setp( g, "NACASER", NACA_SERIES["6"] )
                self.setp( g, "NACASEA", onoff( m.group( 2 ) == "A" ) )
                self.setp( g, "NACADES", num( pos + m.group( 3 ) ) )
                if a is not None:
                    self.setp( g, "NACAMEAN", a )
                if int( pos ) < 3 or int( pos ) > 7:
                    warn( "FINSET%d: the 6%s series is built as the nearest series OpenVSP has" % ( k, pos ) )
        else:
            # Supersonic: type, position of maximum thickness in %, thickness in %, and for the hexagon its flat.
            parts = [ p for p in re.split( r"-", des ) if p ]
            t = 1
            if parts:
                t = int( num( parts[0] ) )
            self.setp( g, "NACASER", { 1: 6, 2: 7, 3: 8 }.get( t, 6 ) )
            if len( parts ) > 1:
                self.setp( g, "NACAXMAX", num( parts[1] ) )
            if len( parts ) > 2:
                self.setp( g, "NACADES", num( parts[2] ) )
            if t == 3 and len( parts ) > 3:
                self.setp( g, "NACAFLAT", num( parts[3] ) )

    def user_airfoil( self, g, k, f ):
        """Gives a USER section the airfoil of XCORD with MEAN and THICK, or YUPPER and YLOWER."""
        vsp = self.vsp
        n = f.count( "XCORD" )
        if n < 2:
            warn( "FINSET%d is SECTYP = USER but has no XCORD; its section is left a biconvex airfoil" % k )
            return
        xs = [ f.num( "XCORD", i, 0.0 ) for i in range( n ) ]
        if f.count( "YUPPER" ) >= n and f.count( "YLOWER" ) >= n:
            up = [ f.num( "YUPPER", i, 0.0 ) for i in range( n ) ]
            lo = [ f.num( "YLOWER", i, 0.0 ) for i in range( n ) ]
        elif f.count( "MEAN" ) >= n and f.count( "THICK" ) >= n:
            mean = [ f.num( "MEAN", i, 0.0 ) for i in range( n ) ]
            thick = [ f.num( "THICK", i, 0.0 ) for i in range( n ) ]
            up = [ mean[i] + 0.5 * thick[i] for i in range( n ) ]
            lo = [ mean[i] - 0.5 * thick[i] for i in range( n ) ]
        else:
            warn( "FINSET%d USER needs XCORD with MEAN and THICK, or with YUPPER and YLOWER" % k )
            return
        order = sorted( range( n ), key=lambda i: xs[i] )
        upper = [ vsp.vec3d( xs[i], up[i], 0.0 ) for i in order ]
        lower = [ vsp.vec3d( xs[i], lo[i], 0.0 ) for i in order ]

        vsp.SetParmVal( g, "SECTYP", "Design", float( USER ) )
        vsp.Update()
        xsec = vsp.GetMdcFinUserXSec( g )
        if not xsec:
            warn( "FINSET%d has no User section to set" % k )
            return
        vsp.SetAirfoilPnts( xsec, upper, lower )

    def build_fin( self, k, f ):
        vsp = self.vsp
        nspan = f.count( "SSPAN" )
        if nspan < 2:
            warn( "FINSET%d needs SSPAN at 2 stations at least; left out" % k )
            return
        if nspan > 10:
            nspan = 10
        sectyp = f.enum( "SECTYP", SECTYPS, HEX )
        g = self.add_geom( "MDC_FIN", "MDC_FIN_%d" % k )
        self.setp( g, "NSPAN", nspan )
        self.setp( g, "SECTYP", sectyp )

        npanel = int( round( f.num( "NPANEL", 0, 4.0 ) ) )
        npanel = max( 1, min( npanel, 8 ) )
        self.setp( g, "NPANEL", npanel )
        # The Parms of each station and panel are made by the update.
        self.vsp.Update()
        phis = [ f.num( "PHIF", i, None ) for i in range( npanel ) ]
        given = all( v is not None for v in phis )
        if not given:
            if any( v is not None for v in phis ):
                warn( "FINSET%d: PHIF is not given for every panel; evenly spaced instead" % k )
            phis = [ 360.0 * i / npanel for i in range( npanel ) ]
        self.setp( g, "PHIFFlag", onoff( given ) )
        for i in range( npanel ):
            self.setp( g, "PHIF_%d" % ( i + 1 ), phis[i] )
            self.setp( g, "GAM_%d" % ( i + 1 ), f.num( "GAM", i, 0.0 ) )

        # Station and segment inputs.  The root's leading edge is the X location, and a later station's XLE is from it.
        xle1 = f.num( "XLE", 0, 0.0, True )
        self.setx( g, xle1 )
        for i in range( nspan ):
            s = "_" + str( i + 1 )
            self.setp( g, "SSPAN" + s, f.num( "SSPAN", i, 0.0, True ) )
            self.setp( g, "CHORD" + s, f.num( "CHORD", i, 0.0, True ) )
            self.setp( g, "CFOC" + s, f.num( "CFOC", i, 1.0 ) )
            if i > 0 and f.has_at( "XLE", i ):
                self.setp( g, "XLE" + s, f.num( "XLE", i, 0.0, True ) - xle1 )
            if i + 1 < nspan:
                self.setp( g, "XLEFlag" + s, onoff( f.has_at( "XLE", i + 1 ) ) )
                self.setp( g, "SWEEP" + s, f.num( "SWEEP", i, 0.0 ) )
                self.setp( g, "STA" + s, f.num( "STA", i, 1.0 ) )
        if not f.has( "XLE" ):
            warn( "FINSET%d has no XLE(1); the leading edge is at the origin" % k )

        if sectyp in ( HEX, ARC ):
            # The lower surface's values are given outright, whether or not they were input.
            self.setp( g, "ZLOWERRelFlag", 0 )
            self.setp( g, "LMAXLRelFlag", 0 )
            self.setp( g, "LFLATLRelFlag", 0 )
            for i in range( nspan ):
                s = "_" + str( i + 1 )
                zu = f.num( "ZUPPER", i, 0.025 )
                mu = f.num( "LMAXU", i, 0.5 )
                fu = f.num( "LFLATU", i, 0.0 )
                self.setp( g, "ZUPPER" + s, zu )
                self.setp( g, "ZLOWER" + s, f.num( "ZLOWER", i, zu ) )
                self.setp( g, "LMAXU" + s, mu )
                self.setp( g, "LMAXL" + s, f.num( "LMAXL", i, mu ) )
                self.setp( g, "LFLATU" + s, fu )
                self.setp( g, "LFLATL" + s, f.num( "LFLATL", i, fu ) )
                self.setp( g, "LER" + s, f.num( "LER", i, 0.0, True ) )
        elif sectyp == NACA:
            self.naca_card( g, k, f )

        # The root sits on the body at the middle of the root chord, unless SSPAN(1) puts it elsewhere.  The body
        # gives the radius there; with no body the root is on the axis.
        if f.num( "SSPAN", 0, 0.0 ) <= 1.0e-12 and self.body is None:
            self.setp( g, "RBODY", 0.0 )

        if sectyp == USER:
            self.user_airfoil( g, k, f )

    # -- the whole model ------------------------------------------------------------------------------------

    def build( self ):
        self.vsp.ClearVSPModel()
        self.build_body()
        self.build_protub()
        self.build_inlet()
        self.build_fins()
        self.vsp.Update()


# ---------------------------------------------------------------------------------------------------------------
# Comparison with a reference STL
# ---------------------------------------------------------------------------------------------------------------

# A length unit in feet.
FEET = { "FT": 1.0, "IN": 1.0 / 12.0, "M": 1.0 / 0.3048, "CM": 1.0 / 30.48 }


def compare_to_stl( vsp, stl, scale, worst=10 ):
    """The distance of each vertex of the STL, its coordinates times scale, from the model's surfaces, reported
    overall, for each component and for the points farthest off.  Answers the largest distance, or None where the STL
    gave no points."""
    geoms = list( vsp.FindGeoms() )

    mesh = vsp.ImportFile( stl, vsp.IMPORT_STL, "" )
    pts = []
    made = []
    if mesh:
        made.append( mesh )
        cloud = vsp.CreatePtCloudGeom( mesh )
        if cloud:
            made.append( cloud )
            pts = [ vsp.vec3d( p.x() * scale, p.y() * scale, p.z() * scale ) for p in vsp.GetPtCloudPnts( cloud ) ]
    if made:
        vsp.DeleteGeomVec( made )
    if not pts:
        warn( "%s gave no points" % stl )
        return None

    # The nearest surface of the model to each point.  The largest surfaces go first, and a point is projected onto
    # a surface only where its bounding box is nearer than the nearest surface so far.
    surfs = []
    for g in geoms:
        nmain = vsp.GetNumMainSurfs( g )
        for s in range( vsp.GetTotalNumSurfs( g ) ):
            box = None
            if s < nmain:
                box = ( vsp.GetGeomBBoxMin( g, s, True ), vsp.GetGeomBBoxMax( g, s, True ) )
            size = float( "inf" )
            if box:
                size = ( box[1] - box[0] ).mag()
            surfs.append( ( -size, g, s, box ) )
    surfs.sort( key=lambda e: e[0] )

    def box_dist( p, box ):
        lo, hi = box
        d2 = 0.0
        for v, a, b in ( ( p.x(), lo.x(), hi.x() ), ( p.y(), lo.y(), hi.y() ), ( p.z(), lo.z(), hi.z() ) ):
            e = max( a - v, 0.0, v - b )
            d2 += e * e
        return d2 ** 0.5

    dist = [ float( "inf" ) ] * len( pts )
    near = [ None ] * len( pts )
    for _, g, s, box in surfs:
        idx = range( len( pts ) )
        if box:
            idx = [ i for i in idx if box_dist( pts[i], box ) < dist[i] ]
        if not idx:
            continue
        us, ws, ds = vsp.ProjVecPnt01( g, s, [ pts[i] for i in idx ] )
        for i, d in zip( idx, ds ):
            if d < dist[i]:
                dist[i] = d
                near[i] = g

    def stats( idx ):
        ds = [ dist[i] for i in idx ]
        n = len( ds )
        rms = ( sum( d * d for d in ds ) / n ) ** 0.5
        far = max( idx, key=lambda i: dist[i] )
        p = pts[far]
        return "%6d points  mean %.4g  rms %.4g  max %.4g at ( %.4g, %.4g, %.4g )" % (
            n, sum( ds ) / n, rms, dist[far], p.x(), p.y(), p.z() )

    xs = [ p.x() for p in pts ]
    size = max( xs ) - min( xs )
    print( "compared with %s, %g long" % ( stl, size ) )
    print( "  all          %s" % stats( range( len( pts ) ) ) )
    for g in geoms:
        idx = [ i for i in range( len( pts ) ) if near[i] == g ]
        if idx:
            print( "  %-12s %s" % ( vsp.GetGeomName( g ), stats( idx ) ) )

    order = sorted( range( len( pts ) ), key=lambda i: -dist[i] )[:worst]
    print( "  farthest:" )
    for i in order:
        p = pts[i]
        print( "    %.4g at ( %.4g, %.4g, %.4g ), nearest %s" % ( dist[i], p.x(), p.y(), p.z(),
                                                                vsp.GetGeomName( near[i] ) ) )
    return max( dist )


# ---------------------------------------------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------------------------------------------

def geometry_key( case ):
    """What a case builds from, to tell whether it changed."""
    key = []
    for name in sorted( case.namelists ):
        if is_geom_namelist( name ) or name == "REFQ":
            d = dict( case.namelists[name] )
            if name == "REFQ":
                d = { "SCALE": d.get( "SCALE" ) }
            key.append( ( name, sorted( d.items(), key=lambda kv: kv[0] ) ) )
    key.append( sorted( case.naca.items() ) )
    return repr( key )


def has_geometry( case ):
    return any( is_geom_namelist( n ) for n in case.namelists )


def main():
    ap = argparse.ArgumentParser( description="Convert a DATCOM input file to an OpenVSP .vsp3 file." )
    ap.add_argument( "input", help="DATCOM input file (for005.dat)" )
    ap.add_argument( "-o", "--output", help="output file, or with several cases its stem" )
    ap.add_argument( "--case", type=int, help="convert only this case number" )
    ap.add_argument( "--dump", action="store_true", help="print the geometry namelists read, and build nothing" )
    ap.add_argument( "--ref", help="a reference STL of the geometry, such as DATCOM writes, to compare the model with; "
                     "with several cases, choose one with --case" )
    ap.add_argument( "--ref-units", choices=sorted( FEET ), default="FT",
                     help="the length unit of the reference STL; DATCOM writes feet (default FT)" )
    ap.add_argument( "--ref-tol", type=float, help="exit with an error where a point of the reference is farther than "
                     "this from the model" )
    args = ap.parse_args()

    cases = read_cases( args.input )
    if not cases:
        print( "datcom2vsp: no cases found in " + args.input, file=sys.stderr )
        return 1

    stem = args.output
    if stem is None:
        stem = os.path.splitext( args.input )[0] + ".vsp3"
    base, ext = os.path.splitext( stem )
    if ext.lower() != ".vsp3":
        base = stem
        ext = ".vsp3"

    todo = []
    seen = set()
    for c in cases:
        if args.case is not None and c.number != args.case:
            continue
        if not has_geometry( c ):
            continue
        key = geometry_key( c )
        if args.case is None and key in seen:
            continue
        seen.add( key )
        todo.append( c )

    if not todo:
        msg = "datcom2vsp: no case with geometry"
        if args.case is not None:
            msg += " numbered %d" % args.case
        print( msg, file=sys.stderr )
        return 1

    if args.ref and len( todo ) > 1:
        print( "datcom2vsp: --ref compares one case; choose it with --case", file=sys.stderr )
        return 1

    if args.dump:
        for c in todo:
            print( "case %d  %s  (DIM %s)" % ( c.number, c.caseid, c.dim ) )
            for name in sorted( c.namelists ):
                if is_geom_namelist( name ) or name == "REFQ":
                    print( "  $" + name )
                    for v, x in c.namelists[name].items():
                        print( "     %s = %s" % ( v, x ) )
            for k, card in sorted( c.naca.items() ):
                print( "  NACA card, fin set %d: %s" % ( k, card ) )
        return 0

    import openvsp as vsp

    for c in todo:
        b = Builder( vsp, c )
        b.build()
        if len( todo ) == 1:
            out = base + ext
        else:
            out = "%s_case%d%s" % ( base, c.number, ext )
        vsp.WriteVSPFile( out, vsp.SET_ALL )
        print( "case %d -> %s" % ( c.number, out ) )

        if args.ref:
            far = compare_to_stl( vsp, args.ref, FEET[args.ref_units] / FEET.get( c.dim.upper(), 1.0 ) )
            if far is None:
                return 1
            if args.ref_tol is not None and far > args.ref_tol:
                print( "datcom2vsp: a point of %s is %g from the model, more than %g" % ( args.ref, far, args.ref_tol ),
                       file=sys.stderr )
                return 1

    return 0


if __name__ == "__main__":
    sys.exit( main() )
