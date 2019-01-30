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
const path = require('path')
const map = require('lodash/map')
const get = require('lodash/get')
const spread = require('lodash/spread')
const concat = require('lodash/concat')
const merge = require('lodash/merge')
const fs = merge(require('fs'), require('fs')
	.promises)
// spellcheck: on

/**
 * @type Directory
 */
class Directory
{
	/**
	 *
	 * @param {string} id The ID for the Directory.
	 * @param {Object} param1
	 * @param {string} param1.root The constant prefix for the Directory.
	 * @param {Array<any>} param1.filters The filters.
	 * @param {Object} param1.folders The build variation specific sub-folders.
	 */
	constructor(id, { root, filters, folders })
	{
		this.id = id
		this.root = root
		this.filters = filters
		this.folders = folders
	}

	/**
	 * Processes the directory entries from JSON.
	 * @param {Object} raw
	 * @returns {Array<Directory>}
	 */
	static process(raw)
	{
		return map(raw, (val, key) => new Directory(key, val))
	}

	/**
	 * Generates a build variation specific path.
	 * @typedef {Object} Variant
	 * @param {Variant} variant The variant to generate a path for.
	 * @returns {string}
	 */
	location(variant)
	{
		const temp = map(this.folders, (val, key) =>
		{
			const x = variant.get(key)
			return get(val, x, x)
		})
		return spread(path.join)(concat(this.root, temp))
	}

	/**
	 * A work around for Node's fs.existsSync() not working correctly.
	 * @param {string} dir
	 */
	static make(dir)
	{
		if (!fs.existsSync(dir))
		{
			const parent = path.dirname(dir)
			if (!fs.existsSync(parent))
			{
				Directory.make(parent)
			}
			fs.mkdirSync(dir)
		}
	}
}

module.exports = {
	Directory: Directory
}
