#!/usr/bin/env node

// Copyright (c) 2018 James Adam Armstrong

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.



const commandLineArgs = require('command-line-args')
const commandLineUsage = require('command-line-usage')
const jsonfile = require('jsonfile')
const _ = require('lodash')
const fs = _.merge(require('fs'), require('fs').promises)
const {machine} = require('asyncmachine')
const {struct} = require('superstruct')
const EventEmitter = require('events')
const _a = require('neo-async')
const path = require('path')



const VariantSchema = struct.dict(['string',
	struct.partial({
		options:
			struct.intersection([struct.list(['string']),
				(x) => 0 < _.size(x)])
	})])

const FilterSchema =
	struct.list([struct.dict(['string', 'string'])])

const ModSchema = struct.dict(['string',
	struct.dict(['string', 'string'])])

const ProjectSchema = struct.partial({
	variants: VariantSchema,
	sources: struct.list([struct.partial({
		group: 'string',
		prefix: 'string',
		suffix: 'string',
		files:
			struct.intersection([struct.list(['string']),
				(x) => 0 < _.size(x)]),
		filters: FilterSchema
	}, {
		prefix: '',
		suffix: '',
		filters: []
	})]),
	steps: struct.dict(['string',
		struct.partial({
			action: 'string',
			filters: FilterSchema,
			sources: struct.list([struct.partial({
				implicit: 'boolean',
				group: 'string',
				produces: 'string?'
			}, {
				implicit: false
			})]),
			inputs: struct.list([struct.partial({
				implicit: 'boolean',
				step: 'string',
				group: 'string',
				produces: 'string?'
			}, {
				implicit: false
			})]),
			outputs: struct.dict(['string',
				struct.intersection([struct.partial({
					implicit: 'boolean',
					location: 'string',
					extension: 'string',
					products: struct.list(['string']),
					single: 'boolean'
				}, {
					implicit: false,
					products: [],
					single: false
				}), ({implicit, single}) => !implicit || !single])
			]),
			after: struct.list(['string'])
		}, {
			filters: [],
			sources: [],
			inputs: [],
			after: []
		})])
})

const NinjaSchema = struct.partial({
	id: struct.literal('ninja'),
	version: 'number?',
	directory: 'string',
	pools: struct.list([struct.partial({
		id: 'string',
		depth: struct.intersection(['number', Number.isInteger])
	})]),
	variables: struct.dict(['string',
		struct.partial({
			default: 'string',
			specific: struct.list([struct.partial({
				filters: FilterSchema,
				rules: struct.list(['string']),
				steps: struct.list(['string']),
				value: 'string'
			}, {
				filters: [],
				rules: [],
				steps: []
			})])
		}, {
			default: '',
			specific: []
		})]),
	rules: struct.dict(['string',
		struct.partial({
			action: 'string',
			filters: FilterSchema,
			vars: struct({
				command: 'string',
				pool: 'string?',
				depfile: 'string?',
				deps: struct.optional(struct.union([
					struct.literal('gcc'),
					struct.literal('msvc')
				])),
				msvc_deps_prefix: 'string?',
				description: 'string?',
				generator: 'boolean',
				restat: 'boolean',
				rspfile: 'string?',
				rspfile_content: 'string?'
			}, {
				generator: false,
				restat: false
			})
		}, {
			filters: []
		})]),
	aliases: struct.partial({
		separator: 'string',
		targets: struct.dict(['string', struct.partial({
			step: 'string',
			filters: FilterSchema
		}, {
			filters: []
		})]),
		prefix: ModSchema,
		suffix: ModSchema
	}, {
		separator: '',
		prefix: {},
		suffix: {}
	}),
	defaults: struct.list(['string'])
}, {
	directory: '',
	defaults: []
})

const ConfigSchema = struct.partial({
	variants: VariantSchema,
	generator: NinjaSchema,
	destinations: struct.dict(['string', struct.partial({
		root: 'string',
		filters: FilterSchema,
		folders: ModSchema
	}, {
		root: '',
		filters: [],
		folders: {}
	})]),
	extensions: struct.dict(['string', struct.list([
		struct.partial({
			filters: FilterSchema,
			value: 'string'
		}, {
			value: '',
			filters: []
		})])
	]),
	products: struct.dict(['string', struct.list([
		struct.partial({
			filters: FilterSchema,
			base: 'string',
			separator: 'string',
			prefix: ModSchema,
			suffix: ModSchema
		}, {
			separator: '',
			filters: [],
			prefix: {},
			suffix: {}
		})])
	])
}, {
	destinations: {},
	extensions: {},
	products: {}
})



