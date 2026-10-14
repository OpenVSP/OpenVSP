# The group of each propeller parm the propeller tests set by name.
#
# Not a test file -- pytest collects test_*.py, which import this.

# The parms in the Articulation group; every other propeller parm the tests set is in Design.
ARTICULATION = (
    "RFeatherAx",
    "RFlapAx", "AxFlapAx", "OffFlapAx", "AzFlapDir", "ElFlapDir", "FlapAngle",
    "RLagAx", "AxLagAx", "OffLagAx", "AzLagDir", "CantLagDir", "LagAngle",
    "FlapMean", "LagMean", "FeatherMean", "PsiRef", "BladeMotionMode", "NumHarmonics",
)
ARTICULATION_PER_BLADE = (
    "Feather_",
    "FlapAngle_",
    "LagAngle_",
    "FlapCos_", "FlapSin_", "LagCos_", "LagSin_", "FeatherCos_", "FeatherSin_",
)


def group( name ):
    if name in ARTICULATION or name.startswith( ARTICULATION_PER_BLADE ):
        return "Articulation"
    return "Design"
