/*
 * Copyright 2019-2020, Intel Corporation
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

#include "unittest.hpp"

#include <libpmemobj++/container/string.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/transaction.hpp>

struct root {
};

using char_string = pmem::obj::basic_string<char>;
using char16_string = pmem::obj::basic_string<char16_t>;
using char32_string = pmem::obj::basic_string<char32_t>;
using wchar_string = pmem::obj::basic_string<wchar_t>;

void
test_capacity(pmem::obj::pool<root> &pop)
{
	pmem::obj::transaction::run(pop, [&] {
		auto ptr1 = pmem::obj::make_persistent<char_string>();
		UT_ASSERTeq(ptr1->capacity(), 23);

		pmem::obj::delete_persistent<char_string>(ptr1);
	});

	pmem::obj::transaction::run(pop, [&] {
		auto ptr1 = pmem::obj::make_persistent<char16_string>();
		UT_ASSERTeq(ptr1->capacity(), 11);

		pmem::obj::delete_persistent<char16_string>(ptr1);
	});

	pmem::obj::transaction::run(pop, [&] {
		auto ptr1 = pmem::obj::make_persistent<char32_string>();
		UT_ASSERTeq(ptr1->capacity(), 5);

		pmem::obj::delete_persistent<char32_string>(ptr1);
	});
}

static void
test(int argc, char *argv[])
{
	if (argc < 2) {
		UT_FATAL("usage: %s file-name", argv[0]);
	}

	auto path = argv[1];
	auto pop = pmem::obj::pool<root>::create(
		path, "StringTest", PMEMOBJ_MIN_POOL, S_IWUSR | S_IRUSR);

	static_assert(sizeof(char_string) == 32, "");
	static_assert(sizeof(char16_string) == 32, "");
	static_assert(sizeof(char32_string) == 32, "");
	static_assert(sizeof(wchar_string) == 32, "");

	static_assert(std::is_standard_layout<char_string>::value, "");
	static_assert(std::is_standard_layout<char16_string>::value, "");
	static_assert(std::is_standard_layout<char32_string>::value, "");
	static_assert(std::is_standard_layout<wchar_string>::value, "");

	test_capacity(pop);

	pop.close();
}

int
main(int argc, char *argv[])
{
	return run_test([&] { test(argc, argv); });
}
