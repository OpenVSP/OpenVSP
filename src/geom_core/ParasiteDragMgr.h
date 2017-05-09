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
#include "Util.h"

#include <string>
#include <vector>

// Structure containing all main table data
struct ParasiteDragTableRow
{
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
    double fineRat;
    int GeomShapeType;
    int FFEqnChoice;
    string FFEqnName;
    double FF;
    double Q;
    double f;
    double CD;
    double PercTotalCd;
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
    double PercTotalCd;
    double f;
};

//==== ParasiteDrag Manager ====//
class ParasiteDragMgrSingleton : public ParmContainer
{
public:
    // ==== ENUMERATORS ==== //
    enum { MANUAL_REF = 0, COMPONENT_REF, }; // Ref Flag
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
    virtual void ParmChanged(Parm* parm_ptr, int type); // Required for extensions of ParmContainers

    // Main Table Related Functions
    void InitTableVec(); // Initialize all table rows to default struct
    void LoadMainTableUserInputs(); // Load User Input Parm Values from Geoms

    // Calculation Methods
    void SetupFullCalculation();
    int CalcRowSize();
    void Calculate_Swet();
    void Calculate_Lref();
    void CalcReferenceBodyLength(int index);
    void CalcReferenceChord(int index);
    void Calculate_Re();
    void CalcRePowerDivisor();
    void ReynoldsNumCalc(int index);
    void Calculate_Cf();
    void CalcPartialTurbulence(int i, double lref, double vinf, double kineVisc);
    void Calculate_fineRat();
    void Calculate_FF();
    void Calculate_AvgSweep(vector <DegenStick> degenSticks);
    void Calculate_f();
    void Calculate_Cd();
    void Calculate_ALL();
    string ComputeBuildUp(); // Used only through API

    // Grouped Geom Overwrite methods
    void OverwritePropertiesFromAncestorGeom();

    // Switch Case Methods For Friction Coefficient Selections
    double CalcTurbCf(double ReyIn, double ref_leng, int cf_case, double roughness_h,
        double gamma, double taw_tw_ratio, double te_tw_ratio);
    double CalcLamCf(double ReyIn, int cf_case);
    string AssignTurbCfEqnName(int cf_case);
    string AssignLamCfEqnName(int cf_case);

    // Switch Case Methods for Form Factor Selections
    double CalcFFWing(double toc, int ff_case, double perc_lam,
        double sweep25, double sweep50);
    double CalcFFBody(double longF, double FR, int ff_case, double ref_leng, double max_x_area);
    string AssignFFWingEqnName(int ff_case);
    string AssignFFBodyEqnName(int ff_case);

    // Setter Methods
    void SetSref(double sref) { m_Sref.Set(sref); }
    void SetActiveGeomVec();
    void SetFreestreamParms();
    void SetCurrExcresIndex(int val) { m_CurrentExcresIndex = val; }
    void SetExcresLabel(const string & newLabel);

    // Getter Methods
    vector < ParasiteDragTableRow > GetMainTableVec() { return m_TableRowVec; }
    vector < ExcrescenceTableRow > GetExcresVec() { return m_ExcresRowVec; }
    vector <string> GetPDGeomIDVec() { return m_PDGeomIDVec; }
    int GetReynoldsDivisor() { return m_ReynoldsPowerDivisor; }
    double GetLrefSigFig(); // For display precision purposes
    double GetGeomfTotal() { return m_GeomfTotal; }
    double GetGeomPercTotal() { return m_GeomPercTotal; }
    double GetExcresfTotal() { return m_ExcresfTotal; }
    double GetExcresPercTotal() { return m_ExcresPercTotal; }
    double GetfTotal() { return m_GeomfTotal + m_ExcresfTotal; }
    double GetPercTotal() { return m_GeomPercTotal + m_ExcresPercTotal; }
    int GetCurrExcresIndex() { return m_CurrentExcresIndex; }
    double GetGeometryCd();
    double GetSubTotalCD();
    double GetTotalCD();
    vector < string > GetExcresIDs();
    string GetCurrentExcresLabel();
    string GetCurrentExcresTypeString();
    double GetCurrentExcresValue();
    int GetCurrentExcresType();

    // Excresence Related Functions
    void AddExcrescence();
    void AddExcrescence(const std::string &excresName, int excresType, double excresVal);
    void DeleteExcrescence();
    void DeleteExcrescence(int index);
    double CalcPercentageGeomCd(double val);
    double CalcPercentageTotalCD(double val);
    double CalcDragAreaCd(double val);
    double GetSubTotalExcresCd();
    double GetTotalExcresCD();
    void ConsolidateExcres();

    // Update Methods
    void Update();
    void UpdateWettedAreaTotals();
    void UpdateRefWing();
    void UpdateAtmos();
    void UpdateVinf(int newunit);
    void UpdateAlt(int newunit);
    void UpdateAltLimits();
    void UpdateTemp(int newunit);
    void UpdateTempLimits();
    void UpdatePres(int newunit);
    void UpdatePercentageCD();
    void UpdateParmActivity();
    void UpdateExportLabels();
    void UpdateExcres();
    void UpdateCurrentExcresVal();

    // Export Methods
    string ExportToCSV();
    string ExportToCSV(const string & file_name);

    // General Methods
    void ClearInputVectors();
    void ClearOutputVectors();
    virtual xmlNodePtr EncodeXml(xmlNodePtr & node);
    virtual xmlNodePtr DecodeXml(xmlNodePtr & node);

    // Miscellaneous Methods
    void SortMap();
    void SortMapByWettedArea();
    void SortMapByPercentageCD();
    void SortMainTableVecByGroupedAncestorGeoms();
    bool CheckAllTrue( vector < bool > vec );
    void DeactivateParms();
    bool IsSameGeomSet();
    bool IsNotZeroLineItem(int index);
    void RefreshDegenGeom();

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

private:
    ParasiteDragMgrSingleton();

    Atmosphere m_Atmos; // Atmosphere class contains all atmosphere related qualities

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
    vector <string> geo_geomID;
    vector <string> geo_subsurfID;
    vector <string> geo_label;
    vector < int > geo_surfNum;
    vector < int > geo_groupedAncestorGen;
    vector < bool > geo_expandedList;
    vector <double> geo_swet;
    vector <double> geo_lref;
    vector <double> geo_Re;
    vector <double> geo_Roughness;
    vector <double> geo_TeTwRatio;
    vector <double> geo_TawTwRatio;
    vector <double> geo_percLam;
    vector <double> geo_cf;
    vector <double> geo_fineRat;
    vector <int> geo_ffType;
    vector <string> geo_ffName;
    vector <int> geo_shapeType;
    vector <double> geo_ffIn;
    vector <double> geo_ffOut;
    vector <double> geo_Q;
    vector <double> geo_f;
    vector <double> geo_Cd;
    vector <double> geo_percTotalCd;

    // Excrescence Vectors
    vector < string > excres_Label;
    vector < string > excres_Type;
    vector < double > excres_Input;
    vector < double> excres_Amount;
    vector < double > excres_PercTotalCd;

    // Totals
    double m_GeomfTotal;
    double m_GeomPercTotal;

    double m_ExcresfTotal;
    double m_ExcresPercTotal;
};

#define ParasiteDragMgr ParasiteDragMgrSingleton::getInstance()

#endif
