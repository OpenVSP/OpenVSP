import openvsp as vsp
import math
import Constants as const
from pathlib import Path
import matplotlib.pyplot as plt
import pickle

scriptpath = str(Path(__file__).parent.resolve())

class WarrenTest:
    '''!Class for running and collecting data from the
        Warren-12 studies
    '''
    def __init__(self):
        self.m_AlphaNpts = 9
        self.m_Cl_alpha_expected = 2.743 # rad
        self.m_Cm_alpha_expected = -3.10 # rad
        
        self.alpha_0 = -20.0 # deg
        self.alpha_f = 20.0 # deg
        
        self.m_Cl_alpha_error = [0.0]*self.m_AlphaNpts
        self.m_Cl_alpha_res = [0.0]*self.m_AlphaNpts
        self.m_Cm_alpha_error = [0.0]*self.m_AlphaNpts
        self.m_Cm_alpha_res = [0.0]*self.m_AlphaNpts
        self.m_AlphaSweepVec = [0.0]*self.m_AlphaNpts
        
        self.Cl_res = [0.0]*self.m_AlphaNpts
        self.Cm_res = [0.0]*self.m_AlphaNpts
        self.Cl_approx_vec = [0.0]*self.m_AlphaNpts
        self.Cm_approx_vec = [0.0]*self.m_AlphaNpts

        

#========================================= Warren-12 Functions =================================#
#==================== Generates the relavent parameteres. Runs the ____________      =============#
#==================== ___________________ studies. Generates the ___ tables and      =============#
#==================== _____ charts charts to include in the markdown file.           =============#
#=================================================================================================#

#========== Wrapper function for ________________________________ Code ===========================#
    def WarrenStudy(self):
        self.GenerateWarrenWings()
        self.TestWarrenWings()
        self.GenerateWarrenCharts()
#===================== Sweapt UWTess Generation Functions =====================
    def GenerateWarrenWings(self):
        #INSERT lines 5159 - 5188 from v&v script
        #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )

        #==== Set Wing Section Controls to AR, Root Chord, and Tail Chord ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()

        #==== Set Airfoil to NACA0012 and Warren-12 Wing Parms ====#
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Sweep_Location', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 53.54 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.5 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 0.5 )
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', math.sqrt(2) )

        vsp.Update()

        #==== Setup export filenames for VSPAERO sweep ====#
        fname = scriptpath + '/warren_files/vsp_files/warren_12.vsp3'

        #==== Save Vehicle to File ====#
        print('-->Saving vehicle file to: ' + fname + '\n' )
        vsp.WriteVSPFile( fname, vsp.SET_ALL )
        print( 'COMPLETE\n' )

        vsp.ClearVSPModel()

