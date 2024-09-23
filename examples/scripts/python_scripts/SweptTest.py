import openvsp as vsp
import math
import Constants as const
import traceback
from pathlib import Path
import matplotlib.pyplot as plt
import pickle

scriptpath = str(Path(__file__).parent.resolve())

class SweptTest:
    '''!Class for running and collecting data from the
        swept wing studies
    '''
    
    def __init__(self):
       
      
        self.m_halfAR = [0] * 6
        self.m_halfAR[0] = 2.5
        self.m_halfAR[1] = 5.006975
        self.m_halfAR[2] = 7.5
        self.m_halfAR[3] = 12.5
        self.m_halfAR[4] = 20.000
        self.m_halfAR[5] = 30
        
        self.m_Tess_U = [0] * 4
        self.m_Tess_U[0] = 5
        self.m_Tess_U[1] = 12
        self.m_Tess_U[2] = 20
        self.m_Tess_U[3] = 41

        self.m_Tess_W = [0] * 4
        self.m_Tess_W[0] = 9
        self.m_Tess_W[1] = 18
        self.m_Tess_W[2] = 29
        self.m_Tess_W[3] = 51
        
        self.m_Sweep = [0] * 5
        self.m_Sweep[0] = 0
        self.m_Sweep[1] = 10.005
        self.m_Sweep[2] = 20
        self.m_Sweep[3] = 30.005
        self.m_Sweep[4] = 40
        
        
        self.Error_Cla = [[0.0]*len(self.m_Tess_W) for i in range(len(self.m_Tess_U))] #array<array<double>> index 0: UTess, index 1: WTess
        self.Error_Cla_W_Tess_Sensitivity = [[0.0]*len(self.m_Tess_U) for i in range(len(self.m_Tess_W))]
        self.Cl_alpha_vlm = [[0.0]*len(self.m_Sweep) for i in range(len(self.m_halfAR))]
        self.Cl_alpha_pm = [[0.0]*len(self.m_Sweep) for i in range(len(self.m_halfAR))]
        self.Cl_alpha_theo_multi = [[0.0]*len(self.m_Sweep) for i in range(len(self.m_halfAR))]
        self.Avg_Cla_Error_VLM = [0.0]*len(self.m_Sweep)
        self.Avg_Cla_Error_PM = [0.0]*len(self.m_Sweep)
        
    # def SweptWingStudy(self):
    #     if(self.uw):
    #         self._someuwStudy()
    #     if(self.ar):
    #         self._somearStudy()

#========================================= SweptUWTess Functions =================================#
#==================== Generates the relavent parameteres. Runs the ____________      =============#
#==================== ___________________ studies. Generates the ___ tables and      =============#
#==================== _____ charts charts to include in the markdown file.           =============#
#=================================================================================================#

#========== Wrapper function for ________________________________ Code ===========================#
    def SweptUWTessStudy(self):
        self.GenerateSweptUWTessWings()
        self.TestSweptUWTessWings()
        self.GenerateSweptUWTessCharts()
#===================== Sweapt UWTess Generation Functions =====================
    def GenerateSweptUWTessWings(self):
        #INSERT lines 4612 - 4658 from v&v script
                # Add Wing Geometry
        wing_id = vsp.AddGeom( 'WING', '' )
        
        # Set Wing Section
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()
        
        # Set NACA 0012 Airfoil and Common Parms 
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1.0 )
        
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        x = 1 # AR
        s = 3 # Sweep
        
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', self.m_halfAR[x] ); # Constant AR
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', self.m_Sweep[s] ); # Constant Sweep
        vsp.SetParmVal( wing_id, 'OutCluster', 'XSec_1', 1.0 ); # Constant Tip Clustering
        
        vsp.Update()
        
        for u in range(len(self.m_Tess_U)):
            for w in range(len(self.m_Tess_W)):
                vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] ) # Constant U Tess
                vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] ) # Constant W Tess
                
                vsp.Update()

                # Setup export filenames for AR Study
                fname = scriptpath + '/swept_files/vsp_files/Swept_U' + str(int(self.m_Tess_U[u]*1000)) + '_W' + str(int(self.m_Tess_W[w]*1000)) + '.vsp3'

                # Save Vehicle to File
                message = '-->Saving vehicle file to: ' + fname + '\n'
                print( message )
                vsp.WriteVSPFile( fname, vsp.SET_ALL )
                print( 'COMPLETE\n' )
            
        
        vsp.ClearVSPModel()

