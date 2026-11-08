clear all
format compact
close all

[V,F] = read_vertices_and_faces_from_obj_file('Parametric Person/Michigan/Male_800Subj_200PC_4Var_2017_06_26/mean_mesh.obj');

N = {'HeadNeckJoint'
'C7T1Jnt'
'T12L1Jnt'
'L5S1Jnt'
'HiptRt'
'HipLt'
'ASISBoneRt'
'ASISBoneLt'
'PSISBoneRt'
'PSISBoneLt'
'ShoulderRtJnt'
'ShoulderLtJnt'
'ElbowJntRt'
'ElbowJntLt'
'WristJntRt'
'WristJntLt'
'KneeJntRt'
'KneeJntLt'
'AnkleJntRt'
'AnkleJntLt'
'Sellion'             % Head start
'Rt_Infraorbitale'
'Lt_Infraorbitale'
'Supramenton'
'Rt_Tragion'
'Rt_Gonion'
'Lt_Tragion'
'Lt_Gonion'
'Nuchale'             % Head end
'Rt_Clavicale'
'Suprasternale'
'Lt_Clavicale'
'Rt_Thelion/Bustpoint'
'Lt_Thelion/Bustpoint'
'Substernale'
'Rt_10thRib'
'Rt_ASIS'
'Lt_10thRib'
'Lt_ASIS'
'Rt_Iliocristale'
'Rt_Trochanterion'
'Lt_Iliocristale'
'Lt_Trochanterion'
'Cervicale'
'RibMidspine10th'
'Rt_PSIS'
'Lt_PSIS'
'Waist_Preferred_Post_'
'Rt_Acromion'
'Rt_Axilla_Ant'
'Rt_RadialStyloid'
'Rt_Axilla_Post_'
'Rt_Olecranon'
'Rt_HumeralLateralEpicn'
'Rt_HumeralMedialEpicn'
'Rt_Radiale'
'Rt_MetacarpalPhal_II'
'Rt_Dactylion'
'Rt_UlnarStyloid'
'Rt_Metacarpal_Phal_V'
'Lt_Acromion'
'Lt_Axilla_Ant'
'Lt_RadialStyloid'
'Lt_Axilla_Post_'
'Lt_Olecranon'
'Lt_HumeralLateralEpicn'
'Lt_HumeralMedialEpicn'
'Lt_Radiale'
'Lt_Metacarpal_Phal_II'
'Lt_Dactylion'
'Lt_UlnarStyloid'
'Lt_Metacarpal_Phal_V'
'Rt_KneeCrease'
'Rt_FemoralLateralEpicn'
'Rt_FemoralMedialEpicn'
'Rt_Metatarsal_Phal_V'
'Rt_LateralMalleolus'
'Rt_MedialMalleolus'
'Rt_Sphyrion'
'Rt_Metatarsal_Phal_I'
'Rt_Calcaneous_Post_'
'Rt_DigitII'
'Lt_KneeCrease'
'Lt_FemoralLateralEpicn'
'Lt_FemoralMedialEpicn'
'Lt_Metatarsal_Phal_V'
'Lt_LateralMalleolus'
'Lt_MedialMalleolus'
'Lt_Sphyrion'
'Lt_Metatarsal_Phal_I'
'Lt_Calcaneous_Post_'
'Lt_DigitII'
'Crotch'
'NewElbow1'
'NewElbow2'
'Rt_Eye'
'Lt_Eye'
'Design_Eye'};

