# MODIFICATIONS — QASP

**QASP** is an **unofficial fork** of [OpenVSP](https://github.com/OpenVSP/openvsp),
which is developed by NASA and released under the
**NASA Open Source Agreement (NOSA) version 1.3**. See `LICENSE`.

QASP is **not** produced, endorsed, sponsored, or supported by NASA, nor by the
OpenVSP development team. Do not report QASP bugs to them.

Baseline: OpenVSP **3.56.0** (upstream commit `ca4958f2`).

---

## Summary of changes from upstream

### 1. Rebranding / product identity
| File | Change |
|---|---|
| `src/vsp/main.h.in` | `VSPVERSION1/2/3` display strings changed from "OpenVSP"/"Vehicle Sketch Pad" to "QASP : Aircraft Design Studio". `VSPVERSION4` **intentionally unchanged** — it is returned by the public `GetVSPVersion()` API and embedded in CAD/DXF/NASTRAN export metadata, so altering it would change machine-readable output. |
| `src/CMakeLists.txt` | `CPACK_PACKAGE_NAME` → `QASP`; package description and homepage updated. |

### 2. Build & packaging
| File | Change |
|---|---|
| `.github/workflows/qasp-release.yml` | New CI pipeline (not present upstream): builds a portable Windows x64 package and publishes it to GitHub Releases. |

---

## Retained from upstream, unmodified

All NASA/NOSA copyright headers, the `LICENSE` file, and all third-party
licenses and notices are retained in full. QASP claims no ownership over the
upstream work.

## Third-party components

OpenVSP bundles or links numerous third-party libraries (FLTK, libxml2, Eigen3,
Clipper2, AngelScript, cminpack, delabella, GLM, GLEW, gl2ps, Code-Eli, libIGES,
OpenABF, exprparse and others). Their licenses still apply. See `LICENSE` and
the individual source directories under `src/external/`.

---

## Trademark note

"NASA", the NASA logo, "OpenVSP" and "Vehicle Sketch Pad" are used only to
describe origin and lineage, as required for attribution. No endorsement is
implied.
