
void AddVars()
{
    AddInput( "PodGeom", 0, "Length", "Design", "L" );
    AddInput( "PodGeom", 0, "FineRatio", "Design", "FR" );

    AddOutput( "PodGeom", 1, "FineRatio", "Design", "OutFR" );

}

double L;
double FR;
double OutFR;

void LoadInput()
{
    L  = GetVar( "L" );
    FR = GetVar( "FR" );
}

void LoadOutput()
{
    SetVar( "OutFR", OutFR );
}

void UpdateLink()
{

    LoadInput();

    double D = L/FR;

    OutFR = D;

    LoadOutput();

}

