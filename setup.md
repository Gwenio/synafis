# The Synafis Setup Script

1. [Requirements](#requirements)
2. [The Why](#the-why)
3. [Usage](#usage)
   1. [Command Line](#command-line)
   2. [Backups](#backups)
4. [Configuration Files](#configuration-files)
   1. [Validating Configuration Files](#validating-configuration-files)
   2. [Project Configuration](#project-configuration)
      1. [Build Variants](#build-variants)
   3. [User Configuration](#user-configuration)
   4. [Ninja Generation Configuration](#ninja-generation-configuration)
      1. [Header](#header)
5. [Generators](#generators)
   1. [Ninja Generator](#ninja-generator)
      1. [Ninja Command Line](#ninja-command-line)
      2. [Automatic Re-Generation](#automatic-re-generation)

---

## Requirements

The setup script is written to be executed using
[Node.js](https://nodejs.org/en/) version 10.9.0 or higher.

Using NPM (comes with Node), you will need to install the following dependencies:

- [command-line-args >= 5.0.2](https://www.npmjs.com/package/command-line-args)
- [command-line-usage >= 5.0.5](https://www.npmjs.com/package/command-line-usage)
- [lodash >= 4.17.10](https://www.npmjs.com/package/lodash)
- [asyncmachine >= 3.4.1](https://www.npmjs.com/package/asyncmachine)
- [json5 >= 2.1.0](https://www.npmjs.com/package/json5)

---

## The Why

Different tools are better for some use cases than others.

Many modern build generation tools (CMake, Meson) focus on being used by end users of the software.
This means they  prefer to make the process require as little user input as possible.

The setup script for Synafis goes the opposite direction, maximizing the user's level of control.
This is because developers should know about their development environment.
The specifics of the project are then provided for those that do not need to edit them.

Once the project reaches a usable state, other methods of building the project will be added.

---

## Usage

### Command Line

To see command line options run:

```sh
node -- setup.js --help
```

### Backups

When the script successfully generates a build file,
the previous version is renamed with '.bak' appended.
Then the new file replaces the old.
If the script fails, the old build is not overwritten nor is the backup.

So for the Ninja generator it produces 'build.ninja'.
The backup would then be 'build.ninja.bak'.

Only one backup is kept automatically.
To keep more manually rename the backups.

To prevent a new backup from being created, pass the '--no-backup' option.

```sh
node -- setup.js --no-backup
```

The purpose of backups is incase the new build is incorrect.
Therefore be careful that the backup is not overwritten until sure the new build works properly.

---

## Configuration Files

The setup scrip uses JSON files, and allows for JSON5.

When running the script three configuration files will be used.
They are the project, user, and generator configurations.

### Validating Configuration Files

At this time the script does not validate the JSON configuration files it uses as input.
To help with checking the configurations,
JSON Schema definitions are provided in the 'templates/schemas' directory.

These provided schemas can by used by an external program to check the configuration files for errors.
An external tool will be required to do so.

If using VS Code to edit the configuration files,
you may set the "$schema" property in the root object of the JSON file to a relative path to the schema.
Then VS Code will check for errors as you edit the file.

```json
{
    "$schema": "./templates/schemas/project.json"
    // rest of the project configuration
}
```

Note that while the script supports JSON5,
some JSON Schema validation tools may not as the JSON Schema standard does not mandate JSON5.
If your tool of choice does not, it will be necessary to remove any comments from the configuration files.

### Project Configuration

- Schema: templates/schemas/project.json
- Default file: project.json

In most cases it will not be necessary to modify the project configuration.
And the main case it will be is to add more files to the build.

#### Build Variants

Build variants are a means of specifying different types of builds for a project.
For example, an "OS" (Operating System) variant might have "windows", "linux", and "mac" as its options.
Then the generated output would contain how to build the project for each type of system.

The variants in the project configuration are only those important to determining the input file
and the steps to build outputs.
Additional variants may be specified in the other configuration file.

### User Configuration

- Schema: templates/schemas/config.json
- Default file: config.json
- Modified file: user.json

### Ninja Generation Configuration

- Schema: templates/schemas/ninja.json
- Preferred file name: ninja.json

Example configurations can be found in 'templates/config'.

#### Header

```json
{
    // Relative path to JSON schema (optional).
    "$schema": "./templates/schemas/ninja.json",
    // Specify that the generator being configured is Ninja.
    "generator": "ninja",
    // Required minimum Ninja version (required).
    "version": 1.7,
    // Directory for Ninja's files (defaults to './').
    "directory": "tmp",
    // Name of the rule for regenerating build.ninja(optional).
    "regenerate": "regen",
    // reset of ninja.json
}
```

The "$schema" property is an optional place to place a relative path to the JSON schema.

The "generator" property specifies the generator, and for the Ninja generator must be set to "ninja".

The "version" property is the the minimum version to Ninja to be supported in the output file.

The "directory" property is where files Ninja creates to store its data will be placed.
If the directory does not exist, it will be created as Ninja does not create directories.
If not specified the current directory will be used.

The "regenerate" property is the name for a Ninja rule used to rebuild the 'build.ninja' file.
If not specified, there will be no build target for rebuilding 'build.ninja'.
The name must not conflict with the names of other rules specified in the configuration.
See [Ninja Re-Generation](#automatic-re-generation).

---

## Generators

Currently the only supported generator is the Ninja generator.

### Ninja Generator

[Ninja Build Tool](https://github.com/ninja-build/ninja)

The Ninja generator creates a file called 'build.ninja'.

#### Ninja Command Line

```sh
# Run Default Build Targets
ninja

# Run up to six jobs in parallel.
ninja -j 6

# Run specific build targets.
ninja target1 target2
```

Ninja does not support specifying an input file and always uses the 'build.ninja' in the current directory.

Please refer to Ninja's manual for other command line options.

#### Automatic Re-Generation

If a name for the regeneration rule is provided in the configuration,
a build for 'build.ninja' will be added to the output.
This target will cause the setup script to run again with the original options used to generate the file.

```sh
# 'build.ninja' is included in the default build targets
ninja

# Specifically rebuild 'build.ninja' if needed.
ninja build.ninja

# Rebuild 'build.ninja' and other specified targets.
# 'build.ninja' will be built first, others will use the new file.
ninja build.ninja target1 target2

# If none of the targets are 'build.ninja', regeneration will not occur.
ninja target1 target2
```

The 'build.ninja' target will be added to the default targets.
It can also be specifically run by adding it to the target list as shown above.

The regeneration target will run before all other build targets.
However, it will not run unless you use the default target or specifically list it.

Ninja will determine if 'build.ninja' needs to be rebuild based on whether or not any of the
configuration files or the setup script have been modified.

When regenerating 'build.ninja' a backup is always created.
