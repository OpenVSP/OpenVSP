import openvsp as vsp


lbefore = vsp.ListAnalysis()

vsp.VSPRenew()

lafter = vsp.ListAnalysis()

assert len(lafter) == len(lbefore)

