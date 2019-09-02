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

#include "motion.h"
#include "shared.h"
#include "../Pinocchio/skeleton.h"
#include "../Pinocchio/utils.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace Pinocchio;

Motion::Motion(const std::string &file) : fixedFrame(-1)
{
  std::ifstream strm(file.c_str());

  if(!strm.is_open())
  {
    std::cout << "Error opening file " << file << std::endl;
    return;
  }

  std::cout << "Reading " << file << std::endl;

  readH(strm);
}


std::vector<Vector3> computePose(const std::vector<Vector3> &nums, const int *prev)
{
  int i;
  std::vector<Vector3> out;
  std::vector<Transform<> > tr;

  for(i = 0; i < (int)nums.size(); i += 2)
  {
    Transform<> cur;
    if(nums[i].length() > 1e-8)
      cur = Transform<>(Quaternion<>(nums[i], nums[i].length()), 1., nums[i + 1]);
    else
      cur = Transform<>(nums[i + 1]);

    if(prev[i / 2] >= 0)
      cur = tr[prev[i / 2]] * cur;

    out.push_back(cur * Vector3(0, 0, 0));
    tr.push_back(cur);
  }
  for(i = 0; i < (int)out.size(); ++i)
    out[i] = 0.0005 * (/*tr[0].inverse() */ out[i]);
  for(i = 0; i < (int)out.size(); ++i)
    out[i] = Quaternion<>(Vector3(1., 1., 1.), 4. * M_PI / 3.) * out[i];
  return out;
}


std::vector<Transform<> > computeTransfs(const std::vector<Vector3> &nums, const std::vector<Vector3> &refNums, const int *prev)
{
  int i;
  std::vector<Transform<> > out;
  std::vector<Transform<> > tr, trr;

  for(i = 0; i < (int)nums.size(); i += 2)
  {
    Transform<> cur;
    if(nums[i].length() > 1e-8)
      cur = Transform<>(Quaternion<>(nums[i], nums[i].length()), 1., nums[i + 1]);
    else
      cur = Transform<>(nums[i + 1]);
    if(prev[i / 2] >= 0)
      cur = tr[prev[i / 2]] * cur;

    Transform<> curr;
    if(refNums[i].length() > 1e-8)
      curr = Transform<>(Quaternion<>(refNums[i], refNums[i].length()), 1., refNums[i + 1]);
    else
      curr = Transform<>(refNums[i + 1]);
    if(prev[i / 2] >= 0)
      curr = trr[prev[i / 2]] * curr;

    tr.push_back(cur);
    trr.push_back(curr);
  }

  Quaternion<> qrot(Vector3(1., 1., 1.), 4. * M_PI / 3.);
  Transform<> rot(qrot);
  for(i = 0; i < (int)tr.size(); ++i)
  {
    tr[i] = rot * tr[i];
    trr[i] = rot * trr[i];
  }

  for(i = 0; i < (int)tr.size(); ++i)
  {
    out.push_back((tr[i] * trr[i].inverse()).linearComponent());
  }

  return out;
}


