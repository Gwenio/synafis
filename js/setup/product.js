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
_.flatMap = require('lodash/flatMap')
_.map = require('lodash/map')
_.get = require('lodash/get')
_.join = require('lodash/join')
_.concat = require('lodash/concat')
// spellcheck: on

/**
 * @typedef {import('../variants').Variant} Variant
 */

class Product
{
	/**
	 * @param {string} id
	 * @param {Object} param1
	 */
	constructor(id, { base, filters, separator, prefix, suffix })
	{
		/** @type {string} */
		this.id = id
		/** @type {string} */
		this.base = base
		/** @type {{ [tag:string]: string }[]} */
		this.filters = filters
		/** @type {string} */
		this.separator = separator
		this.prefix = prefix
		this.suffix = suffix
	}

	static process(raw)
	{
		return _.flatMap(raw, (list, key) =>
			_.map(list, (item) =>
				new Product(key, item)))
	}

	/**
	 * @param {Variant} variant
	 */
	partition(variant)
	{
		/**
		 * @param {{ [x:string]: string }} val
		 * @param {string} key
		 * @returns {string}
		 */
		const convert = (val, key) =>
		{
			const x = variant.get(key)
			return _.get(val, x, x)
		}
		const pre = _.map(this.prefix, convert)
		const post = _.map(this.suffix, convert)
		return _.join(_.concat(pre, this.base, post), this.separator)
	}
}

module.exports = Product
