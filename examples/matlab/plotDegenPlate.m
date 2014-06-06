function h = plotDegenPlate( dgfile )

% Load VSP output degenerate geometry.
run(dgfile);

% Initialize flags to prevent undefined access.
plotplate = false; plotnormals = false; plotcambsurf = false;
plotsurf = false; plotref = false; ploterr = false;
plotuparm = false; plotwtopparm = false; plotwbotparm = false;

plotplate = true;
%plotnormals = true;
plotcambsurf = true;
%plotsurf = true;
%plotref = true;
%ploterr = true;
%plotuparm = true;
%plotwtopparm = true;
%plotwbotparm = true;

ngeom = length(degenGeom);

figure(1)
clf
hold on

for i=1:ngeom
  disp(['Component ' num2str(i) ' Name: ' degenGeom(i).name]);

  for j=1:length(degenGeom(i).plate)

    if(plotplate)
      if(plotuparm)
        surf(degenGeom(i).plate(j).x, degenGeom(i).plate(j).y, degenGeom(i).plate(j).z, degenGeom(i).plate(j).u);
      elseif(plotwtopparm)
        surf(degenGeom(i).plate(j).x, degenGeom(i).plate(j).y, degenGeom(i).plate(j).z, degenGeom(i).plate(j).wTop);
      elseif(plotwbotparm)
        surf(degenGeom(i).plate(j).x, degenGeom(i).plate(j).y, degenGeom(i).plate(j).z, degenGeom(i).plate(j).wBot);
      else
        surf(degenGeom(i).plate(j).x, degenGeom(i).plate(j).y, degenGeom(i).plate(j).z);
      end

      if(plotnormals)
        quiver3(degenGeom(i).plate(j).x(:,1), degenGeom(i).plate(j).y(:,1), degenGeom(i).plate(j).z(:,1),...
          degenGeom(i).plate(j).nx, degenGeom(i).plate(j).ny, degenGeom(i).plate(j).nz);
      end
    end

    % Reconstruct camber surface
    cx = degenGeom(i).plate(j).x + degenGeom(i).plate(j).zCamber.*degenGeom(i).plate(j).nCamberx;
    cy = degenGeom(i).plate(j).y + degenGeom(i).plate(j).zCamber.*degenGeom(i).plate(j).nCambery;
    cz = degenGeom(i).plate(j).z + degenGeom(i).plate(j).zCamber.*degenGeom(i).plate(j).nCamberz;

    if(plotcambsurf)
      surf(cx,cy,cz);

      if(plotnormals)
        quiver3(cx, cy, cz, degenGeom(i).plate(j).nCamberx, degenGeom(i).plate(j).nCambery, degenGeom(i).plate(j).nCamberz);
      end
    end

    % Reconstruct wing surface
    if(plotsurf)
      ux = cx + 0.5*degenGeom(i).plate(j).t.*degenGeom(i).plate(j).nCamberx;
      uy = cy + 0.5*degenGeom(i).plate(j).t.*degenGeom(i).plate(j).nCambery;
      uz = cz + 0.5*degenGeom(i).plate(j).t.*degenGeom(i).plate(j).nCamberz;

      lx = cx - 0.5*degenGeom(i).plate(j).t.*degenGeom(i).plate(j).nCamberx;
      ly = cy - 0.5*degenGeom(i).plate(j).t.*degenGeom(i).plate(j).nCambery;
      lz = cz - 0.5*degenGeom(i).plate(j).t.*degenGeom(i).plate(j).nCamberz;

      % Rearrange reconstructed surface into ordinary surface order.
      rx = [ux(:,1:end-1) fliplr(lx(:,2:end))];
      ry = [uy(:,1:end-1) fliplr(ly(:,2:end))];
      rz = [uz(:,1:end-1) fliplr(lz(:,2:end))];

      if(j==2)
        shft = [0 size(rx,2)/4];
        rx = circshift(rx,shft);
        ry = circshift(ry,shft);
        rz = circshift(rz,shft);
      end

      rx(:,end+1) = rx(:,1);
      ry(:,end+1) = ry(:,1);
      rz(:,end+1) = rz(:,1);

      if(~ploterr)
        surf(rx,ry,rz);
      end

      if(plotref)
        surf(degenGeom(i).surf.x, degenGeom(i).surf.y, degenGeom(i).surf.z);
      end

      if(ploterr)
        ex = rx - degenGeom(i).surf.x;
        ey = ry - degenGeom(i).surf.y;
        ez = rz - degenGeom(i).surf.z;

        e=sqrt(ex.^2+ey.^2+ez.^2);

        surf(rx,ry,rz,e);

        colorbar;
      end
    end
  end
end
hold off

axis equal
axis off
