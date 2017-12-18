#include <sicp_abstract_client.h>
#include <sirius_locks.h>
#include <sicp_command.h>

sirius::library::net::sicp::abstract_client::abstract_client(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,int32_t command_thread_pool_count, bool use_keep_alive , bool dynamic_alloc, int32_t type, bool multicast)
	: sirius::library::net::client(mtu, so_rcvbuf_size, so_sndbuf_size,recv_buffer_size ,dynamic_alloc, type, multicast)
	, sirius::library::net::sicp::base(command_thread_pool_count)
	, _use_keep_alive(use_keep_alive)
{
	memcpy(_uuid, UNDEFINED_UUID, sizeof(_uuid));

	::InitializeCriticalSection(&_session_cs);
	::InitializeCriticalSection(&_commands_cs);

	sirius::library::net::sicp::base::initialize();

	add_command(new create_session_res(this));
	add_command(new destroy_session_noti(this));

	if (_use_keep_alive)
	{
		add_command(new keepalive_res(this));
	}
}

sirius::library::net::sicp::abstract_client::abstract_client(const char * uuid, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,int32_t command_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast)
	: sirius::library::net::client(mtu, so_rcvbuf_size, so_sndbuf_size,recv_buffer_size, dynamic_alloc, type, multicast)
	, sirius::library::net::sicp::base(command_thread_pool_count)
	, _use_keep_alive(use_keep_alive)
{
	strncpy_s(_uuid, uuid, sizeof(_uuid));

	::InitializeCriticalSection(&_session_cs);
	::InitializeCriticalSection(&_commands_cs);

	sirius::library::net::sicp::base::initialize();

	add_command(new create_session_res(this));
	add_command(new destroy_session_noti(this));

	if (_use_keep_alive)
	{
		add_command(new keepalive_res(this));
	}
}

sirius::library::net::sicp::abstract_client::~abstract_client(void)
{
	clear_command_list();
	sirius::library::net::sicp::base::release();

	::DeleteCriticalSection(&_commands_cs);
	::DeleteCriticalSection(&_session_cs);
}

bool sirius::library::net::sicp::abstract_client::connect(const char * address, int32_t port_number, int32_t io_thread_pool_count, bool reconnection)
{
	return sirius::library::net::client::connect(address, port_number, io_thread_pool_count, reconnection);
}

bool sirius::library::net::sicp::abstract_client::disconnect(void)
{
	return sirius::library::net::client::disconnect();
}

bool sirius::library::net::sicp::abstract_client::is_run(void) const
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

void sirius::library::net::sicp::abstract_client::data_indication_callback(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, size_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	std::map<int32_t, sirius::library::net::sicp::abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		if (session->assoc_flag() || (command_id == CMD_KEEPALIVE_REQUEST) || (command_id == CMD_KEEPALIVE_RESPONSE) || (command_id == CMD_CREATE_SESSION_RESPONSE))
		{
			abstract_command * command = (*iter).second;
			command->_execute(dst, src, command_id, version, msg, length, session);
		}
	}
}

void sirius::library::net::sicp::abstract_client::data_request(char * dst, int32_t command_id, char * msg, int32_t length)
{
	data_request(dst, _uuid, command_id, msg, length);
}

void sirius::library::net::sicp::abstract_client::data_request(char * dst, char * src, int32_t command_id, char * msg, int32_t length)
{
	std::shared_ptr<sirius::library::net::sicp::session> sicp_session;
	{
		sirius::autolock mutex(&_session_cs);
		sicp_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(_session);
	}

	if (sicp_session && sicp_session->assoc_flag())
		sicp_session->push_send_packet(dst, src, command_id, msg, length);
}

std::shared_ptr<sirius::library::net::session> sirius::library::net::sicp::abstract_client::create_session_callback(SOCKET client_socket, int32_t mtu,int32_t recv_buffer_size, bool dynamic_alloc)
{
	std::shared_ptr<sirius::library::net::sicp::session> session(new sirius::library::net::sicp::session(this, client_socket, mtu,recv_buffer_size, dynamic_alloc));
	return std::dynamic_pointer_cast<sirius::library::net::session>(session);
}

