import openvsp as vsp
import math
import Constants as const
from pathlib import Path
import matplotlib.pyplot as plt
import pickle

scriptpath = str(Path(__file__).parent.resolve())


class VKTTest:
    '''!Class for running and collecting data from the
        Von Karman-Trefftz studies
    '''
    def __init__(self):
        self.m_epsilon = [0.1,0.2]
        self.m_kappa = [0,0.1]
        self.m_tau = [0,10]
        
        #index of values to use for UW study from the above lists
        self.e = 0
        self.k = 1
        self.t = 1
        
        #Add another intermediary chord and span tesselation value?
        self.m_Tess_U = [12,20,41]
        self.m_Tess_W = [17,29,51]
        
        self.xyz_airfoil_mat_ekt = [[[0.0]*len(self.m_tau) for x in range(len(self.m_kappa))] for y in range(len(self.m_epsilon))]
        self.xyz_airfoil_mat_ekt_noswig = [[[0.0]*len(self.m_tau) for x in range(len(self.m_kappa))] for y in range(len(self.m_epsilon))]
        
        self.cp_airfoil_mat_ekt = [[[0.0]*len(self.m_tau) for x in range(len(self.m_kappa))] for y in range(len(self.m_epsilon))]
        self.x_slicer_mat_ekt = [[[0.0]*len(self.m_tau) for x in range(len(self.m_kappa))] for y in range(len(self.m_epsilon))]
        self.cp_slicer_mat_ekt = [[[0.0]*len(self.m_tau) for x in range(len(self.m_kappa))] for y in range(len(self.m_epsilon))]
        
        self.xyz_airfoil_mat_tess = [[0.0]*len(self.m_Tess_W) for x in range(len(self.m_Tess_U))]
        self.x_slicer_mat_tess = [[0.0]*len(self.m_Tess_W) for x in range(len(self.m_Tess_U))]
        self.cp_slicer_mat_tess = [[0.0]*len(self.m_Tess_W) for x in range(len(self.m_Tess_U))]
        self.xyz_airfoil_mat_tess_noswig = [[0.0]*len(self.m_Tess_W) for x in range(len(self.m_Tess_U))]
        
        self.Xfoil_CpDist = []

        

#========================================= Von Karman-Trefftz Functions =================================#
#==================== Generates the relavent parameteres. Runs the ____________      =============#
#==================== ___________________ studies. Generates the ___ tables and      =============#
#==================== _____ charts charts to include in the markdown file.           =============#
#=================================================================================================#

#========== Wrapper function for ________________________________ Code ===========================#
    def VKTEKTStudy(self):
        self.GenerateVKTEKTWings()
        self.TestVKTEKTWings()
        self.GenerateVKTEKTCharts()
        
    def VKTUWTessStudy(self):
        self.GenerateVKTUWTessWings()
        self.TestVKTUWTessWings()
        self.GenerateVKTUWTessCharts()
        
#===================== Sweapt UWTess Generation Functions =====================
    def GenerateVKTEKTWings(self):
        #INSERT lines 5159 - 5188 from v&v script
        #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )
        
        # Set VKT airfoil
        xsec_surf = vsp.GetXSecSurf( wing_id, 0 )
        vsp.ChangeXSecShape( xsec_surf, 0, vsp.XS_VKT_AIRFOIL )
        xsec_surf1 = vsp.GetXSecSurf(wing_id, 1)
        vsp.ChangeXSecShape( xsec_surf1, 1, vsp.XS_VKT_AIRFOIL )

        #==== Set Wing Section Controls to AR, Root Chord, and Tail Chord ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()

#==== Set Common Parms ====#
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1 )

        #TODO: Make consistent with other VKT wing geoms
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', 15.0001 )
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        u = 2 # UTess
        w = 2 # WTess
        
        vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', self.m_Tess_U[u] ) # Constant U Tess
        vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', self.m_Tess_W[w] ) # Constant W Tess

        vsp.Update()

        for e in self.m_epsilon:
            for k in self.m_kappa:
                for t in self.m_tau:
                    
                    vsp.SetParmVal( wing_id, 'Epsilon', 'XSecCurve_0', e )
                    vsp.SetParmVal( wing_id, 'Epsilon', 'XSecCurve_1', e )
                    vsp.SetParmVal( wing_id, 'Kappa', 'XSecCurve_0', k )
                    vsp.SetParmVal( wing_id, 'Kappa', 'XSecCurve_1', k )
                    vsp.SetParmVal( wing_id, 'Tau', 'XSecCurve_0', t )
                    vsp.SetParmVal( wing_id, 'Tau', 'XSecCurve_1', t )

                    vsp.Update()
                    
                    #==== Setup export filenames for EKT Study ====#
                    fname = scriptpath + '/vkt_files/vsp_files/vkt_e'+str(e)+'_k'+str(k)+'_t'+str(t)+'.vsp3'

                    #==== Save Vehicle to File ====#
                    print('-->Saving vehicle file to: ' + fname + '\n' )
                    vsp.WriteVSPFile( fname, vsp.SET_ALL )
                    print( 'COMPLETE\n' )

        vsp.ClearVSPModel()

