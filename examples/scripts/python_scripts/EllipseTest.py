import openvsp as vsp
import math
import Constants as const
import matplotlib.pyplot as plt
from pathlib import Path
# from bokeh.models.ranges import Range1d 
# from bokeh.models import LinearAxis
# from bokeh.plotting import figure, show
# from bokeh.io import export_png
# from bohek_helper import make_table
import pickle

scriptpath = str(Path(__file__).parent.resolve())

class EllipsoidTest:
    '''!Class for running and collecting data from the
        Ellipsoid studies
    '''
    def __init__(self):
        self.m_alpha_vec = [0.0,20.0]
        self.m_beta_vec = [0.0,20.0]
        
        self.cp_slicer_mat = [[[0.0,0.0,0.0] for x in range(len(self.m_beta_vec))] for y in range(len(self.m_alpha_vec))]
        self.xyz_slicer_mat = [[[0.0,0.0,0.0] for x in range(len(self.m_beta_vec))] for y in range(len(self.m_alpha_vec))]
        self.ellipsoid_cp_mat = [[[0.0,0.0,0.0] for x in range(len(self.m_beta_vec))] for y in range(len(self.m_alpha_vec))]
        
        self.x_slice_pnt_vec_noswig = []
        self.y_slice_pnt_vec_noswig = []
        self.z_slice_pnt_vec_noswig = []

        

#========================================= Ellipse Functions =================================#
#==================== Generates the relavent parameteres. Runs the ____________      =============#
#==================== ___________________ studies. Generates the ___ tables and      =============#
#==================== _____ charts charts to include in the markdown file.           =============#
#=================================================================================================#

#========== Wrapper function for ________________________________ Code ===========================#
    def EllipsoidStudy(self):
        self.GenerateEllipsoid()
        self.TestEllipsoid()
        self.GenerateEllipsoidCharts()
        
            
#===================== Sweapt UWTess Generation Functions =====================
    def GenerateEllipsoid(self):
        
        #==== Add Wing Geometry ====#
        geom_id = vsp.AddGeom( 'ELLIPSOID', '' )
        
        vsp.SetParmVal( geom_id, 'X_Rel_Location', 'XForm', -1.0 )
        vsp.SetParmVal( geom_id, 'A_Radius', 'Design', 1.0 )
        vsp.SetParmVal( geom_id, 'B_Radius', 'Design', 2.0 )
        vsp.SetParmVal( geom_id, 'C_Radius', 'Design', 3.0 )
        vsp.SetParmVal( geom_id, 'Tess_U', 'Shape', 40 )
        vsp.SetParmVal( geom_id, 'Tess_W', 'Shape', 41 )

        vsp.Update()

        for a in self.m_alpha_vec:
            for b in self.m_beta_vec:    
                #==== Setup export filenames ====#
                fname = scriptpath + '/ellipse_files/vsp_files/Ellipsoid_alpha' + str(a) + '_beta' + str(b) + '.vsp3'

                #==== Save Vehicle to File ====#
                message = '-->Saving vehicle file to: ' + fname + '\n'
                print(message )
                vsp.WriteVSPFile( fname, vsp.SET_ALL )
                print( 'COMPLETE\n' )
        vsp.ClearVSPModel()

