# MATLAB scripts for the Human's original data

These are the MATLAB scripts that made `../MaleGeomData.h` and `../FemaleGeomData.h`, the original 200 principal
component data of `HumanGeom`, from the University of Michigan (UMTRI) HumanShape body models.  They are kept as a
record of how the data was made; nothing in the OpenVSP build runs them.

The scripts read the UMTRI model files by relative paths under `Parametric Person/Michigan/`, from the directories
`Male_800Subj_200PC_4Var_2017_06_26` and `Female_800Subj_200PC_4Var_2017_06_26` (`mean_mesh.obj` or
`mean_mesh_tris.obj`, `mean_vector.tsv`, `pcs.tsv`, `regression_coeffs.tsv`).  Those files are not part of OpenVSP;
they were provided by Prof. Matthew P. Reed, University of Michigan, and OpenVSP uses the model with his permission.

| Script | Role |
| ---- | ---- |
| `process_human.m`, `process_human_female.m` | Make the half mesh and write `MaleGeomData.h` / `FemaleGeomData.h`.  The triangles are chosen on the average of the male and female mean meshes: the faces with y >= 0, coarsened with `reducepatch`, which reconnects existing vertices without moving any, so the principal components of the kept vertices still apply.  The skeleton joints are model landmarks, except the elbows (midway between the elbow landmark and a vertex in the crook of the elbow) and the eyes (two head vertices and their midpoint). |
| `landmarks.m`, `landmarks_female.m` | The names and mean positions of the model's landmarks, and the skeleton chosen from them; plots them for picking and writes `skel_pinnochio.out`. |
| `process_skel_pinnochio.m` | An earlier skeleton for Pinocchio, written to `skel_pinnochio.out`. |
| `skel.m` | Plots a skeleton from its joint table. |
| `plotweights.m` | Plots Pinocchio's bone weights (`attachment.out`) on `debug.obj`, as an earlier `HumanGeom` wrote them. |
| `fit_height.m` | The distribution of male stature from the 2012 ANSUR II survey (NATICK/TR-15/007). |

The scripts also call `read_vertices_and_faces_from_obj_file.m` to read an OBJ file, and `landmarks.m` calls
`clickA3DPoint.m` (with `callbackClickA3DPoint.m` and `rowNorm.m`) to pick points in a plot.  Those are other
authors' utilities and are not included here.
