import openvsp as vsp
import math
import Constants as const
import matplotlib.pyplot as plt
import traceback
from pathlib import Path
import pickle

scriptpath = str(Path(__file__).parent.resolve())
def vecofvec3dtolistoflists(temp):
    lis = []
    for i in temp:
        lis.append([i.x(),i.y(),i.z()])
    return lis

class HersheyTest:
    '''! Class for running and collecting data from 
         the Hershey studies
    '''
    def __init__(self):

        #ARWings Vec
        self.m_halfAR = [0]*6
        self.m_halfAR[0] = 2.5
        self.m_halfAR[1] = 5.0016
        self.m_halfAR[2] = 7.5
        self.m_halfAR[3] = 12.5
        self.m_halfAR[4] = 20.0
        self.m_halfAR[5] = 30.0

        self.m_AlphaNpts = 9
        
        #Tip Clustering Vec
        self.m_Tip_Clus = [0] * 3
        self.m_Tip_Clus[0] = 0.1
        self.m_Tip_Clus[1] = 0.5
        self.m_Tip_Clus[2] = 1

        # U Tesseleation Vec
        self.m_Tess_U =[0] * 4
        self.m_Tess_U[0] = 5
        self.m_Tess_U[1] = 12
        self.m_Tess_U[2] = 20
        self.m_Tess_U[3] = 41

        #W Tesselection Vec
        self.m_Tess_W = [0] * 4
        self.m_Tess_W[0] = 9
        self.m_Tess_W[1] = 17
        self.m_Tess_W[2] = 29
        self.m_Tess_W[3] = 51
        
        #Wake Iteration Vec
        self.m_WakeIter = [0]*5
        self.m_WakeIter[0] = 1
        self.m_WakeIter[1] = 2
        self.m_WakeIter[2] = 3
        self.m_WakeIter[3] = 4
        self.m_WakeIter[4] = 5
        
        #Advanced Wake Vec
        self.m_AdvancedWakeVec = [0] * 3
        self.m_AdvancedWakeVec[0] = 1
        self.m_AdvancedWakeVec[1] = 2
        self.m_AdvancedWakeVec[2] = 3

        #What Studies to run
        self.ar = True
        self.uw = True
        self.tc = True
        self.ut = True
        self.wt = True
        self.wi = True
        self.a_s = True

        #Consts
        self.Vinf = 100
        self.num_case = 4
        
        #Data for CLvA chart
        self.alpha_vlm = [[] for i in range(len(self.m_halfAR))]
        self.Cl_vlm = [[] for i in range(len(self.m_halfAR))]
        self.Cl_approx = [[] for i in range(len(self.m_halfAR))]

        #Data for CLavAR
        self.AR = [0.0]*(len(self.m_halfAR))
        self.Cl_alpha_vlm = [0.0]*(len(self.m_halfAR))
        self.Cl_alpha_pm = [0.0]*(len(self.m_halfAR))
        self.Cl_alpha_theo = [0.0]*(len(self.m_halfAR))

        #Data for HB_ClaErrorvAlpha
        self.Error_Cl_alpha_vlm = [0.0]*(self.m_AlphaNpts)

        #Data for Chord Tesse
        self.Error_Cla = [[0.0]*len(self.m_Tess_W) for i in range(len(self.m_Tess_U))] #array<array<double>>
        self.Exe_Time  = [[0.0]*len(self.m_Tess_W) for i in range(len(self.m_Tess_U))] #index 0: UTess, index 0: WTess #array<array<double>>

        #Data for Advance
        self.span_loc_data_adv = [ [[] for i in range(self.num_case)] for i in range(len(self.m_AdvancedWakeVec))] # array<array<double>>
        self.cl_dist_data_adv  = [ [[] for i in range(self.num_case)] for i in range(len(self.m_AdvancedWakeVec))] #array<array<double>>
        self.cl_dist_theo_adv = []

        #This assumes that Hershey_AR10_AVL.dat is in home/some_path/example/airfoil
        # and that this file is located in home/some_path/example/scripts/python_scripts
        self.AVL_file_name = scriptpath + '/../../airfoil/Hershey_AR10_AVL.dat'
        self.m_AR10_Y_Cl_Cd_vec = self.ReadAVLFile()

        #Data for Tip Clustering
        self.span_loc_data_tc = [[] for i in range(len(self.m_Tip_Clus))] #array<array<double>>
        self.cl_dist_data_tc  = [[] for i in range(len(self.m_Tip_Clus))] #array<array<double>>
        self.cd_dist_data_tc  = [[] for i in range(len(self.m_Tip_Clus))] #array<array<double>>
        self.cl_dist_theo = []

        #Data for Utess
        self.span_loc_data_utess = [[] for i in range(len(self.m_Tess_U))] #array<array<double>>
        self.cl_dist_data_utess  = [[] for i in range(len(self.m_Tess_U))] #array<array<double>>
        self.cd_dist_data_utess  = [[] for i in range(len(self.m_Tess_U))] #array<array<double>>
        self.cl_dist_theo_utess = []
        self.cd_dist_theo_utess = []

        #Data for Wtess
        self.span_loc_data_wtess = [[] for i in range(len(self.m_Tess_W))] #array<array<double>>
        self.cl_dist_data_wtess  = [[] for i in range(len(self.m_Tess_W))] #array<array<double>>
        self.cd_dist_data_wtess  = [[] for i in range(len(self.m_Tess_W))] #array<array<double>>
        self.cl_dist_theo_wtess = []
        self.cd_dist_theo_wtess = []
        
        self.wake_cl_dist_theo = []
        

        

#========== Helper Function for Loading Data from an AVL file ====================================#
    def ReadAVLFile(self):
        y_Cl_Cd_vec = []
        AVL_file = None
        try:
            AVL_file = open(self.AVL_file_name,'r')
        except:
            print('Error: Failed to Open ', self.AVL_file_name)
            return y_Cl_Cd_vec
        
        '''
            Read lines from dat file until the desired
            header title
        '''
        line = AVL_file.readline()
        while(not 'Strip Forces referred to Strip Area, Chord' in line):
            line = AVL_file.readline()
        
        '''
            Read two lines to get to the data segement
        '''
        AVL_file.readline()
        line = AVL_file.readline()

        ''' This text section ends in a row of '-' characters.
            Data is read from the table until the presence ofs
            one of these characters
        '''

        while (not '--' in line):
            line_array = line.split('   ')
            '''Create a shallow list object'''
            y_Cl_Cd = [0.0]*3

            '''Populate that list with data. Exit Gracefully on Error'''
            try:
                y_Cl_Cd[0] = float(line_array[2])
                y_Cl_Cd[1] = float(line_array[8])
                y_Cl_Cd[2] = float(line_array[9])
            except: 
                print('HersheyTest ERROR: Exception when parsing Hershey_AR10_AVL.dat')
                   
            '''Append to the returned list'''  
            y_Cl_Cd_vec.append(y_Cl_Cd)
                                
            '''Get the next line of data'''
            line = AVL_file.readline()
        

        AVL_file.close()
        return y_Cl_Cd_vec

#========== Run the Full Hershey Bar Study =======================================================#
    def hersheyBarStudy(self):
        if(self.ar):
            self.AspectRatioAndAngleOfAttackStudy()
        if(self.uw):
            self.TesselationStudy()
        if(self.tc):
            self.TipClusteringStudy()
        if(self.ut):
            self.SpanTesselationStudy()
        if(self.wt):
            self.ChordTesselationStudy()
        if(self.wi):
            self.WakeIterationStudy()
        if(self.a_s):
            self.AdvancedSettingsStudy()
        



#========================================= AR Wing Functions =====================================#
#==================== Generates the relavent parameteres. Runs the Aspect Ratio      =============#
#==================== and Angle of Attack studies. Generates the two tables and      =============#
#==================== three charts charts to include in the markdown file.           =============#
#=================================================================================================#

#========== Wrapper function for Aspect Ratio and Angle of Attack Code ===========================#
    def AspectRatioAndAngleOfAttackStudy(self):
        self.generateHersheyBarARWings()
        self.testHersheyBarARWings()
        self.generateARWingChart()

    #===================== Hershey Bar Wing Generation Functions =====================
    def generateHersheyBarARWings(self):
        #==== Add Wing Geometry ====
        wing_id = vsp.AddGeom( 'WING', '' ) 

        #==== Set Wing Section ====
        vsp.SetDriverGroup(wing_id, 1 ,vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER)

        vsp.Update()

        #==== Set NACA 0012 Airfoil and Common Parms
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )

        u = 3
        w = 3
        t = 2

        vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] ) # Constant U Tess
        vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] ) # Constant W Tess
        vsp.SetParmVal( wing_id, 'OutCluster', 'XSec_1', self.m_Tip_Clus[t] ) # Constant Tip Clustering

        vsp.Update()

        for x in range(len(self.m_halfAR)):
            vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', self.m_halfAR[x] )

            vsp.Update()

            #==== Setup export filenames for AR Study ====
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_AR' + str(2*self.m_halfAR[x]) + '.vsp3'

            #==== Save Vehicle to File ====
            message = '-->Saving vehicle file to: ' + fname + '\n'
            print( message )
            vsp.WriteVSPFile( fname, vsp.SET_ALL )
            print( 'COMPLETE\n' )

        vsp.ClearVSPModel()

