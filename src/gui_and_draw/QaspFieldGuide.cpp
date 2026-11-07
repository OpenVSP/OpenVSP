//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// QaspFieldGuide.cpp
//
// QASP addition (not present in upstream OpenVSP).
//
// A single self-contained FLTK window that explains aircraft design in plain
// English.  It deliberately depends on nothing but FLTK, so it cannot break the
// geometry, analysis or scripting code.
//

#include "QaspFieldGuide.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

#include <string>
#include <vector>
#include <cstring>
#include <cctype>

using std::string;
using std::vector;

//============================================================================//
// Static state
//============================================================================//

Fl_Window*       QaspFieldGuide::s_Window  = nullptr;
Fl_Browser*      QaspFieldGuide::s_Topics  = nullptr;
Fl_Text_Display* QaspFieldGuide::s_Text    = nullptr;
Fl_Text_Buffer*  QaspFieldGuide::s_Buffer  = nullptr;
Fl_Input*        QaspFieldGuide::s_Search  = nullptr;
int              QaspFieldGuide::s_Current = -1;

//============================================================================//
// Colours - a deliberately flat, modern palette rather than the 1990s bevels
//============================================================================//

static Fl_Color QaspInk()      { return fl_rgb_color( 28,  36,  48  ); }  // near-black text
static Fl_Color QaspNavy()     { return fl_rgb_color( 24,  40,  72  ); }  // header bar
static Fl_Color QaspAccent()   { return fl_rgb_color( 0,   122, 204 ); }  // accent blue
static Fl_Color QaspPaper()    { return fl_rgb_color( 255, 255, 255 ); }  // content background
static Fl_Color QaspPanel()    { return fl_rgb_color( 246, 248, 251 ); }  // side panel
static Fl_Color QaspMuted()    { return fl_rgb_color( 108, 122, 137 ); }  // secondary text

//============================================================================//
// Content
//============================================================================//

struct QaspTopic
{
    const char* title;
    const char* body;
};

