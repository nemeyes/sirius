#include <platform.h>
#include <localcache_command.h>
#include <localcache_base.h>
#include <sirius_locks.h>

sirius::library::cache::local::base::base(int32_t number_of_threads)
	: _number_of_threads(number_of_threads)
	, _pool(NULL)
{
	::InitializeCriticalSection(&_lwiftp_base_cs);
}

sirius::library::cache::local::base::~base(void)
{
	::DeleteCriticalSection(&_lwiftp_base_cs);
}

void sirius::library::cache::local::base::initialize(void)
{
	sirius::autolock mutex(&_lwiftp_base_cs);

	if (_pool)
		return;

	::InitializeThreadpoolEnvironment(&_callback_env);

	if (_number_of_threads == 0)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		// 디폴트 쓰레드 수로 
		// 2 * 프로세서수 + 2 의 공식을 따랐음
		_number_of_threads = (si.dwNumberOfProcessors * 2) + 2;
	}

	do
	{
		_pool = ::CreateThreadpool(NULL);
		if (_pool == NULL)
			break;

		::SetThreadpoolThreadMaximum(_pool, _number_of_threads);
		BOOL status = ::SetThreadpoolThreadMinimum(_pool, 1);
		if (!status)
		{
			::CloseThreadpool(_pool);
			break;
		}

		_cleanup_group = ::CreateThreadpoolCleanupGroup();
		if (!_cleanup_group)
		{
			::CloseThreadpool(_pool);
			break;
		}

		::SetThreadpoolCallbackPool(&_callback_env, _pool);
		::SetThreadpoolCallbackCleanupGroup(&_callback_env, _cleanup_group, NULL);

	} while (FALSE);
}

void sirius::library::cache::local::base::release(void)
{
	sirius::autolock mutex(&_lwiftp_base_cs);

	if (!_pool)
		return;

	::CloseThreadpoolCleanupGroupMembers(_cleanup_group, FALSE, NULL);
	::CloseThreadpoolCleanupGroup(_cleanup_group);
	::CloseThreadpool(_pool);
	_pool = NULL;

}

void sirius::library::cache::local::base::add_worker(sirius::library::cache::local::abstract_command * command)
{
	if (!command)
		return;
	{
		sirius::autolock mutex(&_lwiftp_base_cs);
		if (!_pool)
			return;
	}
	command->_work = ::CreateThreadpoolWork(sirius::library::cache::local::abstract_command::tp_work_callback, command, &_callback_env);
}

void sirius::library::cache::local::base::run_worker(PTP_WORK work)
{
	if (!work)
		return;
	{
		sirius::autolock mutex(&_lwiftp_base_cs);
		if (!_pool)
			return;
	}
	::SubmitThreadpoolWork(work);
}

void sirius::library::cache::local::base::add_forarded_command(int32_t command_id)
{
	_valid_command_ids.push_back(command_id);
}

BOOL sirius::library::cache::local::base::check_valid_command_id(int32_t command_id)
{
	std::vector<int32_t> c = _valid_command_ids;//;get_command_entries();
	std::vector<int32_t>::iterator pos = c.begin();
	for (; pos != c.end(); ++pos)
		if ((*pos) == command_id)
			return TRUE;

	return FALSE;
}