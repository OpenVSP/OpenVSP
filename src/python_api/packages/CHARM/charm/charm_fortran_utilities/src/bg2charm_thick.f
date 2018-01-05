      program bg2charm

C** PROGRAM bg2charm.f for CHARM VERSION 5.4.

C** This program allows you to create a CHARM Blade Geometry
C** Input File from alternate format input files.

C** 9-2-15:  xyz LE and TE supported
C** 2-18-06: Westland format supported
C** 10-25-10: Limited  format supported
C** 9-22-15: Removed the qtr entry - this is redundant.

      implicit real*8 (a-h,o-z)
      character*72 finp,fout,cmnt
 
      parameter (maxr=201)

      real*8 r(maxr),rbar(maxr),le(maxr),te(maxr),chord(maxr),
     $       drop(maxr),twist(maxr),qtr(maxr),tdc(maxr),
     $       sl(maxr),sweepd(maxr),twstgd(maxr),anhd(maxr)
      real*8 xqr(3)      ! option for root offset added with iformat=4

C---------------------------------------------------------------

998   format(1X,9F12.5)
999   format(A)

      pi = 3.141592654
      twopi = 2.0*pi
      dtr = pi/180.0
      rtd = 180.0/pi

C** Assign the input file to Unit 98
      if(iargc().lt.1)then
        write(*,*)'Enter input file name'
        read(*,999) finp
      else
       call getarg(1,finp)
      end if
      open(98,file=finp,status='old')
      leninp = lenstr(finp)

C** Assign the output file to Unit 99
      write(*,*)'Enter output file name'
      read(*,999) fout
      open(99,file=fout,status='unknown')
      lenout = lenstr(fout)

C---------------------------------------------------------------

C** Identify the format of the input file

      print*,' '
      print*,' BG2CHARM VERSION 5.0 for CHARM VERSION 5.4'
      print*,'            VERSION DATE 9-2-15'
      print*,' '
      print*,' Enter the file format of the input file: '
      print*,'    "1" for Westland format: '
      print*,'    "2" for limited format (r  rbar  twist  chord): '
      print*,'    "3" for Westland format + t/c column at end: '
      print*,'    "4" for xyz LE, TE format + t/c column at end: '
      print*,'    "5" for additional format: '
5     read(*,*) iformat
      if (iformat.lt.1 .or. iformat.gt.5) then
       print*,' Currently only formats 1 2 3 4 and 5 are supported '
       go to 5
      end if
      if (iformat.ne.2) then
       print*,' Enter the technique for building the blade:'
       print*,'   "0" sweep and anhedral in plane of rotation'
       print*,'        (standard CHARM format) '
       print*,'   "1" sweep and anhedral in chordal plane'
       print*,'        (standard WESTLAND format) '
       read(*,*) kbgeom
      else
       kbgeom = 0
      end if
      print*,' '
      print*,' Enter conversion factor for length scale (Def. = 1)'
      read(*,*) fac
      if (fac.eq.0.0) fac = 1.0

C---------------------------------------------------------------

C** Read in the input file.
      read(98,999) cmnt
      write(*,6)
6     format('       r           rbar        le          te      ',
     $       '    chord     anhedral      twist     1/4chord     t/c')

C** I added this to check for a tdc column for iformat=4 and find radius
      if (iformat.eq.4) then
       read(98,999) cmnt
       call COLUMNS(cmnt,72,ncol)
7      read(98,*,end=8) x1,y1,z1,x2,y2,z2
       go to 7
8      radius = .75*x1 + .25*x2
       rewind 98
       read(98,999) cmnt
      end if
       
      i = 0
      area = 0.0
      chdav = 0.0
10    i = i + 1
      if (iformat.eq.1) then
c** Westland format
       read(98,*,end=20) r(i),rbar(i),le(i),te(i),chord(i),
     $                   drop(i),twist(i)
       qtr(i) = 0.75*le(i) + 0.25*te(i)
       tdc(i) = 0.0
      else if (iformat.eq.2) then
c** Limited format
       read(98,*,end=20) r(i),rbar(i),twist(i),chord(i)
       le(i) = -.25*chord(i)
       te(i) =  .75*chord(i)
       drop(i) = 0.0
       qtr(i) = 0.0
       tdc(i) = 0.0
      else if (iformat.eq.3) then
       read(98,*,end=20) r(i),rbar(i),le(i),te(i),chord(i),
     $                   drop(i),twist(i),tdc(i)
       qtr(i) = 0.75*le(i) + 0.25*te(i)
      else if (iformat.eq.4) then
       tdc(i) = 0.0
       if (ncol.eq.6) read(98,*,end=20) x1,y1,z1,x2,y2,z2
       if (ncol.eq.7) read(98,*,end=20) x1,y1,z1,x2,y2,z2,tdc(i)
       xqc = .75*x1 + .25*x2     ! quarter-chord location
       yqc = .75*y1 + .25*y2
       zqc = .75*z1 + .25*z2
       r(i) = xqc
       rbar(i) = r(i)/radius
       qtr(i) = yqc
       drop(i) = zqc
       dy = y2 - y1
       dz = z2 - z1
       chord(i) = sqrt(dy*dy + dz*dz)
       le(i) = qtr(i) - .25*chord(i)
       te(i) = qtr(i) + .75*chord(i)
       if (dy.eq.0.0) twist(i) = 90.0
       if (dy.ne.0.0) twist(i) = rtd*atan(dz/dy)
