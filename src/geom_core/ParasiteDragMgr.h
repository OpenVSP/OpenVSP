//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParasiteDragMgr.h: interface for the ParasiteDrag Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(PARASITEDRAG__INCLUDED_)
#define PARASITEDRAG__INCLUDED_

#include "Geom.h"
#include "Parm.h"
#include "Atmosphere.h"
#include "UnitConversion.h"
#include "VspUtil.h"

#include <string>
#include <vector>

// Structure containing all main table data
struct ParasiteDragTableRow
{
    bool MasterRow;
    string GeomID;
    string SubSurfID;
    string Label;
    double Swet;
    double Lref;
    double Re;
    double Roughness;
    double TeTwRatio;
    double TawTwRatio;
    double PercLam;
    double Cf;
    double fineRatorToC;
    int GeomShapeType;
    int FFEqnChoice;
    double FF;
    double Q;
    double f;
    double CD;
    double PercTotalCD;
    int GroupedAncestorGen;
    int SurfNum;
    bool ExpandedList;
};

// Structure containing all excres table data
struct ExcrescenceTableRow
{
    string Label;
    int Type;
    string TypeString;
    double Input;
    double Amount;
    double PercTotalCD;
    double f;
};

//==== ParasiteDrag Manager ====//
class ParasiteDragMgrSingleton : public ParmContainer
{
public:
    // ==== ENUMERATORS ==== //
    enum { PD_SORT_NONE = 0, PD_SORT_WETTED_AREA, PD_SORT_PERC_CD }; // Sort by

    // Required Refresh function for NEW file
    virtual void Renew();

    // Default Struct Method
    void SetDefaultStruct();

    // Required for Mgr Style Codes
    static ParasiteDragMgrSingleton& getInstance()
    {
        static ParasiteDragMgrSingleton instance;
        return instance;
    }
    virtual void ParmChanged( Parm* parm_ptr, int type ); // Required for extensions of ParmContainers

    // Main Table Related Functions
    void InitTableVec(); // Initialize all table rows to default struct
    void LoadMainTableUserInputs(); // Load User Input Parm Values from Geoms

    // Calculation Methods
    void SetupFullCalculation();
    int CalcRowSize();
    void Calculate_Swet();
    void Calculate_Lref();
    double CalcReferenceLength( int index );
    double CalcReferenceBodyLength( int index );
    double CalcReferenceChord( int index );
    void Calculate_Re();
    void CalcRePowerDivisor();
    void ReynoldsNumCalc( int index );
    void Calculate_Cf();
    double CalcPartialTurbulence( double perclam, double re, double lref, double reqL,
        double roughness, double tawtwrat, double tetwrat );
    void Calculate_fineRat_and_toc();
    double CalculateFinessRatioAndTOC( int isurf, int irow );
    void Calculate_FF();
    double CalculateFormFactor( int isurf, int irow );
    void Calculate_AvgSweep( vector <DegenStick> degenSticks );
    void Calculate_f();
    void Calculate_CD();
    void Calculate_ALL();
    string ComputeBuildUp(); // Used only through API

    // Grouped Geom Overwrite methods
    void OverwritePropertiesFromAncestorGeom();

    // Switch Case Methods For Friction Coefficient Selections
    double CalcTurbCf( double ReyIn, double ref_leng, int cf_case, double roughness_h,
                       double gamma, double taw_tw_ratio, double te_tw_ratio );
    static double CalcLamCf( double ReyIn, int cf_case );
    string AssignTurbCfEqnName( int cf_case );
    static string AssignLamCfEqnName( int cf_case );

    bool IsTurbBlacklisted( int cf_case );
    int FindAlternateTurb( int cf_case );
    void CorrectTurbEquation();

    // Switch Case Methods for Form Factor Selections
    double CalcFFWing( double toc, int ff_case, double perc_lam,
                       double sweep25, double sweep50 );
    double CalcFFBody(double FR, int ff_case );
    static string AssignFFWingEqnName( int ff_case );
    static string AssignFFBodyEqnName( int ff_case );