#========== Run the actual Aspect Ratio and Angle of Attack Studies ==============================#
    def testHersheyBarARWings(self):
        print('-> Begin HersheyBar AR Study:\n')

        num_AR = len(self.m_halfAR)

        C_bot_two = 1 + (pow(math.tan(0.0),2)/pow(const.b,2))
        
        alpha_0 = -20.0
        alpha_f = 20.0
        d_alpha = alpha_f - alpha_0
        
        alpha_step = d_alpha/(self.m_AlphaNpts - 1)
        alpha_mid_index = int((self.m_AlphaNpts - 1)/2.0)
        
        
        Lift_angle_vlm  = [0.0]*(num_AR)
        Lift_angle_theo = [0.0]*(num_AR)
        
        C_ratio = [0.0]*(num_AR)
        
        Lift_angle_pm = [0.0]*(num_AR)
        
        
        
        for x in range(len(self.m_halfAR)):
        
            #==== Open and test generated wings ====#
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_AR' + str(2*self.m_halfAR[x]) + '.vsp3'
            fname_res_vlm =scriptpath + '/hershey_files/vsp_files/Hershey_AR' + str(2*self.m_halfAR[x]) + '_vlm_res.csv'
            fname_res_pm =scriptpath + '/hershey_files/vsp_files/Hershey_AR' + str(2*self.m_halfAR[x]) + '_pm_res.csv'

            print('Reading in file: ', False )
            print( fname )
            vsp.ReadVSPFile( fname ) # Sets VSP3 file name
            
            #==== Analysis: VSPAero VLM Sweep ====#
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

            #==== Analysis: VSPAero VLM Sweep ====#
            # Set defaults
            vsp.SetAnalysisInputDefaults(const.m_VSPSweepAnalysis)

            # Reference geometry set
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'GeomSet', [vsp.SET_NONE], 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'ThinGeomSet', [vsp.SET_ALL], 0)  # Thin geometry - VLM
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'RefFlag', const.m_RefFlagVec, 0)

            wid = vsp.FindGeomsWithName( 'WingGeom' )
            vsp.SetStringAnalysisInput(const.m_VSPSweepAnalysis, 'WingID', wid, 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Symmetry', const.m_SymFlagVec, 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'WakeNumIter', const.m_WakeIterVec, 0)

            # Freestream Parameters
            AlphaStart = [alpha_0] #array<double> AlphaStart
            AlphaEnd = [alpha_f] #array<double> AlhpaEnd
            AlphaNpts = [self.m_AlphaNpts] #array<int> AlphaNpts
            
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', AlphaStart, 0)
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaEnd', AlphaEnd, 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
            
            MachNpts = [1] # Start and end at 0.1 #array<double>
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
            vsp.WriteResultsCSVFile( rid, fname_res_vlm )
            
            # Get Result ID Vec (History and Load ResultIDs)
            rid_vec = vsp.GetStringResults( rid, 'ResultsVec' )
            
            # Calculate Experimental and Theoretical Values
            # Fluid -Dynamic Lift pg 3-2
            # Method 1 of USAF DATCOM Section 1, page 1-7, and also NACA TN-3911)
            self.AR[x] = 2*self.m_halfAR[x]
            C_top = 2*math.pi*self.AR[x]
            C_bot_one_theo = (pow(self.AR[x],2)*pow(const.b,2))/pow(const.k_theo,2)
            C_bot_theo = (2 + (math.sqrt((C_bot_one_theo*C_bot_two)+4)))
            self.Cl_alpha_theo[x] = (C_top/C_bot_theo)*(math.pi/180) # deg
            Lift_angle_theo[x] = 1/(self.Cl_alpha_theo[x]) # Cl to lift angle (deg)
            C_ratio[x] = 1/self.AR[x] # AR to chord ratio
            
            if ( len(rid_vec) >= 1 ):
                alpha_res = [0.0]*( self.m_AlphaNpts )
                Cl_res = [0.0]*( self.m_AlphaNpts )
                Cl_approx_vec = [0.0]*( self.m_AlphaNpts )
                
                # Get Result from Final Wake Iteration
                for i in range(self.m_AlphaNpts):
                
                    alpha_vec = vsp.GetDoubleResults( rid_vec[i], 'Alpha' )
                    alpha_res[i] = alpha_vec[int(len(alpha_vec)) - 1]
                    
                    cl_vec = vsp.GetDoubleResults( rid_vec[i], 'CL' )
                    Cl_res[i] = cl_vec[int(len(cl_vec)) - 1]
                    
                    Cl_approx_vec[i] = 2 * math.pi * math.sin( math.radians( alpha_res[i] ) )
                
                if ( x == 1 ):
                
                    for i in range(self.m_AlphaNpts):
                        Cl_alpha_res = 0

                        if ( i == 0 ):
                            Cl_alpha_res = ((Cl_res[i+1] - Cl_res[i])/(alpha_res[i+1] - alpha_res[i]))
                        
                        elif ( i == self.m_AlphaNpts - 1 ):
                            Cl_alpha_res = ((Cl_res[i] - Cl_res[i-1])/(alpha_res[i] - alpha_res[i-1]))
                        
                        else: # Central differencing
                            Cl_alpha_res = ((Cl_res[i+1] - Cl_res[i-1])/(alpha_res[i+1] - alpha_res[i-1]))
                        
                        
                        self.Error_Cl_alpha_vlm[i] = (abs((Cl_alpha_res - self.Cl_alpha_theo[x])/self.Cl_alpha_theo[x]))*100
                    
                
                
                # Evaluate Cl_alpha near alpha = 0 to avoid errors due to unmodeled stall characteristics
                self.Cl_alpha_vlm[x] = ((Cl_res[alpha_mid_index + 1] - Cl_res[alpha_mid_index])/alpha_step) 
                
                self.alpha_vlm[x] = alpha_res
                self.Cl_vlm[x] = Cl_res
                self.Cl_approx[x] = Cl_approx_vec
                
                Lift_angle_vlm[x] = 1/(self.Cl_alpha_vlm[x]) # deg
            
            
            #==== Analysis: VSPAero Panel Single ====#
            print( const.m_VSPSweepAnalysis )
            
            #==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====#
            print( const.m_CompGeomAnalysis )

            # Set defaults
            vsp.SetAnalysisInputDefaults( const.m_CompGeomAnalysis )
            vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'GeomSet', [vsp.SET_ALL], 0)  # Thick geometry - Panel
            vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'ThinGeomSet', [vsp.SET_NONE], 0)
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
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'GeomSet', [vsp.SET_ALL], 0)  # Thick geometry - Panel
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'ThinGeomSet', [vsp.SET_NONE], 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'RefFlag', const.m_RefFlagVec, 0)
            vsp.SetStringAnalysisInput(const.m_VSPSweepAnalysis, 'WingID', wid, 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'WakeNumIter', const.m_WakeIterVec, 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Symmetry', const.m_SymFlagVec, 0)
            
            # Freestream Parameters
            Alpha = [1.0]
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', Alpha, 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', const.m_MachVec, 0)
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
            vsp.WriteResultsCSVFile( rid, fname_res_pm )
            
            # Get Result ID Vec (History and Load ResultIDs)
            rid_vec = vsp.GetStringResults( rid, 'ResultsVec' )
            
            if ( len(rid_vec) > 0 ):
            
                # Get Result from Final Wake Iteration
                cl_vec = vsp.GetDoubleResults( rid_vec[0], 'CL' )
                
                Cl_pm = cl_vec[int(len(cl_vec)) - 1]
                self.Cl_alpha_pm[x] = Cl_pm # deg (alpha = 1.0°)
                Lift_angle_pm[x] = 1/(self.Cl_alpha_pm[x]) # deg
            

            vsp.ClearVSPModel()

