DATCOM component help figures: MDC_BOD, MDC_PROTUB, MDC_INLET and MDC_FIN
=======================================================================

mdc_bod_figs.py draws the figures of ../../MdcBod.md into ../../images/: MdcBod_layout, _noses, _secant, _blunt, _aft,
_sections, _camber.  The figures of the other three components are in their own folders beside this one:

    ../mdc_protub/mdc_protub_figs.py    MdcProtub.md:  MdcProtub_types, _members, _dims, _phi
    ../mdc_inlet/mdc_inlet_figs.py      MdcInlet.md:   MdcInlet_types, _stations, _ramp, _hdiv, _diverter, _phi
    ../mdc_fin/mdc_fin_figs.py    MdcFin.md:  MdcFin_overview, _panels, _planform, _sta, _flaps, _sections, _naca

datcomfig.py holds what they share: building a component and a body for it to stand on, sampling every surface of a
component, and the view from behind the body, in which the namelists measure a roll angle.  The drawing helpers are
those of the other body pages, in ../fuselagestack/bodyfig.py.  Nothing in these folders is installed; keep them free
of .md files.

    cd src/help/figure_src/mdc_bod
    <build>/venv/bin/python mdc_bod_figs.py            # every figure, or name some (see FIGS at the end)
    <build>/venv/bin/python verify.py                 # checks what the four pages state, against VSP

verify.py prints what it measured; a failed check raises.  Rerun it after any change to the four Geoms.

mdc_protub_figs.py reads the sizes the Set Default button gives from the table in src/geom_core/MdcProtubGeom.cpp, because the
button is not in the API.

See ../fuselagestack/README.txt for the conventions.  2D views keep OpenVSP's frame; the view from behind the body, the
way the namelists measure a roll angle clockwise from the top looking forward, has Y to the right and Z up.