#========== Run the actual ____________ Studies ==============================#
    def TestWarrenWings(self):
        #Insert lines 5159-5377 from v&v script
        print( '-> Begin Warren Sweep Test:\n' )
        
        
        d_alpha = self.alpha_f - self.alpha_0 # deg
        
        
        #==== Open and test generated wing ====#
        fname = scriptpath + '/warren_files/vsp_files/warren_12.vsp3'
        fname_res = scriptpath + '/warren_files/vsp_files/warren_12_res.csv'

        print( 'Reading in file: ')
        print( fname )
        vsp.ReadVSPFile( fname ) # Sets VSP3 file name

        #==== Analysis: VSPAero Sweep ====#
        print( const.m_VSPSweepAnalysis )

        #==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====#
        print( const.m_CompGeomAnalysis )

        # Set defaults
        vsp.SetAnalysisInputDefaults( const.m_CompGeomAnalysis )
        vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'GeomSet', [vsp.SET_NONE], 0)
        vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'ThinGeomSet', [vsp.SET_ALL], 0)  # Thin geometry - VLM
        vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'Symmetry', const.m_SymFlagVec, 0)

        # list inputs, type, and current values
        vsp.PrintAnalysisInputs( const.m_CompGeomAnalysis )

        # Execute
        print( '\tExecuting...' )
        compgeom_resid = vsp.ExecAnalysis( const.m_CompGeomAnalysis )
        print( 'COMPLETE' )

        # Get & Display Results
        vsp.PrintResults( compgeom_resid )

        #==== Analysis: VSPAero Sweep ====#
        # Set defaults
        vsp.SetAnalysisInputDefaults(const.m_VSPSweepAnalysis)
        print(const.m_VSPSweepAnalysis)

        # Reference geometry set
        vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'GeomSet', [vsp.SET_NONE], 0)
        vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'ThinGeomSet', [vsp.SET_ALL], 0)  # Thin geometry - VLM
        vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'RefFlag', const.m_RefFlagVec, 0)
        vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Symmetry', const.m_SymFlagVec, 0)

        wid = vsp.FindGeomsWithName( 'WingGeom' )
        vsp.SetStringAnalysisInput(const.m_VSPSweepAnalysis, 'WingID', wid, 0)
        
        vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'WakeNumIter', const.m_WakeIterVec, 0)

        # Freestream Parameters
        AlphaStart = [self.alpha_0]
        AlphaEnd = [self.alpha_f]
        AlphaNpts = [self.m_AlphaNpts]
        vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', AlphaStart, 0)
        vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaEnd', AlphaEnd, 0)
        vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
        MachNpts = [1] # Start and end at 0.1
        vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', const.m_MachVec, 0)
        vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachEnd', const.m_MachVec, 0)
        vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)

        vsp.Update()

        # list inputs, type, and current values
        vsp.PrintAnalysisInputs( const.m_VSPSweepAnalysis )
        print( '' )

        # Execute
        print( '\tExecuting...' )
        rid = vsp.ExecAnalysis( const.m_VSPSweepAnalysis )
        print( 'COMPLETE' )

        # Get & Display Results
        vsp.PrintResults( rid )
        vsp.WriteResultsCSVFile( rid, fname_res )

        # Get Result ID Vec (History and Load ResultIDs)
        rid_vec = vsp.GetStringResults( rid, 'ResultsVec' )
        if ( len(rid_vec) <= 1 ):
            vsp.ClearVSPModel()
            return
            
        # Get Result from Final Wake Iteration
        for i in range(self.m_AlphaNpts) :
        
            alpha_vec = vsp.GetDoubleResults( rid_vec[i], 'Alpha' )
            self.m_AlphaSweepVec[i] = alpha_vec[len(alpha_vec) - 1]
            
            cl_vec = vsp.GetDoubleResults( rid_vec[i], 'CL' )
            self.Cl_res[i] = cl_vec[len(cl_vec) - 1]
            
            cmy_vec = vsp.GetDoubleResults( rid_vec[i], 'CMy' )
            self.Cm_res[i] = cmy_vec[len(cmy_vec) - 1]
            
            self.Cl_approx_vec[i] = self.m_Cl_alpha_expected * math.sin( math.radians( self.m_AlphaSweepVec[i]))
            self.Cm_approx_vec[i] = self.m_Cm_alpha_expected * math.sin( math.radians( self.m_AlphaSweepVec[i]))
        
        
        for i in range(self.m_AlphaNpts):
        
            if ( i == 0 ):
            
                self.m_Cl_alpha_res[i] = math.degrees((self.Cl_res[i+1] - self.Cl_res[i])/(self.m_AlphaSweepVec[i+1] - self.m_AlphaSweepVec[i]))
                self.m_Cm_alpha_res[i] = math.degrees((self.Cm_res[i+1] - self.Cm_res[i])/(self.m_AlphaSweepVec[i+1] - self.m_AlphaSweepVec[i]))
            
            elif ( i == self.m_AlphaNpts - 1 ):
            
                self.m_Cl_alpha_res[i] = math.degrees((self.Cl_res[i] - self.Cl_res[i-1])/(self.m_AlphaSweepVec[i] - self.m_AlphaSweepVec[i-1]))
                self.m_Cm_alpha_res[i] = math.degrees((self.Cm_res[i] - self.Cm_res[i-1])/(self.m_AlphaSweepVec[i] - self.m_AlphaSweepVec[i-1]))
            
            else :# Central differencing
            
                self.m_Cl_alpha_res[i] = math.degrees((self.Cl_res[i+1] - self.Cl_res[i-1])/(self.m_AlphaSweepVec[i+1] - self.m_AlphaSweepVec[i-1]))
                self.m_Cm_alpha_res[i] = math.degrees((self.Cm_res[i+1] - self.Cm_res[i-1])/(self.m_AlphaSweepVec[i+1] - self.m_AlphaSweepVec[i-1]))
            
            self.m_Cl_alpha_error[i] = (abs((self.m_Cl_alpha_res[i] - self.m_Cl_alpha_expected)/self.m_Cl_alpha_expected))*100
            self.m_Cm_alpha_error[i] = (abs((self.m_Cm_alpha_res[i] - self.m_Cm_alpha_expected)/self.m_Cm_alpha_expected))*100
        

        self.Cl_alpha_res_avg = math.degrees((self.Cl_res[self.m_AlphaNpts - 1] - self.Cl_res[0])/d_alpha) #rad
        self.Cm_alpha_res_avg = math.degrees((self.Cm_res[self.m_AlphaNpts - 1] - self.Cm_res[0])/d_alpha) #rad

        self.m_Warren_Sweep_Cl_alpha_Err = (abs((self.Cl_alpha_res_avg - self.m_Cl_alpha_expected)/self.m_Cl_alpha_expected))*100
        self.m_Warren_Sweep_Cm_alpha_Err = (abs((self.Cm_alpha_res_avg - self.m_Cm_alpha_expected)/self.m_Cm_alpha_expected))*100
        
        vsp.ClearVSPModel()
