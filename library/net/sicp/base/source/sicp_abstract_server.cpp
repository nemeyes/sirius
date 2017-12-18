#include <sicp_abstract_server.h>
#include <sirius_locks.h>
#include <sicp_command.h>

sirius::library::net::sicp::abstract_server::abstract_server(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,const char * uuid, int32_t command_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast)
	: sirius::library::net::server(mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc, type, multicast)
	, sirius::library::net::sicp::base(command_thread_pool_count)
	, _use_keep_alive(use_keep_alive)
	, _sequence(0)
{
	::InitializeCriticalSection(&_assoc_sessions_cs);
	::InitializeCriticalSection(&_closed_sessions_cs);
	strncpy_s(_uuid, uuid, strlen(uuid)+1);

	sirius::library::net::sicp::base::initialize();

	add_command(new create_session_req(this));
	add_command(new destroy_session_noti(this));

	if (_use_keep_alive)
	{
		add_command(new keepalive_req(this));
	}
}

sirius::library::net::sicp::abstract_server::~abstract_server(void)
{
	clear_command_list();
	sirius::library::net::sicp::base::release();
	::DeleteCriticalSection(&_closed_sessions_cs);
	::DeleteCriticalSection(&_assoc_sessions_cs);
}

const char * sirius::library::net::sicp::abstract_server::uuid(void)
{
	return _uuid;
}

void sirius::library::net::sicp::abstract_server::uuid(const char * uuid)
{
	strncpy_s(_uuid, uuid, sizeof(uuid));
}

bool sirius::library::net::sicp::abstract_server::check_alive_session(const char * uuid)
{
	sirius::autolock lock(&_assoc_sessions_cs);
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
	iter = _assoc_sessions.find(uuid);
	if (iter == _assoc_sessions.end())
		return false;

	std::shared_ptr<sirius::library::net::sicp::session> ses = iter->second;
	if (ses->fd() == INVALID_SOCKET)
		return false;

	return true;
}

std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> sirius::library::net::sicp::abstract_server::get_assoc_clients(void)
{
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> sessions;
	{
		sirius::autolock lock(&_assoc_sessions_cs);
		sessions = _assoc_sessions;
	}
	return sessions;
}

void sirius::library::net::sicp::abstract_server::data_indication_callback(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, size_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	if (!strncmp(src, SERVER_UUID, strlen(SERVER_UUID)) || !strncmp(src, uuid(), strlen(uuid()))) //src의 주소가 SERVER_UUID이거나 서버자체의 할당된 UUID 일경우, 잘못된 src이므로 패킷을 버린다.
		return;

	std::map<int32_t, abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		//		ABS_OUTPUT_DBG("data_indication_callback : find command \n");
		if (session->assoc_flag() || (command_id == CMD_KEEPALIVE_REQUEST) || (command_id == CMD_KEEPALIVE_RESPONSE) || (command_id == CMD_CREATE_SESSION_REQUEST))
		{
			abstract_command * command = (*iter).second;
			command->_execute(dst, src, command_id, version, msg, length, session);
		}
	}
	else
	{
		std::shared_ptr<sirius::library::net::sicp::session> session = nullptr;
		{
			std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;

			sirius::autolock lock(&_assoc_sessions_cs);
			iter = _assoc_sessions.find(dst);
			if (iter != _assoc_sessions.end())
				session = iter->second;
		}
		if (session)
		{
			session->push_send_packet(dst, src, command_id, (char*)msg, length);
		}
	}
}

void sirius::library::net::sicp::abstract_server::data_request(char * dst, int32_t command_id, char * msg, size_t length)
{
	data_request(dst, _uuid, command_id, msg, length);
}

