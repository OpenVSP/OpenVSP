//******************************************************************************
//
//   Unit Conversion Function Group
//
//   Bryan Schmidt
//   Date - 3/27/17
//
//******************************************************************************

#if !defined(UNIT_CONVERSION_INCLUDED_)
#define UNIT_CONVERSION_INCLUDED_

#include <string>

// Velocity
double ConvertVelocityToM_S( double velocity, int cur_unit );
double ConvertVelocityFromM_S( double velocity, int new_unit );
double ConvertVelocity( double velocity, int cur_unit, int new_unit );

// Length
double ConvertLengthToM( double length, int cur_unit );
double ConvertLengthFromM( double length, int new_unit );
double ConvertLength( double length, int cur_unit, int new_unit );

// Temperature
double ConvertTemperatureToK( double temp, int cur_unit );
double ConvertTemperatureFromK( double temp, int new_unit );
double ConvertTemperature( double temp, int cur_unit, int new_unit );

// Pressure
double ConvertPressureToPSF( double pres, int cur_unit );
double ConvertPressureFromPSF( double pres, int new_unit );
double ConvertPressure( double pres, int cur_unit, int new_unit );

// Density
double ConvertDensityToSLUG_FT3( double density, int cur_unit );
double ConvertDensityFromSLUG_FT3( double density, int new_unit );
double ConvertDensity( double density, int cur_unit, int new_unit );

// Kinematic Viscosity
double ConvertKineVisToM2_S( double kinevis, int cur_unit );
double ConvertKineVisFromM2_S( double kinevis, int new_unit );
double ConvertKineVis( double kinevis, int cur_unit, int new_unit );

// Dynamic Viscosity
double ConvertDynaVisToKG_M_S( double dynavis, int cur_unit );
double ConvertDynaVisFromKG_M_S( double dynavis, int new_unit );
double ConvertDynaVis( double dynavis, int cur_unit, int new_unit );

std::string LenUnitName( int len_unit );

#endif
