// AttachWeights.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <fstream>

#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/utils.h"
#include "../Pinocchio/debugging.h"
#include "../Pinocchio/attachment.h"
#include "../Pinocchio/pinocchioApi.h"

using namespace std;
using namespace Pinocchio;

struct ArgData
{
  ArgData() :
  stopAtMesh(false), stopAfterCircles(false), skelScale(1.), noFit(true),
    skeleton(HumanSkeleton()), stiffness(1.),
    skelOutName("skeleton.out"), weightOutName("attachment.out")
  {
  }

  bool stopAtMesh;
  bool stopAfterCircles;
  string filename;
  Quaternion<> meshTransform;
  double skelScale;
  bool noFit;
  Skeleton skeleton;
  string skeletonname;
  double stiffness;
  string skelOutName;
  string weightOutName;
};

void printUsageAndExit()
{
  cout << "Usage: attachWeights filename.{obj | ply | off | gts | stl}" << endl;
  cout << "              [-skel skelname] [-rot x y z deg]* [-scale s]" << endl;
  cout << "              [-meshonly | -mo] [-circlesonly | -co]" << endl;
  cout << "              [-fit] [-stiffness s]" << endl;
  cout << "              [-skelOut skelOutFile] [-weightOut weightOutFile]" << endl;

  exit(0);
}


ArgData processArgs(const vector<string> &args)
{
  ArgData out;
  int cur = 2;
  int num = args.size();
  if(num < 2)
    printUsageAndExit();

  out.filename = args[1];

  while(cur < num)
  {
    string curStr = args[cur++];
    if(curStr == string("-skel"))
    {
      if(cur == num)
      {
        cout << "No skeleton specified; ignoring." << endl;
        continue;
      }
      curStr = args[cur++];
      if(curStr == string("human"))
        out.skeleton = HumanSkeleton();
      else if(curStr == string("horse"))
        out.skeleton = HorseSkeleton();
      else if(curStr == string("quad"))
        out.skeleton = QuadSkeleton();
      else if(curStr == string("centaur"))
        out.skeleton = CentaurSkeleton();
      else
        out.skeleton = FileSkeleton(curStr);
      out.skeletonname = curStr;
      continue;
    }
    if(curStr == string("-rot"))
    {
      if(cur + 3 >= num)
      {
        cout << "Too few rotation arguments; exiting." << endl;
        printUsageAndExit();
      }
      double x, y, z, deg;
      sscanf(args[cur++].c_str(), "%lf", &x);
      sscanf(args[cur++].c_str(), "%lf", &y);
      sscanf(args[cur++].c_str(), "%lf", &z);
      sscanf(args[cur++].c_str(), "%lf", &deg);

      out.meshTransform = Quaternion<>(Vector3(x, y, z), deg * M_PI / 180.) * out.meshTransform;
      continue;
    }
    if(curStr == string("-scale"))
    {
      if(cur >= num)
      {
        cout << "No scale provided; exiting." << endl;
        printUsageAndExit();
      }
      sscanf(args[cur++].c_str(), "%lf", &out.skelScale);
      continue;
    }
    if(curStr == string("-meshonly") || curStr == string("-mo"))
    {
      out.stopAtMesh = true;
      continue;
    }
    if(curStr == string("-circlesonly") || curStr == string("-co"))
    {
      out.stopAfterCircles = true;
      continue;
    }
    if(curStr == string("-fit"))
    {
      out.noFit = false;
      continue;
    }
    if(curStr == string("-stiffness"))
    {
      if(cur >= num)
      {
        cout << "No stiffness provided; exiting." << endl;
        printUsageAndExit();
      }
      sscanf(args[cur++].c_str(), "%lf", &out.stiffness);
      continue;
    }
    if(curStr == string("-skelOut"))
    {
      if(cur == num)
      {
        cout << "No skeleton output specified; ignoring." << endl;
        continue;
      }
      curStr = args[cur++];
      out.skelOutName = curStr;
      continue;
    }
    if(curStr == string("-weightOut"))
    {
      if(cur == num)
      {
        cout << "No weight output specified; ignoring." << endl;
        continue;
      }
      curStr = args[cur++];
      out.weightOutName = curStr;
      continue;
    }
    cout << "Unrecognized option: " << curStr << endl;
    printUsageAndExit();
  }

  return out;
}


void process(const vector<string> &args)
{
  int i;
  ArgData a = processArgs(args);

  Debugging::setOutStream(cout);

  Mesh m(a.filename);
  if(m.vertices.size() == 0)
  {
    cout << "Error reading file.  Aborting." << endl;
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

    o.attachment = new Attachment(m, a.skeleton, o.embedding, tester, a.stiffness);

    delete tester;
    delete distanceField;
  }

  if(o.embedding.size() == 0)
  {
    cout << "Error embedding" << endl;
    exit(0);
  }

  //output skeleton embedding
  for(i = 0; i < (int)o.embedding.size(); ++i)
    o.embedding[i] = (o.embedding[i] - m.toAdd) / m.scale;
  ofstream os(a.skelOutName.c_str());
  for(i = 0; i < (int)o.embedding.size(); ++i)
  {
    os << i << " " << o.embedding[i][0] << " " << o.embedding[i][1] <<
      " " << o.embedding[i][2] << " " << a.skeleton.fPrev()[i] << endl;
  }

  //output attachment
  std::ofstream astrm(a.weightOutName.c_str());
  for(i = 0; i < (int)m.vertices.size(); ++i)
  {
    Vector<double, -1> v = o.attachment->getWeights(i);
    for(int j = 0; j < v.size(); ++j)
    {
      double d = floor(0.5 + v[j] * 10000.) / 10000.;
      astrm << d << " ";
    }
    astrm << endl;
  }

  delete o.attachment;
}


int main(int argc, char **argv)
{

  vector<string> args;
  for(int i = 0; i < argc; ++i)
    args.push_back(argv[i]);
  process(args);

  return 0;
}
