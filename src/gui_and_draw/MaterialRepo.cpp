#include "MaterialRepo.h"

#include <assert.h>

MaterialRepo::MaterialRepo()
{
    MaterialPref emerld, jade, obsidian, brass, chrome, gold, silver, blackplastic, cyanplastic,
        greenplastic, redplastic, blueplastic, yellowplastic, white, aluminum, shinygold,
        glasslight, glassmed, glassdark, glassgolden, blank;

    emerld.name = "Emerld";
    emerld.ambi[0] = 0.0215; emerld.ambi[1] = 0.1745; emerld.ambi[2] = 0.0215, emerld.ambi[3] = 1.0;
    emerld.diff[0] = 0.07568; emerld.diff[1] = 0.61424; emerld.diff[2] = 0.07568; emerld.diff[3] = 1.0;
    emerld.spec[0] = 0.633; emerld.spec[1] = 0.727811; emerld.spec[2] = 0.633; emerld.spec[3] = 1.0;
    emerld.emis[0] = 0.0; emerld.emis[1] = 0.0; emerld.emis[2] = 0.0; emerld.emis[3] = 1.0;
    emerld.shininess = 76.0;
    m_Materials.push_back(emerld);

    jade.name = "Jade";
    jade.ambi[0] = 0.135; jade.ambi[1] = 0.2225; jade.ambi[2] = 0.1575; jade.ambi[3] = 1.0;
    jade.diff[0] = 0.54; jade.diff[1] = 0.89; jade.diff[2] = 0.63; jade.diff[3] = 1.0;
    jade.spec[0] = 0.31622; jade.spec[1] = 0.316; jade.spec[2] = 0.316; jade.spec[3] = 1.0;
    jade.emis[0] = 0.0; jade.emis[1] = 0.0; jade.emis[2] = 0.0; jade.emis[3] = 1.0;
    jade.shininess = 12.0;
    m_Materials.push_back(jade);

    obsidian.name = "Obsidian";
    obsidian.ambi[0] = 0.054; obsidian.ambi[1] = 0.05; obsidian.ambi[2] = 0.066; obsidian.ambi[3] = 1.0;
    obsidian.diff[0] = 0.183; obsidian.diff[1] = 0.17; obsidian.diff[2] = 0.225; obsidian.diff[3] = 1.0;
    obsidian.spec[0] = 0.333; obsidian.spec[1] = 0.329; obsidian.spec[2] = 0.346; obsidian.spec[3] = 1.0;
    obsidian.emis[0] = 0.0; obsidian.emis[1] = 0.0; obsidian.emis[2] = 0.0; obsidian.emis[3] = 1.0;
    obsidian.shininess = 38.0;
    m_Materials.push_back(obsidian);

    brass.name = "Brass";
    brass.ambi[0] = 0.330; brass.ambi[1] = 0.223; brass.ambi[2] = 0.0275; brass.ambi[3] = 1.0;
    brass.diff[0] = 0.78; brass.diff[1] = 0.57; brass.diff[2] = 0.11; brass.diff[3] = 1.0;
    brass.spec[0] = 0.99; brass.spec[1] = 0.94; brass.spec[2] = 0.81; brass.spec[3] = 1.0;
    brass.emis[0] = 0.0; brass.emis[1] = 0.0; brass.emis[2] = 0.0; brass.emis[3] = 1.0;
    brass.shininess = 27.0;
    m_Materials.push_back(brass);

    chrome.name = "Chrome";
    chrome.ambi[0] = 0.25; chrome.ambi[1] = 0.25; chrome.ambi[2] = 0.25; chrome.ambi[3] = 1.0;
    chrome.diff[0] = 0.4; chrome.diff[1] = 0.4; chrome.diff[2] = 0.4; chrome.diff[3] = 1.0;
    chrome.spec[0] = 0.77; chrome.spec[1] = 0.77; chrome.spec[2] = 0.77; chrome.spec[3] = 1.0;
    chrome.emis[0] = 0.0; chrome.emis[1] = 0.0; chrome.emis[2] = 0.0; chrome.emis[3] = 1.0;
    chrome.shininess = 77.0;
    m_Materials.push_back(chrome);

    gold.name = "Gold";
    gold.ambi[0] = 0.25; gold.ambi[1] = 0.20; gold.ambi[2] = 0.07; gold.ambi[3] = 1.0;
    gold.diff[0] = 0.75; gold.diff[1] = 0.61; gold.diff[2] = 0.22; gold.diff[3] = 1.0;
    gold.spec[0] = 0.62; gold.spec[1] = 0.55; gold.spec[2] = 0.37; gold.spec[3] = 1.0;
    gold.emis[0] = 0.0; gold.emis[1] = 0.0; gold.emis[2] = 0.0; gold.emis[3] = 1.0;
    gold.shininess = 51.0;
    m_Materials.push_back(gold);

    silver.name = "Silver";
    silver.ambi[0] = 0.20; silver.ambi[1] = 0.20; silver.ambi[2] = 0.20; silver.ambi[3] = 1.0;
    silver.diff[0] = 0.50; silver.diff[1] = 0.50; silver.diff[2] = 0.50; silver.diff[3] = 1.0;
    silver.spec[0] = 0.51; silver.spec[1] = 0.51; silver.spec[2] = 0.51; silver.spec[3] = 1.0;
    silver.emis[0] = 0.0; silver.emis[1] = 0.0; silver.emis[2] = 0.0; silver.emis[3] = 1.0;
    silver.shininess = 51.0;
    m_Materials.push_back(silver);

    blackplastic.name = "Black Plastic";
    blackplastic.ambi[0] = 0.0; blackplastic.ambi[1] = 0.0; blackplastic.ambi[2] = 0.0; blackplastic.ambi[3] = 1.0;
    blackplastic.diff[0] = 0.01; blackplastic.diff[1] = 0.01; blackplastic.diff[2] = 0.01; blackplastic.diff[3] = 1.0;
    blackplastic.spec[0] = 0.50; blackplastic.spec[1] = 0.50; blackplastic.spec[2] = 0.50; blackplastic.spec[3] = 1.0;
    blackplastic.emis[0] = 0.0; blackplastic.emis[1] = 0.0; blackplastic.emis[2] = 0.0; blackplastic.emis[3] = 1.0;
    blackplastic.shininess = 32.0;
    m_Materials.push_back(blackplastic);

    cyanplastic.name = "Cyan Plastic";
    cyanplastic.ambi[0] = 0.0; cyanplastic.ambi[1] = 0.1; cyanplastic.ambi[2] = 0.06; cyanplastic.ambi[3] = 1.0;
    cyanplastic.diff[0] = 0.0; cyanplastic.diff[1] = 0.51; cyanplastic.diff[2] = 0.51; cyanplastic.diff[3] = 1.0;
    cyanplastic.spec[0] = 0.51; cyanplastic.spec[1] = 0.51; cyanplastic.spec[2] = 0.51; cyanplastic.spec[3] = 1.0;
    cyanplastic.emis[0] = 0.0; cyanplastic.emis[1] = 0.0; cyanplastic.emis[2] = 0.0; cyanplastic.emis[3] = 1.0;
    cyanplastic.shininess = 32.0;
    m_Materials.push_back(cyanplastic);

    greenplastic.name =  "Green Plastic";
    greenplastic.ambi[0] = 0.0; greenplastic.ambi[1] = 0.0; greenplastic.ambi[2] = 0.0; greenplastic.ambi[3] = 1.0;
    greenplastic.diff[0] = 0.1; greenplastic.diff[1] = 0.35; greenplastic.diff[2] = 0.1; greenplastic.diff[3] = 1.0;
    greenplastic.spec[0] = 0.45; greenplastic.spec[1] = 0.55; greenplastic.spec[2] = 0.45; greenplastic.spec[3] = 1.0;
    greenplastic.emis[0] = 0.0; greenplastic.emis[1] = 0.0; greenplastic.emis[2] = 0.0; greenplastic.emis[3] = 1.0;
    greenplastic.shininess = 32.0;
    m_Materials.push_back(greenplastic);

    redplastic.name = "Red Plastic";
    redplastic.ambi[0] = 0.0, redplastic.ambi[1] = 0.0, redplastic.ambi[2] = 0.0, redplastic.ambi[3] = 1.0;
    redplastic.diff[0] = 0.5, redplastic.diff[1] = 0.0, redplastic.diff[2] = 0.0, redplastic.diff[3] = 1.0;
    redplastic.spec[0] = 0.7, redplastic.spec[1] = 0.6, redplastic.spec[2] = 0.6, redplastic.spec[3] = 1.0;
    redplastic.emis[0] = 0.0, redplastic.emis[1] = 0.0, redplastic.emis[2] = 0.0, redplastic.emis[3] = 1.0;
    redplastic.shininess = 32.0;
    m_Materials.push_back(redplastic);

    blueplastic.name = "Blue Plastic";
    blueplastic.ambi[0] = 0.5; blueplastic.ambi[1] = 0.5; blueplastic.ambi[2] = 0.87; blueplastic.ambi[3] = 1.0;
    blueplastic.diff[0] = 0.4; blueplastic.diff[1] = 0.4; blueplastic.diff[2] = 0.7; blueplastic.diff[3] = 1.0;
    blueplastic.spec[0] = 1.0; blueplastic.spec[1] = 1.0; blueplastic.spec[2] = 1.0; blueplastic.spec[3] = 1.0;
    blueplastic.emis[0] = 0.0; blueplastic.emis[1] = 0.0; blueplastic.emis[2] = 0.0; blueplastic.emis[3] = 1.0;
    blueplastic.shininess = 35.0;
    m_Materials.push_back(blueplastic);

    yellowplastic.name = "Yellow Plastic";
    yellowplastic.ambi[0] = 0.3; yellowplastic.ambi[1] = 0.2; yellowplastic.ambi[2] = 0.0; yellowplastic.ambi[3] = 1.0;
    yellowplastic.diff[0] = 0.6; yellowplastic.diff[1] = 0.4; yellowplastic.diff[2] = 0.1; yellowplastic.diff[3] = 1.0;
    yellowplastic.spec[0] = 0.55; yellowplastic.spec[1] = 0.55; yellowplastic.spec[2] = 0.35; yellowplastic.spec[3] = 1.0;
    yellowplastic.emis[0] = 0.0; yellowplastic.emis[1] = 0.0; yellowplastic.emis[2] = 0.0; yellowplastic.emis[3] = 1.0;
    yellowplastic.shininess = 30.0;
    m_Materials.push_back(yellowplastic);

    white.name = "White";
    white.ambi[0] = 0.9; white.ambi[1] = 0.9; white.ambi[2] = 0.9; white.ambi[3] = 1.0;
    white.diff[0] = 0.8; white.diff[1] = 0.8; white.diff[2] = 0.8; white.diff[3] = 1.0;
    white.spec[0] = 0.8; white.spec[1] = 0.8; white.spec[2] = 0.8; white.spec[3] = 1.0;
    white.emis[0] = 0.0; white.emis[1] = 0.0; white.emis[2] = 0.0; white.emis[3] = 1.0;
    white.shininess = 40.0;
    m_Materials.push_back(white);

    aluminum.name = "Aluminum";
    aluminum.ambi[0] = 0.25, aluminum.ambi[1] = 0.25, aluminum.ambi[2] = 0.25, aluminum.ambi[3] = 1.0;
    aluminum.diff[0] = 0.5, aluminum.diff[1] = 0.5, aluminum.diff[2] = 0.5, aluminum.diff[3] = 1.0;
    aluminum.spec[0] = 0.6, aluminum.spec[1] = 0.6, aluminum.spec[2] = 0.6, aluminum.spec[3] = 1.0;
    aluminum.emis[0] = 0.0, aluminum.emis[1] = 0.0, aluminum.emis[2] = 0.0, aluminum.emis[3] = 1.0;
    aluminum.shininess = 5.0;
    m_Materials.push_back(aluminum);

    shinygold.name = "Shiny Gold";
    shinygold.ambi[0] = 0.4; shinygold.ambi[1] = 0.2; shinygold.ambi[2] = 0.0; shinygold.ambi[3] = 1.0;
    shinygold.diff[0] = 0.9; shinygold.diff[1] = 0.5; shinygold.diff[2] = 0.0; shinygold.diff[3] = 1.0;
    shinygold.spec[0] = 0.9; shinygold.spec[1] = 0.9; shinygold.spec[2] = 0.0; shinygold.spec[3] = 1.0;
    shinygold.emis[0] = 0.0; shinygold.emis[1] = 0.0; shinygold.emis[2] = 0.0; shinygold.emis[3] = 1.0;
    shinygold.shininess = 2.0;
    m_Materials.push_back(shinygold);

    glasslight.name = "Glass_Light";
    glasslight.ambi[0] = 0.2; glasslight.ambi[1] = 0.2; glasslight.ambi[2] = 0.2; glasslight.ambi[3] = 0.2;
    glasslight.diff[0] = 0.1; glasslight.diff[1] = 0.1; glasslight.diff[2] = 0.1; glasslight.diff[3] = 0.33;
    glasslight.spec[0] = 0.7; glasslight.spec[1] = 0.7; glasslight.spec[2] = 0.7; glasslight.spec[3] = 0.7;
    glasslight.emis[0] = 0.0; glasslight.emis[1] = 0.0; glasslight.emis[2] = 0.0; glasslight.emis[3] = 1.0;
    glasslight.shininess = 5.0;
    m_Materials.push_back(glasslight);

    glassmed.name = "Glass_Med";
    glassmed.ambi[0] = 0.2; glassmed.ambi[1] = 0.2; glassmed.ambi[2] = 0.2; glassmed.ambi[3] = 0.2;
    glassmed.diff[0] = 0.1; glassmed.diff[1] = 0.1; glassmed.diff[2] = 0.1; glassmed.diff[3] = 0.5;
    glassmed.spec[0] = 0.7; glassmed.spec[1] = 0.7; glassmed.spec[2] = 0.7; glassmed.spec[3] = 0.7;
    glassmed.emis[0] = 0.0; glassmed.emis[1] = 0.0; glassmed.emis[2] = 0.0; glassmed.emis[3] = 1.0;
    glassmed.shininess = 5.0;
    m_Materials.push_back(glassmed);

    glassdark.name = "Glass_Dark";
    glassdark.ambi[0] = 0.0; glassdark.ambi[1] = 0.0; glassdark.ambi[2] = 0.0; glassdark.ambi[3] = 0.0;
    glassdark.diff[0] = 0.0; glassdark.diff[1] = 0.0; glassdark.diff[2] = 0.0; glassdark.diff[3] = 0.67;
    glassdark.spec[0] = 0.7; glassdark.spec[1] = 0.7; glassdark.spec[2] = 0.7; glassdark.spec[3] = 0.7;
    glassdark.emis[0] = 0.0; glassdark.emis[1] = 0.0; glassdark.emis[2] = 0.0; glassdark.emis[3] = 1.0;
    glassdark.shininess = 5.0;
    m_Materials.push_back(glassdark);

    glassgolden.name = "Glass_Golden";
    glassgolden.ambi[0] = 0.27; glassgolden.ambi[1] = 0.28; glassgolden.ambi[2] = 0.23; glassgolden.ambi[3] = 1.0;
    glassgolden.diff[0] = 0.61; glassgolden.diff[1] = 0.41; glassgolden.diff[2] = 0.27; glassgolden.diff[3] = 0.5;
    glassgolden.spec[0] = 0.97; glassgolden.spec[1] = 0.64; glassgolden.spec[2] = 0.28; glassgolden.spec[3] = 1.0;
    glassgolden.emis[0] = 0.0; glassgolden.emis[1] = 0.0; glassgolden.emis[2] = 0.0; glassgolden.emis[3] = 1.0;
    glassgolden.shininess = 10.0;
    m_Materials.push_back(glassgolden);

    blank.name = "Blank";
    blank.ambi[0] = 0.0; blank.ambi[1] = 0.0; blank.ambi[2] = 0.0; blank.ambi[3] = 1.0;
    blank.diff[0] = 0.0; blank.diff[1] = 0.0; blank.diff[2] = 0.0; blank.diff[3] = 1.0;
    blank.spec[0] = 0.0; blank.spec[1] = 0.0; blank.spec[2] = 0.0; blank.spec[3] = 1.0;
    blank.emis[0] = 0.0; blank.emis[1] = 0.0; blank.emis[2] = 0.0; blank.emis[3] = 1.0;
    blank.shininess = 0.0;
    m_Materials.push_back(blank);
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