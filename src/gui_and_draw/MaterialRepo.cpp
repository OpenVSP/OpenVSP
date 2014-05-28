#include "MaterialRepo.h"

#include <assert.h>

MaterialRepo::MaterialRepo()
{
    MaterialPref mat;

    mat.name = "Emerald";
    mat.ambi[0] = 0.0215; mat.ambi[1] = 0.1745; mat.ambi[2] = 0.0215, mat.ambi[3] = 1.0;
    mat.diff[0] = 0.07568; mat.diff[1] = 0.61424; mat.diff[2] = 0.07568; mat.diff[3] = 1.0;
    mat.spec[0] = 0.633; mat.spec[1] = 0.727811; mat.spec[2] = 0.633; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 76.0;
    m_Materials.push_back(mat);

    mat.name = "Jade";
    mat.ambi[0] = 0.135; mat.ambi[1] = 0.2225; mat.ambi[2] = 0.1575; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.54; mat.diff[1] = 0.89; mat.diff[2] = 0.63; mat.diff[3] = 1.0;
    mat.spec[0] = 0.316; mat.spec[1] = 0.316; mat.spec[2] = 0.316; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 12.0;
    m_Materials.push_back(mat);

    mat.name = "Obsidian";
    mat.ambi[0] = 0.054; mat.ambi[1] = 0.05; mat.ambi[2] = 0.066; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.183; mat.diff[1] = 0.17; mat.diff[2] = 0.225; mat.diff[3] = 1.0;
    mat.spec[0] = 0.333; mat.spec[1] = 0.329; mat.spec[2] = 0.346; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 38.0;
    m_Materials.push_back(mat);

    mat.name = "Pearl";
    mat.ambi[0] = 0.250; mat.ambi[1] = 0.207; mat.ambi[2] = 0.207; mat.ambi[3] = 1.0;
    mat.diff[0] = 1.000; mat.diff[1] = 0.829; mat.diff[2] = 0.829; mat.diff[3] = 1.0;
    mat.spec[0] = 0.297; mat.spec[1] = 0.297; mat.spec[2] = 0.297; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 11.0;
    m_Materials.push_back(mat);

    mat.name = "Ruby";
    mat.ambi[0] = 0.175; mat.ambi[1] = 0.01175; mat.ambi[2] = 0.01175; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.61424; mat.diff[1] = 0.04136; mat.diff[2] = 0.04136; mat.diff[3] = 1.0;
    mat.spec[0] = 0.727811; mat.spec[1] = 0.626959; mat.spec[2] = 0.626959; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 76.0;
    m_Materials.push_back(mat);

    mat.name = "Turquoise";
    mat.ambi[0] = 0.1; mat.ambi[1] = 0.18725; mat.ambi[2] = 0.1745; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.396; mat.diff[1] = 0.74151; mat.diff[2] = 0.69102; mat.diff[3] = 1.0;
    mat.spec[0] = 0.297254; mat.spec[1] = 0.30829; mat.spec[2] = 0.306678; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 12.0;
    m_Materials.push_back(mat);

    mat.name = "Brass";
    mat.ambi[0] = 0.330; mat.ambi[1] = 0.223; mat.ambi[2] = 0.0275; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.78; mat.diff[1] = 0.57; mat.diff[2] = 0.11; mat.diff[3] = 1.0;
    mat.spec[0] = 0.99; mat.spec[1] = 0.94; mat.spec[2] = 0.81; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 27.0;
    m_Materials.push_back(mat);

    mat.name = "Bronze";
    mat.ambi[0] = 0.2125; mat.ambi[1] = 0.1275; mat.ambi[2] = 0.054; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.714; mat.diff[1] = 0.4284; mat.diff[2] = 0.18144; mat.diff[3] = 1.0;
    mat.spec[0] = 0.393548; mat.spec[1] = 0.271906; mat.spec[2] = 0.166721; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 27.0;
    m_Materials.push_back(mat);

    mat.name = "Chrome";
    mat.ambi[0] = 0.25; mat.ambi[1] = 0.25; mat.ambi[2] = 0.25; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.4; mat.diff[1] = 0.4; mat.diff[2] = 0.4; mat.diff[3] = 1.0;
    mat.spec[0] = 0.77; mat.spec[1] = 0.77; mat.spec[2] = 0.77; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 77.0;
    m_Materials.push_back(mat);

    mat.name = "Copper";
    mat.ambi[0] = 0.19125; mat.ambi[1] = 0.0735; mat.ambi[2] = 0.0225; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.7038; mat.diff[1] = 0.27048; mat.diff[2] = 0.0828; mat.diff[3] = 1.0;
    mat.spec[0] = 0.256777; mat.spec[1] = 0.137622; mat.spec[2] = 0.086014; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 13.0;
    m_Materials.push_back(mat);

    mat.name = "Gold";
    mat.ambi[0] = 0.25; mat.ambi[1] = 0.20; mat.ambi[2] = 0.07; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.75; mat.diff[1] = 0.61; mat.diff[2] = 0.22; mat.diff[3] = 1.0;
    mat.spec[0] = 0.62; mat.spec[1] = 0.55; mat.spec[2] = 0.37; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 51.0;
    m_Materials.push_back(mat);

    mat.name = "Silver";
    mat.ambi[0] = 0.20; mat.ambi[1] = 0.20; mat.ambi[2] = 0.20; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.50; mat.diff[1] = 0.50; mat.diff[2] = 0.50; mat.diff[3] = 1.0;
    mat.spec[0] = 0.51; mat.spec[1] = 0.51; mat.spec[2] = 0.51; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 51.0;
    m_Materials.push_back(mat);

    mat.name = "Aluminum";
    mat.ambi[0] = 0.25, mat.ambi[1] = 0.25, mat.ambi[2] = 0.25, mat.ambi[3] = 1.0;
    mat.diff[0] = 0.5, mat.diff[1] = 0.5, mat.diff[2] = 0.5, mat.diff[3] = 1.0;
    mat.spec[0] = 0.6, mat.spec[1] = 0.6, mat.spec[2] = 0.6, mat.spec[3] = 1.0;
    mat.emis[0] = 0.0, mat.emis[1] = 0.0, mat.emis[2] = 0.0, mat.emis[3] = 1.0;
    mat.shininess = 5.0;
    m_Materials.push_back(mat);

    mat.name = "Shiny Gold";
    mat.ambi[0] = 0.4; mat.ambi[1] = 0.2; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.9; mat.diff[1] = 0.5; mat.diff[2] = 0.0; mat.diff[3] = 1.0;
    mat.spec[0] = 0.9; mat.spec[1] = 0.9; mat.spec[2] = 0.0; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 2.0;
    m_Materials.push_back(mat);

    mat.name = "Black Plastic";
    mat.ambi[0] = 0.0; mat.ambi[1] = 0.0; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.01; mat.diff[1] = 0.01; mat.diff[2] = 0.01; mat.diff[3] = 1.0;
    mat.spec[0] = 0.50; mat.spec[1] = 0.50; mat.spec[2] = 0.50; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 32.0;
    m_Materials.push_back(mat);

    mat.name = "Cyan Plastic";
    mat.ambi[0] = 0.0; mat.ambi[1] = 0.1; mat.ambi[2] = 0.06; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.0; mat.diff[1] = 0.51; mat.diff[2] = 0.51; mat.diff[3] = 1.0;
    mat.spec[0] = 0.50; mat.spec[1] = 0.50; mat.spec[2] = 0.50; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 32.0;
    m_Materials.push_back(mat);

    mat.name =  "Green Plastic";
    mat.ambi[0] = 0.0; mat.ambi[1] = 0.0; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.1; mat.diff[1] = 0.35; mat.diff[2] = 0.1; mat.diff[3] = 1.0;
    mat.spec[0] = 0.45; mat.spec[1] = 0.55; mat.spec[2] = 0.45; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 32.0;
    m_Materials.push_back(mat);

    mat.name = "Red Plastic";
    mat.ambi[0] = 0.0, mat.ambi[1] = 0.0, mat.ambi[2] = 0.0, mat.ambi[3] = 1.0;
    mat.diff[0] = 0.5, mat.diff[1] = 0.0, mat.diff[2] = 0.0, mat.diff[3] = 1.0;
    mat.spec[0] = 0.7, mat.spec[1] = 0.6, mat.spec[2] = 0.6, mat.spec[3] = 1.0;
    mat.emis[0] = 0.0, mat.emis[1] = 0.0, mat.emis[2] = 0.0, mat.emis[3] = 1.0;
    mat.shininess = 32.0;
    m_Materials.push_back(mat);

    mat.name = "Blue Plastic";
    mat.ambi[0] = 0.5; mat.ambi[1] = 0.5; mat.ambi[2] = 0.87; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.4; mat.diff[1] = 0.4; mat.diff[2] = 0.7; mat.diff[3] = 1.0;
    mat.spec[0] = 1.0; mat.spec[1] = 1.0; mat.spec[2] = 1.0; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 35.0;
    m_Materials.push_back(mat);

    mat.name = "Yellow Plastic";
    mat.ambi[0] = 0.3; mat.ambi[1] = 0.2; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.6; mat.diff[1] = 0.4; mat.diff[2] = 0.1; mat.diff[3] = 1.0;
    mat.spec[0] = 0.55; mat.spec[1] = 0.55; mat.spec[2] = 0.35; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 30.0;
    m_Materials.push_back(mat);

// Materials from reference, but they don't look good.
//    mat.name = "White Plastic";
//    mat.ambi[0] = 0.0; mat.ambi[1] = 0.0; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
//    mat.diff[0] = 0.55; mat.diff[1] = 0.55; mat.diff[2] = 0.55; mat.diff[3] = 1.0;
//    mat.spec[0] = 0.7; mat.spec[1] = 0.7; mat.spec[2] = 0.7; mat.spec[3] = 1.0;
//    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
//    mat.shininess = 32.0;
//    m_Materials.push_back(mat);
//
//    mat.name = "Yellow Plastic 2";
//    mat.ambi[0] = 0.0; mat.ambi[1] = 0.0; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
//    mat.diff[0] = 0.5; mat.diff[1] = 0.5; mat.diff[2] = 0.0; mat.diff[3] = 1.0;
//    mat.spec[0] = 0.6; mat.spec[1] = 0.6; mat.spec[2] = 0.5; mat.spec[3] = 1.0;
//    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
//    mat.shininess = 32.0;
//    m_Materials.push_back(mat);

    mat.name = "White";
    mat.ambi[0] = 0.9; mat.ambi[1] = 0.9; mat.ambi[2] = 0.9; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.8; mat.diff[1] = 0.8; mat.diff[2] = 0.8; mat.diff[3] = 1.0;
    mat.spec[0] = 0.8; mat.spec[1] = 0.8; mat.spec[2] = 0.8; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 40.0;
    m_Materials.push_back(mat);

    mat.name = "Black Rubber";
    mat.ambi[0] = 0.02; mat.ambi[1] = 0.02; mat.ambi[2] = 0.02; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.01; mat.diff[1] = 0.01; mat.diff[2] = 0.01; mat.diff[3] = 1.0;
    mat.spec[0] = 0.4; mat.spec[1] = 0.4; mat.spec[2] = 0.4; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 10.0;
    m_Materials.push_back(mat);

    mat.name = "Cyan Rubber";
    mat.ambi[0] = 0.0; mat.ambi[1] = 0.05; mat.ambi[2] = 0.05; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.4; mat.diff[1] = 0.5; mat.diff[2] = 0.5; mat.diff[3] = 1.0;
    mat.spec[0] = 0.04; mat.spec[1] = 0.7; mat.spec[2] = 0.7; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 10.0;
    m_Materials.push_back(mat);

    mat.name = "Green Rubber";
    mat.ambi[0] = 0.0; mat.ambi[1] = 0.05; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.4; mat.diff[1] = 0.5; mat.diff[2] = 0.4; mat.diff[3] = 1.0;
    mat.spec[0] = 0.04; mat.spec[1] = 0.7; mat.spec[2] = 0.04; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 10.0;
    m_Materials.push_back(mat);

    mat.name = "Red Rubber";
    mat.ambi[0] = 0.05; mat.ambi[1] = 0.0; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.5; mat.diff[1] = 0.4; mat.diff[2] = 0.4; mat.diff[3] = 1.0;
    mat.spec[0] = 0.7; mat.spec[1] = 0.04; mat.spec[2] = 0.04; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 10.0;
    m_Materials.push_back(mat);

    mat.name = "Blue Rubber";
    mat.ambi[0] = 0.0; mat.ambi[1] = 0.0; mat.ambi[2] = 0.05; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.4; mat.diff[1] = 0.4; mat.diff[2] = 0.5; mat.diff[3] = 1.0;
    mat.spec[0] = 0.04; mat.spec[1] = 0.04; mat.spec[2] = 0.7; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 10.0;
    m_Materials.push_back(mat);

    mat.name = "White Rubber";
    mat.ambi[0] = 0.05; mat.ambi[1] = 0.05; mat.ambi[2] = 0.05; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.5; mat.diff[1] = 0.5; mat.diff[2] = 0.5; mat.diff[3] = 1.0;
    mat.spec[0] = 0.7; mat.spec[1] = 0.7; mat.spec[2] = 0.7; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 10.0;
    m_Materials.push_back(mat);

    mat.name = "Yellow Rubber";
    mat.ambi[0] = 0.05; mat.ambi[1] = 0.05; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.5; mat.diff[1] = 0.5; mat.diff[2] = 0.4; mat.diff[3] = 1.0;
    mat.spec[0] = 0.7; mat.spec[1] = 0.7; mat.spec[2] = 0.04; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 10.0;
    m_Materials.push_back(mat);

    mat.name = "Glass Light";
    mat.ambi[0] = 0.2; mat.ambi[1] = 0.2; mat.ambi[2] = 0.2; mat.ambi[3] = 0.2;
    mat.diff[0] = 0.1; mat.diff[1] = 0.1; mat.diff[2] = 0.1; mat.diff[3] = 0.33;
    mat.spec[0] = 0.7; mat.spec[1] = 0.7; mat.spec[2] = 0.7; mat.spec[3] = 0.7;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 5.0;
    m_Materials.push_back(mat);

    mat.name = "Glass Med";
    mat.ambi[0] = 0.2; mat.ambi[1] = 0.2; mat.ambi[2] = 0.2; mat.ambi[3] = 0.2;
    mat.diff[0] = 0.1; mat.diff[1] = 0.1; mat.diff[2] = 0.1; mat.diff[3] = 0.5;
    mat.spec[0] = 0.7; mat.spec[1] = 0.7; mat.spec[2] = 0.7; mat.spec[3] = 0.7;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 5.0;
    m_Materials.push_back(mat);

    mat.name = "Glass Dark";
    mat.ambi[0] = 0.0; mat.ambi[1] = 0.0; mat.ambi[2] = 0.0; mat.ambi[3] = 0.0;
    mat.diff[0] = 0.0; mat.diff[1] = 0.0; mat.diff[2] = 0.0; mat.diff[3] = 0.67;
    mat.spec[0] = 0.7; mat.spec[1] = 0.7; mat.spec[2] = 0.7; mat.spec[3] = 0.7;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 5.0;
    m_Materials.push_back(mat);

    mat.name = "Glass Golden";
    mat.ambi[0] = 0.27; mat.ambi[1] = 0.28; mat.ambi[2] = 0.23; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.61; mat.diff[1] = 0.41; mat.diff[2] = 0.27; mat.diff[3] = 0.5;
    mat.spec[0] = 0.97; mat.spec[1] = 0.64; mat.spec[2] = 0.28; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 10.0;
    m_Materials.push_back(mat);

    mat.name = "Blank";
    mat.ambi[0] = 0.0; mat.ambi[1] = 0.0; mat.ambi[2] = 0.0; mat.ambi[3] = 1.0;
    mat.diff[0] = 0.0; mat.diff[1] = 0.0; mat.diff[2] = 0.0; mat.diff[3] = 1.0;
    mat.spec[0] = 0.0; mat.spec[1] = 0.0; mat.spec[2] = 0.0; mat.spec[3] = 1.0;
    mat.emis[0] = 0.0; mat.emis[1] = 0.0; mat.emis[2] = 0.0; mat.emis[3] = 1.0;
    mat.shininess = 0.0;
    m_Materials.push_back(mat);
}

MaterialRepo::~MaterialRepo()
{
}

bool MaterialRepo::FindMaterial( std::string name, MaterialRepo::MaterialPref& mat_out )
{
    for( int i = 0; i < (int)m_Materials.size(); i++ )
    {
        if( name == m_Materials[i].name )
        {
            mat_out = m_Materials[i];
            return true;
        }
    }
    return false;
}

bool MaterialRepo::FindMaterial( int index, MaterialRepo::MaterialPref& mat_out )
{
    if( index >= 0 && index < m_Materials.size() )
    {
        mat_out = m_Materials[index];
        return true;
    }
    return false;
}

std::vector<std::string> MaterialRepo::GetNames()
{
    std::vector<std::string> names;

    for( int i = 0; i < (int)m_Materials.size(); i++ )
    {
        names.push_back(m_Materials[i].name);
    }

    return names;
}
