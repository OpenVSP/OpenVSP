# Python API Test
#
#

import vsp_g
vsp=vsp_g
#import vsp

stdout = vsp.cvar.cstdout
errorMgr = vsp.ErrorMgrSingleton_getInstance()

#==== Use Case 1 ==== #
vsp.VSPCheckSetup();
errorMgr.PopErrorAndPrint( stdout )

types = vsp.GetGeomTypes()
errorMgr.PopErrorAndPrint( stdout )

print types

# Add Fuse
fuse_id = vsp.AddGeom( "FUSELAGE" )
errorMgr.PopErrorAndPrint( stdout )

# Add Pod
pod_id = vsp.AddGeom( "POD", fuse_id )
errorMgr.PopErrorAndPrint( stdout )

# Set Name
vsp.SetGeomName( pod_id, "Pod" )
errorMgr.PopErrorAndPrint( stdout )

# Change Length
len_id = vsp.GetParm( pod_id, "Length", "Design" )
vsp.SetParmVal( len_id, 7.0 )

# Change Finess Ratio
vsp.SetParmVal( pod_id, "FineRatio", "Design", 10.0 )

# Change Y Location
y_loc_id = vsp.GetParm( pod_id, "Y_Location", "XForm" )
vsp.SetParmVal( y_loc_id, 1.0 )

# Change X Location
vsp.SetParmVal( pod_id, "X_Location", "XForm", 3.0 )

# Change Symmetry
sym_flag_id = vsp.GetParm( pod_id, "Sym_Planar_Flag", "Sym" )
vsp.SetParmVal( sym_flag_id, vsp.SYM_XZ )

# Copy Pod Geom
vsp.CopyGeomToClipboard( pod_id )
vsp.PasteGeomClipboard( fuse_id ) # make fuse parent

# Set Name
vsp.SetGeomName( pod_id, "Original_Pod" )
second_pod_id = vsp.FindGeom( "Pod", 0 )

# Change Location and Symmetry
vsp.SetParmVal( second_pod_id, "Sym_Planar_Flag", "Sym", 0 )
vsp.SetParmVal( second_pod_id, "Y_Location", "XForm", 0.0 )
vsp.SetParmVal( second_pod_id, "Z_Location", "XForm", 1.0 )

fname = "apitest1.vsp3"

vsp.WriteVSPFile( fname )

geoms = vsp.FindGeoms()

print "All geoms in Vehicle."
print geoms

errorMgr.PopErrorAndPrint( stdout )

#==== Use Case 2 ====#

vsp.VSPRenew();
errorMgr.PopErrorAndPrint( stdout )

geoms = vsp.FindGeoms()

print "All geoms in Vehicle."
print geoms

# Add Fuse
fuse_id = vsp.AddGeom( "FUSELAGE" )

# Get XSec Surf ID
xsurf_id = vsp.GetXSecSurf( fuse_id, 0 )

# Change Type of First XSec
vsp.ChangeXSecShape( xsurf_id, 0, vsp.XS_SUPER_ELLIPSE )
errorMgr.PopErrorAndPrint( stdout )

# Change Type First XSec Properties
xsec_id = vsp.GetXSec( xsurf_id, 0 )
width_id = vsp.GetXSecParm( xsec_id, "Super_Width" )
height_id = vsp.GetXSecParm( xsec_id, "Super_Height" )
vsp.SetParmVal( width_id, 4.0 )
vsp.SetParmVal( height_id, 2.0 )

# Copy Cross-Section to Clipboard
vsp.CopyXSec( xsurf_id, 0)

# Paste Cross-Section
vsp.PasteXSec( xsurf_id, 1 )
vsp.PasteXSec( xsurf_id, 2 )
vsp.PasteXSec( xsurf_id, 3 )

# Change Type to File XSec

vsp.ChangeXSecShape( xsurf_id, 0, vsp.XS_FILE_FUSE )
file_xsec_id = vsp.GetXSec( xsurf_id, 0 )

# Build Point Vec

pnt_vec = vsp.Vec3dVec();
pnt_vec.push_back( vsp.vec3d( 0.0, 2.0, 0.0 ) )
pnt_vec.push_back( vsp.vec3d( 1.0, 0.0, 0.0 ) )
pnt_vec.push_back( vsp.vec3d( 0.0,-2.0, 0.0 ) )
pnt_vec.push_back( vsp.vec3d(-1.0, 0.0, 0.0 ) )
pnt_vec.push_back( vsp.vec3d( 0.0, 2.0, 0.0 ) )

# Load Points Into XSec
vsp.SetXSecPnts( file_xsec_id, pnt_vec )

geoms = vsp.FindGeoms()

print "End of second use case, all geoms in Vehicle."
print geoms

vsp.WriteVSPFile("apitest2.vsp3")

#==== Use Case 3 ====#

print "Start of third use case, read in first-case file."

#==== Read Geometry From File ====#
vsp.VSPRenew();
errorMgr.PopErrorAndPrint( stdout )

vsp.ReadVSPFile( fname )

geoms = vsp.FindGeoms()

print "All geoms in Vehicle."
print geoms

# Check for errors

num_err = errorMgr.GetNumTotalErrors()
for i in range(0,num_err):
	err = errorMgr.PopLastError()
	print "error = ", err.m_ErrorString

