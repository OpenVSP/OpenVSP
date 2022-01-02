#include "Atmosphere.h"

Atmosphere::Atmosphere()
{
    m_P0 = 0;
    m_RHO0 = 0;
    m_Hinf = 0;
    m_DeltaT = 0;
    m_Vinf = 0;
    m_Temp = 0;
    m_Pres = 0;
    m_DynamicVisc = 0;
    m_Density = 0;
    m_SoundSpeed = 0;
    m_KTAS = 0;
    m_KEAS = 0;
    m_Mach = 0;
    m_DensityRatio = 0;
    m_PressureRatio = 0;
    m_T0 = 288.15; // K
    m_Rspecific = 287.058; // m2/s2/k
    m_A0 = 661.48; // knots
}

Atmosphere::~Atmosphere()
{
}

void Atmosphere::USStandardAtmosphere1976( double alt, double delta_temp, int altunit, int tempunit, int presunit, double gamma )
{
    static const double arr1[] = { 0.0, 11.0, 20.0, 32.0, 47.0, 51.0, 71.0, 84.852 }; //kilometers
    vector < double > StdAtmosAltSteps ( arr1, arr1 + sizeof( arr1 ) / sizeof( arr1[0] ) );

    static const double arr2[] = { -6.5, 0.0, 1.0, 2.8, 0.0, -2.8, -2.0}; // Kelvin per kilometer
    vector < double > TempGradient ( arr2, arr2 + sizeof( arr2 ) / sizeof( arr2[0] ) );

    // Initial Alt 0 Quantities
    m_RHO0 = 1.22500; // kg/m3
    m_P0 = 1.013250e5; // N/m2 ( Pa )

    m_Hinf = alt;
    m_DeltaT = ConvertTemperature( delta_temp, tempunit, vsp::TEMP_UNIT_K ) - ConvertTemperature( 0.0, tempunit, vsp::TEMP_UNIT_K ); // Into K

    // Conversion if Necessary of Altitude into feet
    if ( altunit == vsp::PD_UNITS_IMPERIAL )
    {
        alt = ConvertLength( alt, vsp::LEN_FT, vsp::LEN_M );
    }
    alt /= 1000; // Into Kilometers

    double T = ( m_T0 + m_DeltaT );
    double P = m_P0;
    for ( size_t i = 0; i < StdAtmosAltSteps.size(); ++i )
    {
        if ( i != StdAtmosAltSteps.size() - 1 )
        {
            if ( alt >= StdAtmosAltSteps[i] )
            {
                if ( alt >= StdAtmosAltSteps[i + 1] )
                {
                    CalcLayerQuantitiesUS1976( T, P, StdAtmosAltSteps[i + 1], StdAtmosAltSteps[i], TempGradient[i] );
                }
                else
                {
                    CalcLayerQuantitiesUS1976( T, P, alt, StdAtmosAltSteps[i], TempGradient[i] );
                    break;
                }
            }
        }
        else
        {
            CalcLayerQuantitiesUS1976( T, P, StdAtmosAltSteps[i], StdAtmosAltSteps[i], 0.0 );
        }
    }

    double rho = P / ( m_Rspecific * T ); // kg/m3
    m_SoundSpeed = sqrt( gamma * m_Rspecific * T ); // m/s

    m_DensityRatio = rho / m_RHO0;
    m_PressureRatio = P / m_P0;

    // Modeling Dynamic Viscosity
    m_DynamicVisc = DynamicViscosityCalc( T, vsp::TEMP_UNIT_K, altunit );

    if ( altunit == vsp::PD_UNITS_IMPERIAL )
    {
        rho = ConvertDensity( rho, vsp::RHO_UNIT_KG_M3, vsp::RHO_UNIT_SLUG_FT3 );
    }

    T = ConvertTemperature( T, vsp::TEMP_UNIT_K, tempunit ); // Into Desired
    P = ConvertPressure( P, vsp::PRES_UNIT_PA, presunit ); // Into Desired
    m_DeltaT = delta_temp; // Back to input value

    // Assign Class Variables Appropriate Values
    m_KEAS = m_A0 * m_Mach * sqrt( m_PressureRatio );
    m_KTAS = m_KEAS / sqrt( 1.0 / m_DensityRatio );
    m_Temp = T; // Converted to Desired
    m_Pres = P; // Converted to Desired
    m_Density = rho; // Converted to Desired
}

void Atmosphere::CalcIsothermalLayerUS1976( const double & temp, double & pres, double alt, double previous_alt_step,
        double G0, double M0, double R0 )
{
    pres = pres * exp( ( ( -G0 * M0 * ( alt - previous_alt_step ) ) / 1000 ) / ( R0 * temp ) );
}

void Atmosphere::CalcGradientLayerUS1976( double & temp, double & pres, double alt, double previous_alt_step,
        double temp_grad, double G0, double M0, double R0 )
{
    pres = pres * pow( ( temp / ( temp + ( temp_grad * ( alt - previous_alt_step ) ) ) ), ( ( G0 * M0 ) / ( R0 * temp_grad ) / 1000 ) );
    temp = temp + temp_grad * ( alt - previous_alt_step );
}

