= Synafis

== Description

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

== Building

The 'ninjagen.rb' Ruby script for generating the build files for the project.

Pass '--config' to create a default build configuration file (config.json).

After editing 'config.json', run the script with '--project' to generate the files for Ninja.

Finally run Ninja in the directory containing the generated files.

== Requirements

- Ruby 2.2 with the 'yajl' gem installed.
- Ninja version 1.7 or higher.
- C++ Compiler (MSVC 2017 is the current baseline for expected standard support).

== Project Layout

- src			: Contains the source files for the project.
- test			: Contains tests cases.
- LICENSE		: The license for the project's code.
- README.md		: This file.
- ninjagen.rb	: Script for generating build files.
- project.json	: Describes the project for ninjagen.rb.
- config.json	: Configuration for ninjagen.rb.
- *.ninja		: Ninja build files.

== Developers

Adam Armstrong

== License

The project's code is under the ISC license, see the LICENSE file for details.

It is also included in files along side the source code.
