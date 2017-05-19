//******************************************************************************
//
//   Unit Conversion Function Group
//
//   Bryan Schmidt
//   Date - 3/27/17
//
//******************************************************************************

#include "APIDefines.h"
#include "UnitConversion.h"


//==================================//
//=========== Velocity =============//
//==================================//
double ConvertVelocityToM_S(double velocity, int cur_unit)
{
    switch ( cur_unit )
    {
    case vsp::V_UNIT_FT_S :
        velocity *= 381.0/1250.0;
        break;

    case vsp::V_UNIT_M_S :
        break;

    case vsp::V_UNIT_MPH :
        velocity *= 1397.0/3125.0;
        break;

    case vsp::V_UNIT_KM_HR :
        velocity *= 25.0/90.0;
        break;

    case vsp::V_UNIT_KEAS :
        velocity *= 463.0/900.0;
        break;

    case vsp::V_UNIT_KTAS :
        velocity *= 463.0/900.0;
        break;
    }
    return velocity;
}

double ConvertVelocityFromM_S(double velocity, int new_unit)
{
    switch ( new_unit )
    {
    case vsp::V_UNIT_FT_S :
        velocity *= 1250.0/381.0;
        break;

    case vsp::V_UNIT_M_S :
        break;

    case vsp::V_UNIT_MPH :
        velocity *= 3125.0/1397.0;
        break;

    case vsp::V_UNIT_KM_HR :
        velocity *= 90.0/25.0;
        break;

    case vsp::V_UNIT_KEAS : // KEAS Conversion must be handled elsewhere
        velocity *= 900.0/463.0;
        break;

    case vsp::V_UNIT_KTAS :
        velocity *= 900.0/463.0;
        break;
    }
    return velocity;
}

double ConvertVelocity(double velocity, int cur_unit, int new_unit)
{
    double vms = ConvertVelocityToM_S( velocity, cur_unit );
    return ConvertVelocityFromM_S( vms, new_unit );
}

//==================================//
//=========== Length ===============//
//==================================//
double ConvertLengthToM(double length, int cur_unit)
{
    switch ( cur_unit )
    {
    case vsp::LEN_MM:
        length *= 1000; 
        break;

    case vsp::LEN_CM:
        length *= 100;
        break;

    case vsp::LEN_M:
        break;

    case vsp::LEN_IN:
        length *= 0.0254;
        break;

    case vsp::LEN_FT:
        length *= 0.3048;
        break;

    case vsp::LEN_YD:
        length *= 0.9144;
        break;

    case vsp::LEN_UNITLESS:
        break;
    }
    return length;
}

double ConvertLengthFromM(double length, int new_unit)
{
    switch ( new_unit )
    {
    case vsp::LEN_MM:
        length /= 1000; 
        break;

    case vsp::LEN_CM:
        length /= 100;
        break;

    case vsp::LEN_M:
        break;

    case vsp::LEN_IN:
        length /= 0.0254;
        break;

    case vsp::LEN_FT:
        length /= 0.3048;
        break;

    case vsp::LEN_YD:
        length /= 0.9144;
        break;

    case vsp::LEN_UNITLESS:
        break;
    }
    return length;
}

double ConvertLength(double length, int cur_unit, int new_unit)
{
    double len_m = ConvertLengthToM( length, cur_unit );
    return ConvertLengthFromM( len_m, new_unit );
}

//==================================//
//========== Temperature ===========//
//==================================//
double ConvertTemperatureToK(double temp, int cur_unit)
{
    switch ( cur_unit )
    {
    case vsp::TEMP_UNIT_K :
        break;

    case vsp::TEMP_UNIT_C :
        temp += 273.15;
        break;

    case vsp::TEMP_UNIT_F :
        temp = (temp + 459.67) * (5.0/9.0);
        break;

    case vsp::TEMP_UNIT_R :
        temp *= (5.0/9.0);
        break;
    }
    return temp;
}

double ConvertTemperatureFromK(double temp, int new_unit)
{
    switch ( new_unit )
    {
    case vsp::TEMP_UNIT_K :
        break;

    case vsp::TEMP_UNIT_C :
        temp -= 273.15;
        break;

    case vsp::TEMP_UNIT_F :
        temp = ( temp * (9.0/5.0) ) - 459.67;
        break;

    case vsp::TEMP_UNIT_R :
        temp *= (9.0/5.0);
        break;
    }
    return temp;
}

double ConvertTemperature(double temp, int cur_unit, int new_unit)
{
    double temp_k = ConvertTemperatureToK( temp, cur_unit );
    return ConvertTemperatureFromK( temp_k, new_unit );
}