async function pipeline(concurrency = 1, tasks) {
	let results = {}
	let error = null
	const automata = machine(_.reduce(_.keys(tasks),
		(acc, x) => _.set(acc, x, {}), {})).id('automata')
	const guard = machine({Abort: {}}).id('guard')
	_.each([automata, guard], (x) => x.set([]))
	guard.Abort_exit = () => false
	guard.Abort_state = _.once((err) => error = err)
	const condition = (states) =>
		Promise.race([guard.when('Abort'),
			automata.when(states)])
	const queue = _a.queue(({id, pass, task}, cb) => {
		const future = guard.is('Abort') ? Promise.resolve(null) :
			(pass ? task(results) : task())
		future.then((val) =>
			_.set(results, id, val),
			(err) => {throw err}).catch((err) =>
				guard.add('Abort', err)).finally(() => {
					automata.add(id)
					cb()
				})
	}, _.max(1, concurrency))
	// Queue independent tasks and set others to be queued once dependencies complete.
	_.each(tasks, (val, key) => {
		if (_.isArray(val)) {
			const len = _.size(val)
			if (len == 1) {
				queue.push({id: key, pass: false, task: _.head(val)})
			} else if (len > 1) {
				condition(_.initial(val)).then(() =>
					queue.push({id: key, pass: true, task: _.last(val)}))
			} else {
				guard.add('Abort',
					Error('A task must have a function at the end of the array.'))
				_.set(results, key, null)
				automata.add(key)
			}
		} else {
			queue.push({id: key, pass: false, task: val})
		}
	})
	// Wait for all tasks to complete.
	await condition(_.keys(tasks))
	if (_.isNil(error)) {
		return results
	} else {
		throw error
	}
}



class Shadow {
	constructor() {
		this.shadows = {}
	}

	static process(raw) {
		return new Shadow()
	}

	apply(item) {
		return _.set(item, 'filters', _.reduce(this.shadows, (acc, val, key) => {
			const [y, z] = _.partition(acc, (a) => _.has(a, key))
			const b = _.flatMap(y, (c) =>
				_.map(val, (d) => _.set(_.merge({}, c), key, d)))
			return _.concat(b, z)
		}, item.filters))
	}
}

class Variant {
	constructor(data) {
		this.data = data
	}

	static process(raw) {
		return _.reduce(raw, (acc, val, key) =>
			_.set(acc, key, _.get(val, 'options', [])), {})
	}

	static merge(base, extra) {
		var temp = _.merge({}, base, extra)
		temp = _.reduce(temp, (acc, val, key) =>
			_.flatMap(val, (x) => _.map(acc, (y) =>
				_.merge(_.set({}, key, x), y))), [{}])
		return _.map(temp, (x) => new Variant(x))
	}

	match(item) {
		const filters = _.get(item, 'filters', [])
		return _.size(filters) == 0 || _.some(filters, x =>
			_.every(this.data, (y, key) => !_.has(x, key) || _.get(x, key) == y))
	}

	get(tag) {
		return this.data[tag]
	}
}

class Step {
	constructor(id, data) {
		this.action = data.action
		this.id = id
		this.filters = _.get(data, 'filters', [])
		this.inputs = _.get(data, 'inputs', [])
		this.outputs = _.map(_.get(data, 'outputs', {}),
			(val, key) => _.set(val, 'id', key))
		this.after = _.get(data, 'after', [])
		this.sources = _.get(data, 'sources', [])
	}

	static process(raw) {
		return _.map(raw, (step, key) => new Step(key, step))
	}

	get_deps() {
		return _.union(this.after,
			_.map(this.inputs, (y) => y['step']))
	}

	static sort(steps) {
		const tree = _.reduce(steps, (acc, item) =>
			_.set(acc, item.id, item.get_deps()), {})
		var [sorted, remaining] = _.partition(steps, (x) => tree[x.id] == [])
		sorted = [sorted]
		while(0 < _.size(remaining)) {
			var removed
			[remaining, removed] = _.partition(remaining, (x) => {
				const deps = tree[x.id]
				return _.some(remaining, (y) => _.includes(deps, y.id))
			})
			if (_.size(removed) == 0) {
				throw new Error("Circular dependency detected in steps.")
			} else {
				sorted.push(removed)
			}
		}
		return _.flatten(sorted)
	}