#========== Run the actual ____________ Studies ==============================#
    def TestSweptUWTessWings(self):
        #Insert lines 4955-5056 from v&v script
        print( '-> Begin Swept Wing UW Tesselation Test:\n' )
        
        x = 1 # AR
        s = 2 # Sweep
        
        
        
        # Calculate Experimental and Theoretical Values
        C_bot_two = 1 + (pow((math.tan(math.radians(self.m_Sweep[s]))),2)/pow(const.b,2))
        C_top = 2*math.pi*2*self.m_halfAR[x]
        C_bot_one_theo = (pow((2*self.m_halfAR[x]),2)*pow(const.b,2))/pow(const.k_theo,2)
        C_bot_theo = (2 + (math.sqrt((C_bot_one_theo*C_bot_two)+4)))
        C_l_alpha_exper_theo = C_top/C_bot_theo
        Cl_alpha_theo = math.radians(C_l_alpha_exper_theo) # rad --> deg
        Lift_angle_theo = 1/Cl_alpha_theo # Cl to lift angle
        
        
        
        for u in range(len(self.m_Tess_U)): 
            for w in range(len(self.m_Tess_W)):
            
                # Open the file
                fname = scriptpath + '/swept_files/vsp_files/Swept_U' + str(int(self.m_Tess_U[u]*1000)) + '_W' + str(int(self.m_Tess_W[w]*1000)) + '.vsp3'
                fname_res = scriptpath + '/swept_files/vsp_files/Swept_U' + str(int(self.m_Tess_U[u]*1000)) + '_W' + str(int(self.m_Tess_W[w]*1000)) + '_res.csv'

                print( 'Reading in file: ' )
                print( fname )
                vsp.ReadVSPFile( fname ) # Sets VSP3 file name
                
                #==== Analysis: VSPAeroSinglePoint ====#
                print( const.m_VSPSweepAnalysis )

                #==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====#
                print( const.m_CompGeomAnalysis )

                # Set defaults
                vsp.SetAnalysisInputDefaults( const.m_CompGeomAnalysis )

                # list inputs, type, and current values
                vsp.PrintAnalysisInputs( const.m_CompGeomAnalysis )

                # Execute
                print( '\tExecuting...' )
                compgeom_resid = vsp.ExecAnalysis( const.m_CompGeomAnalysis )
                print( 'COMPLETE' )

                # Get & Display Results
                vsp.PrintResults( compgeom_resid )

                #==== Analysis: VSPAero Single Point ====#
                # Set defaults
                vsp.SetAnalysisInputDefaults(const.m_VSPSweepAnalysis)
                print(const.m_VSPSweepAnalysis)

                # Reference geometry set
                vsp.SetIntAnalysisInput( const.m_VSPSweepAnalysis, 'GeomSet', const.m_GeomVec, 0 )
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'RefFlag', const.m_RefFlagVec, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Symmetry', const.m_SymFlagVec, 0)

                wid = vsp.FindGeomsWithName( 'WingGeom' )
                vsp.SetStringAnalysisInput(const.m_VSPSweepAnalysis, 'WingID', wid, 0)

                # Freestream Parameters
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', const.m_AlphaVec, 0)
                AlphaNpts = [1]
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'Machstart', const.m_MachVec, 0)
                MachNpts = [1]
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'WakeNumIter', const.m_WakeIterVec, 0)

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
                if len(rid_vec) > 0 :
                
                    # Get History Results (rid_vec[0]) from Final Wake Iteration in History Result
                    cl_vec = vsp.GetDoubleResults( rid_vec[0], 'CL' )
                    Cl_res = cl_vec[len(cl_vec) - 1]

                    # Calculate Error
                    C_l_alpha_vsp = Cl_res # alpha = 1.0 (deg)
                    self.Error_Cla[u][w] = (abs((C_l_alpha_vsp - Cl_alpha_theo)/Cl_alpha_theo))*100
                    self.Error_Cla_W_Tess_Sensitivity[w][u] = self.Error_Cla[u][w]
                
                
                vsp.ClearVSPModel()
        
