#include <localcache_client.h>
#include <sirius_uuid.h>
#include <sirius_locks.h>
#include <localcache_commands_client.h>
#include <localcache_command.h>
#ifdef NDEBUG
#include "malloc_extension.h"
#endif

#define MAXIMUM_IMAGE_SIZE 256 * 180 * 4
#define MTU_SIZE 35 * 1024

sirius::library::cache::local::client::core::core(void)
	: sirius::library::net::iocp::client(MTU_SIZE, MTU_SIZE, MTU_SIZE, MTU_SIZE, FALSE)
	, sirius::library::cache::local::base(3)
	, _session(nullptr)
{
	::InitializeCriticalSection(&_slock);
	::InitializeCriticalSection(&_ulock);
	::InitializeCriticalSection(&_dlock);

	sirius::library::net::iocp::client::initialize();
	sirius::library::cache::local::base::initialize();

	add_command(new upload_res(this));
	add_command(new download_begin_res(this));
	add_command(new download_processing_res(this));
	add_command(new download_end_res(this));
}

sirius::library::cache::local::client::core::~core(void)
{
	clear_command_list();

	sirius::library::cache::local::base::release();
	sirius::library::net::iocp::client::release();

	::DeleteCriticalSection(&_dlock);
	::DeleteCriticalSection(&_ulock);
	::DeleteCriticalSection(&_slock);
}

int32_t sirius::library::cache::local::client::core::initialize(void)
{


	return sirius::library::cache::local::client::core::err_code_t::success;
}

int32_t sirius::library::cache::local::client::core::release(void)
{


	return sirius::library::cache::local::client::core::err_code_t::success;
}

int32_t sirius::library::cache::local::client::core::connect(const char * address, int32_t portnumber, int32_t io_thread_pool_count, BOOL reconnection)
{
	return sirius::library::net::iocp::client::connect(address, portnumber, io_thread_pool_count, reconnection);
}

int32_t sirius::library::cache::local::client::core::disconnect(void)
{
	return sirius::library::net::iocp::client::disconnect();
}

void sirius::library::cache::local::client::core::disconnect(BOOL enable)
{
	sirius::library::net::iocp::client::disconnect(enable);
}

BOOL sirius::library::cache::local::client::core::active(void) const
{
	return _run;
}

