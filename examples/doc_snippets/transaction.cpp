/*
 * Copyright 2016-2020, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * transaction.cpp -- C++ documentation snippets.
 */

/*
 * The following might be necessary to compile the examples on older compilers.
 */
#if !defined(__cpp_lib_uncaught_exceptions) && !defined(_MSC_VER) ||           \
	(_MSC_VER < 1900)

#define __cpp_lib_uncaught_exceptions 201411
namespace std
{

int
uncaught_exceptions() noexcept
{
	return 0;
}

} /* namespace std */
#endif /* __cpp_lib_uncaught_exceptions */

//! [general_tx_example]
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/mutex.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pext.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/shared_mutex.hpp>
#include <libpmemobj++/transaction.hpp>

using namespace pmem::obj;

void
general_tx_example()
{
	// pool root structure
	struct root {
		mutex pmutex;
		shared_mutex shared_pmutex;
		p<int> count;
		persistent_ptr<root> another_root;
	};

	// create a pmemobj pool
	auto pop = pool<root>::create("poolfile", "layout", PMEMOBJ_MIN_POOL);
	auto proot = pop.root();

	// typical usage schemes
	try {
		// take locks and start a transaction
		transaction::run(
			pop,
			[&]() {
				// atomically allocate objects
				proot->another_root = make_persistent<root>();

				// atomically modify objects
				proot->count++;
			},
			proot->pmutex, proot->shared_pmutex);
	} catch (pmem::transaction_error &) {
		// a transaction error occurred, transaction got aborted
		// reacquire locks if necessary
	} catch (...) {
		// some other exception got propagated from within the tx
		// reacquire locks if necessary
	}
}
//! [general_tx_example]

//! [manual_tx_example]
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/mutex.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pext.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/shared_mutex.hpp>
#include <libpmemobj++/transaction.hpp>

using namespace pmem::obj;

int
manual_tx_example()
{
	// pool root structure
	struct root {
		mutex pmutex;
		shared_mutex shared_pmutex;
		p<int> count;
		persistent_ptr<root> another_root;
	};

	// create a pmemobj pool
	auto pop = pool<root>::create("poolfile", "layout", PMEMOBJ_MIN_POOL);

	auto proot = pop.root();

	try {
		transaction::manual tx(pop, proot->pmutex,
				       proot->shared_pmutex);

		// atomically allocate objects
		proot->another_root = make_persistent<root>();

		// atomically modify objects
		proot->count++;

		// It's necessary to commit the transaction manually and
		// it has to be the last operation in the transaction.
		transaction::commit();
	} catch (pmem::transaction_error &) {
		// an internal transaction error occurred, tx aborted
		// reacquire locks if necessary
	} catch (...) {
		// some other exception thrown, tx aborted
		// reacquire locks if necessary
	}

	// In complex cases with library calls, remember to check the status of
	// the previous transaction.
	return transaction::error();
}
//! [manual_tx_example]

//! [automatic_tx_example]
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/mutex.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pext.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/shared_mutex.hpp>
#include <libpmemobj++/transaction.hpp>

using namespace pmem::obj;

int
automatic_tx_example()
{
	// pool root structure
	struct root {
		mutex pmutex;
		shared_mutex shared_pmutex;
		p<int> count;
		persistent_ptr<root> another_root;
	};

	// create a pmemobj pool
	auto pop = pool<root>::create("poolfile", "layout", PMEMOBJ_MIN_POOL);
	auto proot = pop.root();

	try {
		transaction::automatic tx(pop, proot->pmutex,
					  proot->shared_pmutex);

		// atomically allocate objects
		proot->another_root = make_persistent<root>();

		// atomically modify objects
		proot->count++;

		// manual transaction commit is no longer necessary
	} catch (pmem::transaction_error &) {
		// an internal transaction error occurred, tx aborted
		// reacquire locks if necessary
	} catch (...) {
		// some other exception thrown, tx aborted
		// reacquire locks if necessary
	}

	// In complex cases with library calls, remember to check the status of
	// the previous transaction.
	return transaction::error();
}
//! [automatic_tx_example]

//! [tx_callback_example]
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pext.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/transaction.hpp>

using namespace pmem::obj;

void
tx_callback_example()
{
	// pool root structure
	struct root {
		p<int> count;
	};

	// create a pmemobj pool
	auto pop = pool<root>::create("poolfile", "layout", PMEMOBJ_MIN_POOL);

	bool cb_called = false;
	auto internal_tx_function = [&] {
		// callbacks can be registered even in inner transaction but
		// will be called when outer transaction ends
		transaction::run(pop, [&] {
			transaction::register_callback(
				transaction::stage::oncommit,
				[&] { cb_called = true; });
		});

		// cb_called is false here if internal_tx_function is called
		// inside another transaction
	};

	try {
		transaction::run(pop, [&] { internal_tx_function(); });

		// cb_called == true if transaction ended successfully
	} catch (pmem::transaction_error &) {
		// an internal transaction error occurred, tx aborted
		// reacquire locks if necessary
	} catch (...) {
		// some other exception thrown, tx aborted
		// reacquire locks if necessary
	}
}
//! [tx_callback_example]