void Motion::readH(std::istream &strm)
{
  int i;
  int lineNum = 0;

  #if 1
  HumanSkeleton skel;
  //int boneCorresp[17] = { 9, 0, 9, 0, 5, 6, 7, 0, 1, 2, 3, 14, 15, 16, 10, 11, 12 };
  int boneCorresp[17] = { 9, 0, 9, 0, 5, 6, 7, 0, 1, 2, 3, 15, 16, 17, 11, 12, 13 };
  //centaur
  #else
  CentaurSkeleton skel;
  int boneCorresp[24] = { 0, 0, 0, 9, 10, 0, 5, 6, 0, 1, 2, 0, 5, 6, 0, 1, 2, 14, 15, 16, 10, 11, 12 };
  #endif

  //int filePrev[18] = {-1, 0, 1, 2, 3, 0, 5, 6, 7, 0, 9, 10, 11, 12, 9, 14, 15, 16};
  int filePrev[19] = {-1, 0, 1, 2, 3, 0, 5, 6, 7, 0, 9, 9, 11, 12, 13, 9, 15, 16, 17};

  std::vector<Vector3> refNums;

  const int numVals = 114;

  /*
  double refVals[numVals] = {
      0.02711933292448521,-0.02192967757582665,1.477331161499023,345.3876953125,0.*-2260.565185546875,0.*910.3690185546875,0.*0.01346228923648596,-0.3637805581092835,0.2299194484949112,0,92.1827392578125,0,0,0.7944101095199585,0,0,0,-399.4296875,0.09269633144140244,0.3882235288619995,0.06221126392483711,0,0,-383.4981384277344,0,-1.032687664031982,0,178.1968231201172,0,0,0.01702138409018517,-0.1227751448750496,-0.1157603338360786,0,-92.1827392578125,0,0,0.2875869870185852,0,0,0,-399.4296875,-0.1211171820759773,0.4444715082645416,-0.07640518993139267,0,0,-383.4981384277344,0,-0.969978392124176,0,178.1968231201172,0,0,-0.06911179423332214,0.2903465926647186,0.1938483566045761,-50,0,200,-0.3563787341117859,-0.0111835552379489,-0.06207561865448952,0,0,305.3711547851563,0.5505122542381287,0.1025727167725563,0.253344863653183,0,171.9588165283203,31.92419624328613,0.1546099036931992,-0.5534430146217346,-0.5299744009971619,0,0,-273.792724609375,-0.05859549716114998,-0.2393989413976669,0.007049600593745709,0,0,-238.039306640625,0.4226579368114471,-0.007558770943433046,0.03522985056042671,0,0,305.3711547851563,-0.6542086601257324,0.1297714412212372,-0.0420396625995636,0,-171.9588165283203,31.92419624328613,-0.2740777730941773,-0.8770577907562256,0.5632873773574829,0,0,-273.792724609375,0.02855916135013104,-0.04506233334541321,-0.0006436237599700689,0,0,-238.039306640625
  };

  refNums.resize(36);
  for(i = 0; i < numVals; ++i)
      refNums[i / 3][i % 3] = refVals[i];
  for(i = 0; i < 18; ++i) {
      if(i != 10 && i != 11 && i != 14 && i != 15 && i != 7 && i != 3 && i != 16 && i != 12)
          refNums[i * 2] = Vector3();
  }
  refNums[24] = refNums[32] = (refNums[24] + refNums[32]) * 0.25;
  */

  double refVals[numVals] =
  {
    /*
    0*-0.0250254, 0*-0.00934205, 0*-1.26306, -1031.46, 946.599, 922.532, -0.0194843, 0.0568432, 0.121606, 0, 100.054, 0, 0, 0.0799654, 0, 0, 0, -425.477, 0.126552, 0.351484, 0.0883195, 0, 0, -435.898, 0, -0.601974, 0, 175.435, 0, 0, 0.0251119, 0.0218503, -0.251097, 0, -100.054, 0, 0, 0.168678, 0, 0, 0, -425.477, -0.0247609, 0.272914, -0.0187815, 0, 0, -435.898, 0, -0.452158, 0, 175.435, 0, 0, -1.99758e-05, 0.0727399, 0.0286016, -50, 0, 259.033, 0.205393, -0.127389, 0.434131, 0, 0, 330.555, -0.502925, 0.019719, 0.0767161, 0, 0, 261.792, 0.72968, -0.143153, 0.109099, 0, 187.185, 49.4561, 0.116146, -0.430137, -0.518831, 0, 0, -295.832, -0.00958385, 0.0218873, -0.000104887, 0, 0, -274.368, 0.465736, -0.0151283, 0.063763, 0, 0, 261.792, -0.669662, -0.0660955, -0.306066, 0, -187.185, 49.4561, -0.116392, -0.335282, 0.661388, 0, 0, -295.832, -0.0882838, 0.0118039, -0.000521391, 0, 0, -274.368
    */

    0*0.2927989586788964, 0*0.238953736716009, 0*4.601340174853147,
    -239.747966657787, 528.8079085296968, 824.003250634485, 0.0,
    0.07421882822297582, -0.0, 0, 101.7259979248047, 0, 0,
    0.2250166493605267, 0, 0, 0, -390.8139953613281, -0.0,
    0.2742693567580709, -0.0, 0, 0, -380.1099853515625, 0,
    -0.0, 0, 167.8670043945313, 0, 0, 0, 0.07421882822297582,
    -0.0, 0, -101.7259979248047, 0, 0, 0.2250166493605267, 0,
    0, 0, -390.8139953613281, -0.0, 0.2742693567580709, -0.0,
    0, 0, -380.1099853515625, 0, -0.0, 0, 167.8670043945313,
    0, 0, -0.0, 0.2114189413041665, 0.0, -50, 0,
    96.24990081787109, 0.04333145867808721, 0.001625053310422895,
    -0.1178766858603095, 0, 0, 479.9630126953125,
    -0.2432491860025614, -0.01646272902648508, -0.1357890434708229,
    0, 0, 426.9849853515625, 0.4110673301167482,
    -0.07179547997442083, 0.223251015556675, 0, 176.7760009765625,
    2.123519897460938, 0.05564195976910109, -0.1970120842592028,
    -0.5503636706436617, 0, 0, -263.0299987792969,
    0.01854052737835739, -0.05639195309438585, -0.0005072173454329611,
    0, 0, -224.7669982910156, 0.2432491860025614,
    0.01646272902648508, 0.1357890434708229, 0, 0,
    426.9849853515625, -0.4110673301167482, 0.07179547997442083,
    0.223251015556675, 0, -176.7760009765625, 2.123519897460938,
    0.05564195976910109, -0.1970120842592028, -0.5503636706436617,
    0, 0, -263.0299987792969, 0.01854052737835739,
    -0.05639195309438585, -0.0005072173454329611, 0, 0,
    -224.7669982910156

  };

  refNums.resize(38);
  for(i = 0; i < numVals; ++i)
    refNums[i / 3][i % 3] = refVals[i];

  while(!strm.eof())
  {
    ++lineNum;
    if(data.size() > 36000)
      break;

    std::vector<std::string> words = readWords(strm);

    if(words.size() == 0)
      continue;
    //comment
    if(words[0][0] == '#')
      continue;

    if(words.size() != (int)numVals)
    {
      std::cout << "Error reading motion file: not " <<
        numVals << " numbers in line " << lineNum << std::endl;
      data.clear();
      return;
    }

    std::vector<Vector3> nums(words.size() / 3);
    for(i = 0; i < (int)words.size(); ++i)
    {
      double cur;
      sscanf(words[i].c_str(), "%lf", &cur);
      nums[i / 3][i % 3] = cur;
    }

    if(refPose.empty())
    {
      refPose = computePose(refNums, filePrev);
      legWidth = fabs(refPose[4][0] - refPose[8][0]);
      legLength = fabs(refPose[4][1] - refPose[0][1]);
    }

    std::vector<Vector3> pose = computePose(nums, filePrev);

    {
      std::vector<Vector3> cp;
      for(i = 1; i < (int)pose.size(); ++i)
      {
        cp.push_back(pose[filePrev[i]]);
        cp.push_back(pose[i]);
      }
      poses.push_back(cp);
    }

    data.resize(data.size() + 1);
    std::vector<Transform<> > trs = computeTransfs(nums, refNums, filePrev);
    for(i = 0; i < (int)skel.fPrev().size() - 1; ++i)
    {
      data.back().push_back(trs[boneCorresp[i]]);
    }

    Quaternion<> qtrans(Vector3(1., 1., 1.), 4. * M_PI / 3.);
    Transform<> trans(qtrans * nums[1] * 0.0005);
    data.back()[0] = trans * data.back()[0];
  }

  //centaur
  if(false)
  {
    int offset = 180;

    for(i = (int)data.size() - 1; i >= 0; --i)
    {
      int j;
      data[i][1] = data[std::max(0, i - offset)][1];
      for(j = 6; j <= 11; ++j)
        data[i][j] = data[std::max(0, i - offset)][j];
    }
  }
}


