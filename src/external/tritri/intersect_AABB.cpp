/**
 *   A method to compute a ray-AABB intersection.
 *   Original code by Andrew Woo, from "Graphics Gems", Academic Press, 1990
 *   Optimized code by Pierre Terdiman, 2000 (~20-30% faster on my Celeron 500)
 *   Epsilon value added by Klaus Hartmann. (discarding it saves a few cycles only)
 *   Binary comparison optimization removed by Rob McDonald, 2014.
 *
 *   Hence this version is faster as well as more robust than the original one.
 *
 *   Should work provided:
 *   1) the integer representation of 0.0f is 0x00000000
 *   2) the sign bit of the double is the most significant one
 *
 *   Report bugs: p.terdiman@codercorner.com
 *
 *   \param      aabb      [in] the axis-aligned bounding box
 *   \param      origin    [in] ray origin
 *   \param      dir       [in] ray direction
 *   \param      coord     [out] impact coordinates
 *   \return     true if ray intersects AABB
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RAYAABB_EPSILON 0.00001f

bool intersectRayAABB( const double MinB[3], const double MaxB[3], const double origin[3], const double dir[3], double coord[3] )
{
    bool Inside = true;
    double MaxT[3];
    MaxT[0] = MaxT[1] = MaxT[2] = -1.0f;

    // Find candidate planes.
    for ( int i = 0; i < 3; i++ )
    {
        if ( origin[i] < MinB[i] )
        {
            coord[i] = MinB[i];
            Inside = false;

            // Calculate T distances to candidate planes
            if ( dir[i] != 0.0 )
            {
                MaxT[i] = ( MinB[i] - origin[i] ) / dir[i];
            }

        }
        else if ( origin[i] > MaxB[i] )
        {
            coord[i] = MaxB[i];
            Inside = false;

            // Calculate T distances to candidate planes
            if ( dir[i] != 0.0 )
            {
                MaxT[i] = ( MaxB[i] - origin[i] ) / dir[i];
            }

        }

    }

    // Ray origin inside bounding box
    if ( Inside )
    {
        coord[0] = origin[0];
        coord[1] = origin[1];
        coord[2] = origin[2];
        return true;

    }

    // Get largest of the maxT's for final choice of intersection
    int WhichPlane = 0;
    if ( MaxT[1] > MaxT[WhichPlane] )
    {
        WhichPlane = 1;
    }
    if ( MaxT[2] > MaxT[WhichPlane] )
    {
        WhichPlane = 2;
    }

    // Check final candidate actually inside box
    if ( MaxT[WhichPlane] < 0.0 )
    {
        return false;
    }

    for ( int i = 0; i < 3; i++ )
    {
        if ( i != WhichPlane )
        {
            coord[i] = origin[i] + MaxT[WhichPlane] * dir[i];

#ifdef RAYAABB_EPSILON
            if ( coord[i] < MinB[i] - RAYAABB_EPSILON || coord[i] > MaxB[i] + RAYAABB_EPSILON )
            {
                return false;
            }
#else
            if( coord[i] < MinB[i] || coord[i] > MaxB[i] )
            {
                return false;
            }
#endif
        }
    }

    return true;
    // ray hits box
}
