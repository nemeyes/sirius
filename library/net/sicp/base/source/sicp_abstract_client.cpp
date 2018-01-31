#include <sicp_abstract_client.h>
#include <sirius_locks.h>
#include <sicp_command.h>

sirius::library::net::sicp::abstract_client::abstract_client(int32_t command_thread_pool_count, BOOL keepalive, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls)
	: sirius::library::net::iocp::client(so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls)
	, sirius::library::net::sicp::base(command_thread_pool_count)
	, _keepalive(keepalive)
	, _session(nullptr)
{
	memcpy(_uuid, UNDEFINED_UUID, sizeof(_uuid));

	::InitializeCriticalSection(&_slock);

	sirius::library::net::sicp::base::initialize();

	add_command(new create_session_res(this));
	add_command(new destroy_session_noti(this));

	if (_keepalive)
	{
		add_command(new keepalive_res(this));
	}
}

sirius::library::net::sicp::abstract_client::abstract_client(const char * uuid, int32_t command_thread_pool_count, BOOL keepalive, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls)
	: sirius::library::net::iocp::client(so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls)
	, sirius::library::net::sicp::base(command_thread_pool_count)
	, _keepalive(keepalive)
{
	strncpy_s(_uuid, uuid, sizeof(_uuid));

	::InitializeCriticalSection(&_slock);

	sirius::library::net::sicp::base::initialize();

	add_command(new create_session_res(this));
	add_command(new destroy_session_noti(this));

	if (_keepalive)
	{
		add_command(new keepalive_res(this));
	}
}

sirius::library::net::sicp::abstract_client::~abstract_client(void)
{
	clear_command_list();
	sirius::library::net::sicp::base::release();

	::DeleteCriticalSection(&_slock);
}

int32_t sirius::library::net::sicp::abstract_client::initialize(void)
{
	sirius::library::net::iocp::client::initialize();
	sirius::library::net::sicp::base::initialize();

	return sirius::library::net::sicp::abstract_client::err_code_t::success;
}

int32_t sirius::library::net::sicp::abstract_client::release(void)
{
	sirius::library::net::sicp::base::release();
	sirius::library::net::iocp::client::release();

	return sirius::library::net::sicp::abstract_client::err_code_t::success;
}

int32_t sirius::library::net::sicp::abstract_client::connect(const char * address, int32_t portnumber, int32_t io_thread_pool_count, BOOL reconnection)
{
	return sirius::library::net::iocp::client::connect(address, portnumber, io_thread_pool_count, reconnection);
}

int32_t sirius::library::net::sicp::abstract_client::disconnect(void)
{
	return sirius::library::net::iocp::client::disconnect();
}

void sirius::library::net::sicp::abstract_client::disconnect(BOOL enable)
{
	sirius::library::net::iocp::client::disconnect(enable);
}

BOOL sirius::library::net::sicp::abstract_client::active(void) const
{
	return _run;
}

const char * sirius::library::net::sicp::abstract_client::uuid(void)
{
	return _uuid;
}

void sirius::library::net::sicp::abstract_client::uuid(const char * uuid)
{
	strcpy_s(_uuid, uuid);
}

void sirius::library::net::sicp::abstract_client::on_data_indication(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * packet, int32_t packet_size, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	std::map<int32_t, sirius::library::net::sicp::abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		if (session->register_flag() || (command_id == CMD_KEEPALIVE_REQUEST) || (command_id == CMD_KEEPALIVE_RESPONSE) || (command_id == CMD_CREATE_SESSION_RESPONSE))
		{
			abstract_command * command = (*iter).second;
			command->_execute(dst, src, command_id, version, packet, packet_size, session);
		}
	}
}

void sirius::library::net::sicp::abstract_client::data_request(const char * dst, int32_t command_id, const char * packet, int32_t packet_size)
{
	data_request(dst, _uuid, command_id, packet, packet_size);
}

void sirius::library::net::sicp::abstract_client::data_request(const char * dst, const char * src, int32_t command_id, const char * packet, int32_t packet_size)
{
	std::shared_ptr<sirius::library::net::sicp::session> sicp_session;
	{
		sirius::autolock mutex(&_slock);
		sicp_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(_session);
	}

	if (sicp_session && sicp_session->register_flag())
		sicp_session->send(dst, src, command_id, packet, packet_size);
}

std::shared_ptr<sirius::library::net::iocp::session> sirius::library::net::sicp::abstract_client::create_session(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls, SSL_CTX * ssl_ctx, BOOL reconnection)
{
	std::shared_ptr<sirius::library::net::sicp::session> session(new sirius::library::net::sicp::session(this, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls, ssl_ctx, reconnection));
	return std::dynamic_pointer_cast<sirius::library::net::iocp::session>(session);
}

