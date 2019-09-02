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

#include <FL/Fl.H>
#include "MyWindow.h"
#include "DisplayMesh.h"
#include "processor.h"
#include "motion.h"
#include "shared.h"

// Definition of global variable to record start times of animation runs
unsigned long runStartTime;

int main(int argc, char **argv)
{
  MyWindow *window = new MyWindow(1024, 768, "Pinocchio");
  std::vector<std::string> args;
  for(int i = 0; i < argc; ++i)
    args.push_back(argv[i]);
  // Process, and then print out time taken for processing
  unsigned long processTime = getT();
  process(args, window);
  std::cout << "Process time: " << getT() - processTime << std::endl;

  window->show();

  // Start time for the first run
  runStartTime = getT();
  return Fl::run();
}
