%% Atmosphere Plot Script from OpenVSP APIParasiteDragTestSuite
%% Date: 3/28/2017
%% Author: Bryan Schmidt

clear all
close all
clc

height = 4;
width = 6;

legendfontsize = 9;
labelfontsize = 12;

%% Collect Data (US Standard Atmosphere 1976)
% Data Type
Data_Type = 'USStandardAtmosphere1976';

File_Name = sprintf('%sData.csv', Data_Type);

% Read in Altitude, Pressure, Temperature, and Pressure and Density Ratios
alt = csvread( File_Name, 4, 1, [4 1 4 31] );
pres = csvread( File_Name, 5, 1, [5 1 5 31] );
pres_ratio = csvread( File_Name, 6, 1, [6 1 6 31] );
rho_ratio = csvread( File_Name, 7, 1, [7 1 7 31] );
temp = csvread( File_Name, 8, 1, [8 1 8 31] );

% Plot Data
g = figure(1);
plot( temp, alt/1000, 'LineWidth', 2 );
hold on

%% Collect Data (USAF 1966)
% Data Type
Data_Type = 'USAFAtmosphere1966';

File_Name = sprintf('%sData.csv', Data_Type);

% Read in Altitude, Pressure, Temperature, and Pressure and Density Ratios
alt = csvread( File_Name, 4, 1, [4 1 4 31] );
pres = csvread( File_Name, 5, 1, [5 1 5 31] );
pres_ratio = csvread( File_Name, 6, 1, [6 1 6 31] );
rho_ratio = csvread( File_Name, 7, 1, [7 1 7 31] );
temp = csvread( File_Name, 8, 1, [8 1 8 31] );

plot( temp, alt/1000, 'LineWidth', 2 );

legenddata{1} = 'US Standard 1976';
legenddata{2} = 'USAF 1966';

legend( legenddata, 'Location', 'Best','FontSize', legendfontsize );
hold off

grid on
xlabel('Temperature (K)','FontSize', labelfontsize );
ylabel('Altitude (km)','FontSize', labelfontsize );

axis([180 300 0 90]);

box on;
grid on;
set(gca, 'gridlinestyle','--','LineWidth',1.0,'GridColor',[0.05 0.05 0.05],'GridAlpha',0.3);
set(g,'units','inches','position',[0 0 width height]);

% Print it to a PNG
g.PaperUnits = 'inches';
g.PaperPosition = [0 0 width height];
print('PowerTradeSpaceDownSelect','-dpng','-r300')
g.PaperSize = [width height];

print('-dpng','-r600','standard_atmosphere_api_output')

%% Collect Data (Turbulent Fricition Coefficient)
% Data Type
Data_Type = 'FrictionCoefficient';

File_Name = sprintf('%sData.csv', Data_Type);

fid = fopen(File_Name,'r');

for i = 1:4
    fgetl(fid);
end

structnum = 1;
tline = fgetl(fid);
while tline ~= -1
    data = textscan(tline, '%s','Delimiter',',');
    name = data{1,1}(1);
    numbers = zeros(1,length(data{1,1})-1);
    for j = 2:length(data{1,1})
        numbers(j-1) = str2double(cell2mat(data{1,1}(j)));
    end

    if strcmpi(name, 'ReyIn')
        ReyIn = numbers;
    elseif strcmpi(name, 'Ref_Leng')
        Ref_Leng = numbers;
    else
        FrictionCoefficient(structnum).Name = name;
        FrictionCoefficient(structnum).Data = numbers;
        structnum = structnum + 1;
    end
    tline = fgetl(fid);
end
fclose(fid);

% Plotting
g = figure(2);
for i = 1:length(FrictionCoefficient)
    plot( ReyIn, FrictionCoefficient(i).Data, 'LineWidth', 2 );
    hold on
    legenddata1(i) = FrictionCoefficient(i).Name;
end
legend( legenddata1, 'Location', 'EastOutside','FontSize', legendfontsize );
hold off

set(gca,'XScale','log')
set(gca,'YScale','log')

grid on
xlabel( 'Reynolds Number','FontSize', labelfontsize );
ylabel( 'C_f','FontSize', labelfontsize );

box on;
set(gca, 'gridlinestyle','--','LineWidth',1.0,'GridColor',[0.05 0.05 0.05],'GridAlpha',0.3);
set(g,'units','inches','position',[0 0 width+3 height]);

% Print it to a PNG
g.PaperUnits = 'inches';
g.PaperPosition = [0 0 width+3 height];
print('PowerTradeSpaceDownSelect','-dpng','-r300')
g.PaperSize = [width+3 height];

print('-dpng','-r600','friction_coefficient_api_output')


%% Collect Data (Body Form Factor)
% Data Type
Data_Type = 'BodyFormFactor';

File_Name = sprintf('%sData.csv', Data_Type);

fid = fopen(File_Name,'r');

for i = 1:4
    fgetl(fid);
end

