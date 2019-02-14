#!/usr/bin/env node

// Copyright (c) 2018-2019 James Adam Armstrong
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
const fs = require('fs')
const _ = {}
_.isNil = require('lodash/isNil')
const Ninja = require('./js/ninja')
const { Pipeline } = require('./js/pipeline')
const CLI = require('./js/setup/cli')
const tasks = require('./js/setup/tasks')
// spellcheck: on

const options = new CLI()

if (options.help)
{
	CLI.displayUsage()
}
else
{
	const output_filename = 'build.ninja'

	const temp_filename = `${output_filename}.tmp`

	let writer = new Ninja.Writer(temp_filename)

	const driver = new Pipeline("driver")
	tasks(driver, options, writer)
	driver.onFailure((stage, error) =>
	{
		if (!_.isNil(stage))
		{
			stage = String(stage)
			console.warn(`Failure encountered in stage '${stage}' of the Pipeline.`)
		}
		console.error(error)
		process.exitCode = 1
	})
	if (options.verbose > 0)
	{
		driver.applyLogger((stage, result) =>
		{
			console.log(`Completed stage ${stage}.`)
			console.log(result)
			console.log("")
		})
	}
	driver.onComplete(() => { console.log('Completed.') })
	process.exitCode = 0
	driver.run()
		.finally(() =>
		{
			writer.close()
			if (process.exitCode === 0)
			{
				if (options.backup && fs.existsSync(output_filename))
				{
					fs.renameSync(output_filename, `${output_filename}.bak`)
				}
				fs.renameSync(temp_filename, output_filename)
			}
			else
			{
				fs.unlinkSync(temp_filename)
			}
		})

}