void Atmosphere::CalcLayerQuantitiesUS1976( double & temp, double & pres, double alt, double previous_alt_step, double temp_grad )
{
    double G0 = 9.80665; // m/s2
    double R0 = 8.31432e-3; // (N m)/( kmol K )
    double M0 = 28.9644;

    if ( temp_grad == 0.0 )
    {
        CalcIsothermalLayerUS1976( temp, pres, alt, previous_alt_step, G0, M0, R0 );
    }
    else
    {
        CalcGradientLayerUS1976( temp, pres, alt, previous_alt_step, temp_grad, G0, M0, R0 );
    }
}

void Atmosphere::USAF1966( double alt, double delta_temp, int altunit, int tempunit, int presunit, double gamma )
{
    // Set up Sea Level Constants and Other Constants
    double m_P0 = 29.92126; // "Hg
    double m_RHO0 = 0.0023769; // slug/ft3
    double K1 = 6.87535e-6;
    double K2 = 4.80634e-5;
    double K3 = 36089.24;
    double T, P, rho;

    m_Hinf = alt;
    m_DeltaT = ConvertTemperature( delta_temp, tempunit, vsp::TEMP_UNIT_K ) - ConvertTemperature( 0.0, tempunit, vsp::TEMP_UNIT_K ); // Into K

    // Conversion if Necessary of Altitude into feet
    if ( altunit == vsp::PD_UNITS_METRIC )
    {
        alt = ConvertLength( alt, vsp::LEN_M, vsp::LEN_FT );
    }

    // Using Herrington 1966 Model
    if ( alt < 36089 )
    {
        T = ( m_T0 + m_DeltaT ) * ( 1 - K1 * alt );
        P = m_P0 * pow( ( 1 - K1 * alt ), 5.2561 );
        rho = m_RHO0 * pow( ( 1 - K1 * alt ), 4.2561 );
    }
    else if ( alt > 36089 && alt < 82021 )
    {
        // T = 216.66; // Kelvin
        T = ( m_T0 + m_DeltaT ) * ( 1 - K1 * K3 );
        P = m_P0 * ( 0.223358 * exp( -1 * K2 * ( alt - K3 ) ) );
        rho = m_RHO0 * ( 0.29707 * exp( -1 * K2 * ( alt - K3 ) ) );
    }
    else // Less than 0, Greater than 82021
    {
        alt = 82021;
        T = ( m_T0 + m_DeltaT ) * ( 1 - K1 * K3 );
        P = m_P0 * ( 0.223358 * exp( -1 * K2 * ( alt - K3 ) ) );
        rho = m_RHO0 * ( 0.29707 * exp( -1 * K2 * ( alt - K3 ) ) );
    }

    m_DensityRatio = rho / m_RHO0;
    m_PressureRatio = P / m_P0;

    m_SoundSpeed = sqrt( gamma * m_Rspecific * T ); // m/s

    // Modeling Dynamic Viscosity
    m_DynamicVisc = DynamicViscosityCalc( T, vsp::TEMP_UNIT_K, altunit );

    if ( altunit == vsp::PD_UNITS_METRIC )
    {
        alt = ConvertLength( alt, vsp::LEN_M, vsp::LEN_FT ); // Into Desired
        rho = ConvertDensity( rho, vsp::RHO_UNIT_SLUG_FT3, vsp::RHO_UNIT_KG_M3 );
    }

    T = ConvertTemperature( T, vsp::TEMP_UNIT_K, tempunit ); // Into Desired
    P = ConvertPressure( P, vsp::PRES_UNIT_INCHHG, presunit ); // Into Desired
    m_DeltaT = delta_temp; // Back to input value

    // Assign Class Variables Appropriate Values
    m_KEAS = m_A0 * m_Mach * sqrt( m_PressureRatio );
    m_KTAS = m_KEAS / sqrt( 1.0 / m_DensityRatio );
    m_Temp = T; // Converted to Desired
    m_Pres = P; // Converted to Desired
    m_Density = rho; // Converted to Desired
}

double Atmosphere::DynamicViscosityCalc( double T, int tempunit, int altunit )
{
    // Modeling Dynamic Viscosity Using NASA US Std Atm 1976 Equation
    // https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770009539.pdf
    double Beta = 1.458E-6; // kg/(s*m*K^1/2)
    double S = 110.4; // Sunderland's constant (K) 
    double T_atm = ConvertTemperature( T, tempunit, vsp::TEMP_UNIT_K );
    double dynavisc = Beta * pow( T_atm, 1.5 ) / ( T_atm + S ); // kg/m-s

    if ( altunit == vsp::PD_UNITS_IMPERIAL )
    {
        dynavisc /= 47.8803; // kg/m-s --> slug/ft-s
    }

    return dynavisc;

    //if ( medium == MEDIUM_PURE_WATER )
    //{
    //    // Using Vogel Equation
    //    // http://ddbonline.ddbst.de/VogelCalculation/VogelCalculationCGI.exe

    //    double A = -3.7188;
    //    double B = 578.919;
    //    double C = -137.546;
    //    ConvertTemperature( T, tempunit, vsp::TEMP_UNIT_K );

    //    // T must be between 273 and 373 K as it is no longer water at these points
    //    if ( T < 273 )
    //        T = 273;
    //    else if ( T > 373 )
    //        T = 373;

    //    dynavisc = exp( A + ( B / ( C + T ) ) ); // mPa*s
    //    dynavisc = dynavisc * 2.0885444 * pow( 10, -5 ); // mPa*s --> slug/ft-s

    //    if ( altunit == vsp::PD_UNITS_METRIC )
    //        dynavisc *= 47.8803; // slug/ft-s --> kg/m-s

    //    return dynavisc;
    //}
}

