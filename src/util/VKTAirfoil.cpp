
#include "VKTAirfoil.h"

#include <complex>

#include "Defines.h"

typedef std::complex< double > doublec;

doublec cmplx_potential( doublec zeta, double alpha, double beta, double a, doublec mu )
{
    doublec i(0,1);

    doublec cmplx_potentialresult = zeta * exp( -i * alpha ) +
                                    i * 2.0 * a * sin( alpha + beta ) * log( zeta - mu ) +
                                    a * a * exp( i * alpha ) / ( zeta - mu );
    return cmplx_potentialresult;
}

doublec cmplx_velocity( doublec zeta, double alpha, double beta, double a, doublec mu )
{
    doublec i(0,1);

    doublec cmplx_velocityresult = exp( -i * alpha ) +
                                   i * 2.0 * a * sin( alpha + beta ) / ( zeta - mu ) -
                                   a * a * exp( i * alpha ) / ( ( zeta - mu ) * ( zeta - mu ) );
    return cmplx_velocityresult;
}

doublec derivative( doublec zeta, double ell, double n )
{
    doublec tmp = ( pow( zeta + ell, n ) - pow( zeta - ell, n ) );
    doublec derivativeresult = 4.0 * ( n * ell ) * ( n * ell ) * pow( ( zeta + ell ) * ( zeta - ell ) , ( n - 1.0 ) ) /
                               ( tmp * tmp );
    return derivativeresult;
}


vec3d vkt_airfoil_point( double theta, double epsilon, double kappa, double tau )
{
    vec3d p;

    doublec i(0,1);

    double ell = 0.25;

    double a = ell * sqrt( ( 1.0 + epsilon ) * ( 1.0 + epsilon ) + kappa * kappa );

    double beta = asin( ell * kappa / a );

    double n = 2.0 - tau / PI;

    doublec mu = doublec( -ell * epsilon,  ell * kappa );


    double xi = a * cos( theta - beta ) + mu.real();
    double eta = a * sin( theta - beta ) + mu.imag();
    doublec zeta = doublec( xi, eta );

    doublec temp = pow( zeta - ell, n ) / pow( zeta + ell, n );

    doublec z = n * ell * ( 1.0 + temp ) / ( 1.0 - temp );

    p.set_xyz( z.real(), z.imag(), 0.0 );

    return p;
}

double vkt_te_distance( double theta, double epsilon, double kappa, double tau )
{
    vec3d p0, p1;
    p0 = vkt_airfoil_point( 0.0, epsilon, kappa, tau );
    p1 = vkt_airfoil_point( theta, epsilon, kappa, tau );

    return dist( p0, p1 );
}
