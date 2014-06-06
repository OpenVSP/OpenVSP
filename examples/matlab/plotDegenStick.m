function h = plotDegenStick( dgfile )

% Load VSP output degenerate geometry.
run(dgfile);

% Initialize flags to prevent undefined access.
plotLE = false; plotTE = false; plotCGSolid = false; plotCGShell = false;
plotInt = false; plotThick = false;

plotLE = true;
plotTE = true;
%plotInt = true;
%plotCGSolid = true;
%plotCGShell = true;
%plotThick = true;

intpt = 0.25;

ngeom = length(degenGeom);

figure(1)
clf
hold on

for i=1:ngeom
  disp(['Component ' num2str(i) ' Name: ' degenGeom(i).name]);

  for j=1:length(degenGeom(i).stick)

    xle = degenGeom(i).stick(j).lex;
    yle = degenGeom(i).stick(j).ley;
    zle = degenGeom(i).stick(j).lez;

    xte = degenGeom(i).stick(j).tex;
    yte = degenGeom(i).stick(j).tey;
    zte = degenGeom(i).stick(j).tez;

    xint = xle + intpt*(xte-xle);
    yint = yle + intpt*(yte-yle);
    zint = zle + intpt*(zte-zle);

    if(plotLE)
      plot3(xle, yle, zle, 'kx-')
    end

    if(plotTE)
      plot3(xte, yte, zte, 'bo-')
    end

    if(plotInt)
      plot3(xint, yint, zint, 'k-.')
    end

    if(plotCGSolid)
      plot3(degenGeom(i).stick(j).cgSolidx,degenGeom(i).stick(j).cgSolidy,degenGeom(i).stick(j).cgSolidz,'r--');
    end

    if(plotCGShell)
      plot3(degenGeom(i).stick(j).cgShellx,degenGeom(i).stick(j).cgShelly,degenGeom(i).stick(j).cgShellz,'y--');
    end

    if(plotThick)
      xt = xle + degenGeom(i).stick(j).tLoc.*(xte-xle);
      yt = yle + degenGeom(i).stick(j).tLoc.*(yte-yle);
      zt = zle + degenGeom(i).stick(j).tLoc.*(zte-zle);

      % Max thickness line
      plot3(xt,yt,zt,'k--');

      % Needs normal from plate.
      dx = 0.5 * degenGeom(i).stick(j).toc .* degenGeom(i).stick(j).chord .* degenGeom(i).plate(j).nx;
      dy = 0.5 * degenGeom(i).stick(j).toc .* degenGeom(i).stick(j).chord .* degenGeom(i).plate(j).ny;
      dz = 0.5 * degenGeom(i).stick(j).toc .* degenGeom(i).stick(j).chord .* degenGeom(i).plate(j).nz;

      % Points above and below max thickness.
      plot3(xt+dx,yt+dy,zt+dz,'kx',xt-dx,yt-dy,zt-dz,'kx');
    end
  end
end
hold off

axis equal
axis off
