"""Test the OpenVSP Python Interface."""
from __future__ import print_function

import vsp as vsp

stdout = vsp.cvar.cstdout
errorMgr = vsp.ErrorMgrSingleton_getInstance()


# Add Pod
gid = vsp.AddGeom("POD", "")
errorMgr.PopErrorAndPrint(stdout)

vsp.Update()

surf_indx = 0

u = 0.12345
w = 0.67890

pnt = vsp.CompPnt01( gid, surf_indx, u, w )
print( "Point: x " + str(pnt.x()) + " y " + str(pnt.y()) + " z " + str(pnt.z()) )

norm = vsp.CompNorm01( gid, surf_indx, u, w )
print( "Norm: x " + str(norm.x()) + " y " + str(norm.y()) + " z " + str(norm.z()) )

tanu = vsp.CompTanU01( gid, surf_indx, u, w )
print( "Tan U: x " + str(tanu.x()) + " y " + str(tanu.y()) + " z " + str(tanu.z()) )

tanw = vsp.CompTanW01( gid, surf_indx, u, w )
print( "Tan W: x " + str(tanw.x()) + " y " + str(tanw.y()) + " z " + str(tanw.z()) )

k1, k2, ka, kg = vsp.CompCurvature01( gid, surf_indx, u, w )
print( "Curvature : k1 " + str(k1) + " k2 " + str(k2) + " ka " + str(ka) + " kg " + str(kg) )


pnt.set_xyz( pnt.x()+norm.x(), pnt.y()+norm.y(), pnt.z()+norm.z() )

d, uout, wout = vsp.ProjPnt01( gid, surf_indx, pnt )
print( "Dist " + str(d) + " u " + str(uout) + " w " + str(wout) )

d, surf_indx_out, uout, wout = vsp.ProjPnt01I( gid, pnt )
print( "Dist " + str(d) + " u " + str(uout) + " w " + str(wout) + " surf_index " + str(surf_indx_out) )

d, uout, wout = vsp.ProjPnt01Guess( gid, surf_indx, pnt, u + 0.1, w + 0.1 )
print( "Dist " + str(d) + " u " + str(uout) + " w " + str(wout) )



n = 5;

uvec=[]
wvec=[]


for i in range(n):
    uvec.append( (i+1)*1.0/(n+1) )
    wvec.append( (n-i)*1.0/(n+1) )


ptvec = vsp.CompVecPnt01( gid, surf_indx, uvec, wvec )
normvec = vsp.CompVecNorm01( gid, surf_indx, uvec, wvec )
k1vec, k2vec, kavec, kgvec = vsp.CompVecCurvature01( gid, surf_indx, uvec, wvec )


print( "Surface query:" );

for i in range(len(ptvec)):
    print("u " + str(uvec[i]) + " w " + str(wvec[i]) + " Point: x " + str(ptvec[i].x()) + " y " + str(ptvec[i].y()) + " z " + str(ptvec[i].z()) + " Norm: x " + str(normvec[i].x()) + " y " + str(normvec[i].y()) + " z " + str(normvec[i].z()) )
    print( "Curvature : k1 " + str(k1vec[i]) + " k2 " + str(k2vec[i]) + " ka " + str(kavec[i]) + " kg " + str(kgvec[i]) )
    ptvec[i].set_xyz( ptvec[i].x() + normvec[i].x(),ptvec[i].y() + normvec[i].y(),ptvec[i].z() + normvec[i].z() )


uoutv, woutv, doutv = vsp.ProjVecPnt01( gid, surf_indx, ptvec )

print( "Surface projection:" );

for i in range(len(uoutv)):
    print( "u " + str(uoutv[i]) + " w " + str(woutv[i]) + " dist " + str(doutv[i]) )


u0v=[]
w0v=[]

for i in range(n):
    u0v.append( uvec[i] + 0.01234 )
    w0v.append( wvec[i] + 0.05678 )

uoutv, woutv, doutv = vsp.ProjVecPnt01Guess( gid, surf_indx, ptvec, u0v,  w0v )

print( "Surface projection with guesses:" );

for i in range(len(uoutv)):
    print( "u " + str(uoutv[i]) + " w " + str(woutv[i]) + " dist " + str(doutv[i]) )



utess, wtess = vsp.GetUWTess01( gid, surf_indx );

for i in range(len(utess)):
    print( "utess: " + str(utess[i]) )

for j in range(len(wtess)):
    print( "wtess: " + str(wtess[j]) )

# Check for errors

num_err = errorMgr.GetNumTotalErrors()
for i in range(0, num_err):
    err = errorMgr.PopLastError()
    print("error = ", err.m_ErrorString)