void sirius::library::net::sicp::abstract_server::data_request(char * dst, char * src, int32_t command_id, char * msg, size_t length)
{
	if (!strncmp(BROADCAST_UUID, dst, strlen(BROADCAST_UUID)))
	{
		std::vector< std::shared_ptr<sirius::library::net::sicp::session>> sessions;
		std::vector< std::shared_ptr<sirius::library::net::sicp::session>>::iterator send_iter;
		{
			std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
			sirius::autolock lock(&_assoc_sessions_cs);
			for (iter = _assoc_sessions.begin(); iter != _assoc_sessions.end(); iter++)
			{
				std::shared_ptr<sirius::library::net::sicp::session> session = iter->second;
				sessions.push_back(session);
			}
		}

		for (send_iter = sessions.begin(); send_iter != sessions.end(); send_iter++)
		{
			std::shared_ptr<sirius::library::net::sicp::session> session = (*send_iter);
			char src_uuid[64] = { 0 };
			if (!strncmp(_uuid, dst, sizeof(_uuid) - 1) || !strncmp(SERVER_UUID, dst, sizeof(_uuid) - 1))
				strncpy_s(src_uuid, _uuid, sizeof(src_uuid) - 1);
			else
				strncpy_s(src_uuid, dst, sizeof(src_uuid) - 1);
			session->push_send_packet(_uuid, src_uuid, command_id, msg, length);
		}
		sessions.clear();
	}
	else
	{
		std::shared_ptr<sirius::library::net::sicp::session> session = nullptr;
		{
			std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
			sirius::autolock lock(&_assoc_sessions_cs);
			iter = _assoc_sessions.find(dst);
			if (iter != _assoc_sessions.end())
				session = iter->second;
		}

		if (session)
		{
			char src_uuid[64] = { 0 };
			if (!strncmp(_uuid, dst, sizeof(_uuid) - 1) || !strncmp(SERVER_UUID, dst, sizeof(_uuid) - 1))
				strncpy_s(src_uuid, _uuid, sizeof(src_uuid) - 1);
			else
				strncpy_s(src_uuid, dst, sizeof(src_uuid) - 1);
			session->push_send_packet(_uuid, src_uuid, command_id, msg, length);
		}
	}
}

std::shared_ptr<sirius::library::net::session> sirius::library::net::sicp::abstract_server::create_session_callback(SOCKET client_socket, int32_t mtu,int32_t recv_buffer_size, bool dynamic_alloc)
{
	std::shared_ptr<sirius::library::net::sicp::session> session(new sirius::library::net::sicp::session(this, client_socket, mtu,recv_buffer_size, dynamic_alloc));
	return std::dynamic_pointer_cast<sirius::library::net::session>(session);
}

void sirius::library::net::sicp::abstract_server::destroy_session_callback(std::shared_ptr<sirius::library::net::session> session)
{
	std::shared_ptr<sirius::library::net::sicp::session> sicp_session = std::static_pointer_cast<sirius::library::net::sicp::session>(session);
	destroy_session_completion_callback(sicp_session->uuid(), sicp_session);
	unregister_assoc_client(std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session));
}

int32_t sirius::library::net::sicp::abstract_server::clean_conn_session(bool force_clean)
{
	std::vector<std::shared_ptr<sirius::library::net::session>> finalized_sessions;
	std::vector<std::shared_ptr<sirius::library::net::session>> connected_sessions;

	{
		sirius::autolock lock(&_conn_sessions_cs);
		if (_conn_sessions.size() > 0)
		{
			connected_sessions = _conn_sessions;
			_conn_sessions.clear();
		}
	}

	std::vector<std::shared_ptr<sirius::library::net::session>>::iterator iter;
	uint64_t now = ::GetTickCount64();
	iter = connected_sessions.begin();
	while (iter != connected_sessions.end())
	{
		std::shared_ptr<sirius::library::net::sicp::session> ses = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(*iter);
		int64_t interval = now - ses->get_last_access_tm();
		if (interval > KEEPALIVE_INTERVAL)
		{
			if (ses->connected_flag() || force_clean)
			{
				ses->connected_flag(false);
				ses->close();
			}
		}
		else
		{
			finalized_sessions.push_back(ses);
		}
		++iter;
	}

	{
		sirius::autolock lock(&_conn_sessions_cs);
		if (finalized_sessions.size() > 0)
		{
			_conn_sessions = finalized_sessions;
		}
	}

	return _conn_sessions.size();
}