    // Setter Methods
    void SetSref( double sref )
    {
        m_Sref.Set( sref );
    }
    void SetActiveGeomVec();
    void SetFreestreamParms();
    void SetCurrExcresIndex( int val )
    {
        m_CurrentExcresIndex = val;
    }
    void SetExcresLabel( const string & newLabel );
    void SetRecomputeGeomFlag( const bool & val )
    {
        m_RecomputeGeom = val;
    }

    // Getter Methods
    vector < ParasiteDragTableRow > GetMainTableVec()
    {
        return m_TableRowVec;
    }
    vector < ExcrescenceTableRow > GetExcresVec()
    {
        return m_ExcresRowVec;
    }
    vector <string> GetPDGeomIDVec()
    {
        return m_PDGeomIDVec;
    }
    int GetReynoldsDivisor()
    {
        return m_ReynoldsPowerDivisor;
    }
    int GetLrefSigFig(); // For display precision purposes
    double GetGeomfTotal()
    {
        return m_GeomfTotal;
    }
    double GetGeomPercTotal()
    {
        return m_GeomPercTotal;
    }
    double GetExcresfTotal()
    {
        return m_ExcresfTotal;
    }
    double GetExcresPercTotal()
    {
        return m_ExcresPercTotal;
    }
    double GetfTotal()
    {
        return m_GeomfTotal + m_ExcresfTotal;
    }
    double GetPercTotal()
    {
        return m_GeomPercTotal + m_ExcresPercTotal;
    }
    int GetCurrExcresIndex()
    {
        return m_CurrentExcresIndex;
    }
    bool GetRecomputeGeomFlag()
    {
        return m_RecomputeGeom;
    }
    double GetGeometryCD();
    double GetSubTotalCD();
    double GetTotalCD();
    string GetCurrentExcresLabel();
    int GetCurrentExcresType();

    // Excresence Related Functions
    void AddExcrescence();
    void AddExcrescence( const std::string &excresName, int excresType, double excresVal );
    void DeleteExcrescence();
    void DeleteExcrescence( int index );
    double CalcPercentageGeomCD( double val );
    double CalcPercentageTotalCD( double val );
    double CalcDragAreaCD( double val );
    double GetSubTotalExcresCD();
    double GetTotalExcresCD();
    void ConsolidateExcres();

    // Update Methods
    void Update();
    void UpdateWettedAreaTotals();
    bool ShouldAddSubSurfToMasterGeom( const size_t &i, const size_t &j );
    bool ShouldAddGeomToMasterGeom( const size_t &i, const size_t &j );
    void UpdateRefWing();
    void UpdateAtmos();
    void UpdateVinf( int newunit );
    void UpdateAlt( int newunit );
    void UpdateTemp( int newunit );
    void UpdateTempLimits();
    void UpdatePres( int newunit );
    void UpdatePercentageCD();
    void UpdateMasterPercCD();
    void UpdateParmActivity();
    void UpdateExportLabels();
    void UpdateExcres();
    void UpdateCurrentExcresVal();

    // Export Methods
    string ExportToCSV();
    string ExportToCSV( const string & file_name );

    // General Methods
    void ClearInputVectors();
    void ClearOutputVectors();
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    // Miscellaneous Methods
    void SortMap();
    void SortMapByWettedArea();
    void SortMapByPercentageCD();
    void SortGeometryByPercTotalCD();
    void SortExcresByPercTotalCD();
    void SortMainTableVecByGroupedAncestorGeoms();
    static bool CheckAllTrue( vector < bool > vec );
    void DeactivateParms();
    bool IsSameGeomSet();
    bool IsNotZeroLineItem( int index );
    void RefreshBaseDataVectors();
    void RenewDegenGeomVec();
    bool HasSameNames();
    bool IsCaclualted();

    // Reference Qualities Parms
    Parm m_Sref;
    string m_RefGeomID;
    IntParm m_RefFlag;

