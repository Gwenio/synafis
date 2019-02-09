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
_.merge = require('lodash/merge')
_.map = require('lodash/map')
_.reduce = require('lodash/reduce')
_.find = require('lodash/find')
_.reject = require('lodash/reject')
_.some = require('lodash/some')
_.concat = require('lodash/concat')
_.partition = require('lodash/partition')
_.set = require('lodash/set')
_.get = require('lodash/get')
_.each = require('lodash/forEach')
_.size = require('lodash/size')
_.pick = require('lodash/pick')
_.isNil = require('lodash/isNil')
_.join = require('lodash/join')
const { promises: fs } = require('fs')
const path = require('path')
// spellcheck: on

/** @returns {{ [id:string]: number }} */
function process_pools(raw)
{
	return _.reduce(raw, (acc, cur, key) =>
		_.set(acc, key, Math.round(_.get(cur, 'depth', 1))), {})
}

/** @returns {{ [id:string]: string }} */
function process_def_variables(raw)
{
	return _.reduce(raw, (acc, cur, id) =>
		_.set(acc, id, _.get(cur, 'default', '')), {})
}

/** @returns {{ [id:string]: object[] }} */
function process_variables(raw)
{
	return _.reduce(raw, (acc, cur, id) =>
		_.set(acc, id, _.get(cur, 'specific', [])), {})
}

function process_rules(raw)
{
	return _.reduce(raw, (acc, val, key) =>
		_.set(acc, key, _.get(val, 'vars', '')), {})
}

