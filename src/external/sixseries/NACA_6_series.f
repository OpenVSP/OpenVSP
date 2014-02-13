
C+-----------------------------------------------------------------------
C
C      PROGRAM SIXSERIES

      SUBROUTINE SIXSERIES ( TSER, TTOC, TCLI, TA )
C
C    (formerly PROGRAM LADSON)
C
C  PURPOSE: To produce the ordinates for airfoils of any thickness,
C           thickness distribution, or camber in the NACA 6- and 6A-series.
C
C  NAMELIST "INPUTS":
C
C   VAR     DIM   TYPE    DESCRIPTION
C  NAME      -    C*80    Title desired on printed and plotted output 
C  SERIET    -     I      NACA airfoil family (series 63 thru 67 and
C                         63A thru 65A)
C  SERIEC    -     I      Camber line (series 63 thru 66 and 63A thru 65A)
C  TOC       -     R      Thickness-chord ratio of airfoil
C  RLE       -     R      Leading-edge radius may be entered if desired
C                         (not used in program)
C  CMBNMR    -     R      Number of mean lines to be summed (>=1.)
C  CHD       -     R      Model chord used for listing ordinates in dimens-
C                         ional units
C  CLI       10    R      Design lift coefficient; set to 0.0 for a symmetrical
C                         airfoil. (Additional coefficients for up to nine
C                         mean lines may be added.)
C   A        10    R      Mean line chordwise loading (use 0.8 for 6A-series)
C
C  ENVIRONMENT:  DEC VAX/VMS FORTRAN (this version)
C
C  REFERENCE:    NASA Technical Memorandum TM X-3069 (September, 1974) 
C                Charles Ladson and Cuyler Brooks
C                Langley Research Center
C  HISTORY:
C    November 1989   Liam Hardy/NASA Ames   Interpolation routine FTLUP replaced
C                                           by LCSFIT to eliminate non-standard
C                                           %LOC usage; finite difference
C                                           derivatives (FUNCTION DIF) replaced
C                                           with FDI2K.
C-------------------------------------------------------------------------------


      CHARACTER
     >   METHOD*1, NAME*80, SERIEC*3, SERIET*3

      INTEGER
     >   I, KON, MXCOMB, NEVAL, NX, TSER

      LOGICAL
     >   NEW

      PARAMETER
     >   (LUNRD=5, LUNOUT=6, LUNSAV=10, MXCOMB=10, NX=200)

      REAL
     >   CHD, YUPR(NX), YLPR(NX),
     >   XU(NX), XL(NX), YU(NX), YL(NX),
     >   XAU(NX), YAU(NX), XAL(NX), YAL(NX),
     >   XT(NX+1), YT(NX+1), YTP(NX+1), YTPP(NX+1),
     >   PHI(NX+1), EPS(NX+1), PSI(NX+1),
     >   CLI(MXCOMB), A(MXCOMB), TANTH0(MXCOMB), YCMB(MXCOMB),
     >   TANTH(MXCOMB), YCP2(MXCOMB),
     >   AYP(NX+1), AYPP(NX+1), AX(NX+1), AXUC(NX+1), AYUC(NX+1),
     >   AYUP(NX+1), AXLC(NX+1), AYLC(NX+1), AYLP(NX+1), TTOC,
     >   TCLI, TA

      NAMELIST /INPUTS/ A, CHD, CLI, CMBNMR, NAME, SERIEC, SERIET, TOC

      E=0.1D-10
      PI=ATAN(1.) * 4.
      KON=0
      DX=0.01

C     Input parameters normalized by the chord (CHD)
C     TOC - T/C, Thickness, RLE - Leading edge radius, XM - X(YMAX)/CHORD
C     DX - Interval/Chord, CHD - Chord in desired units
C     CMBNMR - Number of mean lines to be summed (>=1.)

C  100 READ ( LUNRD, NML=INPUTS, END=999 )
  100 NAME='WHATEVER' 
      A(1) = TA
      CLI(1) = TCLI
      TOC = TTOC
      CRD = 1.0
      CMBNMR = 1.0

      SERIET = '63'
      SERIEC = '63'

      IF ( TSER.EQ.63 ) SERIET = '63'
      IF ( TSER.EQ.64 ) SERIET = '64'
      IF ( TSER.EQ.65 ) SERIET = '65'
      IF ( TSER.EQ.66 ) SERIET = '66'
      IF ( TSER.EQ.67 ) SERIET = '67'
      IF ( TSER.EQ.-63 ) SERIET = '63A'
      IF ( TSER.EQ.-64 ) SERIET = '64A'
      IF ( TSER.EQ.-65 ) SERIET = '65A'

      SERIEC = SERIET

      ICKY=CMBNMR
      IF(ICKY.LT.1) ICKY=1
      ICKYP=ICKY+1
      DO 110 J=ICKYP,10
         CLI(J)=0.0
         A(J)=0.0
  110 CONTINUE
      IF6XA=0
      KON=KON+1
      FRAC=1.0

C     Slope of camberline at origin, TANTH0:
      CLIS=CLI(1)
      AS=A(1)
      L=0
  200 CONTINUE
         L=L+1
         A(1)=A(L)
         CLI(1)=CLI(L)
         X=0.0
         Y=0.0
         XC=0.0
         YC=0.0
         XU(1)=0.0
         YU(1)=0.0
         XL(1)=0.0
         YL(1)=0.0
         XUC=0.0
         YUC=0.0
         XLC=0.0
         YLC=0.0
         XAU(1)=0.0
         YAU(1)=0.0
         XAL(1)=0.0
         YAL(1)=0.0
         K=2
         U=0.005
         V=-(A(1)-U)/ABS(A(1)-U)
         OMXL=(1.-U)*ALOG(1.-U)
         AMXL=(A(1)-U)*ALOG(ABS(A(1)-U))
         OMXL1=-ALOG(1.-U)-1.
         AMXL1=-ALOG(ABS(A(1)-U))+V
         OMXL2=1./(1.-U)
         AMXL2=-V/ABS(A(1)-U)
         IF(A(1).GE.E.AND.ABS(1.-A(1)).GE.E) THEN
            G=-(A(1)*A(1)*(.5*ALOG(A(1))-0.25)+0.25)/(1.-A(1))
            Q=1.0
            H=(0.5*(1.-A(1))**2*ALOG(1.-A(1))-0.25*(1.-A(1))**2)/
     >        (1.-A(1))+G
            Z=.5*(A(1)-U)*AMXL-.5*(1.-U)*OMXL-.25*(A(1)-U)**2+.25*
     >        (1.-U)**2
            Z1=.5*((A(1)-U)*AMXL1-AMXL-(1.-U)*OMXL1+OMXL+(A(1)-U)-
     >         (1.-U))
            Z2=.5*(A(1)-U)*AMXL2-AMXL1-.5*(1.-U)*OMXL2+OMXL1
         END IF
         IF(A(1).LT.E) THEN
            H=-.5
            Q=1.0
            Z1=U*ALOG(U)-.5*U-.5*(1.-U)*OMXL1+.5*OMXL-.5
         ELSE IF(ABS(A(1)-1.).LT.E) THEN
            H=0.0
            Q=0.0
            Z1=-OMXL1
         END IF
         TANTH0(L)=CLI(1)*(Z1/(1.-Q*A(1))-1.-ALOG(U)-H)/PI/
     >             (A(1)+1.)/2.0
         IF(L.LT.ICKY.AND.ICKY.GT.1)
     >GO TO 200

      IF(ICKY.NE.1) THEN
         DO 210 J=2,ICKY
            TANTH0(1)=TANTH0(1)+TANTH0(J)
  210    CONTINUE
      END IF

C     Slope of profile at origin, Upper and Lower:
      YP=10.E10
      YPP=10.E10
      IF(TANTH0(1).NE.0.) THEN
         YUP=-1./TANTH0(1)
         YLP=-1./TANTH0(1)
      ELSE
         YUP=0.
         YLP=0.
      END IF

C     First station aft of origin on uncambered profile:
      I=1
      X=.00025
C     Start loop for X increment:
  300 CONTINUE
C       Skip thickness computation after first pass:
      IF(I.GT.1) GO TO 550
C       Select series:
      IF (SERIET.EQ.'63')  CALL PHEP63  (PHI,EPS)
      IF (SERIET.EQ.'64')  CALL PHEP64  (PHI,EPS)
      IF (SERIET.EQ.'65')  CALL PHEP65  (PHI,EPS)
      IF (SERIET.EQ.'66')  CALL PHEP66  (PHI,EPS)
      IF (SERIET.EQ.'67')  CALL PHEP67  (PHI,EPS)
      IF (SERIET.EQ.'63')  CALL PHPS63  (PHI,PSI)
      IF (SERIET.EQ.'64')  CALL PHPS64  (PHI,PSI)
      IF (SERIET.EQ.'65')  CALL PHPS65  (PHI,PSI)
      IF (SERIET.EQ.'66')  CALL PHPS66  (PHI,PSI)
      IF (SERIET.EQ.'67')  CALL PHPS67  (PHI,PSI)
      IF (SERIET.EQ.'63A') CALL PHEP63A (PHI,EPS)
      IF (SERIET.EQ.'64A') CALL PHEP64A (PHI,EPS)
      IF (SERIET.EQ.'65A') CALL PHEP65A (PHI,EPS)
      IF (SERIET.EQ.'63A') CALL PHPS63A (PHI,PSI)
      IF (SERIET.EQ.'64A') CALL PHPS64A (PHI,PSI)
      IF (SERIET.EQ.'65A') CALL PHPS65A (PHI,PSI)
      RAT=1.0
      IT=0
      ACRAT=1.0

C       Loop start for thickness iteration:
  400 CONTINUE
         IT=IT+1
         ACRAT=ACRAT*RAT
         YMAX=0.0
         DO 410 J=1,NX
            XT(J)=-2.0*COSH(PSI(J)*ACRAT)*COS(PHI(J)-EPS(J)*ACRAT)
            YT(J)= 2.0*SINH(PSI(J)*ACRAT)*SIN(PHI(J)-EPS(J)*ACRAT)
            IF(YT(J).GT.YMAX) XYM=XT(J)
            IF(YT(J).GT.YMAX) YMAX=YT(J)
  410    CONTINUE

C        Estimate first and second derivatives by finite differencing:

         CALL FD12K (NX, XT, YT, YTP, YTPP, YTPP)

C        Estimate location of maximum thickness:

         XTP=1.0
         DO 420 J=3, NX
            IF (YTP(J).LT.0.0.AND.YTP(J-1).GE.0.0)
     >         XTP=XT(J-1)+YTP(J-1)*(XT(J)-XT(J-1))/(YTP(J-1)-YTP(J))
  420    CONTINUE

         CALL LCSFIT (NX+1, XT, YT, .TRUE., 'B', 1, XTP, YM, YM)
         XO=XT(1)
         XL(1)=XT(NX)
         TR=2.*YM/(XL(1)-XO)
         RAT=TOC/TR
         SF=RAT
         IF(TOC.GT.E .AND. ABS(RAT-1.0).GT.0.0001 .AND. IT.LE.10)
     >GO TO 400

      IF(I.EQ.1) THEN
         DO 500 J=1,NX+1
            XT(J)=(XT(J)-XO)/(XL(1)-XO)
C           Scale linearly to exact thickness:
            YT  (J)=SF*YT  (J)/(XL(1)-XO)
            YTP (J)=SF*YTP(J)
            YTPP(J)=SF*YTPP(J)*(XL(1)-XO)
  500    CONTINUE
      END IF
      XTP=(XTP-XO)/(XL(1)-XO)
      YMAX=YMAX*SF/(XL(1)-XO)
      YM=YM*SF/(XL(1)-XO)
      XYM=(XYM-XO)/(XL(1)-XO)
      XL(1)=0.0
      IF(TOC.GT.E) THEN
C        Fit tilted ellipse at eleventh profile point:
         CN=2.*YTP(11)-YT(11)/XT(11)+0.1
         AN=XT(11)*(YTP(11)*XT(11)-YT(11))/(XT(11)*
     >      (2.*YTP(11)-CN)-YT(11))
         BN=SQRT((YT(11)-CN*XT(11))**2/(1.-(XT(11)-AN)**2/AN**2))
         DO 510 J=1,10
            YT(J)=BN*SQRT(1.-(XT(J)-AN)**2/AN**2)+CN*XT(J)
            IF(XT(J).LE.E) GO TO 510
            YTP(J)=BN**2*(AN-XT(J))/AN**2/(YT(J)-CN*XT(J))+CN
            YTPP(J)=-BN**4/AN**2/(YT(J)-CN*XT(J))**3
  510    CONTINUE
         RNP=BN**2/AN
      END IF
      X=0.0
      ALI=ABS(CLI(1))

      X=0.00025
      XL(1)=0.0
  550 CONTINUE
      YUPR(I) = YUP
      YLPR(I) = YLP

C     Interpolate for thickness and derivatives at desired values of X:
      NEW = .TRUE.
      METHOD = 'B'
      NEVAL = 1.
      CALL LCSFIT (NX, XT, YT, NEW, METHOD, NEVAL, X, Y, Y) 
      CALL LCSFIT (NX, XT, YTP, NEW, METHOD, NEVAL, X, YP, YP)
      CALL LCSFIT (NX, XT, YTPP, NEW, METHOD, NEVAL, X, YPP, YPP)

C     Compute camberline:
      A(1)=AS
      CLI(1)=CLIS
      L=0
  600 CONTINUE
         L=L+1
         A(1)=A(L)
         CLI(1)=CLI(L)
         XC=X*CHD
         YC=Y*CHD
         XLL=X*ALOG(X)
         Q=1.0
         IF(ABS(1.-A(1)).LT.E.AND.ABS(1.-X).LT.E) THEN
            G=0.0
            H=0.0
            Q=0.0
            Z=0.0
            Z1=-10.E10
            Z2=-10.E10
         ELSE IF(A(1).LT.E.AND.(1.-X).LT.E) THEN
            G=-.25
            H=-.5
            Q=1.0
            Z=-.25
            Z1=0.0
            Z2=-10.E10
         ELSE IF(ABS(A(1)-X).LT.E) THEN
            Z=-.5*(1.-X)**2*ALOG(1.-X)+0.25*(1.-X)**2
            Z1=-.5*(1.-X)*(-ALOG(1.-X)-1.)+.5*(1.-X)*ALOG(1.-X)-.5*
     >         (1.-X)
            Z2=-ALOG(1.-X)-0.5
            G=-(A(1)**2*(.5*ALOG(A(1))-0.25)+0.25)/(1.-A(1))
            H=(0.5*(1.-A(1))**2*ALOG(1.-A(1))-0.25*(1.-A(1))**2)/
     >        (1.-A(1))+G
         ELSE IF(ABS(1.-X).LT.E) THEN
            G=-(A(1)**2*(.5*ALOG(A(1))-0.25)+0.25)/(1.-A(1))
            H=(0.5*(1.-A(1))**2*ALOG(1.-A(1))-0.25*(1.-A(1))**2)/
     >        (1.-A(1))+G
            Z=.5*(A(1)-1.)**2*ALOG(ABS(A(1)-1.))-0.25*(A(1)-1.)**2
            Z1=-(A(1)-1.)* ALOG(ABS(A(1)-1.))
            Z2=-10.E10
         ELSE IF(ABS(A(1)-1.).LT.E) THEN
            G=0.0
            H=0.0
            Q=0.0
            Z=-(1.-X)*ALOG(1.-X)
            Z1=ALOG(1.-X)+1.
            Z2=-1./(1.-X)
         ELSE
            V=-(A(1)-X)/ABS(A(1)-X)
            OMXL=(1.-X)*ALOG(1.-X)
            AMXL=(A(1)-X)*ALOG(ABS(A(1)-X))
            OMXL1=-ALOG(1.-X)-1.
            AMXL1=-ALOG(ABS(A(1)-X))-1.
            OMXL2=1./(1.-X)
            AMXL2=1./(A(1)-X)
            Z=.5*(A(1)-X)*AMXL-.5*(1.-X)*OMXL-.25*(A(1)-X)**2+.25*(1.-X)
     >        **2
            Z1=.5*((A(1)-X)*AMXL1-AMXL-(1.-X)*OMXL1+OMXL+(A(1)-X)-
     >         (1.-X))
            Z2=.5*(A(1)-X)*AMXL2-AMXL1-.5*(1.-X)*OMXL2+OMXL1
            IF(A(1).LE.E) THEN
               G=-.25
               H=-.5
            ELSE
               G=-(A(1)*A(1)*(.5*ALOG(A(1))-0.25)+0.25)/(1.-A(1))
               H=(0.5*(1.-A(1))**2*ALOG(1.-A(1))-0.25*(1.-A(1))**2)/
     >           (1.-A(1))+G
            END IF
         END IF

         YCMB(L)=CLI(1)*(Z/(1.-Q*A(1))-XLL+G-H*X)/PI/(A(1)+1.)/2.
         XSV=X
         IF(X.LT.0.005) X=0.005
         TANTH(L)=CLI(1)*(Z1/
     >   (1.-Q*A(1))-1.-ALOG(X)-H)/PI/(A(1)+1.)/2.0
         X=XSV
         IF(IF6XA.EQ.1) TANTH(L)=-2.0
         IF(X.LE.0.005) THEN
               YCP2(L)=0.0
            ELSE IF (ABS(1.-X).LE.E) THEN
                  YCP2(L)=1./E
               ELSE
                  PIA=PI*(A(1)+1.)*2
                  YCP2(L)=CLI(1)*(Z2/(1.-Q*A(1))-1./X)/PIA
         END IF  
C        Modified camberline option:
         IF(SERIEC.EQ.'63A' .OR. SERIEC.EQ.'64A' .OR. SERIEC.EQ.'65A')
     >      THEN
            YCMB(L)=YCMB(L)*0.97948
            TANTH(L)=TANTH(L)*0.97948
            IF(TANTH(L).LE.-.24521*CLI(1)) YCMB(L)=0.24521*CLI(1)*
     >      (1.-X)
            IF(TANTH(L).LE.-.24521*CLI(1)) YCP2(L)=0.0
            IF(TANTH(L).LE.-.24521*CLI(1)) TANTH(L)=-0.24521*CLI(1)
            IF(TANTH(L).LE.-.24521*CLI(1)) IF6XA=1
         END IF
         IF(ICKY.GT.1.AND.L.LT.ICKY)
     > GO TO 600

      IF(ICKY.EQ.1) GO TO 620
      DO 610 J=2,ICKY
         YCMB (1)=YCMB (1)+YCMB (J)
         TANTH(1)=TANTH(1)+TANTH(J)
         YCP2 (1)=YCP2 (1)+YCP2 (J)
  610 CONTINUE
  620 CONTINUE
      F=SQRT(1.+TANTH(1)**2)
      THP=YCP2(1)/F**2
      SINTH=TANTH(1)/F
      COSTH=1./F
C     Camberline and derivatives computed:
      I=I+1
C     Combine thickness distributuion and camberline:
      XU(I)=X-Y*SINTH
      YU(I)=YCMB(1)+Y*COSTH
      XL(I)=X+Y*SINTH
      YL(I)=YCMB(1)-Y*COSTH
      IF (X.GE. .815) THEN
         IF(SERIET.EQ.'63A' .OR. SERIET.EQ.'64A' .OR. SERIET.EQ.'65A') 
     >   THEN
            IF (X.LE. .825) THEN
               X2 = 1.0
               X1 = XU(I)
               Y2 = 0.
               Y1 = YU(I)
               S1 = (Y2 - Y1) / (X2 - X1)
               S2 = (Y2 - YL(I)) / (X2 - XL(I))
               B1 = Y2 - S1 * X2
               B2 = Y2 - S2 * X2
            END IF
         YU(I) = S1 * XU(I) + B1
         YL(I) = S2 * XL(I) + B2
         END IF
      END IF
C       Multiply by chord:
      XUC=XU(I)*CHD
      YUC=YU(I)*CHD
      XLC=XL(I)*CHD
      YLC=YL(I) *CHD
      IF(ALI.GT.E.OR.ICKY.NE.1) THEN
C        Find local slope of cambered profile:
         YUP=(TANTH(1)*F+YP-TANTH(1)*Y*THP)/(F-YP*TANTH(1)-Y*THP)
         YLP=(TANTH(1)*F-YP+TANTH(1)*Y*THP)/(F+YP*TANTH(1)+Y*THP)
      END IF

C     Find X increment:
      IF(X.LE.0.0975) FRAC=0.25
      IF(X.LE.0.01225) FRAC=0.025
C     Store profile in appropriate arrays:
      AXUC(I) = XUC
      AYUC(I) = YUC
      IF (ALI.GE.E .OR. ICKY.NE.1) THEN
         AX(I) = X
         AXLC(I) = XLC
         AYLC(I) = YLC
      END IF

C     Increment X and return to start of X loop:
      X=X+FRAC*DX
      FRAC=1.0
      XAU(I) = XUC
      YAU(I) = YUC
      XAL(I) = XLC
      YAL(I) = YLC
      IF(X.LE.1.0) GO TO 300

      CALL LDDATA( NX, NAME, I, I, XU, XL, YU, YL )

C     Return to read for next case:
C      GO TO 100

C  999 STOP ' '
  999 RETURN


      END
C+----------------------------------------------------------------------
C
      FUNCTION BESSEL (J, H, DEL)
C
C     One-liner: First derivative using central 3-point formula
C     ----------
C
C     Description and usage:
C     ----------------------
C
C        Computes a first derivative approximation using the central
C     3-point formula.  The data must be in the form of arrays containing
C     finite difference interval lengths and 2-point forward difference
C     derivatives.  BESSEL is intended to be used by PLSFIT for determin-
C     ing end conditions on an interval for (non-monotonic) interpolation
C     by piecewise cubics.  See the PLSFIT header for more details.
C
C     Arguments:
C     ----------
C
C     Name    Type/Dimension  I/O/S  Description
C     J       I               I      Indicates at which end of the
C                                    interval the derivative is to be
C                                    estimated. J = 0 means left-hand
C                                    side, J = 1 means right.
C
C     H       R (-1:1)        I      Array of interval lengths. The 0th
C                                    element is the length of the interval
C                                    on which the cubic is to be deter-
C                                    mined.
C
C     DEL     R (-1:1)        I      Array of derivative estimates. The
C                                    0th element is the forward difference
C                                    derivative over the interval on which
C                                    the cubic is to be determined.
C                                     
C     BESSEL  R                 O    The function value is the adjusted
C                                    derivative.
C
C     Notes:
C     ------
C
C     (1)  IMPLICIT NONE is non-standard.
C
C     Author:  Robert Kennelly, Sterling Federal Systems/NASA-Ames
C     -------
C
C     Development history:
C     --------------------
C
C     18 Feb. 1987    RAK    Initial design and coding.
C
C-----------------------------------------------------------------------

C     Declarations.
C     -------------

      IMPLICIT NONE

C     Constants.

      REAL
     &   ONE
      PARAMETER
     &  (ONE = 1.0E+0)

C     Arguments.

      INTEGER
     &   J
      REAL
     &   H (-1:1), DEL (-1:1), BESSEL

C     Local variables.

      REAL
     &   WEIGHT

C     Execution.
C     ----------

C     Estimate first derivative on left (J = 0) or right side (J = 1) of
C     an interval.

      WEIGHT = H (J) / (H (J) + H (J - 1))
      BESSEL = WEIGHT * DEL (J - 1) + (ONE - WEIGHT) * DEL (J)

C     Termination.
C     ------------

      RETURN
      END
C+----------------------------------------------------------------------
C
      FUNCTION BRODLIE (J, H, DEL)
