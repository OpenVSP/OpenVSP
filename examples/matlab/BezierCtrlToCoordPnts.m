## Copyright (C) 2018 Justin Gravett
## 
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*- 
## @deftypefn {Function File} {@var{PntsVec} =} BezierCtrlToCoordPnts (@var{bezier_file}, @var{t_vec})
##
##    bezier_file: File name containing Bezier control point data, exported from OpenVSP
##    t_vec: vector of t = 0.0 - 1.0 to evaluate the curve at
##
##    [PntsVec]: Returned vector of 2D airfoil coordinate points
##
##    Note: This function only cubic or lower order Bezier segments. If higher 
##      segments are expected, a recursive Bezier curve evaluation function 
##      can be implemented
##
##    Reference Equations: https://en.wikipedia.org/wiki/Bezier_curve
##
## @end deftypefn

## Author: Justin Gravett
## Organization: ESAero
## Created: 2018-01-26

function [PntsVec] = BezierCtrlToCoordPnts( bezier_file, t_vec )

t_vec = sort(t_vec); % ensure t_vec is in ascending order

fid = fopen( bezier_file, 'r' );

str_cell = textscan(fid, '%s', 'delimiter', '\n');

% identify # of segments
nseg_index = find(strncmp(str_cell{1}, 'Num Bezier Seg', 14));
num_seg_cell = str_cell{1}(nseg_index);
num_seg = str2num( regexp(num_seg_cell{1},'\d*','Match'){1} );

% identify starting index of Bezier segment data and create data matrix
seg0_index = find(strncmp(str_cell{1}, '# Order', 7)) + 1;
bez_seg_mat = csvread( bezier_file, (seg0_index - 1), 0 ); % note: 0 based indexing

fclose( fid );

PntsVec = zeros( length(t_vec), 2 ); % preallocate vector

pnt_cnt = 1;
seg0_index = 1;

for t = 1:1:length(t_vec)
  
  if ( ( t_vec(t) < 0.0 ) || ( t_vec(t) > 1.0 ) )
    
    msgbox('Error: t is not between 0 and 1')
    break
    
  end
  
  t_set = false; % t_vec(t) has not been evaluated
  
  for j = seg0_index:1:num_seg
    
    if ( t_set == true )
      
      break % t_vec(t) has been evaluated
      
    end
    
    seg = bez_seg_mat(j,:);
    
    order = seg(1);
    seg_t0 = seg(2);
    seg_tmax = seg(3);
    
    if ( ( seg_t0 <= t_vec(t) ) && ( seg_tmax >= t_vec(t) ) )
      
      seg0_index = j;
      
      dt = seg_tmax - seg_t0;
      
      t_local = ( t_vec(t) - seg_t0 ) / dt;
      
      P0 = seg(4:5);
      P1 = seg(6:7);
      P2 = seg(8:9);
      P3 = seg(10:11);
      
      if ( order == 1 )
        
        PntsVec(pnt_cnt,:) = (1-t_local)*P0 + t_local*P1;
        
      elseif ( order == 2 )
        
        PntsVec(pnt_cnt,:) = ((1-t_local)^2)*P0 + 2*(1-t_local)*t_local*P1 + (t_local^2)*P2;
        
      elseif ( order == 3 )
        
        PntsVec(pnt_cnt,:) = ((1-t_local)^3)*P0 + 3*((1-t_local)^2)*t_local*P1 + 3*(1-t_local)*(t_local^2)*P2 + (t_local^3)*P3;
        
      else
        
        pnt_cnt = pnt_cnt - 1; % order not supported, skip point
        
      end
      
      pnt_cnt = pnt_cnt + 1;
      t_set = true; % t_vec(t) has been evaluated
      
    end
    
  end
  
end

endfunction