function process_actions(raw)
{
	return _.map(raw, (val, key) =>
	{
		return {
			filters: _.get(val, 'filters', []),
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
			if (!_.isNil(this.file))
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
		if (!(_.isNil(value) || value === ''))
		{
			this.write(`${indent ? '  ' : ''}${name} = ${value}\n`)
		}
		return this
	}

	/**
	 * Outputs a pool definition to the Ninja file.
	 * @param {number} depth The 'depth' of the pool.
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
		_.each(variables, (x, y) => this.variable(y, x, true))
		return this
	}

	/**
	 * @private
	 * @param {string} pre
	 * @param {string[]} items
	 * @returns {Writer} this
	 */
	buildList(pre, items)
	{
		if (items.length !== 0)
		{
			this.write(pre)
			_.each(items, (val) => this.write(` ${val}`))
		}
		return this
	}

	/**
	 * Outputs a build entry to the Ninja file.
	 * @param {Build} build
	 * @returns {Writer} this
	 */
	build({ action, outputs, implicit, inputs, depends, after, vars })
	{
		this.write("build")
		_.each(outputs, (val) => this.write(` ${val}`))
		this.buildList(" |", implicit)
			.write(`: ${action}`)
		_.each(inputs, (val) => this.write(` ${val}`))
		this.buildList(" |", depends)
			.buildList(" ||", after)
			.write("\n")
		_.each(vars, (val, key) => this.variable(key, val, true))
		return this
	}

	/**
	 * Outputs a default line to the Ninja file.
	 * @param {string[]} targets The build targets to be defauls.
	 * @returns {Writer} this
	 */
	defs(targets)
	{
		if (targets.length !== 0)
		{
			this.write(`default ${_.join(targets, ' ')}\n`)
		}
		return this
	}
}

/**
 * @typedef {{path: string, origin: string}} ProductPath
 */

/**
 * @type Build
 */
class Build
{
	/**
	 *
	 * @param {string} action
	 * @param {{path: string}[]} outputs
	 * @param {{path: string}[]} implicit
	 * @param {{path: string}[]} inputs
	 * @param {{path: string}[]} depends
	 * @param {{path: string}[]} after
	 * @param {{ [name: string]: string }} vars
	 */
	constructor(action, outputs, implicit, inputs, depends, after, vars)
	{
		this.action = action
		this.outputs = _.map(outputs, 'path')
		this.implicit = _.map(implicit, 'path')
		this.inputs = _.map(inputs, 'path')
		this.depends = _.map(depends, 'path')
		this.after = _.map(after, 'path')
		this.vars = vars
	}

	/**
	 * Maps build steps to Ninja rules based on their action.
	 * @typedef {{ [id:string]: string }} ActionMap
	 * @param {ActionMap} acts
	 * @param {{ [id:string]: { action: string }}} steps
	 * @returns {ActionMap}
	 */
	static process_actions(acts, steps)
	{
		/**
		 * @param {ActionMap} acc
		 * @param {Object} step
		 * @param {string} step.action
		 * @param {string} id
		 * @returns {ActionMap} acc
		 */
		function reduction(acc, { action }, id)
		{
			return _.set(acc, id, acts[action])
		}
		return _.reduce(steps, reduction, {})
	}

	/**
	 *
	 * @param {string} root
	 * @param {*} steps
	 * @param {*} sources
	 */
	static process_sources(root, steps, sources)
	{
		return _.reduce(steps, (acc, item, id) =>
			_.set(acc, id, _.map(item.sources, (src) =>
			{
				const files = _.map(_.map(_.get(sources, src.group, []),
						(file) => path.parse(path.join(root, file))), (file) =>
					_.set(file, 'origin', path.join(file.dir, file.name)))
				return _.merge({ files: files },
					_.pick(src, ['implicit', 'produces']))
			})), {})
	}

	/**
	 * @param {any} names
	 * @param {any} steps
	 */
	static process_products(names, steps)
	{
		return _.reduce(steps, (outer, outputs, key) =>
		{
			const temp = _.reduce(outputs,
				(inner, { implicit, products, location, extension }, id) =>
				(_.isNil(products) || products === []) ? inner :
				_.set(inner, id,
				{
					implicit,
					files: _.map(products, (name) =>
					{
						const origin = _.get(names, name, name)
						return {
							dir: location,
							name: origin,
							ext: extension,
							origin
						}
					})
				}), {})
			return _.size(temp) < 1 ? outer : _.set(outer, key, temp)
		}, {})
	}

	/**
	 *
	 * @param {String} name
	 * @param {ProductPath[]} inputs
	 * @returns {Build}
	 */
	static alias(name, inputs)
	{
		return new Build('phony', [{ path: name }], [], inputs, [], [], {})
	}

	/**
	 *
	 * @param {String} action
	 * @param {{single: ProductPath[], multiple: ProductPath[]}} outputs
	 * @param {ProductPath[]} implicit
	 * @param {ProductPath[]} inputs
	 * @param {ProductPath[]} depends
	 * @param {ProductPath[]} after
	 * @param {{ [name: string]: string }} vars
	 * @returns {Build[]}
	 */
	static create(action, { single, multiple }, implicit, inputs, depends, after, vars = {})
	{
		if (single.length === 0)
		{
			return [new Build(action, multiple, implicit, inputs, depends, after, vars)]
		}
		else
		{
			let [isingle, imultiple] = _.partition(inputs, (x) =>
				_.some(single, ['origin', x.origin]))
			return _.map(single,
				/**
				 * @param {ProductPath} target
				 */
				({ path: fpath, origin }) =>
				{
					let o = _.reject(multiple, ['origin', origin])
					let m = _.reject(implicit, ['origin', origin])
					let i = _.find(isingle, ['origin', origin])
					let d = _.reject(depends, ['origin', origin])
					let a = _.reject(after, ['origin', origin])
					return new Build(action, _.concat(o, [{ path: fpath }]), m,
						_.concat(imultiple, i), d, a, vars)
				})
		}
	}
}

module.exports = {
	process_actions: process_actions,
	process_rules: process_rules,
	process_variables: process_variables,
	process_def_variables: process_def_variables,
	process_pools: process_pools
}

module.exports.Writer = Writer
module.exports.Build = Build