void Atmosphere::SetManualQualities( const double & vinf, double & temp, double & pres, double & rho,
                                     double & dynavisc, double gamma, double alt, int altunit, 
                                     int vinfunit, int tempunit, int pressunit, int flowstream )
{
    double Rspecific = 287.058; // J/kg-K
    double A0 = 661.48; // knots
    double P0 = 29.92126; // "Hg
    double RHO0 = 0.0023769; // slug/ft3

    double prestemp = ConvertPressure( pres, pressunit, vsp::PRES_UNIT_KPA ); // kPA;
    double rhotemp = rho;
    if ( altunit == vsp::PD_UNITS_IMPERIAL )
    {
        rhotemp = ConvertDensity( rhotemp, vsp::RHO_UNIT_SLUG_FT3, vsp::RHO_UNIT_KG_M3 ); // kg/m3
    }

    double temptemp = ConvertTemperature( temp, tempunit, vsp::TEMP_UNIT_K ); // K 

    RHO0 = ConvertDensity( RHO0, vsp::RHO_UNIT_SLUG_FT3, vsp::RHO_UNIT_KG_M3 ); // kg/m3
    P0 = ConvertPressure( P0, vsp::PRES_UNIT_INCHHG, vsp::PRES_UNIT_KPA ); // kPA

    if ( flowstream == vsp::ATMOS_TYPE_MANUAL_P_R )
    {
        temptemp = ( 1000.0 * prestemp / rhotemp ) / Rspecific; // kPa --> Pa ( In Kelvin )

        temp = ConvertTemperature( temptemp, vsp::TEMP_UNIT_K, tempunit ); // K --> TempUnit
    }
    else if ( flowstream == vsp::ATMOS_TYPE_MANUAL_P_T )
    {
        rhotemp = ( 1000.0 * prestemp ) / ( Rspecific * temptemp ); // kg/m3

        if ( altunit == vsp::PD_UNITS_IMPERIAL )
        {
            rho = ConvertDensity( rhotemp, vsp::RHO_UNIT_KG_M3, vsp::RHO_UNIT_SLUG_FT3 );
        }
        else if ( altunit == vsp::PD_UNITS_METRIC )
        {
            rho = rhotemp;
        }
    }
    else if ( flowstream == vsp::ATMOS_TYPE_MANUAL_R_T )
    {
        prestemp = Rspecific * temptemp * rhotemp;
        prestemp /= 1000; // Pa --> kPa
        pres = ConvertPressure( prestemp, vsp::PRES_UNIT_KPA, pressunit ); // kPa --> PressUint
    }

    m_DynamicVisc = DynamicViscosityCalc( temptemp, vsp::TEMP_UNIT_K, altunit );
    m_DensityRatio = rhotemp / m_RHO0;
    m_SoundSpeed = sqrt( gamma * ( prestemp * 1000 ) / rhotemp ); // m/s (pressure converted from kPa to Pa)

    if ( vinfunit == vsp::V_UNIT_MACH )
    {
        m_Mach = vinf;
    }
    else
    {
        double vtemp = vinf;
        if ( vinfunit == vsp::V_UNIT_KEAS )
        {
            vtemp *= sqrt( 1.0 / m_DensityRatio ); // KEAS to KTAS
        }
        
        vtemp = ConvertVelocity( vtemp, vinfunit, vsp::V_UNIT_M_S ); // m/s
        m_Mach = vtemp / m_SoundSpeed;
    }

    m_Hinf = alt;
    m_Vinf = vinf;
    m_Pres = pres;
    m_Density = rho;
    m_Temp = temp;
    m_DeltaT = 0.0;
    m_PressureRatio = sqrt( pres / P0 );
    m_KEAS = A0 * m_Mach * m_PressureRatio;
    m_KTAS = m_KEAS * m_DensityRatio;
}

void Atmosphere::UpdateMach( double vinf, int tempunit, int vinfunit )
{
    double T = ConvertTemperature( m_Temp, tempunit, vsp::TEMP_UNIT_K );

    if ( vinfunit == vsp::V_UNIT_KEAS ) // KEAS to KTAS
    {
        vinf *= sqrt( 1.0 / m_DensityRatio );
    }
    double vms = ConvertVelocity( vinf, vinfunit, vsp::V_UNIT_M_S );
    m_Mach = vms / m_SoundSpeed;
}
