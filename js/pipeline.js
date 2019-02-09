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
const _ = {}
_.map = require('lodash/map')
_.includes = require('lodash/includes')
_.concat = require('lodash/concat')
_.spread = require('lodash/spread')
_.zip = require('lodash/zip')
const { machine } = require('asyncmachine')
// spellcheck: on

class Callbacks
{
	/**
	 *
	 * @param {(result: Promise<any | null>) => Promise<any | null>} done
	 * @param {() => boolean} aborted
	 */
	constructor(done, aborted)
	{
		this.done = done
		this.aborted = aborted
	}
}

/**
 * Type for a Stage of a Task.
 * @template Result, Previous
 */
class Stage
{
	/**
	 * Constructs a Stage.
	 * @public
	 * @param {Callbacks} cb
	 * @param {(previous: Previous, ...args: any[]) => Promise<Result> | Result} action
	 * @param {Promise<Previous>} previous
	 * @param {any[]} inputs
	 */
	constructor(cb, action, previous, inputs)
	{
		/** @private */
		this.cb = cb
		/** @private */
		this.result = this.bind(action, Promise.all(_.concat(previous, inputs)))
	}

	/**
	 * @private
	 * @param {(previous: Previous, ...args: any[]) => Promise<Result> | Result} action
	 * @param {Promise<any[]>} inputs
	 * @returns {Promise<Result | null>}
	 */
	async bind(action, inputs)
	{
		const results = await inputs
		return this.cb.aborted() ? null : action.apply(null, results)
	}

	/**
	 * @public
	 * @returns {Promise<Result | null>}
	 */
	complete()
	{
		return this.cb.done(this.result)
	}

	/**
	 * @public
	 * @template Next
	 * @template Args...
	 * @param {(previous: Result, ...args: Args[]) => Promise<Next> | Next} action
	 * @param {...(Promise<Args> | Args)} [inputs]
	 * @returns {Stage<Next, Result>}
	 */
	stage(action, ...inputs)
	{
		return new Stage(this.cb, action, this.result, inputs)
	}

	/**
	 * @public
	 * @template T
	 * @param {Promise<T[]>} by
	 * @param {(by: T, from: Result) => Result} select
	 * @returns {Stage<Result[], Result>}
	 */
	partition(by, select)
	{
		/**
		 * @param {Result} from
		 * @param {T[]} parts
		 * @returns {Result[]}
		 */
		function action(from, parts)
		{
			return _.map(parts, (x) => select(x, from))
		}
		return this.stage(action, by)
	}

	/**
	 * @public
	 * @template Next, Part
	 * @template Args...
	 * @param {(previous: Part, ...args: Args[]) => Next} action
	 * @param {...(Promise<Args[]> | Args[])} [inputs]
	 * @returns {Stage<Next[], Result>}
	 */
	partitioned(action, ...inputs)
	{
		/**
		 * @param {Result & ArrayLike} previous
		 * @param  {...(Args[])} [temp]
		 * @returns {Next[]}
		 */
		function p_action(previous, ...temp)
		{
			return _.map(_.zip(previous, ...temp), _.spread(action))
		}
		return this.stage(p_action, ...inputs)
	}

	/**
	 * @template Next
	 * @param {(current: Stage<Result, Previous>, ...args: any[]) => Stage<Next, any>} func
	 * @param {...any} args
	 * @returns {Stage<Next, any>}
	 */
	apply(func, ...args)
	{
		return func(this, ...args)
	}
}

/**
 * Type for a Task in a Pipeline.
 * @template Result
 */
class Task
{

	/**
	 * Constructs a Task.
	 * @public
	 * @param {Callbacks} cb
	 * @param {(...args: any[]) => Promise<Result> | Result} action
	 * @param {any[]} inputs
	 */
	constructor(cb, action, inputs)
	{
		/** @private */
		this.cb = cb
		/** @private */
		this.result = this.bind(action, Promise.all(inputs))
	}

	/**
	 * @private
	 * @param {(...args: any[]) => Promise<Result> | Result} action
	 * @param {Promise<any[]>} inputs
	 * @returns {Promise<Result | null>}
	 */
	async bind(action, inputs)
	{
		const results = await inputs
		return this.cb.aborted() ? null : action.apply(null, results)
	}

	/**
	 * @public
	 * @returns {Promise<Result | null>}
	 */
	complete()
	{
		return this.cb.done(this.result)
	}

	/**
	 * @public
	 * @template Next
	 * @template Args...
	 * @param {(previous: Result, ...args: Args[]) => Promise<Next> | Next} action
	 * @param {...(Promise<Args> | Args)} inputs
	 * @returns {Stage<Next, Result>}
	 */
	stage(action, ...inputs)
	{
		return new Stage(this.cb, action, this.result, inputs)
	}

	/**
	 * @public
	 * @template T
	 * @param {Promise<T[]>} by
	 * @param {(by: T, from: Result) => Result} select
	 * @returns {Stage<Result[], Result>}
	 */
	partition(by, select)
	{
		/**
		 * @param {Result} from
		 * @param {T[]} parts
		 * @returns {Result[]}
		 */
		function action(from, parts)
		{
			return _.map(parts, (x) => select(x, from))
		}
		return this.stage(action, by)
	}