#======== Use Matplotlib to Create tables and Graphs for the _________ Studies =#
    def GenerateSweptUWTessCharts(self):
        
        # title = 'Tesselation Study Geometry Setup'
        # header = ['Airfoil', 'AR', 'Root Chord', 'Tip Chord', 'Λ (°)', 'Λ Location', 'Span Tess (U)','Chord Tess (W)', 'LE Clustering', 'TE Clustering','Tip Clustering']
        # data = [['NACA0012'], ['10'], ['1.0'],['1.0'],['30.0'],['0.5'],[str(self.m_Tess_U[0])+' to '+str(self.m_Tess_U[-1])],[str(self.m_Tess_W[0])+' to '+str(self.m_Tess_W[-1])],['0.2'],['1.0'],['1.0']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/swept_files/swept_img/chord_tesselation/geometrysetup.png')
        
        # title = 'Tesselation Study Geometry Setup'
        # header = ['Case','Analysis', 'Method', 'α (°)', 'β (°)', 'M', 'Wake Iterations']
        # data = [['1'],['Single Point'], ['VLM'], ['1.0'],['0.0'],['0.1'],['3']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/swept_files/swept_img/chord_tesselation/vspaerosetup.png')
        
        fig, ax = plt.subplots()
        ax.set_title('Swept Wing VLM Cl_alpha Span Tesselation (U Tess) Sensitivity')
        ax.set_xlabel('Chord Tesselation (W Tess)')
        ax.set_ylabel(r'Cl_alpha % Error')
        for i in range(len(self.Error_Cla)):
            ax.plot(self.m_Tess_W,self.Error_Cla[i], 'o-', label='U Tess:'+str(self.m_Tess_U[i]),color=const.colors[i])
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/swept_files/swept_img/span_tesselation/span_tess.svg', bbox_inches='tight')
        
        fig, ax = plt.subplots()
        ax.set_title('Swept Wing VLM Cl_alpha Chord Tesselation (W Tess) Sensitivity')
        ax.set_xlabel('Span Tesselation (U Tess)')
        ax.set_ylabel(r'Cl_alpha % Error')
        for i in range(len(self.Error_Cla)):
            ax.plot(self.m_Tess_U,self.Error_Cla_W_Tess_Sensitivity[i], 'o-', label='W Tess:'+str(self.m_Tess_W[i]),color=const.colors[i])
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/swept_files/swept_img/chord_tesselation/chord_tess.svg', bbox_inches='tight')

        

#========================================= SweptARSweep Functions =================================#
#==================== Generates the relavent parameteres. Runs the ____________      =============#
#==================== ___________________ studies. Generates the ___ tables and      =============#
#==================== _____ charts charts to include in the markdown file.           =============#
#=================================================================================================#

#========== Wrapper function for ________________________________ Code ===========================#
    def SweptARStudy(self):
        self.GenerateSweptARSweepWings()
        self.TestSweptARSweepWings()
        self.GenerateSweptARSweepCharts()
