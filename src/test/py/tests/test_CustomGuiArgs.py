# The arguments a Custom component's script gives AddGui.  CustomScripts/GuiArgsPart.vsppart keeps what each
# AddGui returned in its Results group: the control's index, or -1 when the arguments were refused.

import openvsp as vsp

from testhelp import pop_errors


def added():
    vsp.VSPRenew()
    pop_errors()
    gid = vsp.AddGeom( "GuiArgsPart" )
    assert gid, "GuiArgsPart.vsppart did not load"
    returned = { name: round( vsp.GetParmVal( vsp.FindParm( gid, name, "Results" ) ) )
                 for name in ( "ChoiceValue", "ChoiceNamedValue", "ChoiceBadValue", "ChoiceParmName", "TwoInput",
                               "TwoInputOneParm" ) }
    return returned, pop_errors()


def test_a_choice_item_given_an_integer_value_is_added():
    returned, _ = added()
    assert returned[ "ChoiceValue" ] >= 0


def test_a_choice_item_value_can_be_passed_by_name():
    returned, _ = added()
    assert returned[ "ChoiceNamedValue" ] >= 0


def test_a_choice_item_value_that_is_not_an_integer_is_reported_and_the_item_kept():
    returned, errors = added()
    assert returned[ "ChoiceBadValue" ] >= 0
    assert any( "Other value Mode is not an integer" in e for e in errors ), errors


def test_a_choice_item_given_a_parm_name_is_added_without_complaint():
    # A Parm name and group (arg2, arg3) have no meaning for a choice item, as before choice items took values.
    returned, errors = added()
    assert returned[ "ChoiceParmName" ] >= 0
    assert not any( "Named" in e for e in errors ), errors


def test_a_two_input_slider_given_two_parms_is_added():
    returned, _ = added()
    assert returned[ "TwoInput" ] >= 0


def test_a_two_input_slider_given_one_parm_is_refused():
    returned, errors = added()
    assert returned[ "TwoInputOneParm" ] == -1
    assert any( "none for its second" in e for e in errors ), errors


def test_only_the_refused_controls_are_errors():
    _, errors = added()
    assert len( errors ) == 2, errors


def test_a_negative_integer_parm_reads_back_as_itself():
    vsp.VSPRenew()
    gid = vsp.AddGeom( "GuiArgsPart" )
    pid = vsp.FindParm( gid, "ChoiceValue", "Results" )
    for val in ( -2, -1, 0, 1 ):
        vsp.SetParmVal( pid, val )
        assert vsp.GetIntParmVal( pid ) == val
