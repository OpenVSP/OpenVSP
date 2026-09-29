import os

import pytest

import cadoptions


def pytest_addoption( parser ):
    parser.addoption( "--cad-survey-dir", default = None,
                      help = "test_CADExport: also run a survey of the models in this folder, one folder of "
                             ".vsp3 files per model" )
    parser.addoption( "--cad-slow", action = "store_true", default = False,
                      help = "test_CADExport: also mesh the survey's slow structures" )
    parser.addoption( "--cad-keep", action = "store_true", default = False,
                      help = "test_CADExport: keep the files written" )


def pytest_configure( config ):
    survey_dir = config.getoption( "--cad-survey-dir" )
    if survey_dir:
        survey_dir = os.path.expanduser( survey_dir )
        if not os.path.isdir( survey_dir ):
            raise pytest.UsageError( "--cad-survey-dir: no folder %s" % survey_dir )
    cadoptions.survey_dir = survey_dir
    cadoptions.slow = config.getoption( "--cad-slow" )
    cadoptions.keep = config.getoption( "--cad-keep" )
