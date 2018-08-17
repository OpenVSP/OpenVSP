%% BezierExample.m 
%%  Script that demonstrates converting airfoil Bezier control points into
%%  airfoil coordinate points, plotting the airfoil, and writing the coordinate
%%  points to a Selig airfoil file that can be loaded into a program such as 
%%  XFoil. 
%%
%% Date: 1/28/2018
%% Organization: ESAero
%% Author: Justin Gravett

clear all;
close all;
clc;

format long;

t_vec = linspace(0.0,1.0,160); % points to evaluate bezier curve, 0 <= t <= 1

PntsVec = BezierCtrlToCoordPnts( 'WingFoil.bz', t_vec );

figure(1)
hold on
plot( PntsVec(:,1), PntsVec(:,2) )
title( 'Airfoil') 
xlabel( 'X' )
ylabel( 'Y' )
hold off

%% Write ordered coordinate points to a selig airfoil file that can be read into XFoil
fid = fopen( 'BezierCoordPnts.dat', 'w' );

fprintf( fid, 'SELIG AIRFOIL\n' );

[nrows, ncol] = size( PntsVec );

for i = 1:nrows
  fprintf( fid, ' %17.16f     %17.16f\n', PntsVec(i,1), PntsVec(i,2) );  
end

fclose( fid );