#========== Run the actual ____________ Studies ==============================#
    def TestEllipsoid(self):

        print( '-> Begin Ellipsoid Test:\n' )
        
        
        num_alpha = len(self.m_alpha_vec)
        num_beta = len(self.m_beta_vec)

        
        center = vsp.vec3d(0.0,0.0,0.0)
        abc_rad = vsp.vec3d(1.0,2.0,3.0)
        abc_rad_noswig = [1.0,2.0,3.0]
        V_inf = 100.0

        npts =101
        cut = [0.0]
        
        x_slice_pnt_vec = []
        y_slice_pnt_vec = []
        z_slice_pnt_vec = []
        
        theta_vec = [0]
        for i in range(npts-1):
            theta_vec.append(theta_vec[i]+(math.pi*2/(npts-1)))
        
        for i in range(npts):
            x_slice_pnt_vec.append(vsp.vec3d(0,abc_rad_noswig[1]*math.cos(theta_vec[i]),abc_rad_noswig[2]*math.sin(theta_vec[i])))
            self.x_slice_pnt_vec_noswig.append([0,abc_rad_noswig[1]*math.cos(theta_vec[i]),abc_rad_noswig[2]*math.sin(theta_vec[i])])
            
            y_slice_pnt_vec.append(vsp.vec3d(abc_rad_noswig[0]*math.cos(theta_vec[i]),0,abc_rad_noswig[2]*math.sin(theta_vec[i])))
            self.y_slice_pnt_vec_noswig.append([abc_rad_noswig[0]*math.cos(theta_vec[i]),0,abc_rad_noswig[2]*math.sin(theta_vec[i])])
               
            z_slice_pnt_vec.append(vsp.vec3d(abc_rad_noswig[0]*math.cos(theta_vec[i]),abc_rad_noswig[1]*math.sin(theta_vec[i]),0)) 
            self.z_slice_pnt_vec_noswig.append([abc_rad_noswig[0]*math.cos(theta_vec[i]),abc_rad_noswig[1]*math.sin(theta_vec[i]),0])
            
            
        for a in range(num_alpha):
            for b in range(num_beta):
                          
                #==== Open and test generated ellipsoids ====#
                fname = scriptpath + '/ellipse_files/vsp_files/Ellipsoid_alpha'+str(self.m_alpha_vec[a])+'_beta'+str(self.m_beta_vec[b])+'.vsp3'
                fname_res = scriptpath + '/ellipse_files/vsp_files/vkt_e'+str(self.m_alpha_vec[a])+'_beta'+str(self.m_beta_vec[b])+'_res.csv'

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
                panel_analysis = [vsp.PANEL]
                
                vsp.SetIntAnalysisInput(const.m_CompGeomAnalysis, 'AnalysisMethod', panel_analysis)

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

                #Note: Symmetry will cause VSPAERO to crash

                # Freestream Parameters
                Alpha = [self.m_alpha_vec[a]]
                Beta = [self.m_beta_vec[b]]
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaStart', Alpha, 0)
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'BetaStart', Beta, 0)
                AlphaNpts = [1]
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AlphaNpts', AlphaNpts, 0)

                Mach = [0.0]
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachStart', Mach, 0)
                MachNpts = [1]
                vsp.SetDoubleAnalysisInput(const.m_VSPSweepAnalysis, 'MachNpts', MachNpts, 0)

                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'GeomSet', const.m_GeomVec)
                vsp.SetIntAnalysisInput(const.m_VSPSweepAnalysis, 'AnalysisMethod', panel_analysis)
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

                # Indicate VSPAERO analysis type for CpSlicer (dCp vs. Cp Results)
                vsp.SetIntAnalysisInput( const.m_CpSliceAnalysis, 'AnalysisMethod', panel_analysis )
                
                # Setup cuts
                vsp.SetDoubleAnalysisInput(const.m_CpSliceAnalysis, 'XSlicePosVec', cut, 0 )
                vsp.SetDoubleAnalysisInput(const.m_CpSliceAnalysis, 'YSlicePosVec', cut, 0 )
                vsp.SetDoubleAnalysisInput(const.m_CpSliceAnalysis, 'ZSlicePosVec', cut, 0 )

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
                print(rid_vec)
                #if ( len(rid_vec) > 0 ):
                # X Cut
                self.xyz_slicer_mat[a][b][0] = vsp.GetDoubleResults( rid_vec[0], 'Y_Loc' )
                self.cp_slicer_mat[a][b][0] = vsp.GetDoubleResults( rid_vec[0], 'Cp' )
                
                # Y Cut
                self.xyz_slicer_mat[a][b][1] = vsp.GetDoubleResults( rid_vec[1], 'X_Loc' )
                self.cp_slicer_mat[a][b][1] = vsp.GetDoubleResults( rid_vec[1], 'Cp' )
                
                # Z Cut
                self.xyz_slicer_mat[a][b][2] = vsp.GetDoubleResults( rid_vec[2], 'X_Loc' )
                self.cp_slicer_mat[a][b][2] = vsp.GetDoubleResults( rid_vec[2], 'Cp' )
                    
                # Calculate theoretical distribution
                V_vec = vsp.vec3d((V_inf*math.cos(math.radians(self.m_alpha_vec[a]))*math.cos(math.radians(self.m_beta_vec[b]))), (V_inf*math.sin(math.radians(self.m_beta_vec[b]))), (V_inf*math.sin(math.radians(self.m_alpha_vec[a]))*math.cos(math.radians(self.m_beta_vec[b]))))
                self.ellipsoid_cp_mat[a][b][0] = vsp.GetEllipsoidCpDist(x_slice_pnt_vec, abc_rad, V_vec)
                self.ellipsoid_cp_mat[a][b][1] = vsp.GetEllipsoidCpDist(y_slice_pnt_vec, abc_rad, V_vec)
                self.ellipsoid_cp_mat[a][b][2] = vsp.GetEllipsoidCpDist(z_slice_pnt_vec, abc_rad, V_vec)
        
                vsp.ClearVSPModel()      
                    
                    
                    