C
C     One-liner: First derivative, adjusted for monotonicity
C     ----------
C
C     Description and usage:
C     ----------------------
C
C        BRODLIE is intended to be used by PLSFIT for determining end
C     conditions on an interval for monotonic interpolation by piecewise
C     cubics. The data must be in the form of arrays containing finite
C     difference interval lengths and 2-point forward difference deriva-
C     tives. See the PLSFIT header for more details.
C
C        The method is due to Brodlie, Butland, Carlson, and Fritsch,
C     as referenced in the PLSFIT header.
C
C     Arguments:
C     ----------
C
C     Name    Type/Dimension  I/O/S  Description
C     J       I               I      Indicates at which end of the
C                                    interval the derivative is to be
C                                    estimated. J = 0 means left-hand
C                                    side, J = 1 means right.
C
C     H       R (-1:1)        I      Array of interval lengths. The 0th
C                                    element is the length of the interval
C                                    on which the cubic is to be deter-
C                                    mined.
C
C     DEL     R (-1:1)        I      Array of derivative estimates. The
C                                    0th element is the forward difference
C                                    derivative over the interval on which
C                                    the cubic is to be determined.
C                                     
C     BRODLIE R                 O    The function value is the adjusted
C                                    derivative.
C
C     Notes:
C     ------
C
C     (1)  IMPLICIT NONE and 8-character symbolic names are non-standard.
C
C     Author:  Robert Kennelly, Sterling Federal Systems/NASA-Ames
C     -------
C
C     Development history:
C     --------------------
C
C     18 Feb. 1987    RAK    Initial design and coding.
C
C-----------------------------------------------------------------------

C     Declarations.
C     -------------

      IMPLICIT NONE

C     Constants.

      REAL
     &   ZERO, ONE, THIRD
      PARAMETER
     &  (ZERO   = 0.0E+0,
     &   ONE    = 1.0E+0,
     &   THIRD  = ONE / 3.0E+0)

C     Arguments.

      INTEGER
     &   J
      REAL
     &   BRODLIE, H (-1:1), DEL (-1:1)

C     Local variables.

      REAL
     &   ALPHA

C     Execution.
C     ----------

C     Compare the algebraic signs of the two DEL's.  Have to test that
C     at least one is positive to avoid a zero denominator (this fancy
C     test permits one term to be zero, but the answer below is zero
C     anyway in these cases).  The trick is to work around the SIGN
C     function, which returns positive even if its 2nd argument is zero.

      IF (SIGN (ONE, -DEL (J - 1)) .NE. SIGN (ONE, DEL (J))) THEN

C        Form "weighted harmonic mean" of the two finite-difference
C        derivative approximations.  Note that we try to avoid overflow
C        by not multiplying them together directly.

         ALPHA   = THIRD * (ONE + H (J) / (H (J - 1) + H (J)))
         BRODLIE = DEL (J - 1) * (DEL (J) / (ALPHA * DEL (J) +
     &      (ONE - ALPHA) * DEL (J - 1)))
      ELSE

C        The signs differ, so make this point a local extremum.

         BRODLIE = ZERO
      END IF

C     Termination.
C     ------------

      RETURN
      END
C+----------------------------------------------------------------------
C
      FUNCTION BUTLAND (J, H, DEL)
C
C     One-liner: First derivative, non-central 3-point formula, adjusted
C     ----------
C
C     Description and usage:
C     ----------------------
C
C        Computes a first derivative approximation for PLSFIT over an
C     interval at a data boundary, using a modified forward or backward
C     3-point formula.  The data must be in the form of arrays containing
C     finite difference interval lengths and 2-point forward difference
C     derivatives, and the differencing direction is controlled by a flag.
C     See PLSFIT for more details, or THREEPT for the pure 3-pt. formula.
C
C        The "shape preserving adjustments" are from PCHIP, a monotone
C     piecewise cubic interpolation package by F. N. Fritsch.
C
C     Arguments:
C     ----------
C
C     Name    Type/Dimension  I/O/S  Description
C     J       I               I      Indicates at which end of the
C                                    interval the derivative is to be
C                                    estimated. J = 0 means left-hand
C                                    side, J = 1 means right. 
C
C     H       R (-1:1)        I      Array of interval lengths. The 0th
C                                    element is the length of the interval
C                                    on which the cubic is to be deter-
C                                    mined.
C
C     DEL     R (-1:1)        I      Array of derivative estimates. The
C                                    0th element is the forward difference
C                                    derivative over the interval on which
C                                    the cubic is to be determined.
C                                     
C     BUTLAND R                 O    The function value is the adjusted
C                                    derivative.
C
C     Environment:  VAX/VMS; FORTRAN 77
C     ------------
C
C     IMPLICIT NONE and 8-character symbolic names are non-standard.
C
C     Author:  Robert Kennelly, Sterling Federal Systems/NASA-Ames
C     -------
C
C     History:
C     --------
C
C     18 Feb. 1987    RAK    Initial design and coding, as THREEPT.
C     20 June 1991    DAS    Monotonic form renamed BUTLAND; THREEPT
C                            is now the pure 3-point formula.
C
C-----------------------------------------------------------------------

C     Declarations.
C     -------------

      IMPLICIT NONE

C     Arguments.

      INTEGER
     &   J
      REAL
     &   H (-1:1), DEL (-1:1), BUTLAND

C     Local constants.

      REAL
     &   ZERO, ONE, THREE
      PARAMETER
     &  (ZERO  = 0.0E+0,
     &   ONE   = 1.0E+0,
     &   THREE = 3.0E+0)

C     Local variables.

      INTEGER
     &   STEP
      REAL
     &   DMAX, WEIGHT

C     Execution.
C     ----------

C     Estimate first derivative on a left-hand boundary using a 3-point
C     forward difference (STEP = +1), or with a backward difference for
C     the right-hand boundary (STEP = -1).

      STEP = 1 - J - J   ! J here is consistent with related modules.

C     In {H, DEL} form, the derivative looks like a weighted average.

      WEIGHT  = -H (0) / (H (0) + H (STEP))
      BUTLAND = WEIGHT * DEL (STEP) + (ONE - WEIGHT) * DEL (0)

C     Shape-preserving adjustments.  Note that we try to avoid overflow
C     by not multiplying quantities directly.

      IF (SIGN (ONE, BUTLAND) .NE. SIGN (ONE, DEL (0)) .OR.
     &   DEL (0) .EQ. ZERO) THEN

C        Defer to the estimate closest to the boundary.

         BUTLAND = ZERO
      ELSE IF (SIGN (ONE, DEL (0)) .NE. SIGN (ONE, DEL (STEP))) THEN

C        If the monotonicity switches, may need to bound the estimate.

         DMAX = THREE * DEL (0)
         IF (ABS (BUTLAND) .GT. ABS (DMAX)) BUTLAND = DMAX
      END IF

C     Termination.
C     ------------

      RETURN
      END
C+----------------------------------------------------------------------
C
      SUBROUTINE FD12K (N, X, F, FP, FPP, FK)
C
C  PURPOSE: FD12K returns estimates of 1st and 2nd derivatives and of
C           curvature, by finite differencing, for each of the points
C           (X(I), F(I)), I = 1 : N.  The abscissas are assumed to be
C           nonuniform, and they must be monotonic.
C
C           This routine combines calls to FDCNTR, FD1SID, FDCURV for
C           the common case of needing results for N >= 2 points.
C
C           If (say) curvature is wanted at a single point only, call
C           either FDCNTR or FD1SID and FDCURV directly.
C
C  INPUTS:  X(*) & F(*) are N-vectors defining some curve in 2-space.
C           For N > 2, the 3-pt formulas are used for all I (with the
C                      one-sided forms used at the end-points).
C           For N = 2, the 2-pt formulas are used.
C
C  OUTPUTS: FP, FPP, FK are N-vectors representing 1st and 2nd deriv-
C           atives and curvature respectively.  These are assigned in
C           reverse order (FK, FPP, FP) so that a call such as
C
C                     CALL FD12K (N, X, Y, YP, YP, YP)
C
C           can be used if just 1st derivatives are desired, to avoid
C           declaring storage for FPP and FK. (Similarly for the case
C           when 1st and 2nd derivatives are desired but curvature is
C           not. The unnecessary arithmetic in these cases is consid-
C           ered preferable to another argument and extra logic.)
C
C  METHOD:  Central differencing is used at all interior points, with
C           one-sided 3-point formulas used at each end-point.
C
C           The curvature formula is safeguarded against overflow  in
C           the presence of large 1st derivatives.  The basic formula
C           used here is:
C
C               FK (I) = FPP (I) / (1 + FP(I) ** 2) ** 3/2
C
C           Note that if X is not necessarily monotonic, curvature is
C           defined as
C
C               CURVATURE = (X" ** 2  +  Y" ** 2) ** 1/2
C
C           where " means 2nd derivative with respect to  arc-length.
C           See modules CURV2D and CURV3D for these parametric cases.
C
C  NOTES:   1. Finite differencing errors can be large if the delta-X
C              values are too small,  especially if the precision  in
C              the function values is less than full.
C           2. Nevertheless, finite differences have been observed to
C              behave better than the analytic derivatives of splines
C              in airfoil geometry applications.
C
C  EXTERNALS:
C           FDCNTR modularizes the central 3-point formulas for first
C                  and second derivatives by finite differences.
C           FDCURV modularizes the curvature formula (safe-guarded).
C           FD1SID modularizes the 1-sided forward and backward 3-pt.
C                  formulas for first and second derivatives.
C
C  HISTORY:
C           09/15/83   DAS   Initial implementation (interior pts only).
C           12/27/85   DAS   End points are handled by FD1SID now.
C           09/18/87   DAS   The N=2 case is handled now.
C           08/21/89   DAS   Formulation revised to use separate dF/dX
C                            terms instead of a common denominator.
C           08/17/91   DAS   Introduced FDCNTR and FDCURV when it was
C                            found that FD12K did not lend itself to
C                            application to one point at a time.
C
C  AUTHOR:  David Saunders, Sterling Software/NASA Ames, Palo Alto, CA.
C
C-----------------------------------------------------------------------

      IMPLICIT   NONE

C     Arguments:

      INTEGER    N
      REAL       X (N), F (N), FP (N), FPP (N), FK (N)

C     Local constants:

      REAL       ZERO
      PARAMETER (ZERO = 0.E+0)

C     Local variables:

      INTEGER    I
      REAL       FPI, FPPI

C     Procedures:

      EXTERNAL   FDCNTR, FDCURV, FD1SID

C     Execution:

C     Assign values in the order  curvature, f", f'  so that the
C     user can pass the same array for, say, curvature and f" if
C     curvature is not wanted:

      IF (N .EQ. 2) THEN

         FK (1)  = ZERO
         FPP (1) = ZERO
         FP (1)  = (F (2) - F (1)) / (X (2) - X (1))
         FK (2)  = ZERO
         FPP (2) = ZERO
         FP (2)  = FP (1)

      ELSE

C        Forward 3-pt. differencing for the first point:

         CALL FD1SID (1, 1, X, F, FPI, FPPI)
         CALL FDCURV (FPI, FPPI, FK (1))
         FPP (1) = FPPI
         FP (1)  = FPI
      
C        Central 3-pt. differencing for the bulk of the points:

         DO 20 I = 2, N-1
            CALL FDCNTR (I, X, F, FPI, FPPI)
            CALL FDCURV (FPI, FPPI, FK (I))
            FPP (I) = FPPI
            FP (I)  = FPI
   20    CONTINUE

C        Backward differencing for the last point:

         CALL FD1SID (N, -1, X, F, FPI, FPPI)
         CALL FDCURV (FPI, FPPI, FK (N))
         FPP (N) = FPPI
         FP (N)  = FPI
      END IF

      RETURN
      END
C+----------------------------------------------------------------------
C
      SUBROUTINE FD1SID (I, INC, X, F, FP, FPP)
C
C  PURPOSE: FD1SID returns one-sided 3-point finite-difference estimates
C           of the 1st and 2nd derivatives at the point  ( X(I), F(I) ).
C           If INC = 1, points I, I+1, I+2 are used,  while if INC = -1,
C           points  I-2, I-1, I are used. The abscissas need not be uni-
C           formly spaced.  
C
C  ARGS:    Obvious from PURPOSE.
C
C  METHOD:  FPP is computed first,  in case only FP is desired,  so that
C           the same item may be passed for both arguments. The formula-
C           tion is similar to that of central differencing - see FD12K.
C
C  HISTORY: 12/27/85  DAS  Initial implementation  (prompted by the need
C                          to approximate an airfoil leading edge with a
C                          cubic having specified slope at one end).
C           08/21/89  DAS  Formulation revised as for centrals (FD12K).
C
C  AUTHOR:  David Saunders, Sterling Software/NASA Ames, Palo Alto, CA.
C
C-----------------------------------------------------------------------

      IMPLICIT   NONE

C     Arguments:

      INTEGER    INC, I
      REAL       X (*), F (*), FP, FPP

C     Local constants:

      REAL       ONE
      PARAMETER (ONE = 1.E+0)

C     Local variables:

      INTEGER  I1, I2
      REAL     DEL1, DEL2, DIV, DX1, DX2, W

C     Execution:

C     The minus signs take care of themselves for the backward case.

      I1   = I  + INC
      I2   = I1 + INC
      DX1  =  X (I1) - X (I)
      DEL1 = (F (I1) - F (I)) / DX1
      DX2  =  X (I2) - X (I1)
      DEL2 = (F (I2) - F (I1)) / DX2
      DIV  = ONE / (DX1 + DX2)
      W    = -DX1 * DIV
      FPP  = (DEL2 - DEL1) * (DIV + DIV)
      FP   = W * DEL2 + (ONE - W) * DEL1

      RETURN
      END
C+----------------------------------------------------------------------
C
      SUBROUTINE FDCNTR (I, X, F, FP, FPP)
