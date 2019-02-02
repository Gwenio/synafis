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
const merge = require('lodash/merge')
const map = require('lodash/map')
const reduce = require('lodash/reduce')
const find = require('lodash/find')
const reject = require('lodash/reject')
const some = require('lodash/some')
const concat = require('lodash/concat')
const partition = require('lodash/partition')
const set = require('lodash/set')
const get = require('lodash/get')
const each = require('lodash/forEach')
const size = require('lodash/size')
const pick = require('lodash/pick')
const isNil = require('lodash/isNil')
const join = require('lodash/join')
const { promises: fs } = require('fs')
const path = require('path')
// spellcheck: on

function process_pools(raw)
{
	return reduce(raw, (acc, cur, key) =>
		set(acc, key, Math.round(get(cur, 'depth', 1))), {})
}

function process_def_variables(raw)
{
	return reduce(raw, (acc, cur, id) =>
		set(acc, id, get(cur, 'default', '')), {})
}

function process_variables(raw)
{
	return reduce(raw, (acc, cur, id) =>
		set(acc, id, get(cur, 'specific', [])), {})
}

function process_rules(raw)
{
	return reduce(raw, (acc, val, key) =>
		set(acc, key, get(val, 'vars', '')), {})
}

function process_actions(raw)
{
	return map(raw, (val, key) =>
	{
		return {
			filters: get(val, 'filters', []),
			action: val.action,
			id: key
		}
	})
}

class Writer
{
	/**
	 * Constructs a new Ninja file Writer.
	 * @param {string} filename The name of the output file.
	 */
	constructor(filename)
	{
		this.file = fs.open(filename, 'w')
		this.order = this.file.then((f) => { this.file = f; return })
	}

	close()
	{
		this.order = this.order.finally(async () =>
		{
			if (!isNil(this.file))
			{
				let temp = this.file
				this.file = null
				await temp.sync()
				return temp.close()
			}
		})
	}

	/**
	 * Writes text to the Ninja file.
	 * @param {string} data Text to output.
	 * @private
	 */
	write(data)
	{
		this.order = this.order.then(() => this.file.writeFile(data))
		return this
	}

	/**
	 * Outputs a variable line to the Ninja file.
	 * @param {string} name The name of the Ninja variable.
	 * @param {string | null} value The value of the Ninja variable.
	 * @param {boolean} indent When true the variable line will be indented two spaces.
	 * @returns {Writer} this
	 */
	variable(name, value, indent)
	{
		if (!(isNil(value) || value === ''))
		{
			this.write(`${indent ? '  ' : ''} ${name} = ${value}\n`)
		}
		return this
	}

	/**
	 * Outputs a pool definition to the Ninja file.
	 * @param {Number} depth The 'depth' of the pool.
	 * @param {string} key The name of the pool.
	 * @returns {Writer} this
	 */
	pool(depth, key)
	{
		this.write(`pool ${key}\n`)
		return this.variable('depth', depth.toString(), true)
	}

	/**
	 * Outputs a rule definition to the Ninja file.
	 * @param {Object} variables The variables for the rule.
	 * @param {string} key The name of the rule.
	 * @returns {Writer} this
	 */
	rule(variables, key)
	{
		this.write(`rule ${key}\n`)
		each(variables, (x, y) => this.variable(y, x, true))
		return this
	}

	/**
	 *
	 * @param {string} pre
	 * @param {Array<string>} items
	 * @returns {Writer} this
	 */
	buildList(pre, items)
	{
		if (size(items) > 0)
		{
			this.write(pre)
			each(items, (val) => this.write(` ${val}`))
		}
		return this
	}

	/**
	 * Outputs a build entry to the Ninja file.
	 * @param {Build} param0
	 * @returns {Writer} this
	 */
	build({ action, outputs, implicit, inputs, depends, after, vars })
	{
		this.write("build")
		each(outputs, (val) => this.write(` ${val}`))
		this.buildList(" |", implicit)
			.write(`: ${action}`)
		each(inputs, (val) => this.write(` ${val}`))
		this.buildList(" |", depends)
			.buildList(" ||", after)
			.write("\n")
		each(vars, (val, key) => this.variable(key, val, true))
		return this
	}