% M=[77.369  0  1694.048
% 91.406  0  1563.185
% 139.685  0  1237.789
% 119.471  0  1068.48
% 123.039  90.258  962.934
% 123.039  -90.258  962.934
% 51.718  122.143  1024.262
% 51.718  -122.143  1024.262
% 191.576  58.551  1078.687
% 191.576  -58.551  1078.687
% 129.411  202.7  1454.872
% 129.411  -202.7  1454.872
% 161.187  285.529  1201.811
% 161.187  -285.529  1201.811
% 53.172  402.016  956.364
% 53.172  -402.016  956.364
% 135.53  93.107  523.447
% 135.53  -93.107  523.447
% 161.795  93.316  86.034
% 161.795  -93.316  86.034
% -29.355  0  1740.237
% -15.306  34.148  1710.898
% -15.306  -34.148  1710.898
% -29.979  0  1640.26
% 64.781  75.675  1715.509
% 58.316  63.15  1643.992
% 64.781  -75.675  1715.509
% 58.316  -63.15  1643.992
% 169.062  0  1702.707
% 47.249  27.349  1524.98
% 42.159  0  1514.1
% 47.249  -27.349  1524.98
% -19.929  119.716  1348.753
% -19.929  -119.716  1348.753
% -29.6  0  1296.575
% 13.751  127.396  1191.977
% 20.057  136.45  1047.996
% 13.751  -127.396  1191.977
% 20.057  -136.45  1047.996
% 111.738  178.018  1134.436
% 108.958  189.851  988.491
% 111.738  -178.018  1134.436
% 108.958  -189.851  988.491
% 164.958  0  1601.248
% 198.683  0  1195.401
% 215.097  58.935  1085.068
% 215.097  -58.935  1085.068
% 208.872  0  1100.207
% 131.252  217.699  1526.607
% 64.183  207.032  1396.409
% 23.06  394  959.349
% 201.987  203.856  1393.536
% 168.458  284.499  1194.966
% 143.764  319.009  1217.283
% 148.385  251.096  1187.99
% 138.612  326.451  1202.141
% -1.478  422.637  872.002
% 25.767  465.875  773.103
% 83.284  410.031  953.379
% 87.49  419.754  877.701
% 131.252  -217.699  1526.607
% 64.183  -207.032  1396.409
% 23.06  -394  959.349
% 201.987  -203.856  1393.536
% 168.458  -284.499  1194.966
% 143.764  -319.009  1217.283
% 148.385  -251.096  1187.99
% 138.612  -326.451  1202.141
% -1.478  -422.637  872.002
% 25.767  -465.875  773.103
% 83.284  -410.031  953.379
% 87.49  -419.754  877.701
% 199.349  105.763  523.506
% 136.559  153.132  527.59
% 134.114  32.628  519.043
% 69  185.768  20.268
% 177.285  127.313  79.131
% 146.304  59.318  92.936
% 144.707  61.305  73.78
% 12.66  95.733  26.212
% 221.459  84.369  35.961
% -37.515  164.128  11.602
% 199.349  -105.763  523.506
% 136.559  -153.132  527.59
% 134.114  -32.628  519.043
% 69  -185.768  20.268
% 177.285  -127.313  79.131
% 146.304  -59.318  92.936
% 144.707  -61.305  73.78
% 12.66  -95.733  26.212
% 221.459  -84.369  35.961
% -37.515  -164.128  11.602
% 110.28  0  843.749];

% Mean Landmarks.tsv
M=[80.576  0.000  1621.075;
94.539  0.000  1497.965;
142.433  0.000  1185.511;
121.256  0.000  1023.138;
125.740  89.879  918.106;
125.740  -89.879  918.106;
54.740  120.772  979.336;
54.740  -120.772  979.336;
193.096  57.068  1033.682;
193.096  -57.068  1033.682;
130.428  195.751  1392.092;
130.428  -195.751  1392.092;
163.129  281.188  1152.790;
163.129  -281.188  1152.790;
61.201  395.637  917.072;
61.201  -395.637  917.072;
136.858  91.691  500.338;
136.858  -91.691  500.338;
161.621  92.955  82.998;
161.621  -92.955  82.998;
-23.821  0.000  1667.392;
-10.820  34.108  1637.450;
-10.820  -34.108  1637.450;
-24.542  0.000  1568.140;
68.092  75.362  1642.255;
61.918  62.673  1572.392;
68.092  -75.362  1642.255;
61.918  -62.673  1572.392;
171.902  0.000  1630.677;
51.166  26.746  1459.833;
46.045  0.000  1449.538;
51.166  -26.746  1459.833;
-14.259  117.555  1289.516;
-14.259  -117.555  1289.516;
-26.122  0.000  1240.626;
19.641  128.139  1144.154;
27.892  134.004  1003.864;
19.641  -128.139  1144.154;
27.892  -134.004  1003.864;
114.417  173.517  1084.822;
111.166  184.263  948.790;
114.417  -173.517  1084.822;
111.166  -184.263  948.790;
166.994  0.000  1535.543;
201.354  0.000  1148.510;
215.770  57.144  1039.408;
215.770  -57.144  1039.408;
208.413  0.000  1058.212;
132.428  210.750  1462.905;
68.547  202.112  1333.885;
31.858  387.944  919.747;
200.872  199.165  1330.351;
169.247  278.645  1146.280;
144.359  311.849  1167.628;
150.494  246.022  1138.833;
139.375  319.303  1152.536;
8.275  416.439  836.576;
36.733  461.390  742.175;
90.544  403.330  914.396;
95.350  413.742  842.517;
132.428  -210.750  1462.905;
68.547  -202.112  1333.885;
31.858  -387.944  919.747;
200.872  -199.165  1330.351;
169.247  -278.645  1146.280;
144.359  -311.849  1167.628;
150.494  -246.022  1138.833;
139.375  -319.303  1152.536;
8.275  -416.439  836.576;
36.733  -461.390  742.175;
90.544  -403.330  914.396;
95.350  -413.742  842.517;
198.728  103.975  499.909;
138.074  150.163  504.437;
135.472  33.006  496.102;
73.135  182.902  20.117;
176.723  126.436  76.095;
146.518  59.474  89.901;
145.000  61.315  70.812;
17.648  94.717  25.442;
219.608  83.760  34.581;
-30.255  161.587  11.174;
198.728  -103.975  499.909;
138.074  -150.163  504.437;
135.472  -33.006  496.102;
73.135  -182.902  20.117;
176.723  -126.436  76.095;
146.518  -59.474  89.901;
145.000  -61.315  70.812;
17.648  -94.717  25.442;
219.608  -83.760  34.581;
-30.255  -161.587  11.174;
112.467  0.000  803.438];