#===================== Sweapt ARSweep Generation Functions =====================
    def GenerateSweptARSweepWings(self):
        #Insert lines 4561-4607 from v&v script
         #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )
        
        #==== Set Wing Section ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()
        
        #==== Set NACA 0012 Airfoil and Common Parms 
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1.0 )
        
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        u = 3 # UTess
        w = 3 # WTess
        
        vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] ) # Constant U Tess
        vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] ) # Constant W Tess
        vsp.SetParmVal( wing_id, 'OutCluster', 'XSec_1', 1.0 ) # Constant Tip Clustering
        
        vsp.Update()
        
        for x in range(len(self.m_halfAR)):
            for s in range(len(self.m_Sweep)):
                vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', self.m_halfAR[x] )
                vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', self.m_Sweep[s] )

                vsp.Update()

                #==== Setup export filenames for AR Study ====#
                fname = scriptpath + '/swept_files/vsp_files/Swept_AR' + str(int(2*self.m_halfAR[x]*1000)) + '_Sweep' + str(int(self.m_Sweep[s]*1000)) + '.vsp3'

                #==== Save Vehicle to File ====#
                message = '-->Saving vehicle file to: ' + fname + '\n'
                print( message )
                vsp.WriteVSPFile( fname, vsp.SET_ALL )
                print( 'COMPLETE\n' )

        
        vsp.ClearVSPModel()
        
