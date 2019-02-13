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
_.concat = require('lodash/concat')
_.map = require('lodash/map')
_.flatMap = require('lodash/flatMap')
_.reduce = require('lodash/reduce')
_.get = require('lodash/get')
_.set = require('lodash/set')
_.has = require('lodash/has')
_.pick = require('lodash/pick')
_.omit = require('lodash/omit')
_.values = require('lodash/values')
_.update = require('lodash/update')
_.filter = require('lodash/filter')
_.isNil = require('lodash/isNil')
_.isArray = require('lodash/isArray')
_.some = require('lodash/some')
_.size = require('lodash/size')
_.find = require('lodash/find')
_.each = require('lodash/forEach')
_.union = require('lodash/union')
_.unionBy = require('lodash/unionBy')
_.uniq = require('lodash/uniq')
_.uniqBy = require('lodash/uniqBy')
const path = require('path')
const { Shadow, Variant } = require('../variants')
const { Step, Source } = require('../project')
const Directory = require('../directory')
const read_json_file = require('../json_input')
const Ninja = require('../ninja')
/** @kind class */
const Product = require('./product')
const process_build = require('./process_build')
// spellcheck: on

/** @typedef {import('../pipeline').Pipeline} Pipeline */
/**
 * @template Result
 * @typedef {import('../pipeline').Task<Result>} Task
 */
/**
 * @template Result, Previous
 * @typedef {import('../pipeline').Stage<Result, Previous>} Stage
 */

/**
 * @typedef {Object} Options
 * @typedef {{ [id: string]: {options: string[]}}} RawVariants
 * @typedef {{variants: RawVariants, sources: any, steps: any}} ProjectInput
 * @typedef {Object} ConfigInput
 * @typedef {Object} NinjaInput
 * @typedef {{ path: string, origin: string }} ProductPath
 */

/**
 * @template T
 * @param {(T & { filters: any[] })[]} items
 * @param {Shadow} shadows
 * @returns {T[]}
 */
function apply_shadows(items, shadows)
{
	return _.map(items, (item) => shadows.apply(item))
}

/**
 * @template T, P
 * @param {Stage<T[], P>} current
 * @param {Promise<Shadow>} shadows
 * @returns {Stage<T[], T[]>}
 */
function shadow_stage(current, shadows)
{
	return current.stage(apply_shadows, shadows)
}

/**
 * @template T
 * @param {Task<T[]>} current
 * @param {Promise<Shadow>} shadows
 * @returns {Stage<T[], T[]>}
 */
function shadow_task(current, shadows)
{
	return current.stage(apply_shadows, shadows)
}

/**
 * @template T
 * @param {Variant} by
 * @param {(T & { filters: { [key:string]: string }[] })[]} from
 * @returns {T[]}
 */
function by_variant(by, from)
{
	return _.filter(from, (x) => by.match(x))
}

/**
 *
 * @param {Pipeline} driver
 * @param {Options} options
 * @param {Ninja.Writer} writer
 */