static const QaspTopic g_Topics[] =
{
//----------------------------------------------------------------------------
{
"Start Here - Build Your First Aircraft",
R"QASP(SO YOU WANT TO DESIGN AN AIRCRAFT
======================================

This walks you through your first model. It takes about 15 minutes.
Nothing here is dangerous - you can undo anything with Ctrl+Z.

The short version of aircraft design is this:

    A wing makes lift. Lift fights weight.
    An engine makes thrust. Thrust fights drag.
    Everything else is a detail about HOW those four things balance.

If lift < weight the aircraft cannot leave the ground.
If thrust < drag it cannot go faster.
Every design decision you make changes one of those four numbers.


STEP 1 - START FROM A BLANK FILE
---------------------------------
File > New gives you an empty model. You are looking at empty space,
not a broken program.

The 3D view is the big grey area. Drag with the mouse to rotate.
Scroll to zoom. Press F to fit everything on screen.


STEP 2 - ADD A FUSELAGE (the body)
-----------------------------------
Model > Geometry... opens the geometry browser. Choose "Fuselage".

A fuselage is a stack of cross-sections. OpenVSP gives you a few by
default. In the 3D view you should now see a tube shape.

The fuselage does three jobs:
  1. Holds the payload (people, cargo, sensors, batteries)
  2. Connects the wing and tail together
  3. Creates drag - usually a lot of it

Do not spend long making it pretty yet. It only needs to be roughly
the right SIZE. A small general-aviation aircraft is about 7-8 m long.
An airliner is 35-70 m.


STEP 3 - ADD A WING
--------------------
Back in Geometry, choose "Wing".

You now have the single most important part of the aircraft.
Four numbers matter more than all the rest:

  SPAN          - wingtip to wingtip distance.
                  More span = more efficient. That is why gliders
                  have enormously long wings.

  AREA          - how much wing there is. More area = the aircraft
                  can fly slower before it stalls.

  ASPECT RATIO  - roughly span squared divided by area. A measure of
                  how long and thin the wing is. High = efficient,
                  low = agile.

  SWEEP         - how far back the wing leans. 0 degrees for slow
                  aircraft, 25-40 for fast jets.

Reasonable starting point for a light aircraft:
  Span 11 m, Area 16 m squared, Aspect ratio 7, Sweep 0 degrees.


STEP 4 - ADD A TAIL
--------------------
Add a "Wing" again and name it Tail, or use the dedicated tail
components. Aircraft need a horizontal surface to control pitch
(nose up/down) and a vertical surface to control yaw (nose left/right).

Rule of thumb: the horizontal tail is about 20% of the wing area,
and sits about 2-3 wing chords behind the wing.

Without a tail your aircraft is like a dart with no flights - it
will want to tumble.


STEP 5 - GIVE IT MASS
----------------------
Mass Properties (under Analysis) needs to know how heavy things are.
OpenVSP does not guess. You must enter a mass per component, or a
mass per unit area.

Start rough:
  Wing        60 kg
  Fuselage   200 kg
  Tail        15 kg
  Payload    180 kg    (two people)
  Fuel        90 kg

Total around 545 kg. A Cessna 172 is about 1000 kg, so this is a
small two-seater. That is fine.

The tool tells you where the centre of gravity is. THIS MATTERS
ENORMOUSLY - see the "What happens if..." page.


STEP 6 - FIND OUT IF IT CAN FLY
--------------------------------
Run these three, in this order:

  1. CompGeom        - measures your shape. Everything else uses it.
  2. Mass Properties - weight and balance.
  3. Parasite Drag   - how much drag the shape makes.

Then VSPAERO if you want lift, drag polars and top speed.

The "Reading Your Results" page explains what the numbers mean,
and "How Fast and How High" gives you the arithmetic to turn them
into a real answer.


WHAT TO DO WHEN IT LOOKS WRONG
-------------------------------
It will look wrong at first. That is normal and it is not a failure.

  Wing in the wrong place?  Drag it in the 3D view, or type X/Y/Z
                            in the geometry screen.
  Shape looks twisted?      Check you have not typed a sweep or
                            dihedral value in the wrong box.
  Nothing is visible?       Press F (fit on screen).

Type a number, look at the result, press Ctrl+Z. That loop is the
entire craft of conceptual design.
)QASP"
},
//----------------------------------------------------------------------------
{
"Plain-English Glossary",
R"QASP(AIRCRAFT TERMS, IN PLAIN ENGLISH
======================================

Each entry: what it means, and why you should care.


--- THE SHAPE OF THE AIRCRAFT ---

FUSELAGE
  The body. Holds people, cargo and fuel, and joins the wing to the tail.
  Usually the biggest single source of drag.

WING
  The surface that makes lift by pushing air downwards.
  Its shape is the aircraft's most important design decision.

SPAN
  Distance from one wingtip to the other, tip to tip.
  More span means less drag for a given amount of lift. That is why
  gliders look like they do. The cost is weight and a wider parking space.

CHORD
  The distance from the front of the wing to the back.
  A longer chord gives a "deeper" wing that lifts more at low speed.

WING AREA (S)
  The flat area you would see looking straight down at the wing.
  More area = you can fly slower before stalling. Also more drag.

ASPECT RATIO (AR)
  Roughly: how long and thin the wing is. AR = span squared / area.
  High AR (12+) = efficient, glider-like, poor roll rate.
  Low AR (3-5) = fighter-like: agile, draggy.

TAPER RATIO
  How much narrower the wingtip is than the wing root.
  1.0 = rectangle (both ends same width). 0.0 = triangle.
  Most aircraft use 0.3-0.5: a compromise between weight and stall behaviour.

SWEEP
  How far back the wing leans, in degrees.
  0 degrees = straight out sideways. Good for slow aircraft.
  30-40 degrees = delays the shock waves that hurt fast aircraft,
  but makes low-speed lift worse and encourages the tips to stall first.

DIHEDRAL
  The upward angle of the wing, like a shallow V seen from the front.
  Positive dihedral makes the aircraft want to level itself.
  Too much = it wanders. Too little = it wants to roll over.

TWIST (WASHOUT)
  The wingtip is set at a slightly lower angle than the root.
  This makes the root stall first, which keeps the ailerons working
  near the stall. Nearly every safe aircraft has a few degrees of it.

CAMBER
  How much the wing cross-section curves. More camber = more lift at
  low speed, but more drag at high speed.

THICKNESS RATIO (t/c)
  Wing thickness divided by chord. Around 12-18% for slow aircraft,
  4-6% for fast ones. Thin wings slice through air; thick wings hold
  fuel and structure but create drag.

ANGLE OF ATTACK (alpha)
  The angle between the wing and the air rushing at it.
  More angle = more lift, up to a point. Past that point the air
  separates and the wing STALLS, losing most of its lift suddenly.


--- THE FORCES ---

LIFT (L)
  Upward force from the wing. Must equal weight for level flight.

WEIGHT (W)
  Total mass times gravity. Everything must be weighed.

THRUST (T)
  Forward force from engine or propeller.

DRAG (D)
  Backward force from air resistance. Two kinds:
    Parasite drag - friction and shape drag from the whole aircraft.
    Induced drag  - a penalty for making lift. Big at low speed.

LIFT COEFFICIENT (CL)
  A number describing how much lift the wing makes at a given angle.
  Higher CL = more lift. CL_max decides your stall speed.

DRAG COEFFICIENT (CD)
  Same idea for drag. CD0 (pronounced "CD zero") is the drag
  when the aircraft is NOT making lift: pure shape and friction.

L/D RATIO
  Lift divided by drag. THE single most useful efficiency number.
  Max L/D is how far you can glide per metre of height lost, and for
  propeller aircraft it is roughly the best-range condition.
  Glider: 40. Light aircraft: 10-15. Airliner: 17-20.

WING LOADING (W/S)
  Weight divided by wing area. High loading = fast and firm-landing.
  Low loading = slow, floaty, short runways.

THRUST-TO-WEIGHT (T/W)
  More than 1.0 means you can accelerate straight up. Fighters are
  around 0.8-1.2; airliners around 0.3.

STALL SPEED
  The slowest speed at which the wing can still hold the aircraft up.
  Below it, the aircraft falls. This is THE safety-critical number.


--- ALTITUDE AND SPEED ---

SERVICE CEILING
  The highest altitude where the aircraft can still climb at a
  useful rate (conventionally 100 feet per minute). Above it the
  air is too thin for the engine and wing to keep working well.

MACH NUMBER
  Speed as a fraction of the speed of sound. Mach 1 = about 343 m/s
  at sea level, but less at altitude because the air is colder.

CRITICAL MACH
  The speed where air flowing over some part of the wing first
  reaches Mach 1. Past it, drag rises steeply - the "sound barrier".

REYNOLDS NUMBER
  A measure of whether the airflow is smooth (laminar) or chaotic
  (turbulent). Bigger and faster = higher Reynolds. It changes which
  drag predictions you can trust.


--- BALANCE ---

CENTRE OF GRAVITY (CG)
  The balance point of the aircraft. Everything's mass pulls through
  it. If the CG is in the wrong place, no amount of design skill
  saves you - it will be unflyable or dangerously unstable.

STATIC MARGIN
  How far the CG sits ahead of the wing's aerodynamic centre,
  as a fraction of chord. Positive = self-correcting = stable.
  Too large = sluggish and draggy. Too small or negative = the
  aircraft will pitch up on its own. Aim for roughly 5-15%.

PITCHING MOMENT
  The tendency of the aircraft to rotate nose-up or nose-down.
  Wings and tails are shaped to control this deliberately.

WETTED AREA
  The total surface area touched by air. Another name for "how much
  skin friction you are going to pay for". The bigger the wetted
  area, the higher the parasite drag.
)QASP"
},
//----------------------------------------------------------------------------
{
"What Happens If I Change...?",
R"QASP(WHAT HAPPENS IF I CHANGE...?
===============================

Every change is a trade. There is no free improvement in aircraft
design - if there were, every aircraft would already look like that.

These are the honest consequences, including the bad ones.


IF I MAKE THE WING LONGER (more span)
--------------------------------------
  GOOD: Less induced drag, so better L/D. Lower stall speed. Better
        climb. Better range. Everything gets efficient.
  BAD:  More bending load at the root, so the wing must be stronger
        and therefore heavier. Slower roll rate. Bigger hangar.
  REAL:  This is why gliders have 20 m spans and aerobatic aircraft do not.

IF I MAKE THE WING BIGGER (more area)
--------------------------------------
  GOOD: Lower stall speed. Shorter takeoff and landing runs.
        More room for fuel in the wing.
  BAD:  More wetted area, so more parasite drag. Heavier structure.
        Lower top speed.
  REAL:  Trade low-speed safety against high-speed efficiency.

IF I SWEEP THE WING BACK MORE
------------------------------
  GOOD: Raises critical Mach, so the aircraft can fly faster before
        drag explodes. Looks fast.
  BAD:  Low-speed lift gets worse. The tips tend to stall first,
        which can cause an abrupt wing drop. Needs more twist and
        more structure.
  REAL:  Sweep is a fast-aircraft tool. It is a mistake on a
        slow aircraft.

IF I INCREASE DIHEDRAL (wing tips angled up)
---------------------------------------------
  GOOD: Roll stability - the aircraft levels itself after a gust.
  BAD:  Too much and it "spirals" - it slowly rolls into a turn and
        steepens. Also reduces the natural damping of Dutch roll.
  REAL:  3-7 degrees is typical. Airliners use more because the
        engines hang below the wing.

IF I REDUCE TAPER (pointier tips)
----------------------------------
  GOOD: Less weight at the tips, lower bending moment, often less
        induced drag.
  BAD:  The tip reaches stall angle first. On a sharply tapered wing
        the stall can be violent and asymmetric.
  REAL:  This is exactly why washout (tip twist) exists.

IF I ADD MORE TWIST (washout)
------------------------------
  GOOD: The root stalls before the tips, so the ailerons keep working
        and the aircraft stays controllable near stall.
  BAD:  Slight loss of lift at the tip, so slightly higher stall speed
        overall.
  REAL:  Almost always worth it. 2-4 degrees is normal.

IF I MOVE THE CENTRE OF GRAVITY BACKWARDS
------------------------------------------
  GOOD: Less trim drag, slightly better performance, more responsive.
  BAD:  Static stability drops. Past a point the aircraft pitches up
        on its own and cannot be recovered. This has destroyed aircraft.
  REAL:  The CG range is a hard limit, not a preference. Always check.

IF I MAKE THE TAIL SMALLER
---------------------------
  GOOD: Less wetted area, less drag, less weight, prettier.
  BAD:  Less pitch and yaw control. The aircraft may not be able to
        recover from a stall or a spin. Certification rules require a
        minimum tail size.
  REAL:  Do not do this on a first design.

IF I ADD A BIGGER ENGINE
-------------------------
  GOOD: More thrust: better climb, shorter runway, higher top speed.
  BAD:  Heavier, burns more fuel, bigger nacelle adds drag, and the
        engine may be far too much for the airframe to use well.
  REAL:  Excess power shows up in climb rate, not usually in top speed.

IF I INCREASE WING LOADING (more weight, same wing)
----------------------------------------------------
  GOOD: Faster for the same power, less bumpy ride in turbulence.
  BAD:  Higher stall speed, longer runway, worse turning, harder landings.
  REAL:  A Cessna has W/S around 600 N/m2. A fighter has 4000+.

IF NOTHING SEEMS TO BE ENOUGH
------------------------------
  Then the problem is usually that you are asking for two contradictory
  things at once: long range AND short runway, or high speed AND long
  endurance. Pick which one the aircraft is FOR, optimise that, accept
  the cost elsewhere. That decision - not any parameter value - is
  aircraft design.
)QASP"
},
//----------------------------------------------------------------------------
{
"Analysis Tools - What Each One Is For",
R"QASP(THE ANALYSIS TOOLS, AND WHEN TO USE THEM
==========================================

OpenVSP contains several separate analysis tools. They are not
interchangeable and they do NOT all run automatically. Here is what
each one actually does, in the order you should normally use them.


1.  COMPGEOM  (Geometry > CompGeom)
    WHAT IT DOES: Measures your model. Wetted area, volume, and the
      reference areas and lengths that everything else needs. Also
      produces the triangulated surface mesh.
    WHEN:  FIRST. Always first. Most other tools will give wrong or
      meaningless answers if your geometry has not been processed.
    WATCH OUT: If it reports errors, your surfaces are self-intersecting
      or degenerate. Fix the model before trusting any number.

2.  MASS PROPERTIES  (Analysis > Mass Properties)
    WHAT IT DOES: Adds up component masses, and computes total mass,
      centre of gravity and moments of inertia.
    WHEN:  After the geometry exists. You MUST enter masses - the tool
      cannot invent them.
    WHY IT MATTERS: The CG location decides whether the aircraft is
      stable at all. A technically beautiful wing with the CG in the
      wrong place is a crash.
    WATCH OUT: Masses entered as "mass per unit area" scale with the
      geometry. If you resize the wing, re-check the total.

3.  PARASITE DRAG  (Analysis > Parasite Drag)
    WHAT IT DOES: Builds up drag from every component's wetted area,
      form factor, interference and surface roughness, plus compressibility
      effects. Outputs CD0 and a breakdown per component.
    WHEN:  Once CompGeom is done. This is the standard preliminary drag
      method used across the industry.
    HOW TO READ IT: Look at the BREAKDOWN, not just the total. Whichever
      component contributes most is where any effort should go.
    WATCH OUT: CD0 is meaningless without the reference area it was
      computed against. Always quote the two together.

4.  WAVE DRAG  (Analysis > Wave Drag)
    WHAT IT DOES: Studies your aircraft's cross-sectional area
      distribution along its length, and estimates wave drag for
      transonic and supersonic flight. Implements area ruling.
    WHEN:  Only relevant if you are flying near or above the speed of
      sound. Useless for a light aircraft.
    HOW TO READ IT: A smooth area distribution (no sudden jumps) gives
      lower wave drag. That is the whole idea behind the "Coke bottle"
      fuselage shape.

5.  VSPAERO  (Analysis > VSPAERO)
    WHAT IT DOES: A vortex-lattice plus panel method. Computes lift,
      drag, and pitching moment across angles of attack and sideslip,
      and gives stability derivatives. Can produce drag polars and
      a full L/D curve.
    WHEN:  When you want to know how the aircraft will actually behave
      in the air - top speed, glide ratio, stability.
    HOW TO READ IT: The L/D curve is the key output. Its peak is your
      best-glide ratio and, for propeller aircraft, roughly your
      best-range condition.
    WATCH OUT: VSPAERO is an inviscid method. It predicts induced drag
      well but underestimates parasite drag - pair it with Parasite
      Drag. It also does not predict stall accurately. Do not expect a
      trustworthy CL_max from it.

6.  BEM / PROPELLER  (Analysis > Propeller)
    WHAT IT DOES: Blade element momentum analysis of a propeller.
      Gives thrust and power against advance ratio and RPM.
    WHEN:  Whenever you have a propeller and want thrust rather than
      just "an engine".
    WHY IT MATTERS: "Static thrust" on a spec sheet tells you almost
      nothing useful. Thrust falls off with speed, and the BEM model
      is how you find out by how much.

7.  CFD MESH  (Analysis > CFD Mesh)
    WHAT IT DOES: Exports a surface or volume mesh for an external
      CFD solver (Cart3D, TetGen and others). It does not solve the
      flow itself.
    WHEN:  When conceptual estimates are no longer enough and you need
      real viscous CFD.
    WATCH OUT: This is a serious step with serious consequences.
      CFD is only as good as the mesh and the boundary conditions.

8.  GEOMETRY ANALYSIS  (Analysis > Geometry Analysis)
    WHAT IT DOES: Measures and reports on the geometry itself - gaps,
      tangency, self-intersection, thin regions.
    WHEN:  Before meshing or exporting to CAD or CFD. Debugging is
      much harder downstream.

9.  AERO-STRUCT / STRUCTURES  (Analysis > Aero-Struct)
    WHAT IT DOES: Builds a beam/stick model of the structure and exports
      it to finite-element solvers such as NASTRAN or CalculiX.
    WHEN:  When you care whether the wing physically breaks.

10. HINGE / CONTROL SURFACE
    WHAT IT DOES: Analyses control surface hinge geometry - useful for
      sizing actuators and checking that a flap or aileron can actually
      be hinged in the space available.


THE HONEST SUMMARY
-------------------
  CompGeom      -> "what shape have I actually built?"
  Mass Props    -> "how heavy is it, and is it balanced?"
  Parasite Drag -> "how much drag does the shape cost me?"
  VSPAERO       -> "how does it behave in the air?"
  Wave Drag     -> "will it cope near Mach 1?"
  BEM           -> "what does the propeller really produce?"

All of these are CONCEPTUAL DESIGN estimates. They are the right tools
for exploring a design space quickly. They are not certification data,
and none of them replaces flight test.
)QASP"
},
//----------------------------------------------------------------------------
{
"Reading Your Results - What the Numbers Mean",
R"QASP(READING YOUR RESULTS
=======================

Analysis tools output numbers. Numbers without interpretation are
useless. This page tells you what each one is really saying.


"YOUR CD0 IS 0.028"  - IS THAT GOOD?
--------------------------------------
CD0 alone tells you nothing, because it is referenced to an area.
Always read it as: "CD0 = 0.028 based on wing area S = 16 m2".

Rough guide, based on wing area:
  0.015 - 0.020   very clean, e.g. a motorglider
  0.020 - 0.030   clean light aircraft
  0.030 - 0.040   typical light aircraft, fixed gear, struts
  0.040 - 0.060   draggy - exposed undercarriage, lots of antennas
  above 0.060     something is probably wrong, or deliberately draggy

If your CD0 looks far too high, the usual causes are:
  - Wetted area counted twice (overlapping components)
  - Interference factors left at defaults on badly matched joints
  - Undercarriage and antennas modelled as bare shapes


"YOUR L/D MAX IS 12"
---------------------
This says the aircraft can glide 12 metres forward for every 1 metre
of height lost, with the engine off.

  Gliders                     30 - 60
  Light aircraft              10 - 15
  Turboprop commuter          14 - 17
  Airliner                    16 - 20
  Supersonic fighter           6 - 10

Most efficient speed is at the peak of the L/D curve. This is also
close to your best-range speed and your minimum-power speed.


"YOUR STALL SPEED IS 57 KNOTS"
------------------------------
This is the slowest speed at which the wing still supports the aircraft.
Below it you fall, and the stall can drop a wing suddenly.

Rules of thumb:
  Stall speed <= 45 kt   very forgiving, short fields
  45 - 65 kt             typical light aircraft
  65 - 90 kt             fast single, small turboprop
  above 90 kt            needs long runways and training

The arithmetic behind it (you can do this on paper):

    V_stall = sqrt( 2 * W / (rho * S * CL_max) )

  where W = weight in newtons, rho = air density (1.225 kg/m3 at sea
  level), S = wing area in m2, CL_max = peak lift coefficient.

  Example: W = 12000 N, S = 16 m2, CL_max = 1.4
    2 x 12000              = 24000
    1.225 x 16 x 1.4       = 27.44
    24000 / 27.44          = 874.6
    sqrt(874.6)            = 29.6 m/s  =  57 knots

  Notice that doubling the weight raises stall speed by about 41%,
  because of the square root. Weight matters enormously.


"YOUR CENTRE OF GRAVITY IS AT 1.8 M"
-------------------------------------
Whether this is good depends entirely on where the wing is. Express
the CG as a fraction of wing chord measured from the leading edge:

    CG at 15-25% of chord    normal, and normally safe
    CG at 5-10% of chord     very nose-heavy: stable but draggy and hard
                             to flare on landing
    CG behind 35% of chord   dangerous - likely unstable in pitch

If the CG is in the wrong place, move mass. Do not redesign the wing
to chase it - that is repairing the wrong problem.


"YOUR WETTED AREA IS 42 m2"
-----------------------------
This is the total skin touching the air. It is the biggest single
driver of parasite drag. Wetted area is roughly 3-5 times the wing
area for a typical aircraft.

Ways to reduce it: bury the engine, shorten the fuselage, remove
external struts and antennas, use fewer and bigger nacelles instead
of many small ones.


"THE AREA DISTRIBUTION HAS A BIG JUMP AT 40% LENGTH"
------------------------------------------------------
That is wave drag talking. A sudden change in cross-sectional area
along the aircraft (a step in the fuselage, a canopy, an external
store) creates a large wave-drag penalty near Mach 1.

The fix is area ruling: reshape the aircraft so the total area
distribution is smooth. This is why the F-16, Concorde and B-58
look pinched at the waist.


REMEMBER
---------
  - Every coefficient needs its reference area quoted with it.
  - Inviscid methods (VSPAERO) underestimate drag. Add parasite drag.
  - No method here predicts stall accurately. Treat CL_max as an
    input you chose, not an output you discovered.
  - These are conceptual design estimates. Use them to make decisions
    between configurations, not to promise performance.
)QASP"
},
//----------------------------------------------------------------------------
{
"How Fast and How High Can It Fly?",
R"QASP(HOW FAST, AND HOW HIGH?
===========================

This is the question everyone actually asks, and OpenVSP does not
answer it for you directly - it gives you the pieces. Here is how to
assemble them honestly, with arithmetic you can follow by hand.


WHAT YOU NEED FIRST
--------------------
From the tools:
    1. CD0            <- Parasite Drag
    2. S (wing area)  <- CompGeom
    3. W (weight)     <- Mass Properties
    4. L/D curve      <- VSPAERO
    5. Engine power or thrust <- from the engine data

The missing piece is always the engine. No amount of geometry analysis
tells you how much thrust a propeller makes - you supply that.


TOP SPEED (level flight)
-------------------------
At top speed, thrust exactly balances drag:

    T = D = 0.5 * rho * V^2 * S * CD

and drag coefficient includes the lift-dependent part:

    CD = CD0 + CL^2 / (pi * e * AR)

For a PROPELLER aircraft it is easier to think in POWER, because engine
power is roughly constant with speed:

    P_required = D * V
    Top speed occurs where P_available = P_required

So for a propeller aircraft:
  - more power raises top speed only slowly (speed goes roughly as the
    cube root of power)
  - drag reduction is usually the better lever

For a JET, thrust is roughly constant with speed instead, which is why
jets like high altitude: drag falls with air density, but thrust does not.


RATE OF CLIMB
--------------
Climb happens with SURPLUS POWER:

    Rate of climb = ( P_available - P_required ) / W

Example: a 100 kW engine, 70 kW needed to hold level flight, and a
mass of 550 kg (W = 5396 N):

    surplus = ( 100000 - 70000 ) / 5396  =  5.6 m/s
                                         =  about 1090 feet per minute

That is a healthy light-aircraft climb rate. Under 300 fpm feels
uncomfortably weak. A surplus of zero means it will not climb at all.


SERVICE CEILING
----------------
Here is the part people find surprising. A ceiling is not a hard
altitude limit - it is where the aircraft runs out of surplus.

As you go higher:
    - air density falls, so drag falls (good)
    - but engine power falls too, faster than drag does (bad)
    - and the wing needs a higher angle of attack for the same lift,
      so induced drag rises

Eventually surplus power falls to about zero and the aircraft stops
climbing. Conventionally, the SERVICE CEILING is the altitude at which
climb rate has fallen to 100 feet per minute (0.5 m/s).

A practical way to estimate it, which you can do with a spreadsheet:

    Repeat at each altitude h:
      rho(h)  = air density from the standard atmosphere
      V       = your chosen cruise speed (TAS roughly rises with altitude)
      CL      = 2W / (rho * V^2 * S)
      CD      = CD0 + CL^2 / (pi * e * AR)
      D       = 0.5 * rho * V^2 * S * CD
      P_req   = D * V
      P_avail = rated power x (power lapse at that altitude)
      climb   = (P_avail - P_req) / W
    The ceiling is the altitude where climb falls below 0.5 m/s.

Typical results for light propeller aircraft: 12,000 - 16,000 ft.
Unpressurised aircraft should not normally go above about 12,500 ft
without oxygen for the occupants - that is a human limit, not an
aerodynamic one, and it applies before the aerodynamic ceiling does.


A SANITY CHECK YOU SHOULD ALWAYS DO
------------------------------------
Take your answer and ask whether it makes sense for that class of
aircraft. If your two-seat light aircraft comes out with a 45,000 ft
ceiling, one of these is almost certainly wrong:

    - engine power entered in the wrong units (kW vs hp)
    - weight entered in kg instead of newtons
    - CD0 missing, or referenced to the wrong area
    - drag polars used without adding parasite drag
    - air density taken at sea level for an altitude calculation

Unit errors cause more bad aircraft estimates than physics does.
Changing kg to N alone is a factor of 9.81.


THE HONEST STATEMENT
---------------------
Based on the methods above you can say something like:

  "This configuration, with CD0 = 0.028 and 100 kW, has an estimated
   top speed of about 145 kt at 8,000 ft and a service ceiling near
   14,000 ft. It stalls at 57 kt."

That is a real, useful engineering statement - provided you say it is
a conceptual-design estimate, list the assumptions, and never present
it as guaranteed performance. The gap between a good estimate and a
certified number is flight testing, and it is expensive.
)QASP"
}
};

