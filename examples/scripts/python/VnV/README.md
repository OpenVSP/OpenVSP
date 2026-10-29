# <div align="center">OpenVSP Verification & Validation Studies </div>
#### <div align="center">[OpenVSP Home](http://www.openvsp.org) | [OpenVSP Wiki](http://www.openvsp.org/wiki/doku.php)</div>
## Architecture

This python itteration of the V&V studies is implemented via calls to the c++ api via swig which must be installed.
The python openvsp standard api stuff is needed, and setting that up is done via a venv or a conda enviornment.
There are requirements for this code, mainly matplotlib used for charting, and pathlib used for consistent file structure resasons, both I put in the venv/conda stuff.

Each study is broken out into its own python file, which can be run from command line with no arguments. 
(it has been set up for, with a little future work, running with parameters specifying which substudies to run and at what regenration level)
All studies may be run at once in parallel from one terminal with the Master_VSP_VV_script_test.py with no arguments. (argument format designed for but not built yet)
They have been parallelized via study, so hershey still goes on for a while.
They should all finish, and then a print to console of all of the exit codes of each study shoud happen.
I have not tested the old V&V, but this should run much much faster even with the overhead of python as a lot of time is spent in C++ anyway.

Each study generates its files in the directory of the script itself in a folder named 'testname'+'_files'.
Each study generates its needed vsp files in the vsp_files subdirectory.
Generated images are stored in the 'testname'+'_img' subdirectory, and inside that there may be subdirectories for the subtests being run.
Currently, images are being generated as .svg files, scalable vector graphics, which are infinitely resizable without losing resolution, and can be opened with any web browser or photo app.
If desired the .svg extensions can be replaced with .png and it should all still work as that how it was first implemented.

Each study has 2 main functions, a 'run'+'testname'+'study' function that acts as the pseudo main function, and the setup_filepaths function that is self explanatory.
Each substudy has 3 main functions, one for generating required files, one for testing everything, and a third for creating graphs and tables.
Each study also has the ability to generate a picklefile of the class object, effectively allowing reusing stored computations.
The run modes for each substudy are currently 0 for do nothing, 1 for only regenerate charts which is helpful for debugging charts, 2 for only redoing the testing and charts, and 3 for regenerating everything.

The saved charts are then able to display in the report which is written with Report.MD
To generate a savable report document, I currently am using VSCode with the 'Markdown All In One' extension
It has native LaTeX support and can output either an HTML file, or a PDF, and has a built in preview mode which is invaluable for debugging.
I have reason to believe that the Github markdown flavor would only need a few tweaks to display properly as it supports some LaTeX formatting.
It could be ported to some other markdown flavor or even put into a static webpage using jsMath, MathJax, or something else, but that might be a lot harder.

## State of the current code

The main contributor of this implementation isn't quite done with it.
Initally the charting work was all done with Bokeh, but was rejected due to install requirements.
Graphs are now all done with matplotlib and are mostly ready to go barring formatting tweaks.
Tables are not implemented fully, with quick and dirty testing being done in matplotlibstuff.py.
From there, the supersonic test has been ported to use that table format.
In other tests, the bokeh calls have been comented out for easier porting to matplotlib.
The main contributor is not at all happy with matplotlib tables as they are subject to a framwork not meant for them to be standalone.
They could not find a better option in the time available with the ablility to generate images.

The Hershey test was first written by another contributor. 
While the main contributor tried to port everything over in the same style, there are discrepancies which can be standardized in the future.
Mainly, there are static markdown tables in Hershey that need ported.
There are also other tables throughout that should be using actual variables instead of hardcoded things for maintainability reasons.

Two things that I know of that go wrong with the current implementation are:
Panel sometimes just fails or gives 0 values for the charts, not sure on what is up there.
Supersonic delta wing turns out values, but they don't match with the original results. 
I validated all of the math coincides with the previous implementation, but I could have missed something, or I could have made a mistake elsewhere.