	/**
	 * Outputs a default line to the Ninja file.
	 * @param {Array<string>} targets The build targets to be defauls.
	 * @returns {Writer} this
	 */
	defs(targets)
	{
		if (size(targets) > 0)
		{
			this.write(`default ${join(targets, ' ')}\n`)
		}
		return this
	}
}

/**
 * @type Build
 */
class Build
{
	/**
	 *
	 * @param {String} action
	 * @param {*} outputs
	 * @param {*} implicit
	 * @param {*} inputs
	 * @param {*} depends
	 * @param {*} after
	 * @param {*} vars
	 */
	constructor(action, outputs, implicit, inputs, depends, after, vars)
	{
		this.action = action
		this.outputs = map(outputs, 'path')
		this.implicit = map(implicit, 'path')
		this.inputs = map(inputs, 'path')
		this.depends = map(depends, 'path')
		this.after = map(after, 'path')
		this.vars = vars
	}

	/**
	 * Maps build steps to Ninja rules based on their action.
	 * @param {Object} acts
	 * @param {Object} steps
	 * @returns {Object}
	 */
	static process_actions(acts, steps)
	{
		/**
		 * @param {Object} acc
		 * @param {Object} step
		 * @param {string} step.action
		 * @param {string} id
		 * @returns {Object} acc
		 */
		function reduction(acc, { action }, id)
		{
			return set(acc, id, acts[action])
		}
		return reduce(steps, reduction, {})
	}

	static process_sources(root, sources, steps)
	{
		return reduce(steps, (acc, item, id) =>
			set(acc, id, map(item.sources, (src) =>
			{
				const files = map(map(get(sources, src.group, []),
						(file) => path.parse(path.join(root, file))), (file) =>
					set(file, 'origin', path.join(file.dir, file.name)))
				return merge({ files: files },
					pick(src, ['implicit', 'produces']))
			})), {})
	}

	static process_products(names, steps)
	{
		return reduce(steps, (outer, outputs, key) =>
		{
			const temp = reduce(outputs,
				(inner, { implicit, products, location, extension }, id) =>
				(isNil(products) || size(products) < 1) ? inner :
				set(inner, id,
				{
					implicit,
					files: map(products, (name) =>
					{
						const origin = get(names, name, name)
						return {
							dir: location,
							name: origin,
							ext: extension,
							origin
						}
					})
				}), {})
			return size(temp) < 1 ? outer : set(outer, key, temp)
		}, {})
	}

	/**
	 *
	 * @param {String} name
	 * @param {*} inputs
	 * @returns {Build}
	 */
	static alias(name, inputs)
	{
		return new Build('phony', [{ path: name }],
			[], inputs, [], [], {})
	}

	/**
	 *
	 * @param {String} action
	 * @param {Object} param1
	 * @param {*} param1.single
	 * @param {*} param1.multiple
	 * @param {*} implicit
	 * @param {*} inputs
	 * @param {*} depends
	 * @param {*} after
	 * @param {*} vars
	 * @returns {Array<Build>}
	 */
	static create(action, { single, multiple }, implicit, inputs, depends, after, vars = {})
	{
		if (size(single) < 1)
		{
			return [new Build(action, multiple, implicit, inputs, depends, after, vars)]
		}
		else
		{
			let [isingle, imultiple] = partition(inputs, (x) =>
				some(single, ['origin', x.origin]))
			return map(single, ({ path: fpath, origin }) =>
			{
				let o = reject(multiple, ['origin', origin])
				let m = reject(implicit, ['origin', origin])
				let i = find(isingle, ['origin', origin])
				let d = reject(depends, ['origin', origin])
				let a = reject(after, ['origin', origin])
				return new Build(action, concat(o, { path: fpath }), m,
					concat(imultiple, i), d, a, vars)
			})
		}
	}
}

module.exports = {
	process_actions: process_actions,
	process_rules: process_rules,
	process_variables: process_variables,
	process_def_variables: process_def_variables,
	process_pools: process_pools,
	Writer: Writer,
	Build: Build
}
