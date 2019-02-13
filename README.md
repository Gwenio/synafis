# Synafis

## Description

Synafis is an interpreted language designed to facilitate processing code.
"Processing" can be evaluating, compiling, error checking, or other things
as dictated by the context the code is placed in for processing.

The purpose of Synafis code is not to simply be executed, though it can be,
but to effectively create programming languages that can be combined to make
a greater whole. Thus various parts of the program can have a tailor made
language that best fits the design of the code. Furthermore, the program
can later be split into separate output programs. For example, a program
for a website would be broken into the HTML, CSS, Javascript, SQL, and
any server side code that is needed. Having been one unit previously,
these components will have had the chance to be checked for compatibility
and optimized based on knowledge of the inner workings of each other.

Synafis is primarily based on the Lisp dialect 'Kernel' by John Shutt.
It also draws inspiration from Pharo Smalltalk and Elixir.

## Building

The 'setup.js' Node script for generating the build files for the project.

Run node -- setup.js --help to get a list of command line options.

Create and fill out 'config.json' to configure the generated build file.

Finally run Ninja in the directory containing the generated files.

## Requirements

- See 'setup.md' for the requirements for the setup script.
- Ninja version 1.7 or higher.
- C++ Compiler (MSVC 2017 is the current baseline for expected standard support).

The documentation is written to be generated from the source files by Doxygen.

## Project Layout

- src: Contains the source files for the project.
- test: Contains tests cases.
- templates: Contains examples of various configuration files for tools.
- js: Contains JavaScript files that support scripts in the project root.
- LICENSE: The license for the project's code.
- README.md: This file.
- templates.md: Lists the uses of the files in templates and modifications that may be needed to
  use them.
- setup.md: Documents setup.js and the associated json files.
- setup.js: Script for generating build files.
- project.json: Describes the project for setup.js.
- config.json: Additional configuration for setup.js.
- build.ninja: The Ninja build file.

## Developers

Adam Armstrong

## License

The project's code is under the ISC license, see the LICENSE file for details.

It is also included in files along side the source code.
