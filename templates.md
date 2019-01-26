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

## VSCode

The files in the 'vscode' subdirectory are for the Visual Studio Code text editor.

To use, copy the files into a folder called '.vscode' in the root of the project.

### VSCode Tasks

The file 'tasks.json' defines commands to be run from 'Run Task' in the Terminal drop down menu.

The command setting for each task will need to be changed to point to the appropriate executable
if it is not located on your PATH environment variable.

For the ninja tasks, you may want to adjust the number of jobs run in parallel by changing the
number following "-j".

### VSCode Launch

The file launch.json defines commands to be run from the Debugging menu.

The "type" setting may need to be changed if not compiling with MSVC as a different debugger
may be needed.

### VSCode C/C++ Properties

The file c_cpp_properties.json defines a example configurations for the C/C++ extension.

Currently just defines the configuration for MSVC targeting a release build.

Program and include paths may need to be updated to match your system.

The defines list will need to be changed to simulate different build setups.

## Schemas

The 'schemas' sub-directory contains schemas for various files in the project, especially for JSON
configuration files.

When using VS Code as an editor, the JSON schemas can be used by either setting the "$schema"
property in the root object of the JSON file to apply it to or by configuring the "json.schemas"
setting in the workspace's 'settings.json' file.
See the
[VS Code Documentation for JSON Schemas](https://code.visualstudio.com/docs/languages/json#_json-schemas-settings)
for details.

### Project JSON Schema

The file 'project.json' contains the schema for the 'project.json' located in the root of the project.
