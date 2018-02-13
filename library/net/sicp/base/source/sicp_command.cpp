#include <sirius_uuid.h>
#include <sicp_command.h>

sirius::library::net::sicp::abstract_command::abstract_command(sirius::library::net::sicp::sicp_processor * prcsr, int32_t command_id)
	: _processor(prcsr)
	, _command_id(command_id)
{
	InitializeSRWLock(&_srwl);

	//::InitializeCriticalSection(&_cs);
	if(_processor)
		_processor->add_worker(this);
}

sirius::library::net::sicp::abstract_command::abstract_command(int32_t command_id)
	: _processor(nullptr)
	, _command_id(command_id)
{
	InitializeSRWLock(&_srwl);

	//::InitializeCriticalSection(&_cs);

	if(_processor)
		_processor->add_worker(this);
}

sirius::library::net::sicp::abstract_command::~abstract_command(void)
{
	_tp_ready_workers.clear();
}

void sirius::library::net::sicp::abstract_command::set_processor(sirius::library::net::sicp::sicp_processor * prcsr)
{
	_processor = prcsr;
	if (_processor)
		_processor->add_worker(this);
}

sirius::library::net::sicp::sicp_processor * sirius::library::net::sicp::abstract_command::get_processor(void)
{
	return _processor;
}

const char * sirius::library::net::sicp::abstract_command::uuid(void)
{
	return _processor->uuid();
}

int32_t sirius::library::net::sicp::abstract_command::command_id(void)
{
	return _command_id;
}

bool sirius::library::net::sicp::abstract_command::is_running(void)
{
	bool status = false;
	
	{
		sirius::library::net::sicp::abstract_command::tp_worker_scopped_lock mutex(&_srwl);
		if (_tp_ready_workers.size() > 0)
		{
			status = true;
		}
	}

	return status;
}

void sirius::library::net::sicp::abstract_command::_execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	if(_processor->active())
	{
		sirius::library::net::sicp::abstract_command::tp_worker_scopped_lock mutex(&_srwl);
		std::shared_ptr<sirius::library::net::sicp::abstract_command::tp_worker> worker = std::shared_ptr<sirius::library::net::sicp::abstract_command::tp_worker>(new sirius::library::net::sicp::abstract_command::tp_worker);
		worker->command = this;
		worker->version = version;
		strncpy_s(worker->destination, dst, sizeof(worker->destination));
		strncpy_s(worker->source, src, sizeof(worker->source));
		worker->session = session;
		if (msg && length > 0)
		{
			worker->length = length;
			//worker->msg = (char*)(malloc(worker->length));
			worker->msg = static_cast<char*>(malloc(worker->length));
			memmove(worker->msg, msg, worker->length);
		}
		if(worker)
			_tp_ready_workers.push_back(worker);
	}
	if (_processor)
		_processor->run_worker(_work);
}

void sirius::library::net::sicp::abstract_command::tp_work_callback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work)
{
	sirius::library::net::sicp::abstract_command * self = static_cast<sirius::library::net::sicp::abstract_command*>(parameter);
	self->__execute();
}

void sirius::library::net::sicp::abstract_command::__execute(void)
{
	std::shared_ptr<sirius::library::net::sicp::abstract_command::tp_worker> worker = nullptr;
	{
		sirius::library::net::sicp::abstract_command::tp_worker_scopped_lock mutex(&_srwl);
		if (_tp_ready_workers.size() > 0)
		{
			worker = _tp_ready_workers.front();
			_tp_ready_workers.pop_front();
		}
	}
	if (worker && _processor->active())
		execute(worker->destination, worker->source, _command_id, worker->version, worker->msg, worker->length, worker->session);

	//{
	//	sirius::library::net::sicp::abstract_command::tp_worker_scopped_lock mutex(&_srwl);
	//	if (_tp_ready_workers.size() > 0)
	//	{
	//		//worker = _tp_ready_workers.front();
	//		_tp_ready_workers.pop_front();
	//	}
	//}
}

#if defined(WITH_WORKING_AS_SERVER)
sirius::library::net::sicp::create_session_req::create_session_req(sirius::library::net::sicp::sicp_processor * prcsr)
		: abstract_command(prcsr, CMD_CREATE_SESSION_REQUEST)
{}

sirius::library::net::sicp::create_session_req::~create_session_req(void)
{}