#========== Run the actual ____________ Studies ==============================#
    def TestSweptARSweepWings(self):
        #Insert lines 4663-4848 from v&v script
        print( '-> Begin Swept Wing AR Sweep Test:\n' )
        
        num_AR = len(self.m_halfAR)
        num_Sweep = len(self.m_Sweep)
        
        Lift_angle_vlm = [[0]*num_Sweep for i in range(num_AR)]
        Lift_angle_theo = [[0]*num_Sweep for i in range(num_AR)]
        #Cl_alpha_vlm = [[0]*num_Sweep for i in range(num_AR)]
        #Cl_alpha_theo = [[0]*num_Sweep for i in range(num_AR)]
        Lift_angle_pm = [[0]*num_Sweep for i in range(num_AR)]
        #Cl_alpha_pm = [[0]*num_Sweep for i in range(num_AR)]
        
        #Avg_Cla_Error_VLM = [0.0] * num_Sweep
        Cla_Error_VLM = [0.0] * num_Sweep
        C_ratio = [0.0]*num_AR
        #Avg_Cla_Error_PM = [0.0] * num_Sweep
        
              
        for x in range(num_AR):
            
            C_ratio[x] = 1/(2*self.m_halfAR[x]) # AR to chord ratio
            
            for s in range(num_Sweep):
            
                # Open the file
                fname = scriptpath + '/swept_files/vsp_files/Swept_AR' + str(int(2*self.m_halfAR[x]*1000)) + '_Sweep' + str(int(self.m_Sweep[s]*1000)) + '.vsp3'
                fname_res_vlm = scriptpath + '/swept_files/vsp_files/Swept_AR' + str(int(2*self.m_halfAR[x]*1000)) + '_Sweep' + str(int(self.m_Sweep[s]*1000)) + '_vlm_res.csv'
                fname_res_pm = scriptpath + '/swept_files/vsp_files/Swept_AR' + str(int(2*self.m_halfAR[x]*1000)) + '_Sweep' + str(int(self.m_Sweep[s]*1000)) + '_pm_res.csv'

                print( 'Reading in file: ' )
                print( fname )
                vsp.ReadVSPFile( fname ) # Sets VSP3 file name
                
                #==== Analysis: VSPAeroSinglePoint ====#
                print( const.m_VSPSweepAnalysis )

                #==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====#
                print( const.m_CompGeomAnalysis )

                # Set defaults
                vsp.SetAnalysisInputDefaults( const.m_CompGeomAnalysis )
                
                vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'Symmetry', const.m_SymFlagVec, 0)

                # list inputs, type, and current values
                vsp.PrintAnalysisInputs( const.m_CompGeomAnalysis )
                
                # Execute
                print( '\tExecuting...' )
                compgeom_resid = vsp.ExecAnalysis( const.m_CompGeomAnalysis )
                print( 'COMPLETE' )

                # Get & Display Results
                vsp.PrintResults( compgeom_resid )

                #==== Analysis: VSPAero Single Point ====#
                # Set defaults
                vsp.SetAnalysisInputDefaults(const.m_VSPSweepAnalysis)
                print(const.m_VSPSweepAnalysis)

                # Reference geometry set
                vsp.SetIntAnalysisInput( const.m_VSPSweepAnalysis, 'GeomSet', const.m_GeomVec, 0 )
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'RefFlag', const.m_RefFlagVec, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Symmetry', const.m_SymFlagVec, 0)

                wid = vsp.FindGeomsWithName( 'WingGeom' )
                vsp.SetStringAnalysisInput(const.m_VSPSweepAnalysis, 'WingID', wid, 0)

                # Freestream Parameters
                Alpha = [1.0]
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', Alpha, 0)
                AlphaNpts = [1]
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'Machstart', const.m_MachVec, 0)
                MachNpts = [1]
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'WakeNumIter', const.m_WakeIterVec, 0)

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
                vsp.WriteResultsCSVFile( rid, fname_res_vlm )

                # Calculate Experimental and Theoretical Values
                C_bot_two = 1 + (pow((math.tan(math.radians(self.m_Sweep[s]))),2)/pow(const.b,2))
                C_top = 2*math.pi*2*self.m_halfAR[x]
                C_bot_one_theo = (pow((2*self.m_halfAR[x]),2)*pow(const.b,2))/pow(const.k_theo,2)
                C_bot_theo = (2 + (math.sqrt((C_bot_one_theo*C_bot_two)+4)))
                C_l_alpha_exper_theo = C_top/C_bot_theo
                self.Cl_alpha_theo_multi[x][s] = math.radians(C_l_alpha_exper_theo) # rad --> deg # wrong deg--> rad
                Lift_angle_theo[x][s] = 1/self.Cl_alpha_theo_multi[x][s] # Cl to lift angle
                
                # Get Result ID Vec (History and Load ResultIDs)
                rid_vec = vsp.GetStringResults( rid, 'ResultsVec' )
                if len(rid_vec) > 0 :
                
                    # Get History Results (rid_vec[0]) from Final Wake Iteration in History Result
                    cl_vec = vsp.GetDoubleResults( rid_vec[0], 'CL' )
                    Cl_res = cl_vec[-1]
                    self.Cl_alpha_vlm[x][s] = Cl_res # alpha = 1.0 (deg)
                    Lift_angle_vlm[x][s] = 1/(self.Cl_alpha_vlm[x][s]) # deg
                    
                    # Add error
                    self.Avg_Cla_Error_VLM[s] += abs((self.Cl_alpha_vlm[x][s] - self.Cl_alpha_theo_multi[x][s])/self.Cl_alpha_theo_multi[x][s])/num_AR
                
                
                #==== Analysis: VSPAero Panel Single ====#
                print( const.m_VSPSweepAnalysis )
                
                #==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====#
                print( const.m_CompGeomAnalysis )

                # Set defaults
                vsp.SetAnalysisInputDefaults( const.m_CompGeomAnalysis )
                
                panel_analysis = [vsp.PANEL]
                vsp.SetIntAnalysisInput( const.m_CompGeomAnalysis, 'AnalysisMethod', panel_analysis )
                
                vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'Symmetry', const.m_SymFlagVec, 0)

                # list inputs, type, and current values
                vsp.PrintAnalysisInputs( const.m_CompGeomAnalysis )

                # Execute
                print( '\tExecuting...' )
                compgeom_resid = vsp.ExecAnalysis( const.m_CompGeomAnalysis )
                print( 'COMPLETE' )

                # Get & Display Results
                vsp.PrintResults( compgeom_resid )
                
                #==== Analysis: VSPAero Panel Single ====#
               # Set defaults
                vsp.SetAnalysisInputDefaults(const.m_VSPSweepAnalysis)
                print(const.m_VSPSweepAnalysis)

                # Reference geometry set
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'GeomSet', const.m_GeomVec, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'RefFlag', const.m_RefFlagVec, 0)
                vsp.SetStringAnalysisInput(const.m_VSPSweepAnalysis, 'WingID', wid, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'WakeNumIter', const.m_WakeIterVec, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AnalysisMethod', panel_analysis)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Symmetry', const.m_SymFlagVec, 0)
                
                # Freestream Parameters
                alpha= [1]
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', alpha, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'Machstart', const.m_MachVec, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)

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
                vsp.WriteResultsCSVFile( rid, fname_res_pm )
                
                # Get Result ID Vec (History and Load ResultIDs)
                rid_vec = vsp.GetStringResults( rid, 'ResultsVec' )
                if len(rid_vec) > 0:
                    # Get Result from Final Wake Iteration
                    cl_vec = vsp.GetDoubleResults( rid_vec[0], 'CL' )
                    
                    Cl_pm = cl_vec[len(cl_vec) - 1]
                    self.Cl_alpha_pm[x][s] = Cl_pm # deg (alpha = 1.0°)
                    Lift_angle_pm[x][s] = 1/(self.Cl_alpha_pm[x][s]) # deg
                    
                    # Add error
                    self.Avg_Cla_Error_PM[s] += abs((self.Cl_alpha_pm[x][s] - self.Cl_alpha_theo_multi[x][s])/self.Cl_alpha_theo_multi[x][s])/num_AR
                
                vsp.ClearVSPModel()
