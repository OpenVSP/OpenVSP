clear all
format compact


[V,F] = read_vertices_and_faces_from_obj_file('debug.obj');

skel_weights = dlmread('attachment.out');

for i=1:17
    figure(i)
    
    trisurf(F,V(:,1),V(:,2),V(:,3),skel_weights(:,i) ); 
    title(num2str(i-1));
    axis equal
    axis off
    
end