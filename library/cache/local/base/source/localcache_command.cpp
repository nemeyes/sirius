#include <sirius_uuid.h>
#include <localcache_command.h>

sirius::library::cache::local::abstract_command::abstract_command(sirius::library::cache::local::localcache_processor * prcsr, int32_t command_id)
	: _processor(prcsr)
	, _command_id(command_id)
{
	InitializeSRWLock(&_srwl);
	if(_processor)
		_processor->add_worker(this);
}

sirius::library::cache::local::abstract_command::abstract_command(int32_t command_id)
	: _processor(nullptr)
	, _command_id(command_id)
{
	InitializeSRWLock(&_srwl);
	if(_processor)
		_processor->add_worker(this);
}

sirius::library::cache::local::abstract_command::~abstract_command(void)
{
	_tp_ready_workers.clear();
}

void sirius::library::cache::local::abstract_command::set_processor(sirius::library::cache::local::localcache_processor * prcsr)
{
	_processor = prcsr;
	if (_processor)
		_processor->add_worker(this);
}

sirius::library::cache::local::localcache_processor * sirius::library::cache::local::abstract_command::get_processor(void)
{
	return _processor;
}

int32_t sirius::library::cache::local::abstract_command::command_id(void)
{
	return _command_id;
}

BOOL sirius::library::cache::local::abstract_command::is_running(void)
{
	BOOL status = FALSE;
	{
		sirius::library::cache::local::abstract_command::tp_worker_scopped_lock mutex(&_srwl);
		if (_tp_ready_workers.size() > 0)
			status = TRUE;
	}
	return status;
}

void sirius::library::cache::local::abstract_command::_execute(int32_t command_id, const char * msg, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
{
	if(_processor->active())
	{
		sirius::library::cache::local::abstract_command::tp_worker_scopped_lock mutex(&_srwl);
		std::shared_ptr<sirius::library::cache::local::abstract_command::tp_worker> worker = std::shared_ptr<sirius::library::cache::local::abstract_command::tp_worker>(new sirius::library::cache::local::abstract_command::tp_worker);
		worker->command = this;
		worker->session = session;
		if (msg && length > 0)
		{
			worker->length = length;
			worker->msg = static_cast<char*>(malloc(worker->length));
			memmove(worker->msg, msg, worker->length);
		}
		if(worker)
			_tp_ready_workers.push_back(worker);
	}
	if (_processor)
		_processor->run_worker(_work);
}

void sirius::library::cache::local::abstract_command::tp_work_callback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work)
{
	sirius::library::cache::local::abstract_command * self = static_cast<sirius::library::cache::local::abstract_command*>(parameter);
	self->__execute();
}

void sirius::library::cache::local::abstract_command::__execute(void)
{
	std::shared_ptr<sirius::library::cache::local::abstract_command::tp_worker> worker = nullptr;
	{
		sirius::library::cache::local::abstract_command::tp_worker_scopped_lock mutex(&_srwl);
		if (_tp_ready_workers.size() > 0)
		{
			worker = _tp_ready_workers.front();
			_tp_ready_workers.pop_front();
		}
	}
	if (worker && _processor->active())
		execute(_command_id, worker->msg, worker->length, worker->session);
}