int32_t sirius::library::net::sicp::abstract_server::clean_assoc_session(bool force_clean)
{
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> finalized_sessions;
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> associated_sessions;
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> closed_sessions;


	{
		sirius::autolock lock(&_assoc_sessions_cs);
		if (_assoc_sessions.size() > 0)
		{
			associated_sessions = _assoc_sessions;
			_assoc_sessions.clear();
		}
	}


	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
	uint64_t now = ::GetTickCount64();
	iter = associated_sessions.begin();
	while (iter != associated_sessions.end())
	{
		std::shared_ptr<sirius::library::net::sicp::session> ses = iter->second;
		uint64_t interval = now - ses->get_last_access_tm();
		if (interval > KEEPALIVE_INTERVAL)
		{
			if ((ses->fd() != INVALID_SOCKET) || force_clean)
			{
				if (ses->assoc_flag())
				{
					destroy_session_completion_callback(ses->uuid(), ses);
					ses->assoc_flag(false);
				}
				ses->close();
			}
			_closed_sessions.insert(std::make_pair(iter->first, iter->second));
		}
		else
		{
			finalized_sessions.insert(std::make_pair(iter->first, iter->second));
		}
		++iter;
	}

	{
		sirius::autolock lock(&_assoc_sessions_cs);
		if (finalized_sessions.size() > 0)
		{
			_assoc_sessions = finalized_sessions;
		}
	}

	{
		sirius::autolock lock(&_closed_sessions_cs);
		if (_closed_sessions.size() > 0)
		{
			closed_sessions = _closed_sessions;
			_closed_sessions.clear();
		}
	}

	finalized_sessions.clear();
	now = ::GetTickCount64();
	iter = closed_sessions.begin();
	while (iter != closed_sessions.end())
	{
		std::shared_ptr<sirius::library::net::sicp::session> ses = iter->second;
		uint64_t interval = now - ses->get_last_access_tm();
		if (interval <= DESTROY_INTERVAL)
		{
			finalized_sessions.insert(std::make_pair(iter->first, iter->second));
		}
		++iter;
	}

	{
		sirius::autolock lock(&_closed_sessions_cs);
		if (finalized_sessions.size() > 0)
		{
			_closed_sessions = finalized_sessions;
		}
	}

	return _assoc_sessions.size();
}

bool sirius::library::net::sicp::abstract_server::register_assoc_client(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	sirius::autolock lock(&_assoc_sessions_cs);

	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter = _assoc_sessions.find(uuid);
	if (iter != _assoc_sessions.end())
	{
		std::shared_ptr<sirius::library::net::sicp::session> temp = (*iter).second;
		_server->close(temp);
	}

	if (session->fd() != INVALID_SOCKET)
	{
		_assoc_sessions.insert(std::make_pair(uuid, session));
		session->uuid(uuid);
		session->assoc_flag(true);
		return true;
	}
	else
		return false;
}

bool sirius::library::net::sicp::abstract_server::unregister_assoc_client(std::shared_ptr<sirius::library::net::sicp::session> session)
{
	sirius::autolock lock(&_assoc_sessions_cs);

	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
	for (iter = _assoc_sessions.begin(); iter != _assoc_sessions.end(); iter++)
	{
		if (session.get() == (*iter).second.get())
			break;
	}
	if (iter != _assoc_sessions.end())
	{
		std::shared_ptr<sirius::library::net::sicp::session> ses = iter->second;
		_assoc_sessions.erase(iter);
		ses->assoc_flag(false);
		return true;
	}
	return false;
}

void sirius::library::net::sicp::abstract_server::add_command(sirius::library::net::sicp::abstract_command * command)
{
	if (command != nullptr)
	{
		if (command->get_processor() == nullptr)
			command->set_processor(this);
		_commands.insert(std::make_pair(command->command_id(), command));
		_valid_command_ids.push_back(command->command_id());
	}
}

void sirius::library::net::sicp::abstract_server::remove_command(int32_t command_id)
{
	std::map<int32_t, abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.erase(command_id);
}

void sirius::library::net::sicp::abstract_server::create_session_completion_callback(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	create_session_callback(uuid);
}

void sirius::library::net::sicp::abstract_server::destroy_session_completion_callback(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	destroy_session_callback(uuid);
}

void sirius::library::net::sicp::abstract_server::clear_command_list(void)
{
	std::map<int32_t, sirius::library::net::sicp::abstract_command*>::iterator iter;
	for (iter = _commands.begin(); iter != _commands.end(); iter++)
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.clear();
}

void sirius::library::net::sicp::abstract_server::process(void)
{
	const unsigned long msleep = 100;
	const unsigned long onesec = 1000;
	long long elapsed_millisec = 0;

	_server->start(_address, _port_number, _io_thread_pool_count);
	while (_run)
	{
		clean_conn_session();

		if (_use_keep_alive)
		{
			clean_conn_session();
		}

		::Sleep(msleep);
		elapsed_millisec += msleep;
	}

	clean_assoc_session(true);
	clean_conn_session(true);

	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> sessions = get_assoc_clients();
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
	for (iter = sessions.begin(); iter != sessions.end(); iter++)
	{
		std::shared_ptr<sirius::library::net::sicp::session> session = (*iter).second;
		if (session)
		{
			session->push_send_packet(session->uuid(), _uuid, CMD_DESTROY_SESSION_INDICATION, nullptr, 0);
		}
	}
	_server->stop();
}