#======== Use Matplotlib to Create tables and Graphs for the Aspect Ratio and Angle of Attack Studies =#
    def generateARWingChart(self):
        # #Aspect Ratio Setup Table
        # header = const.STUDY_SETUP_TABLE_HEADER.copy()
        # data = [[1,2],['Sweep','Single Point'],['VLM','Panel'],['-20.0 to 20.0, npts: 8','1.0'],[0.0,0.0],[const.m_MachVec[0]]*2,[const.m_WakeIterVec[0]]*2]
        # table = make_table(header,data)
        # print(len(header),' ',len(data),' ',header,' ',data)
        # export_png(table,filename=scriptpath + '/hershey_files/hershey_img/aspect_ratio/vspasero_setup.png')

        # #Angle of Attack Setup Table
        # data = [[1],['Sweep'],['VLM'],['-20.0 to 20.0, npts: '+str(self.m_AlphaNpts)],[0.0],[const.m_MachVec[0]],[const.m_WakeIterVec[0]]]
        # table = make_table(header,data)
        # export_png(table,filename=scriptpath + '/hershey_files/hershey_img/angle_of_attack/vspasero_setup.png')

        # ClvA figure
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar VLM Cl vs Alpha for Various AR')
        ax.set_xlabel('Alpha (°)')
        ax.set_ylabel('Cl')
        for i in range(len(self.Cl_vlm)):
            ax.plot(self.alpha_vlm[0],self.Cl_vlm[i], 'o-',label='AR: '+str(self.m_halfAR[i]*2),color=const.colors[i])
        ax.plot(self.alpha_vlm[0],self.Cl_approx[0], label='2*pi',color=const.colors[-1])
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/aspect_ratio/ClvA.svg', bbox_inches='tight')
        
       
        #ClvAR Data Generation
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar Cl_alpha vs AR')
        ax.set_xlabel('AR')
        ax.set_ylabel('Cl_alpha (°)')
        ax.plot(self.AR,self.Cl_alpha_vlm,'o-', color=const.colors[0], label='VSPAERO VLM')
        ax.plot(self.AR,self.Cl_alpha_pm,'o-',color=const.colors[1],label='VSPAERO Panel')
        ax.plot(self.AR,self.Cl_alpha_theo,color=const.colors[-1],label='LLT')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/aspect_ratio/ClvAR.svg', bbox_inches='tight')
        
                
        #HB_ClaErrorvAlpha
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar VLM Cl_alpha Alpha Sensitivity: AR = 10')
        ax.set_xlabel('Alpha (°)')
        ax.set_ylabel(r'Cl_alpha % Error')
        ax.plot(self.alpha_vlm[1],self.Error_Cl_alpha_vlm,'o-',color=const.colors[0],label=r'% Error')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/angle_of_attack/HB_ClaErrorvAlpha.svg', bbox_inches='tight')
        
        



#========================================= UW Tess Functions =====================================#
#==================== Generates the relavent parameteres. Runs the Tesselation Study =============#
#==================== Generates the two tables and four charts to include in the     =============#
#==================== markdown file.                                                 =============#
#=================================================================================================#

#========== Wrapper function for Tesselation Code ================================================#
    def TesselationStudy(self):
        self.generateHersheyBarUWTessWings()
        self.testHersheyBarUWTessWings()
        self.generateUWTessChart()

#========== Setup for  Tesselation Study =========================================================#
    def generateHersheyBarUWTessWings(self):
        #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )
        
        #==== Set Wing Section ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()
        
        #==== Set NACA 0012 Airfoil and Common Parms 
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        x = 1 # AR
        t = 2 # Tip Clustering
        
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', self.m_halfAR[x] ) # Constant AR
        vsp.SetParmVal( wing_id, 'OutCluster', 'XSec_1', self.m_Tip_Clus[t] ) # Constant Tip Clustering
        
        vsp.Update()
        
        for u in range(len(self.m_Tess_U)):
        
            for w in range(len(self.m_Tess_W)):
            
                vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] )
                vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] )

                vsp.Update()

                #==== Setup export filenames for UW Tess Study ====#
                fname =scriptpath + '/hershey_files/vsp_files/Hershey_U' + str(self.m_Tess_U[u]) + '_W' + str(self.m_Tess_W[w]) + '.vsp3'

                #==== Save Vehicle to File ====#
                message = '-->Saving vehicle file to: ' + fname + '\n'
                print( message )
                vsp.WriteVSPFile( fname, vsp.SET_ALL )
                print( 'COMPLETE\n' )
            
        
        
        vsp.ClearVSPModel()

#========== Run the actual Tesselation Study =====================================================#
    def testHersheyBarUWTessWings(self):
        print( '-> Begin Hershey Bar U and W Tesselation Study:\n')
        
        x = 1 # AR
        
        numUTess = len(self.m_Tess_U)
        numWTess = len(self.m_Tess_W)
        
        
        # Calculate Experimental and Theoretical Values
        # Fluid -Dynamic Lift pg 3-2
        # Method 1 of USAF DATCOM Section 1, page 1-7, and also NACA TN-3911)
        C_bot_two = 1 + (pow(math.tan(0.0),2)/pow(const.b,2))
        AR = 2*self.m_halfAR[x]
        C_top = 2*math.pi*AR
        C_bot_one_theo = (pow(AR,2)*pow(const.b,2))/pow(const.k_theo,2)
        C_bot_theo = (2 + (math.sqrt((C_bot_one_theo*C_bot_two)+4)))
        Cl_alpha_theo = (C_top/C_bot_theo)*(math.pi/180) # deg
        Lift_angle_theo = 1/(Cl_alpha_theo) # Cl to lift angle (deg)
        
        for u in range(numUTess):
            
            for w in range(numWTess):
            
                fname =scriptpath + '/hershey_files/vsp_files/Hershey_U' + str(self.m_Tess_U[u]) + '_W' + str(self.m_Tess_W[w]) + '.vsp3'
                fname_res =scriptpath + '/hershey_files/vsp_files/Hershey_U' + str(self.m_Tess_U[u]) + '_W' + str(self.m_Tess_W[w])+ '_res.csv'
                
                #==== Open and test generated wings ====#
                print('Reading in file: ', False )
                print( fname )
                vsp.ReadVSPFile( fname ) # Sets VSP3 file name

                #==== Analysis: VSPAEROSinglePoint ====#
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

                #==== Analysis: VSPAero Single Point ====#
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

                # Freestream Parameters
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', const.m_AlphaVec, 0)
                AlphaNpts = [1]
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', const.m_MachVec, 0)
                MachNpts = [1]
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)
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
                if ( len(rid_vec) > 0 ):
                
                    # Get History Results (rid_vec[0]) from Final Wake Iteration in History Result
                    cl_vec = vsp.GetDoubleResults( rid_vec[0], 'CL' )
                    Cl_alpha_vsp = cl_vec[int(len(cl_vec)) - 1] # alpha = 1.0 deg
                    
                    self.Error_Cla[u][w] = (abs((Cl_alpha_vsp - Cl_alpha_theo)/Cl_alpha_theo))*100
                
                
                time_vec = vsp.GetDoubleResults( rid, 'Analysis_Duration_Sec' )
                
                if ( len(time_vec) > 0 ):
                    self.Exe_Time[u][w] = time_vec[0]
                
                
                vsp.ClearVSPModel()

#======== Use Matplotlib to Create tables and Graphs for the Tesselation Study ========================#
    def generateUWTessChart(self):
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar VLM Cl_alpha Span Tesselation (U Tess) Sensitivity')
        ax.set_xlabel('Chord Tesselation (W Tess)')
        ax.set_ylabel(r'Cl_alpha % Error')
        for i in range(len(self.Error_Cla)):
            ax.plot(self.m_Tess_W,self.Error_Cla[i], 'o-', color=const.colors[i],label='U Tess: '+str(self.m_Tess_U[i]))
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/tesselation/Error_Cla_U.svg', bbox_inches='tight')
        
        W_list = [[self.Error_Cla[u][i] for u in range(len(self.Error_Cla))] for i in range(len(self.Error_Cla[0]))]
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar VLM Cl_alpha Chord Tesselation (W Tess) Sensitivity')
        ax.set_xlabel('Chord Tesselation (U Tess)')
        ax.set_ylabel(r'Cl_alpha % Error')
        for i in range(len(W_list)):
            ax.plot(self.m_Tess_U,W_list[i], 'o-', color=const.colors[i], label='W Tess: ' + str(self.m_Tess_W[i]))
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/tesselation/Error_Cla_W.svg', bbox_inches='tight')
        

        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar VLM Execution Time Span Tesselation (U Tess) Sensitivity')
        ax.set_xlabel('Chord Tesselation (W Tess)')
        ax.set_ylabel('Time (sec)')
        for i in range(len(self.Exe_Time)):
            ax.plot(self.m_Tess_W,self.Exe_Time[i], 'o-', color=const.colors[i], label='U Tess: '+str(self.m_Tess_U[i]))
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/tesselation/Exec_Time_U.svg', bbox_inches='tight')
        

        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar VLM Execution Time Chord Tesselation (W Tess) Sensitivity')
        ax.set_xlabel('Span Tesselation (U Tess)')
        ax.set_ylabel('Time (sec)')
        W_list2 = [[self.Exe_Time[u][i] for u in range(len(self.Exe_Time))] for i in range(len(self.Exe_Time[0]))]
        for i in range(len(W_list2)):
            ax.plot(self.m_Tess_U,W_list2[i], 'o-', color=const.colors[i],label='W Tess: '+str(self.m_Tess_W[i]))
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/tesselation/Exec_Time_W.svg', bbox_inches='tight')
        
        

        
        # #Tesselation Setup Table
        # header = ['Analysis','Method','alpha (°)','beta (°)','M','Wake Iterations']
        # data = [['Single Point'],['VLM'],['1.0'],['0.0'],[const.m_MachVec[0]],[const.m_WakeIterVec[0]]] 
        # data_table  = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/hershey_files/hershey_img/tesselation/vspasero_setup.png')



