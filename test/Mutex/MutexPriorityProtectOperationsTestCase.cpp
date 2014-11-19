/**
 * \file
 * \brief MutexPriorityProtectOperationsTestCase class implementation
 *
 * \author Copyright (C) 2014 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \date 2014-11-19
 */

#include "MutexPriorityProtectOperationsTestCase.hpp"

#include "waitForNextTick.hpp"

#include "distortos/Mutex.hpp"
#include "distortos/ThisThread.hpp"

#include <cerrno>

namespace distortos
{

namespace test
{

namespace
{

/*---------------------------------------------------------------------------------------------------------------------+
| local constants
+---------------------------------------------------------------------------------------------------------------------*/

/// single duration used in tests
constexpr auto singleDuration = TickClock::duration{1};

/// priority of current test thread
constexpr uint8_t testThreadPriority {1};

/*---------------------------------------------------------------------------------------------------------------------+
| local functions
+---------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Tests invalid lock attempts.
 *
 * \param [in] type is the Mutex::Type that will be tested
 *
 * \return true if the test case succeeded, false otherwise
 */

bool testInvalidLockAttempt(const Mutex::Type type)
{
	Mutex mutex {type, Mutex::Protocol::PriorityProtect, testThreadPriority - 1};

	{
		// invalid lock attempt - must fail with EINVAL immediately
		waitForNextTick();
		const auto start = TickClock::now();
		const auto ret = mutex.lock();
		if (ret != EINVAL || start != TickClock::now())
			return false;
	}

	{
		// invalid lock attempt - must fail with EINVAL immediately
		waitForNextTick();
		const auto start = TickClock::now();
		const auto ret = mutex.tryLock();
		if (ret != EINVAL || start != TickClock::now())
			return false;
	}

	{
		// invalid lock attempt - must fail with EINVAL immediately
		waitForNextTick();
		const auto start = TickClock::now();
		const auto ret = mutex.tryLockFor(singleDuration);
		if (ret != EINVAL || start != TickClock::now())
			return false;
	}

	{
		// invalid lock attempt - must fail with EINVAL immediately
		waitForNextTick();
		const auto start = TickClock::now();
		const auto ret = mutex.tryLockUntil(start + singleDuration);
		if (ret != EINVAL || start != TickClock::now())
			return false;
	}

	return true;
}

/**
 * \brief Tests changes of priority resulting from locking mutexes with PriorityProtect protocol.
 *
 * \param [in] type is the Mutex::Type that will be tested
 *
 * \return true if the test case succeeded, false otherwise
 */

bool testPriorityChanges(const Mutex::Type type)
{
	// function (lock/unlock), index of mutex ([0;4] only!), expected effective priority
	using Operation = std::tuple<int (Mutex::*)(), uint8_t, uint8_t>;
	static const Operation operations[]
	{
			// ascending
			Operation{&Mutex::lock, 0, testThreadPriority + 0},		// *----
			Operation{&Mutex::lock, 1, testThreadPriority + 1},		// **---
			Operation{&Mutex::lock, 2, testThreadPriority + 2},		// ***--
			Operation{&Mutex::lock, 3, testThreadPriority + 3},		// ****-
			Operation{&Mutex::lock, 4, testThreadPriority + 4},		// *****
			Operation{&Mutex::unlock, 0, testThreadPriority + 4},	// -****
			Operation{&Mutex::unlock, 1, testThreadPriority + 4},	// --***
			Operation{&Mutex::unlock, 2, testThreadPriority + 4},	// ---**
			Operation{&Mutex::unlock, 3, testThreadPriority + 4},	// ----*
			Operation{&Mutex::unlock, 4, testThreadPriority + 0},	// -----
			// descending
			Operation{&Mutex::lock, 4, testThreadPriority + 4},		// ----*
			Operation{&Mutex::lock, 3, testThreadPriority + 4},		// ---**
			Operation{&Mutex::lock, 2, testThreadPriority + 4},		// --***
			Operation{&Mutex::lock, 1, testThreadPriority + 4},		// -****
			Operation{&Mutex::lock, 0, testThreadPriority + 4},		// *****
			Operation{&Mutex::unlock, 4, testThreadPriority + 3},	// ****-
			Operation{&Mutex::unlock, 3, testThreadPriority + 2},	// ***--
			Operation{&Mutex::unlock, 2, testThreadPriority + 1},	// **---
			Operation{&Mutex::unlock, 1, testThreadPriority + 0},	// *----
			Operation{&Mutex::unlock, 0, testThreadPriority + 0},	// -----
			// "random"
			Operation{&Mutex::lock, 1, testThreadPriority + 1},		// -*---
			Operation{&Mutex::lock, 0, testThreadPriority + 1},		// **---
			Operation{&Mutex::lock, 3, testThreadPriority + 3},		// **-*-
			Operation{&Mutex::lock, 2, testThreadPriority + 3},		// ****-
			Operation{&Mutex::lock, 4, testThreadPriority + 4},		// *****
			Operation{&Mutex::unlock, 4, testThreadPriority + 3},	// ****-
			Operation{&Mutex::unlock, 2, testThreadPriority + 3},	// **-*-
			Operation{&Mutex::unlock, 3, testThreadPriority + 1},	// **---
			Operation{&Mutex::unlock, 0, testThreadPriority + 1},	// -*---
			Operation{&Mutex::unlock, 1, testThreadPriority + 0},	// -----
			// mixed sequence
			Operation{&Mutex::lock, 0, testThreadPriority + 0},		// *----
			Operation{&Mutex::lock, 1, testThreadPriority + 1},		// **---
			Operation{&Mutex::unlock, 0, testThreadPriority + 1},	// -*---
			Operation{&Mutex::lock, 2, testThreadPriority + 2},		// -**--
			Operation{&Mutex::unlock, 1, testThreadPriority + 2},	// --*--
			Operation{&Mutex::lock, 3, testThreadPriority + 3},		// --**-
			Operation{&Mutex::unlock, 2, testThreadPriority + 3},	// ---*-
			Operation{&Mutex::lock, 4, testThreadPriority + 4},		// ---**
			Operation{&Mutex::unlock, 3, testThreadPriority + 4},	// ----*
			Operation{&Mutex::lock, 3, testThreadPriority + 4},		// ---**
			Operation{&Mutex::lock, 1, testThreadPriority + 4},		// -*-**
			Operation{&Mutex::unlock, 4, testThreadPriority + 3},	// -*-*-
			Operation{&Mutex::lock, 2, testThreadPriority + 3},		// -***-
			Operation{&Mutex::lock, 0, testThreadPriority + 3},		// ****-
			Operation{&Mutex::unlock, 3, testThreadPriority + 2},	// ***--
			Operation{&Mutex::lock, 4, testThreadPriority + 4},		// ***-*
			Operation{&Mutex::unlock, 2, testThreadPriority + 4},	// **--*
			Operation{&Mutex::lock, 3, testThreadPriority + 4},		// **-**
			Operation{&Mutex::unlock, 0, testThreadPriority + 4},	// -*-**
			Operation{&Mutex::unlock, 4, testThreadPriority + 3},	// -*-*-
			Operation{&Mutex::unlock, 3, testThreadPriority + 1},	// -*---
			Operation{&Mutex::unlock, 1, testThreadPriority + 0},	// -----
	};

	Mutex mutexes[]
	{
			Mutex{type, Mutex::Protocol::PriorityProtect, testThreadPriority + 0},
			Mutex{type, Mutex::Protocol::PriorityProtect, testThreadPriority + 1},
			Mutex{type, Mutex::Protocol::PriorityProtect, testThreadPriority + 2},
			Mutex{type, Mutex::Protocol::PriorityProtect, testThreadPriority + 3},
			Mutex{type, Mutex::Protocol::PriorityProtect, testThreadPriority + 4},
	};

	bool result {true};

	for (const auto& operation : operations)
	{
		const auto function = std::get<0>(operation);
		const auto index = std::get<1>(operation);	/// \todo check index of mutex
		const auto expectedEffectivePriority = std::get<2>(operation);
		const auto ret = (mutexes[index].*function)();
		if (ret != 0 || ThisThread::getEffectivePriority() != expectedEffectivePriority)
			result = false;
	}

	return result;
}

/**
 * \brief Runs the test case.
 *
 * \attention this function expects the priority of test thread to be testThreadPriority
 *
 * \return true if the test case succeeded, false otherwise
 */

bool testRunner()
{
	static const Mutex::Type types[]
	{
			Mutex::Type::Normal,
			Mutex::Type::ErrorChecking,
			Mutex::Type::Recursive,
	};

	for (const auto type : types)
	{
		{
			const auto result = testInvalidLockAttempt(type);
			if (result != true)
				return result;
		}

		{
			const auto result = testPriorityChanges(type);
			if (result != true)
				return result;
		}
	}

	return true;
}

}	// namespace

/*---------------------------------------------------------------------------------------------------------------------+
| private functions
+---------------------------------------------------------------------------------------------------------------------*/

bool MutexPriorityProtectOperationsTestCase::Implementation::run_() const
{
	const auto thisThreadPriority = ThisThread::getPriority();
	ThisThread::setPriority(testThreadPriority);
	const auto ret = testRunner();
	ThisThread::setPriority(thisThreadPriority);
	return ret;
}

}	// namespace test

}	// namespace distortos
