function [con, p, u, v, wedata, partid, utagid, itagname, itag, altfaceid,...
    alttri, altpartid, altutagid, altu, altv] = readvspgeom( fname, plotflag )
%readvspgeom reads a *.vspgeom file into Matlab
%
%   *.vspgeom files are a new file format to facilitate communication
%   between OpenVSP and VSPAERO.  Specifically, this file was designed
%   with three enhancements over existing file formats:
%    1. Multi-valued nodal surface U/V parameter value.
%    2. Explicit wake attachment line identification.
%    3. Support for polygons, not just triangles.
%
%   Though the file format does not require explicit support for it, the
%   intent is for *.vspgeom files to support mixed thick/thin
%   representation of geometry.  This might be used (for example) to
%   represent an aircraft where the fuselage is think and the wing,
%   empennage, and rotors are thin.
%
%   [t, p, u, v, wedata, partid, utagid, itagname, itag,...
%      altfaceid, alttri, altpartid, altutagid, altu, altv ] = readvspgeom( fname, plotflag )
%
%   Reads in a *.vspgeom file passed in fname.  If plotflag is true, then
%   a series of simple plots are generated.  If plotflag is not passed,
%   the plots are not generated.
%
%     con        Polygon connectivity matrix
%     p          Point matrix
%     u          U surface data at polygon nodes
%     v          V surface data at polygon nodes
%     wedata     Wake edge data cell array
%     partid     Part id vector
%     utagid     Unique tag id vector
%     itagname   Individual tag name cell array
%     itag       Individual tag face set cell array
%     altfaceid  Alternate triangulation face id
%     alttri     Alternate triangulation connectivity matrix
%     altpartid  Alternate triangulation part id vector
%     altutagid  Alternate triangulation unique tag id vector
%     altu       Alternate triangulation U surface data
%     altv       Alternate triangulation V surface data
%

%   Rob McDonald
%   6 November 2020 v. 1.0
%   6 May      2023 v. 1.1 - Part and tag ids
%   6 May      2023 v. 1.2 - Handle arbitrary polygons.
%   7 May      2023 v. 1.3 - Handle taglist and tag files.
%   8 May      2023 v. 1.4 - Handle alternate triangulation.
%   8 May      2023 v. 1.5 - Handle tagfiles with local paths.
%   8 May      2023 v. 1.6 - Handle file versioning.
%  26 May      2023 v. 1.7 - Read in alt triangulation aux data.

if ( nargin < 2 )
    plotflag = false;
end

fp = fopen(fname,'r');

ver = fgets(fp);
if ( strfind( ver, 'v2' ) )
    filever = 2;
else
    filever = 1;
    frewind( fp );
end

% Read in number of points
npt = fscanf(fp, '%d', 1);

% Read in the point coordinates.
ptdata = fscanf(fp, '%f', [3 npt]);
p = ptdata(1:3,:);

% Read in number of polygons
npoly = fscanf(fp, '%d', 1);

% Mark this point in the file
mark = ftell(fp);

% Read in the first column of next npoly lines as number of points per line
np = cell2mat( textscan(fp,'%d %*[^\n]',npoly) );

% Find the maximum number of points per line
mnp = max(np);

% Return to marked point in file.
fseek(fp,mark,'bof');

% Initialize connectivity matrix
con = -nan(mnp,npoly);

% Read in connectivity data
for i=1:npoly
    c = fscanf(fp, '%d', [(np(i)+1) 1]);
    con(1:np(i),i) = c(2:end);
end

% Initialize id and poly node data
partid = -ones(npoly,1);
utagid = -ones(npoly,1);
u = nan(mnp,npoly);
v = nan(mnp,npoly);

% Read in the surface id's and poly node data
for i=1:npoly
    % Face ID
    if ( filever == 2 )
        partid(i) = fscanf(fp,'%d',[1 1]);
    end
    utagid(i) = fscanf(fp,'%d',[1 1]);

    % U/V of each polygon node
    uv = fscanf(fp,'%f',[(2*np(i)) 1]);
    u(1:np(i),i) = uv(1:2:end);
    v(1:np(i),i) = uv(2:2:end);