#ifdef _WIN32
#include "windows.h"

long getT()
{
  SYSTEMTIME systime;
  GetSystemTime(&systime);

  return systime.wMilliseconds + 1000 * (systime.wSecond + 60 * (systime.wMinute + 60 * (systime.wHour + 24 * systime.wDay)));
}


#else
#include <sys/time.h>

long getT()
{
  struct timeval tv;
  gettimeofday (&tv, NULL);

  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

int getMsecs()
{
  static unsigned long startTime = getT();
  return getT() - startTime;
}


/* 
 * In this function, the id of the next frame to be shown is retrieved. There
 * are two options though depending on the value f the boolean normalSpeed.
 *
 * If normalSpeed is set to true, the the original algorithm for getting the
 * frame Id is used, which basically uses the current time to determine the
 * next frame id. The only problem with this is that frames will always be
 * skipped in between, thus making it difficult to always pause at the same
 * frames when running our tests and taking screenshots.
 *
 * The second option is to set normalSpeed to false. This was, a counter is
 * kept which indicates what frame we are on, and when the counter reaches the
 * last frame, it is reset to zero. This allows us for consistently being
 * able to pause at the exact frames we want. This is the setting we used
 * for our project. Note it also leads to a much slower run time.
 *
 */
int Motion::getFrameIdx() const
{
  // Indicates how to get frame id, the normal way (faster) or the more
  // precise way (for this, set to false).
  bool normalSpeed = false;

  if(fixedFrame >= 0)
    return fixedFrame;

  // Keep a counter to indicate what frame we are on, and increment it
  // each time through
  if (!normalSpeed)
  {
    static int framenum = -1;

    // If the animation is paused, don't increment the framenumber.
    if (paused)
    {
      return framenum;
    }

    framenum++;

    // If reached last frame, reset the framenumber and output how
    // long it took to run through the animation
    if (framenum >= (signed)data.size())
    {
      std::cout << "Run Time End: " << getT() - runStartTime << std::endl;
      runStartTime = getT();
      framenum = 0;
    }

    // Pause at specific frames to take snapshot
    /*/if (framenum == 33 ||
        framenum == 4827 ||  // angle 4
        framenum == 5655 ||  // angle 4
        framenum == 5827 ||  // angle 4
        framenum == 6254 ||  // angle 3
        framenum == 485 ||  // angle 3
        framenum == 3917 || // angle 5
        framenum == 5571 || // angle 6
        framenum == 5844) { // angle 6
        paused = true;
    }*/

    return framenum;
  }

  // Rough measurement of how long it took to run through the animation
  // at normal speed
  int frame = (getMsecs() / (1000 / 120)) % data.size();
  if (frame >= (signed)data.size() - 10)
  {
    std::cout << "Run Time End: " << getT() - runStartTime << std::endl;
    runStartTime = getT();
  }

  // Use if measuring FPS
  return frame;
}


std::vector<Transform<> > Motion::get() const
{
  return data[getFrameIdx()];
}


std::vector<Vector3> Motion::getPose(int &framenum) const
{
  framenum = getFrameIdx();
  return poses[framenum];
}