	/*static map_products(lookup, steps) {
		var outputs = {}
		_.each(steps, (item) => {
			let {variant, id, outs} = item
			var slot = {}
			_.each(outs, (val, key) => {
				let products = _.get(val, 'products', [])
				if (0 < _.size(products)) {
					let temp = _.map(products, (x) =>
						lookup(variant, id, key, x))
					_.set(val, 'files', temp)
					_.set(slot, key, {variant, files: temp,
						location: _.get(val, 'location'),
						extension: _.get(val, 'extension')})
				}
			})
			_.set(outputs, id, slot)
		})
		return outputs
	}

	static map_deps(inputs, outputs) {
		return _.map(inputs, (input) => {
			let found = _.find(out[input['step']][input['group']],
				(x) => x[0] = variant)
			if (_.isNil(found)) {
				return _.set(input, 'files', [])
			} else {
				_.set(input, 'files', found[1])
				_.set(input, 'location', found[2])
				return _.set(input, 'extension', found[3])
			}
		})
	}

	map_inputs(outputs) {
		return map_deps(this.inputs, outputs)
	}

	map_after(outputs) {
		return map_deps(this.after, outputs)
	}

	map_outputs(step, outputs, inputs) {
		return [step, outputs]
	}

	static map_files(steps, lookup) {
		var outputs = map_products(lookup)
		return _.map(await steps, (item) => {
			const variant = item[0]
			let data = item[2]
			let itemp = this.map_inputs(outputs)
			let atemp = this.map_after(outputs)
			return item
		})
	}*/
}

class Source {
	constructor(src) {
		const prefix = _.get(src, 'prefix', "")
		const suffix = _.get(src, 'suffix', "")
		const files = _.get(src, 'files', [])
		this.filters = _.get(src, 'filters', [])
		this.group = _.get(src, 'group', "")
		this.files = _.map(files, (f) =>
			_.replace(prefix + f + suffix, '/', path.sep))
	}

	static process(raw) {
		return _.map(raw, (src) => new Source(src))
	}

	static merge(sources) {
		const groups = _.uniq(_.map(sources, (x) => x.group))
		const result = _.reduce(groups, (acc, x) => {
			const members = _.filter(sources, (y) => y.group == x)
			const files = _.union.apply(_, _.map(members, 'files'))
			return (_.size(files) < 1) ? acc : _.set(acc, x, files)
		}, {})
		return Source.MergedSchema(result)
	}
}

Source.MergedSchema = struct.dict(['string', struct.list(['string'])])

class Ninja {
	static process_pools(raw) {
		return _.reduce(raw, (acc, cur) => (_.has(cur, 'id') ?
			_.set(acc, cur['id'], _.get(cur, 'depth', 1)) : acc), {})
	}

	static process_def_variables(raw) {
		return _.reduce(raw, (acc, cur, id) =>
			_.set(acc, id, _.get(cur, 'default', '')), {})
	}

	static process_variables(raw) {
		return _.reduce(raw, (acc, cur, id) =>
			_.set(acc, id, _.get(cur, 'specific', [])), {})
	}

	static process_rules(raw) {
		return _.reduce(raw, (acc, val, key) =>
			_.set(acc, key, _.get(val, 'vars', '')), {})
	}

	static process_actions(raw) {
		return _.map(raw, (val, key) => {
			return {filters: _.get(val, 'filters', []),
				action: val.action, id: key}
		})
	}
}

class NinjaWriter {
	constructor(filename) {
		this.file = fs.open(filename, 'w')
		this.order = this.file.then((f) => this.file = f)
	}

	close() {
		this.order = this.order.finally(async () => {
			if (!_.isNil(this.file)) {
				let temp = this.file
				this.file = null
				await temp.sync()
				return temp.close()
			}
		})
	}

	write(data) {
		this.order = this.order.then(() => this.file.writeFile(data))
		return this
	}

	variable(name, value, indent) {
		if (!(_.isNil(value) || value == '')) {
			this.write((indent ? '  ' : '') + name + ' = ' + value + "\n")
		}
		return this
	}

	pool(val, key) {
		this.write("pool " + key + "\n")
		return this.variable('depth', val, true)
	}

	rule(val, key) {
		this.write("rule " + key + "\n")
		return _.reduce(val, (acc, x, y) =>
			acc.variable(y, x, true), this)
	}

	build({action, outputs, implicit, inputs, depends, after, vars}) {
		const list = (pre, items) => {
			if (0 < _.size(items)) {
				this.write(pre)
				_.reduce(items, (acc, val) =>
					acc.write(" " + val), this)
			}
		}
		_.reduce(outputs, (acc, val) =>
			acc.write(" " + val), this.write("build"))
		list(" |", implicit)
		_.reduce(inputs, (acc, val) =>
			acc.write(" " + val), this.write(": " + action))
		list(" |", depends)
		list(" ||", after)
		this.write("\n")
		return _.reduce(vars, (acc, val, key) =>
			acc.variable(key, val, true), this)
	}