end

% Pad polygons with duplicated final point to make them all mnp-gons.
% This makes for simplified plotting later when using the
% patch(x,y,z,c) form.  The patch('Faces','Vertices') form does not need
% this padding and will work with NAN in con.
for i=2:mnp
    mask = isnan(con(i,:));
    con(i,mask) = con(i-1,mask);
    %mask = isnan(u(i,:));
    u(i,mask) = u(i-1,mask);
    %mask = isnan(v(i,:));
    v(i,mask) = v(i-1,mask);
end

% Read in the number of wakes
nwake = fscanf(fp, '%d', 1);

% Loop over wakes reading in points
nwpt = ones(nwake,1);
wedata = cell(nwake, 1);
for iw = 1:nwake
    nwpt(iw) = fscanf(fp, '%d', 1 );
    wedata{iw} = fscanf(fp, '%d', nwpt(iw) );
end

altfaceid = [];
alttri = [];

if ( filever == 2 )
    read_altfaceid = ones(npoly, 1);
    read_nalttri = ones(npoly, 1);
    read_alttri = cell(npoly,1);

    for i=1:npoly
        read_altfaceid(i) = fscanf(fp, '%d', 1 );
        read_nalttri(i) = fscanf(fp, '%d', 1);
        read_alttri{i} = fscanf(fp, '%d', read_nalttri(i) * 3 );
    end

    nalt = sum(read_nalttri);


    altfaceid = ones(nalt, 1);
    alttri = ones(nalt,3);

    ialt = 1;
    for i=1:npoly

        atv = read_alttri{i};

        for j=1:read_nalttri(i)
            altfaceid(ialt) = read_altfaceid(i);
            tindex = 1 + 3 * (j - 1);
            alttri(ialt,:) = atv(tindex:(tindex+2));
            ialt = ialt + 1;
        end
    end


    % Initialize id and poly node data

    % Max number of alternate triangles for a polygon.
    mnat = max(read_nalttri);

    read_altfaceid = ones(npoly, 1);
    read_altpartid = ones(npoly, 1);
    read_altutagid = ones(npoly,1);

    altpartid = -ones(nalt,1);
    altutagid = -ones(nalt,1);
    altu = nan(3,nalt);
    altv = nan(3,nalt);

    ialt = 1;

    % Read in the surface id's and poly node data
    for i=1:npoly
        % Face ID
        read_altfaceid(i) = fscanf( fp, '%d', [1 1] );
        read_altpartid(i) = fscanf( fp, '%d', [1 1] );
        read_altutagid(i) = fscanf( fp, '%d', [1 1] );

        % U/V of each polygon node
        read_uv = fscanf(fp,'%f',[(2*3*read_nalttri(i)) 1]);

        read_altu = read_uv(1:2:end);
        read_altv = read_uv(2:2:end);

        for j=1:read_nalttri(i)

            % altfaceid(ialt) = read_altfaceid(i);
            altpartid(ialt) = read_altpartid(i);
            altutagid(ialt) = read_altutagid(i);

            tindex = 1 + 3 * (j - 1);
            altu(:,ialt) = read_altu(tindex:(tindex+2));
            altv(:,ialt) = read_altv(tindex:(tindex+2));

            ialt = ialt + 1;
        end
    end
end

fclose(fp);