#========================================= TC Wing Functions =====================================#
#==================== Generates the relavent parameteres. Runs the Tip Clustering    =============#
#==================== study. Generates the two tables and four charts to include     =============#
#==================== in the markdown file.                                          =============#
#=================================================================================================#

#========== Wrapper function for Tip Clustering Code ==========================================#
    def TipClusteringStudy(self):
        self.generateHersheyBarTCWings()
        self.testHersheyBarTCWings()
        self.generateTCWingChart()

#========== Setup for Tip Clustering Study ====================================================#
    def generateHersheyBarTCWings(self):
        #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )
        
        #==== Set Wing Section ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()
        
        #==== Set NACA 0012 Airfoil and Common Parms 
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        x = 1 # AR
        u = 1 # UTess
        w = 1 # WTess
        
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', self.m_halfAR[x] ) # Constant AR
        vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] ) # Constant U Tess
        vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] ) # Constant W Tess
        
        vsp.Update()
        
        for t in range(len(self.m_Tip_Clus)):
        
            vsp.SetParmVal( wing_id, 'OutCluster', 'XSec_1', self.m_Tip_Clus[t] )

            vsp.Update()

            #==== Setup export filenames for Tip Clustering Study ====#
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_TC' + str(self.m_Tip_Clus[t]) + '.vsp3'

            #==== Save Vehicle to File ====#
            message = '-->Saving vehicle file to: ' + fname + '\n'
            print( message )
            vsp.WriteVSPFile( fname, vsp.SET_ALL )
            print( 'COMPLETE\n' )
        
        
        vsp.ClearVSPModel()

#========== Run the actual Tip Clustering Study ===============================================#
    def testHersheyBarTCWings(self):
        print( '-> Begin Hershey Bar Tip Clustering Study:\n' )
        
        num_TC = len(self.m_Tip_Clus)
        x = 1 # AR
        


        for  t in range(num_TC):
        
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_TC' + str(self.m_Tip_Clus[t]) + '.vsp3'
            fname_res =scriptpath + '/hershey_files/vsp_files/Hershey_TC' + str(self.m_Tip_Clus[t]) + '_res.csv'
            
            
            #==== Open and test generated wings ====#
            print('Reading in file: ', False )
            print( fname )
            vsp.ReadVSPFile( fname ) # Sets VSP3 file name

            #==== Analysis: VSPAEROSinglePoint ====#
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

            #==== Analysis: VSPAero Single Point ====#
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

            # Freestream Parameters
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', const.m_AlphaVec, 0)
            AlphaNpts = [1]
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', const.m_MachVec, 0)
            MachNpts = [1]
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)
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

            # Get Load Result ID
            load_rid = vsp.FindLatestResultsID( 'VSPAERO_Load' )
            if ( load_rid != '' ):
            
                # Lift Distribution:
                self.span_loc_data_tc[t] = vsp.GetDoubleResults( load_rid, 'Yavg' )
                self.cl_dist_data_tc[t] = vsp.GetDoubleResults( load_rid, 'cl' )
                self.cd_dist_data_tc[t] = vsp.GetDoubleResults( load_rid, 'cd' )
            
            
            vsp.ClearVSPModel()
        temp = vsp.GetHersheyBarLiftDist( int(100), math.radians(const.m_AlphaVec[0]), self.Vinf, (2*self.m_halfAR[x]), False )
        self.cl_dist_theo = vecofvec3dtolistoflists(temp)
#======== Use Matplotlib to Create tables and Graphs for the Tip Clustering Study ==================#
    def generateTCWingChart(self):
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar Lift Distribution Tip Clustering Sensitivity')
        ax.set_xlabel('Span Location (Y)')
        ax.set_ylabel('Cl')
        for i in range(len(self.span_loc_data_tc)):
            ax.plot(self.span_loc_data_tc[i],self.cl_dist_data_tc[i],'o-', color=const.colors[i], label='TC:'+str(self.m_Tip_Clus[i]))
        theo_x = [ vec[0] for vec in self.cl_dist_theo ]
        theo_y = [ vec[1] for vec in self.cl_dist_theo ]
        ax.plot(theo_x,theo_y, color = const.colors[-1], label = 'LLT')
        transposed_list_2 = [[self.m_AR10_Y_Cl_Cd_vec[i][j] for i in range(len(self.m_AR10_Y_Cl_Cd_vec))] for j in range(len(self.m_AR10_Y_Cl_Cd_vec[0]))]
        ax.plot(transposed_list_2[0],transposed_list_2[1],'o-' ,color=const.colors[4], label='AVL')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/tip_clustering/tc_graph.svg', bbox_inches='tight')
        
        

        # #Tip Clustering VSPAERO Setup Table
        # header = ['Analysis','Method','alpha (°)','beta (°)','M','Wake Iterations']
        # data = [['Single Point'],['VLM'],['1.0'],['0.0'],[const.m_MachVec[0]],[const.m_WakeIterVec[0]]] 
        # data_table  = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/hershey_files/hershey_img/tip_clustering/vspasero_setup.png')

        # #Tip Clustering AVL Setup
        # header = ['Nchord','Cspace','Nspan','Sspan','M']
        # data = [['30'],['1.0'],['20'],['-3.0'],[const.m_MachVec[0]]] 
        # data_table  = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/hershey_files/hershey_img/tip_clustering/avl_setup.png')


#========================================= UTess Functions =======================================#
#==================== Generates the relavent parameteres. Runs the Span Tesselation  =============#
#==================== study. Generates the two tables and two charts to include      =============#
#==================== in the markdown file.                                          =============#
#=================================================================================================#

#========== Wrapper function for Span Tesselation Code ===========================================#
    def SpanTesselationStudy(self):
        self.generateHersheyBarUTessWings()
        self.testHersheyBarUTessWings()
        self.generateHersheyBarUTessChart()

#========== Setup for Span Tesselation Study =====================================================#
    def generateHersheyBarUTessWings(self):
        #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )
        
        #==== Set Wing Section ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()
        
        #==== Set NACA 0012 Airfoil and Common Parms 
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        x = 1 # AR
        w = 1 # WTess
        t = 2 # Tip Clustering
        
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', self.m_halfAR[x] ) # Constant AR
        vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] ) # Constant W Tess
        vsp.SetParmVal( wing_id, 'OutCluster', 'XSec_1', self.m_Tip_Clus[t] ) # Constant Tip Clustering
        
        vsp.Update()
        
        for u in range(len(self.m_Tess_U)):
            vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] )

            vsp.Update()

            #==== Setup export filenames for U Tess Study ====#
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_U' + str(self.m_Tess_U[u]) + '.vsp3'

            #==== Save Vehicle to File ====#
            message = '-->Saving vehicle file to: ' + fname + '\n'
            print( message )
            vsp.WriteVSPFile( fname, vsp.SET_ALL )
            print( 'COMPLETE\n' )
        
        
        vsp.ClearVSPModel()

