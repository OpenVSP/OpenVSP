clear all
format compact
close all

% kreduce = 1.0;
kreduce = 0.4;


writefile = true;

[Vm,Fm] = read_vertices_and_faces_from_obj_file('Parametric Person/Michigan/Male_800Subj_200PC_4Var_2017_06_26/mean_mesh.obj');
[Vf,Ff] = read_vertices_and_faces_from_obj_file('Parametric Person/Michigan/Female_800Subj_200PC_4Var_2017_06_26/mean_mesh_tris.obj');

V = (Vm + Vf) * 0.5;
F = Fm;

figure(1)
trisurf(F,V(:,1),V(:,2),V(:,3),'FaceColor',0.7*[1 1 1]); 

axis equal
axis off


tr = triangulation(F,V);

fv.faces = F;
fv.vertices = V;

% nfv = reducepatch( fv, 0.5, 'verbose' );


numf = length(F);
mask = true(numf,1);
for ifac = 1:numf
   % cen = sum(V(F(ifac,:),:))/3.0;
   ceny = sum(V(F(ifac,:),2));    % Only compare on y.  Don't /3 as it doesn't matter for <0 comparison
   if ( ceny < 0 )
       mask(ifac) = false;
   end
end

nfv.faces = F(mask,:);
nfv.vertices = V;

nfv = reducepatch( nfv, kreduce, 'verbose' );

% Some triangles were invalid and repeated.  The following code finds and
% eliminates those tris.

% Sort triangle index such that CW and CCW tris are identical -- also,
% starting index won't matter.  Then find unique rows.
[~,idxu,idxc] = unique( sort( nfv.faces, 2 ), 'rows' );  
[count, ~, idxcount] = histcounts(idxc,numel(idxu));
idxkeep = count(idxcount)==1; % Only keep non-repeated 

nfv.faces = nfv.faces(idxkeep,:);


% % Node 918 is giving trouble, delete all triangles using it.
% % find linear index of uses of node 918
% itri = find( nfv.faces == 918 );
% % Find triangles (rows)
% [ rtri, ~ ] = ind2sub( size( nfv.faces ), itri );
% 
% % Delete tris using node
% mask = true( size( nfv.faces, 1 ), 1 );
% mask(rtri) = false;
% nfv.faces = nfv.faces( mask, : );
% 
% % Delete node
% mask = true( size( nfv.vertices, 1 ), 1 );
% mask( 918 ) = false;
% nfv.vertices = nfv.vertices( mask, : );
% 
% % Re-number nodes for deletion.
% mask = nfv.faces > 918;
% nfv.faces(mask) = nfv.faces(mask) - 1;
% % End of removal of 918


figure(2)
trisurf(nfv.faces,nfv.vertices(:,1),nfv.vertices(:,2),nfv.vertices(:,3),'FaceColor',0.7*[1 1 1]); 

axis equal
axis off

mask = V(:,2) < -.001;
Vshift = V;
Vshift(mask,2) = V(mask,2)-100000;

posindx = @(X) X(:,1) + X(:,2)*100 + X(:,3)*100*100;

Vp = posindx(Vshift);
vpred = posindx(nfv.vertices);

nvred = length(vpred);
ivred = zeros(nvred,1);

for i=1:nvred
    ivred(i) = find(vpred(i)==Vp);
end

fremap = nfv.faces;
nf = numel(fremap);

for i=1:nf
    fremap(i) = ivred(fremap(i));
end


figure(3)
trisurf(fremap,V(:,1),V(:,2),V(:,3),'FaceColor',0.7*[1 1 1]); 

axis equal
axis off

fp = fopen('reduced_mean_human.out', 'w' );
fprintf(fp, '%d %d %d\n', fremap );
fclose( fp );


% input_vector = np.array([1850, bmi, 0.52, age, bmi*age, 1]) # male 4var, includes BMI*age
% self.scores = self.regression_coefficients.dot(input_vec)
% self.res = self.mean_vector + self.pcs[:,:num_scores].dot(scores) 