#======== Use Matplotlib to Create tables and Graphs for the _________ Studies =#
    def GenerateSweptARSweepCharts(self):
        fullAR = list(map(lambda n: n*2,self.m_halfAR))
        
        # title = 'Sweep Study Geometry Setup'
        # header = ['Airfoil', 'AR', 'Root Chord', 'Tip Chord', 'Λ (°)', 'Λ Location', 'Span Tess (U)','Chord Tess (W)', 'LE Clustering', 'TE Clustering','Tip Clustering']
        # data = [['NACA0012'], [str(fullAR[0])+' to '+str(fullAR[-1])], ['1.0'],['1.0'],[str(self.m_Sweep[0])+' to '+str(self.m_Sweep[-1])],['0.5'],['41'],['51'],['0.2'],['1.0'],['1.0']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/swept_files/swept_img/ar_sweep/geometrysetup.png')
        
        # title = 'Sweep Study Geometry Setup'
        # header = ['Case','Analysis', 'Method', 'α (°)', 'β (°)', 'M', 'Wake Iterations']
        # data = [['1','2'],['Single Point','Single Point'],['VLM','Panel'], ['1.0','1.0'],['0.0','0.0'],['0.1','0.1'],['3','3']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/swept_files/swept_img/ar_sweep/vspaerosetup.png')
        
        avgclavlm = list(map(lambda n: n*100, self.Avg_Cla_Error_VLM))
        avgclapm = list(map(lambda n: n*100, self.Avg_Cla_Error_PM))
        
        for i in range(len(self.m_Sweep)):
            fig, ax = plt.subplots()
            ax.set_title(str(self.m_Sweep[i])+'° Sweep: Cl_alpha vs. Aspect Ratio')
            ax.set_xlabel('AR')
            ax.set_ylabel('Cl_alpha (°)')
            ax.plot(fullAR, const.transpose(self.Cl_alpha_vlm)[i],'o-', color=const.colors[0],label=r'VSPAERO VLM')
            ax.plot(fullAR, const.transpose(self.Cl_alpha_pm)[i],color=const.colors[1],label=r'VSPAERO Panel')
            ax.plot(fullAR, const.transpose(self.Cl_alpha_theo_multi)[i],color=const.colors[-1],label=r'LLT')
            ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
            fig.savefig(scriptpath + '/swept_files/swept_img/ar_sweep/ar_sweep_'+str(i)+'.svg', bbox_inches='tight')
        
        fig, ax = plt.subplots()
        ax.set_title(str(self.m_Sweep[i])+r'Average % Error in Cl_alpha Across All Aspect Ratios Sweep Sensitivity')
        ax.set_xlabel('Sweep (°)')
        ax.set_ylabel(r'Cl_alpha % Error')
        ax.plot(self.m_Sweep, avgclavlm,'o-',color=const.colors[0], label=r'VLM')
        ax.plot(self.m_Sweep, avgclapm,color=const.colors[1],label=r'Panel Method')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/swept_files/swept_img/ar_sweep/ar_sweep_avgs.svg', bbox_inches='tight')