	defs(targets) {
		if (0 < _.size(targets)) {
			this.write("default")
			_.each(targets, (x) => this.write(" " + x))
			this.write("\n")
		}
		return this
	}
}

class Directory {
	constructor(id, {root, filters, folders}) {
		this.id = id
		this.root = root
		this.filters = filters
		this.folders = folders
	}

	static process(raw) {
		return _.map(raw, (val, key) => new Directory(key, val))
	}

	location(variant) {
		const temp = _.map(this.folders, (val, key) =>
			_.get(val, variant.get(key), variant.get(key)))
		return _.spread(path.join)(_.concat(this.root, temp))
	}

	static make(dir) {
		if (!fs.existsSync(dir)) {
			const parent = path.dirname(dir)
			if (!fs.existsSync(parent)) {
				Directory.make(parent)
			}
			fs.mkdirSync(dir)
		}
	}
}

class Product {
	constructor(id, {base, filters, separator, prefix, suffix}) {
		this.id = id
		this.base = base
		this.filters = filters
		this.separator = separator
		this.prefix = prefix
		this.suffix = suffix
	}

	static process(raw) {
		return _.flatMap(raw, (list, key) =>
			_.map(list, (item) =>
				new Product(key, item)))
	}

	partition(variant) {
		const pre = _.reduce(this.prefix,
			(acc, val, key) => {
				const x = variant.get(key)
				const y = _.get(val, x, x)
				return acc + y + this.separator
			}, '')
		const post = _.reduce(this.suffix,
			(acc, val, key) => {
				const x = variant.get(key)
				const y = _.get(val, x, x)
				return acc + this.separator + y
			}, '')
		return pre + this.base + post
	}
}

class Build {
	constructor(action, outputs, implicit, inputs, depends, after, vars) {
		this.action = action
		this.outputs = _.map(outputs, 'path')
		this.implicit = _.map(implicit, 'path')
		this.inputs = _.map(inputs, 'path')
		this.depends = _.map(depends, 'path')
		this.after = _.map(after, 'path')
		this.vars = vars
	}

	static process_actions(acts, steps) {
		return _.reduce(steps, (acc, item, id) =>
			_.set(acc, id, acts[item.action]),
			{})
	}

	static process_sources(sources, steps) {
		return _.reduce(steps, (acc, item, id) =>
			_.set(acc, id, _.map(item.sources, (src) => {
					const files = _.map(_.map(_.get(sources, src.group, []),
						(file) => path.parse(file)), (file) =>
							_.set(file, 'origin', path.join(file.dir, file.name)))
					return _.merge({files: files},
						_.pick(src, ['implicit', 'produces']))
				})),
			{})
	}

	static process_products(names, steps) {
		return _.reduce(steps, (acc, outputs, key) => {
			const temp = _.reduce(outputs,
				(acc, {implicit, products, location, extension}, id) =>
					(_.isNil(products) || _.size(products) < 1) ? acc :
						_.set(acc, id, {implicit, files: _.map(products, (name) => {
							const origin = _.get(names, name, name)
							return {
								dir: location,
								name: origin,
								ext: extension,
								origin
							}
						})}), {})
			return _.size(temp) < 1 ? acc : _.set(acc, key, temp)
		}, {})
	}

	static alias(name, inputs) {
		return new Build('phony', [{path: name}],
			[], inputs, [], [], {})
	}

	static create(action, {single, multiple}, implicit, inputs, depends, after, vars = {}) {
		if (_.size(single) < 1) {
			return new Build(action, multiple, implicit, inputs, depends, after, vars)
		} else {
			let [isingle, imultiple] = _.partition(inputs, (x) =>
				_.some(single, (y) => x.origin == y.origin))
			return _.map(single, ({path, origin}) => {
				let o = _.reject(multiple, (x) => x.origin == origin)
				let m = _.reject(implicit, (x) => x.origin == origin)
				let i = _.find(isingle, (x) => x.origin == origin)
				let d = _.reject(depends, (x) => x.origin == origin)
				let a = _.reject(after, (x) => x.origin == origin)
				return new Build(action, _.concat(o, {path}), m,
					_.concat(imultiple, i), d, a, vars)
			})
		}
	}
}