int32_t sirius::library::cache::local::client::core::upload(const char * hash, const char * image, int32_t size, int32_t width, int32_t height)
{
	int32_t status = sirius::library::cache::local::client::core::err_code_t::unknown;
	sirius::library::cache::local::client::core::uplink_state_t * state = NULL;
	std::shared_ptr<sirius::library::cache::local::session> lc_session;
	do
	{
		if (!_session)
		{
			status = sirius::library::cache::local::client::core::err_code_t::fail;
			break;
		}
		lc_session = _session;


		int32_t nsndbuff = lc_session->send_buffer_size();
		char * sndbuff = static_cast<char*>(malloc(nsndbuff));
		if (sndbuff)
		{
			sirius::uuid uuidgen;
			uuidgen.create();
			const char * sessionid = uuidgen.c_str();
			std::shared_ptr<sirius::library::cache::local::client::core::uplink_state_t> state = std::shared_ptr<sirius::library::cache::local::client::core::uplink_state_t>(new sirius::library::cache::local::client::core::uplink_state_t);
			if (!state)
			{
				status = sirius::library::cache::local::client::core::err_code_t::memory_alloc_error;
				break;
			}
			
			{
				sirius::autolock lock(&_ulock);
				_uplink_states.insert(std::make_pair(sessionid, state));
			}

			int32_t offset = 0;
			int32_t ntarget = size;
			for (int32_t index = 0; TRUE; index++)
			{
				if (index == 0)
				{
					sirius::library::cache::local::cmd_upload_begin_req_t begin;
					memset(&begin, 0x00, sizeof(begin));
					strncpy_s(begin.sessionid, sessionid, sizeof(begin.sessionid));
					memmove(begin.hash, hash, 128);
					begin.size = size;
					begin.width = width;
					begin.height = height;

					int32_t nsend = nsndbuff - sizeof(begin);
					if (nsend >= ntarget)
					{
						memmove(sndbuff, &begin, sizeof(begin));
						memmove(sndbuff + sizeof(begin), image + offset, ntarget);
						lc_session->send(CMD_UPLOAD_BEGIN_REQ, const_cast<const char*>(sndbuff), sizeof(begin) + ntarget);
						offset += ntarget;

						sirius::library::cache::local::cmd_upload_end_req_t end;
						memset(&end, 0x00, sizeof(end));
						strncpy_s(end.sessionid, sessionid, sizeof(end.sessionid));
						memmove(end.hash, hash, 128);
						end.size = size;
						end.width = width;
						end.height = height;
						end.offset = offset;
						memmove(sndbuff, &end, sizeof(end));
						lc_session->send(CMD_UPLOAD_END_REQ, const_cast<const char*>(sndbuff), sizeof(end));

						break;
					}
					else
					{
						memmove(sndbuff, &begin, sizeof(begin));
						memmove(sndbuff + sizeof(begin), image + offset, nsend);

						lc_session->send(CMD_UPLOAD_BEGIN_REQ, const_cast<const char*>(sndbuff), nsndbuff);
						offset += nsend;
						ntarget -= nsend;
					}
				}
				else
				{
					if (state->finalized)
					{
						status = state->code;
						break;
					}

					sirius::library::cache::local::cmd_upload_processing_req_t proc;
					int32_t nsend = nsndbuff - sizeof(proc);
					if (nsend >= ntarget)
					{
						sirius::library::cache::local::cmd_upload_end_req_t end;
						memset(&end, 0x00, sizeof(end));
						strncpy_s(end.sessionid, sessionid, sizeof(end.sessionid));
						memmove(end.hash, hash, 128);
						end.size = size;
						end.width = width;
						end.height = height;
						end.offset = offset;

						memmove(sndbuff, &end, sizeof(end));
						memmove(sndbuff + sizeof(end), image + offset, ntarget);
						offset += ntarget;

						lc_session->send(CMD_UPLOAD_END_REQ, const_cast<const char*>(sndbuff), sizeof(end) + ntarget);

						break;
					}
					else
					{
						memset(&proc, 0x00, sizeof(proc));
						strncpy_s(proc.sessionid, sessionid, sizeof(proc.sessionid));
						memmove(proc.hash, hash, 128);
						proc.size = size;
						proc.width = width;
						proc.height = height;
						proc.offset = offset;
						memmove(sndbuff, &proc, sizeof(proc));
						memmove(sndbuff + sizeof(proc), image + offset, nsend);

						lc_session->send(CMD_UPLOAD_PROCESSING_REQ, const_cast<const char*>(sndbuff), nsndbuff);
						offset += nsend;
						ntarget -= nsend;
					}
				}
			}

			for (int32_t index = 0; index < 200; index++) //waiting 1 sec
			{
				if (state->finalized)
				{
					status = state->code;
					break;
				}
				::Sleep(1);
			}

			if (status == sirius::library::cache::local::client::core::err_code_t::unknown)
				status = sirius::library::cache::local::client::core::err_code_t::timeout;

			{
				sirius::autolock lock(&_ulock);
				std::map<std::string, std::shared_ptr<sirius::library::cache::local::client::core::uplink_state_t>>::iterator iter = _uplink_states.find(sessionid);
				if (iter != _uplink_states.end())
					_uplink_states.erase(iter);
			}

			state = NULL;
			free(sndbuff);
			sndbuff = NULL;
		}

	} while (0);

	return status;
}

