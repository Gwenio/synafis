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
const _ = {}
_.map = require('lodash/map')
_.reduce = require('lodash/reduce')
_.some = require('lodash/some')
_.partition = require('lodash/partition')
_.set = require('lodash/set')
_.get = require('lodash/get')
_.union = require('lodash/union')
_.replace = require('lodash/replace')
_.flatten = require('lodash/flatten')
_.includes = require('lodash/includes')
_.mapValues = require('lodash/mapValues')
_.groupBy = require('lodash/groupBy')
_.pick = require('lodash/pick')
_.pickBy = require('lodash/pickBy')
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
		/** @type {string} */
		this.action = data.action
		this.id = id
		/** @type {{ [id:string]: string }[]} */
		this.filters = _.get(data, 'filters', [])
		/** @type {{ step: string, [id:string]: any }[]} */
		this.inputs = _.get(data, 'inputs', [])
		this.outputs = _.map(_.get(data, 'outputs', {}),
			(val, key) => _.set(val, 'id', key))
		/** @type {string[]} */
		this.after = _.get(data, 'after', [])
		this.sources = _.get(data, 'sources', [])
	}

	/**
	 * Creates an array of Step objects from a raw JSON object.
	 * @param {Object} raw
	 * @returns {Step[]}
	 */
	static process(raw)
	{
		return _.map(raw, (step, key) => new Step(key, step))
	}

	get_deps()
	{
		return _.union(this.after, _.map(this.inputs, 'step'))
	}

	/**
	 * Sorts an array of Step objects so they will come later than those they depend on.
	 * @param {Step[]} steps
	 * @returns {Step[]}
	 */
	static sort(steps)
	{
		/** @type {{ [id:string]: string[] }} */
		const tree = _.reduce(steps, (acc, item) =>
			_.set(acc, item.id, item.get_deps()), {})
		let [sorted, remaining] = _.partition(steps, (x) => tree[x.id].length === 0)
		while (remaining.length !== 0)
		{
			/** @type {Step[]} */
			let removed; // this semicolon is important
			[remaining, removed] = _.partition(remaining, (x) =>
			{
				const deps = tree[x.id]
				return _.some(remaining, (y) => _.includes(deps, y.id))
			})
			if (removed.length === 0)
			{
				throw new Error("Circular dependency detected in steps.")
			}
			else
			{
				sorted = sorted.concat(removed)
			}
		}
		return sorted
	}

	/**
	 * Prepares the steps for future processing by removing unneeded properties.
	 * @param {Step[]} steps
	 * @returns {{ [id:string]: { action:string, sources:any[], inputs:any[], outputs:any[], after:string[] } }}
	 */
	static cleanup(steps)
	{
		return _.reduce(steps, (acc, x) =>
			_.set(acc, x.id, _.pick(x,
				['action', 'sources', 'inputs', 'outputs', 'after'])), {})
	}
}

/**
 * @type Source
 */
class Source
{
	constructor(src)
	{
		/** @type {string} */
		const prefix = _.get(src, 'prefix', "")
		/** @type {string} */
		const suffix = _.get(src, 'suffix', "")
		/** @type {string[]} */
		const files = _.get(src, 'files', [])
		/** @type {{ [id:string]: string }[]} */
		this.filters = _.get(src, 'filters', [])
		/** @type {string} */
		this.group = _.get(src, 'group', "")
		this.files = _.map(files, (f) =>
			_.replace(`${prefix}${f}${suffix}`, '/', path.sep))
	}

	/**
	 * @param {object[]} raw
	 * @returns {Source[]}
	 */
	static process(raw)
	{
		return _.map(raw, (src) => new Source(src))
	}

	/**
	 * @param {Source[]} sources
	 * @returns {{ [group:string]: string[] }}
	 */
	static merge(sources)
	{
		const groups = _.mapValues(_.groupBy(sources, 'group'),
			(members) => _.union(..._.map(members, 'files')))
		return _.pickBy(groups, (value) => value !== [])
	}
}

module.exports.Step = Step
module.exports.Source = Source
