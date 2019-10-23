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

#include <FL/fl_ask.H>
#include <fstream>

#include "processor.h"
#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/utils.h"
#include "../Pinocchio/debugging.h"
#include "../Pinocchio/attachment.h"
#include "../Pinocchio/pinocchioApi.h"
#include "defmesh.h"
#include "motion.h"

using namespace Pinocchio;

struct ArgData {
  ArgData() :
  stopAtMesh(false), stopAfterCircles(false), skelScale(1.), noFit(false),
    skeleton(HumanSkeleton())
  {
  }

  bool stopAtMesh;
  bool stopAfterCircles;
  std::string filename;
  std::string motionname;
  Quaternion<> meshTransform;
  double skelScale;
  bool noFit;
  Skeleton skeleton;
  std::string skeletonname;
  // Indicates which skinning algorithm to use
  int skinAlgorithm;
  // Indicates the blending weight for MIX algorithm
  float blendWeight;
};


void printUsageAndExit() {
  std::cout << "Usage: DemoUI filename.{obj | ply | off | gts | stl}" << std::endl;
  std::cout << "              [-skel skelname] [-rot x y z deg]* [-scale s]" << std::endl;
  std::cout << "              [-meshonly | -mo] [-circlesonly | -co]" << std::endl;
  std::cout << "              [-motion motionname] [-nofit]" << std::endl;
  std::cout << "              [-algo skinning_algorithm [blend_weight]]" << std::endl;

  exit(0);
}


ArgData processArgs(const std::vector<std::string> &args)
{
  ArgData out;
  int cur = 2;
  int num = args.size();
  if(num < 2)
    printUsageAndExit();

  out.filename = args[1];
  // set default skinning algorithm as LBS, and the default blending weight
  // as 0.5
  out.skinAlgorithm = Mesh::LBS;
  out.blendWeight = 0.5;

  while(cur < num)
  {
    std::string curStr = args[cur++];
    if(curStr == std::string("-skel"))
    {
      if(cur == num)
      {
        std::cout << "No skeleton specified; ignoring." << std::endl;
        continue;
      }
      curStr = args[cur++];
      if(curStr == std::string("human"))
        out.skeleton = HumanSkeleton();
      else if(curStr == std::string("horse"))
        out.skeleton = HorseSkeleton();
      else if(curStr == std::string("quad"))
        out.skeleton = QuadSkeleton();
      else if(curStr == std::string("centaur"))
        out.skeleton = CentaurSkeleton();
      else
        out.skeleton = FileSkeleton(curStr);
      out.skeletonname = curStr;
    }
    else if(curStr == std::string("-rot"))
    {
      if(cur + 3 >= num)
      {
        std::cout << "Too few rotation arguments; exiting." << std::endl;
        printUsageAndExit();
      }
      double x, y, z, deg;
      sscanf(args[cur++].c_str(), "%lf", &x);
      sscanf(args[cur++].c_str(), "%lf", &y);
      sscanf(args[cur++].c_str(), "%lf", &z);
      sscanf(args[cur++].c_str(), "%lf", &deg);

      out.meshTransform = Quaternion<>(Vector3(x, y, z), deg * M_PI / 180.) * out.meshTransform;
    }
    else if(curStr == std::string("-scale"))
    {
      if(cur >= num)
      {
        std::cout << "No scale provided; exiting." << std::endl;
        printUsageAndExit();
      }
      sscanf(args[cur++].c_str(), "%lf", &out.skelScale);
    }
    else if(curStr == std::string("-meshonly") || curStr == std::string("-mo"))
    {
      out.stopAtMesh = true;
    }
    else if(curStr == std::string("-circlesonly") || curStr == std::string("-co"))
    {
      out.stopAfterCircles = true;
    }
    else if (curStr == std::string("-nofit"))
    {
      out.noFit = true;
    }
    else if(curStr == std::string("-motion"))
    {
      if(cur == num)
      {
        std::cout << "No motion filename specified; ignoring." << std::endl;
        continue;
      }
      out.motionname = args[cur++];
    }
    else if (curStr == std::string("-algo"))
    {
      /*  Option to use a different skinning algorithm than the
       *  default LBS. Currently, options are LBS, DQS, and MIX */
      std::string algo = args[cur++];
      if (algo == std::string("LBS"))
        out.skinAlgorithm = Mesh::LBS;
      else if (algo == std::string("DQS"))
        out.skinAlgorithm = Mesh::DQS;
      else if (algo == std::string("MIX"))
      {
        /*  Grab the desired blending weight for LBS, i.e
         *  how much of the result of LBS you want to see */
        if(cur >= num)
        {
          std::cout << "No blending weight given; exiting." << std::endl;
          std::cout << args[cur] << std::endl;
          printUsageAndExit();
        }
        out.skinAlgorithm = Mesh::MIX;
        sscanf(args[cur++].c_str(), "%f", &out.blendWeight);
      }
      else
      {
        std::cout << "Unrecognized skinning algorithm" << std::endl;
        printUsageAndExit();
      }
    }
    else
    {
      std::cout << "Unrecognized option: " << curStr << std::endl;
      printUsageAndExit();
    }
  }

  return out;
}