#========== Run the actual Span Tesseleation Study ===============================================#
    def testHersheyBarUTessWings(self):
        print('-> Begin Hershey Bar U Tesselation Study:\n')
        
        x = 1 # AR
        numUTess = len(self.m_Tess_U)
        
        for u in range(numUTess):
        
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_U' + str(self.m_Tess_U[u]) + '.vsp3'
            fname_res =scriptpath + '/hershey_files/vsp_files/Hershey_U' + str(self.m_Tess_U[u]) + '_res.csv'
            
            #==== Open and test generated wings ====#
            print('Reading in file: ', False)
            print( fname )
            vsp.ReadVSPFile( fname ) # Sets VSP3 file name

            #==== Analysis: VSPAEROSinglePoint ====#
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

            #==== Analysis: VSPAero Single Point ====#
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

            # Freestream Parameters
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', const.m_AlphaVec, 0)
            AlphaNpts = [1]
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', const.m_MachVec, 0)
            MachNpts = [1]
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)
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

            # Get Load Result ID
            load_rid = vsp.FindLatestResultsID( 'VSPAERO_Load' )
            if ( load_rid != '' ):
            
                # Lift Distribution:
                self.span_loc_data_utess[u] = vsp.GetDoubleResults( load_rid, 'Yavg' )
                self.cl_dist_data_utess[u] = vsp.GetDoubleResults( load_rid, 'cl' )
                self.cd_dist_data_utess[u] = vsp.GetDoubleResults( load_rid, 'cd' )
            
            
            vsp.ClearVSPModel()
        temp = vsp.GetHersheyBarLiftDist( int(100), math.radians(const.m_AlphaVec[0]), self.Vinf, (2*self.m_halfAR[x]), False )
        self.cl_dist_theo_utess = vecofvec3dtolistoflists(temp)
        temp = vsp.GetHersheyBarDragDist( int(100), math.radians(const.m_AlphaVec[0]), self.Vinf, (2*self.m_halfAR[x]), False )
        self.cd_dist_theo_utess = vecofvec3dtolistoflists(temp)

#======== Use Matplotlib to Create tables and Graphs for the Span Tesselation Study ===================#
    def generateHersheyBarUTessChart(self):
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar Lift Distribution Span Tesselation (U Tess) Sensitivity')
        ax.set_xlabel('Span Location (Y)')
        ax.set_ylabel('Cl')
        for i in range(len(self.span_loc_data_utess)):
            ax.plot(self.span_loc_data_utess[i],self.cl_dist_data_utess[i],'o-', color=const.colors[i], label='U Tess: '+str(self.m_Tess_U[i]))
        theo_x = [ vec[0] for vec in self.cl_dist_theo_utess ]
        theo_y = [ vec[1] for vec in self.cl_dist_theo_utess ]
        ax.plot(theo_x,theo_y, color=const.colors[-1], label='LLT')
        transposed_list_2 = [[self.m_AR10_Y_Cl_Cd_vec[i][j] for i in range(len(self.m_AR10_Y_Cl_Cd_vec))] for j in range(len(self.m_AR10_Y_Cl_Cd_vec[0]))]
        ax.plot(transposed_list_2[0],transposed_list_2[1],'o-', color=const.colors[4], label='AVL')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/span_tesselation/lift_dist.svg', bbox_inches='tight')
        
        

        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar Drag Distribution Span Tesselation (U Tess) Sensitivity')
        ax.set_xlabel('Span Location (Y)')
        ax.set_ylabel('Cd')
        for i in range(len(self.span_loc_data_utess)):
            ax.plot(self.span_loc_data_utess[i],self.cd_dist_data_utess[i],'o-', color=const.colors[i], label='U Tess: '+str(self.m_Tess_U[i]))
        theo_x_cd = [ vec[0] for vec in self.cd_dist_theo_utess ]
        theo_z_cd = [ vec[1] for vec in self.cd_dist_theo_utess ]
        ax.plot(theo_x_cd,theo_z_cd, color=const.colors[-1], label='LLT')
        ax.plot(transposed_list_2[0],transposed_list_2[2],'o-', color=const.colors[4], label='AVL')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/span_tesselation/drag_dist.svg', bbox_inches='tight')
        
        

        # #Span Tesselation VSPAERO Setup Table
        # header = ['Analysis','Method','alpha (°)','beta (°)','M','Wake Iterations']
        # data = [['Single Point'],['VLM'],['1.0'],['0.0'],[const.m_MachVec[0]],[const.m_WakeIterVec[0]]] 
        # data_table  = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/hershey_files/hershey_img/span_tesselation/vspasero_setup.png')

        # #Span Tesselations AVL Setup
        # header = ['Nchord','Cspace','Nspan','Sspan','M']
        # data = [['30'],['1.0'],['20'],['-3.0'],[const.m_MachVec[0]]] 
        # data_table  = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/hershey_files/hershey_img/span_tesselation/avl_setup.png')



#========================================= WTess Functions =======================================#
#==================== Generates the relavent parameteres. Runs the Chord Tesselation =============#
#==================== study. Generates the two tables and two charts to include      =============#
#==================== in the markdown file.                                          =============#
#=================================================================================================#

#========== Wrapper function for Chord Tesselation Code ==========================================#    
    def ChordTesselationStudy(self):
        self.generateHersheyBarWTessWings()
        self.testHersheyBarWTessWings()
        self.generateWTessChart()

#========== Setup for Chord Tesselation Study ====================================================#
    def generateHersheyBarWTessWings(self):
        #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )
        
        #==== Set Wing Section ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()
        
        #==== Set NACA 0012 Airfoil and Common Parms 
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        x = 1 # AR
        u = 1 # UTess
        t = 2 # Tip Clustering
        
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', self.m_halfAR[x] ) # Constant AR
        vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] ) # Constant U Tess
        vsp.SetParmVal( wing_id, 'OutCluster', 'XSec_1', self.m_Tip_Clus[t] ) # Constant Tip Clustering
        
        vsp.Update()
        
        for w in range(len(self.m_Tess_W)):
        
            vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] )

            vsp.Update()

            #==== Setup export filenames for W Tess Study ====#
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_W' + str(self.m_Tess_W[w]) + '.vsp3'

            #==== Save Vehicle to File ====#
            message = '-->Saving vehicle file to: ' + fname + '\n'
            print( message )
            vsp.WriteVSPFile( fname, vsp.SET_ALL )
            print( 'COMPLETE\n' )
        
        
        vsp.ClearVSPModel()

#========== Run the actual Chord Tesseleation Study ===============================================#
    def testHersheyBarWTessWings(self):
        print('-> Begin Hershey Bar W Tesselation Study:\n')
        
        x = 1 # AR
        numWTess = len(self.m_Tess_W)
        
        
        for w in range(numWTess):
        
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_W' + str(self.m_Tess_W[w]) + '.vsp3'
            fname_res =scriptpath + '/hershey_files/vsp_files/Hershey_W' + str(self.m_Tess_W[w]) + '_res.csv'
            
            #==== Open and test generated wings ====#
            print('Reading in file: ', False )
            print( fname )
            vsp.ReadVSPFile( fname ) # Sets VSP3 file name

            #==== Analysis: VSPAEROSinglePoint ====#
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

            #==== Analysis: VSPAero Single Point ====#
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

            # Freestream Parameters
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', const.m_AlphaVec, 0)
            AlphaNpts = [1]
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', const.m_MachVec, 0)
            MachNpts = [1]
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)
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

            # Get Load Result ID
            load_rid = vsp.FindLatestResultsID( 'VSPAERO_Load' )
            if ( load_rid != '' ):
            
                # Lift Distribution:
                self.span_loc_data_wtess[w] = vsp.GetDoubleResults( load_rid, 'Yavg' )
                self.cl_dist_data_wtess[w] = vsp.GetDoubleResults( load_rid, 'cl' )
                self.cd_dist_data_wtess[w] = vsp.GetDoubleResults( load_rid, 'cd' )
            
            
            vsp.ClearVSPModel()
        print('SOMETHING BIG', self.cl_dist_theo_wtess)

        temp = vsp.GetHersheyBarLiftDist( int(100), math.radians(const.m_AlphaVec[0]), self.Vinf, (2*self.m_halfAR[x]), False )
        self.cl_dist_theo_wtess = vecofvec3dtolistoflists(temp)
        print('SOMETHING BIG', self.cl_dist_theo_wtess)
        temp = vsp.GetHersheyBarDragDist( int(100), math.radians(const.m_AlphaVec[0]), self.Vinf, (2*self.m_halfAR[x]), False )
        self.cd_dist_theo_wtess = vecofvec3dtolistoflists(temp)
        print('SOMETHING BIG', self.cl_dist_theo_wtess)