M( end + 1, : ) = ( M( 13, : ) + V( 608 + 1, : ) ) * 0.5;  % 94
M( end + 1, : ) = ( M( 14, : ) + V( 2546 + 1, : ) ) * 0.5; % 95

M( end + 1, : ) = V( 8987 + 1, : ); % 96
M( end + 1, : ) = V( 9107 + 1, : ); % 97
M( end + 1, : ) = ( V( 8987 + 1, : ) + V( 9107 + 1, : ) ) * 0.5; % 98

x=M(:,1);
y=M(:,2);
z=M(:,3);

sk1 = [98 96 1 2 3 4 5 17 19 82];
sk2 = [4 6 18 20 92];
sk3 = [58 15 13 11 2 12 14 16 70];
sk4 = [97 96];

skel_pinnochio = [2 3 4 1 6 18 20 92 5 17 19 82 12 95 16 70 11 94 15 58];

skel_prev =     [-1 0 1 0 2  4  5  6 2  8  9 10  0 12 13 15  0 15 16 14];
fp = fopen('skel_pinnochio.out', 'w' );
for i=1:length(skel_pinnochio)
    is = skel_pinnochio(i);
    fprintf(fp, '%d %f %f %f %d\n', i-1, y(is), z(is), -x(is), skel_prev(i) );  % Transformed.
%   fprintf(fp, '%d %f %f %f %d\n', i-1, x(is), y(is), z(is), skel_prev(i) );  % Un-transformed.
end
fclose( fp );


figure(1)
plot3(x,y,z,'x')
hold on
plot3(x(sk1),y(sk1),z(sk1),'k',x(sk2),y(sk2),z(sk2),'k',x(sk3),y(sk3),z(sk3),'k')
hold off

text(x,y,z,N,'Interpreter', 'none')
axis equal
axis off

npt = length(N);

