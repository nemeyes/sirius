#include <sicp_abstract_server.h>
#include <sirius_locks.h>
#include <sicp_command.h>
#include <sirius_log4cplus_logger.h>
#ifdef NDEBUG
#include "malloc_extension.h"
#endif

sirius::library::net::sicp::abstract_server::abstract_server(const char * uuid, int32_t command_thread_pool_count, BOOL keepalive, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls)
	: sirius::library::net::iocp::server(so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls)
	, sirius::library::net::sicp::base(command_thread_pool_count)
	, _keepalive(keepalive)
	, _sequence(0)
{
	::InitializeCriticalSection(&_handshaking_slock);
	::InitializeCriticalSection(&_connected_slock);
	::InitializeCriticalSection(&_active_slock);
	::InitializeCriticalSection(&_closing_slock);

	strncpy_s(_uuid, uuid, strlen(uuid)+1);

	sirius::library::net::sicp::base::initialize();

	add_command(new create_session_req(this));
	add_command(new destroy_session_noti(this));

	if (_keepalive)
	{
		add_command(new keepalive_req(this));
	}
}

sirius::library::net::sicp::abstract_server::~abstract_server(void)
{
	clean_command_list();
	sirius::library::net::sicp::base::release();


	::DeleteCriticalSection(&_closing_slock);
	::DeleteCriticalSection(&_active_slock);
	::DeleteCriticalSection(&_connected_slock);
	::DeleteCriticalSection(&_handshaking_slock);
}

int32_t sirius::library::net::sicp::abstract_server::initialize(void)
{
	sirius::library::net::iocp::server::initialize();
	sirius::library::net::sicp::base::initialize();

	return sirius::library::net::sicp::abstract_server::err_code_t::success;
}

int32_t sirius::library::net::sicp::abstract_server::release(void)
{
	sirius::library::net::sicp::base::release();
	sirius::library::net::iocp::server::release();

	return sirius::library::net::sicp::abstract_server::err_code_t::success;
}

const char * sirius::library::net::sicp::abstract_server::uuid(void)
{
	return _uuid;
}

void sirius::library::net::sicp::abstract_server::uuid(const char * uuid)
{
	strncpy_s(_uuid, uuid, sizeof(uuid));
}

std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> sirius::library::net::sicp::abstract_server::activated_sessions(void)
{
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> sessions;
	{
		sirius::autolock lock(&_active_slock);
		sessions = _activated_sessions;
	}
	return sessions;
}

void sirius::library::net::sicp::abstract_server::on_data_indication(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * packet, int32_t packet_size, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	if (!strncmp(src, SERVER_UUID, strlen(SERVER_UUID)) || !strncmp(src, uuid(), strlen(uuid()))) //src의 주소가 SERVER_UUID이거나 서버자체의 할당된 UUID 일경우, 잘못된 src이므로 패킷을 버린다.
		return;

	std::map<int32_t, abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		if (session->register_flag() || (command_id == CMD_KEEPALIVE_REQUEST) || (command_id == CMD_KEEPALIVE_RESPONSE) || (command_id == CMD_CREATE_SESSION_REQUEST))
		{
			abstract_command * command = (*iter).second;
			command->_execute(dst, src, command_id, version, packet, packet_size, session);
		}
	}
	else
	{
		std::shared_ptr<sirius::library::net::sicp::session> session = nullptr;
		{
			std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;

			sirius::autolock lock(&_active_slock);
			iter = _activated_sessions.find(dst);
			if (iter != _activated_sessions.end())
				session = iter->second;
		}
		if (session)
		{
			session->send(dst, src, command_id, packet, packet_size);
			LOGGER::make_info_log(SAA, "%s, %d on_data_indication, command_id=%d, dst=%s, src=%s", __FUNCTION__, __LINE__, command_id, dst, src);
		}
	}
}

void sirius::library::net::sicp::abstract_server::data_request(const char * dst, int32_t command_id, const char * packet, int32_t packet_size)
{
	data_request(dst, _uuid, command_id, packet, packet_size);
}

