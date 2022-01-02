function [con, p, uv1, uv2, uv3, wedata, id] = readvspgeom( fname, plotflag )
%readvspgeom reads a *.vspgeom file into Matlab
%
%   *.vspgeom files are a new file format to facilitate communication
%   between OpenVSP and VSPAERO.  Specifically, this file was designed
%   with three enhancements over existing file formats:
%    1. Multi-valued nodal surface U/V parameter value.
%    2. Explicit wake attachment line identification.
%    3. Support for polygons, not just triangles (not yet utilized).
%
%   Though the file format does not require explicit support for it, the
%   intent is for *.vspgeom files to support mixed thick/thin
%   representation of geometry.  This might be used (for example) to
%   represent an aircraft where the fuselage is think and the wing,
%   empennage, and rotors are thin.
%
%   [t, p, uv1, uv2, uv3, wedata, id] = readvspgeom( fname, plotflag )
%
%   Reads in a *.vspgeom file passed in fname.  If plotflag is true, then
%   a series of simple plots are generated.  If plotflag is not passed,
%   the plots are not generated.
%
%     con      Polygon connectivity matrix
%     p        Point matrix
%     uv1      U/V surface data at poly node 1
%     uv2      U/V surface data at poly node 2
%     uv3      U/V surface data at poly node 3
%     wedata   Wake edge data cell array
%     id       Surface id vector
%

%   Rob McDonald
%   6 November 2020 v. 1.0

if ( nargin < 2 )
    plotflag = false;
end

fp = fopen(fname,'r');

% Read in number of points
npt = fscanf(fp, '%d', 1);

% Read in the point coordinates.
ptdata = fscanf(fp, '%f', [3 npt]);
p = ptdata(1:3,:);

% Read in number of polygons
npoly = fscanf(fp, '%d', 1);

% Read in the connectivity.  The file format is designed to support
% polygons, but currently is only implemented for triangles.  Hence
% the triangle assumption here.
condata = fscanf(fp, '%d', [4 npoly]);
% Number of points in each polygon
np = condata(1,:);
% Polygon points (currently triangles)
con = condata(2:4,:);

% Read in the surface id's and poly node data
tdata = fscanf(fp, '%f', [7 npoly]);

% Face ID
id = tdata(1,:);
% U/V of each polygon node
uv1 = tdata(2:3,:);
uv2 = tdata(4:5,:);
uv3 = tdata(6:7,:);


% Read in the number of wakes
nwake = fscanf(fp, '%d', 1);

% Loop over wakes reading in points
wedata=[];
for iw = 1:nwake
    nwpt(iw) = fscanf(fp, '%d', 1 );
    wedata{iw} = fscanf(fp, '%d', nwpt(iw) );
end

fclose(fp);

if ( plotflag )

    figure
    trisurf(con', p(1,:), p(2,:), p(3,:),id); % 'EdgeColor','none');
    axis equal
    axis off
    h = plotwakes( wedata, p );
    set(h,'LineWidth',5);
    %set(h,'Color','k');
    title('Mesh colored by ID with wake lines')

    figure
    h = plotwakes( wedata, p );
    axis equal
    axis off
    set(h,'LineWidth',5);
    %set(h,'Color','k');
    title('Wake lines')

    % Format data in manner appropriate for 'patch'.  This allows
    % plotting multi-valued surface data.  In this case, u/v surface
    % data has multiple values where two surfaces intersect at a single
    % point.
    x=[p(1,con(1,:))' p(1,con(2,:))' p(1,con(3,:))'];
    y=[p(2,con(1,:))' p(2,con(2,:))' p(2,con(3,:))'];
    z=[p(3,con(1,:))' p(3,con(2,:))' p(3,con(3,:))'];
    u=[uv1(1,:)' uv2(1,:)' uv3(1,:)'];
    v=[uv1(2,:)' uv2(2,:)' uv3(2,:)'];

    figure
    patch(x',y',z',u','EdgeColor','none')
    axis off
    axis equal
    h = plotwakes( wedata, p );
    set(h,'LineWidth',5);
    %set(h,'Color','k');
    title('U surface parameter with wake lines')

    figure
    patch(x',y',z',v','EdgeColor','none')
    %patch(x',y',z',v')
    axis off
    axis equal
    h = plotwakes( wedata, p );
    set(h,'LineWidth',5);
    %set(h,'Color','k');
    title('V surface parameter with wake lines')
end

end % End of readvspgeom

% Helper function to plot wake data
function h = plotwakes( wedata, p )
h=[];

hold on
for i=1:length(wedata)
    wpts = wedata{i};
    h = [h plot3(p(1,wpts)',p(2,wpts)',p(3,wpts)')];
end
hold off

end
