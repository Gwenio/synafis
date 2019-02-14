// Copyright (c) 2019 James Adam Armstrong
//
// ISC License (ISC)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

"use strict";

// spellcheck: off
const commandLineArgs = require('command-line-args')
const commandLineUsage = require('command-line-usage')
// spellcheck: on

const cmd_args = [
{
	name: 'help',
	alias: 'h',
	type: Boolean,
	defaultValue: false,
	description: 'Displays this help message.'
},
{
	name: 'project',
	alias: 'p',
	type: String,
	typeLabel: '{underline file.json}',
	defaultValue: 'project.json',
	description: 'The file containing a build environment agnostic project description. [project.json]'
},
{
	name: 'config',
	alias: 'c',
	type: String,
	typeLabel: '{underline file.json}',
	defaultValue: 'config.json',
	description: 'The file containing build environment specific configurations. [config.json]'
},
{
	name: 'generate',
	alias: 'g',
	type: String,
	typeLabel: '{underline file.json}',
	defaultValue: 'ninja.json',
	description: 'The file containing configurations for the build generator. [ninja.json]'
},
{
	name: 'root',
	alias: 'r',
	type: String,
	typeLabel: '{underline path}',
	defaultValue: '.',
	description: 'The path to the root of the source code. [./]'
},
{
	name: 'jobs',
	alias: 'j',
	type: Number,
	defaultValue: 4,
	description: 'The number of tasks to run in parallel. [4]'
},
{
	name: 'no-backup',
	type: Boolean,
	defaultValue: false,
	description: 'Prevents the previous build from being backed up. [false]'
},
{
	name: 'verbose',
	alias: 'v',
	type: Number,
	defaultValue: 0
}]

const usage = [
{
	header: 'setup.js',
	content: 'A Node.js script to generate Ninja build files from JSON descriptions of the project.',
},
{
	header: 'Synopsis',
	content: [
		'$ node -- setup.js --help',
		'$ node -- setup.js -p path/project.json -c path/config.json',
		'$ node -- setup.js -r path'
	]
},
{
	header: 'Options',
	optionList: cmd_args,
	hide: ['verbose', 'jobs']
}]

class CLI
{
	constructor()
	{
		const options = commandLineArgs(cmd_args)
		/** @type {boolean} */
		this.help = options.help
		/** @type {boolean} */
		this.backup = !options['no-backup']
		/** @type {string} */
		this.project = options.project
		/** @type {string} */
		this.config = options.config
		/** @type {string} */
		this.generate = options.generate
		/** @type {string} */
		this.root = options.root
		/** @type {number} */
		this.jobs = Math.max(Math.round(options.jobs), 1)
		/** @type {number} */
		this.verbose = Math.round(options.verbose)

	}

	static displayUsage()
	{
		console.log(commandLineUsage(usage))
	}
}

module.exports = CLI
