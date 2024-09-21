#Constants pulled from V&V Base Class in Master_VSP_VV_Script.vspscript

#TABLE CONSTANTS
STUDY_SETUP_TABLE_HEADER =["Case #","Analysis","Method","alpha (°)","beta (°)","M","Wake Iterations"]

#STRING CONSTANTS
m_VSPSweepAnalysis = "VSPAEROSweep"
m_CompGeomAnalysis = "VSPAEROComputeGeometry"
m_CpSliceAnalysis = 'CpSlicer'

#VECTOR CONSTANTS
m_GeomVec = [0]
m_AlphaVec = [1.0]
m_MachVec = [0.1]
m_SymFlagVec = [1]
m_RefFlagVec = [1] # Wing Reference
m_WakeIterVec = [3]

#Bokeh graph constants
bokehcolors = ["blue","red","gold","green","purple", "skyblue","gray"] #Bokeh uses css color names https://www.w3schools.com/colors/colors_names.asp
bokehlinewidth = 3
bokehsize = 5
bokehwidth = 800
bokehheight = 400        

#CONSTANTS
b = 0.9949874371 # M = 0.1, b = (1-M^2)^0.5
k_theo = 1.0

#Transpose utility function
def transpose(listoflists):
    duped =  [[0.0]*len(listoflists) for i in range(len(listoflists[0]))]
    for i in range(len(listoflists)):
        for j in range(len(listoflists[i])):
            duped[j][i] = listoflists[i][j]
    return duped

#other utility functions that were out of place

def ReadCpDistFile( file_name ):
    with open(file_name, 'r') as cp_file:
        lines = cp_file.readlines()
        x_y_cp_vec = []
        for linenum in range(len(lines)):
            if (linenum > 2):
                text = (lines[linenum]).split()
                print(text)
                floats = [float(text[0]),float(text[1]),float(text[2])]
                x_y_cp_vec.append(floats)
    print(x_y_cp_vec)        
    return x_y_cp_vec