static const int g_NumTopics = ( int )( sizeof( g_Topics ) / sizeof( g_Topics[0] ) );

//============================================================================//
// Search / filtering
//============================================================================//

// Indices of topics currently listed in the browser, in display order.
static vector<int> g_Visible;

static string ToLower( const string& in )
{
    string out = in;

    for ( size_t i = 0 ; i < out.size() ; i++ )
    {
        out[i] = ( char )std::tolower( ( unsigned char )out[i] );
    }

    return out;
}

//============================================================================//
// Build the window
//============================================================================//

void QaspFieldGuide::BuildWindow()
{
    const int W = 980;
    const int H = 660;

    s_Window = new Fl_Window( W, H, "QASP Field Guide" );
    s_Window->color( QaspPaper() );
    s_Window->begin();

    //---- Header strip ----------------------------------------------------
    Fl_Group* header = new Fl_Group( 0, 0, W, 54 );
    header->box( FL_FLAT_BOX );
    header->color( QaspNavy() );

    Fl_Box* title = new Fl_Box( 18, 8, 620, 22, "QASP Field Guide" );
    title->labelfont( FL_HELVETICA_BOLD );
    title->labelsize( 18 );
    title->labelcolor( FL_WHITE );
    title->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE );

    Fl_Box* sub = new Fl_Box( 18, 30, 620, 18,
                              "Aircraft design explained without the jargon" );
    sub->labelsize( 12 );
    sub->labelcolor( fl_rgb_color( 170, 195, 225 ) );
    sub->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE );

    Fl_Button* closeBtn = new Fl_Button( W - 108, 13, 90, 28, "Close" );
    closeBtn->box( FL_FLAT_BOX );
    closeBtn->color( QaspAccent() );
    closeBtn->labelcolor( FL_WHITE );
    closeBtn->labelfont( FL_HELVETICA_BOLD );
    closeBtn->callback( CloseCB, s_Window );

    header->end();

    //---- Left: topic list ------------------------------------------------
    Fl_Group* left = new Fl_Group( 0, 54, 300, H - 54 );
    left->box( FL_FLAT_BOX );
    left->color( QaspPanel() );

    s_Search = new Fl_Input( 14, 68, 272, 28 );
    s_Search->textsize( 13 );
    s_Search->textcolor( QaspInk() );
    s_Search->when( FL_WHEN_CHANGED );
    s_Search->tooltip( "Type to filter the topics below" );
    s_Search->callback( SearchCB, nullptr );

    Fl_Box* searchLbl = new Fl_Box( 14, 100, 272, 16, "Search topics" );
    searchLbl->labelsize( 11 );
    searchLbl->labelcolor( QaspMuted() );
    searchLbl->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE );

    s_Topics = new Fl_Browser( 14, 120, 272, H - 190 );
    s_Topics->box( FL_FLAT_BOX );
    s_Topics->color( FL_WHITE );
    s_Topics->textsize( 13 );
    s_Topics->textfont( FL_HELVETICA );
    s_Topics->textcolor( QaspInk() );
    s_Topics->has_scrollbar( Fl_Browser::VERTICAL );
    s_Topics->callback( TopicCB, nullptr );

    Fl_Box* hint = new Fl_Box( 14, H - 62, 272, 46,
                               "Tip: hover over any control in the main window\n"
                               "to see a short explanation of it." );
    hint->labelsize( 11 );
    hint->labelcolor( QaspMuted() );
    hint->align( FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_WRAP | FL_ALIGN_INSIDE );

    left->end();

    //---- Right: article --------------------------------------------------
    s_Buffer = new Fl_Text_Buffer();

    s_Text = new Fl_Text_Display( 300, 54, W - 300, H - 54 );
    s_Text->box( FL_FLAT_BOX );
    s_Text->color( FL_WHITE );
    s_Text->textcolor( QaspInk() );
    s_Text->textsize( 14 );
    s_Text->textfont( FL_HELVETICA );
    s_Text->buffer( s_Buffer );
    s_Text->wrap_mode( Fl_Text_Display::WRAP_AT_BOUNDS, 0 );
    s_Text->scrollbar_width( 14 );

    s_Window->end();
    s_Window->resizable( s_Text );

    //---- Populate --------------------------------------------------------
    for ( int i = 0 ; i < g_NumTopics ; i++ )
    {
        s_Topics->add( g_Topics[i].title );
        g_Visible.push_back( i );
    }

    s_Topics->select( 1 );
    LoadTopic( 0 );
}

