# CMPUT414-Skinning

This is group F's project on Skinning for the course of CMPUT414 Winter 2015. To view the breakdown of our weekly tasks, our proposal, literature review, demo slides, final report, and all the data we collected from performance test runs and user studies, please see our [wiki](https://github.com/CMPUT414W15TeamF/CMPUT414-Skinning/wiki).


## Table of Content
* [Pinocchio Code](#pinocchio)
* [Dual Quaternion Skinning Library](#dqs)
* [Our Changes / Contributions ](#changes)
* [How to Compile](#compile)
* [How to Run](#run)
* [How to Obtain Run Times](#runtimes)
* [How to Automatically Pause Pinocchio at Specific Frames](#pause)
* [References](#references)
* [Our Wiki](https://github.com/CMPUT414W15TeamF/CMPUT414-Skinning/wiki)


## <a name="pinocchio"></a> Pinocchio Code 

Pinocchio prototype was written by Ilya Baran and Jovan Popovic. Details for
the implementation and algorithms used can be found in their paper paper
"Automatic Rigging and Animation of 3D Characters" SIGGRAPH 2007.

The readme and license information for Pinocchio are available in the 
Pinocchio directory.

For detailed information on Pinocchio, you can visit their website at 
[this link](http://www.mit.edu/~ibaran/autorig/pinocchio.html).

## <a name="dqs"></a> Dual Quaternion Skinning Library

The Dual Quaternion Skinning Library was created by Rodolphe Vaillant-David.

The code provides the functionality necessary for creating, manipulating, and
transformting with dual quaternions. The files encompassing this library are:
* dual_quat_cu.hpp
* mat3.hpp
* point3.hpp
* quat_cu.hpp
* transfo.hpp
* vec3.hpp 


## <a name="changes"></a>Our Changes / Contributions

The main goal of our project was to use the existing prototype Pinocchio and 
to change the skinning implementation of LBS it uses to improve the results.
At first we tried changing the original LBS algorithm a bit, but the results
were quite terrible. 

Our next step was to implement an altogether different algorithm for skinning.
We decided on dual quaternion skinning (DQS) because it was the easiest in terms of
changing a program that currently uses LBS to use dual quaternions. We found
a dual quaternion skinning library by Vaillant-David (mentioned above) which
already had the functions implemented to accomplish the creation and 
manipulation of dual quaternions (eg. multiplying them together), and also
a function to transform a point by a transformation defined by a dual 
quaternion. Our main challenge was just integrating the two approaches since
they both used their own different data structures.

We were able to succesfully integrate this two different codes so that 
Pinocchio was skinning by using dual quaternions instead of transformation
matrices (LBS). 

We then also decided to see if by blending the results of LBS and DQS the
visual results could be improved (since DQS is said to have the problem of
the "bulging" artifact). We did a few more modifications in order to choose
if we wanted to run the pinocchio using LBS, DQS, or a mix of both.

In total, we experimented with 5 different techniques. 
1) Using the original linear blend skinning algorithm to skin models.
2) Using the dual quaternion skinning library to skin.
3) Using both algorithms, but blending the result so as to have 25% of the
	linear blend result, and 75% of the dual quaternion result.
4) Using both algorithms, but blending the result so as to have 50% of the
	linear blend result, and 50% of the dual quaternion result.
5) Using both algorithms, but blending the result so as to have 75% of the
	linear blend result, and 25% of the dual quaternion result.


All in all, below is a list of all the files we modified or created in order 
for our project, along with a description on major changes in each.
Within Pinocchio directory:
* Pinocchio/attachment.cpp: Our main change in this file was the integration
	of the three different skinning techniques (LBS, DQS, MIX). The original 
	function that did the deformation of the skin was deform(). We changed it
	so that within this function, depending on the skinning algorithm to be 
	used, one of 3 new functions was called. The function, dualQuaternion(), 
	was called to perform dual quaternion sknning. The function, linearBlend(),
	was called to perform the linear blend skinning, which was actually just
	the same code that Pinocchio originally used. The function, mixedBlend(),
	was called to perform both LBS and DQS, and then depending on the blending
	weight given, multiplied the results by the appropriate blending weight
	to produce a mixed result.
* Pinocchio/attachment.h: Made necessary changes to declare any new functions 
	and variables we created in attachment.cpp.
* Pinocchio/mesh.cpp: We mostly just added two new members to a mesh object, namely
	`blendWight`, specifying the blending weight for the MIX algorithm, and
	` algo` which specifies which skinning algorithm the program is to use.
* Pinocchio/mesh.h: Made necessary changes to declare any new functions 
	and variables we created in mesh.cpp.
* Pinocchio/transform.h: Our main additions here were to overide the << 
	operator for the Transform class and the Quaternion class. This allowed us
	to be able to decide how we wanted objects of type transform and quaternion
	to print out, thus allowing us later to print out camera rotations by 
	pressing 'r'.
* quatinterface.cpp: We created the following file to help with the interfacing 
	between the different classes from pinocchio and the dual quaternion 
	skinning library. Since they both used data classes defined by themselves,
	we had to convert between the different data classes. For example, while
	Pinocchio uses the Vector3 class, the DQS library uses a Vec3 class.
	
	The two functions we created were `getQuatFromMat`, which takes a matrix
	of type Transform, extracts the individual components, and constructs
	a dual quaternion from it.
	
	The other function is transformPoint(), which takes a vertex of type
	Vector3, and a dual quaternion, and applies the transformation defined
	by the dual quaternion on the vertex.
* quatinterface.hpp: Made necessary changes to declare any new functions 
	and variables we created in quatinterface.cpp.
* Pinocchio/Makefile: Made a few changes to the build command in order to fix
	compilation errors we were having. We also added any new files we made
	so they would be built, or made sure to including new header files needed
	for existing source files.


Within DemoUI directory:
* DemoUI/demoUI.cpp: We didn't change much in this file. We just added the
	definition of the runStartTime global variable, and also added a few lines
	of code to time how long the processing took (the call to process()) function,
	and also defined the start time for the first run of the animation for later.
* DemoUI/processor.cpp: Our main changes in this file were to allow users to
	to specify what skinning algorithm they wanted to use through command line
	arguments. This meant allowing for the additions of `-algo [skinning Algorithm]`
	to be added as an optional command line argument when running the program.
	The three different choices were: linear blend (LBS) which is the default
	if not specified; dual quaternion skinning (DQS); a blend of the two (MIX).
	If MIX was chosen, an additional command line argument could be added that
	specified the blending weight. By default, it is set to 0.5, so 50% of each
	algorithm result is used.
* DemoUI/MyWindow.cpp: We added the function `changeAngle()` that applies a
	transformation on the camera given a rotation axis and angle, a scale
	factor, and a translation vector. This function is then called anytime
	the user enters one of the following keys on the keyboard: 1, 2, 3, 4, 5, 
	and 6. Each of these supply different transformations to the changeAngle
	function so that each allows the user to view the scene at a different 
	angle. 
	
	We also modified the code so that when the user hits 'r' on their keyboard,
	whatever transformation belonging to the current camera view is printed
	to the terminal. We used this information to manually set the transformation
	information needed to change camera angles on different key presses.
	
	One last major change is the modification to pause the current animation
	when the user hits 'p' on their keyboard, as well as the modification to
	print the current frame number as the title of the scene window. If the 
	user hit 'p' again, the animation could continue as normal. This was
	useful because it allowed us to pause at different places, take not of what
	the frame number was, and then later on in a different file set the program
	to pause whenever it reached that frame number so we could adjust the 
	camera angle and take a screenshot. This allowed for running pinocchio with
	different algorithms multiple times, but always pausing at the same frames
	and taking screenshots at the same camera angles.
* DemoUI/MyWindow.h: Made necessary changes to declare any new functions we
	created in MyWindow.cpp.
* DemoUI/motion.cpp: We mostly worked with the getFrameIdx() function so that
	how it ran was defined by normalSpeed variable.
	
	If normalSpeed is set to true, the the original algorithm for getting the
	frame Id is used, which basically uses the current time to determine the
	next frame id. The only problem with this is that frames will always be
	skipped in between, thus making it difficult to always pause at the same
	frames when running our tests and taking screenshots.

	The second option is to set normalSpeed to false. This was, a counter is
	kept which indicates what frame we are on, and when the counter reaches the
	last frame, it is reset to zero. This allows us for consistently being 
	able to pause at the exact frames we want. This is the setting we used
	for our project. Note it also leads to a much slower run time.
* DemoUI/motion.h: Made necessary changes to correspond with changes in
	motion.cpp.
* DemoUI/defmesh.cpp: A few modifications to the already defined functions
	so that a framenum parameter can be passed around from the MyWindow.cpp
	and motion.cpp.
* DemoUI/defmesh.h: Made necessary changes to correspond with changes in
	defmesh.cpp.
* DemoUI/shared.h: This is just a header file we created for declaring global
	variables that needed to be shared by multiple files. The two variables
	are `paused`, a boolean indicating whether or not program is paused, and
	`runStartTime`, a number indicating the start time of each run of the animation.
* DemoUI/Makefile: Made a few changes to the build command in order to fix
	compilation errors we were having. We also added any new files we made
	so they would be built, or made sure to including new header files needed
	for existing source files.


## <a name="compile"></a> How to Compile

Before compiling, make sure you have the following libraries installed:
* FLTK library (on linux, run `sudo apt-get install libfltk1.1-dev`)
* OpenGL (on linux, run `sudo apt-get install freeglut3-dev`)

To compile, cd into the uppermost Pinocchio directory
`cd CMPUT414-Skinning/Pinocchio/`

Then, run `make`.

## <a name="run"></a>How to Run

In order to run Pinocchio for the purposes of this project, first make sure you add the path to libpinocchio.so to your
LD_LIBRARY_PATH variable in your .bashrc. By default, it should be in the folder `CMPUT414-Skinning/Pinocchio/Pinocchio`.

Then, to actually run the program, there are a few options. Below is the general command to use:
`/path/to/DemoUI path/to/obj/file -motion path/to/mocap/file [-algo <LBS|DQS|MIX> [blend_weight]]`

* /path/to/DemoUI : the path to the DemoUI executable file (by default should be in `CMPUT414-Skinning/Pinocchio/DemoUI`)
* /path/to/obj/file : the path to the object file you wish to rig / skin
* -motion : specifies you want to use a motion capture file
* path/to/mocap/file : path to the motion capture file you want to use. Note that you can only use the motion capture files that come with Pinocchio (found under `CMPUT414-Skinning/Pinocchio/DemoUI/data`)
* -algo : used to specify a specific skinning algorithm you want to use. If this is not specified, by default LBS will be used.
* LBS|DQS|MIX : the 3 different skinning algorithms to choose from. LBS = linear blend skinning, DQS = dual quaternion skinning, MIX = mixed result from both LBS and DQS.
* blend_weight : only needed if chosen skinning algorithm is MIX. This blend weight specifies how much of the LBS result you want blended in. By default, if not specified, .5 will be used so the final result will be 50% from LBS and 50% from DQS. A blend weight of 0.0 would mean you are only using the result from DQS, while a blend weight of 1.0 means the result will be solely based on LBS.

Below is an example of running the program on command line, assuming you have not changed the directory structure of this repo and that you are in the uppermost Pinocchio folder:
`./DemoUI/DemoUI ../meshes/Model1.obj -motion DemoUI/data/wakeUpSequence2.txt -algo MIX 0.2`

To adjust the camera angles for your own model, go to Pinocchio/DemoUI/MyWindow.cpp. Find the case switch statement for FL_KEYBOARD. You'll see that some of the cases are '1', '2', '3', '4', '5'. 
When you hit one of these keys on the keyboard, it will change the camera angle to whatever was defined here by calling the changeAngle() function. It expects to receive 
* a rotation axis (Vector3)
* a rotaion angle (double)
* a scale factor (double)
* a translation vector (Vector3)

You can get all of these by running Pinocchio, adjusting the camera using your mouse, 
and then hitting the 'r' key once you have found an angle that you like. 
This will print all the transformation information you need on the terminal.

Also, if you want to pause Pinocchio at anytime, press 'p', and press 'p' again
to unpause the animation.

If you are wanting to know the performance of different algorithms, each time
the program is run, two times are printed out. The first is process time, which
is the time taken to set everything up, and includes skeleton embedding, and
weight assigning. The second time printed out is Run Time, which indicates how
long it takes to display and run the animation from the first frame to the last 
frame. The times printed out are in seconds.

## <a name="runtimes"></a> How to Obtain Run Times
By running Pinocchio, two times will automatically be printed out to your 
terminal. The first is the `process time`, which inclued Discretization Time,
Embedding Time, and Attachment Time (which includes the assignment of weights
to joints). 

The second time that is printed out is the `run time`. This is the time it takes
to run the animation from frame 0 to the last frame. During this time is when 
the live mesh deformation is happening.

## <a name="pause"></a> How to Automatically Pause Pinocchio at Specific Frames
We used a bit of code in `demoUI/motion.cpp` to pause the animation at specific frames. 
You can uncomment the lines between 328 and 338 start with //Pause at specific frames, 
compile, and then run Pinocchio. The animation will then stop at whatever frames
were specified, so you can have time to adjust the camera angle, and take a 
snapshot. You can replace the frame numbers in motion.cpp with whatever frames
you want Pinocchio to stop at. 

But make sure to comment out these lines again when you want to obtain run times! 

## <a name="references"></a>References

We used the following websites and papers to help us and as guides to 
integrating dual quaternions into pinocchio.
* http://rodolphe-vaillant.fr/?e=29
* The meshes used for testing were initially created using Cosmic Blobs(R) software developed by Dassault Systemes SolidWorks Corp.
* Kavan, Ladislav, et al. "Skinning with dual quaternions." Proceedings of the
  2007 symposium on Interactive 3D graphics and games. ACM, 2007.
* Baran, Ilya, and Jovan Popović. "Automatic rigging and animation of 3d
  characters." ACM Transactions on Graphics (TOG). Vol. 26. No. 3. ACM, 2007.

