# An API call handed an ID that names nothing reports it under its own name, with the code for the
# kind of ID: VSP_INVALID_GEOM_ID for a Geom, VSP_INVALID_XSEC_ID for an XSec, VSP_CANT_FIND_PARM
# for a Parm and VSP_INVALID_ID for anything else.  The calls and their arguments are read from
# VSP_Geom_API.h, so each call is checked as soon as it is declared.

import functools
import inspect
import os
import re

import openvsp as vsp

from testhelp import drop_errors


HEADER = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "..", "..", "..", "geom_api", "VSP_Geom_API.h" )

# ID arguments that name a Geom, an XSec or a Parm; any other ID argument names something else
GEOM_IDS = { "geom_id", "geomid", "startgeomid", "midgeomid", "endgeomid", "parent_id", "parent_geom_id",
             "target_geom_id", "ancestor_id", "original_id", "clone_id", "bor_id", "gear_id", "prop_id",
             "routing_id", "wing_id" }
XSEC_IDS = { "xsec_id" }
PARM_IDS = { "parm_id", "parm_a_id", "parm_b_id" }

# ID arguments whose code their names do not give, by call
CODE_OF = {
    ( "GetParm", "geom_id" ): vsp.VSP_INVALID_ID,       # any Parm container
    ( "GetParmVal", "geom_id" ): vsp.VSP_INVALID_ID,
    ( "SetParmVal", "geom_id" ): vsp.VSP_INVALID_ID,
    ( "SetParmValUpdate", "geom_id" ): vsp.VSP_INVALID_ID,
    ( "ControlSurfaceHingeLine", "id" ): vsp.VSP_INVALID_GEOM_ID,
    ( "DeleteUserParm", "id" ): vsp.VSP_CANT_FIND_PARM,
    ( "GetXSecAlias", "id" ): vsp.VSP_INVALID_XSEC_ID,
    ( "SetXSecAlias", "id" ): vsp.VSP_INVALID_XSEC_ID,
    ( "GetXSecCurveAlias", "id" ): vsp.VSP_INVALID_XSEC_ID,
    ( "SetXSecCurveAlias", "id" ): vsp.VSP_INVALID_XSEC_ID,
    ( "GetDesignVarTypeByID", "parm_id" ): vsp.VSP_INVALID_ID,         # a design variable
    ( "DeleteDesignVar", "parm_id" ): vsp.VSP_INVALID_ID,
    ( "AddAdvLinkInput", "parm_id" ): vsp.VSP_INDEX_OUT_RANGE,         # the link index comes first
    ( "AddAdvLinkOutput", "parm_id" ): vsp.VSP_INDEX_OUT_RANGE,
    ( "SetFitModelTargetPtGeom", "geom_id" ): vsp.VSP_INDEX_OUT_RANGE, # the point index comes first
}


def split_args( text ):
    """The arguments of a declaration, without their defaults."""
    args, depth, cur = [], 0, ""
    for ch in text:
        if ch in "(<":
            depth += 1
        elif ch in ")>":
            depth -= 1
        if ch == "," and depth == 0:
            args.append( cur )
            cur = ""
        else:
            cur += ch
    if cur.strip():
        args.append( cur )
    return [ a.split( "=" )[0].strip() for a in args ]


def declarations():
    """Each declared call, with a list of ( type, name ) for its arguments per overload."""
    s = open( HEADER ).read()
    s = re.sub( r"/\*.*?\*/", "", s, flags=re.S )
    s = re.sub( r"//[^\n]*", "", s )
    calls = {}
    for m in re.finditer( r"^extern\s+[^;{]*?\b(\w+)\s*\(([^;{]*)\)\s*;", s, re.M | re.S ):
        args = []
        for a in split_args( m.group( 2 ) ):
            am = re.match( r"(.*?)(\w+)$", a, re.S )
            args.append( ( " ".join( am.group( 1 ).split() ), am.group( 2 ) ) )
        calls.setdefault( m.group( 1 ), [] ).append( args )
    return calls


def is_id( arg_type, arg_name ):
    return "string" in arg_type and "vector" not in arg_type and re.search( r"id$", arg_name, re.I ) is not None


def plain_value( arg_type ):
    if "vector" in arg_type:
        return []
    if "string" in arg_type:
        return ""
    if "vec3d" in arg_type:
        return vsp.vec3d()
    if "Matrix4d" in arg_type:
        return vsp.Matrix4d()
    if "bool" in arg_type:
        return False
    if "double" in arg_type:
        return 0.0
    return 0


def codes_for( name, ids ):
    codes = set()
    for i in ids:
        if ( name, i ) in CODE_OF:
            codes.add( CODE_OF[ ( name, i ) ] )
        elif i in GEOM_IDS:
            codes.add( vsp.VSP_INVALID_GEOM_ID )
        elif i in XSEC_IDS:
            codes.add( vsp.VSP_INVALID_XSEC_ID )
        elif i in PARM_IDS:
            codes.add( vsp.VSP_CANT_FIND_PARM )
        else:
            codes.add( vsp.VSP_INVALID_ID )
    return codes


@functools.lru_cache( maxsize=None )
def bad_id_reports():
    """For each call that takes an ID, handed "BADID" for every ID and a plain value for the rest:
    its name, its ID arguments, whether it set the error flag and the errors it reported, first
    first."""
    mgr = vsp.ErrorMgrSingleton.getInstance()
    reports = []
    for name, overloads in sorted( declarations().items() ):
        for args in overloads:
            types = dict( ( n, t ) for t, n in args )
            # Python takes output arguments as return values, and names the rest unless the
            # call is overloaded; a name that is a Python keyword gains a leading _
            names = [ n.lstrip( "_" ) for n in inspect.signature( getattr( vsp, name ) ).parameters ]
            if len( overloads ) > 1 or names == [ "args" ]:
                names = [ n for t, n in args if "&" not in t or "const" in t ]
            ids = tuple( n for n in names if is_id( types[ n ], n ) )
            if not ids:
                continue
            vsp.VSPRenew()
            vsp.AddGeom( "POD" )
            drop_errors()
            getattr( vsp, name )( *[ "BADID" if n in ids else plain_value( types[ n ] ) for n in names ] )
            flagged = mgr.GetErrorLastCallFlag()
            errs = [ mgr.PopLastError() for _ in range( mgr.GetNumTotalErrors() ) ]
            reports.append( ( name, ids, flagged, [ ( e.GetErrorCode(), e.GetErrorString() ) for e in reversed( errs ) ] ) )
    return reports


def testABadIDIsReportedUnderTheCallsNameWithItsCode():
    wrong = []
    for name, ids, flagged, errs in bad_id_reports():
        if not errs:
            continue
        if not all( msg.startswith( name + "::" ) for code, msg in errs ) or errs[0][0] not in codes_for( name, ids ):
            wrong.append( ( name, ids, errs ) )
    assert not wrong, wrong