//============================================================================//
// Show
//============================================================================//

void QaspFieldGuide::Show()
{
    if ( !s_Window )
    {
        BuildWindow();
    }

    s_Window->show();

    // Raise to the front if it was already open behind something.
    if ( s_Window->visible() )
    {
        s_Window->show();
    }
}

//============================================================================//
// Select a topic
//============================================================================//

void QaspFieldGuide::LoadTopic( int index )
{
    if ( !s_Buffer || index < 0 || index >= g_NumTopics )
    {
        return;
    }

    s_Current = index;
    s_Buffer->text( g_Topics[index].body );

    if ( s_Text )
    {
        s_Text->scroll( 0, 0 );
    }
}

//============================================================================//
// Filter topics by the search box
//============================================================================//

void QaspFieldGuide::FilterTopics()
{
    if ( !s_Topics || !s_Search )
    {
        return;
    }

    string needle = ToLower( string( s_Search->value() ? s_Search->value() : "" ) );

    s_Topics->clear();
    g_Visible.clear();

    for ( int i = 0 ; i < g_NumTopics ; i++ )
    {
        bool match = needle.empty();

        if ( !match )
        {
            string hay = ToLower( string( g_Topics[i].title ) + " " + g_Topics[i].body );

            if ( hay.find( needle ) != string::npos )
            {
                match = true;
            }
        }

        if ( match )
        {
            s_Topics->add( g_Topics[i].title );
            g_Visible.push_back( i );
        }
    }

    if ( !g_Visible.empty() )
    {
        s_Topics->select( 1 );
        LoadTopic( g_Visible[0] );
    }
    else
    {
        s_Buffer->text( "\n\n   No topic matches that search.\n\n"
                        "   Try a shorter word, for example: stall, drag, span, ceiling.\n" );
        s_Current = -1;
    }
}

//============================================================================//
// Callbacks
//============================================================================//

void QaspFieldGuide::TopicCB( Fl_Widget* w, void* )
{
    Fl_Browser* b = ( Fl_Browser* )w;
    int row = b->value();   // 1-based

    if ( row >= 1 && row <= ( int )g_Visible.size() )
    {
        LoadTopic( g_Visible[row - 1] );
    }
}

void QaspFieldGuide::SearchCB( Fl_Widget*, void* )
{
    FilterTopics();
}

void QaspFieldGuide::CloseCB( Fl_Widget*, void* data )
{
    Fl_Window* win = ( Fl_Window* )data;

    if ( win )
    {
        win->hide();
    }
}
