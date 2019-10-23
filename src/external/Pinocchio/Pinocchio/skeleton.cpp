/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* 06.11.2014 RR: * makeJoint(), if defined PINOCCHIO_SKEL_XYZ_ROT
                    rotate the positon componentes for the joints.
                    X Y Z  -->  Z Y X
*/

#include "skeleton.h"
#include "utils.h"
#include "debugging.h"
#include <fstream>
#include <unordered_map>

namespace Pinocchio {

void Skeleton::initCompressed() {
  fcMapV.resize(fPrevV.size(), -1);
  fcFractionV.resize(fPrevV.size(), -1.);

  for (int i = 0; i < (int)fPrevV.size(); ++i) {
    if(fGraphV.edges[i].size() == 2) {
      continue;
    }
    fcMapV[i] = cfMapV.size();
    cfMapV.push_back(i);
  }

  cPrevV.resize(cfMapV.size(), -1);
  cSymV.resize(cfMapV.size(), -1);
  cGraphV.edges.resize(cfMapV.size());
  cFeetV = std::vector<bool>(cPrevV.size(), false);
  cFatV = std::vector<bool>(cPrevV.size(), false);

  for (int i = 0; i < (int)cfMapV.size(); ++i) {
    cGraphV.verts.push_back(fGraphV.verts[cfMapV[i]]);

    //symmetry--TODO: need to make sure all unreduced bones in chain
    //          are marked as symmetric before marking the reduced one
    if (fSymV[cfMapV[i]] >= 0) {
      cSymV[i] = fcMapV[fSymV[cfMapV[i]]];
    }

    // Prev
    if (i > 0) {
      int curPrev = fPrevV[cfMapV[i]];
      while(fcMapV[curPrev]  < 0) {
        curPrev = fPrevV[curPrev];
      }
      cPrevV[i] = fcMapV[curPrev];
    }
  }

  // Graph edges
  for (int i = 1; i < (int)cPrevV.size(); ++i) {
    cGraphV.edges[i].push_back(cPrevV[i]);
    cGraphV.edges[cPrevV[i]].push_back(i);
  }

  cLengthV.resize(cPrevV.size(), 0.);

  //lengths/fraction computation
  for (int i = 1; i < (int)cPrevV.size(); ++i){
    int cur = cfMapV[i];
    std::unordered_map<int, double> lengths;
    do {
      lengths[cur] = (fGraphV.verts[cur] - fGraphV.verts[fPrevV[cur]]).length();
      cLengthV[i] += lengths[cur];
      cur = fPrevV[cur];
    } while(fcMapV[cur] == -1);

    for(std::unordered_map<int, double>::iterator it = lengths.begin(); it != lengths.end(); ++it)
      fcFractionV[it->first] = it->second / cLengthV[i];
  }
}


void Skeleton::scale(double factor) {
  for (int i = 0; i < (int)fGraphV.verts.size(); ++i) {
    fGraphV.verts[i] *= factor;
  }
  for (int i = 0; i < (int)cGraphV.verts.size(); ++i) {
    cGraphV.verts[i] *= factor;
    cLengthV[i] *= factor;
  }
}


void Skeleton::makeJoint(const std::string &name, const Vector3 &pos, const std::string &previous) {
  int cur = fSymV.size();
  fSymV.push_back(-1);

  #if PINOCCHIO_SKEL_XYZ_ROT
  //skeletons specified in [-1,1] will be fit to object in [0,1]
  fGraphV.verts.push_back(Vector3( pos[2], pos[0], pos[1]) * 0.5);
  #else
  //skeletons specified in [-1,1] will be fit to object in [0,1]
  fGraphV.verts.push_back(pos * 0.5);  // Do not understand * 0.5
  #endif

  fGraphV.edges.resize(cur + 1);
  jointNames[name] = cur;

  if(previous == std::string("")) {
    fPrevV.push_back(-1);
  } else { // Add a bone
    int prev = jointNames[previous];
    fGraphV.edges[cur].push_back(prev);
    fGraphV.edges[prev].push_back(cur);
    fPrevV.push_back(prev);
  }
}


void Skeleton::makeSymmetric(const std::string &name1, const std::string &name2) {
  int i1 = jointNames[name1];
  int i2 = jointNames[name2];

  if (i1 > i2) {
    std::swap(i1, i2);
  }
  fSymV[i2] = i1;
}

void Skeleton::makeSymmetric(const int &name1, const int &name2)
{
   makeSymmetric( std::to_string( name1 ), std::to_string( name2 ) );
}

void Skeleton::setFoot(const std::string &name) {
  int i = jointNames[name];
  cFeetV[fcMapV[i]] = true;
}

void Skeleton::setFoot(const int &name)
{
    setFoot( std::to_string( name ) );
}

void Skeleton::setFat(const std::string &name) {
  int i = jointNames[name];
  cFatV[fcMapV[i]] = true;
}

void Skeleton::setFat(const int &name)
{
    setFat( std::to_string( name ) );
}

//-----------------actual skeletons-------------------

HumanSkeleton::HumanSkeleton() {
    // Order of makeJoint calls is very important
    makeJoint("shoulders",  Vector3(0., 0.5, 0.));                          //0
    makeJoint("back",       Vector3(0., 0.15, 0.),      "shoulders");       //1
    makeJoint("hips",       Vector3(0., 0., 0.),        "back");            //2
    makeJoint("head",       Vector3(0., 0.7, 0.),       "shoulders");       //3

    makeJoint("lthigh",     Vector3(-0.1, 0., 0.),      "hips");            //4
    makeJoint("lknee",      Vector3(-0.15, -0.35, 0.),  "lthigh");          //5
    makeJoint("lankle",      Vector3(-0.15, -0.8, 0.),  "lknee");           //6
    makeJoint("lfoot",      Vector3(-0.15, -0.8, 0.1),  "lankle");          //7

    makeJoint("rthigh",     Vector3(0.1, 0., 0.),       "hips");            //8
    makeJoint("rknee",      Vector3(0.15, -0.35, 0.),   "rthigh");          //9
    makeJoint("rankle",      Vector3(0.15, -0.8, 0.),   "rknee");           //10
    makeJoint("rfoot",      Vector3(0.15, -0.8, 0.1),   "rankle");          //11

    makeJoint("lshoulder",  Vector3(-0.2, 0.5, 0.),     "shoulders");       //12
    makeJoint("lelbow",     Vector3(-0.4, 0.25, 0.075), "lshoulder");       //13
    makeJoint("lhand",      Vector3(-0.6, 0.0, 0.15),   "lelbow");          //14

    makeJoint("rshoulder",  Vector3(0.2, 0.5, 0.),      "shoulders");       //15
    makeJoint("relbow",     Vector3(0.4, 0.25, 0.075),  "rshoulder");       //16
    makeJoint("rhand",      Vector3(0.6, 0.0, 0.15),    "relbow");          //17

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

#ifdef USE_KINECSDK_SKEL
// 13.12.2014 RR: use kinecSDK order

HumanSkeletonKinect::HumanSkeletonKinect() {
    // Order of makeJoint calls is very important
    makeJoint("hips",       Vector3(0., 0., 0.));                           //0
    makeJoint("back",       Vector3(0., 0.15, 0.),      "hips");            //1
    makeJoint("shoulders",  Vector3(0., 0.5, 0.),       "back");            //2
    makeJoint("head",       Vector3(0., 0.7, 0.),       "shoulders");       //3

    makeJoint("lshoulder",  Vector3(-0.2, 0.5, 0.),     "shoulders");       //4
    makeJoint("lelbow",     Vector3(-0.4, 0.25, 0.075), "lshoulder");       //5
    makeJoint("lwrist",     Vector3(-0.6, 0.05, 0.15),  "lelbow");          //6
    makeJoint("lhand",      Vector3(-0.6, 0.0, 0.15),   "lwrist");          //7

    makeJoint("rshoulder",  Vector3(0.2, 0.5, 0.),      "shoulders");       //8
    makeJoint("relbow",     Vector3(0.4, 0.25, 0.075),  "rshoulder");       //9
    makeJoint("rwrist",     Vector3(0.6, 0.05, 0.15),   "relbow");          //10
    makeJoint("rhand",      Vector3(0.6, 0.00, 0.15),   "rwrist");          //11

    makeJoint("lthigh",     Vector3(-0.1, 0., 0.),      "hips");            //12
    makeJoint("lknee",      Vector3(-0.15, -0.35, 0.),  "lthigh");          //13
    makeJoint("lankle",     Vector3(-0.15, -0.8, 0.),   "lknee");           //14
    makeJoint("lfoot",      Vector3(-0.15, -0.8, 0.1),  "lankle");          //15

    makeJoint("rthigh",     Vector3(0.1, 0., 0.),       "hips");            //16
    makeJoint("rknee",      Vector3(0.15, -0.35, 0.),   "rthigh");          //17
    makeJoint("rankle",     Vector3(0.15, -0.8, 0.),    "rknee");           //18
    makeJoint("rfoot",      Vector3(0.15, -0.8, 0.1),   "rankle");          //19

    // Symmetry
    makeSymmetric("lshoulder", "rshoulder");
    makeSymmetric("lelbow", "relbow");
    makeSymmetric("lwrist", "rwrist");
    makeSymmetric("lhand", "rhand");

    makeSymmetric("lthigh", "rthigh");
    makeSymmetric("lknee", "rknee");
    makeSymmetric("lankle", "rankle");
    makeSymmetric("lfoot", "rfoot");

    initCompressed();

    setFoot("lfoot");
    setFoot("rfoot");

    setFat("hips");
    setFat("shoulders");
    setFat("head");
}

#endif

QuadSkeleton::QuadSkeleton() {
    // Order of makeJoint calls is very important
    makeJoint("shoulders",  Vector3(0., 0., 0.5));
    makeJoint("back",       Vector3(0., 0., 0.),         "shoulders");
    makeJoint("hips",       Vector3(0., 0., -0.5),       "back");
    makeJoint("neck",       Vector3(0., 0.2, 0.63),      "shoulders");
    makeJoint("head",       Vector3(0., 0.2, 0.9),       "neck");

    makeJoint("lthigh",     Vector3(-0.15, 0., -0.5),    "hips");
    makeJoint("lhknee",     Vector3(-0.2, -0.4, -0.5),   "lthigh");
    makeJoint("lhfoot",     Vector3(-0.2, -0.8, -0.5),   "lhknee");

    makeJoint("rthigh",     Vector3(0.15, 0., -0.5),     "hips");
    makeJoint("rhknee",     Vector3(0.2, -0.4, -0.5),    "rthigh");
    makeJoint("rhfoot",     Vector3(0.2, -0.8, -0.5),    "rhknee");

    makeJoint("lshoulder",  Vector3(-0.2, 0., 0.5),      "shoulders");
    makeJoint("lfknee",     Vector3(-0.2, -0.4, 0.5),    "lshoulder");
    makeJoint("lffoot",      Vector3(-0.2, -0.8, 0.5),   "lfknee");

    makeJoint("rshoulder",  Vector3(0.2, 0.0, 0.5),      "shoulders");
    makeJoint("rfknee",     Vector3(0.2, -0.4, 0.5),     "rshoulder");
    makeJoint("rffoot",      Vector3(0.2, -0.8, 0.5),    "rfknee");

    makeJoint("tail",       Vector3(0., 0., -0.7),       "hips");

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

HorseSkeleton::HorseSkeleton() {
    // Order of makeJoint calls is very important
    makeJoint("shoulders",  Vector3(0., 0., 0.5));
    makeJoint("back",       Vector3(0., 0., 0.),         "shoulders");
    makeJoint("hips",       Vector3(0., 0., -0.5),       "back");
    makeJoint("neck",       Vector3(0., 0.2, 0.63),      "shoulders");
    makeJoint("head",       Vector3(0., 0.2, 0.9),       "neck");

    makeJoint("lthigh",     Vector3(-0.15, 0., -0.5),     "hips");
    makeJoint("lhknee",     Vector3(-0.2, -0.2, -0.45),  "lthigh");
    makeJoint("lhheel",     Vector3(-0.2, -0.4, -0.5),   "lhknee");
    makeJoint("lhfoot",     Vector3(-0.2, -0.8, -0.5),   "lhheel");

    makeJoint("rthigh",     Vector3(0.15, 0., -0.5),      "hips");
    makeJoint("rhknee",     Vector3(0.2, -0.2, -0.45),   "rthigh");
    makeJoint("rhheel",     Vector3(0.2, -0.4, -0.5),    "rhknee");
    makeJoint("rhfoot",     Vector3(0.2, -0.8, -0.5),    "rhheel");

    makeJoint("lshoulder",  Vector3(-0.2, 0., 0.5),      "shoulders");
    makeJoint("lfknee",     Vector3(-0.2, -0.4, 0.5),    "lshoulder");
    makeJoint("lffoot",      Vector3(-0.2, -0.8, 0.5),   "lfknee");

    makeJoint("rshoulder",  Vector3(0.2, 0.0, 0.5),      "shoulders");
    makeJoint("rfknee",     Vector3(0.2, -0.4, 0.5),     "rshoulder");
    makeJoint("rffoot",      Vector3(0.2, -0.8, 0.5),    "rfknee");

    makeJoint("tail",       Vector3(0., 0., -0.7),       "hips");

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

CentaurSkeleton::CentaurSkeleton() {
    // Order of makeJoint calls is very important
    makeJoint("shoulders",  Vector3(0., 0., 0.5));                      //0
    makeJoint("back",       Vector3(0., 0., 0.),         "shoulders");  //1
    makeJoint("hips",       Vector3(0., 0., -0.5),       "back");       //2

    makeJoint("hback",      Vector3(0., 0.25, 0.5),      "shoulders");  //3
    makeJoint("hshoulders", Vector3(0., 0.5, 0.5),       "hback");      //4
    makeJoint("head",       Vector3(0., 0.7, 0.5),       "hshoulders"); //5

    makeJoint("lthigh",     Vector3(-0.15, 0., -0.5),    "hips");       //6
    makeJoint("lhknee",     Vector3(-0.2, -0.4, -0.45),  "lthigh");     //7
    makeJoint("lhfoot",     Vector3(-0.2, -0.8, -0.5),   "lhknee");     //8

    makeJoint("rthigh",     Vector3(0.15, 0., -0.5),     "hips");       //9
    makeJoint("rhknee",     Vector3(0.2, -0.4, -0.45),   "rthigh");     //10
    makeJoint("rhfoot",     Vector3(0.2, -0.8, -0.5),    "rhknee");     //11

    makeJoint("lshoulder",  Vector3(-0.2, 0., 0.5),      "shoulders");  //12
    makeJoint("lfknee",     Vector3(-0.2, -0.4, 0.5),    "lshoulder");  //13
    makeJoint("lffoot",     Vector3(-0.2, -0.8, 0.5),    "lfknee");     //14

    makeJoint("rshoulder",  Vector3(0.2, 0.0, 0.5),      "shoulders");  //15
    makeJoint("rfknee",     Vector3(0.2, -0.4, 0.5),     "rshoulder");  //16
    makeJoint("rffoot",     Vector3(0.2, -0.8, 0.5),     "rfknee");     //17

    makeJoint("hlshoulder", Vector3(-0.2, 0.5, 0.5),     "hshoulders"); //18
    makeJoint("lelbow",     Vector3(-0.4, 0.25, 0.575),  "hlshoulder"); //19
    makeJoint("lhand",      Vector3(-0.6, 0.0, 0.65),    "lelbow");     //20

    makeJoint("hrshoulder", Vector3(0.2, 0.5, 0.5),      "hshoulders"); //21
    makeJoint("relbow",     Vector3(0.4, 0.25, 0.575),   "hrshoulder"); //22
    makeJoint("rhand",      Vector3(0.6, 0.0, 0.65),     "relbow");     //23

    makeJoint("tail",       Vector3(0., 0., -0.7),       "hips");       //24

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

FileSkeleton::FileSkeleton(const std::string &filename) {
  std::ifstream strm(filename.c_str());

  if (!strm.is_open()) {
    Debugging::out() << "Error opening file " << filename << std::endl;
    return;
  }

  while (!strm.eof()) {
    std::vector<std::string> line = readWords(strm);
    if (line.size() < 5) { // Error
      continue;
    }

    Vector3 p;
    sscanf(line[1].c_str(), "%lf", &(p[0]));
    sscanf(line[2].c_str(), "%lf", &(p[1]));
    sscanf(line[3].c_str(), "%lf", &(p[2]));

    if (line[4] == "-1") {
      line[4] = std::string();
    }

    makeJoint(line[0], p * 2., line[4]);
  }

  initCompressed();
}

DataSkeleton::DataSkeleton()
{
}

void DataSkeleton::init( const std::vector<Vector3> &pts, const std::vector<int> &previd )
{

    int npts = pts.size();

    for ( int i = 0; i < npts; i++ )
    {
        std::string start = std::to_string( i );
        std::string end = std::string();
        if ( previd[i] >= 0 ) // -1 will leave empty string.
        {
            end = std::to_string( previd[i] );
        }
        makeJoint( start, pts[i] * 2., end );  // Do not understand * 2.
    }
}



} // namespace Pinocchio