C
C  PURPOSE: FDCNTR returns central 3-point finite-difference estimates
C           of the 1st and 2nd derivatives at the point  (X(I), F(I)).
C           Use FD1SID for the end-point cases.
C
C  ARGS:    Obvious from PURPOSE.
C
C  METHOD:  FPP is computed first, in case only FP is desired, so that
C           the same item may be passed for both arguments.
C
C  HISTORY: 08/17/91  DAS  FDCNTR adapted from FD12K's in-line code,
C                          as for FD1SID, for the case of a single I
C                          at a time (which FD12K can't do).
C
C  AUTHOR:  David Saunders, Sterling Software/NASA Ames, Palo Alto, CA.
C
C-----------------------------------------------------------------------

      IMPLICIT   NONE

C     Arguments:

      INTEGER    I
      REAL       X (*), F (*), FP, FPP

C     Local constants:

      REAL       ONE
      PARAMETER (ONE = 1.E+0)

C     Local variables:

      REAL     DEL1, DEL2, DIV, DX1, DX2, W

C     Execution:

      DX1  =  X (I) - X (I-1)
      DEL1 = (F (I) - F (I-1)) / DX1
      DX2  =  X (I+1) - X (I)
      DEL2 = (F (I+1) - F (I)) / DX2
      DIV  = ONE / (DX1 + DX2)
      W    = DX2 * DIV
      FPP  = (DEL2 - DEL1) * (DIV + DIV)
      FP   = W * DEL1 + (ONE - W) * DEL2

      RETURN
      END
C+----------------------------------------------------------------------
C
      SUBROUTINE FDCURV (DYDX, D2YDX2, KAPPA)
C
C  PURPOSE: FDCURV (Finite-Difference CURVature estimate) returns a
C           safe-guarded value for curvature at one point on the
C           curve Y = Y (X) given the 1st and 2nd derivatives at
C           that point, using the formula
C
C               KAPPA = D2YDX2 / (1 + DYDX ** 2) ** 3/2
C
C           The sign of KAPPA is clearly that of the 2nd derivative.
C           The derivatives could well be obtained from a spline, but
C           experience shows finite differencing can be preferable.
C
C           See modules CURV2D and CURV3D for the parametric cases.
C
C  ARGUMENTS:  Obvious from the description.  KAPPA is REAL.
C
C  HISTORY: 08/17/91  Derived FDCURV from FD12K, along with FDCNTR
C                     when it was found that FD12K did not lend
C                     itself to application to one point at a time.
C
C  AUTHOR:  David Saunders, Sterling Software/NASA Ames, Palo Alto, CA.
C
C-----------------------------------------------------------------------

      IMPLICIT   NONE

C     Arguments:

      REAL       DYDX, D2YDX2, KAPPA

C     Local constants:

      REAL       DYDXMAX, ONE
      PARAMETER (DYDXMAX = 1.E+10, ONE = 1.E+0)

C     Local variables:

      REAL       TERM

C     Execution:

      TERM = ONE + (MIN (ABS (DYDX), DYDXMAX)) ** 2
      KAPPA = D2YDX2 / (TERM * SQRT (TERM))

      RETURN
      END
C+----------------------------------------------------------------------
C
      SUBROUTINE INTERVAL (NX, X, XFIND, ARROW, LEFT)
C
C     One-liner: Interpolation search for interval containing a point.
C     ----------
C
C     Description and usage:
C     ----------------------
C
C        Written primarily for interval-based interpolations such as
C     piecewise linear or cubic spline, INTERVAL performs a search to
C     locate the best interval for evaluating the interpolant at a
C     given point. The normal case returns the "left-hand" endpoint of
C     the interval bracketing the point, but for the out-of-range cases
C     below or above the range of the knots, the interval to be used is
C     the first or last. The array of knots must be monotonic, either
C     increasing or decreasing. Diagrammatically, LEFT is returned as
C     shown below for the normal case (no extrapolation):
C
C          X (1)  ...   X (LEFT)   X (LEFT+1)   ...      X (NX)
C                                ^
C                              XFIND
C
C     And for extrapolation:
C
C                     X (LEFT = 1)  ...   X (NX)
C             ^
C           XFIND
C
C     or,
C                X (1)  ...   X (LEFT = NX-1)    X (NX)
C                                                           ^
C                                                         XFIND
C
C     If the point to be bracketed (XFIND) matches one of the knots, the
C     index of that knot is returned as LEFT, i.e., the condition for a
C     bracket of an interior point is:
C
C        X (LEFT) <= XFIND < X (LEFT+1)  if  ARROW = +1.0,  or
C        X (LEFT) >= XFIND > X (LEFT+1)  if  ARROW = -1.0.
C
C        This is a low-level routine with minimal error checking. The
C     calling program is assumed to have verified the following:
C
C     (1)  NX >= 2
C     (2)  X strictly monotonic
C     (3)  ARROW = +1.0 or -1.0
C
C     Subroutine PROTECT is available from the author for easily checking
C     conditions (2) and (3). LEFT is verified on input, but efficiency in
C     loops will benefit from passing the best estimate available, usually
C     just the result of the last call.
C
C        INTERVAL was originally written for use with CSEVAL and TABLE1.
C     The interpolation search was adapted from ideas in Sedgewick's book
C     referenced below.
C
C     Arguments:
C     ----------
C
C     Name  Dimension  Type  I/O/S  Description
C     NX                I    I      Number of points in array X; must
C                                   be >= 2 (no check performed).
C
C     X        NX       R    I      Array of points defining the set
C                                   of intervals to be examined. Only
C                                   the first NX-1 points are required.
C
C     XFIND             R    I      The point for which a bracketing
C                                   interval is sought.
C
C     ARROW             R    I      Monotonicity indicator for input
C                                   array X:
C                                     -1.0  strictly decreasing
C                                      0.0  NOT ALLOWED!
C                                     +1.0  strictly increasing
C                                   Supplied by the calling routine for
C                                   reasons of speed (not checked).
C
C     LEFT              I    I/O    Input: guessed index of left-hand
C                                   endpoint of the interval containing
C                                   the specified point.
C
C                                   Output: index of the largest array
C                                   value <= specified point (if ARROW=+1.0).
C                                   Special case for data out of range:
C                                   return left endpoint of closest interval.
C                                   Thus, LEFT = 1 for XFIND < X (2), and
C                                   LEFT = NX-1 for XFIND >= X (NX-1).
C                                   (If ARROW=-1.0, reverse the inequalities.)
C
C     Environment:  Digital VAX-11/780, VMS FORTRAN
C     ------------  Apple Macintosh, Absoft MacFORTRAN/020 v2.3
C
C     Notes:
C     ------
C
C     (1)  IMPLICIT NONE and eight character symbols are not (yet) standard.
C
C     (2)  In speed-critical applications, it might be a good idea to build
C          this algorithm in-line since it is typically called many times
C          from within a loop. Another potential speed-up is removal of the
C          ARROW multiplies, which restricts the method to increasing data.
C          So far, the simplicity of separating out the messy search details
C          and the generality of bi-directional searching have outweighed
C          the modest speed penalty incurred.
C
C     Bibliography:
C     -------------
C
C     (1) Sedgewick, R.  Algorithms.  Reading: Addison-Wesley, 1983.
C            (Chap. 14)
C
C     Author:  Robert Kennelly and David Saunders, Sterling Federal Systems
C     -------
C
C     Development history:
C     --------------------
C
C     20 Oct. 1987    RAK    Interpolation search adapted (with mods.
C                            for bidirectional search and some minor
C                            repair) from CSEVAL (RAK) and TABLE1 (DAS).
C     08 Aug. 1988    DAS    Clarified descriptions of bracketing, where
C                            the inequalities depend upon ARROW.
C
C-----------------------------------------------------------------------

C     Declarations.
C     -------------

      IMPLICIT NONE

C     Constants.

      REAL
     &   ONE
      PARAMETER
     &  (ONE = 1.0E+0)

C     Arguments.

      INTEGER
     &   LEFT, NX
      REAL
     &   ARROW, X (NX), XFIND

C     Local variables.

      INTEGER
     &   LENGTH, NXLESS1, RIGHT, TRIAL
      REAL
     &   XBYARROW

C     Execution.
C     ----------

      XBYARROW = XFIND * ARROW

C     Simplify things by disposing of two important special cases so that
C     X (LEFT) and X (RIGHT) can really bracket XFIND. As a by-product,
C     this also takes care of the NX = 2, 3 cases.

      NXLESS1 = NX - 1

      IF (XBYARROW .GE. X (NXLESS1) * ARROW) THEN
         LEFT = NXLESS1
         GO TO 990
      ELSE IF (XBYARROW .LT. X (2) * ARROW) THEN
         LEFT = 1
         GO TO 990
      END IF

C      ---------------------------------
C     |                                 |
C     |   X (2) <= XFIND < X (NX - 1)   |
C     |            - or -               |
C     |   X (2) > XFIND >= X (NX - 1)   |
C     |                                 |
C     |   NX > 3                        |
C     |                                 |
C      ---------------------------------

C     Adjust the pointers. We hope that the calling routine has provided
C     a reasonable guess (since it's probably working on an ordered array
C     of points to evaluate), but check anyway.

      LEFT = MIN (MAX (2, LEFT), NX - 2)

      IF (XBYARROW .GE. X (LEFT) * ARROW) THEN
         IF (XBYARROW .LT. X (LEFT + 1) * ARROW) THEN

C           XFIND is in the original guessed-at interval.

            GO TO 990
         ELSE

C           We'll look farther to the right. Evidently LEFT was < NX - 2.

            RIGHT = NXLESS1
            LEFT  = LEFT + 1
         END IF
      ELSE

C        Look to the left of the guess. Evidently LEFT was > 2.

         RIGHT = LEFT
         LEFT  = 2
      END IF

C      ----------------------------------
C     |                                  |
C     |   2 <= LEFT < RIGHT <= NX - 1    |
C     |                                  |
C      ----------------------------------

C     The interval length must decrease each time through - terminate
C     when the correct interval is found or when the interval length
C     cannot be decreased.

   10 CONTINUE
         LENGTH = RIGHT - LEFT
         IF (LENGTH .GT. 1) THEN

C           The trial value is a "linear" estimate of the left-hand endpoint
C           of the interval bracketing the target XFIND, with protection
C           against round-off (which can affect convergence).

            TRIAL = MIN (RIGHT - 1, LEFT + MAX (0, INT (REAL (LENGTH) *
     &         (XFIND - X (LEFT)) / (X (RIGHT) - X (LEFT)))))

C            ------------------------------------------
C           |                                          |
C           |   2 <= LEFT <= TRIAL < RIGHT <= NX - 1   |
C           |                                          |
C            ------------------------------------------

C           Adjust pointers. Increase LEFT or decrease RIGHT until done.

            IF (XBYARROW .GE. X (TRIAL + 1) * ARROW) THEN
               LEFT  = TRIAL + 1
            ELSE IF (XBYARROW .LT. X (TRIAL) * ARROW) THEN
               RIGHT = TRIAL
            ELSE

C              We're done: XFIND is in the interval [X (TRIAL), X (TRIAL+1)).

               LEFT  = TRIAL
               GO TO 990
            END IF
            GO TO 10

         END IF

C     Termination.
C     ------------

  990 CONTINUE
      RETURN
      END
C+----------------------------------------------------------------------
C
      SUBROUTINE LCSFIT (NDATA, X, Y, NEW, METHOD, NEVAL, XEVAL, YEVAL,
     &   YPEVAL)
C
C     Two-liner:  Storage-efficient local cubic spline fit (2-space)
C     ----------  (monotonic and piecewise linear options too)
C
C     Description and usage:
C     ----------------------
C
C        LCSFIT is the non-parametric analog of PLSFIT (parametric).
C     It is intended for spline applications which do not require the
C     spline coefficients as output.  It is efficient for repeated
C     calls with the same data, so repeated use with NEVAL = 1 may be
C     preferable to storing vectors of results.
C
C        LCSFIT offers monotonic spline and piecewise linear options
C     also.  And it returns an interpolated first derivative along
C     with the function value.  (The second derivative is omitted
C     because Y" is not guaranteed to be continuous by local methods.)
C
C        See PLSFIT for more details on local methods.  As with most
C     numerical methods, scaling of the data to the unit interval (and
C     unscaling of the result) is recommended to avoid unnecessary
C     effects attributable to the data units.  Utilities GETSCALE and
C     USESCALE from the present authors are appropriate.  The data
C     abscissas should be distinct and either ascending or descending.
C     PROTECT is available to check this.  Extrapolation is permitted
C     (mainly in case of round-off; it is normally inadvisable).
C
C        The CSFIT/CSEVAL or CSDVAL pair are probably preferable if
C     efficiency is not an issue, since CSFIT gives Y" continuity.
C
C     Arguments:
C     ----------
C
C     Name    Type/Dimension  I/O/S  Description
C     NDATA   I               I      Length of X, Y input data arrays.
C
C     X,      R (NDATA)       I      Input data coordinates.  The Xs
C     Y                              must be distinct and monotonic,
C                                    either ascending or descending.
C                                    (No check here.) 
C
C     NEW     L               I      If control flag NEW is .TRUE., the
C                                    search for a bracket starts from
C                                    scratch, otherwise locally-saved
C                                    search and fit information will be
C                                    assumed to be correct. If calling
C                                    LCSFIT from within a loop, set
C                                    NEW = .FALSE. after the first call.
C
C     METHOD   C*1            I      (Uppercase) Type of fit to be used:
C                                    'M' means Monotonic piecewise cubics;
C                                    'B' means non-monotonic "Bessel"-type
C                                        piecewise cubics (looser fit);
C                                    'L' means piecewise Linear fit;
C                                    'C' means Cyclic (periodic) end
C                                        conditions: loose fit assumed.
C
C     NEVAL   I               I      Number of interpolations requested.
C                                    NEVAL >= 1.  One call per result
C                                    (NEVAL = 1) may save storage, and is
C                                    not too inefficient as long as NEW
C                                    is set to .FALSE. after the first.
C
C     XEVAL   R (NEVAL)       I      Abscissa(s) to interpolate to.  These
C                                    are normally in the data range, but
C                                    extrapolation - probably due to
C                                    round-off - is not prevented.
C
C     YEVAL   R (NEVAL)       O      Interpolated function value(s).
C
C     YPEVAL  R (NEVAL)       O      Interpolated 1st derivative value(s).
C                                    Pass the same storage as for YEVAL
C                                    if no derivatives are required.
C
C     Significant local variables:
C     ----------------------------
C
C     MEMORY         Indicates that coefficients are correct for the
C                    current point.
C
C     H, DEL         Delta X and forward difference derivative arrays.
C
C     B, C, D        Coefficients of cubic on the bracketing interval.
C
C     Procedures:
C     -----------
C
C     INTERVAL  1-D "interpolation" search.
C     BESSEL    First derivative (central 3-point formula).
C     BRODLIE   First derivative (central), adjusted for monotonicity.
C     BUTLAND   First derivative (non-central), adjusted for monotonicity.
C     THREEPT   First derivative (non-central 3-point formula).
C
C     Environment:  Digital VAX-11/780, VMS FORTRAN 77
C     ------------
C
C     Error handling:  None
C     ---------------
C
C     Notes:
C     ------
C
C     (1)  IMPLICIT NONE, 8-character symbolic names, and "!" as comment
C          character are not (yet) standard.
C
C     (2)  Since many of the calculations must be repeated at both ends
C          of an interval, the various finite difference quantities used
C          are stored as arrays. The following "map" of a typical interior
C          interval and its neighbors should help in understanding the
C          notation.  The local array indices are all numbered relative
C          to the left-hand end of the interval which brackets the point
C          to be evaluated.
C
C                                  LEFT       RIGHT
C
C          Point         -1          0         +1          +2
C
C          Data           x -------- x -------- x --------- x
C
C          Interval      -1          0         +1
C
C
C     Author: Robert Kennelly, Sterling Software/NASA Ames  (PLSFIT)
C     -------
C
C     History:
C     --------
C
C     27 Feb. 1987  R.A.Kennelly  Initial design and coding of PLSFIT.
C     23 Aug. 1989  D.A.Saunders  LCSFIT adapted as non-parametric form,
C                                 for embedding in other utilities where
C                                 minimizing work-space is desirable.
C     20 June 1991    "    "      THREEPT (monotonic) renamed BUTLAND;
C                                 THREEPT (pure 3-pt. formula) now used
C                                 for nonmonotonic end-point handling;
C                                 METHOD='C' case belatedly added, as
C                                 needed by PLSINTRP for closed curves.
C     23 July 1991    "    "      The tests for being in the same interval
C                                 as before were not allowing for the
C                                 descending-Xs case.
C-----------------------------------------------------------------------

C     Declarations.
C     -------------

      IMPLICIT NONE

C     Arguments.

      LOGICAL
     &   NEW
      INTEGER
     &   NDATA, NEVAL
      REAL
     &   X (NDATA), XEVAL (NEVAL), Y (NDATA), YEVAL (NEVAL),
     &   YPEVAL (NEVAL)
      CHARACTER
     &   METHOD * 1

C     Local constants.

      REAL
     &   ZERO, ONE, TWO, THREE
      PARAMETER
     &  (ZERO  = 0.0E+0,
     &   ONE   = 1.0E+0,
     &   TWO   = 2.0E+0,
     &   THREE = 3.0E+0)

C     Local variables.

      LOGICAL
     &   CYCLIC, MEMORY, MONO
      INTEGER
     &   IEVAL, J, K, LEFT, RIGHT
      REAL
     &   ARROW, B (0:1), C, DELY (-1:1), D, DX, H (-1:1), XBYARROW, XE

C     Procedures.

      REAL
     &   BESSEL, BRODLIE, BUTLAND, THREEPT
      EXTERNAL
     &   BESSEL, BRODLIE, BUTLAND, THREEPT, INTERVAL

C     Storage.

      SAVE
     &   ARROW, B, C, D, LEFT, RIGHT

C     Execution.
C     ----------

      MONO   = METHOD .EQ. 'M'
      CYCLIC = METHOD .EQ. 'C'

      IF (CYCLIC) THEN
         IF (Y (NDATA) .NE. Y (1)) STOP 'LCSFIT: End points must match.'
      END IF

C     Initialize search or avoid it if possible.

      IF (NEW) THEN
         MEMORY = .FALSE.
         ARROW  = SIGN (ONE, X (2) - X (1))
         LEFT   = 1
      END IF

      IEVAL = 1
      XE = XEVAL (1)
      XBYARROW = XE * ARROW

      IF (.NOT. NEW) THEN
      
C        We can save a lot of time when LCSFIT is being called from within
C        a loop by setting MEMORY if possible. The out-of-range checking
C        relies on the fact that RIGHT = LEFT + 1 after the last return.
C        Cater to the more likely case of XE in the previous, interior
C        interval.

         MEMORY = XBYARROW .GE. X (LEFT)  * ARROW .AND.
     &            XBYARROW .LT. X (RIGHT) * ARROW
         IF (.NOT. MEMORY) THEN
            MEMORY =
     &         LEFT  .EQ. 1     .AND. XBYARROW .LT. X (RIGHT) * ARROW
     &         .OR.
     &         RIGHT .EQ. NDATA .AND. XBYARROW .GE. X (LEFT)  * ARROW
         END IF
      END IF

      IF (MEMORY) GO TO 70        ! Skip the bulk of the computation.

C     Loop over evaluation points requiring a new search.
C     ---------------------------------------------------

   10 CONTINUE

C        Interpolation search for bracketing interval.
C        ---------------------------------------------

         CALL INTERVAL (NDATA, X, XE, ARROW, LEFT)

         RIGHT = LEFT + 1

C         -------------------------------------------
C        |                                           |
C        |   1 <= LEFT < RIGHT = LEFT + 1 <= NDATA   |
C        |                                           |
C         -------------------------------------------

C        Compute derivatives by finite-differences.
C        ------------------------------------------

         IF (NDATA .GT. 2 .AND. METHOD .NE. 'L') THEN

C           Interval and derivative approximations.
C           ---------------------------------------

C           The following duplicates more code than PLSFIT's approach,
C           but eliminates some indirection - no need to wrap-around here.
C           Handle the end conditions first to minimize testing LEFT, RIGHT.

            IF (LEFT .EQ. 1) THEN

               H (0) = X (2) - X (1)
               DELY (0) = (Y (2) - Y (1)) / H (0)
               H (1) = X (3) - X (2)
               DELY (1) = (Y (3) - Y (2)) / H (1)

               IF (CYCLIC) THEN  ! Loose fit assumed
                  H (-1) = X (NDATA) - X (NDATA - 1)
                  DELY (-1) = (Y (NDATA) - Y (NDATA - 1)) / H (-1)
                  B (0) = BESSEL (0, H, DELY)
                  B (1) = BESSEL (1, H, DELY)
               ELSE
                  IF (MONO) THEN
                     B (0) = BUTLAND (0, H, DELY)
                     B (1) = BRODLIE (1, H, DELY)
                  ELSE
                     B (0) = THREEPT (0, H, DELY)
                     B (1) = BESSEL  (1, H, DELY)
                  END IF
               END IF

            ELSE IF (RIGHT .EQ. NDATA) THEN

               H(-1) = X (LEFT) - X (LEFT-1)
               DELY(-1) = (Y (LEFT) - Y (LEFT-1)) / H (-1)
               H (0) = X (RIGHT) - X (LEFT)
               DELY (0) = (Y (RIGHT) - Y (LEFT)) / H (0)

               IF (CYCLIC) THEN
                  H (1) = X (2) - X (1)
                  DELY (1) = (Y (2) - Y (1)) / H (1)
                  B (0) = BESSEL (0, H, DELY)
                  B (1) = BESSEL (1, H, DELY)
               ELSE

                  IF (MONO) THEN
                     B (0) = BRODLIE (0, H, DELY)
                     B (1) = BUTLAND (1, H, DELY)
                  ELSE
                     B (0) = BESSEL  (0, H, DELY)
                     B (1) = THREEPT (1, H, DELY)
                  END IF
               END IF

            ELSE

               K = LEFT
               DO 40, J = -1, +1
                  H (J)    =  X (K) - X (K-1)
                  DELY (J) = (Y (K) - Y (K-1)) / H (J)
                  K = K + 1
   40          CONTINUE

C              Select interpolation scheme.
C              ----------------------------

C              Compute (possibly adjusted) first derivatives at both
C              left- and right-hand endpoints of the interval.

               IF (MONO) THEN

C                 Monotone - use Brodlie modification of Butland's
C                 formula to adjust the derivatives at the knots.

                  B (0) = BRODLIE (0, H, DELY)
                  B (1) = BRODLIE (1, H, DELY)

               ELSE                          ! IF (METHOD .EQ. 'B') THEN

C                 Bessel - use central difference formula at the knots.

                  B (0) = BESSEL (0, H, DELY)
                  B (1) = BESSEL (1, H, DELY)

               END IF

            END IF

C           Compute the remaining cubic coefficients.

            C = (THREE * DELY (0) - TWO * B (0) - B (1)) / H (0)
            D = ( -TWO * DELY (0) + B (0) + B (1)) / H (0) ** 2

         ELSE              ! IF (NDATA .EQ. 2 .OR. METHOD .EQ. 'L') THEN

C           Degenerate case (linear).
C           -------------------------

            B (0) = (Y (RIGHT) - Y (LEFT)) / (X (RIGHT) - X (LEFT))
            C     = ZERO
            D     = ZERO
         END IF

C        Evaluate the cubic (derivative first in case only YEVAL is reqd.)
C        -----------------------------------------------------------------

   70    CONTINUE    ! Start of same-interval loop inside new-interval loop.

            DX = XE - X (LEFT)
            YPEVAL (IEVAL) = B (0) + DX * (TWO * C + DX * THREE * D)
            YEVAL  (IEVAL) = Y (LEFT) + DX * (B (0) + DX * (C + DX * D))

C           The next evaluation point may be in the same interval.
C           ------------------------------------------------------

            IF (IEVAL .LT. NEVAL) THEN      ! Skips this if NEVAL = 1.
               IEVAL = IEVAL + 1
               XE = XEVAL (IEVAL)
               XBYARROW = XE * ARROW
               IF (XBYARROW .GE. X (LEFT)  * ARROW  .AND.
     &             XBYARROW .LT. X (RIGHT) * ARROW) GO TO 70
            
               GO TO 10                     ! Else much more work required.

            END IF

C     Termination.
C     ------------

      RETURN
      END
C     Machine dependent interpolation routine FTLUP replaced with LCSFIT.
C     20 Nov. 1989  L. Hardy   RAA Branch NASA Ames
  
      SUBROUTINE PHEP63(PHI,EPS)
      DIMENSION PHI(1),EPS(1)
      DIMENSION A(402)
C     DIMENSION PHILD(201),EPSLD(201)
C     DATA (PHILD(I),EPSLD(I),I=1,25 )/
      DATA (A(I), I = 1, 50) /
     1   0.00000,   0.00000,    .01569,    .00164,    .03139,
     1    .00327,    .04708,    .00487,    .06278,    .00641,
     1    .07848,    .00789,    .09419,    .00928,    .10990,
     1    .01057,    .12562,    .01174,    .14135,    .01278,
     1    .15708,    .01367,    .17277,    .01439,    .18846,
     1    .01497,    .20416,    .01542,    .21987,    .01576,
     1    .23558,    .01601,    .25129,    .01619,    .26701,
     1    .01632,    .28273,    .01642,    .29844,    .01651,
     1    .31416,    .01661,    .32987,    .01673,    .34559,
     1    .01688,    .36130,    .01705,    .37700,    .01725/
C     DATA (PHILD(I),EPSLD(I),I=26,50)/
      DATA (A(I), I = 51,100) /
     1    .39271,    .01747,    .40842,    .01771,    .42412,
     1    .01797,    .43983,    .01824,    .45553,    .01853,
     1    .47124,    .01884,    .48695,    .01916,    .50266,
     1    .01949,    .51837,    .01984,    .53407,    .02020,
     1    .54978,    .02058,    .56549,    .02097,    .58120,
     1    .02137,    .59691,    .02179,    .61261,    .02223,
     1    .62832,    .02268,    .64403,    .02315,    .65974,
     1    .02363,    .67545,    .02413,    .69116,    .02464,
     1    .70687,    .02517,    .72257,    .02571,    .73828,
     1    .02626,    .75399,    .02683,    .76969,    .02741/
C     DATA (PHILD(I),EPSLD(I),I=51,75)/
      DATA (A(I), I = 101,150) /
     1    .78540,    .02801,    .80111,    .02862,    .81682,
     1    .02924,    .83253,    .02988,    .84824,    .03052,
     1    .86395,    .03118,    .87965,    .03185,    .89536,
     1    .03253,    .91107,    .03323,    .92677,    .03393,
     1    .94248,    .03465,    .95819,    .03538,    .97390,
     1    .03611,    .98961,    .03686,   1.00532,    .03762,
     1   1.02103,    .03839,   1.03673,    .03917,   1.05244,
     1    .03995,   1.06815,    .04075,   1.08385,    .04156,
     1   1.09956,    .04237,   1.11527,    .04319,   1.13098,
     1    .04402,   1.14669,    .04486,   1.16240,    .04571/
C     DATA (PHILD(I),EPSLD(I),I=76,100)/
      DATA (A(I), I = 151,200)/
     1   1.17811,    .04657,   1.19381,    .04743,   1.20952,
     1    .04831,   1.22523,    .04919,   1.24093,    .05008,
     1   1.25664,    .05098,   1.27235,    .05189,   1.28806,
     1    .05280,   1.30376,    .05372,   1.31947,    .05464,
     1   1.33518,    .05556,   1.35089,    .05648,   1.36659,
     1    .05740,   1.38230,    .05831,   1.39801,    .05921,
     1   1.41372,    .06011,   1.42942,    .06099,   1.44513,
     1    .06187,   1.46084,    .06273,   1.47654,    .06357,
     1   1.49225,    .06440,   1.50796,    .06522,   1.52367,
     1    .06602,   1.53938,    .06681,   1.55509,    .06757/
C     DATA (PHILD(I),EPSLD(I),I=101,125)/
      DATA (A(I), I = 201,250) /
     1   1.57080,    .06832,   1.58650,    .06905,   1.60221,
     1    .06976,   1.61791,    .07044,   1.63362,    .07111,
     1   1.64933,    .07176,   1.66504,    .07238,   1.68075,
     1    .07298,   1.69646,    .07356,   1.71217,    .07411,
     1   1.72788,    .07464,   1.74358,    .07514,   1.75929,
     1    .07562,   1.77500,    .07607,   1.79070,    .07650,
     1   1.80641,    .07690,   1.82212,    .07727,   1.83783,
     1    .07761,   1.85354,    .07793,   1.86925,    .07822,
     1   1.88496,    .07848,   1.90067,    .07871,   1.91637,
     1    .07891,   1.93208,    .07908,   1.94779,    .07922/
C     DATA (PHILD(I),EPSLD(I),I=126,150)/
      DATA (A(I), I = 251, 300) /
     1   1.96350,    .07933,   1.97921,    .07941,   1.99491,
     1    .07945,   2.01062,    .07946,   2.02633,    .07944,
     1   2.04204,    .07938,   2.05775,    .07929,   2.07346,
     1    .07916,   2.08917,    .07900,   2.10487,    .07880,
     1   2.12058,    .07856,   2.13629,    .07829,   2.15200,
     1    .07799,   2.16770,    .07764,   2.18341,    .07726,
     1   2.19911,    .07685,   2.21482,    .07640,   2.23054,
     1    .07591,   2.24625,    .07539,   2.26196,    .07483,
     1   2.27767,    .07423,   2.29338,    .07359,   2.30908,
     1    .07293,   2.32479,    .07222,   2.34049,    .07148/
C     DATA (PHILD(I),EPSLD(I),I=151,175)/
      DATA (A(I), I = 301,350) /
     1   2.35619,    .07070,   2.37191,    .06989,   2.38762,
     1    .06904,   2.40334,    .06815,   2.41905,    .06723,
     1   2.43476,    .06628,   2.45046,    .06529,   2.46617,
     1    .06427,   2.48187,    .06322,   2.49757,    .06214,
     1   2.51327,    .06103,   2.52899,    .05989,   2.54470,
     1    .05871,   2.56042,    .05751,   2.57613,    .05628,
     1   2.59184,    .05502,   2.60754,    .05374,   2.62325,
     1    .05243,   2.63895,    .05109,   2.65465,    .04973,
     1   2.67035,    .04834,   2.68607,    .04693,   2.70178,
     1    .04549,   2.71749,    .04404,   2.73320,    .04256/
C     DATA (PHILD(I),EPSLD(I),I=176,201)/
      DATA (A(I), I=351, 402)/
     1   2.74891,    .04106,   2.76462,    .03955,   2.78032,
     1    .03802,   2.79603,    .03647,   2.81173,    .03491,
     1   2.82743,    .03333,   2.84314,    .03174,   2.85885,
     1    .03014,   2.87456,    .02853,   2.89027,    .02690,
     1   2.90598,    .02527,   2.92169,    .02363,   2.93740,
     1    .02198,   2.95310,    .02032,   2.96881,    .01865,
     1   2.98451,    .01698,   3.00022,    .01530,   3.01593,
     1    .01361,   3.03164,    .01192,   3.04735,    .01023,
     1   3.06305,    .00853,   3.07876,    .00683,   3.09447,
     1    .00512,   3.11018,    .00342,   3.12588,    .00171,
     1   3.14159,   0.00000/
      DO 201 J=1,201
      PHI(J) = A(2*J-1)
      EPS(J) = A(2*J)
  201 CONTINUE
      RETURN
      END
      SUBROUTINE PHEP63A(PHI,EPS)
      DIMENSION PHI(1),EPS(1)
      DIMENSION PHILD(251),EPSLD(251), A(502)
      DATA (A(I), I = 1, 50) /
     1   0.00000,   0.00000,    .01459,    .00138,    .02917,
     1    .00276,    .04376,    .00413,    .05835,    .00548,
     1    .07294,    .00683,    .08754,    .00815,    .10213,
     1    .00945,    .11673,    .01072,    .13133,    .01196,
     1    .14594,    .01317,    .14925,    .01344,    .15256,
     1    .01370,    .15587,    .01396,    .15919,    .01421,
     1    .16250,    .01445,    .16581,    .01469,    .16912,
     1    .01490,    .17244,    .01510,    .17575,    .01529,
     1    .17907,    .01546,    .18420,    .01568,    .18933,
     1    .01585,    .19447,    .01599,    .19960,    .01610/
      DATA (A(I), I = 51,100) /
     1    .20474,    .01617,    .20988,    .01623,    .21501,
     1    .01628,    .22015,    .01631,    .22529,    .01635,
     1    .23043,    .01638,    .24002,    .01647,    .24960,
     1    .01657,    .25918,    .01670,    .26877,    .01684,
     1    .27835,    .01698,    .28793,    .01713,    .29751,
     1    .01728,    .30709,    .01741,    .31667,    .01754,
     1    .32626,    .01764,    .33981,    .01775,    .35335,
     1    .01781,    .36691,    .01785,    .38046,    .01787,
     1    .39401,    .01787,    .40756,    .01788,    .42111,
     1    .01788,    .43466,    .01791,    .44821,    .01796/
      DATA (A(I), I = 101,150) /
     1    .46176,    .01804,    .47241,    .01814,    .48306,
     1    .01826,    .49371,    .01841,    .50436,    .01857,
     1    .51501,    .01875,    .52566,    .01895,    .53631,
     1    .01917,    .54696,    .01939,    .55761,    .01963,
     1    .56826,    .01987,    .57741,    .02009,    .58655,
     1    .02031,    .59570,    .02053,    .60485,    .02075,
     1    .61399,    .02098,    .62314,    .02121,    .63229,
     1    .02144,    .64143,    .02168,    .65058,    .02192,
     1    .65973,    .02216,    .67542,    .02257,    .69111,
     1    .02299,    .70680,    .02342,    .72249,    .02385/
      DATA (A(I), I = 151,200)/
     1    .73818,    .02429,    .75386,    .02475,    .76955,
     1    .02521,    .78524,    .02569,    .80093,    .02618,
     1    .81662,    .02669,    .83032,    .02715,    .84402,
     1    .02762,    .85772,    .02810,    .87143,    .02859,
     1    .88513,    .02909,    .89883,    .02960,    .91253,
     1    .03011,    .92623,    .03063,    .93993,    .03115,
     1    .95363,    .03168,    .96612,    .03216,    .97860,
     1    .03265,    .99109,    .03314,   1.00358,    .03363,
     1   1.01607,    .03412,   1.02856,    .03462,   1.04104,
     1    .03513,   1.05353,    .03564,   1.06602,    .03616/
      DATA (A(I), I = 201,250) /
     1   1.07850,    .03669,   1.09023,    .03719,   1.10196,
     1    .03769,   1.11369,    .03821,   1.12541,    .03873,
     1   1.13714,    .03925,   1.14887,    .03979,   1.16060,
     1    .04033,   1.17232,    .04087,   1.18405,    .04143,
     1   1.19577,    .04198,   1.20699,    .04253,   1.21821,
     1    .04307,   1.22942,    .04362,   1.24064,    .04418,
     1   1.25186,    .04473,   1.26307,    .04528,   1.27429,
     1    .04584,   1.28551,    .04639,   1.29672,    .04694,
     1   1.30794,    .04749,   1.31876,    .04801,   1.32958,
     1    .04853,   1.34041,    .04904,   1.35123,    .04955/
      DATA (A(I), I = 251, 300) /
     1   1.36205,    .05005,   1.37288,    .05056,   1.38370,
     1    .05106,   1.39453,    .05155,   1.40535,    .05205,
     1   1.41617,    .05254,   1.42675,    .05302,   1.43732,
     1    .05350,   1.44789,    .05398,   1.45847,    .05445,
     1   1.46904,    .05492,   1.47961,    .05538,   1.49019,
     1    .05585,   1.50076,    .05630,   1.51133,    .05676,
     1   1.52191,    .05721,   1.53232,    .05764,   1.54273,
     1    .05807,   1.55315,    .05850,   1.56356,    .05892,
     1   1.57397,    .05933,   1.58439,    .05973,   1.59480,
     1    .06012,   1.60522,    .06050,   1.61563,    .06088/
      DATA (A(I), I = 301,350) /
     1   1.62605,    .06124,   1.63637,    .06158,   1.64669,
     1    .06192,   1.65701,    .06224,   1.66733,    .06255,
     1   1.67765,    .06286,   1.68797,    .06315,   1.69829,
     1    .06344,   1.70862,    .06372,   1.71894,    .06399,
     1   1.72926,    .06425,   1.73961,    .06451,   1.74996,
     1    .06476,   1.76031,    .06500,   1.77066,    .06523,
     1   1.78101,    .06546,   1.79136,    .06567,   1.80171,
     1    .06588,   1.81206,    .06608,   1.82241,    .06626,
     1   1.83276,    .06643,   1.84322,    .06660,   1.85368,
     1    .06675,   1.86414,    .06689,   1.87461,    .06702/
      DATA (A(I), I = 351,400) /
     1   1.88507,    .06714,   1.89553,    .06724,   1.90600,
     1    .06733,   1.91646,    .06740,   1.92692,    .06746,
     1   1.93738,    .06751,   1.94807,    .06754,   1.95875,
     1    .06755,   1.96943,    .06756,   1.98011,    .06754,
     1   1.99080,    .06752,   2.00148,    .06748,   2.01216,
     1    .06742,   2.02284,    .06736,   2.03353,    .06728,
     1   2.04421,    .06718,   2.05529,    .06707,   2.06637,
     1    .06695,   2.07745,    .06681,   2.08853,    .06667,
     1   2.09961,    .06650,   2.11068,    .06632,   2.12176,
     1    .06613,   2.13284,    .06593,   2.14392,    .06571/
      DATA (A(I), I=401,450)/
     1   2.15500,    .06548,   2.16671,    .06522,   2.17842,
     1    .06494,   2.19013,    .06464,   2.20184,    .06434,
     1   2.21355,    .06401,   2.22526,    .06367,   2.23697,
     1    .06332,   2.24868,    .06295,   2.26038,    .06257,
     1   2.27209,    .06217,   2.28483,    .06173,   2.29758,
     1    .06126,   2.31032,    .06078,   2.32306,    .06029,
     1   2.33580,    .05979,   2.34855,    .05927,   2.36129,
     1    .05875,   2.37403,    .05821,   2.38677,    .05767,
     1   2.39951,    .05712,   2.41402,    .05648,   2.42853,
     1    .05583,   2.44304,    .05517,   2.45755,    .05448/
      DATA (A(I), I=451,502) /
     1   2.47205,    .05376,   2.48656,    .05300,   2.50106,
     1    .05222,   2.51556,    .05138,   2.53005,    .05050,
     1   2.54455,    .04957,   2.56243,    .04834,   2.58031,
     1    .04703,   2.59819,    .04565,   2.61606,    .04421,
     1   2.63393,    .04273,   2.65180,    .04120,   2.66966,
     1    .03965,   2.68753,    .03807,   2.70539,    .03649,
     1   2.72325,    .03491,   2.76507,    .03125,   2.80690,
     1    .02764,   2.84873,    .02408,   2.89056,    .02056,
     1   2.93240,    .01708,   2.97423,    .01363,   3.01607,
     1    .01020,   3.05791,    .00679,   3.09975,    .00339,
     1   3.14159,   0.00000/
      DO 100 J=1,251
      PHILD(J) = A(2*J-1)
      EPSLD(J) = A(2*J)
  100 CONTINUE
      DO 201 J=1,201
      PHI(J)=FLOAT(J-1)*3.141592654/200.
  201 CALL LCSFIT (251, PHILD, EPSLD, .TRUE., 'B', 201, PHI, EPS, EPS)
      RETURN
      END
      SUBROUTINE PHEP64(PHI,EPS)
      DIMENSION PHI(1),EPS(1)
      DIMENSION A(402)
      DATA (A(I), I = 1, 50) /
     1   0.00000,   0.00000,    .01568,    .00233,    .03136,
     1    .00464,    .04705,    .00692,    .06274,    .00914,
     1    .07843,    .01129,    .09414,    .01336,    .10985,
     1    .01531,    .12558,    .01714,    .14132,    .01883,
     1    .15708,    .02035,    .17274,    .02169,    .18842,
     1    .02287,    .20411,    .02390,    .21981,    .02480,
     1    .23552,    .02557,    .25124,    .02624,    .26697,
     1    .02682,    .28270,    .02731,    .29843,    .02774,
     1    .31416,    .02812,    .32987,    .02846,    .34558,
     1    .02877,    .36129,    .02905,    .37700,    .02931/
      DATA (A(I), I = 51,100) /
     1    .39271,    .02957,    .40842,    .02982,    .42412,
     1    .03007,    .43983,    .03033,    .45553,    .03060,
     1    .47124,    .03090,    .48695,    .03122,    .50266,
     1    .03158,    .51837,    .03196,    .53408,    .03236,
     1    .54979,    .03280,    .56549,    .03326,    .58120,
     1    .03375,    .59691,    .03427,    .61262,    .03481,
     1    .62832,    .03538,    .64403,    .03598,    .65975,
     1    .03660,    .67546,    .03725,    .69117,    .03792,
     1    .70688,    .03862,    .72258,    .03935,    .73829,
     1    .04010,    .75399,    .04087,    .76970,    .04167/
      DATA (A(I), I = 101,150) /
     1    .78540,    .04250,    .80111,    .04335,    .81683,
     1    .04423,    .83254,    .04512,    .84825,    .04605,
     1    .86396,    .04699,    .87967,    .04796,    .89537,
     1    .04896,    .91108,    .04998,    .92678,    .05102,
     1    .94248,    .05208,    .95820,    .05317,    .97391,
     1    .05428,    .98962,    .05541,   1.00533,    .05657,
     1   1.02104,    .05774,   1.03675,    .05894,   1.05245,
     1    .06016,   1.06816,    .06140,   1.08386,    .06267,
     1   1.09956,    .06395,   1.11528,    .06526,   1.13099,
     1    .06658,   1.14671,    .06793,   1.16242,    .06931/
      DATA (A(I), I = 151,200)/
     1   1.17813,    .07070,   1.19384,    .07213,   1.20954,
     1    .07357,   1.22524,    .07505,   1.24094,    .07655,
     1   1.25664,    .07808,   1.27236,    .07964,   1.28807,
     1    .08123,   1.30378,    .08284,   1.31949,    .08447,
     1   1.33520,    .08613,   1.35091,    .08780,   1.36661,
     1    .08949,   1.38232,    .09119,   1.39802,    .09290,
     1   1.41372,    .09462,   1.42942,    .09635,   1.44512,
     1    .09808,   1.46083,    .09980,   1.47653,    .10151,
     1   1.49223,    .10321,   1.50794,    .10488,   1.52365,
     1    .10653,   1.53936,    .10815,   1.55508,    .10972/
      DATA (A(I), I = 201,250) /
     1   1.57080,    .11125,   1.58649,    .11273,   1.60218,
     1    .11415,   1.61788,    .11553,   1.63358,    .11686,
     1   1.64929,    .11814,   1.66500,    .11938,   1.68072,
     1    .12057,   1.69643,    .12171,   1.71216,    .12281,
     1   1.72788,    .12386,   1.74358,    .12487,   1.75928,
     1    .12583,   1.77498,    .12675,   1.79068,    .12762,
     1   1.80639,    .12844,   1.82210,    .12922,   1.83781,
     1    .12994,   1.85353,    .13062,   1.86924,    .13125,
     1   1.88496,    .13182,   1.90066,    .13234,   1.91637,
     1    .13281,   1.93207,    .13322,   1.94778,    .13358/
      DATA (A(I), I = 251, 300) /
     1   1.96349,    .13389,   1.97920,    .13414,   1.99491,
     1    .13434,   2.01062,    .13448,   2.02633,    .13456,
     1   2.04204,    .13459,   2.05775,    .13456,   2.07346,
     1    .13447,   2.08917,    .13433,   2.10488,    .13413,
     1   2.12059,    .13387,   2.13630,    .13354,   2.15200,
     1    .13316,   2.16771,    .13272,   2.18341,    .13222,
     1   2.19911,    .13166,   2.21483,    .13104,   2.23055,
     1    .13035,   2.24627,    .12960,   2.26198,    .12879,
     1   2.27770,    .12792,   2.29340,    .12698,   2.30911,
     1    .12598,   2.32481,    .12492,   2.34050,    .12380/
      DATA (A(I), I = 301,350) /
     1   2.35619,    .12261,   2.37192,    .12136,   2.38765,
     1    .12004,   2.40337,    .11866,   2.41909,    .11722,
     1   2.43480,    .11572,   2.45051,    .11416,   2.46621,
     1    .11254,   2.48190,    .11087,   2.49759,    .10914,
     1   2.51327,    .10735,   2.52901,    .10550,   2.54474,
     1    .10360,   2.56047,    .10164,   2.57619,    .09963,
     1   2.59190,    .09757,   2.60760,    .09545,   2.62330,
     1    .09328,   2.63899,    .09105,   2.65467,    .08878,
     1   2.67035,    .08645,   2.68609,    .08406,   2.70182,
     1    .08162,   2.71755,    .07913,   2.73327,    .07660/
      DATA (A(I), I=351, 402)/
     1   2.74898,    .07402,   2.76468,    .07139,   2.78038,
     1    .06873,   2.79607,    .06603,   2.81175,    .06329,
     1   2.82743,    .06052,   2.84316,    .05770,   2.85889,
     1    .05486,   2.87461,    .05198,   2.89032,    .04907,
     1   2.90603,    .04614,   2.92173,    .04318,   2.93743,
     1    .04020,   2.95313,    .03720,   2.96882,    .03417,
     1   2.98451,    .03113,   3.00023,    .02807,   3.01594,
     1    .02499,   3.03165,    .02189,   3.04736,    .01879,
     1   3.06307,    .01567,   3.07878,    .01255,   3.09448,
     1    .00942,   3.11018,    .00628,   3.12589,    .00314,
     1   3.14159,   0.00000/
      DO 201 J=1,201
      PHI(J) = A(2*J-1)
      EPS(J) = A(2*J)
  201 CONTINUE
      RETURN
      END
      SUBROUTINE PHEP64A(PHI,EPS)
      DIMENSION PHI(1),EPS(1)
      DIMENSION PHILD(251),EPSLD(251), A(502)
      DATA (A(I), I = 1, 50) /
     1   0.00000,   0.00000,    .01468,    .00149,    .02936,
     1    .00297,    .04403,    .00445,    .05872,    .00591,
     1    .07340,    .00736,    .08808,    .00878,    .10277,
     1    .01018,    .11746,    .01156,    .13216,    .01290,
     1    .14686,    .01420,    .15019,    .01449,    .15352,
     1    .01477,    .15685,    .01505,    .16018,    .01532,
     1    .16351,    .01558,    .16684,    .01583,    .17017,
     1    .01606,    .17350,    .01628,    .17684,    .01648,
     1    .18017,    .01667,    .18528,    .01690,    .19039,
     1    .01709,    .19551,    .01724,    .20062,    .01736/
      DATA (A(I), I = 51,100) /
     1    .20574,    .01745,    .21086,    .01752,    .21598,
     1    .01758,    .22109,    .01762,    .22621,    .01767,
     1    .23133,    .01772,    .24090,    .01782,    .25046,
     1    .01796,    .26003,    .01811,    .26959,    .01828,
     1    .27915,    .01845,    .28871,    .01861,    .29827,
     1    .01877,    .30783,    .01891,    .31739,    .01903,
     1    .32695,    .01912,    .34040,    .01919,    .35384,
     1    .01920,    .36728,    .01916,    .38072,    .01908,
     1    .39417,    .01897,    .40761,    .01886,    .42105,
     1    .01874,    .43450,    .01862,    .44794,    .01854/
      DATA (A(I), I = 101,150) /
     1    .46139,    .01848,    .47194,    .01846,    .48249,
     1    .01847,    .49304,    .01851,    .50359,    .01856,
     1    .51414,    .01864,    .52469,    .01873,    .53523,
     1    .01883,    .54578,    .01895,    .55633,    .01907,
     1    .56688,    .01921,    .57595,    .01933,    .58502,
     1    .01945,    .59410,    .01958,    .60317,    .01972,
     1    .61224,    .01985,    .62131,    .01999,    .63038,
     1    .02013,    .63945,    .02028,    .64852,    .02043,
     1    .65759,    .02058,    .67317,    .02084,    .68874,
     1    .02111,    .70432,    .02139,    .71989,    .02169/
      DATA (A(I), I = 151,200)/
     1    .73546,    .02199,    .75104,    .02231,    .76661,
     1    .02264,    .78218,    .02299,    .79776,    .02335,
     1    .81333,    .02373,    .82695,    .02408,    .84057,
     1    .02444,    .85419,    .02482,    .86781,    .02520,
     1    .88142,    .02560,    .89504,    .02601,    .90866,
     1    .02643,    .92228,    .02685,    .93589,    .02728,
     1    .94951,    .02772,    .96194,    .02812,    .97437,
     1    .02852,    .98679,    .02893,    .99922,    .02935,
     1   1.01165,    .02977,   1.02407,    .03020,   1.03650,
     1    .03064,   1.04892,    .03108,   1.06135,    .03153/
      DATA (A(I), I = 201,250) /
     1   1.07377,    .03198,   1.08546,    .03242,   1.09714,
     1    .03286,   1.10882,    .03332,   1.12050,    .03378,
     1   1.13219,    .03425,   1.14387,    .03472,   1.15555,
     1    .03521,   1.16723,    .03571,   1.17891,    .03622,
     1   1.19059,    .03674,   1.20180,    .03724,   1.21301,
     1    .03776,   1.22423,    .03828,   1.23544,    .03882,
     1   1.24665,    .03936,   1.25786,    .03990,   1.26907,
     1    .04045,   1.28028,    .04100,   1.29149,    .04156,
     1   1.30270,    .04212,   1.31357,    .04266,   1.32444,
     1    .04320,   1.33531,    .04375,   1.34618,    .04429/
      DATA (A(I), I = 251, 300) /
     1   1.35705,    .04484,   1.36792,    .04539,   1.37878,
     1    .04594,   1.38965,    .04649,   1.40052,    .04704,
     1   1.41139,    .04760,   1.42204,    .04814,   1.43268,
     1    .04869,   1.44333,    .04923,   1.45397,    .04978,
     1   1.46462,    .05032,   1.47527,    .05087,   1.48591,
     1    .05141,   1.49656,    .05194,   1.50721,    .05248,
     1   1.51785,    .05301,   1.52834,    .05352,   1.53883,
     1    .05403,   1.54932,    .05454,   1.55980,    .05504,
     1   1.57029,    .05552,   1.58078,    .05600,   1.59127,
     1    .05647,   1.60176,    .05693,   1.61225,    .05738/
      DATA (A(I), I = 301,350) /
     1   1.62274,    .05781,   1.63312,    .05822,   1.64349,
     1    .05862,   1.65387,    .05901,   1.66425,    .05939,
     1   1.67463,    .05976,   1.68501,    .06011,   1.69539,
     1    .06045,   1.70577,    .06079,   1.71615,    .06111,
     1   1.72653,    .06143,   1.73692,    .06174,   1.74732,
     1    .06204,   1.75771,    .06233,   1.76811,    .06262,
     1   1.77851,    .06289,   1.78890,    .06316,   1.79930,
     1    .06341,   1.80970,    .06365,   1.82009,    .06389,
     1   1.83049,    .06411,   1.84100,    .06432,   1.85150,
     1    .06452,   1.86201,    .06471,   1.87252,    .06489/
      DATA (A(I), I = 351,400) /
     1   1.88303,    .06505,   1.89354,    .06520,   1.90405,
     1    .06534,   1.91455,    .06546,   1.92506,    .06557,
     1   1.93557,    .06567,   1.94630,    .06575,   1.95704,
     1    .06582,   1.96777,    .06587,   1.97850,    .06591,
     1   1.98923,    .06594,   1.99996,    .06595,   2.01069,
     1    .06594,   2.02142,    .06593,   2.03215,    .06590,
     1   2.04288,    .06586,   2.05401,    .06580,   2.06514,
     1    .06573,   2.07627,    .06565,   2.08740,    .06555,
     1   2.09853,    .06544,   2.10965,    .06531,   2.12078,
     1    .06517,   2.13191,    .06502,   2.14304,    .06485/
      DATA (A(I), I=401,450)/
     1   2.15416,    .06466,   2.16592,    .06445,   2.17767,
     1    .06422,   2.18943,    .06398,   2.20118,    .06371,
     1   2.21293,    .06344,   2.22469,    .06314,   2.23644,
     1    .06283,   2.24819,    .06251,   2.25994,    .06216,
     1   2.27169,    .06180,   2.28447,    .06140,   2.29725,
     1    .06097,   2.31002,    .06053,   2.32279,    .06007,
     1   2.33557,    .05960,   2.34834,    .05912,   2.36111,
     1    .05862,   2.37389,    .05811,   2.38666,    .05760,
     1   2.39943,    .05707,   2.41397,    .05646,   2.42850,
     1    .05584,   2.44304,    .05520,   2.45757,    .05454/
      DATA (A(I), I=451,502) /
     1   2.47210,    .05384,   2.48663,    .05312,   2.50115,
     1    .05235,   2.51568,    .05154,   2.53020,    .05068,
     1   2.54471,    .04977,   2.56262,    .04856,   2.58052,
     1    .04728,   2.59842,    .04592,   2.61631,    .04451,
     1   2.63420,    .04304,   2.65209,    .04154,   2.66997,
     1    .04000,   2.68785,    .03844,   2.70573,    .03687,
     1   2.72362,    .03530,   2.76541,    .03165,   2.80720,
     1    .02804,   2.84899,    .02446,   2.89079,    .02091,
     1   2.93259,    .01739,   2.97439,    .01389,   3.01619,
     1    .01040,   3.05799,    .00693,   3.09979,    .00346,
     1   3.14159,   0.00000/
      DO 100 J= 1, 251
      PHILD(J) = A(2*J-1)
      EPSLD(J) = A(2*J)
  100 CONTINUE
      DO 201 J=1,201
      PHI(J)=FLOAT(J-1)*3.141592654/200.
  201 CALL LCSFIT (251, PHILD, EPSLD, .TRUE., 'B', 201, PHI, EPS, EPS)
      RETURN
      END
      SUBROUTINE PHEP65(PHI,EPS)
      DIMENSION PHI(1),EPS(1)
      DIMENSION PHILD(251),EPSLD(251), A(502)
      DATA (A(I), I = 1, 50) /
     1   0.00000,   0.00000,    .01484,    .00156,    .02967,
     1    .00312,    .04451,    .00466,    .05935,    .00618,
     1    .07420,    .00767,    .08904,    .00913,    .10389,
     1    .01054,    .11875,    .01191,    .13361,    .01322,
     1    .14848,    .01446,    .15175,    .01473,    .15503,
     1    .01499,    .15830,    .01524,    .16157,    .01549,
     1    .16485,    .01572,    .16812,    .01595,    .17140,
     1    .01616,    .17468,    .01636,    .17795,    .01655,
     1    .18123,    .01671,    .18623,    .01693,    .19123,
     1    .01712,    .19623,    .01727,    .20123,    .01740/
      DATA (A(I), I = 51,100) /
     1    .20623,    .01750,    .21124,    .01759,    .21624,
     1    .01766,    .22124,    .01773,    .22625,    .01780,
     1    .23125,    .01788,    .24072,    .01804,    .25018,
     1    .01822,    .25964,    .01842,    .26910,    .01863,
     1    .27856,    .01884,    .28802,    .01903,    .29748,
     1    .01920,    .30694,    .01934,    .31640,    .01945,
     1    .32586,    .01950,    .33918,    .01949,    .35250,
     1    .01938,    .36582,    .01919,    .37915,    .01895,
     1    .39247,    .01866,    .40580,    .01836,    .41912,
     1    .01805,    .43245,    .01775,    .44577,    .01749/
      DATA (A(I), I = 101,150) /
     1    .45910,    .01728,    .46962,    .01716,    .48015,
     1    .01708,    .49068,    .01703,    .50120,    .01702,
     1    .51173,    .01704,    .52226,    .01708,    .53278,
     1    .01714,    .54331,    .01722,    .55384,    .01731,
     1    .56436,    .01741,    .57344,    .01751,    .58252,
     1    .01760,    .59160,    .01771,    .60068,    .01782,
     1    .60976,    .01794,    .61884,    .01806,    .62792,
     1    .01820,    .63700,    .01834,    .64607,    .01850,
     1    .65515,    .01867,    .67086,    .01898,    .68656,
     1    .01933,    .70226,    .01971,    .71797,    .02012/
      DATA (A(I), I = 151,200)/
     1    .73367,    .02055,    .74937,    .02100,    .76508,
     1    .02147,    .78078,    .02196,    .79648,    .02246,
     1    .81218,    .02298,    .82590,    .02343,    .83962,
     1    .02390,    .85333,    .02437,    .86705,    .02485,
     1    .88077,    .02534,    .89448,    .02584,    .90820,
     1    .02636,    .92192,    .02689,    .93563,    .02744,
     1    .94934,    .02800,    .96192,    .02853,    .97449,
     1    .02907,    .98706,    .02963,    .99964,    .03020,
     1   1.01221,    .03078,   1.02478,    .03137,   1.03735,
     1    .03198,   1.04992,    .03259,   1.06249,    .03321/
      DATA (A(I), I = 201,250) /
     1   1.07506,    .03385,   1.08689,    .03445,   1.09872,
     1    .03506,   1.11055,    .03568,   1.12238,    .03631,
     1   1.13421,    .03694,   1.14603,    .03759,   1.15786,
     1    .03824,   1.16969,    .03890,   1.18151,    .03957,
     1   1.19334,    .04025,   1.20469,    .04091,   1.21605,
     1    .04158,   1.22740,    .04225,   1.23876,    .04294,
     1   1.25011,    .04363,   1.26146,    .04434,   1.27281,
     1    .04505,   1.28416,    .04577,   1.29551,    .04650,
     1   1.30686,    .04724,   1.31793,    .04798,   1.32900,
     1    .04872,   1.34007,    .04947,   1.35114,    .05023/
      DATA (A(I), I = 251, 300) /
     1   1.36221,    .05099,   1.37328,    .05177,   1.38434,
     1    .05255,   1.39541,    .05334,   1.40648,    .05414,
     1   1.41754,    .05495,   1.42845,    .05575,   1.43935,
     1    .05655,   1.45025,    .05737,   1.46116,    .05819,
     1   1.47206,    .05901,   1.48296,    .05984,   1.49387,
     1    .06067,   1.50477,    .06150,   1.51567,    .06233,
     1   1.52657,    .06316,   1.53737,    .06399,   1.54816,
     1    .06481,   1.55896,    .06563,   1.56975,    .06645,
     1   1.58055,    .06727,   1.59134,    .06808,   1.60214,
     1    .06889,   1.61293,    .06969,   1.62373,    .07049/
      DATA (A(I), I = 301,350) /
     1   1.63452,    .07127,   1.64525,    .07205,   1.65598,
     1    .07282,   1.66671,    .07358,   1.67744,    .07433,
     1   1.68817,    .07507,   1.69890,    .07580,   1.70963,
     1    .07651,   1.72036,    .07722,   1.73109,    .07792,
     1   1.74182,    .07861,   1.75254,    .07928,   1.76325,
     1    .07994,   1.77396,    .08058,   1.78468,    .08121,
     1   1.79539,    .08182,   1.80611,    .08242,   1.81683,
     1    .08299,   1.82754,    .08355,   1.83826,    .08408,
     1   1.84898,    .08459,   1.85972,    .08508,   1.87046,
     1    .08555,   1.88120,    .08599,   1.89194,    .08641/
      DATA (A(I), I = 351,400) /
     1   1.90268,    .08680,   1.91343,    .08718,   1.92417,
     1    .08754,   1.93491,    .08787,   1.94566,    .08819,
     1   1.95640,    .08848,   1.96732,    .08876,   1.97823,
     1    .08903,   1.98915,    .08927,   2.00006,    .08949,
     1   2.01098,    .08969,   2.02190,    .08988,   2.03282,
     1    .09004,   2.04373,    .09019,   2.05465,    .09031,
     1   2.06557,    .09041,   2.07682,    .09050,   2.08806,
     1    .09056,   2.09931,    .09061,   2.11056,    .09062,
     1   2.12180,    .09061,   2.13305,    .09058,   2.14429,
     1    .09052,   2.15554,    .09043,   2.16679,    .09032/
      DATA (A(I), I=401,450)/
     1   2.17803,    .09017,   2.18979,    .08999,   2.20155,
     1    .08977,   2.21330,    .08953,   2.22506,    .08925,
     1   2.23681,    .08894,   2.24856,    .08861,   2.26032,
     1    .08824,   2.27207,    .08785,   2.28382,    .08743,
     1   2.29557,    .08698,   2.30822,    .08647,   2.32088,
     1    .08593,   2.33353,    .08536,   2.34618,    .08477,
     1   2.35883,    .08414,   2.37148,    .08348,   2.38412,
     1    .08279,   2.39677,    .08208,   2.40942,    .08134,
     1   2.42206,    .08056,   2.43618,    .07967,   2.45029,
     1    .07872,   2.46440,    .07772,   2.47851,    .07666/
      DATA (A(I), I=451,502) /
     1   2.49260,    .07552,   2.50669,    .07429,   2.52077,
     1    .07297,   2.53483,    .07155,   2.54888,    .07000,
     1   2.56292,    .06833,   2.57978,    .06615,   2.59661,
     1    .06380,   2.61343,    .06132,   2.63024,    .05873,
     1   2.64704,    .05606,   2.66383,    .05334,   2.68062,
     1    .05060,   2.69740,    .04788,   2.71420,    .04519,
     1   2.73100,    .04258,   2.77185,    .03664,   2.81276,
     1    .03122,   2.85373,    .02628,   2.89475,    .02176,
     1   2.93582,    .01759,   2.97693,    .01371,   3.01807,
     1    .01008,   3.05923,    .00662,   3.10041,    .00328,
     1   3.14159,   0.00000/
      DO 100 J= 1, 251
      PHILD(J) = A(2*J-1)
      EPSLD(J) = A(2*J)
  100 CONTINUE
      DO 201 J=1,201
      PHI(J)=FLOAT(J-1)*3.141592654/200.
  201 CALL LCSFIT (251, PHILD, EPSLD, .TRUE., 'B', 201, PHI, EPS, EPS)
      RETURN
      END
      SUBROUTINE PHEP65A(PHI,EPS)
      DIMENSION PHI(1),EPS(1)
      DIMENSION PHILD(251),EPSLD(251), A(502)
      DATA (A(I), I = 1, 50) /
     1   0.00000,   0.00000,    .01453,    .00137,    .02907,
     1    .00274,    .04360,    .00410,    .05814,    .00545,
     1    .07267,    .00679,    .08721,    .00811,    .10175,
     1    .00941,    .11630,    .01070,    .13084,    .01195,
     1    .14539,    .01318,    .14877,    .01346,    .15214,
     1    .01374,    .15552,    .01401,    .15889,    .01427,
     1    .16227,    .01453,    .16564,    .01477,    .16902,
     1    .01500,    .17240,    .01521,    .17578,    .01541,
     1    .17916,    .01559,    .18436,    .01583,    .18956,
     1    .01602,    .19477,    .01618,    .19997,    .01630/
      DATA (A(I), I = 51,100) /
     1    .20518,    .01640,    .21039,    .01648,    .21559,
     1    .01654,    .22080,    .01660,    .22601,    .01665,
     1    .23122,    .01671,    .24077,    .01683,    .25032,
     1    .01698,    .25987,    .01715,    .26941,    .01732,
     1    .27896,    .01749,    .28851,    .01766,    .29805,
     1    .01782,    .30760,    .01796,    .31714,    .01807,
     1    .32669,    .01814,    .33996,    .01819,    .35322,
     1    .01816,    .36649,    .01807,    .37976,    .01793,
     1    .39302,    .01775,    .40629,    .01755,    .41956,
     1    .01732,    .43283,    .01708,    .44610,    .01684/
      DATA (A(I), I = 101,150) /
     1    .45937,    .01661,    .46976,    .01644,    .48015,
     1    .01628,    .49054,    .01614,    .50093,    .01601,
     1    .51132,    .01589,    .52171,    .01578,    .53210,
     1    .01569,    .54249,    .01561,    .55288,    .01554,
     1    .56327,    .01549,    .57223,    .01546,    .58118,
     1    .01543,    .59014,    .01541,    .59909,    .01541,
     1    .60805,    .01541,    .61700,    .01542,    .62596,
     1    .01544,    .63491,    .01547,    .64387,    .01551,
     1    .65282,    .01555,    .66829,    .01565,    .68376,
     1    .01577,    .69923,    .01590,    .71470,    .01607/
      DATA (A(I), I = 151,200)/
     1    .73017,    .01625,    .74564,    .01645,    .76110,
     1    .01668,    .77657,    .01693,    .79204,    .01720,
     1    .80750,    .01750,    .82106,    .01778,    .83461,
     1    .01807,    .84816,    .01837,    .86172,    .01869,
     1    .87527,    .01902,    .88882,    .01936,    .90237,
     1    .01970,    .91593,    .02005,    .92948,    .02040,
     1    .94303,    .02076,    .95538,    .02108,    .96772,
     1    .02141,    .98007,    .02174,    .99242,    .02207,
     1   1.00476,    .02240,   1.01711,    .02274,   1.02945,
     1    .02309,   1.04180,    .02344,   1.05415,    .02381/
      DATA (A(I), I = 201,250) /
     1   1.06649,    .02418,   1.07810,    .02454,   1.08971,
     1    .02491,   1.10132,    .02528,   1.11293,    .02567,
     1   1.12454,    .02607,   1.13615,    .02647,   1.14776,
     1    .02689,   1.15937,    .02731,   1.17098,    .02775,
     1   1.18259,    .02819,   1.19373,    .02863,   1.20486,
     1    .02907,   1.21600,    .02952,   1.22713,    .02998,
     1   1.23827,    .03044,   1.24940,    .03091,   1.26054,
     1    .03138,   1.27167,    .03185,   1.28281,    .03232,
     1   1.29394,    .03280,   1.30473,    .03326,   1.31552,
     1    .03371,   1.32631,    .03417,   1.33711,    .03463/
      DATA (A(I), I = 251, 300) /
     1   1.34790,    .03510,   1.35869,    .03557,   1.36948,
     1    .03604,   1.38027,    .03652,   1.39106,    .03701,
     1   1.40185,    .03750,   1.41248,    .03800,   1.42312,
     1    .03851,   1.43375,    .03902,   1.44438,    .03954,
     1   1.45501,    .04007,   1.46564,    .04060,   1.47628,
     1    .04114,   1.48691,    .04169,   1.49754,    .04224,
     1   1.50817,    .04279,   1.51872,    .04335,   1.52927,
     1    .04391,   1.53983,    .04447,   1.55038,    .04502,
     1   1.56093,    .04558,   1.57148,    .04614,   1.58203,
     1    .04669,   1.59258,    .04723,   1.60314,    .04777/
      DATA (A(I), I = 301,350) /
     1   1.61369,    .04830,   1.62418,    .04882,   1.63467,
     1    .04933,   1.64516,    .04984,   1.65565,    .05033,
     1   1.66614,    .05082,   1.67663,    .05130,   1.68712,
     1    .05177,   1.69762,    .05224,   1.70811,    .05270,
     1   1.71860,    .05316,   1.72914,    .05361,   1.73967,
     1    .05406,   1.75020,    .05449,   1.76074,    .05493,
     1   1.77127,    .05535,   1.78181,    .05577,   1.79234,
     1    .05618,   1.80288,    .05657,   1.81341,    .05696,
     1   1.82395,    .05734,   1.83460,    .05771,   1.84526,
     1    .05807,   1.85591,    .05842,   1.86657,    .05876/
      DATA (A(I), I = 351,400) /
     1   1.87722,    .05908,   1.88788,    .05939,   1.89853,
     1    .05969,   1.90919,    .05997,   1.91985,    .06023,
     1   1.93050,    .06049,   1.94138,    .06073,   1.95225,
     1    .06095,   1.96313,    .06116,   1.97401,    .06135,
     1   1.98488,    .06153,   1.99576,    .06170,   2.00664,
     1    .06185,   2.01751,    .06199,   2.02839,    .06212,
     1   2.03927,    .06224,   2.04786,    .06232,   2.05645,
     1    .06240,   2.06504,    .06247,   2.07363,    .06254,
     1   2.08222,    .06259,   2.09081,    .06264,   2.09940,
     1    .06268,   2.10799,    .06272,   2.11658,    .06274/
      DATA (A(I), I=401,450)/
     1   2.12517,    .06276,   2.13979,    .06277,   2.15441,
     1    .06275,   2.16903,    .06271,   2.18365,    .06264,
     1   2.19826,    .06255,   2.21288,    .06242,   2.22750,
     1    .06226,   2.24211,    .06207,   2.25673,    .06185,
     1   2.27134,    .06159,   2.28424,    .06134,   2.29714,
     1    .06106,   2.31003,    .06075,   2.32292,    .06042,
     1   2.33582,    .06007,   2.34871,    .05970,   2.36160,
     1    .05932,   2.37449,    .05891,   2.38739,    .05850,
     1   2.40028,    .05807,   2.41490,    .05756,   2.42952,
     1    .05704,   2.44413,    .05650,   2.45875,    .05592/
      DATA (A(I), I=451,502) /
     1   2.47336,    .05531,   2.48798,    .05466,   2.50259,
     1    .05397,   2.51719,    .05323,   2.53180,    .05244,
     1   2.54640,    .05159,   2.56435,    .05046,   2.58229,
     1    .04925,   2.60023,    .04795,   2.61816,    .04659,
     1   2.63610,    .04517,   2.65402,    .04370,   2.67195,
     1    .04220,   2.68987,    .04065,   2.70779,    .03909,
     1   2.72571,    .03751,   2.76731,    .03383,   2.80891,
     1    .03013,   2.85050,    .02640,   2.89209,    .02266,
     1   2.93368,    .01891,   2.97526,    .01514,   3.01684,
     1    .01136,   3.05843,    .00758,   3.10001,    .00379,
     1   3.14159,   0.00000/
      DO 100 J = 1, 251
      PHILD(J) = A(2*J-1)
      EPSLD(J) = A(2*J)
  100 CONTINUE
      DO 201 J=1,201
      PHI(J)=FLOAT(J-1)*3.141592654/200.
  201 CALL LCSFIT (251, PHILD, EPSLD, .TRUE., 'B', 201, PHI, EPS, EPS)
      RETURN
      END
      SUBROUTINE PHEP66(PHI,EPS)
      DIMENSION PHI(1),EPS(1)
      DIMENSION A(402)
      DATA (A(I), I = 1, 50) /
     1   0.00000,   0.00000,    .01570,    .00145,    .03139,
     1    .00290,    .04709,    .00433,    .06279,    .00574,
     1    .07849,    .00712,    .09420,    .00847,    .10991,
     1    .00978,    .12563,    .01105,    .14135,    .01225,
     1    .15708,    .01340,    .17277,    .01447,    .18847,
     1    .01547,    .20417,    .01638,    .21987,    .01719,
     1    .23559,    .01789,    .25130,    .01847,    .26701,
     1    .01893,    .28273,    .01924,    .29844,    .01940,
     1    .31416,    .01940,    .32987,    .01924,    .34558,
     1    .01893,    .36129,    .01850,    .37699,    .01799/
      DATA (A(I), I = 51,100) /
     1    .39270,    .01741,    .40841,    .01679,    .42411,
     1    .01616,    .43982,    .01556,    .45553,    .01499,
     1    .47124,    .01450,    .48694,    .01410,    .50265,
     1    .01379,    .51836,    .01356,    .53406,    .01340,
     1    .54977,    .01331,    .56548,    .01327,    .58119,
     1    .01328,    .59690,    .01333,    .61261,    .01340,
     1    .62832,    .01350,    .64403,    .01361,    .65974,
     1    .01373,    .67545,    .01387,    .69116,    .01402,
     1    .70686,    .01419,    .72257,    .01438,    .73828,
     1    .01458,    .75399,    .01480,    .76969,    .01504/
      DATA (A(I), I = 101,150) /
     1    .78540,    .01530,    .80111,    .01558,    .81682,
     1    .01588,    .83253,    .01620,    .84824,    .01654,
     1    .86394,    .01689,    .87965,    .01726,    .89536,
     1    .01765,    .91107,    .01805,    .92677,    .01847,
     1    .94248,    .01890,    .95819,    .01934,    .97390,
     1    .01980,    .98961,    .02026,   1.00532,    .02074,
     1   1.02103,    .02124,   1.03673,    .02174,   1.05244,
     1    .02226,   1.06815,    .02279,   1.08385,    .02334,
     1   1.09956,    .02390,   1.11527,    .02447,   1.13098,
     1    .02506,   1.14669,    .02566,   1.16240,    .02627/
      DATA (A(I), I = 151,200)/
     1   1.17811,    .02690,   1.19381,    .02754,   1.20952,
     1    .02819,   1.22523,    .02885,   1.24093,    .02952,
     1   1.25664,    .03020,   1.27235,    .03089,   1.28806,
     1    .03160,   1.30377,    .03231,   1.31948,    .03304,
     1   1.33519,    .03378,   1.35090,    .03453,   1.36660,
     1    .03530,   1.38231,    .03608,   1.39802,    .03688,
     1   1.41372,    .03770,   1.42943,    .03853,   1.44514,
     1    .03938,   1.46085,    .04025,   1.47656,    .04113,
     1   1.49227,    .04202,   1.50798,    .04293,   1.52368,
     1    .04386,   1.53939,    .04479,   1.55510,    .04574/
      DATA (A(I), I = 201,250) /
     1   1.57080,    .04670,   1.58651,    .04767,   1.60223,
     1    .04866,   1.61794,    .04966,   1.63365,    .05067,
     1   1.64936,    .05171,   1.66507,    .05277,   1.68077,
     1    .05386,   1.69648,    .05498,   1.71218,    .05612,
     1   1.72788,    .05730,   1.74359,    .05851,   1.75931,
     1    .05976,   1.77501,    .06103,   1.79072,    .06231,
     1   1.80643,    .06362,   1.82214,    .06493,   1.83784,
     1    .06625,   1.85355,    .06758,   1.86925,    .06889,
     1   1.88496,    .07020,   1.90066,    .07149,   1.91636,
     1    .07277,   1.93206,    .07402,   1.94776,    .07524/
      DATA (A(I), I = 251, 300) /
     1   1.96347,    .07644,   1.97918,    .07760,   1.99489,
     1    .07872,   2.01060,    .07979,   2.02632,    .08082,
     1   2.04204,    .08180,   2.05773,    .08272,   2.07343,
     1    .08359,   2.08913,    .08440,   2.10484,    .08515,
     1   2.12054,    .08585,   2.13625,    .08649,   2.15196,
     1    .08708,   2.16768,    .08761,   2.18339,    .08808,
     1   2.19911,    .08850,   2.21482,    .08886,   2.23052,
     1    .08916,   2.24623,    .08941,   2.26194,    .08959,
     1   2.27765,    .08972,   2.29336,    .08978,   2.30906,
     1    .08978,   2.32477,    .08972,   2.34048,    .08959/
      DATA (A(I), I = 301,350) /
     1   2.35619,    .08940,   2.37191,    .08914,   2.38762,
     1    .08882,   2.40333,    .08843,   2.41905,    .08797,
     1   2.43476,    .08745,   2.45046,    .08687,   2.46617,
     1    .08622,   2.48187,    .08551,   2.49757,    .08474,
     1   2.51327,    .08390,   2.52900,    .08300,   2.54473,
     1    .08203,   2.56045,    .08101,   2.57616,    .07991,
     1   2.59188,    .07875,   2.60758,    .07752,   2.62328,
     1    .07622,   2.63898,    .07485,   2.65467,    .07341,
     1   2.67035,    .07190,   2.68609,    .07031,   2.70183,
     1    .06864,   2.71756,    .06691,   2.73328,    .06510/
      DATA (A(I), I=351, 402)/
     1   2.74899,    .06323,   2.76469,    .06129,   2.78039,
     1    .05928,   2.79608,    .05722,   2.81176,    .05509,
     1   2.82743,    .05290,   2.84317,    .05064,   2.85891,
     1    .04833,   2.87463,    .04596,   2.89035,    .04354,
     1   2.90606,    .04108,   2.92176,    .03856,   2.93746,
     1    .03601,   2.95315,    .03341,   2.96883,    .03077,
     1   2.98451,    .02810,   3.00023,    .02539,   3.01595,
     1    .02264,   3.03167,    .01987,   3.04738,    .01707,
     1   3.06309,    .01426,   3.07879,    .01143,   3.09449,
     1    .00858,   3.11019,    .00573,   3.12589,    .00287,
     1   3.14159,   0.00000/
      DO 201 J=1,201
      PHI(J) = A(2*J-1)
      EPS(J) = A(2*J)
  201 CONTINUE
      RETURN
      END
      SUBROUTINE PHEP67(PHI,EPS)
      DIMENSION PHI(1),EPS(1)
      DIMENSION PHILD(251),EPSLD(251), A(502)
      DATA (A(I), I = 1, 50) /
     1   0.00000,   0.00000,    .01495,    .00161,    .02990,
     1    .00322,    .04485,    .00482,    .05980,    .00640,
     1    .07476,    .00796,    .08971,    .00949,    .10468,
     1    .01100,    .11964,    .01246,    .13461,    .01388,
     1    .14959,    .01525,    .15280,    .01554,    .15601,
     1    .01582,    .15922,    .01610,    .16243,    .01637,
     1    .16565,    .01664,    .16886,    .01690,    .17207,
     1    .01714,    .17529,    .01738,    .17850,    .01761,
     1    .18172,    .01782,    .18687,    .01813,    .19201,
     1    .01841,    .19716,    .01865,    .20231,    .01888/
      DATA (A(I), I = 51,100) /
     1    .20747,    .01908,    .21262,    .01927,    .21777,
     1    .01945,    .22293,    .01962,    .22808,    .01979,
     1    .23323,    .01996,    .24276,    .02029,    .25229,
     1    .02063,    .26182,    .02097,    .27135,    .02131,
     1    .28088,    .02164,    .29041,    .02195,    .29994,
     1    .02224,    .30947,    .02249,    .31900,    .02269,
     1    .32853,    .02285,    .34180,    .02298,    .35507,
     1    .02301,    .36834,    .02296,    .38162,    .02284,
     1    .39489,    .02267,    .40817,    .02245,    .42144,
     1    .02220,    .43472,    .02194,    .44799,    .02168/
      DATA (A(I), I = 101,150) /
     1    .46127,    .02143,    .47164,    .02125,    .48201,
     1    .02108,    .49238,    .02093,    .50274,    .02079,
     1    .51311,    .02067,    .52348,    .02055,    .53385,
     1    .02045,    .54422,    .02035,    .55458,    .02026,
     1    .56495,    .02017,    .57385,    .02010,    .58275,
     1    .02004,    .59165,    .01998,    .60055,    .01992,
     1    .60945,    .01987,    .61835,    .01983,    .62725,
     1    .01979,    .63615,    .01975,    .64505,    .01972,
     1    .65395,    .01970,    .66928,    .01967,    .68461,
     1    .01966,    .69994,    .01967,    .71527,    .01969/
      DATA (A(I), I = 151,200)/
     1    .73060,    .01973,    .74593,    .01978,    .76126,
     1    .01985,    .77658,    .01992,    .79191,    .02001,
     1    .80724,    .02010,    .82061,    .02019,    .83397,
     1    .02029,    .84734,    .02039,    .86070,    .02051,
     1    .87407,    .02062,    .88743,    .02075,    .90080,
     1    .02088,    .91416,    .02102,    .92753,    .02117,
     1    .94089,    .02133,    .95309,    .02148,    .96530,
     1    .02164,    .97750,    .02180,    .98970,    .02198,
     1   1.00190,    .02216,   1.01410,    .02234,   1.02630,
     1    .02254,   1.03850,    .02274,   1.05070,    .02295/
      DATA (A(I), I = 201,250) /
     1   1.06289,    .02316,   1.07435,    .02337,   1.08580,
     1    .02359,   1.09726,    .02381,   1.10871,    .02404,
     1   1.12017,    .02427,   1.13162,    .02451,   1.14308,
     1    .02476,   1.15453,    .02501,   1.16599,    .02527,
     1   1.17744,    .02553,   1.18840,    .02579,   1.19936,
     1    .02606,   1.21033,    .02632,   1.22129,    .02660,
     1   1.23225,    .02688,   1.24321,    .02716,   1.25417,
     1    .02745,   1.26513,    .02775,   1.27609,    .02805,
     1   1.28706,    .02836,   1.29770,    .02866,   1.30835,
     1    .02897,   1.31900,    .02928,   1.32965,    .02960/
      DATA (A(I), I = 251, 300) /
     1   1.34030,    .02992,   1.35094,    .03025,   1.36159,
     1    .03059,   1.37224,    .03093,   1.38289,    .03128,
     1   1.39353,    .03163,   1.40401,    .03199,   1.41450,
     1    .03235,   1.42498,    .03271,   1.43546,    .03309,
     1   1.44594,    .03346,   1.45642,    .03385,   1.46690,
     1    .03424,   1.47738,    .03464,   1.48786,    .03505,
     1   1.49834,    .03546,   1.50878,    .03588,   1.51921,
     1    .03630,   1.52965,    .03673,   1.54008,    .03717,
     1   1.55052,    .03762,   1.56095,    .03807,   1.57139,
     1    .03852,   1.58182,    .03899,   1.59226,    .03945/
      DATA (A(I), I = 301,350) /
     1   1.60269,    .03993,   1.61319,    .04041,   1.62369,
     1    .04090,   1.63420,    .04139,   1.64470,    .04190,
     1   1.65520,    .04241,   1.66570,    .04293,   1.67620,
     1    .04346,   1.68670,    .04400,   1.69720,    .04455,
     1   1.70770,    .04511,   1.71841,    .04570,   1.72913,
     1    .04629,   1.73984,    .04690,   1.75056,    .04752,
     1   1.76127,    .04815,   1.77199,    .04880,   1.78270,
     1    .04945,   1.79341,    .05011,   1.80413,    .05078,
     1   1.81484,    .05145,   1.82589,    .05216,   1.83694,
     1    .05288,   1.84799,    .05360,   1.85904,    .05434/
      DATA (A(I), I = 351,400) /
     1   1.87009,    .05509,   1.88113,    .05584,   1.89218,
     1    .05661,   1.90323,    .05739,   1.91427,    .05818,
     1   1.92531,    .05899,   1.93688,    .05985,   1.94844,
     1    .06072,   1.95999,    .06160,   1.97155,    .06250,
     1   1.98311,    .06340,   1.99467,    .06432,   2.00622,
     1    .06524,   2.01778,    .06617,   2.02933,    .06711,
     1   2.04089,    .06805,   2.05305,    .06905,   2.06522,
     1    .07005,   2.07739,    .07105,   2.08956,    .07204,
     1   2.10172,    .07302,   2.11389,    .07398,   2.12606,
     1    .07493,   2.13824,    .07586,   2.15041,    .07677/
      DATA (A(I), I=401,450)/
     1   2.16259,    .07765,   2.17534,    .07854,   2.18810,
     1    .07939,   2.20085,    .08020,   2.21361,    .08097,
     1   2.22637,    .08171,   2.23914,    .08240,   2.25190,
     1    .08305,   2.26467,    .08365,   2.27744,    .08421,
     1   2.29021,    .08471,   2.30376,    .08520,   2.31733,
     1    .08563,   2.33089,    .08601,   2.34445,    .08634,
     1   2.35802,    .08662,   2.37159,    .08685,   2.38516,
     1    .08703,   2.39874,    .08716,   2.41232,    .08726,
     1   2.42590,    .08731,   2.44078,    .08731,   2.45566,
     1    .08725,   2.47054,    .08710,   2.48542,    .08684/
      DATA (A(I), I=451,502) /
     1   2.50029,    .08646,   2.51515,    .08594,   2.53000,
     1    .08525,   2.54483,    .08439,   2.55965,    .08333,
     1   2.57444,    .08205,   2.59147,    .08029,   2.60847,
     1    .07826,   2.62544,    .07599,   2.64239,    .07351,
     1   2.65933,    .07088,   2.67625,    .06811,   2.69317,
     1    .06526,   2.71007,    .06235,   2.72698,    .05943,
     1   2.74389,    .05653,   2.78356,    .04995,   2.82326,
     1    .04366,   2.86298,    .03763,   2.90274,    .03183,
     1   2.94251,    .02623,   2.98231,    .02079,   3.02211,
     1    .01548,   3.06193,    .01026,   3.10176,    .00511,
     1   3.14159,   0.00000/
      DO 100 J = 1, 251
      PHILD(J) = A(2*J-1)
      EPSLD(J) = A(2*J)
  100 CONTINUE
      DO 201 J=1,201
      PHI(J)=FLOAT(J-1)*3.141592654/200.
  201 CALL LCSFIT (251, PHILD, EPSLD, .TRUE., 'B', 201, PHI, EPS, EPS)
      RETURN
      END
      SUBROUTINE PHPS63(PHI,PSI)
      DIMENSION PHI(1),PSI(1)
      DIMENSION PHILD(201),PSILD(201), A(402)
      DATA (A(I), I = 1, 50) /
     1   0.00000,    .15066,    .01571,    .15058,    .03142,
     1    .15035,    .04713,    .14999,    .06284,    .14950,
     1    .07855,    .14891,    .09426,    .14823,    .10997,
     1    .14748,    .12567,    .14668,    .14138,    .14583,
     1    .15708,    .14497,    .17279,    .14410,    .18850,
     1    .14323,    .20420,    .14238,    .21991,    .14155,
     1    .23561,    .14074,    .25132,    .13998,    .26703,
     1    .13927,    .28274,    .13862,    .29845,    .13804,
     1    .31416,    .13753,    .32986,    .13711,    .34556,
     1    .13676,    .36127,    .13648,    .37698,    .13627/
      DATA (A(I), I = 51,100) /
     1    .39268,    .13610,    .40839,    .13598,    .42411,
     1    .13590,    .43982,    .13584,    .45553,    .13579,
     1    .47124,    .13576,    .48695,    .13573,    .50266,
     1    .13570,    .51837,    .13567,    .53408,    .13564,
     1    .54978,    .13561,    .56549,    .13558,    .58120,
     1    .13555,    .59691,    .13552,    .61261,    .13550,
     1    .62832,    .13547,    .64403,    .13544,    .65974,
     1    .13542,    .67544,    .13539,    .69115,    .13536,
     1    .70686,    .13533,    .72257,    .13529,    .73828,
     1    .13525,    .75398,    .13521,    .76969,    .13516/
      DATA (A(I), I = 101,150) /
     1    .78540,    .13511,    .80111,    .13505,    .81682,
     1    .13499,    .83252,    .13491,    .84823,    .13483,
     1    .86394,    .13475,    .87965,    .13465,    .89536,
     1    .13454,    .91106,    .13442,    .92677,    .13428,
     1    .94248,    .13414,    .95819,    .13398,    .97390,
     1    .13381,    .98961,    .13363,   1.00531,    .13343,
     1   1.02102,    .13321,   1.03673,    .13299,   1.05244,
     1    .13275,   1.06815,    .13249,   1.08385,    .13222,
     1   1.09956,    .13194,   1.11527,    .13164,   1.13098,
     1    .13133,   1.14669,    .13100,   1.16240,    .13065/
      DATA (A(I), I = 151,200)/
     1   1.17811,    .13028,   1.19381,    .12988,   1.20952,
     1    .12947,   1.22523,    .12903,   1.24093,    .12857,
     1   1.25664,    .12808,   1.27235,    .12756,   1.28807,
     1    .12702,   1.30378,    .12644,   1.31949,    .12584,
     1   1.33520,    .12521,   1.35090,    .12455,   1.36661,
     1    .12385,   1.38231,    .12313,   1.39802,    .12238,
     1   1.41372,    .12160,   1.42944,    .12079,   1.44515,
     1    .11994,   1.46086,    .11907,   1.47657,    .11817,
     1   1.49228,    .11724,   1.50799,    .11628,   1.52369,
     1    .11529,   1.53940,    .11428,   1.55510,    .11324/
      DATA (A(I), I = 201,250) /
     1   1.57080,    .11218,   1.58652,    .11109,   1.60223,
     1    .10998,   1.61794,    .10884,   1.63365,    .10768,
     1   1.64936,    .10650,   1.66507,    .10530,   1.68077,
     1    .10407,   1.69648,    .10283,   1.71218,    .10157,
     1   1.72788,    .10029,   1.74359,    .09899,   1.75931,
     1    .09767,   1.77502,    .09634,   1.79073,    .09499,
     1   1.80644,    .09363,   1.82214,    .09224,   1.83785,
     1    .09085,   1.85355,    .08944,   1.86926,    .08801,
     1   1.88496,    .08657,   1.90067,    .08512,   1.91638,
     1    .08365,   1.93209,    .08217,   1.94780,    .08068/
      DATA (A(I), I = 251, 300) /
     1   1.96351,    .07917,   1.97922,    .07766,   1.99493,
     1    .07614,   2.01063,    .07461,   2.02634,    .07307,
     1   2.04204,    .07153,   2.05775,    .06998,   2.07346,
     1    .06842,   2.08916,    .06687,   2.10487,    .06530,
     1   2.12058,    .06374,   2.13628,    .06217,   2.15199,
     1    .06060,   2.16770,    .05904,   2.18340,    .05747,
     1   2.19911,    .05591,   2.21481,    .05435,   2.23052,
     1    .05280,   2.24623,    .05125,   2.26193,    .04970,
     1   2.27764,    .04817,   2.29335,    .04664,   2.30906,
     1    .04512,   2.32477,    .04362,   2.34048,    .04213/
      DATA (A(I), I = 301,350) /
     1   2.35619,    .04065,   2.37189,    .03919,   2.38759,
     1    .03774,   2.40330,    .03631,   2.41900,    .03490,
     1   2.43471,    .03350,   2.45042,    .03213,   2.46613,
     1    .03077,   2.48184,    .02943,   2.49755,    .02811,
     1   2.51327,    .02682,   2.52897,    .02555,   2.54467,
     1    .02430,   2.56037,    .02308,   2.57608,    .02188,
     1   2.59179,    .02071,   2.60749,    .01956,   2.62321,
     1    .01844,   2.63892,    .01735,   2.65463,    .01630,
     1   2.67035,    .01527,   2.68605,    .01428,   2.70175,
     1    .01331,   2.71745,    .01239,   2.73316,    .01149/
      DATA (A(I), I=351, 402)/
     1   2.74887,    .01062,   2.76458,    .00979,   2.78029,
     1    .00899,   2.79600,    .00823,   2.81171,    .00750,
     1   2.82743,    .00680,   2.84313,    .00614,   2.85884,
     1    .00551,   2.87454,    .00491,   2.89025,    .00435,
     1   2.90596,    .00382,   2.92167,    .00332,   2.93737,
     1    .00286,   2.95309,    .00244,   2.96880,    .00205,
     1   2.98451,    .00169,   3.00022,    .00137,   3.01592,
     1    .00108,   3.03163,    .00083,   3.04734,    .00061,
     1   3.06305,    .00042,   3.07875,    .00027,   3.09446,
     1    .00015,   3.11017,    .00007,   3.12588,    .00002,
     1   3.14159,   0.00000/
      DO 100 J=1,201
      PHILD(J) = A(2*J-1)
      PSILD(J) = A(2*J)
  100 CONTINUE
      DO 202 J=1,201
      CALL LCSFIT (201, PHILD, PSILD, .TRUE., 'B', 201, PHI, PSI, PSI)
  202 CONTINUE
      RETURN
      END
      SUBROUTINE PHPS63A(PHI,PSI)
      DIMENSION PHI(1),PSI(1)
      DIMENSION PHILD(251), PSILD(251), A(502)
      DATA (A(I), I = 1, 50) /
     1   0.00000,    .15517,    .01461,    .15507,    .02922,
     1    .15478,    .04383,    .15431,    .05843,    .15368,
     1    .07303,    .15290,    .08763,    .15199,    .10222,
     1    .15097,    .11680,    .14984,    .13138,    .14863,
     1    .14594,    .14735,    .14925,    .14705,    .15257,
     1    .14675,    .15588,    .14645,    .15919,    .14615,
     1    .16250,    .14585,    .16582,    .14555,    .16913,
     1    .14525,    .17244,    .14496,    .17576,    .14468,
     1    .17907,    .14440,    .18420,    .14397,    .18933,
     1    .14357,    .19447,    .14318,    .19960,    .14281/
      DATA (A(I), I = 51,100) /
     1    .20474,    .14245,    .20987,    .14210,    .21501,
     1    .14177,    .22015,    .14145,    .22529,    .14115,
     1    .23043,    .14086,    .24001,    .14035,    .24958,
     1    .13987,    .25916,    .13944,    .26875,    .13904,
     1    .27833,    .13867,    .28791,    .13833,    .29750,
     1    .13802,    .30708,    .13773,    .31667,    .13746,
     1    .32626,    .13722,    .33980,    .13690,    .35335,
     1    .13662,    .36690,    .13637,    .38045,    .13614,
     1    .39401,    .13594,    .40756,    .13577,    .42111,
     1    .13562,    .43466,    .13548,    .44821,    .13537/
      DATA (A(I), I = 101,150) /
     1    .46176,    .13527,    .47241,    .13520,    .48306,
     1    .13514,    .49371,    .13509,    .50436,    .13504,
     1    .51501,    .13500,    .52566,    .13496,    .53631,
     1    .13492,    .54696,    .13489,    .55761,    .13486,
     1    .56826,    .13483,    .57741,    .13481,    .58655,
     1    .13478,    .59570,    .13476,    .60485,    .13474,
     1    .61399,    .13471,    .62314,    .13469,    .63229,
     1    .13467,    .64143,    .13464,    .65058,    .13462,
     1    .65973,    .13460,    .67541,    .13456,    .69110,
     1    .13452,    .70679,    .13447,    .72248,    .13442/
      DATA (A(I), I = 151,200)/
     1    .73817,    .13438,    .75386,    .13432,    .76955,
     1    .13427,    .78524,    .13420,    .80093,    .13414,
     1    .81662,    .13406,    .83032,    .13400,    .84402,
     1    .13392,    .85772,    .13384,    .87142,    .13376,
     1    .88512,    .13367,    .89882,    .13357,    .91253,
     1    .13347,    .92623,    .13336,    .93993,    .13324,
     1    .95363,    .13312,    .96612,    .13299,    .97860,
     1    .13287,    .99109,    .13274,   1.00358,    .13260,
     1   1.01607,    .13245,   1.02855,    .13230,   1.04104,
     1    .13214,   1.05353,    .13197,   1.06602,    .13180/
      DATA (A(I), I = 201,250) /
     1   1.07850,    .13162,   1.09023,    .13145,   1.10196,
     1    .13127,   1.11369,    .13108,   1.12541,    .13089,
     1   1.13714,    .13068,   1.14887,    .13047,   1.16059,
     1    .13024,   1.17232,    .13001,   1.18405,    .12976,
     1   1.19577,    .12950,   1.20699,    .12923,   1.21821,
     1    .12896,   1.22943,    .12867,   1.24064,    .12837,
     1   1.25186,    .12806,   1.26308,    .12774,   1.27429,
     1    .12741,   1.28551,    .12706,   1.29672,    .12670,
     1   1.30794,    .12634,   1.31876,    .12597,   1.32959,
     1    .12560,   1.34041,    .12521,   1.35124,    .12481/
      DATA (A(I), I = 251, 300) /
     1   1.36206,    .12440,   1.37289,    .12398,   1.38371,
     1    .12355,   1.39453,    .12310,   1.40535,    .12264,
     1   1.41617,    .12217,   1.42675,    .12170,   1.43733,
     1    .12121,   1.44790,    .12072,   1.45847,    .12021,
     1   1.46905,    .11969,   1.47962,    .11916,   1.49019,
     1    .11862,   1.50077,    .11807,   1.51134,    .11750,
     1   1.52191,    .11693,   1.53232,    .11636,   1.54274,
     1    .11577,   1.55316,    .11518,   1.56357,    .11458,
     1   1.57399,    .11397,   1.58440,    .11335,   1.59481,
     1    .11272,   1.60523,    .11208,   1.61564,    .11143/
      DATA (A(I), I = 301,350) /
     1   1.62605,    .11078,   1.63637,    .11012,   1.64670,
     1    .10945,   1.65702,    .10878,   1.66734,    .10810,
     1   1.67766,    .10741,   1.68798,    .10671,   1.69830,
     1    .10601,   1.70862,    .10530,   1.71894,    .10458,
     1   1.72926,    .10385,   1.73961,    .10312,   1.74996,
     1    .10238,   1.76031,    .10164,   1.77067,    .10089,
     1   1.78101,    .10013,   1.79136,    .09936,   1.80171,
     1    .09860,   1.81206,    .09782,   1.82241,    .09704,
     1   1.83276,    .09625,   1.84322,    .09545,   1.85369,
     1    .09465,   1.86415,    .09384,   1.87461,    .09302/
      DATA (A(I), I = 351,400) /
     1   1.88508,    .09220,   1.89554,    .09137,   1.90600,
     1    .09054,   1.91646,    .08971,   1.92692,    .08887,
     1   1.93738,    .08802,   1.94807,    .08716,   1.95875,
     1    .08629,   1.96943,    .08541,   1.98012,    .08454,
     1   1.99080,    .08366,   2.00148,    .08278,   2.01216,
     1    .08189,   2.02284,    .08100,   2.03353,    .08012,
     1   2.04421,    .07923,   2.05529,    .07830,   2.06637,
     1    .07738,   2.07744,    .07646,   2.08852,    .07553,
     1   2.09960,    .07461,   2.11068,    .07369,   2.12176,
     1    .07278,   2.13284,    .07186,   2.14392,    .07095/
      DATA (A(I), I=401,450)/
     1   2.15500,    .07005,   2.16671,    .06910,   2.17841,
     1    .06815,   2.19012,    .06721,   2.20183,    .06628,
     1   2.21354,    .06535,   2.22525,    .06443,   2.23696,
     1    .06351,   2.24867,    .06259,   2.26038,    .06168,
     1   2.27209,    .06078,   2.28483,    .05980,   2.29757,
     1    .05882,   2.31031,    .05785,   2.32305,    .05688,
     1   2.33579,    .05593,   2.34854,    .05497,   2.36128,
     1    .05403,   2.37402,    .05309,   2.38676,    .05216,
     1   2.39951,    .05123,   2.41401,    .05019,   2.42851,
     1    .04916,   2.44302,    .04813,   2.45752,    .04711/
      DATA (A(I), I=451,502) /
     1   2.47203,    .04608,   2.48653,    .04506,   2.50103,
     1    .04403,   2.51554,    .04299,   2.53004,    .04195,
     1   2.54455,    .04090,   2.56241,    .03958,   2.58028,
     1    .03826,   2.59814,    .03694,   2.61601,    .03563,
     1   2.63388,    .03435,   2.65175,    .03311,   2.66962,
     1    .03191,   2.68750,    .03077,   2.70537,    .02970,
     1   2.72325,    .02871,   2.76503,    .02675,   2.80682,
     1    .02524,   2.84864,    .02412,   2.89046,    .02333,
     1   2.93230,    .02282,   2.97414,    .02253,   3.01600,
     1    .02240,   3.05786,    .02238,   3.09973,    .02241,
     1   3.14159,    .02243/
      DO 100 J = 1, 251
      PHILD(J) = A(2*J-1)
      PSILD(J) = A(2*J)
  100 CONTINUE
      DO 202 J=1,201
      CALL LCSFIT (251, PHILD, PSILD, .TRUE., 'B', 201, PHI, PSI, PSI)
  202 CONTINUE
      RETURN
      END
      SUBROUTINE PHPS64(PHI,PSI)
      DIMENSION PHI(1),PSI(1)
      DIMENSION PHILD(201),PSILD(201), A(402)
      DATA (A(I), I = 1, 50) /
     1   0.00000,    .25269,    .01573,    .25265,    .03145,
     1    .25251,    .04717,    .25227,    .06289,    .25193,
     1    .07861,    .25147,    .09432,    .25090,    .11002,
     1    .25020,    .12572,    .24937,    .14140,    .24841,
     1    .15708,    .24730,    .17280,    .24605,    .18851,
     1    .24467,    .20422,    .24321,    .21992,    .24170,
     1    .23562,    .24016,    .25132,    .23864,    .26702,
     1    .23715,    .28272,    .23573,    .29844,    .23442,
     1    .31416,    .23325,    .32984,    .23224,    .34553,
     1    .23138,    .36122,    .23066,    .37693,    .23006/
      DATA (A(I), I = 51,100) /
     1    .39264,    .22956,    .40836,    .22916,    .42408,
     1    .22884,    .43980,    .22858,    .45552,    .22836,
     1    .47124,    .22818,    .48695,    .22802,    .50266,
     1    .22788,    .51837,    .22775,    .53408,    .22764,
     1    .54979,    .22755,    .56549,    .22747,    .58120,
     1    .22740,    .59691,    .22736,    .61261,    .22732,
     1    .62832,    .22730,    .64403,    .22729,    .65973,
     1    .22730,    .67544,    .22731,    .69115,    .22733,
     1    .70686,    .22736,    .72257,    .22739,    .73827,
     1    .22742,    .75398,    .22745,    .76969,    .22748/
      DATA (A(I), I = 101,150) /
     1    .78540,    .22751,    .80111,    .22753,    .81682,
     1    .22755,    .83252,    .22756,    .84823,    .22756,
     1    .86394,    .22755,    .87965,    .22753,    .89536,
     1    .22751,    .91106,    .22747,    .92677,    .22742,
     1    .94248,    .22736,    .95819,    .22729,    .97390,
     1    .22720,    .98961,    .22709,   1.00531,    .22697,
     1   1.02102,    .22683,   1.03673,    .22668,   1.05244,
     1    .22650,   1.06815,    .22630,   1.08385,    .22608,
     1   1.09956,    .22584,   1.11527,    .22557,   1.13098,
     1    .22528,   1.14669,    .22497,   1.16240,    .22462/
      DATA (A(I), I = 151,200)/
     1   1.17811,    .22426,   1.19381,    .22386,   1.20952,
     1    .22345,   1.22523,    .22300,   1.24093,    .22253,
     1   1.25664,    .22203,   1.27236,    .22150,   1.28808,
     1    .22094,   1.30379,    .22034,   1.31951,    .21969,
     1   1.33522,    .21899,   1.35093,    .21823,   1.36663,
     1    .21741,   1.38233,    .21652,   1.39803,    .21554,
     1   1.41372,    .21449,   1.42946,    .21334,   1.44519,
     1    .21211,   1.46091,    .21080,   1.47663,    .20940,
     1   1.49234,    .20794,   1.50804,    .20641,   1.52374,
     1    .20481,   1.53943,    .20315,   1.55512,    .20143/
      DATA (A(I), I = 201,250) /
     1   1.57080,    .19966,   1.58653,    .19784,   1.60226,
     1    .19596,   1.61798,    .19405,   1.63370,    .19209,
     1   1.64941,    .19008,   1.66511,    .18804,   1.68081,
     1    .18596,   1.69651,    .18384,   1.71220,    .18169,
     1   1.72788,    .17950,   1.74361,    .17727,   1.75933,
     1    .17501,   1.77505,    .17273,   1.79076,    .17041,
     1   1.80647,    .16806,   1.82218,    .16569,   1.83788,
     1    .16329,   1.85357,    .16087,   1.86927,    .15843,
     1   1.88496,    .15596,   1.90068,    .15347,   1.91640,
     1    .15095,   1.93211,    .14842,   1.94782,    .14587/
      DATA (A(I), I = 251, 300) /
     1   1.96353,    .14330,   1.97924,    .14072,   1.99494,
     1    .13812,   2.01064,    .13551,   2.02634,    .13288,
     1   2.04204,    .13024,   2.05775,    .12759,   2.07346,
     1    .12492,   2.08917,    .12225,   2.10488,    .11957,
     1   2.12059,    .11688,   2.13629,    .11418,   2.15200,
     1    .11149,   2.16770,    .10878,   2.18341,    .10608,
     1   2.19911,    .10338,   2.21481,    .10068,   2.23051,
     1    .09798,   2.24622,    .09529,   2.26192,    .09260,
     1   2.27763,    .08992,   2.29334,    .08725,   2.30905,
     1    .08459,   2.32476,    .08195,   2.34047,    .07932/
      DATA (A(I), I = 301,350) /
     1   2.35619,    .07671,   2.37188,    .07412,   2.38757,
     1    .07156,   2.40327,    .06901,   2.41897,    .06649,
     1   2.43468,    .06399,   2.45039,    .06152,   2.46610,
     1    .05908,   2.48182,    .05667,   2.49754,    .05428,
     1   2.51327,    .05193,   2.52895,    .04962,   2.54464,
     1    .04734,   2.56034,    .04510,   2.57604,    .04289,
     1   2.59174,    .04073,   2.60745,    .03860,   2.62317,
     1    .03651,   2.63889,    .03447,   2.65462,    .03247,
     1   2.67035,    .03051,   2.68603,    .02861,   2.70172,
     1    .02675,   2.71741,    .02494,   2.73311,    .02318/
      DATA (A(I), I=351, 402)/
     1   2.74882,    .02148,   2.76453,    .01983,   2.78024,
     1    .01825,   2.79597,    .01672,   2.81170,    .01525,
     1   2.82743,    .01385,   2.84312,    .01252,   2.85881,
     1    .01125,   2.87451,    .01006,   2.89021,    .00892,
     1   2.90591,    .00786,   2.92163,    .00686,   2.93734,
     1    .00593,   2.95306,    .00506,   2.96878,    .00426,
     1   2.98451,    .00353,   3.00021,    .00287,   3.01591,
     1    .00227,   3.03162,    .00174,   3.04732,    .00128,
     1   3.06303,    .00089,   3.07874,    .00057,   3.09445,
     1    .00032,   3.11016,    .00014,   3.12588,    .00004,
     1   3.14159,   0.00000/
      DO 100 J= 1, 201
      PHILD(J) = A(2*J-1)
      PSILD(J) = A(2*J)
  100 CONTINUE
      DO 202 J=1,201
      CALL LCSFIT (201, PHILD, PSILD, .TRUE., 'B', 201, PHI, PSI, PSI)
  202 CONTINUE
      RETURN
      END
      SUBROUTINE PHPS64A(PHI,PSI)
      DIMENSION PHI(1), PSI(1)
      DIMENSION PHILD(251),PSILD(251), A(502)
      DATA (A(I), I = 1, 50) /
     1   0.00000,    .15365,    .01470,    .15355,    .02940,
     1    .15324,    .04410,    .15275,    .05880,    .15209,
     1    .07349,    .15129,    .08818,    .15035,    .10286,
     1    .14930,    .11753,    .14816,    .13220,    .14693,
     1    .14686,    .14565,    .15019,    .14535,    .15352,
     1    .14505,    .15685,    .14475,    .16019,    .14444,
     1    .16352,    .14414,    .16685,    .14384,    .17018,
     1    .14353,    .17351,    .14323,    .17684,    .14293,
     1    .18017,    .14263,    .18528,    .14218,    .19040,
     1    .14174,    .19551,    .14130,    .20063,    .14087/
      DATA (A(I), I = 51,100) /
     1    .20574,    .14044,    .21086,    .14003,    .21598,
     1    .13963,    .22109,    .13924,    .22621,    .13886,
     1    .23133,    .13849,    .24088,    .13783,    .25044,
     1    .13722,    .26000,    .13665,    .26956,    .13612,
     1    .27912,    .13562,    .28869,    .13516,    .29825,
     1    .13473,    .30782,    .13432,    .31739,    .13395,
     1    .32695,    .13359,    .34039,    .13313,    .35384,
     1    .13271,    .36728,    .13233,    .38072,    .13199,
     1    .39416,    .13167,    .40761,    .13139,    .42105,
     1    .13113,    .43450,    .13090,    .44794,    .13069/
      DATA (A(I), I = 101,150) /
     1    .46139,    .13051,    .47194,    .13037,    .48249,
     1    .13024,    .49303,    .13013,    .50358,    .13003,
     1    .51413,    .12993,    .52468,    .12985,    .53523,
     1    .12977,    .54578,    .12971,    .55633,    .12966,
     1    .56688,    .12961,    .57595,    .12958,    .58502,
     1    .12955,    .59409,    .12953,    .60317,    .12952,
     1    .61224,    .12951,    .62131,    .12950,    .63038,
     1    .12949,    .63945,    .12949,    .64852,    .12949,
     1    .65759,    .12949,    .67316,    .12948,    .68874,
     1    .12947,    .70431,    .12947,    .71989,    .12946/
      DATA (A(I), I = 151,200)/
     1    .73546,    .12945,    .75103,    .12944,    .76661,
     1    .12943,    .78218,    .12942,    .79776,    .12941,
     1    .81333,    .12940,    .82695,    .12939,    .84057,
     1    .12939,    .85418,    .12938,    .86780,    .12937,
     1    .88142,    .12936,    .89504,    .12934,    .90866,
     1    .12932,    .92228,    .12930,    .93589,    .12927,
     1    .94951,    .12923,    .96194,    .12920,    .97436,
     1    .12915,    .98679,    .12910,    .99922,    .12905,
     1   1.01164,    .12899,   1.02407,    .12892,   1.03650,
     1    .12885,   1.04892,    .12878,   1.06135,    .12869/
      DATA (A(I), I = 201,250) /
     1   1.07377,    .12861,   1.08546,    .12852,   1.09714,
     1    .12843,   1.10882,    .12833,   1.12050,    .12822,
     1   1.13218,    .12811,   1.14386,    .12800,   1.15554,
     1    .12788,   1.16723,    .12775,   1.17891,    .12761,
     1   1.19059,    .12747,   1.20180,    .12733,   1.21301,
     1    .12718,   1.22422,    .12702,   1.23543,    .12685,
     1   1.24665,    .12668,   1.25786,    .12650,   1.26907,
     1    .12631,   1.28028,    .12611,   1.29149,    .12590,
     1   1.30270,    .12569,   1.31357,    .12547,   1.32444,
     1    .12524,   1.33531,    .12500,   1.34618,    .12475/
      DATA (A(I), I = 251, 300) /
     1   1.35705,    .12448,   1.36792,    .12420,   1.37879,
     1    .12390,   1.38966,    .12358,   1.40052,    .12325,
     1   1.41139,    .12289,   1.42204,    .12252,   1.43269,
     1    .12212,   1.44334,    .12171,   1.45398,    .12128,
     1   1.46463,    .12083,   1.47528,    .12036,   1.48592,
     1    .11988,   1.49657,    .11938,   1.50721,    .11887,
     1   1.51785,    .11834,   1.52835,    .11781,   1.53884,
     1    .11727,   1.54933,    .11672,   1.55982,    .11615,
     1   1.57030,    .11557,   1.58079,    .11499,   1.59128,
     1    .11439,   1.60177,    .11378,   1.61225,    .11316/
      DATA (A(I), I = 301,350) /
     1   1.62274,    .11253,   1.63312,    .11189,   1.64350,
     1    .11125,   1.65388,    .11059,   1.66426,    .10993,
     1   1.67464,    .10925,   1.68502,    .10857,   1.69540,
     1    .10788,   1.70578,    .10719,   1.71615,    .10648,
     1   1.72653,    .10577,   1.73693,    .10505,   1.74732,
     1    .10433,   1.75772,    .10360,   1.76812,    .10286,
     1   1.77851,    .10212,   1.78891,    .10137,   1.79931,
     1    .10061,   1.80970,    .09985,   1.82009,    .09908,
     1   1.83049,    .09831,   1.84100,    .09752,   1.85151,
     1    .09672,   1.86202,    .09592,   1.87253,    .09512/
      DATA (A(I), I = 351,400) /
     1   1.88304,    .09431,   1.89354,    .09349,   1.90405,
     1    .09267,   1.91456,    .09184,   1.92507,    .09101,
     1   1.93557,    .09017,   1.94631,    .08931,   1.95704,
     1    .08845,   1.96777,    .08758,   1.97850,    .08670,
     1   1.98923,    .08583,   1.99996,    .08495,   2.01069,
     1    .08406,   2.02142,    .08318,   2.03215,    .08229,
     1   2.04288,    .08140,   2.05401,    .08048,   2.06514,
     1    .07955,   2.07627,    .07863,   2.08740,    .07770,
     1   2.09852,    .07678,   2.10965,    .07585,   2.12078,
     1    .07493,   2.13191,    .07401,   2.14304,    .07309/
      DATA (A(I), I=401, 450)/
     1   2.15416,    .07217,   2.16592,    .07120,   2.17767,
     1    .07023,   2.18942,    .06927,   2.20117,    .06831,
     1   2.21293,    .06735,   2.22468,    .06639,   2.23643,
     1    .06545,   2.24819,    .06450,   2.25994,    .06356,
     1   2.27169,    .06262,   2.28447,    .06161,   2.29724,
     1    .06060,   2.31001,    .05960,   2.32278,    .05861,
     1   2.33556,    .05762,   2.34833,    .05664,   2.36111,
     1    .05567,   2.37388,    .05470,   2.38666,    .05375,
     1   2.39943,    .05279,   2.41396,    .05172,   2.42849,
     1    .05066,   2.44302,    .04960,   2.45754,    .04855/
      DATA (A(I), I=451,502) /
     1   2.47207,    .04749,   2.48660,    .04644,   2.50113,
     1    .04538,   2.51566,    .04432,   2.53019,    .04324,
     1   2.54471,    .04216,   2.56260,    .04081,   2.58049,
     1    .03945,   2.59837,    .03809,   2.61626,    .03675,
     1   2.63415,    .03543,   2.65203,    .03414,   2.66993,
     1    .03291,   2.68782,    .03173,   2.70572,    .03061,
     1   2.72362,    .02958,   2.76535,    .02751,   2.80711,
     1    .02589,   2.84888,    .02465,   2.89067,    .02374,
     1   2.93247,    .02312,   2.97428,    .02273,   3.01610,
     1    .02252,   3.05793,    .02243,   3.09976,    .02242,
     1   3.14159,    .02243/
      DO 100 J = 1, 251
      PHILD(J) = A(2*J-1)
      PSILD(J) = A(2*J)
  100 CONTINUE
      DO 202 J=1,201
      CALL LCSFIT (251, PHILD, PSILD, .TRUE., 'B', 201, PHI, PSI, PSI)
  202 CONTINUE
      RETURN
      END
      SUBROUTINE PHPS65(PHI,PSI)
      DIMENSION PHI(1), PSI(1)
      DIMENSION PHILD(251),PSILD(251), A(502)
      DATA (A(I), I = 1, 50) /
     1   0.00000,    .17464,    .01486,    .17457,    .02972,
     1    .17437,    .04458,    .17404,    .05944,    .17360,
     1    .07429,    .17303,    .08914,    .17236,    .10398,
     1    .17159,    .11882,    .17073,    .13365,    .16977,
     1    .14848,    .16872,    .15176,    .16848,    .15503,
     1    .16824,    .15831,    .16799,    .16159,    .16773,
     1    .16486,    .16747,    .16814,    .16721,    .17141,
     1    .16694,    .17469,    .16666,    .17796,    .16638,
     1    .18123,    .16609,    .18624,    .16563,    .19124,
     1    .16516,    .19624,    .16468,    .20124,    .16420/
      DATA (A(I), I = 51,100) /
     1    .20624,    .16371,    .21124,    .16321,    .21625,
     1    .16272,    .22125,    .16223,    .22625,    .16175,
     1    .23125,    .16127,    .24070,    .16041,    .25015,
     1    .15959,    .25960,    .15881,    .26906,    .15808,
     1    .27852,    .15740,    .28798,    .15676,    .29744,
     1    .15616,    .30691,    .15562,    .31638,    .15512,
     1    .32586,    .15468,    .33917,    .15413,    .35249,
     1    .15368,    .36581,    .15331,    .37913,    .15301,
     1    .39246,    .15278,    .40578,    .15261,    .41911,
     1    .15249,    .43244,    .15241,    .44577,    .15237/
      DATA (A(I), I = 101,150) /
     1    .45910,    .15235,    .46962,    .15235,    .48015,
     1    .15236,    .49068,    .15238,    .50121,    .15241,
     1    .51173,    .15245,    .52226,    .15251,    .53279,
     1    .15257,    .54331,    .15264,    .55384,    .15272,
     1    .56436,    .15281,    .57344,    .15289,    .58252,
     1    .15298,    .59160,    .15308,    .60068,    .15318,
     1    .60976,    .15328,    .61884,    .15339,    .62791,
     1    .15350,    .63699,    .15361,    .64607,    .15372,
     1    .65515,    .15383,    .67085,    .15402,    .68656,
     1    .15422,    .70226,    .15440,    .71796,    .15459/
      DATA (A(I), I = 151,200)/
     1    .73366,    .15477,    .74937,    .15496,    .76507,
     1    .15513,    .78077,    .15531,    .79648,    .15548,
     1    .81218,    .15564,    .82590,    .15578,    .83961,
     1    .15592,    .85333,    .15605,    .86704,    .15618,
     1    .88076,    .15630,    .89448,    .15642,    .90819,
     1    .15653,    .92191,    .15663,    .93563,    .15673,
     1    .94934,    .15682,    .96192,    .15690,    .97449,
     1    .15697,    .98706,    .15704,    .99963,    .15709,
     1   1.01220,    .15714,   1.02477,    .15719,   1.03734,
     1    .15723,   1.04992,    .15726,   1.06249,    .15728/
      DATA (A(I), I = 201,250) /
     1   1.07506,    .15730,   1.08689,    .15731,   1.09872,
     1    .15731,   1.11054,    .15730,   1.12237,    .15729,
     1   1.13420,    .15727,   1.14603,    .15724,   1.15786,
     1    .15721,   1.16968,    .15716,   1.18151,    .15711,
     1   1.19334,    .15704,   1.20469,    .15697,   1.21604,
     1    .15689,   1.22740,    .15680,   1.23875,    .15671,
     1   1.25010,    .15660,   1.26145,    .15648,   1.27281,
     1    .15636,   1.28416,    .15623,   1.29551,    .15609,
     1   1.30686,    .15594,   1.31793,    .15578,   1.32900,
     1    .15562,   1.34007,    .15544,   1.35114,    .15526/
      DATA (A(I), I = 251, 300) /
     1   1.36221,    .15506,   1.37327,    .15484,   1.38434,
     1    .15462,   1.39541,    .15437,   1.40647,    .15411,
     1   1.41754,    .15383,   1.42845,    .15354,   1.43935,
     1    .15322,   1.45026,    .15289,   1.46116,    .15254,
     1   1.47207,    .15216,   1.48297,    .15177,   1.49387,
     1    .15135,   1.50477,    .15091,   1.51567,    .15046,
     1   1.52657,    .14998,   1.53737,    .14948,   1.54817,
     1    .14896,   1.55897,    .14842,   1.56977,    .14785,
     1   1.58057,    .14726,   1.59136,    .14665,   1.60215,
     1    .14602,   1.61295,    .14536,   1.62374,    .14467/
      DATA (A(I), I = 301,350) /
     1   1.63452,    .14396,   1.64526,    .14323,   1.65600,
     1    .14247,   1.66673,    .14169,   1.67746,    .14088,
     1   1.68820,    .14005,   1.69892,    .13920,   1.70965,
     1    .13833,   1.72038,    .13744,   1.73110,    .13652,
     1   1.74182,    .13559,   1.75255,    .13464,   1.76327,
     1    .13367,   1.77399,    .13268,   1.78471,    .13167,
     1   1.79542,    .13065,   1.80614,    .12961,   1.81685,
     1    .12856,   1.82756,    .12749,   1.83828,    .12641,
     1   1.84898,    .12531,   1.85973,    .12420,   1.87048,
     1    .12307,   1.88122,    .12194,   1.89197,    .12079/
      DATA (A(I), I = 351,400) /
     1   1.90271,    .11962,   1.91345,    .11845,   1.92419,
     1    .11726,   1.93493,    .11606,   1.94567,    .11485,
     1   1.95640,    .11363,   1.96732,    .11237,   1.97824,
     1    .11111,   1.98916,    .10983,   2.00008,    .10854,
     1   2.01100,    .10725,   2.02191,    .10594,   2.03283,
     1    .10463,   2.04374,    .10332,   2.05466,    .10199,
     1   2.06557,    .10066,   2.07682,    .09929,   2.08807,
     1    .09791,   2.09932,    .09652,   2.11056,    .09513,
     1   2.12181,    .09374,   2.13305,    .09234,   2.14430,
     1    .09094,   2.15554,    .08953,   2.16679,    .08811/
      DATA (A(I), I=401, 450)/
     1   2.17803,    .08669,   2.18979,    .08520,   2.20154,
     1    .08370,   2.21330,    .08220,   2.22505,    .08070,
     1   2.23680,    .07919,   2.24856,    .07768,   2.26031,
     1    .07617,   2.27206,    .07465,   2.28382,    .07314,
     1   2.29557,    .07163,   2.30822,    .07000,   2.32086,
     1    .06838,   2.33351,    .06675,   2.34616,    .06514,
     1   2.35881,    .06352,   2.37146,    .06191,   2.38411,
     1    .06030,   2.39676,    .05870,   2.40941,    .05710,
     1   2.42206,    .05551,   2.43614,    .05374,   2.45023,
     1    .05199,   2.46431,    .05024,   2.47839,    .04850/
      DATA (A(I), I=451,502) /
     1   2.49248,    .04677,   2.50656,    .04504,   2.52065,
     1    .04333,   2.53474,    .04162,   2.54883,    .03992,
     1   2.56292,    .03824,   2.57971,    .03624,   2.59651,
     1    .03426,   2.61330,    .03230,   2.63010,    .03037,
     1   2.64691,    .02848,   2.66372,    .02662,   2.68053,
     1    .02481,   2.69735,    .02305,   2.71417,    .02134,
     1   2.73100,    .01970,   2.77194,    .01597,   2.81292,
     1    .01263,   2.85393,    .00968,   2.89497,    .00712,
     1   2.93604,    .00495,   2.97712,    .00317,   3.01823,
     1    .00178,   3.05934,    .00079,   3.10046,    .00020,
     1   3.14159,   0.00000/
      DO 100 J = 1, 251
      PHILD(J) = A(2*J-1)
      PSILD(J) = A(2*J)
  100 CONTINUE
      DO 202 J=1,201
      CALL LCSFIT (251, PHILD, PSILD, .TRUE., 'B', 201, PHI, PSI, PSI)
  202 CONTINUE
      RETURN
      END
      SUBROUTINE PHPS65A(PHI,PSI)
      DIMENSION PHI(1), PSI(1)
      DIMENSION PHILD(251),PSILD(251), A(502)
      DATA (A(I), I = 1, 50) /
     1   0.00000,    .14761,    .01455,    .14748,    .02910,
     1    .14709,    .04364,    .14649,    .05819,    .14570,
     1    .07273,    .14476,    .08727,    .14369,    .10181,
     1    .14253,    .11634,    .14131,    .13087,    .14007,
     1    .14539,    .13882,    .14877,    .13854,    .15214,
     1    .13826,    .15552,    .13798,    .15890,    .13770,
     1    .16227,    .13743,    .16565,    .13716,    .16903,
     1    .13690,    .17240,    .13665,    .17578,    .13640,
     1    .17916,    .13616,    .18436,    .13581,    .18956,
     1    .13547,    .19477,    .13515,    .19997,    .13484/
      DATA (A(I), I = 51,100) /
     1    .20518,    .13455,    .21039,    .13426,    .21560,
     1    .13398,    .22080,    .13371,    .22601,    .13345,
     1    .23122,    .13318,    .24077,    .13270,    .25032,
     1    .13221,    .25986,    .13172,    .26941,    .13124,
     1    .27896,    .13075,    .28850,    .13026,    .29805,
     1    .12976,    .30760,    .12926,    .31714,    .12876,
     1    .32669,    .12825,    .33995,    .12753,    .35322,
     1    .12681,    .36648,    .12610,    .37974,    .12541,
     1    .39301,    .12475,    .40628,    .12415,    .41955,
     1    .12359,    .43282,    .12311,    .44609,    .12270/
      DATA (A(I), I = 101,150) /
     1    .45937,    .12239,    .46975,    .12221,    .48014,
     1    .12209,    .49053,    .12202,    .50092,    .12199,
     1    .51131,    .12199,    .52170,    .12203,    .53210,
     1    .12209,    .54249,    .12216,    .55288,    .12224,
     1    .56327,    .12233,    .57223,    .12240,    .58118,
     1    .12246,    .59014,    .12253,    .59909,    .12259,
     1    .60805,    .12265,    .61700,    .12271,    .62596,
     1    .12277,    .63491,    .12283,    .64387,    .12289,
     1    .65282,    .12295,    .66829,    .12305,    .68376,
     1    .12316,    .69923,    .12326,    .71470,    .12337/
      DATA (A(I), I = 151,200)/
     1    .73016,    .12348,    .74563,    .12359,    .76110,
     1    .12370,    .77657,    .12381,    .79204,    .12391,
     1    .80750,    .12401,    .82106,    .12410,    .83461,
     1    .12418,    .84816,    .12426,    .86171,    .12434,
     1    .87527,    .12441,    .88882,    .12448,    .90237,
     1    .12455,    .91592,    .12461,    .92948,    .12467,
     1    .94303,    .12472,    .95538,    .12477,    .96772,
     1    .12482,    .98007,    .12486,    .99241,    .12489,
     1   1.00476,    .12492,   1.01711,    .12495,   1.02945,
     1    .12498,   1.04180,    .12500,   1.05414,    .12501/
      DATA (A(I), I = 201,250) /
     1   1.06649,    .12502,   1.07810,    .12503,   1.08971,
     1    .12503,   1.10132,    .12502,   1.11293,    .12501,
     1   1.12454,    .12500,   1.13615,    .12498,   1.14776,
     1    .12495,   1.15937,    .12492,   1.17098,    .12489,
     1   1.18259,    .12485,   1.19372,    .12481,   1.20486,
     1    .12476,   1.21599,    .12471,   1.22713,    .12465,
     1   1.23827,    .12459,   1.24940,    .12452,   1.26054,
     1    .12445,   1.27167,    .12437,   1.28281,    .12428,
     1   1.29394,    .12419,   1.30473,    .12409,   1.31552,
     1    .12398,   1.32631,    .12387,   1.33710,    .12375/
      DATA (A(I), I = 251, 300) /
     1   1.34790,    .12362,   1.35869,    .12349,   1.36948,
     1    .12334,   1.38027,    .12319,   1.39106,    .12303,
     1   1.40185,    .12286,   1.41248,    .12268,   1.42311,
     1    .12249,   1.43375,    .12229,   1.44438,    .12208,
     1   1.45501,    .12186,   1.46564,    .12163,   1.47628,
     1    .12138,   1.48691,    .12113,   1.49754,    .12085,
     1   1.50817,    .12057,   1.51872,    .12027,   1.52928,
     1    .11996,   1.53983,    .11964,   1.55038,    .11930,
     1   1.56094,    .11894,   1.57149,    .11857,   1.58204,
     1    .11818,   1.59259,    .11778,   1.60314,    .11736/
      DATA (A(I), I = 301,350) /
     1   1.61369,    .11693,   1.62418,    .11648,   1.63468,
     1    .11602,   1.64517,    .11554,   1.65566,    .11504,
     1   1.66616,    .11453,   1.67665,    .11400,   1.68714,
     1    .11346,   1.69763,    .11291,   1.70811,    .11234,
     1   1.71860,    .11175,   1.72914,    .11115,   1.73968,
     1    .11054,   1.75021,    .10991,   1.76075,    .10928,
     1   1.77128,    .10863,   1.78182,    .10797,   1.79235,
     1    .10730,   1.80289,    .10662,   1.81342,    .10594,
     1   1.82395,    .10524,   1.83461,    .10453,   1.84527,
     1    .10381,   1.85592,    .10308,   1.86658,    .10235/
      DATA (A(I), I = 351,400) /
     1   1.87723,    .10161,   1.88789,    .10086,   1.89854,
     1    .10010,   1.90920,    .09933,   1.91985,    .09856,
     1   1.93050,    .09778,   1.94138,    .09698,   1.95226,
     1    .09617,   1.96314,    .09535,   1.97402,    .09452,
     1   1.98489,    .09369,   1.99577,    .09285,   2.00664,
     1    .09201,   2.01752,    .09116,   2.02839,    .09030,
     1   2.03927,    .08943,   2.05056,    .08853,   2.06185,
     1    .08763,   2.07314,    .08671,   2.08444,    .08579,
     1   2.09573,    .08487,   2.10702,    .08394,   2.11831,
     1    .08301,   2.12960,    .08207,   2.14089,    .08113/
      DATA (A(I), I=401, 450)/
     1   2.15217,    .08019,   2.16409,    .07919,   2.17601,
     1    .07818,   2.18793,    .07717,   2.19985,    .07616,
     1   2.21176,    .07515,   2.22368,    .07413,   2.23560,
     1    .07311,   2.24751,    .07209,   2.25943,    .07106,
     1   2.27134,    .07004,   2.28424,    .06892,   2.29713,
     1    .06781,   2.31002,    .06670,   2.32292,    .06559,
     1   2.33581,    .06448,   2.34870,    .06338,   2.36159,
     1    .06229,   2.37449,    .06119,   2.38738,    .06011,
     1   2.40028,    .05904,   2.41489,    .05783,   2.42950,
     1    .05664,   2.44411,    .05545,   2.45872,    .05427/
      DATA (A(I), I=451,502) /
     1   2.47334,    .05309,   2.48795,    .05191,   2.50256,
     1    .05072,   2.51717,    .04953,   2.53178,    .04834,
     1   2.54640,    .04713,   2.56432,    .04564,   2.58225,
     1    .04413,   2.60018,    .04263,   2.61810,    .04114,
     1   2.63603,    .03967,   2.65396,    .03823,   2.67189,
     1    .03683,   2.68983,    .03549,   2.70777,    .03421,
     1   2.72571,    .03300,   2.76723,    .03052,   2.80877,
     1    .02844,   2.85033,    .02675,   2.89190,    .02539,
     1   2.93350,    .02434,   2.97510,    .02355,   3.01671,
     1    .02301,   3.05834,    .02266,   3.09996,    .02248,
     1   3.14159,    .02242/
      DO 100 J = 1, 251
      PHILD(J) = A(2*J-1)
      PSILD(J) = A(2*J)
  100 CONTINUE
      DO 202 J=1,201
      CALL LCSFIT (251, PHILD, PSILD, .TRUE., 'B', 201, PHI, PSI, PSI)
  202 CONTINUE
      RETURN
      END
      SUBROUTINE PHPS66(PHI,PSI)
      DIMENSION PHI(1),PSI(1)
      DIMENSION PHILD(201),PSILD(201)
      DIMENSION A(402)
      DATA (A(I), I = 1, 50) /
     1   0.00000,    .16457,    .01573,    .16455,    .03145,
     1    .16449,    .04718,    .16437,    .06290,    .16416,
     1    .07862,    .16386,    .09433,    .16345,    .11003,
     1    .16292,    .12572,    .16223,    .14141,    .16139,
     1    .15708,    .16037,    .17280,    .15916,    .18851,
     1    .15779,    .20422,    .15631,    .21992,    .15475,
     1    .23562,    .15316,    .25131,    .15157,    .26701,
     1    .15002,    .28272,    .14856,    .29844,    .14722,
     1    .31416,    .14604,    .32984,    .14506,    .34552,
     1    .14427,    .36122,    .14364,    .37693,    .14316/
      DATA (A(I), I = 51,100) /
     1    .39264,    .14281,    .40835,    .14257,    .42407,
     1    .14242,    .43980,    .14235,    .45552,    .14233,
     1    .47124,    .14236,    .48696,    .14241,    .50267,
     1    .14248,    .51838,    .14257,    .53409,    .14267,
     1    .54979,    .14280,    .56550,    .14294,    .58121,
     1    .14310,    .59691,    .14327,    .61262,    .14346,
     1    .62832,    .14366,    .64403,    .14387,    .65973,
     1    .14410,    .67544,    .14433,    .69115,    .14457,
     1    .70686,    .14481,    .72257,    .14506,    .73827,
     1    .14530,    .75398,    .14554,    .76969,    .14578/
      DATA (A(I), I = 101,150) /
     1    .78540,    .14601,    .80111,    .14623,    .81682,
     1    .14645,    .83252,    .14665,    .84823,    .14685,
     1    .86394,    .14704,    .87965,    .14722,    .89536,
     1    .14740,    .91106,    .14757,    .92677,    .14774,
     1    .94248,    .14790,    .95819,    .14806,    .97390,
     1    .14821,    .98960,    .14835,   1.00531,    .14849,
     1   1.02102,    .14862,   1.03673,    .14875,   1.05244,
     1    .14886,   1.06814,    .14897,   1.08385,    .14908,
     1   1.09956,    .14917,   1.11527,    .14925,   1.13098,
     1    .14933,   1.14668,    .14940,   1.16239,    .14945/
      DATA (A(I), I = 151,200)/
     1   1.17810,    .14950,   1.19381,    .14954,   1.20952,
     1    .14957,   1.22522,    .14959,   1.24093,    .14961,
     1   1.25664,    .14961,   1.27235,    .14960,   1.28806,
     1    .14959,   1.30376,    .14956,   1.31947,    .14953,
     1   1.33518,    .14948,   1.35089,    .14943,   1.36660,
     1    .14936,   1.38230,    .14928,   1.39801,    .14918,
     1   1.41372,    .14908,   1.42943,    .14896,   1.44514,
     1    .14883,   1.46085,    .14869,   1.47655,    .14853,
     1   1.49226,    .14835,   1.50797,    .14816,   1.52368,
     1    .14796,   1.53939,    .14774,   1.55509,    .14750/
      DATA (A(I), I = 201,250) /
     1   1.57080,    .14725,   1.58651,    .14698,   1.60222,
     1    .14669,   1.61793,    .14638,   1.63364,    .14606,
     1   1.64934,    .14571,   1.66505,    .14533,   1.68076,
     1    .14494,   1.69647,    .14452,   1.71217,    .14407,
     1   1.72788,    .14360,   1.74360,    .14310,   1.75932,
     1    .14256,   1.77504,    .14198,   1.79075,    .14135,
     1   1.80647,    .14067,   1.82217,    .13992,   1.83788,
     1    .13910,   1.85358,    .13820,   1.86927,    .13722,
     1   1.88496,    .13615,   1.90070,    .13498,   1.91642,
     1    .13371,   1.93214,    .13236,   1.94786,    .13093/
      DATA (A(I), I = 251, 300) /
     1   1.96357,    .12942,   1.97927,    .12786,   1.99497,
     1    .12623,   2.01067,    .12456,   2.02636,    .12284,
     1   2.04204,    .12108,   2.05777,    .11929,   2.07349,
     1    .11746,   2.08920,    .11561,   2.10492,    .11373,
     1   2.12062,    .11182,   2.13633,    .10987,   2.15203,
     1    .10790,   2.16773,    .10590,   2.18342,    .10386,
     1   2.19911,    .10180,   2.21483,    .09970,   2.23055,
     1    .09758,   2.24626,    .09542,   2.26197,    .09325,
     1   2.27768,    .09106,   2.29338,    .08885,   2.30909,
     1    .08662,   2.32479,    .08439,   2.34049,    .08214/
      DATA (A(I), I = 301,350) /
     1   2.35619,    .07989,   2.37190,    .07763,   2.38761,
     1    .07537,   2.40331,    .07311,   2.41902,    .07085,
     1   2.43473,    .06859,   2.45043,    .06633,   2.46614,
     1    .06407,   2.48185,    .06182,   2.49756,    .05957,
     1   2.51327,    .05733,   2.52897,    .05510,   2.54467,
     1    .05287,   2.56037,    .05066,   2.57607,    .04846,
     1   2.59178,    .04627,   2.60749,    .04411,   2.62320,
     1    .04196,   2.63891,    .03983,   2.65463,    .03773,
     1   2.67035,    .03566,   2.68604,    .03362,   2.70173,
     1    .03161,   2.71742,    .02964,   2.73312,    .02770/
      DATA (A(I), I=351, 402)/
     1   2.74882,    .02580,   2.76453,    .02395,   2.78025,
     1    .02215,   2.79597,    .02039,   2.81170,    .01869,
     1   2.82743,    .01705,   2.84311,    .01547,   2.85880,
     1    .01396,   2.87449,    .01250,   2.89019,    .01112,
     1   2.90590,    .00981,   2.92161,    .00857,   2.93733,
     1    .00740,   2.95305,    .00631,   2.96878,    .00531,
     1   2.98451,    .00439,   3.00020,    .00356,   3.01590,
     1    .00281,   3.03160,    .00216,   3.04731,    .00159,
     1   3.06302,    .00110,   3.07873,    .00071,   3.09444,
     1    .00040,   3.11016,    .00018,   3.12587,    .00004,
     1   3.14159,   0.00000/
      DO 100 J=1,201
      PHILD(J) = A(2*J-1)
      PSILD(J) = A(2*J)
  100 CONTINUE
      DO 202 J=1,201
      CALL LCSFIT (201, PHILD, PSILD, .TRUE., 'B', 201, PHI, PSI, PSI)
  202 CONTINUE
      RETURN
      END
      SUBROUTINE PHPS67(PHI,PSI)
      DIMENSION PHI(1), PSI(1)
      DIMENSION PHILD(251),PSILD(251), A(502)
      DATA (A(I), I=1, 50) /
     1   0.00000,    .18028,    .01499,    .18026,    .02998,
     1    .18018,    .04497,    .18002,    .05995,    .17976,
     1    .07492,    .17938,    .08988,    .17885,    .10483,
     1    .17815,    .11976,    .17725,    .13468,    .17614,
     1    .14959,    .17480,    .15280,    .17447,    .15601,
     1    .17414,    .15923,    .17380,    .16244,    .17345,
     1    .16565,    .17310,    .16886,    .17276,    .17208,
     1    .17241,    .17529,    .17207,    .17850,    .17174,
     1    .18172,    .17141,    .18686,    .17092,    .19201,
     1    .17045,    .19716,    .17001,    .20231,    .16959/
      DATA (A(I), I = 51,100) /
     1    .20746,    .16918,    .21262,    .16879,    .21777,
     1    .16840,    .22293,    .16802,    .22808,    .16764,
     1    .23323,    .16726,    .24277,    .16653,    .25229,
     1    .16578,    .26182,    .16502,    .27135,    .16426,
     1    .28088,    .16349,    .29041,    .16272,    .29993,
     1    .16196,    .30946,    .16121,    .31899,    .16048,
     1    .32853,    .15978,    .34178,    .15885,    .35505,
     1    .15798,    .36832,    .15717,    .38159,    .15641,
     1    .39486,    .15572,    .40814,    .15507,    .42142,
     1    .15449,    .43470,    .15396,    .44798,    .15348/
      DATA (A(I), I = 101,150) /
     1    .46127,    .15306,    .47164,    .15277,    .48200,
     1    .15251,    .49237,    .15228,    .50274,    .15207,
     1    .51311,    .15188,    .52348,    .15171,    .53384,
     1    .15156,    .54421,    .15142,    .55458,    .15129,
     1    .56495,    .15116,    .57385,    .15105,    .58275,
     1    .15095,    .59165,    .15085,    .60055,    .15075,
     1    .60945,    .15066,    .61835,    .15057,    .62725,
     1    .15049,    .63615,    .15041,    .64505,    .15034,
     1    .65395,    .15027,    .66928,    .15016,    .68461,
     1    .15007,    .69994,    .15000,    .71527,    .14994/
      DATA (A(I), I = 151,200)/
     1    .73060,    .14990,    .74593,    .14986,    .76125,
     1    .14984,    .77658,    .14982,    .79191,    .14982,
     1    .80724,    .14982,    .82061,    .14982,    .83397,
     1    .14983,    .84734,    .14985,    .86070,    .14986,
     1    .87407,    .14988,    .88743,    .14991,    .90080,
     1    .14993,    .91416,    .14996,    .92753,    .14999,
     1    .94089,    .15002,    .95309,    .15005,    .96529,
     1    .15008,    .97749,    .15011,    .98969,    .15014,
     1   1.00189,    .15018,   1.01409,    .15021,   1.02629,
     1    .15024,   1.03849,    .15028,   1.05069,    .15031/
      DATA (A(I), I = 201,250) /
     1   1.06289,    .15035,   1.07435,    .15038,   1.08580,
     1    .15042,   1.09726,    .15045,   1.10871,    .15048,
     1   1.12017,    .15052,   1.13162,    .15056,   1.14308,
     1    .15059,   1.15453,    .15063,   1.16599,    .15067,
     1   1.17744,    .15070,   1.18840,    .15074,   1.19936,
     1    .15077,   1.21032,    .15081,   1.22129,    .15084,
     1   1.23225,    .15088,   1.24321,    .15091,   1.25417,
     1    .15094,   1.26513,    .15096,   1.27609,    .15099,
     1   1.28706,    .15101,   1.29770,    .15103,   1.30835,
     1    .15104,   1.31900,    .15106,   1.32965,    .15107/
      DATA (A(I), I = 251, 300) /
     1   1.34029,    .15108,   1.35094,    .15108,   1.36159,
     1    .15109,   1.37224,    .15109,   1.38289,    .15110,
     1   1.39353,    .15110,   1.40401,    .15110,   1.41449,
     1    .15110,   1.42498,    .15110,   1.43546,    .15109,
     1   1.44594,    .15109,   1.45642,    .15108,   1.46690,
     1    .15107,   1.47738,    .15106,   1.48786,    .15104,
     1   1.49834,    .15102,   1.50878,    .15100,   1.51921,
     1    .15097,   1.52965,    .15094,   1.54008,    .15090,
     1   1.55052,    .15086,   1.56095,    .15081,   1.57139,
     1    .15076,   1.58182,    .15071,   1.59226,    .15065/
      DATA (A(I), I = 301,350) /
     1   1.60269,    .15059,   1.61319,    .15053,   1.62369,
     1    .15046,   1.63419,    .15038,   1.64469,    .15030,
     1   1.65519,    .15022,   1.66569,    .15013,   1.67619,
     1    .15003,   1.68669,    .14993,   1.69719,    .14982,
     1   1.70770,    .14970,   1.71841,    .14957,   1.72912,
     1    .14943,   1.73984,    .14929,   1.75055,    .14914,
     1   1.76127,    .14897,   1.77198,    .14880,   1.78270,
     1    .14862,   1.79341,    .14842,   1.80412,    .14822,
     1   1.81484,    .14800,   1.82589,    .14776,   1.83694,
     1    .14752,   1.84798,    .14725,   1.85903,    .14698/
      DATA (A(I), I = 351,400) /
     1   1.87008,    .14669,   1.88113,    .14639,   1.89218,
     1    .14607,   1.90322,    .14574,   1.91427,    .14539,
     1   1.92531,    .14502,   1.93688,    .14462,   1.94844,
     1    .14420,   1.96000,    .14376,   1.97156,    .14329,
     1   1.98312,    .14279,   1.99468,    .14227,   2.00623,
     1    .14171,   2.01779,    .14111,   2.02934,    .14048,
     1   2.04089,    .13981,   2.05308,    .13905,   2.06526,
     1    .13825,   2.07745,    .13739,   2.08962,    .13648,
     1   2.10180,    .13551,   2.11397,    .13449,   2.12613,
     1    .13341,   2.13829,    .13227,   2.15044,    .13106/
      DATA (A(I), I=401, 450)/
     1   2.16259,    .12980,   2.17538,    .12839,   2.18816,
     1    .12692,   2.20093,    .12538,   2.21370,    .12380,
     1   2.22646,    .12216,   2.23922,    .12048,   2.25197,
     1    .11876,   2.26472,    .11700,   2.27746,    .11523,
     1   2.29021,    .11343,   2.30379,    .11150,   2.31737,
     1    .10955,   2.33095,    .10758,   2.34453,    .10559,
     1   2.35810,    .10358,   2.37167,    .10154,   2.38523,
     1    .09948,   2.39879,    .09738,   2.41235,    .09526,
     1   2.42590,    .09310,   2.44078,    .09070,   2.45566,
     1    .08825,   2.47053,    .08576,   2.48539,    .08324/
      DATA (A(I), I=451,502) /
     1   2.50025,    .08068,   2.51510,    .07808,   2.52995,
     1    .07545,   2.54478,    .07278,   2.55962,    .07008,
     1   2.57444,    .06735,   2.59138,    .06420,   2.60831,
     1    .06102,   2.62523,    .05782,   2.64216,    .05463,
     1   2.65909,    .05145,   2.67603,    .04830,   2.69297,
     1    .04518,   2.70993,    .04212,   2.72690,    .03912,
     1   2.74389,    .03620,   2.78335,    .02979,   2.82290,
     1    .02391,   2.86255,    .01859,   2.90227,    .01386,
     1   2.94205,    .00977,   2.98189,    .00635,   3.02178,
     1    .00362,   3.06170,    .00163,   3.10164,    .00041,
     1   3.14159,   0.00000/
      DO 100 J = 1, 251
      PHILD(J) = A(2*J-1)
      PSILD(J) = A(2*J)
  100 CONTINUE
      DO 202 J=1,201
      CALL LCSFIT (251, PHILD, PSILD, .TRUE., 'B', 201, PHI, PSI, PSI)
  202 CONTINUE
      RETURN
      END
C+---------------------------------------------------------------------
C

C+----------------------------------------------------------------------
C
      FUNCTION THREEPT (J, H, DEL)
C
C     One-liner: First derivative, non-central 3-point formula
C     ----------
C
C     Description and usage:
C     ----------------------
C
C        Computes a first derivative approximation for PLSFIT over an
C     interval at a data boundary, using a forward or backward 3-point
C     formula.  The data must be in the form of arrays containing finite
C     difference interval lengths and 2-point forward difference deriva-
C     tives, and the differencing direction is controlled by a flag. See
C     PLSFIT for more details.
C
C        See module BUTLAND for a version with "shape-preserving"
C     adjustments.
C
C     Arguments:
C     ----------
C
C     Name    Type/Dimension  I/O/S  Description
C     J       I               I      Indicates at which end of the
C                                    interval the derivative is to be
C                                    estimated. J = 0 means left-hand
C                                    side, J = 1 means right. 
C
C     H       R (-1:1)        I      Array of interval lengths. The 0th
C                                    element is the length of the interval
C                                    on which the cubic is to be deter-
C                                    mined.
C
C     DEL     R (-1:1)        I      Array of derivative estimates. The
C                                    0th element is the forward difference
C                                    derivative over the interval on which
C                                    the cubic is to be determined.
C                                     
C     THREEPT R                 O    The function value is the derivative.
C
C     Environment:  VAX/VMS; FORTRAN 77
C     ------------
C
C     IMPLICIT NONE and 8-character symbolic names are non-standard.
C
C     Author:  Robert Kennelly, Sterling Federal Systems/NASA-Ames
C     -------
C
C     History:
C     --------
C
C     18 Feb. 1987    RAK    Initial design and coding.
C     06 June 1991    DAS    Original THREEPT renamed BUTLAND; THREEPT
C                            now gives unmodified 1-sided 3-pt. results.
C
C-----------------------------------------------------------------------

      IMPLICIT NONE

C     Arguments.

      INTEGER
     &   J
      REAL
     &   H (-1:1), DEL (-1:1), THREEPT

C     Local constants.

      REAL
     &   ONE
      PARAMETER
     &  (ONE = 1.0E+0)

C     Local variables.

      INTEGER
     &   STEP
      REAL
     &   WEIGHT

C     Execution.
C     ----------

C     Estimate first derivative on a left-hand boundary using a 3-point
C     forward difference (STEP = +1), or with a backward difference for
C     the right-hand boundary (STEP = -1).

      STEP = 1 - J - J   ! J here is consistent with related modules.

C     In {H, DEL} form, the derivative looks like a weighted average.

      WEIGHT  = -H (0) / (H (0) + H (STEP))
      THREEPT = WEIGHT * DEL (STEP) + (ONE - WEIGHT) * DEL (0)

C     Termination.
C     ------------

      RETURN
      END

C+---------------------------------------------------------------------
C
      SUBROUTINE LDDATA (MAXPTS, TITLE, NU, NL, XU, XL, YU, YL )
C
C  PARAMETERS:
C   ARG     DIM   TYPE  I/O/S   DESCRIPTION
C   MAXPTS   -      I     I     Max. no. of points on any one surface.
C   TITLE    -     C*(*)  I     Variable-length title for profile.
C   NU,NL    -      I     I     Number of data points for upper and lower
C                               surfaces (NU > 0; NL=0 is OK if FORMAT=1;
C                               NU=NL=no. of pts. in 3-column format and
C                               the camber/thickness distributions - FORMATs
C                               4 & 5).
C   XU     MAXPTS   R     I     Upper surface abscissas.
C   XL     MAXPTS   R     I     Lower surface abscissas (if any).
C   YU,YL  MAXPTS   R     I     Corresponding ordinates, y" values, or
C----------------------------------------------------------------------

      COMMON /SIXPNTS/
     >      NML,NMU,XXL(1000),XXU(1000),YYL(1000),YYU(1000)
      SAVE /SIXPNTS/

C  *  Arguments:

      CHARACTER
     >   TITLE * (*)
      INTEGER
     >   MAXPTS, NL, NU
      REAL
     >   XU (MAXPTS), XL (MAXPTS), YU (MAXPTS), YL (MAXPTS)

C  *  Local variables:

      INTEGER
     >   I

C  *  Execution:
			NML = NL
      NMU = NU

      DO 780 I=1,NL
         XXL(I) = XL(I)
         YYL(I) = YL(I)
  780 CONTINUE

      DO 790 I=1,NU
         XXU(I) = XU(I)
         YYU(I) = YU(I)
  790 CONTINUE


      RETURN

      END