void sirius::library::net::sicp::create_session_req::execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	CMD_CREATE_SESSION_RES_T res;
	memset(&res, 0x00, sizeof(CMD_CREATE_SESSION_RES_T));

	if (session->register_flag())	//이미 Assoc이 되어 있으면 처리를 하지 않는다.
	{

		//strcpy_s(res.uuid, src);
		//session->uuid(res.uuid);
		res.code = CMD_ERR_CODE_SUCCESS;

		session->send(session->uuid(), uuid(), CMD_CREATE_SESSION_RESPONSE, reinterpret_cast<char*>(&res), sizeof(CMD_CREATE_SESSION_RES_T));
		return;
	}

	
	bool code = false;
	if (!strncmp(src, UNDEFINED_UUID, strlen(UNDEFINED_UUID))) //UUID값이 정의되지 않은 UUID일경우 새로 생성.
	{
		sirius::uuid  gen_uuid;
		gen_uuid.create();

		code = _processor->activate_session(gen_uuid.c_str(), session);
		if (code)
		{
			strcpy_s(res.uuid, gen_uuid.c_str());
			res.code = CMD_ERR_CODE_SUCCESS;
		}
		else
			res.code = CMD_ERR_CODE_FAIL;
	}
	else //UUID값이 정의된 경우
	{
		if (strlen(src)>0)
		{
			code = _processor->activate_session(src, session);
			if (code)
			{
				strcpy_s(res.uuid, src);
				res.code = CMD_ERR_CODE_SUCCESS;
			}
			else
				res.code = CMD_ERR_CODE_FAIL;
		}
		else
			res.code = CMD_ERR_CODE_FAIL;
	}
	session->send(session->uuid(), uuid(), CMD_CREATE_SESSION_RESPONSE, reinterpret_cast<char*>(&res), sizeof(CMD_CREATE_SESSION_RES_T));
	if (code)
		_processor->on_create_session(res.uuid, session);
}

#else
sirius::library::net::sicp::create_session_res::create_session_res(sirius::library::net::sicp::sicp_processor * prcsr)
	: abstract_command(prcsr, CMD_CREATE_SESSION_RESPONSE)
{}

sirius::library::net::sicp::create_session_res::~create_session_res(void)
{}

void sirius::library::net::sicp::create_session_res::execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	CMD_CREATE_SESSION_RES_T payload;
	memset(&payload, 0x00, sizeof(CMD_CREATE_SESSION_RES_T));
	memcpy(&payload, msg, sizeof(CMD_CREATE_SESSION_RES_T));

	if (payload.code == CMD_ERR_CODE_SUCCESS)
	{
		//session->assoc_flag(true);
		session->uuid(payload.uuid);
		_processor->uuid(payload.uuid);
		_processor->on_create_session(session);
	}
}
#endif

#if defined(WITH_WORKING_AS_SERVER)
sirius::library::net::sicp::destroy_session_noti::destroy_session_noti(sirius::library::net::sicp::sicp_processor * prcsr)
	: sirius::library::net::sicp::abstract_command(prcsr, CMD_DESTROY_SESSION_INDICATION)
{}

sirius::library::net::sicp::destroy_session_noti::~destroy_session_noti(void)
{}

void sirius::library::net::sicp::destroy_session_noti::execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	if (!session->register_flag())	//이미 Leave 되어 있으면 처리를 하지 않는다.
		return;
	session->register_flag(false);

	if (_processor && !session->is_session_destroy())
		_processor->on_destroy_session(session->uuid(), session);	
}
#else
sirius::library::net::sicp::destroy_session_noti::destroy_session_noti(sirius::library::net::sicp::sicp_processor * prcsr)
	: sirius::library::net::sicp::abstract_command(prcsr, CMD_DESTROY_SESSION_INDICATION)
{}

sirius::library::net::sicp::destroy_session_noti::~destroy_session_noti(void)
{}

void sirius::library::net::sicp::destroy_session_noti::execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	//_processor->enable_disconnect_flag(true);
}
#endif

sirius::library::net::sicp::keepalive_req::keepalive_req(sirius::library::net::sicp::sicp_processor * prcsr)
	: sirius::library::net::sicp::abstract_command(prcsr, CMD_KEEPALIVE_REQUEST)
{}

sirius::library::net::sicp::keepalive_req::~keepalive_req(void)
{}

void sirius::library::net::sicp::keepalive_req::execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	session->send(SERVER_UUID, uuid(), CMD_KEEPALIVE_RESPONSE, nullptr, 0);
}

sirius::library::net::sicp::keepalive_res::keepalive_res(sirius::library::net::sicp::sicp_processor * prcsr)
	: sirius::library::net::sicp::abstract_command(prcsr, CMD_KEEPALIVE_RESPONSE)
{}

sirius::library::net::sicp::keepalive_res::~keepalive_res(void)
{}

void sirius::library::net::sicp::keepalive_res::execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{}