% Order in data files.
% num_anthro
% num_landmark*3
% num_vert*3

% num_anthro = 49;  % For Male model.
num_anthro = 51;
num_landmark = 93;
num_vert = 14427;

anthro_indx = 1:num_anthro;
landmark_indx = (1:(num_landmark*3)) + anthro_indx(end);
vert_indx =     (1:(num_vert*3)) + landmark_indx(end);


% Verified that this is 43609
% nentry = num_anthro + num_landmark*3 + num_vert*3


% 43609
[C] = dlmread('Parametric Person/Michigan/Female_800Subj_200PC_4Var_2017_06_26/mean_vector.tsv');

% 43609x200
[PCS] = dlmread('Parametric Person/Michigan/Female_800Subj_200PC_4Var_2017_06_26/pcs.tsv');

% 200x6
[coeffs] = dlmread('Parametric Person/Michigan/Female_800Subj_200PC_4Var_2017_06_26/regression_coeffs.tsv');


mean_landmarks = C( landmark_indx );
mean_verts = C( vert_indx );

pcs_landmarks = PCS( landmark_indx, : );
pcs_verts = PCS( vert_indx, : );

mean_landmarks_v = reshape( mean_landmarks, 3, [] )';   % index order and transpose significant.
mean_verts_v = reshape( mean_verts, 3, [] )';