void sirius::library::net::sicp::abstract_server::data_request(const char * dst, const char * src, int32_t command_id, const char * packet, int32_t packet_size)
{
	LOGGER::make_info_log(SAA, "%s, %d data_request. command_id=%d, dst=%s, src=%s", __FUNCTION__, __LINE__, command_id, dst, src);
	if (!strncmp(BROADCAST_UUID, dst, strlen(BROADCAST_UUID)))
	{
		std::vector< std::shared_ptr<sirius::library::net::sicp::session>> sessions;
		std::vector< std::shared_ptr<sirius::library::net::sicp::session>>::iterator send_iter;
		{
			std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
			sirius::autolock lock(&_active_slock);
			for (iter = _activated_sessions.begin(); iter != _activated_sessions.end(); iter++)
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
			session->send(_uuid, src_uuid, command_id, packet, packet_size);
		}
		sessions.clear();
	}
	else
	{
		std::shared_ptr<sirius::library::net::sicp::session> session = nullptr;
		{
			std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
			sirius::autolock lock(&_active_slock);
			iter = _activated_sessions.find(dst);
			if (iter != _activated_sessions.end())
				session = iter->second;
		}

		if (session)
		{
			char src_uuid[64] = { 0 };
			if (!strncmp(_uuid, dst, sizeof(_uuid) - 1) || !strncmp(SERVER_UUID, dst, sizeof(_uuid) - 1))
			{
				//strncpy_s(src_uuid, _uuid, sizeof(src_uuid) - 1);
			}
			else
			{
				//strncpy_s(src_uuid, dst, sizeof(src_uuid) - 1);
				//session->send(_uuid, src_uuid, command_id, packet, packet_size);
				session->send(dst, _uuid, command_id, packet, packet_size);
			}
		}
	}
}

int32_t sirius::library::net::sicp::abstract_server::clean_handshaking_session(BOOL force_clean)
{
	std::vector<std::shared_ptr<sirius::library::net::sicp::session>> finalized_sessions;
	std::vector<std::shared_ptr<sirius::library::net::sicp::session>> handshaking_sessions;

	{
		sirius::autolock lock(&_handshaking_slock);
		if (_handshaking_sessions.size() > 0)
		{
			handshaking_sessions = _handshaking_sessions;
			_handshaking_sessions.clear();
		}
	}

	std::vector<std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
	uint64_t now = ::GetTickCount64();
	iter = handshaking_sessions.begin();
	while (iter != handshaking_sessions.end())
	{
		std::shared_ptr<sirius::library::net::sicp::session> session = *iter;
		int64_t interval = now - session->timestamp();
		if ((interval > MAXIUM_REGISTING_SESSION_WAITING_INTERVAL) || force_clean)
		{
			session->close();
			sirius::autolock lock(&_closing_slock);
			session->update_timestamp();
			_closing_sessions.push_back(session);
		}
		else
		{
			if (session->socket() == NULL && session->socket() == INVALID_SOCKET)
			{
				sirius::autolock lock(&_closing_slock);
				session->update_timestamp();
				_closing_sessions.push_back(session);
			}
			else
			{
				finalized_sessions.push_back(session);
			}
		}
		++iter;
	}

	{
		sirius::autolock lock(&_handshaking_slock);
		for (iter = finalized_sessions.begin(); iter != finalized_sessions.end(); iter++)
			_handshaking_sessions.push_back(*iter);
	}

	return _handshaking_sessions.size();
}