void sirius::library::net::sicp::abstract_client::destroy_session(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	std::shared_ptr<sirius::library::net::sicp::session> sicp_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session);
	on_destroy_session(sicp_session);

	/*
	{
		sirius::autolock mutex(&_slock);
		_session = nullptr;
	}
	*/
}

void sirius::library::net::sicp::abstract_client::add_command(sirius::library::net::sicp::abstract_command * command)
{
	if (command != nullptr)
	{
		if (command->get_processor() == nullptr)
			command->set_processor(this);

		{
			_commands.insert(std::make_pair(command->command_id(), command));
			_valid_command_ids.push_back(command->command_id());
		}
	}
}

void sirius::library::net::sicp::abstract_client::wait_command_thread_end(void)
{
	std::map<int32_t, sirius::library::net::sicp::abstract_command*>::iterator iter;
	bool running = true;
	while (running)
	{
		running = false;
		for (iter = _commands.begin(); iter != _commands.end(); iter++)
		{
			abstract_command * command = (*iter).second;
			if (command->is_running())
			{
				::Sleep(5);
				running = true;
				break;
			}
		}
	}
}

void sirius::library::net::sicp::abstract_client::remove_command(int32_t command_id)
{
	std::map<int32_t, sirius::library::net::sicp::abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.erase(command_id);
}

void sirius::library::net::sicp::abstract_client::on_create_session(std::shared_ptr<sirius::library::net::sicp::session> session)
{
	if (!session->register_flag())
	{
		session->register_flag(true);
		on_create_session();
	}
}

void sirius::library::net::sicp::abstract_client::on_destroy_session(std::shared_ptr<sirius::library::net::sicp::session> session)
{
	if (session->register_flag())
	{
		session->register_flag(false);
		on_destroy_session();
	}
}

void sirius::library::net::sicp::abstract_client::clear_command_list(void)
{
	std::map<int32_t, sirius::library::net::sicp::abstract_command*>::iterator iter;
	for (iter = _commands.begin(); iter != _commands.end(); iter++)
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.clear();
}

void sirius::library::net::sicp::abstract_client::on_app_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	sirius::autolock lock(&_slock);
	_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session);
}

void sirius::library::net::sicp::abstract_client::on_app_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	sirius::autolock lock(&_slock);
	if(!_session)
		_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session);
	_session->send(SERVER_UUID, _uuid, CMD_CREATE_SESSION_REQUEST, NULL, 0);
}

void sirius::library::net::sicp::abstract_client::on_app_session_close(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	sirius::autolock lock(&_slock);
	if (_session)
	{
		_session->close();
		//_session = nullptr;
	}
}

void sirius::library::net::sicp::abstract_client::on_start(void)
{

}

void sirius::library::net::sicp::abstract_client::on_running(void)
{
	const unsigned long msleep = 100;
	const unsigned long onesec = 1000;
	long long elapsed_millisec = 0;

	while (_run)
	{
		std::shared_ptr<sirius::library::net::sicp::session> sicp_session = nullptr;
		{
			sirius::autolock mutex(&_slock);
			sicp_session = _session;
		}

		if (sicp_session && _on_connected)
		{
			if ((!sicp_session->register_flag()) && ((elapsed_millisec % 3000) == 0))
				sicp_session->send(SERVER_UUID, _uuid, CMD_CREATE_SESSION_REQUEST, NULL, 0);

			if (_keepalive && ((elapsed_millisec % (KEEPALIVE_INTERVAL - KEEPALIVE_INTERVAL_MARGIN)) == 0))
			{
				sicp_session->send(SERVER_UUID, _uuid, CMD_KEEPALIVE_REQUEST, NULL, 0);
			}
		}
		else if(_on_disconnected)
		{
			break;
		}
		::Sleep(msleep);
		elapsed_millisec += msleep;
	}
}

void sirius::library::net::sicp::abstract_client::on_stop(void)
{
	std::shared_ptr<sirius::library::net::sicp::session> sicp_session = nullptr;

	{
		sirius::autolock mutex(&_slock);
		if (_session)
			sicp_session = _session;
	}

	if (sicp_session)
	{
		if (sicp_session->register_flag())
		{
			sicp_session->send(SERVER_UUID, _uuid, CMD_DESTROY_SESSION_INDICATION, NULL, 0);
			on_destroy_session(sicp_session);
		}

		sicp_session->close();
		wait_command_thread_end();
		close_waiting_flag(TRUE);
	}
	else
	{
		wait_command_thread_end();
		close_waiting_flag(TRUE);
	}

	const unsigned long msleep = 100;
	const unsigned long onesec = 1000;
	for (int32_t count = 0; _waiting; count++)
	{
		if (count == (onesec%msleep)) //force close when iocp client can not detect socket disconnect during 1 sec
			break;
		::Sleep(msleep);
	}

	{
		sirius::autolock mutex(&_slock);
		if (_session)
			_session = nullptr;
	}
}