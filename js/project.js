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
const map = require('lodash/map')
const reduce = require('lodash/reduce')
const some = require('lodash/some')
const partition = require('lodash/partition')
const set = require('lodash/set')
const get = require('lodash/get')
const size = require('lodash/size')
const union = require('lodash/union')
const replace = require('lodash/replace')
const flatten = require('lodash/flatten')
const includes = require('lodash/includes')
const filter = require('lodash/filter')
const uniq = require('lodash/uniq')
const path = require('path')
// spellcheck: on

/**
 * @type Step
 */
class Step
{
	/**
	 * Constructs a Step from input.
	 * @param {string} id The Step ID.
	 * @param {Object} data
	 */
	constructor(id, data)
	{
		this.action = data.action
		this.id = id
		this.filters = get(data, 'filters', [])
		this.inputs = get(data, 'inputs', [])
		this.outputs = map(get(data, 'outputs', {}),
			(val, key) => set(val, 'id', key))
		this.after = get(data, 'after', [])
		this.sources = get(data, 'sources', [])
	}

	/**
	 * Creates an array of Step objects from a raw JSON object.
	 * @param {Object} raw
	 * @returns {Array<Step>}
	 */
	static process(raw)
	{
		return map(raw, (step, key) => new Step(key, step))
	}

	get_deps()
	{
		return union(this.after, map(this.inputs, 'step'))
	}

	/**
	 * Sorts an array of Step objects so they will come later than those they depend on.
	 * @param {Array<Step>} steps
	 * @returns {Array<Step>}
	 */
	static sort(steps)
	{
		const tree = reduce(steps, (acc, item) =>
			set(acc, item.id, item.get_deps()), {})
		let [temp, remaining] = partition(steps, (x) => tree[x.id] === [])
		let sorted = [temp]
		while (size(remaining) > 0)
		{
			let removed; // this semicolon is important
			[remaining, removed] = partition(remaining, (x) =>
			{
				const deps = tree[x.id]
				return some(remaining, (y) => includes(deps, y.id))
			})
			if (size(removed) === 0)
			{
				throw new Error("Circular dependency detected in steps.")
			}
			else
			{
				sorted.push(removed)
			}
		}
		return flatten(sorted)
	}
}

/**
 * @type Source
 */
class Source
{
	constructor(src)
	{
		const prefix = get(src, 'prefix', "")
		const suffix = get(src, 'suffix', "")
		const files = get(src, 'files', [])
		this.filters = get(src, 'filters', [])
		this.group = get(src, 'group', "")
		this.files = map(files, (f) =>
			replace(prefix + f + suffix, '/', path.sep))
	}

	/**
	 * @param {Array<Object>} raw
	 * @returns {Array<Source>}
	 */
	static process(raw)
	{
		return map(raw, (src) => new Source(src))
	}

	/**
	 * @param {Array<Source>} sources
	 * @returns {Object}
	 */
	static merge(sources)
	{
		const groups = uniq(map(sources, 'group'))
		const result = reduce(groups, (acc, x) =>
		{
			const members = filter(sources, ['group', x])
			const files = union.apply(null, map(members, 'files'))
			return (size(files) < 1) ? acc : set(acc, x, files)
		}, {})
		return result
	}
}

module.exports = {
	Step: Step,
	Source: Source
}