int32_t sirius::library::cache::local::client::core::download(const char * hash, char * image, int32_t capacity, int32_t & size)
{
	int32_t status = sirius::library::cache::local::client::core::err_code_t::unknown;
	sirius::library::cache::local::client::core::downlink_state_t * state = NULL;
	std::shared_ptr<sirius::library::cache::local::session> lc_session;
	do
	{
		if (!_session)
		{
			status = sirius::library::cache::local::client::core::err_code_t::fail;
			break;
		}
		lc_session = _session;

		sirius::uuid uuidgen;
		uuidgen.create();
		const char * sessionid = uuidgen.c_str();
		std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t> state = std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t>(new sirius::library::cache::local::client::core::downlink_state_t);
		if (!state)
		{
			status = sirius::library::cache::local::client::core::err_code_t::memory_alloc_error;
			break;
		}

		memmove(state->hash, hash, 128);

		{
			sirius::autolock lock(&_dlock);
			_downlink_states.insert(std::make_pair(sessionid, state));
		}


		sirius::library::cache::local::cmd_download_req_t req;
		strncpy_s(req.sessionid, sessionid, sizeof(req.sessionid));
		memmove(req.hash, hash, 128);

		/*
		char debug[MAX_PATH] = { 0 };
		_snprintf_s(debug, MAX_PATH, "download_req : %s\n", req.sessionid);
		::OutputDebugStringA(debug);
		*/

		lc_session->send(CMD_DOWNLOAD_REQ, reinterpret_cast<const char*>(&req), sizeof(req));


		if (::WaitForSingleObject(state->finalized, 200) == WAIT_OBJECT_0)
		{
			sirius::shared_scopedlock lock(&state->lock);
			status = state->code;
		}

		if (status == sirius::library::cache::local::client::err_code_t::success)
		{
			sirius::shared_scopedlock lock(&state->lock);
			size = capacity > state->nbuffer ? state->nbuffer : capacity;
			memmove(image, state->buffer, size);
			
			/*
			char debug[MAX_PATH] = { 0 };
			_snprintf_s(debug, MAX_PATH, "download sucess: capacity[%d], size[%d]\n", capacity, size);
			::OutputDebugStringA(debug);
			*/
		}
		else if (status == sirius::library::cache::local::client::err_code_t::unknown)
		{
			status = sirius::library::cache::local::client::err_code_t::cached_image_read_timeout;
			/*
			char debug[MAX_PATH] = { 0 };
			_snprintf_s(debug, MAX_PATH, "download timed out\n");
			::OutputDebugStringA(debug);
			*/
		}

		{
			sirius::autolock lock(&_dlock);
			std::map<std::string, std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t>>::iterator iter = _downlink_states.find(sessionid);
			if (iter != _downlink_states.end())
				_downlink_states.erase(iter);
		}

		state = NULL;

	} while (0);

	return status;
}

int32_t sirius::library::cache::local::client::core::ftell(const char * hash, int32_t fsize)
{
	int32_t status = sirius::library::cache::local::client::core::err_code_t::unknown;
	std::shared_ptr<sirius::library::cache::local::session> lc_session;
	do
	{
		if (!_session)
		{
			status = sirius::library::cache::local::client::core::err_code_t::fail;
			break;
		}
		lc_session = _session;

		sirius::library::cache::local::cmd_ftell_ind_t ind;
		memmove(ind.hash, hash, 128);
		ind.size = fsize;

		lc_session->send(CMD_FTELL_IND, reinterpret_cast<const char*>(&ind), sizeof(ind));
		status = sirius::library::cache::local::client::core::err_code_t::success;

	} while (0);

	return status;
}

void sirius::library::cache::local::client::core::on_upload(const char * sessionid, int32_t code)
{
	std::shared_ptr<sirius::library::cache::local::client::core::uplink_state_t> state;
	{
		sirius::autolock lock(&_ulock);
		std::map<std::string, std::shared_ptr<sirius::library::cache::local::client::core::uplink_state_t>>::iterator iter = _uplink_states.find(sessionid);
		if (iter != _uplink_states.end())
		{
			state = iter->second;
			_uplink_states.erase(iter);
		}
	}

	if (state)
	{
		sirius::autolock lock(&state->lock);
		state->code = code;
		state->finalized = TRUE;
		
		switch (code)
		{
			case sirius::library::cache::local::client::err_code_t::uploading_write_already_exist :
				::OutputDebugStringA("uploading_write_already_exist \n");
				break;
			case sirius::library::cache::local::client::err_code_t::uploading_segment_write_fail :
				::OutputDebugStringA("uploading_segment_write_fail \n");
				break;
			case sirius::library::cache::local::client::err_code_t::uploading_segment_read_fail :
				::OutputDebugStringA("uploading_segment_read_fail \n");
				break;
			case sirius::library::cache::local::client::err_code_t::uploading_state_not_exist :
				::OutputDebugStringA("uploading_state_not_exist \n");
				break;
			case sirius::library::cache::local::client::err_code_t::uploading_hash_no_equal :
				::OutputDebugStringA("uploading_hash_no_equal \n");
				break;
			case sirius::library::cache::local::client::err_code_t::uploading_no_segment :
				::OutputDebugStringA("uploading_no_segment \n");
				break;
			case sirius::library::cache::local::client::err_code_t::uploading_memalloc_fail :
				::OutputDebugStringA("uploading_memalloc_fail \n");
				break;
			case sirius::library::cache::local::client::err_code_t::uploading_segment_not_exist :
				::OutputDebugStringA("uploading_segment_not_exist \n");
				break;
			case sirius::library::cache::local::client::err_code_t::uploading_temp_file_creation_fail :
				::OutputDebugStringA("uploading_temp_file_creation_fail \n");
				break;
		}
	}
}

