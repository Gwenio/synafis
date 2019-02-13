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
const EventEmitter = require('events')
const path = require('path')
const _ = {}
_.each = require('lodash/each')
_.forOwn = require('lodash/forOwn')
_.union = require('lodash/union')
_.keys = require('lodash/keys')
_.set = require('lodash/set')
_.has = require('lodash/has')
_.get = require('lodash/get')
_.partial = require('lodash/partial')
_.bind = require('lodash/bind')
_.concat = require('lodash/concat')
// spellcheck: on

/**
 * @typedef {import('events')} EventEmitter
 * @typedef {import("path").FormatInputPathObject & {origin: string}} SourcePath
 * @typedef {{ path: string, origin: string }} ProductPath
 * @typedef {{implicit: boolean, single: boolean, files: ProductPath[]}} Target
 * @typedef {{step: string, group: string, implicit: boolean, produces?: string}} InputGroup
 * @typedef {{extension: string, location: string, implicit: boolean, single: boolean, products: string[]}} OutputGroup
 */

/**
 * @param group
 * @param {number} index
 * @param {string} dir
 * @param {string} ext
 * @param {number} select
 * @param {{name:string, origin: string}} file
 * @this {EventEmitter}
 */
function rename_file(group, index, dir, ext, select, { name, origin })
{
	if (select === index)
	{
		this.emit(group,
		{
			dir: path.dirname(path.join(dir, origin)),
			ext,
			name,
			origin
		})
	}
}

/**
 * @param {{ [id:string]: {implicit: boolean, produces?: string, files: SourcePath[]}[] }} sources
 * @param {{ [id:string]: { [key:string]: {implicit: boolean, files: SourcePath[]} }}} products
 * @param {{ [id:string]: InputGroup[] }} i
 * @param {{ [id:string]: { [key:string]: OutputGroup } }} o
 */
function process_build(sources, products, i, o)
{
	const inputs = {}
	/** @type {{ [id:string]: { [key:string]: Target } }} */
	const outputs = {}
	/** @type {{ [id:string]: EventEmitter }} */
	const oports = {}
	/** @type {EventEmitter} */
	const iport = new EventEmitter()
	_.each(_.union(_.keys(products), _.keys(o)), (id) =>
	{
		_.set(outputs, id, {})
		_.set(oports, id, new EventEmitter())
	})
	/**
	 * @param {{ [key: string]: {implicit: boolean, single?: boolean}}} step
	 * @param {string} id
	 */
	function olink(step, id)
	{
		const target = outputs[id]
		const port = oports[id]
		_.forOwn(step, ({ implicit, single }, key) =>
		{
			if (!_.has(target, key))
			{
				_.set(target, key, { implicit, single, files: [] })
				port.on(key, _.partial(
					/**
					 * @param {{ path: string, origin: string }[]} list
					 * @param {SourcePath} file
					 */
					(list, file) =>
					{
						list.push(
						{
							path: path.format(file),
							origin: file.origin
						})
					}, target[key].files))
			}
		})
	}
	_.forOwn(products, olink)
	_.forOwn(o, olink)
	_.each(_.union(_.keys(sources), _.keys(i)), (id) =>
	{
		/** @type {{implicit: boolean, files: ProductPath[] }[]} */
		const step = _.set(inputs, id, [])[id]
		iport.on(id, (index, file) =>
		{
			step[index].files.push(
			{
				path: path.format(file),
				origin: file.origin
			})
		})
		_.each(_.concat(_.get(sources, id, []), _.get(i, id, [])),
			/**
			 * @param {{implicit:boolean, produces?:string, step?:string, group?:string}} src
			 * @param {number} index
			 */
			(src, index) =>
			{
				step.push({ implicit: src.implicit, files: [] })
				if (_.has(src, 'produces'))
				{
					const group = src.produces
					const { location, extension } = _.get(o, `${id}.${group}`,
					{
						location: '',
						extension: ''
					})
					iport.on(id, _.bind(rename_file, oports[id], group, index, location, extension))
				}
				if (_.has(src, 'step') && _.has(src, 'group'))
				{
					oports[src.step].on(src.group,
						/**
						 * @param {SourcePath} file
						 */
						(file) =>
						{
							iport.emit(id, index, file)
						})
				}
			})
	})
	_.forOwn(products, (step, id) =>
	{
		const port = oports[id]
		_.forOwn(step, ({ files }, group) =>
		{
			_.each(files, (x) => port.emit(group, x))
		})
	})
	_.forOwn(sources, (step, id) =>
	{
		_.each(step, ({ files }, index) =>
		{
			_.each(files, (x) => iport.emit(id, index, x))
		})
	})
	return { inputs, outputs }
}

module.exports = process_build
