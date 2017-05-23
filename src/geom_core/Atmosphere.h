#if !defined(VSP_ATMOS__INCLUDED_)
#define VSP_ATMOS__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"
#include "ParmMgr.h"
#include "UnitConversion.h"

class Atmosphere
{
public:
    Atmosphere();
    virtual ~Atmosphere();

    // US Standard Atmosphere 1976 Methods
    void USStandardAtmosphere1976(double alt, double delta_temp, int altunit,
        int tempunit, int presunit, double gamma = 1.4);
    void CalcIsothermalLayerUS1976( double & temp, double & pres, double alt, double previous_alt_step,
        double G0, double M0, double R0 );
    void CalcGradientLayerUS1976( double & temp, double & pres, double alt, double previous_alt_step,
        double temp_grad, double G0, double M0, double R0 );
    void CalcLayerQuantitiesUS1976( double & temp, double & pres, double alt, double previous_alt_step, double temp_grad );

    // US Air Force 1966
    void USAF1966(double alt, double delta_temp, int altunit,
        int tempunit, int presunit, double gamma = 1.4);

    // TODO: Add Other Atmosphere Models
    // International Standard Model (ISA)
    // COESA
    // CIRA
    // NRL MSISE00

    void SetManualQualities(double & vinf, double & temp, double & pres, double & rho,
        double & dynavisc, double gamma, int altunit, int vinfunit, int tempunit, int flowstream);
    void SetMach(double mach) { m_Mach = mach; }

    void UpdateMach( double vinf, int tempunit, int vinfunit );

    double DynamicViscosityCalc(double T, int tempunit, int altunit);

    double GetAlt() { return m_Hinf; }
    double GetDeltaT() { return m_DeltaT; }
    double GetTemp() { return m_Temp; }
    double GetPres() { return m_Pres; }
    double GetDensity() { return m_Density; }
    double GetDynaVisc() { return m_DynamicVisc; }
    double GetSoundSpeed() { return m_SoundSpeed; }
    double GetMach() { return m_Mach; }
    double GetKEAS() { return m_KEAS; }
    double GetKTAS() { return m_KTAS; }
    double GetPressureRatio() { return m_PressureRatio; }
    double GetDensityRatio() { return m_DensityRatio; }

private:

    double m_Rspecific;
    double m_T0;
    double m_P0;
    double m_RHO0;
    double m_A0;

    double m_Hinf;
    double m_DeltaT;
    double m_Vinf;
    double m_Temp;
    double m_Pres;
    double m_DynamicVisc;
    double m_Density;
    double m_SoundSpeed;
    double m_KTAS;
    double m_KEAS;
    double m_Mach;
    double m_DensityRatio;
    double m_PressureRatio;
};
#endif
