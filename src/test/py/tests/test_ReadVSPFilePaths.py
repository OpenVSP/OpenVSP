# Paths a model saves relative to its own file.
#
# ReadVSPFile names the model before reading it, so a path saved relative to the file comes back
# against the file's folder, wherever the reader is running from.

import openvsp as vsp

import os


def testATexturePathComesBackAgainstTheFile( tmp_path, monkeypatch ):
    vsp.VSPRenew()

    model_dir = tmp_path / "model"
    model_dir.mkdir()
    tex = str( model_dir / "tex.png" )
    open( tex, "wb" ).close()

    pod = vsp.AddGeom( "POD" )
    vsp.AttachGeomTexture( pod, tex )
    vsp.Update()

    fname = str( model_dir / "tex.vsp3" )
    vsp.WriteVSPFile( fname )

    # Read from somewhere else, after a model that was never saved.
    elsewhere = tmp_path / "elsewhere"
    elsewhere.mkdir()
    monkeypatch.chdir( elsewhere )
    vsp.VSPRenew()

    vsp.ReadVSPFile( fname )

    pod = vsp.FindGeoms()[0]
    tid = vsp.GetGeomTextureIDVec( pod )[0]
    got = vsp.GetGeomTextureFileName( pod, tid )
    assert os.path.samefile( got, tex )


def testAFileThatCannotBeReadKeepsTheModelName( tmp_path ):
    vsp.VSPRenew()
    vsp.ErrorMgrSingleton.getInstance().SilenceErrors()

    fname = str( tmp_path / "kept.vsp3" )
    vsp.AddGeom( "POD" )
    vsp.WriteVSPFile( fname )
    assert vsp.GetComputationFileName( vsp.COMP_GEOM_TXT_TYPE ) == str( tmp_path / "kept_CompGeom.txt" )

    bad = str( tmp_path / "bad.vsp3" )
    with open( bad, "w" ) as f:
        f.write( "<NotAModel/>\n" )

    vsp.ReadVSPFile( bad )
    mgr = vsp.ErrorMgrSingleton.getInstance()
    assert mgr.GetNumTotalErrors() > 0
    for _ in range( mgr.GetNumTotalErrors() ):
        mgr.PopLastError()

    # The export names follow the model's name.
    assert vsp.GetComputationFileName( vsp.COMP_GEOM_TXT_TYPE ) == str( tmp_path / "kept_CompGeom.txt" )
