clear all
format compact
close all

[V,F] = read_vertices_and_faces_from_obj_file('Parametric Person/Michigan/Female_800Subj_200PC_4Var_2017_06_26/mean_mesh_tris.obj');

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
'NewElbow2'};

% Mean Landmarks.tsv
M=[108.3815987    0    1487.173551
114.0477664    0    1373.200804
141.583844    0    1085.198303
134.4209768    0    935.917875
144.7154688    87.63000019    835.6801025
144.7154688    -87.63000019    835.6801025
77.74103694    112.6362999    896.93327
77.74103694    -112.6362999    896.93327
203.8341075    59.51065094    948.012485
203.8341075    -59.51065094    948.012485
135.6958649    172.7059817    1279.996736
135.6958649    -172.7059817    1279.996736
156.5124087    262.7351359    1067.637164
156.5124087    -262.7351359    1067.637164
83.63765644    396.691735    867.2814069
83.63765644    -396.691735    867.2814069
152.5460874    87.05859581    453.7927094
152.5460874    -87.05859581    453.7927094
167.7027063    92.95788844    74.05085944
167.7027063    -92.95788844    74.05085944
11.4897308    0    1531.428888
20.68878312    32.72827715    1506.0504
20.68878312    -32.72827715    1506.0504
10.6545174    0    1440.847713
96.95076081    70.14366856    1508.155669
87.55402284    56.10934219    1447.180663
96.95076081    -70.14366856    1508.155669
87.55402284    -56.10934219    1447.180663
195.7199076    0    1496.603981
76.76293102    22.18079642    1341.498825
70.52326923    0    1330.022975
76.76293102    -22.18079642    1341.498825
-22.16887251    93.25669013    1177.138713
-22.16887251    -93.25669013    1177.138713
-8.442604377    0    1139.660145
29.6776094    99.33360732    1051.34174
28.28915328    130.1629759    913.369405
29.6776094    -99.33360732    1051.34174
28.28915328    -130.1629759    913.369405
112.6007259    156.7959191    994.6060144
116.4046816    187.7181427    873.6303123
112.6007259    -156.7959191    994.6060144
116.4046816    -187.7181427    873.6303123
178.9696635    0    1406.611242
200.0914838    0    1056.971546
225.4006169    59.54512013    960.6761694
225.4006169    -59.54512013    960.6761694
201.1113068    0    1010.019463
140.0613179    187.7009606    1344.012463
83.90944461    182.1471263    1231.561619
57.82302219    392.2223    867.7316338
196.8071357    180.6875436    1222.828076
159.149613    260.9875121    1057.028147
147.8202861    288.8868644    1082.837468
139.3107051    233.2755958    1049.530203
143.4511386    297.3409244    1067.375027
43.46601157    425.8353706    793.3997444
75.27798362    479.9978059    720.5617453
109.4522217    401.16074    866.8308819
117.2974448    416.1984881    805.6570544
140.0613179    -187.7009606    1344.012463
83.90944461    -182.1471263    1231.561619
57.82302219    -392.2223    867.7316338
196.8071357    -180.6875436    1222.828076
159.149613    -260.9875121    1057.028147
147.8202861    -288.8868644    1082.837468
139.3107051    -233.2755958    1049.530203
143.4511386    -297.3409244    1067.375027
43.46601157    -425.8353706    793.3997444
75.27798362    -479.9978059    720.5617453
109.4522217    -401.16074    866.8308819
117.2974448    -416.1984881    805.6570544
210.4336763    100.5427034    446.8028356
150.5485558    145.3429363    458.5780144
154.7296271    27.01370723    448.7710374
82.33684094    173.5420131    19.86225156
180.6925681    124.563602    69.037682
154.7004009    61.35799888    79.06261775
153.1197549    62.95945005    60.57158126
37.41620716    90.61327562    23.06025703
220.2956375    83.23803675    30.46162465
-5.720469378    151.556495    10.80634824
210.4336763    -100.5427034    446.8028356
150.5485558    -145.3429363    458.5780144
154.7296271    -27.00189387    448.7710374
82.33684094    -173.5420131    19.86225156
180.6925681    -124.563602    69.037682
154.7004009    -61.35799888    79.06261775
153.1197549    -62.95945005    60.57158126
37.41620716    -90.61327562    23.06025703
220.2956375    -83.23803675    30.46162465
-5.720469378    -151.556495    10.80634824
116.423811    0    748.5567037];

