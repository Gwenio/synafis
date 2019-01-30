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
const reduce = require('lodash/reduce')
const set = require('lodash/set')
const get = require('lodash/get')
const size = require('lodash/size')
const merge = require('lodash/merge')
const map = require('lodash/map')
const flatMap = require('lodash/flatMap')
const some = require('lodash/some')
const concat = require('lodash/concat')
const partition = require('lodash/partition')
const has = require('lodash/has')
const every = require('lodash/every')
// spellcheck: on

class Shadow
{
	constructor()
	{
		this.shadows = {}
	}

	static process(raw)
	{
		return new Shadow()
	}

	apply(item)
	{
		return set(item, 'filters', reduce(this.shadows, (acc, val, key) =>
		{
			const [y, z] = partition(acc, (a) => has(a, key))
			const b = flatMap(y, (c) =>
				map(val, (d) => set(merge({}, c), key, d)))
			return concat(b, z)
		}, item.filters))
	}
}

class Variant
{
	constructor(data)
	{
		this.data = data
	}

	static process(raw)
	{
		return reduce(raw, (acc, val, key) =>
			set(acc, key, get(val, 'options', [])), {})
	}

	static merge(base, extra)
	{
		let temp = merge({}, base, extra)
		temp = reduce(temp, (acc, val, key) =>
			flatMap(val, (x) => map(acc, (y) =>
				merge(set({}, key, x), y))), [{}])
		return map(temp, (x) => new Variant(x))
	}

	match(item)
	{
		const filters = get(item, 'filters', [])
		return size(filters) === 0 || some(filters, x =>
			every(this.data, (y, key) => !has(x, key) || get(x, key) === y))
	}

	get(tag)
	{
		return this.data[tag]
	}
}

module.exports = {
	Shadow: Shadow,
	Variant: Variant
}