c** Option for offset root quarter chord
       if (i.eq.1) then
        xqr(1) = xqc
        xqr(2) = yqc
        xqr(3) = zqc
       end if
      else if (iformat.eq.5) then
CDAW 2016-01-21: One customer sent a file without enough digits in the r column.
CDAW 2016-01-21: Since this customer determines r from rbar, I put this in quickly
CDAW 2016-01-21: to fix this.
       if (i.eq.1) then
11      read(98,*,end=12) radius
        go to 11
12      rewind(98)
        read(98,999) cmnt
       end if
       read(98,*,end=20) r(i),rbar(i),le(i),te(i),dum,chord(i),
     $                   drop(i),twist(i),tdc(i)
       le(i) = -le(i)
       te(i) = -te(i)
       qtr(i) = le(i) + .25*chord(i)
       r(i) = radius*rbar(i)
      end if
      r(i) = fac*r(i)
      le(i) = fac*le(i)
      te(i) = fac*te(i)
      chord(i) = fac*chord(i)
      qtr(i) = fac*qtr(i)
c** determine blade area
      if (i.gt.1) then
       area = area + 0.5*(chord(i) + chord(i-1))*(r(i)-r(i-1))
      end if
      drop(i) = fac*drop(i)
      write(*,998) r(i),rbar(i),le(i),te(i),chord(i),drop(i),twist(i),
     $             qtr(i),tdc(i)
      go to 10
20    continue
      n = i - 1
c** average chord
      chdav = area / ( r(n) - r(1) )
      write(*,997) chdav
997   format(/' Average chord = ',F10.5)

c** check that the last input rbar is 1.0
      test = abs( rbar(n) - 1.0 )
      if (test.gt.1.E-4) then
       print*,' '
       print*,' ERROR: Last rbar value must be 1.0'
       print*,'        instead of ',rbar(n)
       stop
      end if

c** check that rbar values correspond to r values
      do i=1,n
       test = abs( rbar(i) - r(i)/r(n) )
       if ( test.gt.1.E-4) then
        print*,' '
        print*,' ERROR: r/rbar mismatch for entry',i
        print*,'        rbar entry is ',rbar(i)
        print*,'    computed value is ',r(i)/r(n)
        stop
       end if
      end do

c** check that rbar values increase monotonically
      do i=2,n
       if (rbar(i).le.rbar(i-1)) then
        print*,' '
        print*,' ERROR: rbar must increase monotonically '
        print*,'        Check entries',i-1,i
        stop
       end if
      end do

c** check that the chord matches the LE and TE values
      do i=1,n
       test = abs( chord(i) - (te(i) - le(i)) ) / chdav
       if ( test.gt.1.E-4) then
        print*,' '
        print*,' ERROR: chord,te,le mismatch for entry',i
        print*,'        chord = ',chord(i)
        print*,'        te = ',te(i)
        print*,'        le = ',le(i)
        print*,'     chdav = ',chdav
        print*,'     area = ',area
        stop
       end if
      end do

c** check that le + .25*chord is close to qtr values
      do i=1,n
       qtest = le(i) + 0.25*chord(i)
       test = abs( qtr(i) - qtest )
       if (test.gt.1.E-4) then
        print*,' '
        print*,' ERROR: chord,qtr,le mismatch for entry',i
        print*,'        chord = ',chord(i)
        print*,'        le = ',le(i)
        print*,'    1/4 chord = ',qtr(i)
        print*,'     le + .25*chord = ',qtest
        print*,'     1/4 chord should equal le + .25*chord'
        stop
       end if
      end do

c** assign CHARM values
      fspan = 50.0
      nseg = n - 1
      cutout = r(1)
cdaw added 11-13-2015 for offset of root chord without iformat=4
      xqr(1) = cutout
      xqr(2) = qtr(1)
      xqr(3) = 0.0
      radius = r(n)
      span = r(n) - r(1)
      twrd = twist(1)
      do i=2,n
       im1 = i-1
       dx = r(i) - r(im1)
       dy = qtr(i) - qtr(im1)
       dz = drop(i) - drop(im1)
       sl(im1) = dx
       fspan = max( fspan , radius/dx) 
       sweepd(im1) = rtd*atan2( dy , dx )
       if (abs(sweepd(im1)).lt.1.E-7) sweepd(im1) = 0.0 
       anhd(im1) = rtd*atan2( dz , dx )
       if (abs(anhd(im1)).lt.1.E-7) anhd(im1) = 0.0 
       twstgd(im1) = twist(i) - twist(im1)
      end do
      nspan = fspan
      nspan = min( nspan , 100)

      go to 100