int32_t sirius::library::net::sicp::abstract_server::clean_connected_session(BOOL force_clean)
{
	std::vector<std::shared_ptr<sirius::library::net::sicp::session>> finalized_sessions;
	std::vector<std::shared_ptr<sirius::library::net::sicp::session>> connected_sessions;

	{
		sirius::autolock lock(&_connected_slock);
		if (_connected_sessions.size() > 0)
		{
			connected_sessions = _connected_sessions;
			_connected_sessions.clear();
		}
	}

	std::vector<std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
	uint64_t now = ::GetTickCount64();
	iter = connected_sessions.begin();
	while (iter != connected_sessions.end())
	{
		std::shared_ptr<sirius::library::net::sicp::session> session = *iter;
		int64_t interval = now - session->timestamp();
		if ((interval > MAXIUM_REGISTING_SESSION_WAITING_INTERVAL) || force_clean)
		{
			session->close();
			sirius::autolock lock(&_closing_slock);
			session->update_timestamp();
			_closing_sessions.push_back(session);

			if(!force_clean)
				sirius::library::log::log4cplus::logger::make_debug_log(SAA, "connected session is not activated during waiting interval, connected session is closed and moved to closing list");
			else
				sirius::library::log::log4cplus::logger::make_debug_log(SAA, "connected session is force to close and move to closing list\n");
		}
		else
		{
			if (session->socket() == NULL && session->socket() == INVALID_SOCKET)
			{
				sirius::autolock lock(&_closing_slock);
				session->update_timestamp();
				_closing_sessions.push_back(session);
				sirius::library::log::log4cplus::logger::make_debug_log(SAA, "connected session is already closed and moved to closing list\n");
			}
			else
			{
				finalized_sessions.push_back(session);
			}
		}
		++iter;
	}

	{
		sirius::autolock lock(&_connected_slock);
		for (iter = finalized_sessions.begin(); iter != finalized_sessions.end(); iter++)
			_connected_sessions.push_back(*iter);
	}

	return _connected_sessions.size();
}

int32_t sirius::library::net::sicp::abstract_server::clean_activated_session(BOOL force_clean)
{
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> finalized_sessions;
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> activated_sessions;

	{
		sirius::autolock lock(&_active_slock);
		if (_activated_sessions.size() > 0)
		{
			activated_sessions = _activated_sessions;
			_activated_sessions.clear();
		}
	}

	uint64_t now = ::GetTickCount64();
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator activated_session_iter = activated_sessions.begin();
	while (activated_session_iter != activated_sessions.end())
	{
		std::string uuid = activated_session_iter->first;
		std::shared_ptr<sirius::library::net::sicp::session> session = activated_session_iter->second;
		uint64_t interval = now - session->timestamp();
		if (interval > KEEPALIVE_INTERVAL || force_clean)
		{
			if (session->socket() != NULL && session->socket() != INVALID_SOCKET)
			{
				if (session->register_flag())
				{
					on_destroy_session(session->uuid(), session);
					session->register_flag(false);
				}
				session->close();
				if (!force_clean)
					sirius::library::log::log4cplus::logger::make_debug_log(SAA, "activated session doesn't recv/send any data during keepalive interval, activated session is closed and moved to closing list\n");
				else
					sirius::library::log::log4cplus::logger::make_debug_log(SAA, "activated session is force to close and move to closing list\n");
			}
			
			{
				sirius::autolock lock(&_closing_slock);
				session->update_timestamp();
				_closing_sessions.push_back(session);
			}
		}
		else
		{
			if ((session->socket() == NULL) || (session->socket() == INVALID_SOCKET))
			{
				sirius::autolock lock(&_closing_slock);
				session->update_timestamp();
				_closing_sessions.push_back(session);
				sirius::library::log::log4cplus::logger::make_debug_log(SAA, "activated session is already closed and moved to closing list\n");
			}
			else
			{
				finalized_sessions.insert(std::make_pair(uuid, session));
			}
		}
		++activated_session_iter;
	}

	{
		sirius::autolock lock(&_active_slock);
		if (finalized_sessions.size() > 0)
		{
			_activated_sessions = finalized_sessions;
		}
	}

	return _activated_sessions.size();
}