#======== Use Bokeh to Create tables and Graphs for the _________ Studies =#
    def GenerateEllipsoidCharts(self):
        # title = 'Ellipsoid Geometry Setup'
        # header = ['A Radius', 'B Radius','C Radius','Center','Span Tess (U)', 'Chord Tess (W)']
        # data = [['1.0'], ['2.0'], ['3.0'],['(0.0, 0.0, 0.0)'],['40'],['41']]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/ellipse_files/ellipse_img/ellipse/geometrysetup.png')
        
        # title = 'Ellipsoid VSPAERO Setup'
        # header = ['Run Case #','Analysis', 'Method', 'α (°)', 'β (°)', 'M', 'Wake Iterations']
        # data = [['1','2','3','4'],['Single Point']*4, ['Panel']*4, ['0.0','0.0','20.0','20.0'],['0.0','20.0','0.0','20.0'],['0.0']*4,['3']*4]
        # data_table = make_table(header,data)
        # export_png(data_table,filename=scriptpath + '/ellipse_files/ellipse_img/ellipse/vspaerosetup.png')
        
        slic = ['X','Y','Z']
        count = 0
        for a in range(len(self.m_alpha_vec)):
            for b in range(len(self.m_beta_vec)):
                for i in range(3): # x y and z slices
                    if i == 0:
                        x, y2, pntvec = 1, 2, const.transpose(self.x_slice_pnt_vec_noswig)
                    if i == 1:
                        x, y2, pntvec = 0, 2, const.transpose(self.y_slice_pnt_vec_noswig)
                    if i == 2:
                        x, y2, pntvec = 0, 1, const.transpose(self.z_slice_pnt_vec_noswig)
                    fig, ax = plt.subplots()
                    ax.plot(pntvec[x],self.ellipsoid_cp_mat[a][b][i], label='Exact',color=const.bokehcolors[-1])
                    ax.scatter(self.xyz_slicer_mat[a][b][i],self.cp_slicer_mat[a][b][i], label='VSPAERO',color=const.bokehcolors[2])
                    ax.set_title('Ellipsoid Cp Distribution at '+slic[i]+'= 0: Alpha = '+str(self.m_alpha_vec[a])+'°, Beta = '+str(self.m_beta_vec[b])+'°')
                    ax.set_xlabel(slic[x])
                    ax.set_ylabel('Cp')
                    ax.set_ylim(-5.,1.5)
                    ax2 = ax.twinx()
                    ax2.set_ylim(-3,3)
                    ax2.set_ylabel(slic[y2])
                    ax2.plot(pntvec[x],pntvec[y2],label='Ellipsoid',color=const.bokehcolors[0])
                    #ax.legend(bbox_to_anchor=(1.05,1),loc='center left')
                    ax.legend(bbox_to_anchor=(.5,-.1),loc='upper center', ncols=10)
                    fig.savefig(scriptpath + '/ellipse_files/ellipse_img/ellipse/'+str(count)+'.svg', bbox_inches='tight')
                
                    count += 1
        #print(self.cp_airfoil_mat_ekt[e][k][t])
        #print(type(self.cp_airfoil_mat_ekt[e][k][t]))

    
        
    
def runEllipsoidstudy(mode = 3):
    setup_filepaths()
    
    test = EllipsoidTest()
    if (mode == 1 or mode == 2):
        with open(scriptpath+'/ellipse_files/ellipsetest.pckl','rb') as picklefile:    
            test = pickle.load(picklefile)
    if (mode == 1): 
        test.GenerateEllipsoidCharts()
    if (mode == 3):
        test.EllipsoidStudy()
        with open(scriptpath+'/ellipse_files/ellipsetest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)
    if (mode == 2):
        test.TestEllipsoid()
        test.GenerateEllipsoidCharts()
        with open(scriptpath+'/ellipse_files/ellipsetest.pckl','wb') as picklefile:
            pickle.dump(test,picklefile)        
            
            
def setup_filepaths():
    scriptpathlib = Path(__file__).parent.resolve()
    testnames = ['ellipse_files/']
    subnames = [['ellipse_img/','vsp_files/']]
    subsubnames = [[['ellipse'],['']]]
    for i in range(len(testnames)):
        for j in range(len(subnames[i])):
            for k in range(len(subsubnames[i][j])):
                dirname = Path.joinpath(scriptpathlib, testnames[i]+subnames[i][j]+subsubnames[i][j][k])
                dirname.mkdir(parents=True, exist_ok=True)
                
if __name__ == '__main__':
    runEllipsoidstudy(mode = 3)    