figure(2)
plot3(x,y,z,'x')
hold on
trisurf(F,V(:,1),V(:,2),V(:,3),'FaceColor',0.7*[1 1 1],'Facealpha',0.3,'EdgeColor','none')
plot3(x(sk1),y(sk1),z(sk1),'k',x(sk2),y(sk2),z(sk2),'k',x(sk3),y(sk3),z(sk3),'k',x(sk4),y(sk4),z(sk4),'k')
hold off
text(x,y,z,num2str((0:npt-1)'),'Interpreter', 'none')
lighting gouraud
% text(x,y,z,N,'Interpreter', 'none')
axis equal
axis off


ptlist = [1172, 1251, 1186, 3104, 122, 912, 325, 2267, 803, 808, 795, 805,...
    669, 666, 659, 662, 63, 30, 1973, 3, 574, 582, 605, 577, 162, 368, 380, 150,...
    1665, 1663, 1674, 1671, 1771, 1776, 1773, 1785, 13522, 13906, 14405, 14423,...
     14090, 14133, 13994, 13364, 608, 2546, 8987, 9107] + 1;
npt = length(ptlist);


figure(3)
hold on
trisurf(F,V(:,1),V(:,2),V(:,3),'FaceColor',0.7*[1 1 1],'EdgeColor','none', 'Facealpha',0.3)
plot3(V(ptlist,1),V(ptlist,2),V(ptlist,3),'b.','MarkerSize',20);
text(V(ptlist,1),V(ptlist,2),V(ptlist,3),num2str((0:npt-1)'),'Interpreter', 'none')

axis equal
axis off



% sk1 = [1 2 3 4 5 17 19];
% sk2 = [4 6 18 20];
% sk3 = [15 13 11 2 12 14 16];


figure(5)
plot3(x(sk1),y(sk1),z(sk1),'k',x(sk2),y(sk2),z(sk2),'k:',x(sk3),y(sk3),z(sk3),'k--',x(sk4),y(sk4),z(sk4),'k-.')
hold on
text(x(sk1),y(sk1),z(sk1),num2str(sk1'))
text(x(sk2),y(sk2),z(sk2),num2str(sk2'))
text(x(sk3),y(sk3),z(sk3),num2str(sk3'))
hold off

axis equal
axis off


figure(6)
trisurf(F,V(:,1),V(:,2),V(:,3),'FaceColor',0.75*[1 1 1],'Facealpha',0.5); %,'EdgeColor','none')
hold on
plot3(x(sk1),y(sk1),z(sk1),'b',x(sk2),y(sk2),z(sk2),'b',x(sk3),y(sk3),z(sk3),'b',x(sk4),y(sk4),z(sk4),'b', 'LineWidth', 2.5)
plot3(x(sk1),y(sk1),z(sk1),'y.',x(sk2),y(sk2),z(sk2),'y.',x(sk3),y(sk3),z(sk3),'y.',x(sk4),y(sk4),z(sk4),'y.', 'MarkerSize', 25)

hold off
lighting gouraud
axis equal
axis off




figure(4)
hold on
trisurf(F,V(:,1),V(:,2),V(:,3),'FaceColor',0.7*[1 1 1],'Facealpha',0.7); %,'EdgeColor','none')
plot3(x(sk1),y(sk1),z(sk1),'y.',x(sk2),y(sk2),z(sk2),'y.',x(sk3),y(sk3),z(sk3),'y.',x(sk4),y(sk4),z(sk4),'y.', 'MarkerSize', 25)
plot3(V(ptlist,1),V(ptlist,2),V(ptlist,3),'b.','MarkerSize',40);

plot3([x(13) x(94) V(608+1,1) ], [y(13) y(94) V(608+1,2) ], [z(13) z(94) V(608+1,3) ],'k.-', 'MarkerSize', 30)
plot3([x(14) x(95) V(2546+1,1) ], [y(14) y(95) V(2546+1,2) ], [z(13) z(95) V(2546+1,3) ],'k.-', 'MarkerSize', 30)
plot3([x(1) V(8987+1,1) ], [y(1) V(8987+1,2) ], [z(1) V(8987+1,3) ],'k.-', 'MarkerSize', 30)
plot3([x(1) V(9107+1,1) ], [y(1) V(9107+1,2) ], [z(1) V(9107+1,3) ],'k.-', 'MarkerSize', 30)


clickA3DPoint(V');
% hold off
lighting gouraud
axis equal
axis off



npt = length(N);

% sk1 = [1 2 3 4 5 17 19 82];
% sk2 = [4 6 18 20 92];
% sk3 = [15 13 11 2 12 14 16];
% 
% % skel_pinnochio = [2 3 4 1 6 18 20 92 5 17 19 82 12 14 16 11 13 15];
% skel_pinnochio = [2 3 4 1 6 18 20 92 5 17 19 82 12 95 16 11 94 15];
% 
% skel_prev = [-1 0 1 0 2 4 5 6 2 8 9 10 0 12 13 0 15 16];

figure(7)
plot3(x(sk1),y(sk1),z(sk1),'k',x(sk2),y(sk2),z(sk2),'k',x(sk3),y(sk3),z(sk3),'k',x(sk4),y(sk4),z(sk4),'k')
hold on
for i=1:length(skel_pinnochio)
    text(x(skel_pinnochio(i)),y(skel_pinnochio(i)),z(skel_pinnochio(i)),num2str(i-1));
end
hold off

axis equal
axis off