function partition(parts, source, trans, filter = _.curry((x, y) => x.match(y))) {
	if (_.isNil(trans)) {
		return [parts, source, async (input) =>
			_.map(input[parts], (x) =>
				_.filter(input[source], filter(x)))]
	} else {
		return [parts, source, async (input) =>
			_.map(input[parts], (x) =>
				trans(_.filter(input[source], filter(x)), x))]
	}
}

function partitioned(pick, func) {
	return _.concat(pick, async (input) => {
		return _.map(_.zip.apply(_, _.at(input, pick)),
			_.spread(func))
	})
}

function display(x, depth = 1) {
	if (0 < depth) {
		if (_.isArray(x)) {
			if (_.size(x) < 1) {
				console.log([])
			} else {
				_.each(x, (y, z) => {
					console.log("[", z, "]:")
					console.group()
					display(y, depth - 1)
					console.groupEnd()
				})
			}
		} else if (_.isPlainObject(x)) {
			if (_.size(x) < 1) {
				console.log({})
			} else {
				_.each(x, (y, z) => {
					console.log(z, ":")
					console.group()
					display(y, depth - 1)
					console.groupEnd()
				})
			}
		} else {
			console.log(x)
		}
	} else {
		console.log(x)
	}
}



const cmd_args = [
	{
		name: 'help',
		alias: 'h',
		type: Boolean,
		defaultValue: false,
		description: 'Displays this help message.'
	},
	{
		name: 'project',
		alias: 'p',
		type: String,
		typeLabel: '{underline file.json}',
		defaultValue: 'project.json',
		description: 'The file containing a build environment agnostic project description. [project.json]'
	},
	{
		name: 'config',
		alias: 'c',
		type: String,
		typeLabel: '{underline file.json}',
		defaultValue: 'config.json',
		description: 'The file containing build environment specific configurations. [config.json]'
	},
	{
		name: 'output',
		alias: 'o',
		type: String,
		typeLabel: '{underline build.ninja}',
		defaultValue: './build.ninja',
		description: 'The output Ninja file. The directory is to be the current directory while building. [./build.ninja]'
	},
	{
		name: 'jobs',
		alias: 'j',
		type: Number,
		defaultValue: 4,
		description: 'The number of tasks to run in parallel. [4]'
	},
	{
		name: 'verbose',
		alias: 'v',
		type: Number,
		defaultValue: 0
	}
]

const options = commandLineArgs(cmd_args)