const prepare_tasks = (driver, options, writer) =>
{
	/** @type Promise<ProjectInput | null> */
	const Project = driver.task('Project',
			/** @returns {Promise<ProjectInput>} */
			function() { return read_json_file(options.project) })
		.complete()
	const Config = driver.task('Config',
			/** @returns {Promise<ConfigInput>} */
			function() { return read_json_file(options.config) })
		.complete()
	const Gen = driver.task('NinjaConfig',
			/** @returns {Promise<NinjaInput>} */
			function()
			{
				return read_json_file(options.generate)
			})
		.complete()
	const HasRegen = driver.task('HasRegen',
			function(x) { return !_.isNil(_.get(x, 'regenerate', null)) }, Gen)
		.complete()
	const Variants = driver.task('Variants',
			function({ variants: proj }, { variants: cfg })
			{
				return Variant.merge(Variant.process(proj), Variant.process(cfg))
			}, Project, Config)
		.complete()
	const Shadows = driver.task('Shadows', function() { return Shadow.process({}) }, Config)
		.complete()
	const Steps = driver.task('Steps',
			function({ steps }) { return Step.process(steps) }, Project)
		.stage(function(steps) { return Step.sort(steps) })
		.apply(shadow_stage, Shadows)
		.partition(Variants, by_variant)
		.partitioned(Step.cleanup)
		.complete()
	const Sources = driver.task('Sources',
			function({ sources })
			{
				return Source.process(sources)
			}, Project)
		.apply(shadow_task, Shadows)
		.partition(Variants, by_variant)
		.stage(
			function(parts)
			{
				return _.map(parts, (s) => Source.merge(s))
			})
		.partitioned(function(x, y)
		{
			return Ninja.Build.process_sources(options.root, y, x)
		}, Steps)
		.complete()
	const Actions = driver.task('Actions',
			function(ninja)
			{
				return Ninja.process_actions(ninja.rules)
			}, Gen)
		.partition(Variants, by_variant)
		.partitioned((acts) =>
			_.reduce(acts,
				(acc, { action, id }) => _.set(acc, action, id), {}))
		.partitioned(Ninja.Build.process_actions, Steps)
		.complete()
	const Destinations = driver.task('Destinations',
			function(config)
			{
				return Directory.process(config.destinations)
			}, Config)
		.partition(Variants, by_variant)
		.partitioned((dirs, variant) =>
			_.reduce(dirs, (acc, dir) =>
				_.set(acc, dir.id, dir.location(variant)), {}), Variants)
		.complete()
	driver.task('MkDirs',
			function(locations, ninja)
			{
				const dirs = _.reduce(_.map(locations, _.values),
					(acc, val) => _.union(acc, val),
					[ninja.directory])
				_.each(dirs, (dir) => Directory.make(path.normalize(dir)))
			}, Destinations, Gen)
		.complete()
	const Extensions = driver.task('Extensions',
			function(config) { return config.extensions }, Config)
		.stage(function(exts, variants)
		{
			return _.map(variants, (v) =>
				_.reduce(exts, (acc, val, key) =>
					_.set(acc, key, _.get(_.find(val, (y) => v.match(y)), 'value', '')), {}))
		}, Variants)
		.complete()
	const StepInputs = driver.partitioned('StepInputs', (steps) =>
			_.reduce(steps, (acc, { inputs }, id) =>
				(_.size(inputs) < 1) ? acc : _.set(acc, id, inputs), {}), Steps)
		.complete()
	const StepAfter = driver.partitioned('StepAfter', (part) =>
			_.reduce(part, (acc, { after }, id) =>
				_.set(acc, id, after), {}), [], Steps)
		.complete()
	const StepOutputs = driver.partitioned('StepOutputs',
			/**
			 * @param {{ [key: string]: { outputs: object[] } }} steps
			 * @param {{ [x: string]: string }} exts
			 * @param {{ [x: string]: string }} dirs
			 */
			(steps, exts, dirs) =>
			_.reduce(steps, (outer, { outputs }, key) =>
			{
				return _.set(outer, key, _.reduce(outputs,
					/**
					 * @param {Object} inner
					 * @param {Object} val
					 * @param {string} val.id
					 * @param {string} val.location
					 * @param {string} val.extension
					 * @param {boolean} val.implicit
					 * @param {boolean} val.single
					 * @param {string[]} val.products
					 */
					(inner, { id, location, extension, implicit, single, products }) =>
					{
						return _.set(inner, id,
						{
							location: _.get(dirs, location, ''),
							extension: _.get(exts, extension, ''),
							implicit,
							single,
							products
						})
					}, {}))
			}, {}), Steps, Extensions, Destinations)
		.complete()
	const Products = driver.task('Products',
			function(config) { return Product.process(config.products) }, Config)
		.partition(Variants, by_variant)
		.partitioned((products, variant) =>
			_.reduce(_.uniqBy(products, 'id'),
				(acc, item) => _.set(acc, item['id'], item['partition'](variant)), {}), Variants)
		.partitioned(Ninja.Build.process_products, StepOutputs)
		.complete()
	const StepOutFilt = driver.partitioned('StepOutputsFilter',
			/** @returns {{ [id: string]: { [key: string]: Object } }} */
			(steps, inputs, sources) =>
			{
				return _.reduce(steps, (outer, outputs, id) =>
					_.set(outer, id, _.reduce(outputs, (inner, val, key) =>
						_.some(_.concat(_.get(inputs, id, []),
							_.get(sources, id, [])), ['produces', key]) ?
						_.set(inner, key, _.omit(val, ['products'])) :
						inner, {})), {});
			}, StepOutputs, StepInputs, Sources)
		.complete()
	const ProcessBuild = driver.partitioned('ProcessBuild',
			process_build, Sources, Products, StepInputs, StepOutFilt)
		.complete()
	const BuildOutputs = driver.partitioned('BuildOutputs',
			/**
			 * @typedef {{ single: ProductPath[], multiple: ProductPath[], implicit: ProductPath[] }} Output
			 * @returns {{ [id:string]: Output }}
			 */
			({ outputs }) =>
			_.reduce(outputs, (outer, group, id) =>
			{
				/** @type {Output} */
				const temp = _.reduce(group, (inner, val) =>
				{
					if (_.size(val.files) > 0)
					{
						if (val.implicit)
						{
							return _.update(inner, 'implicit',
								(old) => _.union(old, val.files))
						}
						else
						{
							return _.update(inner, val.single ? 'single' : 'multiple',
								(old) => _.union(old, val.files))
						}
					}
					else return inner
				}, { single: [], multiple: [], implicit: [] })
				return (_.size(temp.single) + _.size(temp.multiple) + _.size(temp.implicit)) > 0 ?
					_.set(outer, id, temp) : outer
			}, {}), ProcessBuild)
		.complete()
	const BuildInputs = driver.partitioned('BuildOutputs',
			/**
			 * @typedef {{ explicit: ProductPath[], implicit: ProductPath[] }} Input
			 * @returns {{ [id:string]: Input }}
			 */
			({ inputs }) =>
			_.reduce(inputs, (outer, group, id) =>
			{
				/** @type {Input} */
				const temp = _.reduce(group, (inner, val) =>
				{
					if (_.size(val.files) > 0)
					{
						if (val.implicit)
						{
							return _.update(inner, 'implicit',
								(old) => _.union(old, val.files))
						}
						else
						{
							return _.update(inner, 'explicit',
								(old) => _.union(old, val.files))
						}
					}
					else return inner
				}, { explicit: [], implicit: [] })
				return (_.size(temp.explicit) + _.size(temp.implicit)) > 0 ?
					_.set(outer, id, temp) : outer
			}, {}), ProcessBuild)
		.complete()
	const BuildAfter = driver.partitioned('BuildAfter',
			(part, outs) =>
			_.reduce(part, (acc, list, step) =>
				_.set(acc, step, _.uniq(_.flatMap(list, (id) =>
					_.concat(_.get(outs, `${id}.implicit`, []),
						_.get(outs, `${id}.single`, []),
						_.get(outs, `${id}.multiple`, []))))), {}),
			StepAfter, BuildOutputs)
		.stage(function(after, regen)
		{
			if (regen)
			{
				return _.map(after, (part) =>
					_.map(part, (step) => _.concat(step, { path: 'build.ninja' })))
			}
			return after
		}, HasRegen)
		.complete()
	const NinjaHeader = driver.task('NinjaHeader',
			function(ninja)
			{
				/** @type string | null */
				const version = _.get(ninja, 'version', null)
				writer.variable('ninja_required_version',
					version, false)
				/** @type string | null */
				const directory = _.get(ninja, 'directory', null)
				writer.variable('builddir',
					directory, false)
				return { version, directory }
			}, Gen)
		.complete()
	const NinjaPools = driver.task('NinjaPools',
			/** @returns {{ [name:string]: number }} */
			(ninja) => Ninja.process_pools(ninja.pools), Gen)
		.stage(function(pools)
		{
			_.each(pools, (val, key) => writer.pool(val, key))
			return pools
		}, NinjaHeader)
		.complete()
	const NinjaDefVars = driver.task('NinjaDefVars',
			function(ninja)
			{
				return Ninja.process_def_variables(ninja.variables)
			}, Gen)
		.stage(function(vars)
		{
			_.each(vars, (val, key) => writer.variable(key, val, false))
		}, NinjaPools)
		.complete()
	const NinjaRules = driver.task('NinjaRules',
			/** @returns {{ [id:string]: { [key:string]: string } }} */
			(ninja) => Ninja.process_rules(ninja.rules), Gen)
		.stage(function(rules, regen, ninja)
		{
			if (regen)
			{
				const rule = ninja.regenerate
				const s = 'setup.js'
				const { project: p, config: c, generate: g } = options
				const r = options.root === '.' ? ' ' : ' -r ${options.root} '
				return _.set(rules, rule,
				{
					command: `node -- ${s} -p ${p} -c ${c} -g ${g}${r}-j ${options.jobs}`,
					description: rule,
					generator: 'true'
				})
			}
			else return rules
		}, HasRegen, Gen)
		.stage(function(rules)
		{
			_.each(rules, (val, id) => writer.rule(val, id))
			return rules
		}, NinjaDefVars)
		.complete()
	const NinjaBuilds = driver.task('NinjaBuilds',
			function(ninja)
			{
				return Ninja.process_variables(ninja.variables)
			}, Gen)
		.stage(
			/** @returns {{ [id:string]: object[] }[]} */
			function(variables, variants)
			{
				return _.map(variants, (v) =>
					_.reduce(variables, (acc, val, key) =>
					{
						const temp = _.filter(val, (y) => v.match(y))
						return _.size(temp) > 0 ? _.set(acc, key, temp) : acc
					}, {}))
			}, Variants)
		.partitioned(function(variables, actions)
		{
			return _.reduce(actions, (outer, rule, step) =>
				_.set(outer, step, _.reduce(variables,
					(inner, list, id) =>
					{
						const select = ({ steps, rules }) =>
							((_.size(steps) < 1) || _.some(steps, (x) => x === step)) &&
							((_.size(rules) < 1) || _.some(rules, (y) => y === rule))
						const temp = _.find(list, select)
						return _.isNil(temp) ? inner :
							_.set(inner, id, temp.value)
					}, {})), {})
		}, Actions)
		.partitioned(
			(vars, acts, outs, ins, afters) =>
			_.flatMap(acts, (action, id) =>
			{
				return Ninja.Build.create(action, _.get(outs, id, { single: [], multiple: [] }),
					_.get(outs, `${id}.implicit`, []), _.get(ins, `${id}.explicit`, []),
					_.get(ins, `${id}.implicit`, []), _.get(afters, id, []),
					_.get(vars, id, {}))
			}), Actions, BuildOutputs, BuildInputs, BuildAfter)
		.stage((builds, regen, ninja) =>
		{
			if (regen)
			{
				const rule = ninja.regenerate
				const s = 'setup.js'
				const { project: p, config: c, generate: g } = options
				const temp = Ninja.Build.create(rule, { single: [], multiple: [] },
					[{ path: 'build.ninja', origin: '' }], [], _.map([p, c, g, s], (x) =>
					{
						return { path: x, origin: '' }
					}), [], {})
				builds.unshift(temp)
			}
			return builds
		}, HasRegen, Gen)
		.partitioned((builds) => _.filter(builds, (x) => !_.isNil(x.action)))
		.union(
			/**
			 * @param {Ninja.Build} lhs
			 * @param {Ninja.Build} rhs
			 */
			(lhs, rhs) => lhs.compare(rhs))
		.stage(function(builds)
		{
			_.each(builds, (val) => { writer.build(val) })
			return builds
		}, NinjaRules)
		.complete()
	const NinjaAliases = driver.task('NinjaAliases',
			function({ aliases })
			{
				/**
				 * @const {string} separator
				 */
				const { separator, targets, modifiers } = aliases
				return _.map(targets, (val, base) =>
					new Product(val.step,
					{
						base,
						separator,
						prefix: _.pick(modifiers, val.prefix),
						suffix: _.pick(modifiers, val.suffix),
						filters: val.filters
					}))
			}, Gen)
		.partition(Variants, by_variant)
		.partitioned((aliases, variant) =>
			_.reduce(_.uniqBy(aliases, 'id'),
				(acc, item) => _.set(acc, item['partition'](variant), _.get(item, 'id')), {}
			), Variants)
		.partitioned(
			/** @returns {{ [key:string]: ProductPath[] }} */
			(parts, outs) => _.reduce(parts, (acc, val, key) =>
			{
				const x = _.get(outs, val, { single: [], multiple: [], implicit: [] })
				return _.set(acc, key, _.union(x.implicit, x.single, x.multiple))
			}, {}), BuildOutputs)
		.merge(
			/**
			 * @param {string} _key
			 * @param {...(ProductPath[] | null | undefined)} values
			 * @returns {ProductPath[]}
			 */
			(_key, ...values) => _.reduce(values, (acc, x) =>
			{
				if (_.isNil(x))
				{
					return acc
				}
				else if (_.isArray(x))
				{
					return _.unionBy(acc, x, 'path')
				}
				else
				{
					throw new TypeError('Expected parameter \'x\' to be null or an array.')
				}
			}, []))
		.stage(function(aliases)
		{
			_.each(aliases, (val, key) =>
				writer.build(Ninja.Build.alias(key, val)))
			return aliases
		}, NinjaBuilds)
		.complete()
	driver.task('NinjaDefaults', function(ninja)
		{
			writer.defs(ninja.defaults)
			return ninja.defaults
		}, Gen, NinjaAliases)
		.complete()
}

module.exports = prepare_tasks