    // Options Parms
    IntParm m_SortByFlag;
    IntParm m_LamCfEqnType;
    IntParm m_TurbCfEqnType;
    IntParm m_VinfUnitType;
    IntParm m_AltLengthUnit;
    IntParm m_SetChoice;
    IntParm m_LengthUnit;
    IntParm m_TempUnit;
    IntParm m_PresUnit;

    // Excrescence Parm
    Parm m_ExcresValue;
    IntParm m_ExcresType;
    string m_ExcresName;
    int m_CurrentExcresIndex;

    // Air Qualities Parms
    Parm m_Vinf;
    Parm m_Hinf;
    IntParm m_FreestreamType;
    Parm m_Temp;
    Parm m_DeltaT;
    Parm m_Pres;
    Parm m_Rho;
    Parm m_DynaVisc;
    Parm m_SpecificHeatRatio;
    Parm m_KineVisc;
    Parm m_Mach;
    Parm m_ReqL;

    // CSV File Name Default
    string m_FileName;

    BoolParm m_ExportSubCompFlag;

    // Labels for Export
    string m_SwetLabel;
    string m_LrefLabel;
    string m_fLabel;
    string m_AltLabel;
    string m_VinfLabel;
    string m_TempLabel;
    string m_PresLabel;
    string m_RhoLabel;
    string m_SrefLabel;
    string m_TurbCfEqnName;
    string m_LamCfEqnName;

    Atmosphere m_Atmos; // Atmosphere class contains all atmosphere related qualities

private:
    ParasiteDragMgrSingleton();

    vector < ParasiteDragTableRow > m_TableRowVec;
    vector < ExcrescenceTableRow > m_ExcresRowVec;
    ParasiteDragTableRow m_DefaultStruct;

    // Main Table Geom Related Vectors
    vector <string> m_PDGeomIDVec;
    vector <DegenGeom> m_DegenGeomVec;
    Results* m_CompGeomResults;

    // Miscellaneous Variables
    int m_ReynoldsPowerDivisor; // Used to create a fluid reynolds number display
    int m_RowSize;              // Number of rows for main table

    // Form Factor Variables
    // Some equations used quarter chord and others use half chord sweep angles
    double m_Sweep25;
    double m_Sweep50;

    // Main Build Up Table Variables
    vector <bool> m_geo_masterRow;
    vector <string> m_geo_geomID;
    vector <string> m_geo_subsurfID;
    vector <string> m_geo_name;
    vector <string> m_geo_label;
    vector < int > m_geo_surfNum;
    vector < int > m_geo_groupedAncestorGen;
    vector < bool > m_geo_expandedList;
    vector <double> m_geo_swet;
    vector <double> m_geo_lref;
    vector <double> m_geo_Re;
    vector <double> m_geo_Roughness;
    vector <double> m_geo_TeTwRatio;
    vector <double> m_geo_TawTwRatio;
    vector <double> m_geo_percLam;
    vector <double> m_geo_cf;
    vector <double> m_geo_fineRat_or_toc;
    vector <int> m_geo_ffType;
    vector <string> m_geo_ffName;
    vector <int> m_geo_shapeType;
    vector <double> m_geo_ffIn;
    vector <double> m_geo_ffOut;
    vector <double> m_geo_Q;
    vector <double> m_geo_f;
    vector <double> m_geo_CD;
    vector <double> m_geo_percTotalCD;

    // Excrescence Vectors
    vector < string > m_excres_Label;
    vector < string > m_excres_Type;
    vector < double > m_excres_Input;
    vector < double > m_excres_f;
    vector < double> m_excres_Amount;
    vector < double > m_excres_PercTotalCD;

    // Totals
    double m_GeomfTotal;
    double m_GeomPercTotal;

    double m_ExcresfTotal;
    double m_ExcresPercTotal;

    // Execution Control
    bool m_RecomputeGeom;

    vector<int> m_TurbBlackList;
    vector<int> m_TurbAlternateList;

    const int m_TurbTypeDefault = vsp::CF_TURB_SCHLICHTING_COMPRESSIBLE;
};

#define ParasiteDragMgr ParasiteDragMgrSingleton::getInstance()

#endif
