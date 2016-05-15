//
// Standalone version of the Eminton-Lord procedure for obtaining the
// zero-lift wavedrag D/Q written by Sriram Rallabhandi
//
// Modifications for use in OpenVSP v3 by Michael Waddington
//
//////////////////////////////////////////////////////////////////////

#include "wavedragEL.h"

/*
This is simple rewrite of the procedure from AWAVE - Basically inverts
the matrix "a" and solves the linear system a*x = rhs

Better routines may be written to do this.
*/

void matinv(double ***a, int prts,double *rhs,double **soln) {
    double swap,amax,tmax,**iwk,pivot,sum,t;
    int *ipivot,j,k,l,i,jrow,jcolum,icolum,irow;

    ipivot = (int *) calloc(prts,sizeof(int));
    iwk = (double **) calloc(prts+1,sizeof(double *));
    for (i=0;i<(prts+1);i++) {
        iwk[i] = (double *) calloc(2,sizeof(double));
    }

    for (i=0;i<prts;i++) {
        amax=0.0;
        for (j=0;j<prts;j++) {
            if (ipivot[j]!=1) {
                for (k=0;k<prts;k++) {
                    if (ipivot[k]<1) {
                        tmax = fabs((*a)[j][k]);
                        if (amax<tmax) {
                            irow = j;
                            icolum = k;
                            amax = tmax;
                        }
                    }
                    else if (ipivot[k]>1) {
                        return;
                    }
                }
            }
        }
        if (amax<=0)
            return;
        else
            ipivot[icolum] = 1;

        if (irow!=icolum) {
            for (l=0;l<prts;l++) {
                swap = (*a)[irow][l];
                (*a)[irow][l] = (*a)[icolum][l];
                (*a)[icolum][l] = swap;
            }
        }

        iwk[i][0] = irow;
        iwk[i][1] = icolum;
        pivot = (*a)[icolum][icolum];

        (*a)[icolum][icolum]=1.0;
        for (l=0;l<prts;l++) {
            (*a)[icolum][l] = (*a)[icolum][l]/pivot;
        }

        for (l=0;l<prts;l++) {
            if (l!=icolum) {
                t = (*a)[l][icolum];
                (*a)[l][icolum]=0.0;
                for (k=0;k<prts;k++) {
                    (*a)[l][k] = (*a)[l][k]-(*a)[icolum][k]*t;
                }
            }
        }
    }

    for (i=0;i<prts;i++) {
        l = prts-1-i;
        if (iwk[l][0]!=iwk[l][1]) {
            jrow = iwk[l][0];
            jcolum = iwk[l][1];
            for (k=0;k<prts;k++) {
                swap = (*a)[k][jrow];
                (*a)[k][jrow] = (*a)[k][jcolum];
                (*a)[k][jcolum] = swap;
            }
        }
    }

    for (i=0;i<prts;i++) {
        sum=0.0;
        for (j=0;j<prts;j++) {
            sum = sum + (*a)[i][j]*rhs[j];
        }
        (*soln)[i] = sum;
    }

    free(ipivot);
    for (i=0;i<(prts+1);i++) {
        free(iwk[i]);
    }
    free(iwk);

}

/*
This routine uses a clever way of approximating the equivalent area
distribution using a Fourier bases. The wave drag values using the
prescribed expressions.

Ref :  Eminton, Lord, " Numerical evaluation of the wave drag of smooth
slender bodies using optimum area distributions for minimum wave drag",
Journal of the Royal Aeronautical Society, Vol. 60, pp 61-63, January
1956

*/


double emlord( int prts, double *naxs, double *sarea, double *r ){
    int i,j=0,k=0;
    double nx, ny, e, e1, e2, totlength, sum=0.0, drag;
    double *x, *c, *qq, **pqq;
    double pival=4.0*atan(1.0);

    x = (double *) calloc(prts,sizeof(double));
    qq = (double *) calloc(prts,sizeof(double));
    pqq = (double **) calloc(prts,sizeof(double*));
    for (i=0;i<prts;i++) {
        pqq[i] = (double *) calloc(prts,sizeof(double));
    }
    c = (double *) calloc(prts,sizeof(double));

    for (i=0;i<prts;i++) {
        x[k] = naxs[i];
        if (k==prts-3)
            break;
        k++;
        if (i==0 || i==(prts-1))
            k--;
    }

    drag = 0.0;
    for (i=0;i<(prts-2);i++) {
        nx = (x[i]-naxs[0])/(naxs[prts-1]-naxs[0]);
        qq[i] = (acos(1.0-2.0*nx)-(2.0-4.0*nx)*sqrt(nx-(nx*nx)))/pival;
        for (j=i;j<prts;j++) {
            ny = (x[j]-naxs[0])/(naxs[prts-1]-naxs[0]);
            e=(nx-ny)*(nx-ny);
            e1=nx+ny-2.0*nx*ny;
            e2=2.0*sqrt(nx*ny*(1.0-nx)*(1.0-ny));

            if (i==j) {
                pqq[j][i] = e1*e2;
            }
            else
                pqq[j][i] = 0.5*e*log((e1-e2)/(e1+e2))+e1*e2;

        }
        if (i>0) {
            for (k=0;k<i;k++) {
                e = pqq[i][k];
                pqq[k][i] = e;
            }
        }
    }

    k=0;
    for (i=0;i<prts;i++) {
        c[k] = sarea[i]-sarea[0]-(sarea[prts-1]-sarea[0])*qq[k];
        if (k==prts-3)
            break;
        k++;
        if ((i==0) || (i==(prts-1)))
            k--;
    }

    matinv(&pqq,prts-2,c,&r);

    sum=0.0;
    for (j=0;j<prts-2;j++) {
        sum += r[j]*c[j];
    }

    totlength = naxs[prts-1]-naxs[0];
    drag = (4.0*(sarea[0]-sarea[prts-1])*(sarea[0]-sarea[prts-1])/pival+sum*pival)/(totlength*totlength);

    free(x);
    free(qq);
    for (i=0;i<prts;i++)
    {
        free(pqq[i]);
    }
    free(pqq);
    free(c);

    return(drag);

}