M( end + 1, : ) = ( M( 13, : ) + V( 608 + 1, : ) ) * 0.5;  % 94
M( end + 1, : ) = ( M( 14, : ) + V( 2546 + 1, : ) ) * 0.5; % 95

x=M(:,1);
y=M(:,2);
z=M(:,3);

sk1 = [1 2 3 4 5 17 19 82];
sk2 = [4 6 18 20 92];
sk3 = [15 13 11 2 12 14 16];

% skel_pinnochio = [2 3 4 1 6 18 20 92 5 17 19 82 12 14 16 11 13 15];
skel_pinnochio = [2 3 4 1 6 18 20 92 5 17 19 82 12 95 16 11 94 15];

skel_prev = [-1 0 1 0 2 4 5 6 2 8 9 10 0 12 13 0 15 16];
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
plot3(x(sk1),y(sk1),z(sk1),'k',x(sk2),y(sk2),z(sk2),'k',x(sk3),y(sk3),z(sk3),'k')
hold off
text(x,y,z,num2str((0:npt-1)'),'Interpreter', 'none')
lighting gouraud
% text(x,y,z,N,'Interpreter', 'none')
axis equal
axis off


ptlist = [1172, 1251, 1186, 3104, 122, 912, 325, 2267, 803, 808, 795, 805,...
    669, 666, 659, 662, 63, 30, 1973, 3, 574, 582, 605, 577, 162, 368, 380, 150,...
    1665, 1663, 1674, 1671, 1771, 1776, 1773, 1785, 13522, 13906, 14405, 14423,...
     14090, 14133, 13994, 13364, 608, 2546] + 1;
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
plot3(x(sk1),y(sk1),z(sk1),'k',x(sk2),y(sk2),z(sk2),'k:',x(sk3),y(sk3),z(sk3),'k--')
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
plot3(x(sk1),y(sk1),z(sk1),'b',x(sk2),y(sk2),z(sk2),'b',x(sk3),y(sk3),z(sk3),'b', 'LineWidth', 2.5)
plot3(x(sk1),y(sk1),z(sk1),'y.',x(sk2),y(sk2),z(sk2),'y.',x(sk3),y(sk3),z(sk3),'y.', 'MarkerSize', 25)

hold off
lighting gouraud
axis equal
axis off




figure(4)
hold on
trisurf(F,V(:,1),V(:,2),V(:,3),'FaceColor',0.7*[1 1 1],'Facealpha',0.7); %,'EdgeColor','none')
plot3(x(sk1),y(sk1),z(sk1),'y.',x(sk2),y(sk2),z(sk2),'y.',x(sk3),y(sk3),z(sk3),'y.', 'MarkerSize', 25)
plot3(V(ptlist,1),V(ptlist,2),V(ptlist,3),'b.','MarkerSize',40);

plot3([x(13) x(94) V(608+1,1) ], [y(13) y(94) V(608+1,2) ], [z(13) z(94) V(608+1,3) ],'k.-', 'MarkerSize', 30)
plot3([x(14) x(95) V(2546+1,1) ], [y(14) y(95) V(2546+1,2) ], [z(13) z(95) V(2546+1,3) ],'k.-', 'MarkerSize', 30)


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
plot3(x(sk1),y(sk1),z(sk1),'k',x(sk2),y(sk2),z(sk2),'k',x(sk3),y(sk3),z(sk3),'k')
hold on
for i=1:length(skel_pinnochio)
    text(x(skel_pinnochio(i)),y(skel_pinnochio(i)),z(skel_pinnochio(i)),num2str(i-1));
end
hold off

axis equal
axis off