structnum = 1;
tline = fgetl(fid);
while tline ~= -1
    data = textscan(tline, '%s','Delimiter',',');
    name = data{1,1}(1);
    numbers = zeros(1,length(data{1,1})-1);
    for j = 2:length(data{1,1})
        numbers(j-1) = str2double(cell2mat(data{1,1}(j)));
    end

    if strcmpi(name, 'D_L')
        D_L = numbers;
    elseif strcmpi(name, 'Ref_Leng')
        Ref_Leng = numbers;
    elseif strcmpi(name, 'Max_X_Area')
        Max_X_Area = numbers;
    elseif strcmpi(name, 'Manual')
        % Skip
    else
        BodyFormFactor(structnum).Name = name;
        BodyFormFactor(structnum).Data = numbers;
        structnum = structnum + 1;
    end
    tline = fgetl(fid);
end
fclose(fid);

% Plotting
g = figure(3);
for i = 1:length(BodyFormFactor)
    plot( D_L, BodyFormFactor(i).Data, 'LineWidth', 2 );
    hold on
    legenddata2(i) = BodyFormFactor(i).Name;
end
legend( legenddata2, 'Location', 'Best','FontSize', legendfontsize );
hold off

grid on
xlabel( 'Fineness D/L','FontSize', labelfontsize );
ylabel( 'Form Factor','FontSize', labelfontsize );

box on;
set(gca, 'gridlinestyle','--','LineWidth',1.0,'GridColor',[0.05 0.05 0.05],'GridAlpha',0.3);
set(g,'units','inches','position',[0 0 width height]);

% Print it to a PNG
g.PaperUnits = 'inches';
g.PaperPosition = [0 0 width height];
print('PowerTradeSpaceDownSelect','-dpng','-r300')
g.PaperSize = [width height];

print('-dpng','-r600','body_form_factor_api_output')

%% Collect Data (Wing Form Factor)
% Data Type
Data_Type = 'WingFormFactor';

File_Name = sprintf('%sData.csv', Data_Type);

fid = fopen(File_Name,'r');

for i = 1:4
    fgetl(fid);
end

structnum = 1;
tline = fgetl(fid);
while tline ~= -1
    data = textscan(tline, '%s','Delimiter',',');
    name = data{1,1}(1);
    numbers = zeros(1,length(data{1,1})-1);
    for j = 2:length(data{1,1})
        numbers(j-1) = str2double(cell2mat(data{1,1}(j)));
    end

    if strcmpi(name, 'T_C')
        D_L = numbers;
    elseif strcmpi(name, 'Manual')
        % Skip
    else
        WingFormFactor(structnum).Name = name;
        WingFormFactor(structnum).Data = numbers;
        structnum = structnum + 1;
    end
    tline = fgetl(fid);
end
fclose(fid);

% Plotting
g = figure(4);
for i = 1:length(WingFormFactor)
    plot( D_L, WingFormFactor(i).Data, 'LineWidth', 2 );
    hold on
    legenddata3(i) = WingFormFactor(i).Name;
end
legend( legenddata3, 'Location', 'Best','FontSize', legendfontsize );
hold off
grid on
xlabel( 'Thickness/Chord','FontSize', labelfontsize );
ylabel( 'Form Factor','FontSize', labelfontsize );

box on;
grid on;
set(gca, 'gridlinestyle','--','LineWidth',1.0,'GridColor',[0.05 0.05 0.05],'GridAlpha',0.3);
set(g,'units','inches','position',[0 0 width height]);

% Print it to a PNG
g.PaperUnits = 'inches';
g.PaperPosition = [0 0 width height];
print('PowerTradeSpaceDownSelect','-dpng','-r300')
g.PaperSize = [width height];

print('-dpng','-r600','wing_form_factor_api_output')

%% Collect Data (Partial Friction Coefficient)
% Data Type
Data_Type = 'PartialFrictionMethod';

File_Name = sprintf('%sData.csv', Data_Type);

fid = fopen(File_Name,'r');

for i = 1:4
    fgetl(fid);
end

structnum = 1;
tline = fgetl(fid);
while tline ~= -1
    data = textscan(tline, '%s','Delimiter',',');
    name = data{1,1}(1);
    numbers = zeros(1,length(data{1,1})-1);
    for j = 2:length(data{1,1})
        numbers(j-1) = str2double(cell2mat(data{1,1}(j)));
    end

    if strcmpi(name, 'Cf')
        cf = numbers;
    elseif strcmpi(name, 'Manual')
        % Skip
    elseif strcmpi(name, 'LamPerc')
        lamperc = numbers;
    end
    tline = fgetl(fid);
end
fclose(fid);

% Plotting
g = figure(5);
plot( lamperc, cf, 'LineWidth', 2 );

grid on
xlabel( 'Laminar Percent','FontSize', labelfontsize );
ylabel( 'Equivalent C_f','FontSize', labelfontsize );

box on;
grid on;
%set(gca, 'gridlinestyle','--','LineWidth',1.0,'GridColor',[0.05 0.05 0.05],'GridAlpha',0.3);
set(g,'units','inches','position',[0 0 width height]);

% Print it to a PNG
g.PaperUnits = 'inches';
g.PaperPosition = [0 0 width height];
print('PowerTradeSpaceDownSelect','-dpng','-r300')
g.PaperSize = [width height];

print('-dpng','-r600','partial_turbulence_api_output')
