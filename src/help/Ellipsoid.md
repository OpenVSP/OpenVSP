---
title:  'Ellipsoid'
---

The Ellipsoid is a closed body with three radii, one along each axis.  With all three equal it is a sphere; with two
equal, a spheroid such as a radome, a tank end or a blister.  It has one tab of its own, Design.  The [Gen](GenTab.md),
[XForm](XFormTab.md), [Mass](MassTab.md) and [Sub](SubTab.md) tabs are described on their own pages.

## Quick Start

1. Add an Ellipsoid.  A new one is a sphere of radius 1.
2. Place it with the XForm tab.
3. Set *A Radius*, *B Radius* and *C Radius*.

## Design

| Control  | Parm       | Default | Meaning                    |
|:---------|:-----------|:--------|:---------------------------|
| A Radius | `A_Radius` | 1       | Semi-axis along $X$, $a$.  |
| B Radius | `B_Radius` | 1       | Semi-axis along $Y$, $b$.  |
| C Radius | `C_Radius` | 1       | Semi-axis along $Z$, $c$.  |

The surface is

$$
\left(\frac{x - a}{a}\right)^2 + \left(\frac{y}{b}\right)^2 + \left(\frac{z}{c}\right)^2 = 1
$$

so the Ellipsoid's nose, not its center, is at its origin, and it runs from $X = 0$ to $X = 2a$.  Changing *A Radius*
keeps the nose in place and moves the center.  *Rot Origin(X)* on the XForm tab is a fraction of the length $2a$, so 0.5
turns the Ellipsoid about its center.

The surface follows this equation closely but not exactly: every point lies within 0.04 percent of the radius in its
direction from the true ellipsoid.  A radius may be zero, which flattens the Ellipsoid into an elliptical
disk.