void process(const std::vector<std::string> &args, MyWindow *w)
{
  int i;
  ArgData a = processArgs(args);

  Debugging::setOutStream(std::cout);

  Mesh m(a.filename, a.skinAlgorithm, a.blendWeight);
  if(m.vertices.size() == 0)
  {
    std::cout << "Error reading file.  Aborting." << std::endl;
    exit(0);
    return;
  }

  for(i = 0; i < (int)m.vertices.size(); ++i)
    m.vertices[i].pos = a.meshTransform * m.vertices[i].pos;
  m.normalizeBoundingBox();
  m.computeVertexNormals();

  Skeleton given = a.skeleton;
  given.scale(a.skelScale * 0.7);

  //if early bailout
  if(a.stopAtMesh)
  {
    w->addMesh(new StaticDisplayMesh(m));
    return;
  }

  PinocchioOutput o;
  //do everything
  if(!a.noFit)
  {
    o = autorig(given, m);
  }
  //skip the fitting step--assume the skeleton is already correct for the mesh
  else
  {
    TreeType *distanceField = constructDistanceField(m);
    VisTester<TreeType> *tester = new VisTester<TreeType>(distanceField);

    o.embedding = a.skeleton.fGraph().verts;
    for(i = 0; i < (int)o.embedding.size(); ++i)
      o.embedding[i] = m.toAdd + o.embedding[i] * m.scale;

    o.attachment = new Attachment(m, a.skeleton, o.embedding, tester);

    delete tester;
    delete distanceField;
  }

  if(o.embedding.size() == 0)
  {
    std::cout << "Error embedding" << std::endl;
    exit(0);
  }

  if(a.motionname.size() > 0)
  {
    w->addMesh(new DefMesh(m, given, o.embedding, *(o.attachment),
      new Motion(a.motionname)));
  }
  else
  {
    w->addMesh(new StaticDisplayMesh(m));

    for(i = 1; i < (int)o.embedding.size(); ++i)
    {
      w->addLine(LineSegment(o.embedding[i], o.embedding[given.fPrev()[i]], Vector3(.5, .5, 0), 4.));
    }
  }

  //output skeleton embedding
  for(i = 0; i < (int)o.embedding.size(); ++i)
    o.embedding[i] = (o.embedding[i] - m.toAdd) / m.scale;
  std::ofstream os("skeleton.out");
  for(i = 0; i < (int)o.embedding.size(); ++i)
  {
    os << i << " " << o.embedding[i][0] << " " << o.embedding[i][1] <<
      " " << o.embedding[i][2] << " " << a.skeleton.fPrev()[i] << std::endl;
  }

  //output attachment
  std::ofstream astrm("attachment.out");
  for(i = 0; i < (int)m.vertices.size(); ++i)
  {
    Vector<double, -1> v = o.attachment->getWeights(i);
    for(int j = 0; j < v.size(); ++j)
    {
      double d = floor(0.5 + v[j] * 10000.) / 10000.;
      astrm << d << " ";
    }
    astrm << std::endl;
  }

  delete o.attachment;
}