#======== Use Matplotlib to Create tables and Graphs for the Span Tesselation Study ===================#
    def generateWTessChart(self):
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar Lift Distribution Chord Tesselation (W Tess) Sensitivity')
        ax.set_xlabel('Span Location (Y)')
        ax.set_ylabel('Cl')
        for i in range(len(self.span_loc_data_wtess)):
            ax.plot(self.span_loc_data_wtess[i],self.cl_dist_data_wtess[i],'o-', color=const.colors[i], label='W Tess: '+str(self.m_Tess_W[i]))
        theo_x = [ vec[0] for vec in self.cl_dist_theo_wtess ]
        theo_y = [ vec[1] for vec in self.cl_dist_theo_wtess ]
        ax.plot(theo_x,theo_y,color=const.colors[-1],label='LLT')
        transposed_list_2 = [[self.m_AR10_Y_Cl_Cd_vec[i][j] for i in range(len(self.m_AR10_Y_Cl_Cd_vec))] for j in range(len(self.m_AR10_Y_Cl_Cd_vec[0]))]
        ax.plot(transposed_list_2[0],transposed_list_2[1],'o-', color=const.colors[4], label='AVL')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/chord_tesselation/lift_dist.svg', bbox_inches='tight')
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar Drag Distribution Chord Tesselation (W Tess) Sensitivity')
        ax.set_xlabel('Span Location (Y)')
        ax.set_ylabel('Cd')
        for i in range(len(self.span_loc_data_wtess)):
            ax.plot(self.span_loc_data_wtess[i],self.cd_dist_data_wtess[i],'o-', color=const.colors[i], label='W Tess: '+str(self.m_Tess_W[i]))
        theo_x_cd = [ vec[0] for vec in self.cd_dist_theo_wtess ]
        theo_z_cd = [ vec[1] for vec in self.cd_dist_theo_wtess ]
        ax.plot(theo_x_cd,theo_z_cd,color=const.colors[-1],label='LLT')
        ax.plot(transposed_list_2[0],transposed_list_2[2],'o-', color=const.colors[4],label='AVL')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/chord_tesselation/drag_dist.svg', bbox_inches='tight')
        

        # #Chord Tesselation VSPAERO Setup Table
        # header = ['Analysis','Method','alpha (°)','beta (°)','M','Wake Iterations']
        # data = [['Single Point'],['VLM'],['1.0'],['0.0'],[const.m_MachVec[0]],[const.m_WakeIterVec[0]]] 
        # data_table  = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/hershey_files/hershey_img/chord_tesselation/vspasero_setup.png')

        # #Chord Tesselations AVL Setup
        # header = ['Nchord','Cspace','Nspan','Sspan','M']
        # data = [['30'],['1.0'],['20'],['-3.0'],[const.m_MachVec[0]]] 
        # data_table  = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/hershey_files/hershey_img/chord_tesselation/avl_setup.png')




#========================================= Wake Functions =======================================#
#==================== Generates the relavent parameteres. Runs the Wake Iteration    =============#
#==================== study. Generates the one tables and two charts to include      =============#
#==================== in the markdown file.                                          =============#
#=================================================================================================#

#========== Wrapper function for Wake_Iteration Code ==========================================#    
    def WakeIterationStudy(self):
        self.generateHersheyBarWakeWings()
        self.testHersheyBarWakeWings()
        self.generateWakeChart()

#========== Setup for Wake Iteration Study ====================================================#
    def generateHersheyBarWakeWings(self):
        #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )
        
        #==== Set Wing Section ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()
        
        #==== Set NACA 0012 Airfoil and Common Parms 
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        x = 1 # AR
        u = 1 # UTess
        w = 1 # WTess
        t = 2 # Tip Clustering
        
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', self.m_halfAR[x] ) # Constant AR
        vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] ) # Constant U Tess
        vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] ) # Constant W Tess
        vsp.SetParmVal( wing_id, 'OutCluster', 'XSec_1', self.m_Tip_Clus[t] ) # Constant Tip Clustering
        
        vsp.Update()
        
        for i in range(len(self.m_WakeIter)):
        
            #==== Setup export filenames for Wake Iteration Study ====#
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_Wake' + str(self.m_WakeIter[i]) + '.vsp3'

            #==== Save Vehicle to File ====#
            message = '-->Saving vehicle file to: ' + fname + '\n'
            print( message )
            vsp.WriteVSPFile( fname, vsp.SET_ALL )
            print( 'COMPLETE\n' )
        
        
        vsp.ClearVSPModel()

#========== Run the actual Wake Iteration Study ===============================================#
    def testHersheyBarWakeWings(self):
        print('-> Begin Hershey Bar Wake Study:\n')
        
        num_Wake = len(self.m_WakeIter)
        x = 1 # AR
        
        self.wake_span_loc_data = [[] for i in range(num_Wake)] #array<array<double>>
        self.wake_cl_dist_data  = [[] for i in range(num_Wake)] #array<array<double>>
        self.computation_time   = [0.0]*(num_Wake) #array<double>
        
        # Wake Iteration Study
        for i in range(num_Wake):
        
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_Wake' + str(self.m_WakeIter[i]) + '.vsp3'
            fname_res =scriptpath + '/hershey_files/vsp_files/Hershey_Wake' + str(self.m_WakeIter[i]) + '_res.csv'
    
            #==== Open and test generated wings ====#
            print( 'Reading in file: ' , False)
            print( fname )
            vsp.ReadVSPFile( fname ) # Sets VSP3 file name
            
            #==== Analysis: VSPAEROSinglePoint ====#
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

            #==== Analysis: VSPAero Single Point ====#
            # Set defaults
            vsp.SetAnalysisInputDefaults(const.m_VSPSweepAnalysis)
            print(const.m_VSPSweepAnalysis)

            # Reference geometry set
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'GeomSet', [vsp.SET_NONE], 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'ThinGeomSet', [vsp.SET_ALL], 0)  # Thin geometry - VLM
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'RefFlag', const.m_RefFlagVec, 0)

            wid = vsp.FindGeomsWithName( 'WingGeom' )
            vsp.SetStringAnalysisInput(const.m_VSPSweepAnalysis, 'WingID', wid, 0)

            # Freestream Parameters
            Alpha = [1.0]
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', Alpha, 0)
            AlphaNpts = [1]
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', const.m_MachVec, 0)
            MachNpts = [1]
            vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)
            wake_vec = [self.m_WakeIter[i]]
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'WakeNumIter', wake_vec, 0)
            vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Symmetry', const.m_SymFlagVec, 0)

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

            # Get Load Result ID
            load_rid = vsp.FindLatestResultsID( 'VSPAERO_Load' )
            if ( load_rid != '' ):
            
                # Lift Distribution:
                self.wake_span_loc_data[i] = vsp.GetDoubleResults( load_rid, 'Yavg' )
                self.wake_cl_dist_data[i] = vsp.GetDoubleResults( load_rid, 'cl' )
            
            
            time_vec = vsp.GetDoubleResults( rid, 'Analysis_Duration_Sec' )
            
            if ( len(time_vec) > 0 ):
            
                self.computation_time[i] = time_vec[0]
            
            
            vsp.ClearVSPModel()
        temp = vsp.GetHersheyBarLiftDist( int(100), math.radians(const.m_AlphaVec[0]), self.Vinf, (2*self.m_halfAR[x]), False )
        self.wake_cl_dist_theo = vecofvec3dtolistoflists(temp)

#======== Use Matplotlib to Create tables and Graphs for the Wake Iteration Study ===================#
    def generateWakeChart(self):
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar Lift Distribution Wake Iteration Sensitivity')
        ax.set_xlabel('Span Location (Y)')
        ax.set_ylabel('Cl')
        for i in range(len(self.wake_span_loc_data)):
            ax.plot(self.wake_span_loc_data[i],self.wake_cl_dist_data[i],'o-', color=const.colors[i],label='Wake Iter: '+str(self.m_WakeIter[i]))
        x = [vec[0] for vec in self.wake_cl_dist_theo ]
        y = [vec[1] for vec in self.wake_cl_dist_theo ]
        ax.plot(x,y,color=const.colors[-1],label='LLT')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/wake_iteration/lift_dist.svg', bbox_inches='tight')
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar VSPAERO Total Computation Time vs. Wake Iterations')
        ax.set_xlabel('Wake Iterations')
        ax.set_ylabel('Time (sec)')
        ax.plot(range(0,len(self.computation_time)),self.computation_time,'o-', color='blue',label='Total Computation Time')
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/wake_iteration/comp_time.svg', bbox_inches='tight')
        

        
        # #Wake Iteration VSPAERO Setup Table
        # header = ['Analysis','Method','alpha (°)','beta (°)','M','Wake Iterations']
        # data = [['Single Point'],['VLM'],['1.0'],['0.0'],[const.m_MachVec[0]],['1 to 5']] 
        # data_table  = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/hershey_files/hershey_img/wake_iteration/vspasero_setup.png')
        



#========================================= Advanced Wings Functions ==============================#
#==================== Generates the relavent parameteres. Runs the Advanced Settings =============#
#==================== study. Generates the one tables and two charts to include      =============#
#==================== in the markdown file.                                          =============#
#=================================================================================================#

#========== Wrapper function for Advanced Wings Code ==========================================#    
    def AdvancedSettingsStudy(self):
        self.generateHersheyBarAdvancedWings()
        self.testHersheyBarAdvancedWings()
        self.generateAdvChart()

