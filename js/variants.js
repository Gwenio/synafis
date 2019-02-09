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
_.reduce = require('lodash/reduce')
_.set = require('lodash/set')
_.get = require('lodash/get')
_.merge = require('lodash/merge')
_.map = require('lodash/map')
_.flatMap = require('lodash/flatMap')
_.some = require('lodash/some')
_.concat = require('lodash/concat')
_.partition = require('lodash/partition')
_.has = require('lodash/has')
_.every = require('lodash/every')
// spellcheck: on

class Shadow
{
	constructor(input)
	{
		this.shadows = input
	}

	static process(raw)
	{
		return new Shadow(raw)
	}

	/**
	 * @template T
	 * @param {T & { filters: any[] }} item
	 * @returns {T & { filters: any[] }}
	 */
	apply(item)
	{
		return _.set(item, 'filters', _.reduce(this.shadows, (acc, val, key) =>
		{
			const [y, z] = _.partition(acc, (a) => _.has(a, key))
			const b = _.flatMap(y, (c) =>
				_.map(val, (d) => _.set(_.merge({}, c), key, d)))
			return _.concat(b, z)
		}, _.get(item, 'filters', [])))
	}
}

class Variant
{
	/**
	 *
	 * @param {{ [id:string]: string }} data
	 */
	constructor(data)
	{
		this.data = data
	}

	/**
	 * Processes raw input into a form ready to be passed to Variant.merge().
	 * @param {*} raw Raw input.
	 * @returns {{ [id:string]: string[] }}
	 */
	static process(raw)
	{
		return _.reduce(raw, (acc, val, key) =>
			_.set(acc, key, _.get(val, 'options', [])), {})
	}

	/**
	 *
	 * @param {{ [id:string]: string[] }} base
	 * @param {{ [id:string]: string[] }} extra
	 * @returns {Variant[]}
	 */
	static merge(base, extra)
	{
		const merged = _.merge({}, base, extra)
		const temp = _.reduce(merged, (acc, val, key) =>
			_.flatMap(val, (x) => _.map(acc, (y) =>
				_.merge(_.set({}, key, x), y))), [{}])
		return _.map(temp, (x) => new Variant(x))
	}

	/**
	 * @param {{ filters: { [key:string]: string }[] }} item
	 * @returns {boolean}
	 */
	match(item)
	{
		const filters = item.filters
		return filters === [] || _.some(filters, x =>
			_.every(this.data, (y, key) => !_.has(x, key) || _.get(x, key) === y))
	}

	/**
	 * Gets the value for a condition.
	 * @param {string} tag The tag of the condition.
	 * @returns {string}
	 */
	get(tag)
	{
		return this.data[tag]
	}
}

module.exports.Shadow = Shadow
module.exports.Variant = Variant