def test_init():
    print('Testing SweptTest __init__()')
    swept = SweptTest()
    print(f'\tm_halfAR {swept.m_halfAR}')
    #print(f'\tm_AlphaNpts {swept.m_AlphaNpts}')
    #print(f'\tm_Tip_Clus {swept.m_Tip_Clus}')
    print(f'\tm_Tess_U {swept.m_Tess_U}')
    #print(f'\tm_WakeIter {swept.m_WakeIter}')
    #print(f'\tm_AdvancedWakeVec {swept.m_AdvancedWakeVec}')
    #print(f'\tm_AR10_Y_Cl_Cd_vec {swept.m_AR10_Y_Cl_Cd_vec}')
    print('\n')
    return swept

def generateCharts(swept: SweptTest):
    swept.GenerateSweptUWTessCharts()
    swept.GenerateSweptARSweepCharts()


# 0 is don't run, 1 is only graph from pickle, 2 is run with pickle if available, 3 is run without pickle
def runsweptstudy(uw = 3,ar = 3):
    setup_filepaths()    
    
    swept = test_init()
    if (uw == 1 or uw == 2):
        with open(scriptpath+'/swept_files/swepttestuw.pckl','rb') as picklefile:    
            swept = pickle.load(picklefile)
    if (uw == 1): 
        swept.GenerateSweptUWTessCharts()
    if (uw > 1):
        swept.SweptUWTessStudy()
        with open(scriptpath+'/swept_files/swepttestuw.pckl','wb') as picklefile:
            pickle.dump(swept,picklefile)
    
            
            
    swept = test_init()
    if (ar == 1 or ar == 2):
        with open(scriptpath+'/swept_files/swepttestar.pckl','rb') as picklefile:    
            swept = pickle.load(picklefile)
    if (ar == 1):
        swept.GenerateSweptARSweepCharts()
    if (ar > 1):
        swept.SweptARStudy()
        with open(scriptpath+'/swept_files/swepttestar.pckl','wb') as picklefile:
            pickle.dump(swept,picklefile)
    print('### NOTE: PANEL sometimes has issues and the graphs for it are all obviously 0. I have not solved this as sometimes it just works. Might be instability or expected filepaths?')      

def setup_filepaths():
    scriptpathlib = Path(__file__).parent.resolve()
    testnames = ['swept_files/']
    subnames = [['swept_img/','vsp_files/']]
    subsubnames = [[['chord_tesselation','span_tesselation', 'ar_sweep'],['']]]
    for i in range(len(testnames)):
        for j in range(len(subnames[i])):
            for k in range(len(subsubnames[i][j])):
                dirname = Path.joinpath(scriptpathlib, testnames[i]+subnames[i][j]+subsubnames[i][j][k])
                dirname.mkdir(parents=True, exist_ok=True)

def test_swept_generate(swept: SweptTest):
    print('Testing Wing Generation')
    try:
        swept.GenerateSweptUWTessWings()
        print('Completed GenerateSweptUWTessWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed GenerateSweptUWTessWings()')
        traceback.print_exc()
        return    
    print('Testing Wing Generation')
    try:
        swept.GenerateSweptARSweepWings()
        print('Completed GenerateSweptARSweepWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed GenerateSweptARSweepWings()')
        traceback.print_exc()
        return    
    
def test_swept_test(swept: SweptTest):
    print('Testing Test Functions')
    print('Testing TestSweptUWTessWings')
    try:
        swept.TestSweptUWTessWings()   
        print('\tCompleted TestSweptUWTessWings()') 
        print('-------------------------------------')
    except:
        print('\tERROR: Failed TestSweptUWTessWings()')
        traceback.print_exc()
        return    
    print('Testing SweptARSweepWings')
    try:
        swept.TestSweptARSweepWings()   
        print('\tCompleted TestSweptARSweepWings()') 
        print('-------------------------------------')
    except:
        print('\tERROR: Failed TestSweptARSweepWings()')
        traceback.print_exc()
        return    

if __name__ == '__main__':
    runsweptstudy(uw = 3,ar = 3)