void sirius::library::cache::local::client::core::on_download_begin(const char * sessionid, const char * hash, int32_t size, const char * packet, int32_t npacket)
{
	std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t> state;
	{
		sirius::autolock lock(&_dlock);
		std::map<std::string, std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t>>::iterator iter = _downlink_states.find(sessionid);
		if (iter != _downlink_states.end())
		{
			state = iter->second;
		}
	}

	/*
	char debug[MAX_PATH] = { 0 };
	_snprintf_s(debug, MAX_PATH, "on_download_begin : %s[%d]\n", hash, size);
	::OutputDebugStringA(debug);
	*/

	if (state)
	{
		if (size > 0)
		{
			sirius::exclusive_scopedlock lock(&state->lock);
			if (!state->buffer)
			{
				state->nbuffer = size;
				state->buffer = static_cast<char*>(malloc(state->nbuffer));
			}
		}

		if (packet && (npacket > 0))
		{
			sirius::shared_scopedlock lock(&state->lock);
			memmove(state->buffer, packet, npacket);
			state->nprocessed += npacket;
			/*
			char debug[MAX_PATH] = { 0 };
			_snprintf_s(debug, MAX_PATH, "on_download_begin : hash[%s], size[%d], npacket[%d]\n", hash, size, npacket);
			::OutputDebugStringA(debug);
			*/
		}
	}
}

void sirius::library::cache::local::client::core::on_download_processing(const char * sessionid, int32_t size, int32_t offset, const char * packet, int32_t npacket)
{
	std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t> state;
	{
		sirius::autolock lock(&_dlock);
		std::map<std::string, std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t>>::iterator iter = _downlink_states.find(sessionid);
		if (iter != _downlink_states.end())
		{
			state = iter->second;
		}
	}

	/*
	char debug[MAX_PATH] = { 0 };
	_snprintf_s(debug, MAX_PATH, "on_download_processing : %s\n", sessionid);
	::OutputDebugStringA(debug);
	*/

	if (state)
	{
		if (size > 0)
		{
			sirius::exclusive_scopedlock lock(&state->lock);
			if (!state->buffer)
			{
				state->nbuffer = size;
				state->buffer = static_cast<char*>(malloc(state->nbuffer));
			}
		}

		if (packet && (npacket > 0))
		{
			sirius::shared_scopedlock lock(&state->lock);
			memmove(state->buffer + offset, packet, npacket);
			state->nprocessed += npacket;
		}
	}
}

void sirius::library::cache::local::client::core::on_download_end(const char * sessionid, int32_t code, int32_t size, int32_t offset, const char * packet, int32_t npacket)
{
	std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t> state;
	{
		sirius::autolock lock(&_dlock);
		std::map<std::string, std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t>>::iterator iter = _downlink_states.find(sessionid);
		if (iter != _downlink_states.end())
		{
			state = iter->second;
			_downlink_states.erase(iter);
		}
	}

	/*
	char debug[MAX_PATH] = { 0 };
	_snprintf_s(debug, MAX_PATH, "on_download_end : %s\n", sessionid);
	::OutputDebugStringA(debug);
	*/

	if (state)
	{
		if (code != sirius::library::cache::local::client::err_code_t::success)
		{
			sirius::exclusive_scopedlock lock(&state->lock);
			state->code = code;
			::SetEvent(state->finalized);
			return;
		}

		if (size > 0)
		{
			sirius::exclusive_scopedlock lock(&state->lock);
			if (!state->buffer)
			{
				state->nbuffer = size;
				state->buffer = static_cast<char*>(malloc(state->nbuffer));
			}
		}

		if (packet && (npacket > 0))
		{
			sirius::exclusive_scopedlock lock(&state->lock);

			memmove(state->buffer + offset, packet, npacket);
			state->nprocessed += npacket;
		}

		while (TRUE)
		{
			{
				sirius::shared_scopedlock lock(&state->lock);
				if (state->nbuffer == state->nprocessed)
					break;
			}
			::Sleep(1);
		}

		{
			sirius::exclusive_scopedlock lock(&state->lock);
			state->code = code;
			::SetEvent(state->finalized);
		}
	}
}

void sirius::library::cache::local::client::core::on_data_indication(int32_t command_id, const char * packet, int32_t packet_size, std::shared_ptr<sirius::library::cache::local::session> session)
{
	std::map<int32_t, sirius::library::cache::local::abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		sirius::library::cache::local::abstract_command * command = (*iter).second;
		command->_execute(command_id, packet, packet_size, session);
	}
}