pcs_landmarks_v = reshape( pcs_landmarks', 200, 3, [] );
pcs_verts_v = reshape( pcs_verts', 200, 3, [] );

% permute to put in easier to use index order
pcs_landmarks_v = permute( pcs_landmarks_v, [3, 2, 1]);
pcs_verts_v = permute( pcs_verts_v,[ 3, 2, 1]);

% Create new landmarks to form more skeleton joints.
% M( end + 1, : ) = ( M( 13, : ) + V( 608 + 1, : ) ) * 0.5;  % 94
% M( end + 1, : ) = ( M( 14, : ) + V( 2546 + 1, : ) ) * 0.5; % 95

% New landmarks (center of elbows) are mid-point between elbow landmark and
% elbow-pit mesh vertex.


% Average of mean points.
new_mean_landmarks_v(1,:) = ( mean_landmarks_v( 13, : ) + mean_verts_v( 608 + 1, : ) ) * 0.5;
new_mean_landmarks_v(2,:) = ( mean_landmarks_v( 14, : ) + mean_verts_v( 2546 + 1, : ) ) * 0.5;

new_mean_landmarks_v(3,:) = mean_verts_v( 8987 + 1, : );
new_mean_landmarks_v(4,:) = mean_verts_v( 9107 + 1, : );
new_mean_landmarks_v(5,:) = ( mean_verts_v( 8987 + 1, : ) + mean_verts_v( 9107 + 1, : ) ) * 0.5;

% Average of weights
new_pcs_landmarks_v(1,:,:) = ( pcs_landmarks_v( 13, :, : ) + pcs_verts_v( 608 + 1, :, : ) ) * 0.5;
new_pcs_landmarks_v(2,:,:) = ( pcs_landmarks_v( 14, :, : ) + pcs_verts_v( 2546 + 1, :, : ) ) * 0.5;

new_pcs_landmarks_v(3,:,:) = pcs_verts_v( 8987 + 1, :, : );
new_pcs_landmarks_v(4,:,:) = pcs_verts_v( 9107 + 1, :, : );
new_pcs_landmarks_v(5,:,:) = ( pcs_verts_v( 8987 + 1, :, : ) + pcs_verts_v( 9107 + 1, :, : ) ) * 0.5;

% Append new landmarks.
mean_landmarks_v = [mean_landmarks_v; new_mean_landmarks_v];
pcs_landmarks_v = [pcs_landmarks_v; new_pcs_landmarks_v];

% Skeleton index through landmarks.
skel_indx = [2 3 4 1 6 18 20 92 5 17 19 82 12 95 16 70 11 94 15 58 96 97 98 51 63];


mean_skel_v = mean_landmarks_v( skel_indx, : );
pcs_skel_v = pcs_landmarks_v( skel_indx, :, : );


skel_pindx = posindx( mean_skel_v );

nskel = length(skel_pindx);
iskel = 1:nskel;

for i = 1:nskel
    if ( mean_skel_v(i,2) < -.001 )
        p = mean_skel_v(i,:);
        p(2) = -p(2);
        pid = posindx( p );
        iskel(i) = -find( skel_pindx == pid );
    end
end

% Grab the positive only 
iiskel = unique(iskel(iskel>0));

% Simplify down the skeleton points to keep (positive only)
mean_skel_v = mean_skel_v( iiskel, : );
pcs_skel_v = pcs_skel_v( iiskel, :, : );

for i=1:length(iskel)
    mult = 1;
    sn = sign(iskel(i));
    ip = abs( iskel(i) );
    
    iiiskel(i) = sn * (find( ip == iiskel ) - 1 );    % -1 to zero-index
end

% Overwrite skeleton index to tolerable variable name.
skel_indx = iiiskel;

[iminverts,IA,IC] = unique(fremap);

fremap = reshape( IC, [], 3 );

mean_verts_v = mean_verts_v(iminverts,:);
pcs_verts_v = pcs_verts_v(iminverts,:,:);


figure(4)
trisurf(fremap,mean_verts_v(:,1),mean_verts_v(:,2),mean_verts_v(:,3),'FaceColor',0.7*[1 1 1]); 

axis equal
axis off



% Information that needs to be written out:
%
% mean_verts_v
% pcs_verts_v
% fremap
% 
% mean_skel_v
% pcs_skel_v
% skel_indx
%
% coeffs

pcs_verts_v = permute(pcs_verts_v,[2,1,3]);
pcs_verts_v = reshape(pcs_verts_v,[],200);

pcs_skel_v = permute(pcs_skel_v,[2,1,3]);
pcs_skel_v = reshape(pcs_skel_v,[],200);



if( writefile )
    
    fp = fopen('FemaleGeomData.h', 'w' );
    
    fprintf( fp, ['#if !defined(VSP_FEMALE_GEOM_DATA__INCLUDED_)\n' ...
        '#define VSP_FEMALE_GEOM_DATA__INCLUDED_\n' ...
        '\n'...
        '//////////////////////////////////////////////////////////////////////////////\n' ...
        '// This anthropometric model is based on the model from Human Shapes\n' ...
        '// (http://humanshape.org/), used with permission of Prof. Matthew P. Reed,\n' ...
        '// University of Michigan.  Prof. Reed provided the data files and example\n' ...
        '// Python code to guide this work.\n' ...
        '//\n' ...
        '// To speed execution and minimize memory use, the left-hand side of the\n' ...
        '// mesh was discarded (a symmetrical mirror will be inserted later).  Then\n' ...
        '// the mesh was coarsened to 20%% of the original number of faces using\n' ...
        '// Matlab`s `reducepatch`.  This routine does not move, add, or delete\n' ...
        '// any vertices -- it re-connects existing vertices into a coarser mesh\n' ...
        '// while attempting to preserve features like edges.\n' ...
        '//\n' ...
        '// Since original points were used, the principal component fitting\n' ...
        '// coefficients for those verts can continue to be used without need for\n' ...
        '// interpolation or blending.\n' ...
        '//\n' ...
        '// The skeleton joint locations were taken from landmarks included in the\n' ...
        '// same data set -- with the exception of the elbow joint.  The elbow joint\n' ...
        '// was created as the midpoint between the elbow landmark and a mesh point\n' ...
        '// in the crook of the elbow.  The principal component coefficients were\n' ...
        '// likewise averaged to synthesize values for the new joint location.\n' ...
        '//\n' ...
        '// The skeleton will be used with the Pinocchio auto-rigging tool\n' ...
        '// (https://github.com/elrond79/Pinocchio) to make this model posable.\n' ...
        '//\n' ...
        '// Rob McDonald\n' ...
        '//\n' ...
        '//////////////////////////////////////////////////////////////////////////////\n']);
    
    fprintf(fp, '\nconst REAL_T HumanGeom::m_female_coeffs[200][6] = {\n' );
    for i=1:size(coeffs,1)
        fprintf(fp, '{%.19ef, %.19ef, %.19ef, %.19ef, %.19ef, %.19ef}', coeffs(i,:) );
        if ( i < size(coeffs,1) )
            fprintf(fp, ',\n');
        end
    end
    fprintf(fp, '};\n');
    
%     fprintf(fp, '\nconst int HumanGeom::m_skel_indx[NUM_SKEL] = {' );
%     for i=1:length( skel_indx )
%         fprintf(fp, '%d', skel_indx(i) );  % zero indexed above to work with negative values.
%         if ( i < length(skel_indx) )
%             fprintf(fp, ', ');
%         end
%     end
%     fprintf(fp, '};\n');
    
    fprintf(fp, '\nconst REAL_T HumanGeom::m_female_skel_verts[NUM_SKEL_VERT][3] = {\n' );
    for i=1:size(mean_skel_v,1)
        fprintf(fp, '{%.19ef, %.19ef, %.19ef}', mean_skel_v(i,:) );
        if ( i < size(mean_skel_v,1))
            fprintf(fp, ',\n' );
        end
    end
    fprintf(fp, '};\n');
    
    fprintf(fp, '\nconst REAL_T HumanGeom::m_female_skel_pcs[NUM_SKEL_VERT*3][200] = {\n' );
    for i=1:size(pcs_skel_v,1)
        fprintf(fp,'{');
        for j=1:200
            fprintf(fp, '%.19ef', pcs_skel_v(i,j) );
            if ( j < 200 )
                fprintf(fp, ', ');
            end
        end
        fprintf(fp,'}');
        if ( i < size(pcs_skel_v,1) )
            fprintf(fp, ',\n');
        end
    end
    fprintf(fp, '};\n');
    
%     fprintf(fp, '\nconst int HumanGeom::m_half_tris[NUM_MESH_TRI][3] = {\n' );
%     for i=1:size(fremap,1)
%         fprintf(fp, '{%d, %d, %d}', fremap(i,:) - 1 );
%         if ( i < size(fremap,1))
%             fprintf(fp, ',\n');
%         end
%     end
%     fprintf(fp, '};\n');
    
    fprintf(fp, '\nconst REAL_T HumanGeom::m_female_half_verts[NUM_MESH_VERT][3] = {\n' );
    for i=1:size(mean_verts_v,1)
        fprintf(fp, '{%.19ef, %.19ef, %.19ef}', mean_verts_v(i,:) );
        if ( i < size(mean_verts_v,1))
            fprintf(fp, ',\n');
        end
    end
    fprintf(fp, '};\n');
    
    fprintf(fp, '\nconst REAL_T HumanGeom::m_female_half_pcs[NUM_MESH_VERT*3][200] = {\n' );
    for i=1:size(pcs_verts_v,1)
        fprintf(fp,'{');
        for j=1:200
            fprintf(fp, '%.19ef', pcs_verts_v(i,j) );
            if ( j < 200 )
                fprintf(fp, ', ');
            end
        end
        fprintf(fp,'}');
        if ( i < size(pcs_verts_v,1))
            fprintf(fp, ',\n');
        end
    end
    fprintf(fp, '};\n');
    
    fprintf(fp, '\n#endif // !defined(VSP_FEMALE_GEOM_DATA__INCLUDED_)\n' );
    
    fclose( fp );
    
end