if ( plotflag )

    figure
    patch('Faces',con','Vertices',p','FaceColor','w')
    axis equal
    axis off
    h = plotwakes( wedata, p );
    set(h,'LineWidth',5);
    %set(h,'Color','k');
    title('Mesh with wake lines')

    figure
    patch('Faces',con','Vertices',p','FaceVertexCData',partid,'FaceColor','flat'); %,'EdgeColor','none')
    axis equal
    axis off
    h = plotwakes( wedata, p );
    set(h,'LineWidth',5);
    %set(h,'Color','k');
    title('Mesh colored by part ID with wake lines')

    figure
    patch('Faces',con','Vertices',p','FaceVertexCData',utagid,'FaceColor','flat'); %,'EdgeColor','none')
    axis equal
    axis off
    h = plotwakes( wedata, p );
    set(h,'LineWidth',5);
    %set(h,'Color','k');
    title('Mesh colored by tag ID with wake lines')

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

    x = nan(mnp,npoly);
    y = nan(mnp,npoly);
    z = nan(mnp,npoly);

    for i=1:mnp
        x(i,:) = p(1,con(i,:));
        y(i,:) = p(2,con(i,:));
        z(i,:) = p(3,con(i,:));
    end

    figure
    patch(x,y,z,u,'EdgeColor','none')
    axis off
    axis equal
    h = plotwakes( wedata, p );
    set(h,'LineWidth',5);
    %set(h,'Color','k');
    title('U surface parameter with wake lines')

    figure
    patch(x,y,z,v,'EdgeColor','none')
    axis off
    axis equal
    h = plotwakes( wedata, p );
    set(h,'LineWidth',5);
    %set(h,'Color','k');
    title('V surface parameter with wake lines')


    if ( filever == 2 )
        figure
        trisurf(alttri, p(1,:), p(2,:), p(3,:),altpartid); % 'EdgeColor','none');
        axis equal
        axis off
        h = plotwakes( wedata, p );
        set(h,'LineWidth',5);
        %set(h,'Color','k');
        title('Alternate triangular colored by part ID with wake lines')

        figure
        trisurf(alttri, p(1,:), p(2,:), p(3,:),altutagid); % 'EdgeColor','none');
        axis equal
        axis off
        h = plotwakes( wedata, p );
        set(h,'LineWidth',5);
        %set(h,'Color','k');
        title('Alternate triangular colored by tag ID with wake lines')

        % Format data in manner appropriate for 'patch'.  This allows
        % plotting multi-valued surface data.  In this case, u/v surface
        % data has multiple values where two surfaces intersect at a single
        % point.
        x=[p(1,alttri(:,1))' p(1,alttri(:,2))' p(1,alttri(:,3))'];
        y=[p(2,alttri(:,1))' p(2,alttri(:,2))' p(2,alttri(:,3))'];
        z=[p(3,alttri(:,1))' p(3,alttri(:,2))' p(3,alttri(:,3))'];

        figure
        patch(x',y',z',altu,'EdgeColor','none')
        axis off
        axis equal
        h = plotwakes( wedata, p );
        set(h,'LineWidth',5);
        %set(h,'Color','k');
        title('Alternate triangular U surface parameter with wake lines')

        figure
        patch(x',y',z',altv,'EdgeColor','none')
        axis off
        axis equal
        h = plotwakes( wedata, p );
        set(h,'LineWidth',5);
        %set(h,'Color','k');
        title('Alternate triangular V surface parameter with wake lines')
    end
end

[filepath,basename,~] = fileparts(fname);

taglistname = [filepath filesep basename '.taglist'];


itagname=[];
itag=[];


% Check for taglist file.
if ( exist(taglistname, 'file') )

    fp = fopen( taglistname );

    % Read in number of tag files
    ntf = fscanf(fp, '%d', 1);

    itag = cell(ntf,1);
    itagname = cell(ntf,1);

    % Loop over tag files
    for i=1:ntf

        tagfile = fscanf(fp, '%s', 1);

        [~,tagbasename,~] = fileparts(tagfile);

        tagfilename = [filepath filesep tagbasename '.tag'];


        if ( exist(tagfilename, 'file') )
            tfp = fopen( tagfilename );

            % Read in tag data.
            itag{i} = fscanf(tfp, '%d');

            justtag = tagbasename((1+length(basename)):end);
            itagname{i} = justtag;

            if ( plotflag )
                figure
                patch('Faces',con(:,itag{i})','Vertices',p','FaceColor','w')
                axis equal
                axis off
                h = plotwakes( wedata, p );
                set(h,'LineWidth',5);
                %set(h,'Color','k');
                title(['Tag ' itagname{i}], 'Interpreter', 'none')
            end

            fclose(tfp);
        end
    end
    fclose(fp);
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