int32_t sirius::library::net::sicp::abstract_server::clean_closing_session(BOOL force_clean)
{

	if (force_clean)
	{
		sirius::autolock lock(&_closing_slock);
		_closing_sessions.clear();
	}
	else
	{
		uint64_t now = ::GetTickCount64();
		std::vector<std::shared_ptr<sirius::library::net::sicp::session>> final_sessions;
		std::vector<std::shared_ptr<sirius::library::net::sicp::session>> closing_sessions;
		{
			sirius::autolock lock(&_closing_slock);
			if (_closing_sessions.size() > 0)
			{
				closing_sessions = _closing_sessions;
				_closing_sessions.clear();
			}
		}

		std::vector<std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter = closing_sessions.begin();
		while (iter != closing_sessions.end())
		{
			std::shared_ptr<sirius::library::net::sicp::session> session = *iter;
			uint64_t interval = now - session->timestamp();
			if (interval > MAXIUM_CLOSING_SESSION_WAITING_INTERVAL || force_clean)
			{
				if (!force_clean)
				{
					sirius::library::log::log4cplus::logger::make_debug_log(SAA, "closing session is removed from memory after waiting interval\n");
				}
				else
				{
					sirius::library::log::log4cplus::logger::make_debug_log(SAA, "closing session is force to be removed from memory\n");
				}
			}
			else
			{
				final_sessions.push_back(session);
			}
			++iter;
		}

		{
			sirius::autolock lock(&_closing_slock);
			for (iter = final_sessions.begin(); iter != final_sessions.end(); iter++)
				_closing_sessions.push_back(*iter);
		}
		//_closing_sessions.clear();
		//final_sessions.clear();
		//closing_sessions.clear();
#ifdef NDEBUG
		MallocExtension::instance()->ReleaseFreeMemory();
#endif
	}

	return _activated_sessions.size();
}

bool sirius::library::net::sicp::abstract_server::activate_session(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	//std::shared_ptr<sirius::library::net::iocp::session> iocp_session = std::dynamic_pointer_cast<sirius::library::net::iocp::session>(session);
	{
		sirius::autolock lock(&_active_slock);
		std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter = _activated_sessions.find(uuid);
		if (iter != _activated_sessions.end())
		{
			std::shared_ptr<sirius::library::net::sicp::session> temp = (*iter).second;
			temp->close();
		}
	}

	{
		sirius::autolock lock(&_connected_slock);
		std::vector<std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
		iter = std::find(_connected_sessions.begin(), _connected_sessions.end(), session);
		if (iter != _connected_sessions.end())
			_connected_sessions.erase(iter);
	}

	{
		sirius::autolock lock(&_active_slock);
		if (session->socket() != NULL && session->socket() != INVALID_SOCKET)
		{
			session->update_timestamp();
			_activated_sessions.insert(std::make_pair(uuid, session));
			session->uuid(uuid);
			session->register_flag(true);
			return true;
		}
		else
			return false;
	}
}

bool sirius::library::net::sicp::abstract_server::deactivate_session(std::shared_ptr<sirius::library::net::sicp::session> session)
{
	{
		sirius::autolock lock(&_active_slock);
		std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
		for (iter = _activated_sessions.begin(); iter != _activated_sessions.end(); iter++)
		{
			if (session.get() == (*iter).second.get())
				break;
		}
		if (iter != _activated_sessions.end())
		{
			std::shared_ptr<sirius::library::net::sicp::session> session = iter->second;
			_activated_sessions.erase(iter);
			session->register_flag(false);
			return true;
		}
	}

	{
		sirius::autolock lock(&_closing_slock);
		session->update_timestamp();
		_closing_sessions.push_back(session);

	}
	return false;
}

bool sirius::library::net::sicp::abstract_server::check_activate_session(const char * uuid)
{
	sirius::autolock lock(&_active_slock);

	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
	iter = _activated_sessions.find(uuid);
	if (iter == _activated_sessions.end())
		return false;

	std::shared_ptr<sirius::library::net::sicp::session> session = iter->second;
	if (session->socket() == INVALID_SOCKET)
		return false;

	return true;
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

void sirius::library::net::sicp::abstract_server::on_create_session(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	on_create_session(uuid);
}

void sirius::library::net::sicp::abstract_server::on_destroy_session(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	session->increase_session_destroy_count();
	on_destroy_session(uuid);
}

void sirius::library::net::sicp::abstract_server::clean_command_list(void)
{
	std::map<int32_t, sirius::library::net::sicp::abstract_command*>::iterator iter;
	for (iter = _commands.begin(); iter != _commands.end(); iter++)
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.clear();
}

void sirius::library::net::sicp::abstract_server::on_app_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	sirius::autolock lock(&_handshaking_slock);
	_handshaking_sessions.push_back(std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session));
}