//==================================//
//=========== Pressure =============//
//==================================//
double ConvertPressureToPSF(double pres, int cur_unit)
{
    switch ( cur_unit ) 
    {
    case vsp::PRES_UNIT_PSF:
        break;

    case vsp::PRES_UNIT_PSI:
        pres *= 144;
        break;

    case vsp::PRES_UNIT_PA:
        pres *= 0.02088543;
        break;

    case vsp::PRES_UNIT_KPA:
        pres *= 20.88543;
        break;

    case vsp::PRES_UNIT_INCHHG:
        pres *= 70.72619;
        break;

    case vsp::PRES_UNIT_MMHG:
        pres *= 2.784496;
        break;

    case vsp::PRES_UNIT_MMH20:
        pres *= 0.204816;
        break;

    case vsp::PRES_UNIT_MB:
        pres *= 2.088543;
        break;

    case vsp::PRES_UNIT_ATM:
        pres *= 2116.217;
        break;
    }
    return pres;
}

double ConvertPressureFromPSF(double pres, int new_unit)
{
    switch ( new_unit )
    {
    case vsp::PRES_UNIT_PSF:
        break;

    case vsp::PRES_UNIT_PSI:
        pres /= 144;
        break;

    case vsp::PRES_UNIT_PA:
        pres /= 0.02088543;
        break;

    case vsp::PRES_UNIT_KPA:
        pres /= 20.88543;
        break;

    case vsp::PRES_UNIT_INCHHG:
        pres /= 70.72619;
        break;

    case vsp::PRES_UNIT_MMHG:
        pres /= 2.784496;
        break;

    case vsp::PRES_UNIT_MMH20:
        pres /= 0.204816;
        break;

    case vsp::PRES_UNIT_MB:
        pres /= 2.088543;
        break;

    case vsp::PRES_UNIT_ATM:
        pres /= 2116.217;
        break;
    }
    return pres;
}

double ConvertPressure(double pres, int cur_unit, int new_unit)
{
    double pres_psf = ConvertPressureToPSF( pres, cur_unit );
    return ConvertPressureFromPSF( pres_psf, new_unit );
}

//==================================//
//============ Density =============//
//==================================//
double ConvertDensityToSLUG_FT3(double density, int cur_unit)
{
    switch ( cur_unit )
    {
    case vsp::RHO_UNIT_SLUG_FT3:
        break;

    case vsp::RHO_UNIT_LBF_FT3:
        density /= 32.174;
        break;

    case vsp::RHO_UNIT_KG_M3:
        density /= 515.379;
        break;
    }
    return density;
}

double ConvertDensityFromSLUG_FT3(double density, int new_unit)
{
    switch ( new_unit )
    {
    case vsp::RHO_UNIT_SLUG_FT3:
        break;

    case vsp::RHO_UNIT_LBF_FT3:
        density *= 32.174;
        break;

    case vsp::RHO_UNIT_KG_M3:
        density *= 515.379;
    }
    return density;
}

double ConvertDensity(double density, int cur_unit, int new_unit)
{
    double density_slug_ft3 = ConvertDensityToSLUG_FT3( density, cur_unit );
    return ConvertDensityFromSLUG_FT3( density_slug_ft3, new_unit );
}

//==================================//
//====== Kinematic Viscosity =======//
//==================================//
double ConvertKineVisToM2_S(double kinevis, int cur_unit)
{
    switch (cur_unit)
    {
    case vsp::PD_UNITS_IMPERIAL:
        break;

    case vsp::PD_UNITS_METRIC:
        kinevis /= 10.7639;
        break;

    }
    return kinevis;
}

double ConvertKineVisFromM2_S(double kinevis, int new_unit)
{
    switch (new_unit)
    {
    case vsp::PD_UNITS_IMPERIAL:
        kinevis *= 10.7639;
        break;

    case vsp::PD_UNITS_METRIC:
        break;

    }
    return kinevis;
}

// This method currently takes in FREESTREAM_PD_UNITS which define
// whether or not Kinematic Viscosity is in imperial or metric units.
double ConvertKineVis(double kinevis, int cur_unit, int new_unit)
{
    double kinevis_m2_s = ConvertKineVisToM2_S( kinevis, cur_unit );
    return ConvertKineVisFromM2_S( kinevis_m2_s, new_unit );
}

//==================================//
//======= Dynamic Viscosity ========//
//==================================//
double ConvertDynaVisToKG_M_S(double dynavis, int cur_unit)
{
    switch (cur_unit)
    {
    case vsp::PD_UNITS_IMPERIAL:
        break;

    case vsp::PD_UNITS_METRIC:
        dynavis *= 47.8803; // slug/ft-s --> kg/m-s
        break;
    }
    return dynavis;
}

double ConvertDynaVisFromKG_M_S(double dynavis, int new_unit)
{
    switch (new_unit)
    {
    case vsp::PD_UNITS_IMPERIAL:
        dynavis /= 47.8803; // kg/m-s --> slug/ft-s
        break;

    case vsp::PD_UNITS_METRIC:
        break;
    }
    return dynavis;
}

// This method currently takes in FREESTREAM_PD_UNITS which define
// whether or not Dynamic Viscosity is in imperial or metric units.
double ConvertDynaVis(double dynavis, int cur_unit, int new_unit)
{
    double dynavis_kg_m_s = ConvertDynaVisToKG_M_S( dynavis, cur_unit );
    return ConvertDynaVisFromKG_M_S( dynavis_kg_m_s, new_unit );
}