#========== Setup for Advanced Settings Study ====================================================#
    def generateHersheyBarAdvancedWings(self):
        #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )
        
        #==== Set Wing Section ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()
        
        #==== Set NACA 0012 Airfoil and Common Parms 
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_0', 0.12 )
        vsp.SetParmVal( wing_id, 'ThickChord', 'XSecCurve_1', 0.12 )
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1.0 )
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        x = 2 # AR
        u = 1 # UTess
        w = 1 # WTess
        t = 2 # Tip Clustering
        
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', self.m_halfAR[x] ) # Constant AR
        vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] ) # Constant U Tess
        vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] ) # Constant W Tess
        vsp.SetParmVal( wing_id, 'OutCluster', 'XSec_1', self.m_Tip_Clus[t] ) # Constant Tip Clustering
        
        vsp.Update()
        
        num_case = 4
        
        for i in range(num_case):
        
            #==== Setup export filenames for Wake Iteration Study ====#
            fname =scriptpath + '/hershey_files/vsp_files/Hershey_Advanced_' + str(i) + '.vsp3'

            #==== Save Vehicle to File ====#
            message = '-->Saving vehicle file to: ' + fname + '\n'
            print( message )
            vsp.WriteVSPFile( fname, vsp.SET_ALL )
            print( 'COMPLETE\n' )
        
        
        vsp.ClearVSPModel()

#========== Run the actual Advanced Settings Study ===============================================#
    def testHersheyBarAdvancedWings(self):
        print('-> Begin Hershey Bar Advanced Settings Study:\n')
        
        x = 2 # AR
        t = 2 # Tip Clustering
    
        num_case = 4 # Number of advanced VSPAERO settings to test
        num_wake = len(self.m_AdvancedWakeVec)

        self.m_AdvancedTimeVec = [[0.0]*num_case for i in range(num_wake)]
        
        for w in range (num_wake):
        
            for i in range( num_case ):
            
                fname =scriptpath + '/hershey_files/vsp_files/Hershey_Advanced_' + str(i) + '.vsp3'
                fname_res =scriptpath + '/hershey_files/vsp_files/Hershey_Advanced_' + str(i) + '_res.csv'
        
                #==== Open and test generated wings ====#
                print('Reading in file: ' , False )
                print( fname )
                vsp.ReadVSPFile( fname ) # Sets VSP3 file name
                
                #==== Analysis: VSPAEROSinglePoint ====#
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

                #==== Analysis: VSPAero Single Point ====#
                # Set defaults
                vsp.SetAnalysisInputDefaults(const.m_VSPSweepAnalysis)
                print(const.m_VSPSweepAnalysis)

                # Reference geometry set
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'GeomSet', [vsp.SET_NONE], 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'ThinGeomSet', [vsp.SET_ALL], 0)  # Thin geometry - VLM
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'RefFlag', const.m_RefFlagVec, 0)

                wid = vsp.FindGeomsWithName( 'WingGeom' )
                vsp.SetStringAnalysisInput(const.m_VSPSweepAnalysis, 'WingID', wid, 0)

                # Freestream Parameters
                Alpha = [1.0]
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', Alpha, 0)
                AlphaNpts = [1]
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', const.m_MachVec, 0)
                MachNpts = [1]
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)
                wake_vec=[self.m_WakeIter[w]]
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'WakeNumIter', wake_vec, 0)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Symmetry', const.m_SymFlagVec, 0)

                # if i == 0 -> use default advanced settings
                
                if ( i == 1 ):
                    precon_vec=[vsp.PRECON_JACOBI] # Jacobi Preconditioner
                    vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Precondition', precon_vec, 0)
                
                elif ( i == 2 ):
                    precon_vec=[vsp.PRECON_SSOR] # SSOR Preconditioner
                    vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Precondition', precon_vec, 0)
                
                elif ( i == 3 ):
                    KTCorrect_vec=[0] # 2nd Orrder Mach Correction On
                    vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'KTCorrection', KTCorrect_vec, 0)
                
                
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

                # Get Load Result ID
                load_rid = vsp.FindLatestResultsID( 'VSPAERO_Load' )
                if ( load_rid != '' ):
                
                    # Lift Distribution:
                    self.span_loc_data_adv[w][i] = vsp.GetDoubleResults( load_rid, 'Yavg' )
                    self.cl_dist_data_adv[w][i] = vsp.GetDoubleResults( load_rid, 'cl' )
                
                
                time_vec = vsp.GetDoubleResults( rid, 'Analysis_Duration_Sec' )
            
                if ( len(time_vec) > 0 ):
                
                    self.m_AdvancedTimeVec[w][i] = time_vec[0]
                
                
                vsp.ClearVSPModel()
            temp = vsp.GetHersheyBarLiftDist( int(100), math.radians(const.m_AlphaVec[0]), self.Vinf, (2*self.m_halfAR[x]), False )
            self.cl_dist_theo_adv = vecofvec3dtolistoflists(temp)

#======== Use Matplotlib to Create tables and Graphs for the Advanced Settings Study ===================#
    def generateAdvChart(self):
        
        fig, ax = plt.subplots()
        ax.set_title('Hershey Bar Total Computation Time vs. Wake Iterations Advanced Settings Sensitivity')
        ax.set_xlabel('Wake Iter')
        ax.set_ylabel('Time (Sec)')
        time_vec_trans = [[ self.m_AdvancedTimeVec[i][j] for i in range(len(self.m_AdvancedWakeVec))] for j in range(self.num_case)]
        print(f'time vec{time_vec_trans}')
        for i in range(len(time_vec_trans)):
            ax.plot(range(1,self.num_case),time_vec_trans[i],'o-', color=const.colors[i+1],label='Case #'+str(i+1))
        ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        fig.savefig(scriptpath + '/hershey_files/hershey_img/advanced_settings/comp_time.svg', bbox_inches='tight')
        
        # for plot_n in range(len(self.m_AdvancedWakeVec)):
        #     fig, ax = plt.subplots()
        #     ax.set_title('???')
        #     ax.set_xlabel('???')
        #     ax.set_ylabel('???')
            
        #     for i in range(self.num_case):
        #         ax.plot(self.span_loc_data_adv[plot_n][i],self.cl_dist_data_adv[plot_n][i],'o-', label='???' )
        #     x = [vec[0] for vec in self.cl_dist_theo_adv[plot_n] ]
        #     y = [vec[1] for vec in self.cl_dist_theo_adv[plot_n] ]
        #     ax.plot(x,y)
        #     print('lift dist')
        #     ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
        #     fig.savefig(scriptpath + '/hershey_files/hershey_img/advanced_settings/adv_set_lift_dist_{plot_n}.svg', bbox_inches='tight')
        
        

        # header = const.STUDY_SETUP_TABLE_HEADER.copy() + ['Preconditioner','Mach Correction','Exe Time (sec)']
        # data_base = [['Default','1','2','3'],['Single Point']*4,['VLM']*4,['1.0']*4,[0.0]*4,[const.m_MachVec[0],const.m_MachVec[0],const.m_MachVec[0],const.m_MachVec[0]]]

        # #Wake Iter = 1 Setup Table
        # data = data_base + [[1]*4,['Matrix','Jacobi','SSOR','Matrix'],['Off']*3+['On'],[t for t in time_vec_trans[0]]+[0]]
        # table = make_table(header,data)
        # print(len(header),' ',len(data),' ',header,' ',data)
        # export_png(table,filename=scriptpath + '/hershey_files/hershey_img/advanced_settings/vspasero_setup1.png')

        # #Wake Iter = 2 Setup Table
        # data = data_base + [[1]*4,['Matrix','Jacobi','SSOR','Matrix'],['Off']*3+['On'],[t for t in time_vec_trans[1]]+[0]]
        # table = make_table(header,data)
        # print(len(header),' ',len(data),' ',header,' ',data)
        # export_png(table,filename=scriptpath + '/hershey_files/hershey_img/advanced_settings/vspasero_setup2.png')

        # #Wake Iter = 3 Setup Table
        # data = data_base + [[1]*4,['Matrix','Jacobi','SSOR','Matrix'],['Off']*3+['On'],[t for t in time_vec_trans[2]]+[0]]
        # table = make_table(header,data)
        # print(len(header),' ',len(data),' ',header,' ',data)
        # export_png(table,filename=scriptpath + '/hershey_files/hershey_img/advanced_settings/vspasero_setup3.png')


#==========FUNCTIONS FOR TESING THE FUNCTIONALITY OF EACH FUNCTION===================#
def test_init():
    print('Testing HersheyTest __init__()')
    hershey = HersheyTest()
    print(f'\tm_halfAR {hershey.m_halfAR}')
    print(f'\tm_AlphaNpts {hershey.m_AlphaNpts}')
    print(f'\tm_Tip_Clus {hershey.m_Tip_Clus}')
    print(f'\tm_Tess_U {hershey.m_Tess_U}')
    print(f'\tm_WakeIter {hershey.m_WakeIter}')
    print(f'\tm_AdvancedWakeVec {hershey.m_AdvancedWakeVec}')
    print(f'\tm_AR10_Y_Cl_Cd_vec {hershey.m_AR10_Y_Cl_Cd_vec}')
    print('\n')
    return hershey