#======== Use Matplotlib to Create tables and Graphs for the _________ Studies =#
    def GenerateWarrenCharts(self):
        # title = 'Warren-12 Geometry Setup'
        # header = ['Airfoil', 'AR', 'Root Chord', 'Tip Chord', 'Λ (°)', 'Λ Location', 'Span Tess (U)','Chord Tess (W)','Tip Clustering']
        # data = [['NACA0012'], ['2√2'], ['1.5'],['0.5'],['53.54'],['0.0'],['6'],['33'],['1.0']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/warren_files/warren_img/warren/geometrysetup.png')
        
        # title = 'Warren-12 VSPAERO Setup'
        # header = ['Analysis', 'Method', 'α (°)', 'β (°)', 'M', 'Wake Iterations']
        # data = [['Sweep'], ['VLM'], [str(self.alpha_0)+' to '+str(self.alpha_f)+', npts: '+str(self.m_AlphaNpts)],['0.0'],['0.1'],['3']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/warren_files/warren_img/warren/vspaerosetup.png')
        
        fig, ax = plt.subplots()
        ax.set_title('Warren VLM: Cl vs Alpha')
        ax.set_xlabel('Alpha (°)')
        ax.set_ylabel('Cl')
        ax.plot(self.m_AlphaSweepVec,self.Cl_res,'o-', label='VSPAERO',color=const.colors[0])
        ax.plot(self.m_AlphaSweepVec,self.Cl_approx_vec, label='Expected',color=const.colors[-1])
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/warren_files/warren_img/warren/warrenrawcl.svg', bbox_inches='tight')
        
        fig, ax = plt.subplots()
        ax.set_title('Warren VLM Cl_alpha Alpha Sensitivity')
        ax.set_xlabel('Alpha (°)')
        ax.set_ylabel(r'Cl_alpha % Difference')
        ax.plot(self.m_AlphaSweepVec,self.m_Cl_alpha_error,'o-' , label=r'% Difference',color=const.colors[0])
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/warren_files/warren_img/warren/warrenpercentcl.svg', bbox_inches='tight')
        
        fig, ax = plt.subplots()
        ax.set_title('Warren VLM: Cm vs Alpha')
        ax.set_xlabel('Alpha (°)')
        ax.set_ylabel('Cm')
        ax.plot(self.m_AlphaSweepVec,self.Cm_res,'o-', label='VSPAERO',color=const.colors[0])
        ax.plot(self.m_AlphaSweepVec,self.Cm_approx_vec, label='Expected',color=const.colors[-1])
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/warren_files/warren_img/warren/warrenrawcm.svg', bbox_inches='tight')
        
        fig, ax = plt.subplots()
        ax.set_title('Warren VLM Cm_alpha Alpha Sensitivity')
        ax.set_xlabel('Alpha (°)')
        ax.set_ylabel(r'Cm_alpha % Difference')
        ax.plot(self.m_AlphaSweepVec,self.m_Cm_alpha_error,'o-' , label=r'% Difference',color=const.colors[0])
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/warren_files/warren_img/warren/warrenpercentcm.svg', bbox_inches='tight')
        
        
        # title = 'Warren Results'
        # header = ['α (°)', 'CLα Expected (rad)', 'CLα Result (rad)', 'CLα % Difference', 'CMα Expected (rad)', 'CMα Result (rad)', 'CMα % Difference']
        # data = [self.m_AlphaSweepVec, [self.m_Cl_alpha_expected]*self.m_AlphaNpts, self.m_Cl_alpha_res,self.m_Cl_alpha_error,[self.m_Cm_alpha_expected]*self.m_AlphaNpts, self.m_Cm_alpha_res,self.m_Cm_alpha_error]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/warren_files/warren_img/warren/results.png')

        
    
def runWarrenstudy(mode = 3):
    setup_filepaths()
    
    test = WarrenTest()
    if (mode == 1 or mode == 2):
        with open(scriptpath+'/warren_files/warrentest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (mode == 1): 
        test.GenerateWarrenCharts()
    if (mode == 3):
        test.WarrenStudy()
        with open(scriptpath+'/warren_files/warrentest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (mode == 2):
        test.TestWarrenWings()
        test.GenerateWarrenCharts()
        with open(scriptpath+'/warren_files/warrentest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)        
            
def setup_filepaths():
    scriptpathlib = Path(__file__).parent.resolve()
    testnames = ['warren_files/']
    subnames = [['warren_img/','vsp_files/']]
    subsubnames = [[['warren'],['']]]
    for i in range(len(testnames)):
        for j in range(len(subnames[i])):
            for k in range(len(subsubnames[i][j])):
                dirname = Path.joinpath(scriptpathlib, testnames[i]+subnames[i][j]+subsubnames[i][j][k])
                dirname.mkdir(parents=True, exist_ok=True)
                
if __name__ == '__main__':
    runWarrenstudy(mode = 3)    