void sirius::library::net::sicp::abstract_client::destroy_session_callback(std::shared_ptr<sirius::library::net::session> session)
{
	std::shared_ptr<sirius::library::net::sicp::session> sicp_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session);
	destroy_session_completion_callback(sicp_session);
	
	{
		sirius::autolock mutex(&_session_cs);
		_session = nullptr;
	}

}

void sirius::library::net::sicp::abstract_client::add_command(sirius::library::net::sicp::abstract_command * command)
{
	if (command != nullptr)
	{
		if (command->get_processor() == nullptr)
			command->set_processor(this);

		{
			sirius::autolock mutex(&_commands_cs);
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
		sirius::autolock mutex(&_commands_cs);
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
	sirius::autolock mutex(&_commands_cs);
	std::map<int32_t, sirius::library::net::sicp::abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.erase(command_id);
}

void sirius::library::net::sicp::abstract_client::create_session_completion_callback(std::shared_ptr<sirius::library::net::sicp::session> session)
{
	if (!session->assoc_flag())
	{
		session->assoc_flag(true);
		create_session_callback();
	}
}

void sirius::library::net::sicp::abstract_client::destroy_session_completion_callback(std::shared_ptr<sirius::library::net::sicp::session> session)
{
	if (session->assoc_flag())
	{
		session->assoc_flag(false);
		destroy_session_callback();
	}
}

void sirius::library::net::sicp::abstract_client::clear_command_list(void)
{
	sirius::autolock mutex(&_commands_cs);
	std::map<int32_t, sirius::library::net::sicp::abstract_command*>::iterator iter;
	for (iter = _commands.begin(); iter != _commands.end(); iter++)
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.clear();
}

void sirius::library::net::sicp::abstract_client::process(void)
{
	const unsigned long msleep = 100;
	const unsigned long onesec = 1000;
	do
	{
		long long elapsed_millisec = 0;

		{
			sirius::autolock mutex(&_session_cs);
			_session = _client->connect(_address, _port_number, _io_thread_pool_count);
			if (_session)
				_run = true;
		}
		while (_run)
		{
			std::shared_ptr<sirius::library::net::sicp::session> sicp_session;
			{
				sirius::autolock mutex(&_session_cs);
				if (_session)
					sicp_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(_session);
			}
			
			if (sicp_session)
			{
				if ((!sicp_session->assoc_flag()) && ((elapsed_millisec % 3000) == 0))
					sicp_session->push_send_packet(SERVER_UUID, _uuid, CMD_CREATE_SESSION_REQUEST, nullptr, 0);

				if (_use_keep_alive && ((elapsed_millisec % (KEEPALIVE_INTERVAL - KEEPALIVE_INTERVAL_MARGIN)) == 0))
				{
					sicp_session->push_send_packet(SERVER_UUID, _uuid, CMD_KEEPALIVE_REQUEST, nullptr, 0);
				}

				::Sleep(msleep);
				elapsed_millisec += msleep;
			}
			else
			{
				break;
			}
		}

		std::shared_ptr<sirius::library::net::sicp::session> sicp_session;
		{
			sirius::autolock mutex(&_session_cs);
			if (_session)
				sicp_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(_session);
		}

		if (sicp_session)
		{
			if (sicp_session->assoc_flag())
			{
				sicp_session->push_send_packet(SERVER_UUID, _uuid, CMD_DESTROY_SESSION_INDICATION, nullptr, 0);
				destroy_session_completion_callback(sicp_session);
				sicp_session->close();

				wait_command_thread_end();
				enable_close_waiting_flag(true);
			}
			else
			{
				sicp_session->close();

				wait_command_thread_end();
				enable_close_waiting_flag(true);
			}
		}

		for (int32_t count = 0; _bwaiting; count++)
		{
			if (count == (onesec%msleep)) //force close when iocp client can not detect socket disconnect during 1 sec
				break;
			::Sleep(msleep);
		}

		_client->disconnect();

		{
			sirius::autolock mutex(&_session_cs);
			if (_session)
				_session = nullptr;
		}

		::Sleep(msleep);

	} while (_reconnection);
}