#========== Run the actual ____________ Studies ==============================#
    def TestVKTEKTWings(self):
        #Insert lines 5159-5377 from v&v script
        print( '-> Begin VKT Sweep Test:\n' )
        
        
        Alpha = [0.0]
        num_epsilon = len(self.m_epsilon)
        num_kappa = len(self.m_kappa)
        num_tau = len(self.m_tau)
        
        cut = [0.0] #cut at root
        npts_l = 122 #low point density.
        
        for e in range(num_epsilon):
            for k in range(num_kappa):
                for t in range(num_tau):
        
                    
                    #==== Open and test generated wing ====#
                    fname = scriptpath + '/vkt_files/vsp_files/vkt_e'+str(self.m_epsilon[e])+'_k'+str(self.m_kappa[k])+'_t'+str(self.m_tau[t])+'.vsp3'
                    fname_res = scriptpath + '/vkt_files/vsp_files/vkt_e'+str(self.m_epsilon[e])+'_k'+str(self.m_kappa[k])+'_t'+str(self.m_tau[t])+'_res.csv'

                    print( 'Reading in file: ')
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
                    vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'GeomSet', [vsp.SET_ALL], 0)  # Thick geometry - Panel
                    vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'ThinGeomSet', [vsp.SET_NONE], 0)

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
                    vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'GeomSet', [vsp.SET_ALL], 0)  # Thick geometry - Panel
                    vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'ThinGeomSet', [vsp.SET_NONE], 0)
                    vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'RefFlag', const.m_RefFlagVec, 0)
                    #vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'Symmetry', const.m_SymFlagVec, 0) #TODO: Add symmetry if VSPAERO doesn't crash

                    wid = vsp.FindGeomsWithName( 'WingGeom' )
                    vsp.SetStringAnalysisInput(const.m_VSPSweepAnalysis, 'WingID', wid, 0)

                    # Freestream Parameters

                    vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', Alpha, 0)
                    AlphaNpts = [1]
                    vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
                    vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'Machstart', const.m_MachVec, 0)
                    MachNpts = [1]
                    vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)
                    vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'WakeNumIter', const.m_WakeIterVec, 0)

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
                    
                    # Calculate Analytical Solution
                    self.xyz_airfoil_mat_ekt[e][k][t] = vsp.GetVKTAirfoilPnts( npts_l, Alpha[0], self.m_epsilon[e], self.m_kappa[k], math.radians(self.m_tau[t] ))
                    self.cp_airfoil_mat_ekt[e][k][t] = vsp.GetVKTAirfoilCpDist( Alpha[0], self.m_epsilon[e], self.m_kappa[k], math.radians(self.m_tau[t] ), self.xyz_airfoil_mat_ekt[e][k][t] )
                    
                    # Setup and Execute CpSlicer
                    print( '\tGenerating Cp Slices...\n' )
                    
                    #==== Analysis: CpSlicer ====#
                    print( const.m_CpSliceAnalysis )

                    # Set defaults
                    vsp.SetAnalysisInputDefaults( const.m_CpSliceAnalysis )

                    # Setup cuts
                    vsp.SetDoubleAnalysisInput(const.m_CpSliceAnalysis, 'YSlicePosVec', cut, 0 )

                    # list inputs, type, and current values
                    vsp.PrintAnalysisInputs( const.m_CpSliceAnalysis )
                    print( '' )

                    # Execute
                    print( '\tExecuting...' )
                    sid = vsp.ExecAnalysis( const.m_CpSliceAnalysis )
                    print( 'COMPLETE' )

                    # Get & Display Results
                    vsp.PrintResults( sid )
                    print( '' )
                    
                    rid_vec = vsp.GetStringResults( sid, 'CpSlice_Case_ID_Vec' )
                    if ( len(rid_vec) >= 1 ):
                        self.x_slicer_mat_ekt[e][k][t] = vsp.GetDoubleResults( rid_vec[0], 'X_Loc' )
                        self.cp_slicer_mat_ekt[e][k][t] = vsp.GetDoubleResults( rid_vec[0], 'Cp' )
                        
                    vsp.ClearVSPModel()
        for e in range(len(self.m_epsilon)):
            for k in range(len(self.m_kappa)):
                for t in range(len(self.m_tau)):
                    replaceswigstuff = []
                    for i in range(len(self.xyz_airfoil_mat_ekt[e][k][t])):
                        replaceswigstuff.append([self.xyz_airfoil_mat_ekt[e][k][t][i].x(), self.xyz_airfoil_mat_ekt[e][k][t][i].y(), self.xyz_airfoil_mat_ekt[e][k][t][i].z()])
                    self.xyz_airfoil_mat_ekt_noswig[e][k][t] = const.transpose(replaceswigstuff)
        self.xyz_airfoil_mat_ekt = [[[0.0]*len(self.m_tau)]*len(self.m_kappa)]*len(self.m_epsilon)
        
                    
                    
                    
