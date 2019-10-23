/*
Copyright (c) 2007 Ilya Baran

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "Model.h"

#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/utils.h"
#include "../Pinocchio/debugging.h"
#include "../Pinocchio/attachment.h"
#include "../Pinocchio/pin_mesh.h"
#include "../Pinocchio/pinocchioApi.h"

using namespace Pinocchio;

#include "DefMesh.h"
#include "Motion.h"

#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <string>
#include <fstream>

#include <GL/gl.h>

#include "cube_model.h"

// This will generate a number from LO to HI, inclusive.
#define RANDOM_FLOAT(LO,HI) (static_cast <float> (LO) + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(static_cast <float> (HI) - static_cast <float> (LO)))))

struct SkelHuman : public Skeleton {
    SkelHuman() {
        // Order of makeJoint calls is very important
        makeJoint("shoulders",  Vector3( 0.00,  0.50,  0.00));                // 0
        makeJoint("back",       Vector3( 0.00,  0.15,  0.00),  "shoulders");  // 1
        makeJoint("hips",       Vector3( 0.00,  0.00,  0.00),  "back");       // 2
        makeJoint("head",       Vector3( 0.00,  0.70,  0.00),  "shoulders");  // 3

        makeJoint("lthigh",     Vector3(-0.10,  0.00,  0.00),  "hips");       // 4
        makeJoint("lknee",      Vector3(-0.15, -0.35,  0.00),  "lthigh");     // 5
        makeJoint("lankle",     Vector3(-0.15, -0.80,  0.00),  "lknee");      // 6
        makeJoint("lfoot",      Vector3(-0.15, -0.80,  0.10),  "lankle");     // 7

        makeJoint("rthigh",     Vector3( 0.10,  0.00,  0.00),  "hips");       // 8
        makeJoint("rknee",      Vector3( 0.15, -0.35,  0.00),  "rthigh");     // 9
        makeJoint("rankle",     Vector3( 0.15, -0.80,  0.00),  "rknee");      // 10
        makeJoint("rfoot",      Vector3( 0.15, -0.80,  0.10),  "rankle");     // 11

        makeJoint("lshoulder",  Vector3(-0.20,  0.50,  0.00),  "shoulders");  // 12
        makeJoint("lelbow",     Vector3(-0.40,  0.25,  0.075), "lshoulder");  // 13
        makeJoint("lhand",      Vector3(-0.60,  0.00,  0.15),  "lelbow");     // 14

        makeJoint("rshoulder",  Vector3( 0.20,  0.50,  0.00),  "shoulders");  // 15
        makeJoint("relbow",     Vector3( 0.40,  0.25,  0.075), "rshoulder");  // 16
        makeJoint("rhand",      Vector3( 0.60,  0.00,  0.15),  "relbow");     // 17

        // Symmetry
        makeSymmetric("lthigh", "rthigh");
        makeSymmetric("lknee", "rknee");
        makeSymmetric("lankle", "rankle");
        makeSymmetric("lfoot", "rfoot");

        makeSymmetric("lshoulder", "rshoulder");
        makeSymmetric("lelbow", "relbow");
        makeSymmetric("lhand", "rhand");

        initCompressed();

        setFoot("lfoot");
        setFoot("rfoot");

        setFat("hips");
        setFat("shoulders");
        setFat("head");
    }
};

struct SkelQuad : public Skeleton {
    SkelQuad() {
        // Order of makeJoint calls is very important
        makeJoint("shoulders",  Vector3( 0.00,  0.00,  0.50));
        makeJoint("back",       Vector3( 0.00,  0.00,  0.00),  "shoulders");
        makeJoint("hips",       Vector3( 0.00,  0.00, -0.50),  "back");
        makeJoint("neck",       Vector3( 0.00,  0.20,  0.63),  "shoulders");
        makeJoint("head",       Vector3( 0.00,  0.20,  0.90),  "neck");

        makeJoint("lthigh",     Vector3(-0.15,  0.00, -0.50),  "hips");
        makeJoint("lhknee",     Vector3(-0.20, -0.40, -0.50),  "lthigh");
        makeJoint("lhfoot",     Vector3(-0.20, -0.80, -0.50),  "lhknee");

        makeJoint("rthigh",     Vector3( 0.15,  0.00, -0.50),  "hips");
        makeJoint("rhknee",     Vector3( 0.20, -0.40, -0.50),  "rthigh");
        makeJoint("rhfoot",     Vector3( 0.20, -0.80, -0.50),  "rhknee");

        makeJoint("lshoulder",  Vector3(-0.20,  0.00,  0.50),  "shoulders");
        makeJoint("lfknee",     Vector3(-0.20, -0.40,  0.50),  "lshoulder");
        makeJoint("lffoot",     Vector3(-0.20, -0.80,  0.50),  "lfknee");

        makeJoint("rshoulder",  Vector3( 0.20,  0.00,  0.50),  "shoulders");
        makeJoint("rfknee",     Vector3( 0.20, -0.40,  0.50),  "rshoulder");
        makeJoint("rffoot",     Vector3( 0.20, -0.80,  0.50),  "rfknee");

        makeJoint("tail",       Vector3( 0.00,  0.00, -0.70),  "hips");

        // Symmetry
        makeSymmetric("lthigh", "rthigh");
        makeSymmetric("lhknee", "rhknee");
        makeSymmetric("lhfoot", "rhfoot");

        makeSymmetric("lshoulder", "rshoulder");
        makeSymmetric("lfknee", "rfknee");
        makeSymmetric("lffoot", "rffoot");

        initCompressed();

        setFoot("lhfoot");
        setFoot("rhfoot");
        setFoot("lffoot");
        setFoot("rffoot");

        setFat("hips");
        setFat("shoulders");
        setFat("head");
    }
};

struct SkelHorse : public Skeleton {
    SkelHorse() {
        // Order of makeJoint calls is very important
        makeJoint("shoulders",  Vector3( 0.00,  0.00,  0.50));
        makeJoint("back",       Vector3( 0.00,  0.00,  0.00),  "shoulders");
        makeJoint("hips",       Vector3( 0.00,  0.00, -0.50),  "back");
        makeJoint("neck",       Vector3( 0.00,  0.20,  0.63),  "shoulders");
        makeJoint("head",       Vector3( 0.00,  0.20,  0.90),  "neck");

        makeJoint("lthigh",     Vector3(-0.15,  0.00, -0.50),  "hips");
        makeJoint("lhknee",     Vector3(-0.20, -0.20, -0.45),  "lthigh");
        makeJoint("lhheel",     Vector3(-0.20, -0.40, -0.50),  "lhknee");
        makeJoint("lhfoot",     Vector3(-0.20, -0.80, -0.50),  "lhheel");

        makeJoint("rthigh",     Vector3( 0.15,  0.00, -0.50),  "hips");
        makeJoint("rhknee",     Vector3( 0.20, -0.20, -0.45),  "rthigh");
        makeJoint("rhheel",     Vector3( 0.20, -0.40, -0.50),  "rhknee");
        makeJoint("rhfoot",     Vector3( 0.20, -0.80, -0.50),  "rhheel");

        makeJoint("lshoulder",  Vector3(-0.20,  0.00,  0.50),  "shoulders");
        makeJoint("lfknee",     Vector3(-0.20, -0.40,  0.50),  "lshoulder");
        makeJoint("lffoot",     Vector3(-0.20, -0.80,  0.50),  "lfknee");

        makeJoint("rshoulder",  Vector3( 0.20,  0.00,  0.50),  "shoulders");
        makeJoint("rfknee",     Vector3( 0.20, -0.40,  0.50),  "rshoulder");
        makeJoint("rffoot",     Vector3( 0.20, -0.80,  0.50),  "rfknee");

        makeJoint("tail",       Vector3( 0.00,  0.00, -0.70),  "hips");

        // Symmetry
        makeSymmetric("lthigh", "rthigh");
        makeSymmetric("lhknee", "rhknee");
        makeSymmetric("lhheel", "rhheel");
        makeSymmetric("lhfoot", "rhfoot");

        makeSymmetric("lshoulder", "rshoulder");
        makeSymmetric("lfknee", "rfknee");
        makeSymmetric("lffoot", "rffoot");

        initCompressed();

        setFoot("lhfoot");
        setFoot("rhfoot");
        setFoot("lffoot");
        setFoot("rffoot");

        setFat("hips");
        setFat("shoulders");
        setFat("head");
    }
};

struct SkelCentaur : public Skeleton {
    SkelCentaur() {
        // Order of makeJoint calls is very important
        makeJoint("shoulders",  Vector3( 0.00,  0.00,  0.50));                // 0
        makeJoint("back",       Vector3( 0.00,  0.00,  0.00),  "shoulders");  // 1
        makeJoint("hips",       Vector3( 0.00,  0.00, -0.50),  "back");       // 2

        makeJoint("hback",      Vector3( 0.00,  0.25,  0.50),  "shoulders");  // 3
        makeJoint("hshoulders", Vector3( 0.00,  0.50,  0.50),  "hback");      // 4
        makeJoint("head",       Vector3( 0.00,  0.70,  0.50),  "hshoulders"); // 5

        makeJoint("lthigh",     Vector3(-0.15,  0.00, -0.50),  "hips");       // 6
        makeJoint("lhknee",     Vector3(-0.20, -0.40, -0.45),  "lthigh");     // 7
        makeJoint("lhfoot",     Vector3(-0.20, -0.80, -0.50),  "lhknee");     // 8

        makeJoint("rthigh",     Vector3( 0.15,  0.00, -0.50),  "hips");       // 9
        makeJoint("rhknee",     Vector3( 0.20, -0.40, -0.45),  "rthigh");     // 10
        makeJoint("rhfoot",     Vector3( 0.20, -0.80, -0.50),  "rhknee");     // 11

        makeJoint("lshoulder",  Vector3(-0.20,  0.00,  0.50),  "shoulders");  // 12
        makeJoint("lfknee",     Vector3(-0.20, -0.40,  0.50),  "lshoulder");  // 13
        makeJoint("lffoot",     Vector3(-0.20, -0.80,  0.50),  "lfknee");     // 14

        makeJoint("rshoulder",  Vector3( 0.20,  0.00,  0.50),  "shoulders");  // 15
        makeJoint("rfknee",     Vector3( 0.20, -0.40,  0.50),  "rshoulder");  // 16
        makeJoint("rffoot",     Vector3( 0.20, -0.80,  0.50),  "rfknee");     // 17

        makeJoint("hlshoulder", Vector3(-0.20,  0.50,  0.50),  "hshoulders"); // 18
        makeJoint("lelbow",     Vector3(-0.40,  0.25,  0.575), "hlshoulder"); // 19
        makeJoint("lhand",      Vector3(-0.60,  0.00,  0.65),  "lelbow");     // 20

        makeJoint("hrshoulder", Vector3( 0.20,  0.50,  0.50),  "hshoulders"); // 21
        makeJoint("relbow",     Vector3( 0.40,  0.25,  0.575), "hrshoulder"); // 22
        makeJoint("rhand",      Vector3( 0.60,  0.00,  0.65),  "relbow");     // 23

        makeJoint("tail",       Vector3( 0.00,  0.00, -0.7),   "hips");       // 24

        // Symmetry
        makeSymmetric("lthigh", "rthigh");
        makeSymmetric("lhknee", "rhknee");
        makeSymmetric("lhheel", "rhheel");
        makeSymmetric("lhfoot", "rhfoot");

        makeSymmetric("lshoulder", "rshoulder");
        makeSymmetric("lfknee", "rfknee");
        makeSymmetric("lffoot", "rffoot");

        makeSymmetric("hlshoulder", "hrshoulder");
        makeSymmetric("lelbow", "relbow");
        makeSymmetric("lhand", "rhand");

        initCompressed();

        setFoot("lhfoot");
        setFoot("rhfoot");
        setFoot("lffoot");
        setFoot("rffoot");

        setFat("hips");
        setFat("shoulders");
        setFat("hshoulders");
        setFat("head");
    }
};

void AnimatedModel::loadObject(std::string obj_filename, std::string motion_filenamename) {
    // Calculate Skeleton and Attachment Values with Pinocchio
    Mesh m(obj_filename, Mesh::DQS);
    Quaternion<> meshTransform;
    double skelScale = 1.;
    double stiffness = 1.;
    bool fitSkeleton = true;
    assert (m.vertices.size() != 0);

    for (int i = 0; i < (int)m.vertices.size(); ++i) {
        m.vertices[i].pos = meshTransform * m.vertices[i].pos;
    }
    m.normalizeBoundingBox();
    m.computeVertexNormals();

    Skeleton skeleton;
    skeleton = SkelHuman();

    Skeleton given = skeleton;
    given.scale(skelScale * 0.7);
    PinocchioOutput o;
    if (fitSkeleton) {
        o = autorig(given, m);
    } else { // Skip the fitting step--assume the skeleton is already correct for the mesh
        TreeType *distanceField = constructDistanceField(m);
        VisTester<TreeType> *tester = new VisTester<TreeType>(distanceField);
        o.embedding = skeleton.fGraph().verts;
        for (int i = 0; i < (int)o.embedding.size(); ++i) {
            o.embedding[i] = m.toAdd + o.embedding[i] * m.scale;
        }
        o.attachment = new Attachment(m, skeleton, o.embedding, tester, stiffness);
        delete tester;
        delete distanceField;
    }
    assert (o.embedding.size() != 0);

    if(motion_filenamename.size() > 0) {
        addMesh(new DefMesh(m, given, o.embedding, *(o.attachment), new Motion(motion_filenamename)));
    } else {
        addMesh(new StaticDisplayMesh(m));
        for(int i = 1; i < (int)o.embedding.size(); ++i) {
            addLine(LineSegment(o.embedding[i], o.embedding[given.fPrev()[i]], Vector3(.5, .5, 0), 4.));
        }
    }

    // Output skeleton embedding
    std::string skelOutName("skeleton.out");
    for (int i = 0; i < (int)o.embedding.size(); ++i) {
        o.embedding[i] = (o.embedding[i] - m.toAdd) / m.scale;
    }
    std::ofstream os(skelOutName.c_str());
    for (int i = 0; i < (int)o.embedding.size(); ++i) {
        os << i << " " << o.embedding[i][0] << " " << o.embedding[i][1] <<
            " " << o.embedding[i][2] << " " << skeleton.fPrev()[i] << std::endl;
    }

    // Output attachment
    std::string weightOutName("attachment.out");
    std::ofstream astrm(weightOutName.c_str());
    for (int i = 0; i < (int)m.vertices.size(); ++i) {
        Vector<double, -1> v = o.attachment->getWeights(i);
        for (int j = 0; j < v.size(); ++j) {
            double d = ::floor(0.5 + v[j] * 10000.0) / 10000.0;
            astrm << d << " ";
        }
        astrm << std::endl;
    }
    delete o.attachment;

    //printf("Mesh vertices: ");
    //for (std::vector<MeshVertex>::iterator it = m.vertices.begin() ; it != m.vertices.end(); ++it) {
    //    printf("(%f, %f, %f) ", it->pos[0], it->pos[1], it->pos[2]);
    //}
    //printf("\n");

    //printf("Mesh normals: ");
    //for (std::vector<MeshVertex>::iterator it = m.vertices.begin() ; it != m.vertices.end(); ++it) {
    //    printf("(%f, %f, %f) ", it->normal[0], it->normal[1], it->normal[2]);
    //}
    //printf("\n");

    if (vp) { free (vp); }
    if (vn) { free (vn); }
    if (vt) { free (vt); }
    if (bp) { free (bp); }

    point_count = 10000;
    bones_count = 100;

    // calloc is different than malloc in that it initializes the memory allocated.
    // With calloc, the memory is set to zero. With malloc, the memory is not cleared.
    vp = (GLfloat*)calloc(point_count*3, sizeof(vp[0]));
    vt = (GLfloat*)calloc(point_count*2, sizeof(vt[0]));
    vn = (GLfloat*)calloc(point_count*3, sizeof(vn[0]));
    bp = (GLfloat*)calloc(bones_count*6, sizeof(bp[0]));
}

void AnimatedModel::getModelShape(int * p_point_count, GLfloat const * p_vp[], GLfloat const * p_vn[], GLfloat const * p_vt[], int * p_bones_count, GLfloat const * p_bp[], const Vector3 trans) {
	static int framenum;
	SkelHuman human;

	//*p_point_count = cube_point_count;
	//*p_vp = cube_vp;
	//*p_vt = cube_vt;
	//*p_vn = cube_vn;

	std::vector<const Mesh *> model_mesh(meshes.size());
	for (int i = 0; i < (int)meshes.size(); ++i) {
		model_mesh[i] = &(meshes[i]->getMesh(framenum));
	}

	int i_point_count = 0;
	GLfloat * i_vp = vp;
	GLfloat * i_vt = vt;
	GLfloat * i_vn = vn;

	int i_bones_count = 0;
	GLfloat * i_bp = bp;

	for (int i = 0; i < (int)meshes.size(); ++i) {
		const Mesh &m = *(model_mesh[i]);
		Vector3 normal;

		for (int j = 0; j < (int)m.edges.size(); ++j) {
			int v = m.edges[j].vertex;
			const Vector3 &p = m.vertices[v].pos;
			normal = m.vertices[v].normal;
			*(i_vp++) = p[0] + trans[0];
			*(i_vp++) = p[1] + trans[1];
			*(i_vp++) = p[2] + trans[2];

			*(i_vn++) = normal[0];
			*(i_vn++) = normal[1];
			*(i_vn++) = normal[2];

			*(i_vt++) = 1.0;
			*(i_vt++) = 0.0;

			i_point_count++;
		}

		std::vector<Vector3> v = meshes[i]->getSkel();
		if (v.size() != 0) {
			const std::vector<int> &prev = human.fPrev();
			for (int j = 1; j < (int)prev.size(); ++j) {
				int k = prev[j];

				*(i_bp++) = v[j][0];
				*(i_bp++) = v[j][1];
				*(i_bp++) = v[j][2];

				*(i_bp++) = v[k][0];
				*(i_bp++) = v[k][1];
				*(i_bp++) = v[k][2];

				i_bones_count++;
			}
		}
	}

	*p_point_count = i_point_count;
	*p_vp = vp;
	*p_vt = vt;
	*p_vn = vn;

	*p_bones_count = i_bones_count;
	*p_bp = bp;
}
