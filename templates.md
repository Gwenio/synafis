# Templates

## Doxyfile

Doxyfile is a partial configuration for Doxygen documentation program.

Copy it into the root directory of the project and then modify any setting desired.

Either run 'doxygen Doxyfile' or 'doxywizard Doxyfile' in the root directly to
generate the documentation.

Doxywizard can be used to adjust settings with a graphical interface rather than
editing the file manually.

It is recommended to set HAVE_DOT to yes if you have Graphviz installed and
then set DOT_PATH to the directory containing dot.

## Setup Config

Examples for config.json used by setup.js are in the config subfolder.

Copy the files before modifying.

To use them either place the template in the root project folder and rename to config.json or
pass its name to setup.js in the commandline.

### MSVC Config

templates/config/msvc2017.json

This is an example for using the Visual Studio 2017 C++ compiler.

The default values for msvc_version and winsdk_version may need to be changed to match the
directories on your system.
