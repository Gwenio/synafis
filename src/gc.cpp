
/*
ISC License (ISC)

Copyright 2018 Adam Armstrong

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

#include "gc.hpp"

#include <atomic>

namespace gc {

class soft_ptr::data {
public:
	/**	\var ptr
	 *	\brief A pointer to the object weakly referenced.
	 */
	void *ptr;

	/**	\var next
	 *	\brief A pointer to another data that takes precedence over this.
	 *	\details When immutable objects are merged, soft pointers to those
	 *	\details objects have next set to the data of the soft pointer for
	 *	\details the remaining object.
	 *	\todo It is likely that either next does not need to be atomic or
	 *	\todo that ptr does. But more of the collector interals need to be
	 *	\todo implemented to tell for sure.
	 */
	std::atomic<data *> next;

	/**	\var count
	 *	\brief Number of times the data is referenced.
	 */
	std::atomic<std::size_t> count;

	/**	\fn data(void *obj) noexcept
	 *	\brief Creates a new data object.
	 *	\param ptr The address of the object the data points to.
	 *	\details The reference count starts at zero so we can increment it with
	 *	\details copy when returned by collector::get_soft_ptr just as we would
	 *	\details for a pre-existing data.
	 */
	constexpr data(void *obj) noexcept :
		ptr(obj),  next(nullptr), count(0) {}

	/**	\fn ~data() noexcept
	 *	\brief Default.
	 */
	~data() noexcept = default;

	/**	\fn operator=(std::nullptr_t) noexcept
	 *	\brief Sets ptr to nullptr.
	 *	\returns *this
	 *	\post The data is stale, and will be deallocated when the reference count reaches zero.
	 */
	constexpr data &operator=(std::nullptr_t) noexcept {
		ptr = nullptr;
		return *this;
	}

	/**	\fn operator=(data *other) noexcept
	 *	\brief Sets next to other.
	 *	\param other The value to set next to.
	 *	\returns *this
	 *	\post The data is stale, and will be deallocated when the reference count reaches zero.
	 */
	data &operator=(data *other) noexcept {
		SYNAFIS_ASSERT(other != nullptr);
		SYNAFIS_ASSERT(other != this);
		SYNAFIS_ASSERT(other->next == nullptr);
		next.store(other);
		return *this;
	}
};

}

#include "gc/impl.hpp"

namespace gc {

/**	\var impl
 *	\brief The singleton of type collector.
 */
static collector impl{};

identity::~identity() {
#warning Not implemented.
}

std::tuple<void *, identity::allocate_cb>
identity::select_alloc(identity const&id, std::size_t size,
		std::size_t align, traits::flag_type flags) {
#warning Not implemented.
	return std::tuple<void *, identity::allocate_cb>{nullptr, nullptr};
}

identity const* identity::fetch_impl(void *obj) noexcept {
#warning Not implemented.
	return nullptr;
}

/**	\var lock_count
 *	\brief Count how many times a thread has acquired the collector's lock.
 */
thread_local std::size_t lock_count{0};

scoped_lock::scoped_lock() {
	if (lock_count++ == 0) {
		impl.lock();
	}
}

scoped_lock::~scoped_lock() {
	if (--lock_count == 0) {
		impl.unlock();
	}
}

soft_ptr::data *soft_ptr::get_soft(hard_ptr const&other) {
	return copy(impl.get_soft_ptr(other.ptr));
}

soft_ptr::data *soft_ptr::copy(soft_ptr::data *other) noexcept {
	data *temp{other->next.load(std::memory_order_relaxed)};
	if (temp) {
		return copy(temp);
	} else if (other->ptr) {
		other->count.fetch_add(1, std::memory_order_relaxed);
		return other;
	} else {
		return nullptr;
	}
}

soft_ptr::data *soft_ptr::update(soft_ptr::data *old) noexcept {
	data *temp{old->next.load(std::memory_order_relaxed)};
	if (temp) {
		soft_ptr::free(old);
		return copy(temp);
	} else if (old->ptr) {
		return old;
	} else {
		soft_ptr::free(old);
		return nullptr;
	}
}

void soft_ptr::free(soft_ptr::data *other) noexcept {
	std::size_t temp{other->count.fetch_sub(1, std::memory_order_seq_cst)};
	if ((!(other->ptr) || other->next) && temp == 1) {	// temp == 1 means it is now zero
		impl.free_soft_ptr(other);
	}
}

std::tuple<void *, identity const*> hard_ptr::get_hard(soft_ptr const&other) {
	using rtype = std::tuple<void *, identity const*>;
	if (other.ptr && other.ptr->ptr) {
		return rtype{other.ptr->ptr,
			identity::fetch(other.ptr->ptr, std::nothrow)};
	} else {
		return rtype{nullptr, nullptr};
	}
}

void *hard_ptr::base_ptr(void *source) noexcept {
	return impl.base_ptr(source);
}

void root::register_impl(void *obj, traverse_cb tcb, root_cb rcb) {
	SYNAFIS_ASSERT(obj != nullptr);
	SYNAFIS_ASSERT(tcb != nullptr);
	SYNAFIS_ASSERT(rcb != nullptr);
#warning Not implemented.
	;
}

void root::unregister_impl(void *obj) {
	SYNAFIS_ASSERT(obj != nullptr);
#warning Not implemented.
	;
}

void initialize() {
	impl.init();
}

}
