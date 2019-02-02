#!/usr/bin/env node

// Copyright (c) 2018-2019 James Adam Armstrong
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
const _ = require('lodash')
const fs = require('fs')
const Ajv = require('ajv')
const EventEmitter = require('events')
const path = require('path')
const Ninja = require('./js/ninja')
const { Pipeline } = require('./js/pipeline')
const { Shadow, Variant } = require('./js/variants')
const { Step, Source } = require('./js/project')
const { Directory } = require('./js/directory')
const read_json_file = require('./js/json_input')
const { displayUsage, processCommandLine } = require('./js/setup/cli')
// spellcheck: on

const ajv = new Ajv()

class Product
{
	constructor(id, { base, filters, separator, prefix, suffix })
	{
		this.id = id
		this.base = base
		this.filters = filters
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

	partition(variant)
	{
		const pre = _.reduce(this.prefix,
			(acc, val, key) =>
			{
				const x = variant.get(key)
				const y = _.get(val, x, x)
				return acc + y + this.separator
			}, '')
		const post = _.reduce(this.suffix,
			(acc, val, key) =>
			{
				const x = variant.get(key)
				const y = _.get(val, x, x)
				return acc + this.separator + y
			}, '')
		return pre + this.base + post
	}
}

/**
 * Partitions the result of a Pipeline stage.
 * @param {string} parts The Pipeline stage used to determine the partitions.
 * @param {string} source The Pipeline stage to partition the result of.
 * @param {(value: any, by: any) => any} trans
 * @param {*} filter
 */
function partition(parts, source, trans, filter = _.curry((x, y) => x.match(y)))
{
	if (_.isNil(trans))
	{
		return [parts, source, async (input) =>
			_.map(input[parts], (x) =>
				_.filter(input[source], filter(x)))]
	}
	else
	{
		return [parts, source, async (input) =>
			_.map(input[parts], (x) =>
				trans(_.filter(input[source], filter(x)), x))]
	}
}

function partitioned(pick, func)
{
	return _.concat(pick, async (input) =>
	{
		return _.map(_.zip.apply(_, _.at(input, pick)),
			_.spread(func))
	})
}

const options = processCommandLine()

if (options.help)
{
	displayUsage()
}
else
{
	const output_filename = 'build.ninja'

	const source_root = options.root

	const temp_filename = `${output_filename}.tmp`

	let writer = new Ninja.Writer(temp_filename)

	const driver = new Pipeline("driver", options.jobs,
	{
		Project: async () => read_json_file(options.project),
		Config: async () => read_json_file(options.config),
		NinjaCfg: async () => read_json_file(options.generate),
		HasRegen: ['NinjaCfg', async ({ NinjaCfg }) =>
			_.has(NinjaCfg, 'regenerate') && NinjaCfg.regenerate !== ''],
		Variants: ['Project', 'Config',
			async (results) =>
			{
				const [proj, cfg] = _.at(results, ['Project.variants', 'Config.variants'])
				return Variant.merge(Variant.process(proj),
					Variant.process(cfg))
			}
		],
		Shadows: ['Config',
			async ({ Config }) => Shadow.process(Config.variants)
		],
		Sources: ['Project',
			async ({ Project }) => Source.process(Project.sources)
		],
		ShadowSources: ['Sources', 'Shadows',
			async ({ Sources, Shadows }) =>
				_.map(Sources, (item) => Shadows.apply(item))
		],
		PartSources: partition('Variants', 'ShadowSources', null),
		MergeSources: ['PartSources', async ({ PartSources: parts }) =>
			_.map(parts, (sources) => Source.merge(sources))],
		Steps: ['Project',
			async ({ Project }) => Step.process(Project.steps)
		],
		SortedSteps: ['Steps', async ({ Steps }) => Step.sort(Steps)],
		ShadowSteps: ['SortedSteps', 'Shadows',
			async ({ SortedSteps: steps, Shadows }) =>
				_.map(steps, (item) => Shadows.apply(item))
		],
		PartSteps: partition('Variants', 'ShadowSteps', (steps) =>
			_.reduce(steps, (acc, x) =>
				_.set(acc, x.id, _.pick(x,
					['action', 'sources', 'inputs', 'outputs', 'after'])), {})),
		Actions: ['NinjaCfg', async ({ NinjaCfg }) =>
			Ninja.process_actions(NinjaCfg.rules)],
		PartActions: partition('Variants', 'Actions', (acts) =>
			_.reduce(acts,
				(acc, { action, id }) => _.set(acc, action, id), {})),
		BuildActions: partitioned(['PartActions', 'PartSteps'],
			Ninja.Build.process_actions),
		Variables: ['NinjaCfg', async ({ NinjaCfg }) =>
			Ninja.process_variables(NinjaCfg.variables)],
		PartVars: ['Variants', 'Variables',
			async ({ Variants, Variables }) =>
				_.map(Variants, (v) =>
					_.reduce(Variables, (acc, val, key) =>
					{
						const temp = _.filter(val, (y) => v.match(y))
						return _.size(temp) > 0 ? _.set(acc, key, temp) : acc
					}, {}))
		],
		BuildVars: partitioned(['PartVars', 'BuildActions'],
			(vars, acts) =>
			_.reduce(acts, (outer, rule, step) =>
				_.set(outer, step, _.reduce(vars,
					(inner, list, id) =>
					{
						const filter = ({ steps, rules }) =>
							((_.size(steps) < 1) || _.some(steps, (x) => x === step)) &&
							((_.size(rules) < 1) || _.some(rules, (y) => y === rule))
						const temp = _.find(list, filter)
						return _.isNil(temp) ? inner :
							_.set(inner, id, temp.value)
					}, {})), {})),
		StepInputs: partitioned(['PartSteps'], (steps) =>
			_.reduce(steps, (acc, { inputs }, id) =>
				(_.size(inputs) < 1) ? acc : _.set(acc, id, inputs), {})),
		StepAfter: partitioned(['PartSteps'], (part) =>
			_.reduce(part, (acc, { after }, id) =>
				_.set(acc, id, after), {})),
		BuildSources: partitioned(['MergeSources', 'PartSteps'],
			_.partial(Ninja.Build.process_sources, source_root)),
		Destinations: ['Config', async ({ Config }) =>
			Directory.process(Config.destinations)],
		PartDests: partition('Variants', 'Destinations', (dirs, variant) =>
			_.reduce(dirs,
				(acc, dir) => _.set(acc, dir.id, dir.location(variant)), {})),
		DirList: ['PartDests', 'NinjaCfg',
			async ({ PartDests, NinjaCfg }) =>
				_.reduce(_.map(PartDests, (part) => _.values(part)),
					(acc, val) => _.union(acc, val),
					[NinjaCfg.directory])
		],
		MkDirs: ['DirList', async ({ DirList }) =>
		{
			_.each(DirList, (dir) =>
				Directory.make(path.normalize(dir)))
		}],
		Extensions: ['Config', async ({ Config }) => Config.extensions],
		PartExts: ['Variants', 'Extensions',
			async ({ Variants, Extensions }) =>
				_.map(Variants, (v) =>
					_.reduce(Extensions, (acc, val, key) =>
						_.set(acc, key, _.find(val, (y) => v.match(y))
							.value), {}))
		],
		StepOutputs: partitioned(['PartSteps', 'PartExts', 'PartDests'],
			(steps, exts, dirs) =>
			_.reduce(steps, (outer, { outputs }, id) =>
				_.set(outer, id, _.reduce(outputs, (inner, val) =>
					_.set(inner, val.id,
						_.merge(
							{
								location: dirs[val.location],
								extension: exts[val.extension]
							},
							_.pick(val, ['implicit', 'single', 'products']))), {})), {})),
		StepOutputsFilter: partitioned(['StepOutputs', 'StepInputs', 'BuildSources'],
			(steps, inputs, sources) =>
			_.reduce(steps, (outer, outputs, id) =>
				_.set(outer, id, _.reduce(outputs, (inner, val, key) =>
					_.some(_.concat(_.get(inputs, id, []), _.get(sources, id, [])),
						['produces', key]) ?
					_.set(inner, key, _.omit(val, ['products'])) :
					inner, {})), {})),
		Products: ['Config', async ({ Config }) =>
			Product.process(Config.products)],
		PartProducts: partition('Variants', 'Products', (products, variant) =>
			_.reduce(_.uniqBy(products, 'id'),
				(acc, item) => _.set(acc, item['id'], item['partition'](variant)), {})),
		BuildProducts: partitioned(['PartProducts', 'StepOutputs'],
			Ninja.Build.process_products),
		ProcessBuild: partitioned(['BuildSources', 'BuildProducts', 'StepInputs',
				'StepOutputsFilter'
			],
			(sources, products, i, o) =>
			{
				let inputs = {}
				let outputs = {}
				let oports = {}
				let iport = new EventEmitter()
				_.each(_.union(_.keys(products), _.keys(o)), (id) =>
				{
					_.set(outputs, id, {})[id]
					_.set(oports, id, new EventEmitter())
				})

				function olink(step, id)
				{
					const target = outputs[id]
					const port = oports[id]
					_.each(step, (group, key) =>
					{
						if (!_.has(target, key))
						{
							_.set(target, key, _.set(_.pick(group,
								['implicit', 'single']), 'files', []))
							port.on(key, _.partial((list, file) =>
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
				_.each(products, olink)
				_.each(o, olink)
				_.each(_.union(_.keys(sources), _.keys(i)), (id) =>
				{
					let step = _.set(inputs, id, [])[id]
					iport.on(id, (index, file) =>
					{
						step[index].files.push(
						{
							path: path.format(file),
							origin: file.origin
						})
					})
					_.each(_.concat(_.get(sources, id, []), _.get(i, id, [])),
						(src, index) =>
						{
							step.push({ implicit: src.implicit, files: [] })
							if (_.has(src, 'produces'))
							{
								const group = src.produces
								const { location, extension } =
								_.merge(_.get(products[id], group, {}),
									_.get(o[id], group, {}))
								iport.on(id, _.bind(function(dir, ext, filter, { name, origin })
								{
									if (filter === index)
									{
										this.emit(group,
										{
											dir: path.dirname(path.join(dir, origin)),
											ext,
											name,
											origin
										})
									}
								}, oports[id], location, extension))
							}
							if (_.has(src, 'step') && _.has(src, 'group'))
							{
								oports[src.step].on(src.group, (file) =>
								{
									iport.emit(id, index, file)
								})
							}
						})
				})
				_.each(products, (step, id) =>
				{
					const port = oports[id]
					_.each(step, ({ files }, group) =>
					{
						_.each(files, (x) => port.emit(group, x))
					})
				})
				_.each(sources, (step, id) =>
				{
					_.each(step, ({ files }, index) =>
					{
						_.each(files, (x) => iport.emit(id, index, x))
					})
				})
				return { inputs, outputs }
			}),
		BuildOutputs: partitioned(['ProcessBuild'], ({ outputs }) =>
			_.reduce(outputs, (outer, group, id) =>
			{
				const temp = _.reduce(group, (inner, val) =>
				{
					if (!val.implicit && _.size(val.files) > 0)
					{
						return _.update(inner, val.single ? 'single' : 'multiple',
							(old) => _.union(old, val.files))
					}
					else return inner
				}, { single: [], multiple: [] })
				return (_.size(temp.single) + _.size(temp.multiple)) > 0 ?
					_.set(outer, id, temp) : outer
			}, {})),
		BuildImplicits: partitioned(['ProcessBuild'], ({ outputs }) =>
			_.reduce(outputs, (outer, group, id) =>
			{
				const temp = _.reduce(group, (inner, val) =>
				{
					if (val.implicit && _.size(val.files) > 0)
					{
						return _.concat(inner, val.files)
					}
					else return inner
				}, [])
				return _.size(temp) > 0 ? _.set(outer, id, _.uniq(temp)) : outer
			}, {})),
		BuildInputs: partitioned(['ProcessBuild'], ({ inputs }) =>
			_.reduce(inputs, (outer, group, id) =>
			{
				const temp = _.reduce(group, (inner, val) =>
				{
					if (!val.implicit && _.size(val.files) > 0)
					{
						return _.concat(inner, val.files)
					}
					else return inner
				}, [])
				return _.size(temp) > 0 ? _.set(outer, id, _.uniq(temp)) : outer
			}, {})),
		BuildDepends: partitioned(['ProcessBuild'], ({ inputs }) =>
			_.reduce(inputs, (outer, group, id) =>
			{
				const temp = _.reduce(group, (inner, val) =>
				{
					if (val.implicit && _.size(val.files) > 0)
					{
						return _.concat(inner, val.files)
					}
					else return inner
				}, [])
				return _.size(temp) > 0 ? _.set(outer, id, _.uniq(temp)) : outer
			}, {})),
		BuildAfter: partitioned(['StepAfter', 'BuildOutputs', 'BuildImplicits'],
			(part, outs, imps) =>
			_.reduce(part, (acc, list, step) =>
				_.set(acc, step, _.uniq(_.flatMap(list, (id) =>
					_.concat(_.get(imps, id, []),
						_.get(outs, `${id}.single`, []),
						_.get(outs, `${id}.multiple`, []))))), {})),
		AppendRegen: ['BuildAfter', 'HasRegen',
			async ({ BuildAfter, HasRegen }) =>
			{
				if (HasRegen)
				{
					_.each(BuildAfter, (part) =>
					{
						_.each(part, (step) =>
							step.push({ path: 'build.ninja' }))
					})
				}
				return BuildAfter
			}
		],
		NinjaVersion: ['NinjaCfg', async ({ NinjaCfg }) =>
		{
			writer.variable('ninja_required_version',
				_.get(NinjaCfg, 'version', null), false)
		}],
		NinjaDirectory: ['NinjaCfg', 'NinjaVersion', async ({ NinjaCfg }) =>
		{
			writer.variable('builddir',
				_.get(NinjaCfg, 'directory', null), false)
		}],
		NinjaPools: ['NinjaCfg', async ({ NinjaCfg }) =>
			Ninja.process_pools(NinjaCfg.pools)],
		NinjaPoolsOut: ['NinjaPools', 'NinjaDirectory',
			async ({ NinjaPools: pools }) =>
			{
				_.each(pools, (val, key) => writer.pool(val, key))
			}
		],
		NinjaDefVars: ['NinjaCfg', async ({ NinjaCfg }) =>
			Ninja.process_def_variables(NinjaCfg.variables)],
		NinjaDefVarsOut: ['NinjaDefVars', 'NinjaPoolsOut',
			async ({ NinjaDefVars: vars }) =>
			{
				_.each(vars, (val, key) =>
					writer.variable(key, val, false))
			}
		],
		NinjaRules: ['NinjaCfg', async ({ NinjaCfg }) =>
			Ninja.process_rules(NinjaCfg.rules)],
		NinjaRulesOut: ['NinjaRules', 'NinjaDefVarsOut',
			async ({ NinjaRules: rules }) =>
			{
				_.each(rules, (val, key) =>
					writer.rule(val, key))
			}
		],
		NinjaRegen: ['NinjaCfg', 'NinjaRulesOut', 'HasRegen', async ({ NinjaCfg, HasRegen }) =>
		{
			if (HasRegen)
			{
				const rule = NinjaCfg.regenerate
				const s = 'setup.js'
				const p = options.project
				const c = options.config
				const g = options.generate
				const r = options.root
				writer.rule(
				{
					command: `node -- ${s} -p ${p} -c ${c} -g ${g} -r ${r} -j ${options.jobs}`,
					description: 'regen',
					generator: 'true'
				}, rule)
				const builds = Ninja.Build.create(rule, { single: [], multiple: [] },
					[{ path: 'build.ninja' }], [], _.map([p, c, g, s], (x) =>
					{
						return { path: x }
					}), [], {})
				_.each(builds, (x) => writer.build(x))
			}
		}],
		NinjaBuilds: partitioned(['BuildActions', 'BuildOutputs', 'BuildImplicits',
				'BuildInputs', 'BuildDepends', 'AppendRegen', 'BuildVars'
			],
			(acts, outs, imps, ins, deps, afters, vars) =>
			_.reduce(acts, (acc, action, id) =>
			{
				return _.concat(acc, Ninja.Build.create(action, _.get(outs, id, []),
					_.get(imps, id, []), _.get(ins, id, []),
					_.get(deps, id, []), _.get(afters, id, []),
					_.get(vars, id, {})))
			}, [])),
		NinjaBuildsOut: ['NinjaRegen', 'NinjaBuilds',
			async ({ NinjaBuilds: parts }) =>
			{
				_.each(parts, (builds) =>
				{
					_.each(builds, (val) => writer.build(val))
				})
			}
		],
		NinjaAliases: ['NinjaCfg', async ({ NinjaCfg }) =>
		{
			const { separator, targets, prefix, suffix } = NinjaCfg.aliases
			return _.map(targets, (val, base) =>
				new Product(val.step,
				{
					base,
					separator,
					prefix,
					suffix,
					filters: val.filters
				}))
		}],
		NinjaAliasesPart: partition('Variants', 'NinjaAliases', (aliases, variant) =>
			_.reduce(_.uniqBy(aliases, 'id'),
				(acc, item) => _.set(acc, item['partition'](variant), _.get(item, 'id')), {})),
		NinjaAliasesAssign: partitioned(['NinjaAliasesPart', 'BuildOutputs', 'BuildImplicits'],
			(part, outs, deps) => _.reduce(part, (acc, val, key) =>
			{
				const x = _.get(outs, val, {})
				const y = _.get(deps, val, [])
				return _.set(acc, key, _.union(y, x.single, x.multiple))
			}, {})),
		NinjaAliasesOut: ['NinjaBuildsOut', 'NinjaAliasesAssign',
			async ({ NinjaAliasesAssign: aliases }) =>
			{
				_.each(aliases, (part) =>
				{
					_.each(part, (val, key) =>
						writer.build(Ninja.Build.alias(key, val)))
				})
			}
		],
		NinjaDefaultOut: ['NinjaAliasesOut', 'NinjaCfg',
			async ({ NinjaCfg }) => { writer.defs(NinjaCfg.defaults) }
		]
	})
	driver.applyFilter('Sources', ajv.compile(
	{
		type: 'array',
		items:
		{
			type: 'object',
			'additionalProperties':
			{
				type: 'array',
				items: { type: 'string' }
			}
		}
	}))
	driver.onFailure((stage, error) =>
	{
		if (!_.isNil(stage))
		{
			stage = String(stage)
			console.warn(`Failure encountered in stage '${stage}' of the Pipeline.`)
		}
		console.error(error)
		process.exitCode = 1
	})
	driver.onComplete(() => { console.log('Completed.') })
	process.exitCode = 0
	driver.run()
		.finally(() =>
		{
			writer.close()
			if (process.exitCode === 0)
			{
				if (fs.existsSync(output_filename))
				{
					fs.renameSync(output_filename, `${output_filename}.bak`)
				}
				fs.renameSync(temp_filename, output_filename)
			}
			else
			{
				fs.unlinkSync(temp_filename)
			}
		})

}