#======== Use Matplotlib to Create tables and Graphs for the _________ Studies =#
    def GenerateVKTEKTCharts(self):
        # title = 'VKT ε κ τ Study Geometry Setup'
        # header = ['Airfoil', 'ε','κ','τ (°)','AR', 'Root Chord', 'Tip Chord', 'Λ (°)', 'Span Tess (U)','Chord Tess (W)','LE Clustering','TE Clustering']
        # data = [['VKT'], [str(self.m_epsilon[0])+' to '+str(self.m_epsilon[-1])], [str(self.m_kappa[0])+' to '+str(self.m_kappa[-1])],[str(self.m_tau[0])+' to '+str(self.m_tau[-1])],['30'],['1.0'],['1.0'],['0.0'],['41'],['51'],['0.2'],['1.0']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/vkt_files/vkt_img/ekt/ektgeometrysetup.png')
        
        # title = 'VKT ε κ τ Study VSPAERO Setup'
        # header = ['Analysis', 'Method', 'α (°)', 'β (°)', 'M', 'Wake Iterations']
        # data = [['Single Point'], ['Panel'], ['0.0'],['0.0'],['0.1'],['3']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/vkt_files/vkt_img/ekt/ektvspaerosetup.png')
        

        for e in range(len(self.m_epsilon)):
            for k in range(len(self.m_kappa)):
                for t in range(len(self.m_tau)):
                    fig, ax = plt.subplots()
                    ax.set_title('VKT Cp Distribution at Y=0: Epsilon = '+str(self.m_epsilon[e])+', Kappa = '+str(self.m_kappa[k])+', Tau = '+str(self.m_tau[t])+'°')
                    ax.set_xlabel('Chord Location (X)')
                    ax.set_ylabel('Cp')
                    ax.plot(self.x_slicer_mat_ekt[e][k][t],self.cp_slicer_mat_ekt[e][k][t], 'o-', label='VSPAERO',color=const.colors[2])
                    ax.plot(self.xyz_airfoil_mat_ekt_noswig[e][k][t][0],self.cp_airfoil_mat_ekt[e][k][t], label='Exact',color=const.colors[-1])
                    ax.set_ylim(-1.5,1.5)
                    ax2 = ax.twinx()
                    ax2.set_ylim(-.175,.175)
                    ax2.set_ylabel('Z')
                    
                    ax2.plot(self.xyz_airfoil_mat_ekt_noswig[e][k][t][0],self.xyz_airfoil_mat_ekt_noswig[e][k][t][1], label='Airfoil',color=const.colors[0])
                    #ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
                    ax.legend(bbox_to_anchor=(.5,-.1),loc='upper center', ncols=10)
                    fig.savefig(scriptpath + '/vkt_files/vkt_img/ekt/ekt_'+str(e*4+k*2+t)+'.svg', bbox_inches='tight')

        #print(self.cp_airfoil_mat_ekt[e][k][t])
        #print(type(self.cp_airfoil_mat_ekt[e][k][t]))

    def GenerateVKTUWTessWings(self):
        #==== Add Wing Geometry ====#
        wing_id = vsp.AddGeom( 'WING', '' )
        
        # Set VKT airfoil
        xsec_surf = vsp.GetXSecSurf( wing_id, 0 )
        vsp.ChangeXSecShape( xsec_surf, 0, vsp.XS_VKT_AIRFOIL )
        xsec_surf1 = vsp.GetXSecSurf(wing_id, 1)
        vsp.ChangeXSecShape( xsec_surf1, 1, vsp.XS_VKT_AIRFOIL )
        
        #==== Set Wing Section Controls ====#
        vsp.SetDriverGroup( wing_id, 1, vsp.AR_WSECT_DRIVER, vsp.ROOTC_WSECT_DRIVER, vsp.TIPC_WSECT_DRIVER )
        
        vsp.Update()
        
        #==== Set Common Parms ====#
        vsp.SetParmVal( wing_id, 'Sweep', 'XSec_1', 0 )
        vsp.SetParmVal( wing_id, 'Root_Chord', 'XSec_1', 1 )
        vsp.SetParmVal( wing_id, 'Tip_Chord', 'XSec_1', 1 )
        
        #TODO: Make consistent with other VKT wing geoms
        vsp.SetParmVal( wing_id, 'Aspect', 'XSec_1', 7.5005 )
        vsp.SetParmVal( wing_id, 'TECluster', 'WingGeom', 1.0 )
        vsp.SetParmVal( wing_id, 'LECluster', 'WingGeom', 0.2 )
        
        
        vsp.SetParmVal( wing_id, 'Epsilon', 'XSecCurve_0', self.m_epsilon[self.e] )
        vsp.SetParmVal( wing_id, 'Epsilon', 'XSecCurve_1', self.m_epsilon[self.e] )
        vsp.SetParmVal( wing_id, 'Kappa', 'XSecCurve_0', self.m_kappa[self.k] )
        vsp.SetParmVal( wing_id, 'Kappa', 'XSecCurve_1', self.m_kappa[self.k] )
        vsp.SetParmVal( wing_id, 'Tau', 'XSecCurve_0', self.m_tau[self.t] )
        vsp.SetParmVal( wing_id, 'Tau', 'XSecCurve_1', self.m_tau[self.t] )
        
        for u in self.m_Tess_U:
            for w in self.m_Tess_W:
                vsp.SetParmVal( wing_id, 'Tess_W', 'Shape', w )
                vsp.SetParmVal( wing_id, 'SectTess_U', 'XSec_1', u )

                vsp.Update()

                #==== Setup export filenames ====#
                fname = scriptpath + '/vkt_files/vsp_files/VKT_U' + str(u) + '_W' + str(w) + '.vsp3'

                #==== Save Vehicle to File ====#
                message = '-->Saving vehicle file to: ' + fname + '\n'
                print(message )
                vsp.WriteVSPFile( fname, vsp.SET_ALL )
                print( 'COMPLETE\n' )
            
        
        
        vsp.ClearVSPModel()
        
    def TestVKTUWTessWings(self):
        print( '-> Begin VKT Tesselation Test:\n' ) 
        
        num_TessU = len(self.m_Tess_U)
        num_TessW = len(self.m_Tess_W)
        
        z_slicer_mat_tess = [[0.0]*num_TessW]*num_TessU
        # index 0: U Tess, index 1: W Tess
        
        cut=[0.0] # cut at root
        npts_l = 122 # low point density       
        Alpha = [0.0]
        
        # Load XFoil's Cp Distribution for VKT with epsilon = 0.1, kappa = 0.1, && tau = 10°
        xfoil_file_name = scriptpath + '/../../airfoil/XFoil_VKT_CpDist.txt'
        self.Xfoil_CpDist = const.ReadCpDistFile( xfoil_file_name ) #list of lenght 3 lists
        
        # Calculate Analytical Solution
        self.xyz_airfoil_mat_tess = vsp.GetVKTAirfoilPnts(npts_l, Alpha[0], self.m_epsilon[self.e], self.m_kappa[self.k], math.radians(self.m_tau[self.t]) ) #array<vec3d> 
        self.cp_airfoil_mat_tess = vsp.GetVKTAirfoilCpDist( Alpha[0], self.m_epsilon[self.e], self.m_kappa[self.k], math.radians(self.m_tau[self.t]), self.xyz_airfoil_mat_tess ) #array<double> 
        
        #==== Tesselation Study ====#
        for u in range(num_TessU):
            for w in range(num_TessW):
                #==== Open and test generated wings ====#
                # Note: *.csv result file name can not be the same as *.vsp3 file name
                fname = scriptpath + '/vkt_files/vsp_files/VKT_U' + str(self.m_Tess_U[u]) + '_W' + str(self.m_Tess_W[w]) + '.vsp3'
                fname_res = scriptpath + '/vkt_files/vsp_files/VKT_U' + str(self.m_Tess_U[u]) + '_W' + str(self.m_Tess_W[w]) + '_res.csv'

                print( 'Reading in file: ')
                print( fname )
                vsp.ReadVSPFile( fname ) # Sets VSP3 file name
                
                #==== Analysis: VSPAEROSinglePoint ====#
                print( const.m_VSPSweepAnalysis )

                #==== Analysis: VSPAero Compute Geometry to Create Vortex Lattice DegenGeom File ====#
                print( const.m_CompGeomAnalysis )

                # Set defaults
                vsp.SetAnalysisInputDefaults( const.m_CompGeomAnalysis )

                # list inputs, type, and current values
                vsp.PrintAnalysisInputs( const.m_CompGeomAnalysis )

                vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'GeomSet', [vsp.SET_ALL], 0)  # Thick geometry - Panel
                vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'ThinGeomSet', [vsp.SET_NONE], 0)

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

                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'GeomSet', [vsp.SET_ALL], 0)  # Thick geometry - Panel
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'ThinGeomSet', [vsp.SET_NONE], 0)

                # Freestream Parameters
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', Alpha, 0)
                AlphaNpts = [1]
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', const.m_MachVec, 0)
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

                # Setup and Execute CpSlicer
                print( '\tGenerating Cp Slices...\n' )
                
                #==== Analysis: CpSlicer ====#
                print( const.m_CpSliceAnalysis )

                # Set defaults
                vsp.SetAnalysisInputDefaults( const.m_CpSliceAnalysis )
                
                # Setup cuts
                vsp.SetDoubleAnalysisInput( const.m_CpSliceAnalysis, 'YSlicePosVec', cut, 0 )

                # list inputs, type, and current values
                vsp.PrintAnalysisInputs( const.m_CpSliceAnalysis )
                print( '' )

                # Execute
                print( '\tExecuting...' )
                sid = vsp.ExecAnalysis( const.m_CpSliceAnalysis )
                print( 'COMPLETE' )

                # Get & Display Results
                vsp.PrintResults( sid )
                print( '' )
                
                rid_vec = vsp.GetStringResults( sid, 'CpSlice_Case_ID_Vec' )
                if ( len(rid_vec) > 0 ):
                    self.x_slicer_mat_tess[u][w] = vsp.GetDoubleResults( rid_vec[0], 'X_Loc' )
                    z_slicer_mat_tess[u][w] = vsp.GetDoubleResults( rid_vec[0], 'Z_Loc' )
                    self.cp_slicer_mat_tess[u][w] = vsp.GetDoubleResults( rid_vec[0], 'Cp' )

                vsp.ClearVSPModel()
        
        replaceswigstuff = []
        for i in range(len(self.xyz_airfoil_mat_tess)):
            replaceswigstuff.append([self.xyz_airfoil_mat_tess[i].x(), self.xyz_airfoil_mat_tess[i].y(), self.xyz_airfoil_mat_tess[i].z()])
        self.xyz_airfoil_mat_tess_noswig = replaceswigstuff
        
        self.xyz_airfoil_mat_tess = [[0.0]*len(self.m_Tess_W)]*len(self.m_Tess_U)
                
    def GenerateVKTUWTessCharts(self):
        # title = 'VKT Chord Tesselation Study Geometry Setup'
        # header = ['Airfoil', 'ε','κ','τ (°)','AR', 'Root Chord', 'Tip Chord', 'Λ (°)', 'Span Tess (U)','Chord Tess (W)','LE Clustering','TE Clustering']
        # data = [['VKT'], ['0.1'], ['0.1'],['10'],['15'],['1.0'],['1.0'],['0.0'],[str(self.m_Tess_U[0])+' to '+str(self.m_Tess_U[-1])],[str(self.m_Tess_W[0])+' to '+str(self.m_Tess_W[-1])],['0.2'],['1.0']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/vkt_files/vkt_img/uw/uwgeometrysetup.png')
        
        # title = 'VKT ε κ τ Study VSPAERO Setup'
        # header = ['Analysis', 'Method', 'α (°)', 'β (°)', 'M', 'Wake Iterations']
        # data = [['Single Point'], ['Panel'], ['0.0'],['0.0'],['0.1'],['3']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/vkt_files/vkt_img/uw/uwvspaerosetup.png')
        
        for u in range(len(self.m_Tess_U)):
            fig, ax = plt.subplots()
            ax.set_title('VKT Cp Distribution at Y=0 Chord Tesselation (W Tess) Sensitivity: Span Tess = '+str(self.m_Tess_U[u]))
            ax.set_xlabel('Chord Location (X)')
            ax.set_ylabel('Cp')
            transpose = const.transpose(self.Xfoil_CpDist)
            ax.plot(transpose[0],transpose[2],'o-', label='XFoil',color=const.colors[2])
            transpose = const.transpose(self.xyz_airfoil_mat_tess_noswig)
            ax.plot(transpose[0],self.cp_airfoil_mat_tess, label='Exact',color=const.colors[-1])
            for w in range(len(self.m_Tess_W)):
                ax.plot(self.x_slicer_mat_tess[u][w],self.cp_slicer_mat_tess[u][w],'o-' ,label='W Tess: '+str(self.m_Tess_W[w]),color=const.colors[3+w])
            ax.set_ylim(-1.5,1.5)
            ax2 = ax.twinx()
            ax2.set_ylim(-.175,.175)
            ax2.set_ylabel('Z')
            ax2.plot(transpose[0],transpose[1], label='Airfoil',color=const.colors[0])
            #ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
            ax.legend(bbox_to_anchor=(.5,-.1),loc='upper center', ncols=10)
            fig.savefig(scriptpath + '/vkt_files/vkt_img/uw/u_'+str(u)+'.svg', bbox_inches='tight')
        

        for w in range(len(self.m_Tess_W)):
            fig, ax = plt.subplots()
            ax.set_title('VKT Cp Distribution at Y=0 Span Tesselation (U Tess) Sensitivity: Chord Tess = '+str(self.m_Tess_W[w]))
            ax.set_xlabel('Chord Location (X)')
            ax.set_ylabel('Cp')
            transpose = const.transpose(self.Xfoil_CpDist)
            ax.plot(transpose[0],transpose[2], 'o-', label='XFoil',color=const.colors[2])
            transpose = const.transpose(self.xyz_airfoil_mat_tess_noswig)
            ax.plot(transpose[0],self.cp_airfoil_mat_tess, label='Exact',color=const.colors[-1])
            for u in range(len(self.m_Tess_U)):
                ax.plot(self.x_slicer_mat_tess[u][w],self.cp_slicer_mat_tess[u][w],'o-', label='U Tess: '+str(self.m_Tess_U[u]),color=const.colors[3+u])
            ax.set_ylim(-1.5,1.5)
            ax2 = ax.twinx()
            ax2.set_ylim(-.175,.175)
            ax2.set_ylabel('Z')
            ax2.plot(transpose[0],transpose[1], label='Airfoil',color=const.colors[0])
            #ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
            ax.legend(bbox_to_anchor=(.5,-.1),loc='upper center', ncols=10)
            fig.savefig(scriptpath + '/vkt_files/vkt_img/uw/w_'+str(w)+'.svg', bbox_inches='tight')
            
            

        
    
