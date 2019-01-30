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
const max = require('lodash/max')
const reduce = require('lodash/reduce')
const includes = require('lodash/includes')
const set = require('lodash/set')
const get = require('lodash/get')
const at = require('lodash/at')
const forOwn = require('lodash/forOwn')
const keys = require('lodash/keys')
const each = require('lodash/forEach')
const size = require('lodash/size')
const head = require('lodash/head')
const initial = require('lodash/initial')
const last = require('lodash/last')
const unset = require('lodash/unset')
const bind = require('lodash/bind')
const identity = require('lodash/identity')
const isArray = require('lodash/isArray')
const async_queue = require('neo-async/queue')
const { machine } = require('asyncmachine')
// spellcheck: on

/**
 * @type Pipeline
 */
class Pipeline
{
	/**
	 *
	 * @param {string | symbol} id
	 * @param {Number} concurrency
	 * @param {Object} tasks
	 */
	constructor(id, concurrency, tasks)
	{
		const name = String(id)
		this.filters = {}
		this.stages = keys(tasks)
		this.emitter = new EventEmitter()
		this.emitter.on('error', bind(this.abort, this))
		this.inputs = reduce(this.stages, (acc, x) => set(acc, x, {}), {})
		this.automata = machine(reduce(this.stages,
				(acc, x) => set(acc, x, {}), {}))
			.id(`Pipeline.${name}.automata`)
		this.guard = machine(
			{
				Ready: {},
				Abort: {},
				Complete: {}
			})
			.id(`Pipeline.${name}.guard`)
		each([this.automata, this.guard], (x) => x.set([]))
		this.queue = async_queue(bind(this.worker, this), max([1, Math.round(concurrency)]))
		this.prepare(tasks)
	}

	/**
	 * @param {string | symbol} stage
	 * @param {Error} error
	 */
	abort(stage, error)
	{
		this.guard.add('Abort')
		this.emitter.emit('failure', stage, error)
	}

	/**
	 * @param {Array<string | symbol>} states
	 * @returns {Promise}
	 */
	condition(states)
	{
		return Promise.race([this.guard.when('Abort'),
			this.automata.when(states)
		])
	}

	/**
	 * Applies the filter for the stage if there is one and emits the event for the stage.
	 * @param {string | symbol} id The ID of the stage to forward the result of.
	 * @param {*} result
	 */
	forward(id, result)
	{
		const filter = get(this.filters, id, identity)
		this.emitter.emit(id, filter(result))
	}

	/**
	 * Signals that a stage is complete and calls the worker exit callback.
	 * @param {string | symbol} id The ID of the stage that completed.
	 * @param {() => void} cb The completion callback for th worker.
	 */
	complete(id, cb)
	{
		this.automata.add(id)
		unset(this.inputs, id)
		cb()
	}

	/**
	 * The queue worker implementation.
	 * @param {Object} param0
	 * @param {string | symbol} param0.id The ID of the stage to run.
	 * @param {() => any} param0.task The ID of the stage to run.
	 * @param {() => void} cb The completion callback for th worker.
	 */
	worker({ id, task }, cb)
	{
		if (this.guard.is('Abort'))
		{
			this.complete(id, cb)
		}
		else
		{
			task()
				.then((result) => this.forward(id, result),
					(err) => { throw err })
				.catch((err) => this.abort(id, err))
				.finally(() => this.complete(id, cb))
		}
	}

	/**
	 * Sets an event to add the result of a task to the input set for all tasks that depend on it.
	 * @param {string | symbol} from
	 * @param {Array<string | symbol>} depends
	 */
	setReport(from, depends)
	{
		this.emitter.once(from, (result) =>
		{
			each(depends, (to) => set(this.inputs[to], from, result))
		})
	}

	/**
	 * Sets an event to generate a log event when a task completes.
	 * @param {string | symbol} id The ID of the stage to set an event for.
	 */
	setLogEvent(id)
	{
		this.emitter.once(id, (result) => this.emitter.emit('log', id, result))
	}

	standalone(id, task)
	{
		unset(this.inputs, id)
		this.setLogEvent(id)
		this.guard.when('Ready')
			.then(this.queue.push({ id, task }))
	}

	dependent(id, task, deps)
	{
		this.setLogEvent(id)
		this.condition(deps)
			.then(() =>
				this.queue.push({ id, task: () => task(this.inputs[id]) }))
	}

	/**
	 *
	 * @param {*} tasks
	 */
	prepare(tasks)
	{
		let deps = reduce(this.stages, (acc, x) => set(acc, x, []), {})
		// Queue independent tasks and set others to be queued once dependencies complete.
		each(tasks, (val, key) =>
		{
			if (isArray(val))
			{
				const len = size(val)
				if (len === 1)
				{
					this.standalone(key, head(val))
				}
				else if (len > 1)
				{
					const inputs = initial(val)
					each(at(deps, inputs), (from) => from.push(key))
					this.dependent(key, last(val), inputs)
				}
				else
				{
					this.guard.add('Abort',
						Error('A task must have a function at the end of the array.'))
					this.automata.add(key)
				}
			}
			else
			{
				this.standalone(key, val)
			}
		})
		forOwn(deps, (to, from) => this.setReport(from, to))
	}

	async run()
	{
		if (!this.guard.is('Complete'))
		{
			this.guard.set('Ready')
			// Wait for all tasks to complete.
			await this.condition(this.stages)
			this.guard.set('Complete')
			this.emitter.emit('complete')
		}
	}

	applyFilter(id, filter)
	{
		if (includes(this.stages, id))
		{
			set(this.filters, id, filter)
		}
		else
		{
			throw Error('Attempted to apply a filter for a stage that was not in the Pipeline.')
		}
	}

	/**
	 * @param {string | symbol} id
	 * @param {(result: any) => void} listener
	 */
	applyListener(id, listener)
	{
		if (includes(this.stages, id))
		{
			this.emitter.once(id, listener)
		}
		else
		{
			throw Error('Attempted to apply a listener for a stage that was not in the Pipeline.')
		}
	}

	/**
	 * @param {(stage: string | symbol | null, result: any) => void} logger
	 */
	applyLogger(logger) { this.emitter.on('log', logger) }

	/**
	 * @param {(stage: string | symbol | null, err: Error) => void} logger
	 */
	onFailure(logger) { this.emitter.on('failure', logger) }

	/**
	 * @param {() => void} listener
	 */
	onComplete(listener) { this.emitter.once('complete', listener) }
}

module.exports = {
	Pipeline: Pipeline
}