if (options.help) {
	const usage = [
		{
			header: 'setup.js',
			content: 'A Node.js script to generate Ninja build files from JSON descriptions of the project.',
		},
		{
			header: 'Synopsis',
			content: [
				'$ node -- setup.js --help',
				'$ node -- setup.js -p path/project.json -c path/config.json',
				'$ node -- setup.js -o path/build.ninja',
				'$ node -- setup.js -j 20'
			]
		},
		{
			header: 'Options',
			optionList: cmd_args,
			hide: ['verbose']
		}
	]
	console.log(commandLineUsage(usage))
} else {

const build_root = path.resolve(path.dirname(options.output))

Directory.make(build_root)

let writer = new NinjaWriter(options.output)

const driver = pipeline(options.jobs, {
	ReadProject: async () => jsonfile.readFile(options.project),
	ReadConfig: async () => jsonfile.readFile(options.config),
	Project: ['ReadProject',
		async ({ReadProject: proj}) => ProjectSchema(proj)],
	Config: ['ReadConfig',
		async ({ReadConfig: cfg}) => ConfigSchema(cfg)],
	NinjaCfg: ['Config', async ({Config}) => Config.generator],
	Variants: ['Project', 'Config',
		async (results) => {
			const [proj, cfg] = _.at(results, ['Project.variants', 'Config.variants'])
			return Variant.merge(Variant.process(proj),
				Variant.process(cfg))
		}],
	Shadows: ['Config',
		async ({Config}) => Shadow.process(Config.variants)],
	Sources: ['Project',
		async ({Project}) => Source.process(Project.sources)],
	ShadowSources: ['Sources', 'Shadows',
		async ({Sources, Shadows}) =>
			_.map(Sources, (item) => Shadows.apply(item))],
	PartSources:
		partition('Variants','ShadowSources', null),
	MergeSources: ['PartSources', async ({PartSources: parts}) =>
		_.map(parts, (sources) => Source.merge(sources))],
	Steps: ['Project',
		async ({Project}) => Step.process(Project.steps)],
	SortedSteps: ['Steps', async ({Steps}) => Step.sort(Steps)],
	ShadowSteps: ['SortedSteps', 'Shadows',
		async ({SortedSteps: steps, Shadows}) =>
			_.map(steps, (item) => Shadows.apply(item))],
	PartSteps: partition('Variants', 'ShadowSteps', (steps) =>
		_.reduce(steps, (acc, x) =>
			_.set(acc, x.id, _.pick(x,
				['action', 'sources', 'inputs', 'outputs', 'after'])),
			{})),
	Actions: ['NinjaCfg', async ({NinjaCfg}) =>
		Ninja.process_actions(NinjaCfg.rules)],
	PartActions: partition('Variants', 'Actions', (acts) =>
		_.reduce(acts,
			(acc, {action, id}) => _.set(acc, action, id),
			{})),
	BuildActions:
		partitioned(['PartActions', 'PartSteps'],
			Build.process_actions),
	Variables: ['NinjaCfg', async ({NinjaCfg}) =>
		Ninja.process_variables(NinjaCfg.variables)],
	PartVars: ['Variants', 'Variables',
		async ({Variants, Variables}) =>
			_.map(Variants, (v) =>
				_.reduce(Variables, (acc, val, key) => {
					const temp = _.filter(val, (y) => v.match(y))
					return 0 < _.size(temp)? _.set(acc, key, temp) : acc
				}, {}))],
	BuildVars: partitioned(['PartVars', 'BuildActions'],
		(vars, acts) =>
			_.reduce(acts, (outer, rule, step) =>
				_.set(outer, step, _.reduce(vars,
					(inner, list, id) => {
						const filter = ({steps, rules}) =>
							((_.size(steps) < 1) || _.some(steps, (x) => x == step)) &&
								((_.size(rules) < 1) || _.some(rules, (y) => y == rule))
						const temp = _.find(list, filter)
						display({step, rule, id, temp, list}, 2)
						return _.isNil(temp)? inner :
							_.set(inner, id, temp.value)
					}, {})),
				{})),
	StepInputs: partitioned(['PartSteps'], (steps) =>
		_.reduce(steps, (acc, {inputs}, id) =>
			(_.size(inputs) < 1) ? acc : _.set(acc, id, inputs), {})),
	StepAfter: partitioned(['PartSteps'], (part) =>
		_.reduce(part, (acc, {after}, id) =>
			_.set(acc, id, after), {})),
	BuildSources:
		partitioned(['MergeSources', 'PartSteps'],
			Build.process_sources),
	Destinations: ['Config', async ({Config}) =>
		Directory.process(Config.destinations)],
	PartDests: partition('Variants', 'Destinations', (dirs, variant) =>
		_.reduce(dirs,
			(acc, dir) => _.set(acc, dir.id, dir.location(variant)),
			{})),
	DirList: ['PartDests', 'NinjaCfg',
		async ({PartDests, NinjaCfg}) =>
			_.reduce(_.map(PartDests, (part) => _.values(part)),
				(acc, val) => _.union(acc, val),
				[NinjaCfg.directory])],
	MkDirs: ['DirList', async ({DirList}) => {
		_.each(DirList, (dir) =>
			Directory.make(path.normalize(path.join(build_root, dir))))
	}],
	Extensions: ['Config', async ({Config}) => Config.extensions],
	PartExts: ['Variants', 'Extensions',
		async ({Variants, Extensions}) =>
			_.map(Variants, (v) =>
				_.reduce(Extensions, (acc, val, key) =>
					_.set(acc, key, _.head(_.filter(val, (y) => v.match(y))).value),
					{}))],
	StepOutputs: partitioned(['PartSteps', 'PartExts', 'PartDests'],
		(steps, exts, dirs) =>
			_.reduce(steps, (acc, {outputs}, id) =>
				_.set(acc, id, _.reduce(outputs, (acc, val) =>
					_.set(acc, val.id,
						_.merge({location: dirs[val.location], extension: exts[val.extension]},
							_.pick(val, ['implicit', 'single', 'products']))),
						{})),
				{})),
	StepOutputsFilter: partitioned(['StepOutputs', 'StepInputs', 'BuildSources'],
		(steps, inputs, sources) =>
			_.reduce(steps, (acc, outputs, id) =>
				_.set(acc, id, _.reduce(outputs, (acc, val, key) =>
					_.some(_.concat(_.get(inputs, id, []), _.get(sources, id, [])),
						(i) => i.produces == key)?
						_.set(acc, key, _.omit(val, ['products'])) :
						acc, {})),
				{})),
	Products: ['Config', async ({Config}) =>
		Product.process(Config.products)],
	PartProducts: partition('Variants', 'Products', (products, variant) =>
		_.reduce(_.uniqBy(products, 'id'),
			(acc, item) => _.set(acc, item.id, item.partition(variant)),
			{})),
	BuildProducts:
		partitioned(['PartProducts', 'StepOutputs'],
			Build.process_products),
	ProcessBuild:
		partitioned(['BuildSources', 'BuildProducts', 'StepInputs', 'StepOutputsFilter'],
			(sources, products, i, o) => {
				let inputs = {}
				let outputs = {}
				let oports = {}
				let iport = new EventEmitter()
				_.each(_.union(_.keys(products), _.keys(o)), (id) => {
					_.set(outputs, id, {})[id]
					_.set(oports, id, new EventEmitter())
				})
				function olink(step, id) {
					const target = outputs[id]
					const port = oports[id]
					_.each(step, (group, key) => {
						if (!_.has(target, key)) {
							_.set(target, key, _.set(_.pick(group,
								['implicit', 'single']), 'files', []))
							port.on(key, _.partial((list, file) => {
								list.push({
									path: path.format(file),
									origin: file.origin
								})
							}, target[key].files))
						}
					})
				}
				_.each(products, olink)
				_.each(o, olink)
				_.each(_.union(_.keys(sources), _.keys(i)), (id) => {
					let step = _.set(inputs, id, [])[id]
					iport.on(id, (index, file) => {
						step[index].files.push({
							path: path.format(file),
							origin: file.origin
						})
					})
					_.each(_.concat(_.get(sources, id, []), _.get(i, id, [])),
						(src, index) => {
							step.push({implicit: src.implicit, files: []})
							if (_.has(src, 'produces')) {
								const group = src.produces
								const {location, extension} =
									_.merge(_.get(products[id], group, {}),
										_.get(o[id], group, {}))
								iport.on(id, _.bind(function (group, dir, ext, filter, index, {name, origin}) {
									if (filter == index) {
										this.emit(group, {
											dir: path.dirname(path.join(dir, origin)),
											ext, name, origin
										})
									}
								}, oports[id], group, location, extension, index))
							}
							if (_.has(src, 'step') && _.has(src, 'group')) {
								oports[src.step].on(src.group, _.partial((id, index, file) => {
									iport.emit(id, index, file)
								}, id, index))
							}
						})
				})
				_.each(products, (step, id) => {
					const port = oports[id]
					_.each(step, ({files}, group) => {
						_.each(files, (x) => port.emit(group, x))
					})
				})
				_.each(sources, (step, id) => {
					_.each(step, ({files}, index) => {
						_.each(files, (x) => iport.emit(id, index, x))
					})
				})
				return {inputs, outputs}
			}),
	BuildOutputs: partitioned(['ProcessBuild'], ({outputs}) =>
		_.reduce(outputs, (outer, group, id) => {
			const temp = _.reduce(group, (inner, val) => {
				if (!val.implicit && 0 < _.size(val.files)) {
					return _.update(inner, val.single ? 'single' : 'multiple',
						(old) => _.union(old, val.files))
				} else return inner
			}, {single: [], multiple: []})
			return 0 < (_.size(temp.single) + _.size(temp.multiple)) ?
				_.set(outer, id, temp) : outer
		}, {})),
	BuildImplicits: partitioned(['ProcessBuild'], ({outputs}) =>
		_.reduce(outputs, (outer, group, id) => {
			const temp = _.reduce(group, (inner, val) => {
				if (val.implicit && 0 < _.size(val.files)) {
					return _.concat(inner, val.files)
				} else return inner
			}, [])
			return 0 < _.size(temp) ? _.set(outer, id, _.uniq(temp)) : outer
		}, {})),
	BuildInputs: partitioned(['ProcessBuild'], ({inputs}) =>
		_.reduce(inputs, (outer, group, id) => {
			const temp = _.reduce(group, (inner, val) => {
				if (!val.implicit && 0 < _.size(val.files)) {
					return _.concat(inner, val.files)
				} else return inner
			}, [])
			return 0 < _.size(temp) ? _.set(outer, id, _.uniq(temp)) : outer
		}, {})),
	BuildDepends: partitioned(['ProcessBuild'], ({inputs}) =>
		_.reduce(inputs, (outer, group, id) => {
			const temp = _.reduce(group, (inner, val, key) => {
				if (val.implicit && 0 < _.size(val.files)) {
					return _.concat(inner, val.files)
				} else return inner
			}, [])
			return 0 < _.size(temp) ? _.set(outer, id, _.uniq(temp)) : outer
		}, {})),
	BuildAfter: partitioned(['StepAfter', 'BuildOutputs', 'BuildImplicits'],
		(part, outs, imps) =>
			_.reduce(part, (acc, list, step) =>
				_.set(acc, step, _.uniq(_.flatMap(list, (id) =>
					_.concat(_.get(imps, id, []),
						_.get(outs, `${id}.single`, []),
						_.get(outs, `${id}.multiple`, []))))), {})),
	NinjaVersion: ['NinjaCfg', async ({NinjaCfg}) => {
		writer.variable('ninja_required_version',
			_.get(NinjaCfg, 'version', null), false)}],
	NinjaDirectory: ['NinjaCfg', 'NinjaVersion', async ({NinjaCfg}) => {
		writer.variable('builddir',
			_.get(NinjaCfg, 'directory', null), false)}],
	NinjaPools: ['NinjaCfg', async ({NinjaCfg}) =>
		Ninja.process_pools(NinjaCfg.pools)],
	NinjaPoolsOut: ['NinjaPools', 'NinjaDirectory',
		async ({NinjaPools: pools}) => {
			_.reduce(pools, (acc, val, key) =>
				acc.pool(val, key), writer)}],
	NinjaDefVars: ['NinjaCfg', async ({NinjaCfg}) =>
		Ninja.process_def_variables(NinjaCfg.variables)],
	NinjaDefVarsOut: ['NinjaDefVars', 'NinjaPoolsOut',
		async ({NinjaDefVars: vars}) => {
			_.reduce(vars, (acc, val, key) =>
				acc.variable(key, val, false), writer)}],
	NinjaRules: ['NinjaCfg', async ({NinjaCfg}) =>
		Ninja.process_rules(NinjaCfg.rules)],
	NinjaRulesOut: ['NinjaRules', 'NinjaDefVarsOut',
		async ({NinjaRules: rules}) => {
			_.reduce(rules, (acc, val, key) =>
				acc.rule(val, key), writer)}],
	NinjaBuilds: partitioned(['BuildActions', 'BuildOutputs', 'BuildImplicits',
		'BuildInputs', 'BuildDepends', 'BuildAfter', 'BuildVars'],
		(acts, outs, imps, ins, deps, afters, vars) =>
			_.reduce(acts, (acc, action, id) => {
				return _.concat(acc, Build.create(action, _.get(outs, id, []),
					_.get(imps, id, []), _.get(ins, id, []),
					_.get(deps, id, []), _.get(afters, id, []),
					_.get(vars, id, {})))
			}, [])),
	NinjaBuildsOut: ['NinjaRulesOut', 'NinjaBuilds',
		async ({NinjaBuilds: parts}) => {
			_.each(parts, (builds) =>
				_.reduce(builds, (acc, val) => acc.build(val), writer))
		}],
	NinjaAliases: ['NinjaCfg', async ({NinjaCfg}) => {
		const {separator, targets, prefix, suffix} = NinjaCfg.aliases
		return _.map(targets, (val, base) =>
			new Product(val.step, {base, separator, prefix, suffix,
				filters: val.filters}))
	}],
	NinjaAliasesPart: partition('Variants', 'NinjaAliases', (aliases, variant) =>
		_.reduce(_.uniqBy(aliases, 'id'),
			(acc, item) => _.set(acc, item.partition(variant), item.id),
			{})),
	NinjaAliasesAssign: partitioned(['NinjaAliasesPart', 'BuildOutputs', 'BuildImplicits'],
		(part, outs, deps) => _.reduce(part, (acc, val, key) => {
			const x = _.get(outs, val, {})
			const y = _.get(deps, val, [])
			return _.set(acc, key, _.union(y, x.single, x.multiple))}, {})),
	NinjaAliasesOut: ['NinjaBuildsOut', 'NinjaAliasesAssign',
		async ({NinjaAliasesAssign: aliases}) => {
			_.each(aliases, (part) =>
				_.reduce(part, (acc, val, key) =>
					acc.build(Build.alias(key, val)), writer))
		}],
	NinjaDefaultOut: ['NinjaAliasesOut', 'NinjaCfg',
		async ({NinjaCfg}) => {writer.defs(NinjaCfg.defaults)}]
}).then((results) => {
	console.log('Completed.')
	display(_.pick(results,
		['PartVars', 'BuildActions', 'BuildVars']),
		options.verbose)
}, (err) => {throw err}).catch((err) => {
	console.log('Error!')
	console.log(err)
}).finally(() => {
	writer.close()
})

}