def runVKTstudy(ekt = 3, uw = 3):
    setup_filepaths()
    
    test = VKTTest()
    if (ekt == 1 or ekt == 2):
        with open(scriptpath+'/vkt_files/vkttest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (ekt == 1): 
        test.GenerateVKTEKTCharts()
    if (ekt == 3):
        test.VKTEKTStudy()
        with open(scriptpath+'/vkt_files/vkttest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (ekt == 2):
        test.TestVKTEKTWings()
        test.GenerateVKTEKTCharts()
        with open(scriptpath+'/vkt_files/vkttest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)        
            
    if (uw == 1 or uw == 2):
        with open(scriptpath+'/vkt_files/vkttest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (uw == 1): 
        test.GenerateVKTUWTessCharts()
    if (uw == 3):
        test.VKTUWTessStudy()
        with open(scriptpath+'/vkt_files/vkttest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (uw == 2):
        test.TestVKTUWTessWings()
        test.GenerateVKTUWTessCharts()
        with open(scriptpath+'/vkt_files/vkttest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)               
            
def setup_filepaths():
    scriptpathlib = Path(__file__).parent.resolve()
    testnames = ['vkt_files/']
    subnames = [['vkt_img/','vsp_files/']]
    subsubnames = [[['ekt','uw'],['']]]
    for i in range(len(testnames)):
        for j in range(len(subnames[i])):
            for k in range(len(subsubnames[i][j])):
                dirname = Path.joinpath(scriptpathlib, testnames[i]+subnames[i][j]+subsubnames[i][j][k])
                dirname.mkdir(parents=True, exist_ok=True)
                
if __name__ == '__main__':
    runVKTstudy(ekt = 3,uw = 3)    