void sirius::library::net::sicp::abstract_server::on_app_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	std::shared_ptr<sirius::library::net::sicp::session> sicp_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session);
	{
		sirius::autolock lock(&_handshaking_slock);
		std::vector<std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter = std::find(_handshaking_sessions.begin(), _handshaking_sessions.end(), sicp_session);
		if (iter != _handshaking_sessions.end())
			_handshaking_sessions.erase(iter);
	}

	{
		sirius::autolock lock(&_connected_slock);
		session->update_timestamp();
		_connected_sessions.push_back(std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session));
		sirius::library::log::log4cplus::logger::make_debug_log(SAA, "on_app_session_connect\n");
	}
}

void sirius::library::net::sicp::abstract_server::on_app_session_close(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	std::shared_ptr<sirius::library::net::sicp::session> sicp_session = nullptr;
	{
		sirius::autolock lock(&_connected_slock);
		sicp_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session);
		std::vector<std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter = std::find(_connected_sessions.begin(), _connected_sessions.end(), sicp_session);
		if (iter != _connected_sessions.end())
		{
			_connected_sessions.erase(iter);
			sirius::library::log::log4cplus::logger::make_debug_log(SAA, "on_app_session_close : removed from connected session list\n");
		}
	}

	if (sicp_session)
	{
		sirius::autolock lock(&_closing_slock);
		sicp_session->update_timestamp();
		_closing_sessions.push_back(sicp_session);
		sirius::library::log::log4cplus::logger::make_debug_log(SAA, "on_app_session_close : added to closing session list\n");
	}
}

std::shared_ptr<sirius::library::net::iocp::session> sirius::library::net::sicp::abstract_server::create_session(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls, SSL_CTX * ssl_ctx, BOOL reconnection)
{
	std::shared_ptr<sirius::library::net::sicp::session> session(new sirius::library::net::sicp::session(this, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls, ssl_ctx, reconnection));
	return std::dynamic_pointer_cast<sirius::library::net::iocp::session>(session);
}

void sirius::library::net::sicp::abstract_server::destroy_session(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	std::shared_ptr<sirius::library::net::sicp::session> sicp_session = std::static_pointer_cast<sirius::library::net::sicp::session>(session);
	on_destroy_session(sicp_session->uuid(), sicp_session);
	deactivate_session(std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session));
}

void sirius::library::net::sicp::abstract_server::on_start(void)
{

}

void sirius::library::net::sicp::abstract_server::on_stop(void)
{
	clean_handshaking_session(TRUE);
	clean_connected_session(TRUE);
	clean_activated_session(TRUE);
	clean_closing_session(TRUE);
}

void sirius::library::net::sicp::abstract_server::on_running(void)
{
	const unsigned long msleep = 100;
	const unsigned long onesec = 1000;
	long long elapsed_millisec = 0;

	while (_run)
	{
		if (elapsed_millisec%onesec == 0)
		{
			if(_tls)
				clean_handshaking_session(FALSE);
			clean_connected_session(FALSE);
			clean_closing_session(FALSE);
		}

		if (_keepalive)
			clean_activated_session(FALSE);

		::Sleep(msleep);
		elapsed_millisec += msleep;
		//if (elapsed_millisec % onesec == 0)
		//	::sirius::library::log::log4cplus::logger::make_debug_log(SAA, "onesec elapsed\n");
	}

	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> sessions = activated_sessions();
	std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>::iterator iter;
	for (iter = sessions.begin(); iter != sessions.end(); iter++)
	{
		std::shared_ptr<sirius::library::net::sicp::session> session = (*iter).second;
		if (session)
		{
			session->send(session->uuid(), _uuid, CMD_DESTROY_SESSION_INDICATION, NULL, 0);
		}
	}
}