C-----------------------------------------------------------------
C** Insert alternate formats here
C-----------------------------------------------------------------

100   continue

c** printout the CHARM Blade Geometry Input File

      elofsg = 0.0
      thiknd = 0.12
      kflap = 0
      flapnd = 0.0
      flhnge = 0.0
      fldefl = 0.0
      nsegp1 = nseg + 1
      write(99,1010) fout(1:lenout),finp(1:leninp)
1010  format(A,': CHARM Blade Geometry Input File made from ',A)
      write(99,1015) kbgeom
      write(99,1020) nseg
c** Option for offset root quarter chord
cdaw added 11-13-2015 for offset of root chord without iformat=4
c     if (iformat.ne.4) write(99,1030) cutout
c     if (iformat.eq.4) write(99,1030) xqr
      write(99,1030) xqr
      write(99,1040)
      write(99,2000) (sl(i),i=1,nseg)
      write(99,1050)
      write(99,2000) (chord(i),i=1,nsegp1)
      write(99,1060)
      write(99,2000) (elofsg,i=1,nsegp1)
      write(99,1070)
      write(99,2000) (sweepd(i),i=1,nseg)
      write(99,1080) twrd
      write(99,1090)
      write(99,2000) (twstgd(i),i=1,nseg)
      write(99,1100)
      write(99,2000) (anhd(i),i=1,nseg)
      write(99,1110)
      write(99,2000) (tdc(i),i=1,nsegp1)
      write(99,1120) nseg
      write(99,1130) nseg 
      write(99,1140) nseg
      write(99,1150) nseg
      write(99,1160) nspan
1015  format('KBGEOM'/I3)
1020  format('NSEG'/I3)
c** Option for offset root quarter chord
1030  format('CUTOUT'/3F14.6)
1040  format('SL(ISEG)')
1050  format('CHORD(ISEG)')
1060  format('ELOFSG(ISEG) - (elastic axis offset)')
1070  format('SWEEPD(ISEG)')
1080  format('TWRD (Blade root twist at zero collective in degrees)'/
     $        F14.6)
1090  format('TWSTGD(ISEG)')
1100  format('ANHD(ISEG)')
1110  format('THIKND(ISEG)')
1120  format('KFLAP(ISEG)'/I3,'*0')
1130  format('FLAPND(ISEG)'/I3,'*0.0')
1140  format('FLHNGE(ISEG)'/I3,'*0.0')
1150  format('FLDEFL(ISEG)'/I3,'*0.0')
1160  format('NCAM'/' 0'/
     $  'NCHORD  NSPAN  ICOS'/'  1     ',I3,'    -1')

      write(*,3000) fout
3000  format(//' Your Blade Geometry Input File resides in   ',A/)
2000  format(5F14.6)

      stop
      end

C====================================================================================
      SUBROUTINE COLUMNS(cmnt,nlength,ncol)
      character*(*) cmnt
      character*1 c,cj
      character*6 cval
      ncol = 7 
      return
      iblank = 1
      iast = 0
      do i=1,nlength
       c = cmnt(i:i)
 
c---------------------- added 4-4-08 -----------------------c
       if (c.eq."*") then
c** Determine the integer in front of the asterisk and
c** add this value (minus 1) to ncol.
c** Do this by backing up in front of the * until you hit
c** a comma, blank or the first character of the line.
        j = i
10      j = j - 1
        if (j.eq.0) then
c** this is the first entry and it begins in column 1
         cval = cmnt(1:i-1)
         read(cval,1000) nadd
         ncol = ncol + nadd - 1
         go to 20
        else
         cj = cmnt(j:j)
         if (cj.eq."," .or. cj.eq."") then
          cval = cmnt(j+1:i-1)
          read(cval,1000) nadd
          ncol = ncol + nadd - 1
          go to 20
         end if
        end if
        go to 10
20      continue
1000  format(i6)
       end if
c---------------------- end added 4-4-08 --------------------c
         
       if (c.ne."" .and. c.ne."," .and. iblank.eq.1) then
        ncol = ncol + 1
        iblank = 0
       end if
       if ( (c.eq."" .or. c.eq.",") .and. iblank.eq.0) iblank = 1
      end do

      return
      end

      FUNCTION LENSTR(S)
C
C Returns the number of characters in string S with 
C leading blanks removed and trailing blanks not counted.
C
      CHARACTER*(*) S
      DO I=LEN(S),1,-1
       IF (S(I:I).NE.' ') THEN
        LENSTR=I
        GO TO 10
       END IF
      END DO
      LENSTR=0
      RETURN

10    IBLANK = 0
      DO I=1,LENSTR
       IF (S(I:I).EQ.' ') THEN
        IBLANK = IBLANK + 1
       ELSE
        GO TO 20
       END IF
      END DO

20    IF (IBLANK.EQ.0) RETURN
      LENSTR = LENSTR - IBLANK
      S(1:LENSTR) = S(1+IBLANK:LENSTR+IBLANK)
      DO I=1,IBLANK
       S(LENSTR+I:LENSTR+I)=' '
      END DO
 
      RETURN
      END