def test_hershey_generate(hershey: HersheyTest):
    print('Testing Wing Generation')
    print('\t Testing ARWings')
    try:
        hershey.generateHersheyBarARWings()
        print('Completed generateHersheyBarARWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed generateHersheyBarARWings()')
        traceback.print_exc()
        return
    print('\t Testing UWTessWings')
    try:
        hershey.generateHersheyBarUWTessWings()
        print('Completed generateHersheyBarUWTessWings()')
        print('-------------------------------------')

    except:
        print('\tERROR: Failed generateHersheyBarUWTessWings()')
        traceback.print_exc()
        return
    print('\t Testing TCWings')
    try:
        hershey.generateHersheyBarTCWings()
        print('Completed generateHersheyBarTCWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed generateHersheyBarTCWings()')
        traceback.print_exc()
        return
    print('\t Testing UTessWings')
    try:
        hershey.generateHersheyBarUTessWings()
        print('Completed generateHersheyBarARWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed generateHersheyBarUTessWings()')
        traceback.print_exc()
        return
    print('\t Testing WTessWings')
    try:
        hershey.generateHersheyBarWTessWings()
        print('Completed generateHersheyBarWTessWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed generateHersheyBarWTessWings()')
        traceback.print_exc()
        return
    print('\t Testing WakeWings')
    try:
        hershey.generateHersheyBarWakeWings()
        print('Completed generateHersheyBarWakeWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed generateHersheyBarWakeWings()')
        traceback.print_exc()
        return
    print('\t Testing AdvancedWings')
    try:
        hershey.generateHersheyBarAdvancedWings()
        print('Completed generateHersheyBarAdvancedWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed generateHersheyBarAdvancedWings()')
        traceback.print_exc()
        return

def test_hershey_test(hershey: HersheyTest):
    print('Testing Test Functions')
    print('\t Testing ARWings')
    try:
        hershey.testHersheyBarARWings()
        print('\tCompleted testHersheyBarARWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed testHersheyBarARWings()')
        traceback.print_exc()
        return
    try:
        hershey.testHersheyBarUWTessWings()
        print('\tCompleted testHersheyBarUWTessWings()')
        print('-------------------------------------')

    except:
        print('\tERROR: Failed testHersheyBarUWTessWings()')
        traceback.print_exc()
        return

    try:
        hershey.testHersheyBarTCWings()
        print('\tCompleted testHersheyBarTCWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed testHersheyBarTCWings()')
        traceback.print_exc()
        return
    try:
        hershey.testHersheyBarUTessWings()
        print('\tCompleted testHersheyBarUTessWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed testHersheyBarUTessWings()')
        traceback.print_exc()
        return

    try:
        hershey.testHersheyBarWTessWings()
        print('\tCompleted testHersheyBarWTessWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed testHersheyBarWTessWings()')
        traceback.print_exc()
        return

    try:
        hershey.testHersheyBarWakeWings()
        print('\tCompleted testHersheyBarWakeWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed testHersheyBarWakeWings()')
        traceback.print_exc()
        return

    try:
        hershey.testHersheyBarAdvancedWings()
        print('\tCompleted testHersheyBarAdvancedWings()')
        print('-------------------------------------')
    except:
        print('\tERROR: Failed testHersheyBarAdvancedWings()')
        traceback.print_exc()
        return
    
def generateCharts(hershey: HersheyTest):
    hershey.generateARWingChart()
    hershey.generateUWTessChart()
    hershey.generateTCWingChart()
    hershey.generateHersheyBarUTessChart()
    hershey.generateWTessChart()
    hershey.generateWakeChart()
    hershey.generateAdvChart()

 
def setup_filepaths():
    scriptpathlib = Path(__file__).parent.resolve()
    testnames = ['hershey_files/']
    subnames = [['hershey_img/','vsp_files/']]
    subsubnames = [[['advanced_settings','angle_of_attack','aspect_ratio','chord_tesselation','span_tesselation','tesselation','tip_clustering','wake_iteration'],['']]]
    for i in range(len(testnames)):
        for j in range(len(subnames[i])):
            for k in range(len(subsubnames[i][j])):
                dirname = Path.joinpath(scriptpathlib, testnames[i]+subnames[i][j]+subsubnames[i][j][k])
                dirname.mkdir(parents=True, exist_ok=True)
                
def runhersheybarstudy(arandaoa = 3, uw = 3, tc=3, ctes=3, stes=3, wi=3, ads=3):
    setup_filepaths()
    test = HersheyTest()
    # This feels like it should be a loop, but it seems like that would need lazy evaluation or a redesign, and this is the only real case where it matters.
    
    if (arandaoa == 1 or arandaoa == 2):
        with open(scriptpath+'/hershey_files/arandaoatest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (arandaoa == 1): 
        test.generateARWingChart()
    if (arandaoa == 3):
        test.AspectRatioAndAngleOfAttackStudy()
        with open(scriptpath+'/hershey_files/arandaoatest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (arandaoa == 2):
        test.testHersheyBarARWings()
        test.generateARWingChart()
        with open(scriptpath+'/hershey_files/arandaoatest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)      
    ################################################################################
            
    if (uw == 1 or uw == 2):
        with open(scriptpath+'/hershey_files/uwtest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (uw == 1): 
        test.generateUWTessChart()
    if (uw == 3):
        test.TesselationStudy()
        with open(scriptpath+'/hershey_files/uwtest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (uw == 2):
        test.testHersheyBarUWTessWings()
        test.generateUWTessChart()
        with open(scriptpath+'/hershey_files/uwtest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)  
    ################################################################################

    if (tc == 1 or tc == 2):
        with open(scriptpath+'/hershey_files/tctest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (tc == 1): 
        test.generateTCWingChart()
    if (tc == 3):
        test.TipClusteringStudy()
        with open(scriptpath+'/hershey_files/tctest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (tc == 2):
        test.testHersheyBarTCWings()
        test.generateTCWingChart()
        with open(scriptpath+'/hershey_files/tctest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)  
    ################################################################################
    if (ctes == 1 or ctes == 2):
        with open(scriptpath+'/hershey_files/ctestest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (ctes == 1): 
        test.generateWTessChart()
    if (ctes == 3):
        test.ChordTesselationStudy()
        with open(scriptpath+'/hershey_files/ctestest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (ctes == 2):
        test.testHersheyBarWTessWings()
        test.generateWTessChart()
        with open(scriptpath+'/hershey_files/ctestest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)  
    ################################################################################
    if (stes == 1 or stes == 2):
        with open(scriptpath+'/hershey_files/stestest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (stes == 1): 
        test.generateHersheyBarUTessChart()
    if (stes == 3):
        test.SpanTesselationStudy()
        with open(scriptpath+'/hershey_files/stestest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (stes == 2):
        test.testHersheyBarUTessWings()
        test.generateHersheyBarUTessWings()
        with open(scriptpath+'/hershey_files/stestest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)  
    ################################################################################
    if (wi == 1 or wi == 2):
        with open(scriptpath+'/hershey_files/witest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (wi == 1): 
        test.generateWakeChart()
    if (wi == 3):
        test.WakeIterationStudy()
        with open(scriptpath+'/hershey_files/witest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (wi == 2):
        test.testHersheyBarWakeWings()
        test.generateWakeChart()
        with open(scriptpath+'/hershey_files/witest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)  
    ################################################################################
    if (ads == 1 or ads == 2):
        with open(scriptpath+'/hershey_files/adstest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (ads == 1): 
        test.generateAdvChart()
    if (ads == 3):
        test.AdvancedSettingsStudy()
        with open(scriptpath+'/hershey_files/adstest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (ads == 2):
        test.testHersheyBarAdvancedWings()
        test.generateAdvChart()
        with open(scriptpath+'/hershey_files/adstest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)  
    ################################################################################
    
def unit_test_hershey():

    setup_filepaths()
    hershey = test_init()

    test_hershey_generate(hershey)

    test_hershey_test(hershey)
    print('New Generate')
    generateCharts(hershey)


if __name__ == '__main__':
    runhersheybarstudy(arandaoa = 3, uw = 3, tc=3, ctes=3, stes=3, wi=3, ads=3)
    # unit_test_hershey()

# self.AspectRatioAndAngleOfAttackStudy()
#         if(self.uw):
#             self.TesselationStudy()
#         if(self.tc):
#             self.TipClusteringStudy()
#         if(self.ut):
#             self.SpanTesselationStudy()
#         if(self.wt):
#             self.ChordTesselationStudy()
#         if(self.wi):
#             self.WakeIterationStudy()
#         if(self.a_s):
#             self.AdvancedSettingsStudy()