	/**
	 * @public
	 * @template Next, Part
	 * @template Args...
	 * @param {(previous: Part, ...args: Args[]) => Next} action
	 * @param {...(Promise<Args[]> | Args[])} [inputs]
	 * @returns {Stage<Next[], Result>}
	 */
	partitioned(action, ...inputs)
	{
		/**
		 * @param {Result & ArrayLike} previous
		 * @param  {...(Args[])} [temp]
		 * @returns {Next[]}
		 */
		function p_action(previous, ...temp)
		{
			return _.map(_.zip(previous, ...temp), _.spread(action))
		}
		return this.stage(p_action, ...inputs)
	}

	/**
	 * @template Next
	 * @param {(current: Task<Result>, ...args: any[]) => Stage<Next, any>} func
	 * @param {...any} args
	 * @returns {Stage<Next, any>}
	 */
	apply(func, ...args)
	{
		return func(this, ...args)
	}
}

/**
 * A Pipeline runs Tasks in the needed order in parallel up to a specified maximum.
 */
class Pipeline
{
	/**
	 *
	 * @param {string} id
	 */
	constructor(id)
	{
		this.count = 0
		/**
		 * @member this.stages
		 * @type {string[]}
		 */
		this.stages = []
		this.emitter = new EventEmitter()
		this.emitter.on('error', (err) => this.abort(null, err))
		this.automata = machine(
		{
			Ready: {},
			Abort: {},
			Complete: {}
		})
		this.automata.id(`Pipeline.${id}`)
			.set([])
		this.ready = this.automata.when(['Ready'])
	}

	/**
	 * @param {string} stage
	 * @param {Error} error
	 */
	abort(stage, error)
	{
		this.automata.add('Abort')
		this.emitter.emit('failure', stage, error)
	}

	/**
	 * @template T
	 * Signals that a stage is complete and calls the worker exit callback.
	 * @param {string} id The ID of the stage that completed.
	 * @param {Promise<T | null>} task
	 * @returns {Promise<T | null>}
	 * @private
	 */
	completion(id, task)
	{
		if (this.automata.is('Ready'))
		{
			throw new Error('The Pipeline cannot add a new task after it is ready to run.')
		}
		this.stages.push(id)
		this.count++
		return task.then((result) =>
				{
					this.emitter.emit(id, result)
					this.emitter.emit('log', id, result)
					return result
				},
				(error) => { throw error })
			.catch((error) =>
			{
				this.abort(id, error)
				return null
			})
			.finally(() =>
			{
				if (--this.count === 0)
				{
					this.automata.add('Complete')
				}
			})
	}

	/**
	 * @public
	 * @template Result
	 * @template Args...
	 * @param {string} id
	 * @param {(...args: Args[]) => Promise<Result> | Result} action
	 * @param {...(Promise<Args> | Args)} [inputs]
	 * @returns {Task<Result>}
	 */
	task(id, action, ...inputs)
	{
		/**
		 * @param {Promise<any>} result
		 * @returns {Promise<any | null>}
		 */
		const done = (result) => this.completion(id, result)
		const cb = new Callbacks(done, () => this.automata.is('Abort'))
		return new Task(cb, action, _.concat(inputs, [this.ready]))
	}

	/**
	 * @public
	 * @template Result
	 * @template Args...
	 * @param {string} id
	 * @param {(...args: Args[]) => Result} action
	 * @param {...(Promise<Args[]> | Args[])} [inputs]
	 * @returns {Task<Result[]>}
	 */
	partitioned(id, action, ...inputs)
	{
		/**
		 * @param  {...(Args[])} [temp]
		 * @returns {Result[]}
		 */
		function p_action(...temp)
		{
			/** @const {Result} x */
			const x = this
			return _.map(_.zip(x, ...temp), _.spread(action))
		}
		return this.task(id, p_action, ...inputs)
	}

	async run()
	{
		if (!this.automata.is('Complete'))
		{
			this.automata.set('Ready')
			// Wait for all tasks to complete.
			await this.automata.when(['Complete'])
			this.emitter.emit('complete')
		}
	}

	/**
	 * @param {string | symbol} id
	 * @param {(result: any) => void} listener
	 */
	applyListener(id, listener)
	{
		if (_.includes(this.stages, id))
		{
			this.emitter.once(id, listener)
		}
		else
		{
			throw new Error('Attempted to apply a listener for a stage that was not in the Pipeline.')
		}
	}

	/**
	 * @param {(stage: string | null, result: any) => void} logger
	 */
	applyLogger(logger) { this.emitter.on('log', logger) }

	/**
	 * @param {(stage: string | null, err: Error) => void} logger
	 */
	onFailure(logger) { this.emitter.on('failure', logger) }

	/**
	 * @param {() => void} listener
	 */
	onComplete(listener) { this.emitter.once('complete', listener) }
}

module.exports.Pipeline = Pipeline
module.exports.Task = Task
module.exports.Stage = Stage