void sirius::library::cache::local::client::core::send(int32_t command_id, const char * packet, int32_t packet_size)
{
	std::shared_ptr<sirius::library::cache::local::session> lc_session;
	{
		sirius::autolock mutex(&_slock);
		lc_session = std::dynamic_pointer_cast<sirius::library::cache::local::session>(_session);
	}

	if(lc_session)
		lc_session->send(command_id, packet, packet_size);
}
std::shared_ptr<sirius::library::net::iocp::session> sirius::library::cache::local::client::core::create_session(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls, SSL_CTX * ssl_ctx, BOOL reconnection)
{
	std::shared_ptr<sirius::library::cache::local::session> session(new sirius::library::cache::local::session(this, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls, ssl_ctx, reconnection));
	return std::dynamic_pointer_cast<sirius::library::net::iocp::session>(session);
}

void sirius::library::cache::local::client::core::destroy_session(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	std::shared_ptr<sirius::library::cache::local::session> lc_session = std::dynamic_pointer_cast<sirius::library::cache::local::session>(session);

	//TODO
	/*
	{
		sirius::autolock mutex(&_slock);
		_session = nullptr;
	}
	*/
}

void sirius::library::cache::local::client::core::add_command(sirius::library::cache::local::abstract_command * command)
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

void sirius::library::cache::local::client::core::wait_command_thread_end(void)
{
	std::map<int32_t, sirius::library::cache::local::abstract_command*>::iterator iter;
	BOOL running = TRUE;
	while (running)
	{
		running = FALSE;
		for (iter = _commands.begin(); iter != _commands.end(); iter++)
		{
			sirius::library::cache::local::abstract_command * command = (*iter).second;
			if (command->is_running())
			{
				::Sleep(5);
				running = TRUE;
				break;
			}
		}
	}
}

void sirius::library::cache::local::client::core::remove_command(int32_t command_id)
{
	std::map<int32_t, sirius::library::cache::local::abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		sirius::library::cache::local::abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.erase(command_id);
}

void sirius::library::cache::local::client::core::clear_command_list(void)
{
	std::map<int32_t, sirius::library::cache::local::abstract_command*>::iterator iter;
	for (iter = _commands.begin(); iter != _commands.end(); iter++)
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.clear();
}

void sirius::library::cache::local::client::core::on_app_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	/*
	sirius::autolock lock(&_slock);
	if (!_session || ((_session->status() & sirius::library::net::iocp::session::status_t::closed) == sirius::library::net::iocp::session::status_t::closed))
	{
		_session = std::dynamic_pointer_cast<sirius::library::net::sicp::session>(session);
	}
	*/
}

void sirius::library::cache::local::client::core::on_app_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	sirius::autolock lock(&_slock);
	if (!_session || ((_session->status() & sirius::library::net::iocp::session::status_t::closed)== sirius::library::net::iocp::session::status_t::closed))
	{
		_session = std::dynamic_pointer_cast<sirius::library::cache::local::session>(session);
	}
}

void sirius::library::cache::local::client::core::on_app_session_close(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	sirius::autolock lock(&_slock);
	if (_session)
	{
		_session->close();
		//_session = nullptr;
	}
}

void sirius::library::cache::local::client::core::on_start(void)
{

}

void sirius::library::cache::local::client::core::on_running(void)
{
	const unsigned long msleep = 100;
	const unsigned long onesec = 1000;
	long long elapsed_millisec = 0;

	while (_run)
	{
		/*
		std::shared_ptr<sirius::library::cache::local::session> lc_session = nullptr;
		{
			sirius::autolock mutex(&_slock);
			lc_session = _session;
		}
		if (lc_session && _on_connected)
		{

		}
		else if(_on_disconnected)
		{
			break;
		}
		*/
		::Sleep(msleep);
		elapsed_millisec += msleep;
	}
}

void sirius::library::cache::local::client::core::on_stop(void)
{
	std::shared_ptr<sirius::library::cache::local::session> lc_session = nullptr;

	{
		sirius::autolock mutex(&_slock);
		if (_session)
			lc_session = _session;
	}

	if (lc_session)
	{
		/*
		if (lc_session->register_flag())
		{
			lc_session->send(SERVER_UUID, _uuid, CMD_DESTROY_SESSION_INDICATION, NULL, 0);
			on_destroy_session(lc_session);
		}
		*/

		lc_session->close();
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
