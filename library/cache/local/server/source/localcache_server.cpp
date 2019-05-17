#include "localcache_server.h"
#include <sirius_locks.h>
#include <localcache_command.h>
#include "localcache_commands_server.h"
#include <sirius_log4cplus_logger.h>
#ifdef NDEBUG
#include "malloc_extension.h"
#endif

sirius::library::cache::local::server::core::core(sirius::library::cache::local::server * front, int32_t max_sessions)
	: sirius::library::net::iocp::server(MTU_SIZE, MTU_SIZE, MTU_SIZE, MTU_SIZE, 0)
	, sirius::library::cache::local::base(0)
	, _max_sessions(max_sessions)
	, _front(front)
	, _storage_manager(NULL)
{
	sirius::library::net::iocp::server::initialize();
	sirius::library::cache::local::base::initialize();

	::InitializeCriticalSection(&_slock);
	_storage_manager = new sirius::library::cache::local::storage::manager;

	add_command(new upload_begin_req(this));
	add_command(new upload_processing_req(this));
	add_command(new upload_end_req(this));
	add_command(new download_req(this));
	add_command(new ftell_ind(this));
}

sirius::library::cache::local::server::core::~core(void)
{
	clean_command_list();
	if (_storage_manager)
	{
		delete _storage_manager;
		_storage_manager = NULL;
	}
	::DeleteCriticalSection(&_slock);

	sirius::library::cache::local::base::release();
	sirius::library::net::iocp::server::release();
}

int32_t sirius::library::cache::local::server::core::initialize(sirius::library::cache::local::server::context_t * context)
{
	_context = context;
	_storage_manager->initialize(context);
	return sirius::library::cache::local::server::core::err_code_t::success;
}

int32_t sirius::library::cache::local::server::core::release(void)
{
	_storage_manager->release();
	return sirius::library::cache::local::server::core::err_code_t::success;
}

int32_t sirius::library::cache::local::server::core::start(int32_t portnumber, int32_t threadpool)
{
	_storage_manager->start();
	return sirius::library::net::iocp::server::start("127.0.0.1", portnumber, threadpool);
}

int32_t sirius::library::cache::local::server::core::stop(void)
{
	int32_t status = sirius::library::net::iocp::server::stop();
	_storage_manager->stop();
	return status;
}

void sirius::library::cache::local::server::core::on_data_indication(int32_t command_id, const char * packet, int32_t packet_size, std::shared_ptr<sirius::library::cache::local::session> session)
{
	std::map<int32_t, abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		abstract_command * command = (*iter).second;
		command->_execute(command_id, packet, packet_size, session);
	}
}

void sirius::library::cache::local::server::core::add_command(sirius::library::cache::local::abstract_command * command)
{
	if (command != nullptr)
	{
		if (command->get_processor() == nullptr)
			command->set_processor(this);
		_commands.insert(std::make_pair(command->command_id(), command));
		_valid_command_ids.push_back(command->command_id());
	}
}

void sirius::library::cache::local::server::core::remove_command(int32_t command_id)
{
	std::map<int32_t, abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.erase(command_id);
}

void sirius::library::cache::local::server::core::clean_command_list(void)
{
	std::map<int32_t, sirius::library::cache::local::abstract_command*>::iterator iter;
	for (iter = _commands.begin(); iter != _commands.end(); iter++)
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.clear();
}

void sirius::library::cache::local::server::core::on_app_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session)
{

}

void sirius::library::cache::local::server::core::on_app_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	std::shared_ptr<sirius::library::cache::local::session> lc_session = std::dynamic_pointer_cast<sirius::library::cache::local::session>(session);
	{
		sirius::autolock lock(&_slock);
		_sessions.push_back(lc_session);
	}
}

void sirius::library::cache::local::server::core::on_app_session_close(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	std::shared_ptr<sirius::library::cache::local::session> siftp_session = NULL;
	{
		sirius::autolock lock(&_slock);
		siftp_session = std::dynamic_pointer_cast<sirius::library::cache::local::session>(session);
		std::vector<std::shared_ptr<sirius::library::cache::local::session>>::iterator iter = std::find(_sessions.begin(), _sessions.end(), siftp_session);
		if (iter != _sessions.end())
			_sessions.erase(iter);
	}
}

std::shared_ptr<sirius::library::net::iocp::session> sirius::library::cache::local::server::core::create_session(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls, SSL_CTX * ssl_ctx, BOOL reconnection)
{
	std::shared_ptr<sirius::library::cache::local::session> session(new sirius::library::cache::local::session(this, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls, ssl_ctx, reconnection));
	return std::dynamic_pointer_cast<sirius::library::net::iocp::session>(session);
}

void sirius::library::cache::local::server::core::destroy_session(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	std::shared_ptr<sirius::library::cache::local::session> lc_session = std::static_pointer_cast<sirius::library::cache::local::session>(session);

	//TODO
	{

	}
}

void sirius::library::cache::local::server::core::on_start(void)
{

}

void sirius::library::cache::local::server::core::on_stop(void)
{

}

void sirius::library::cache::local::server::core::on_running(void)
{
	const unsigned long msleep = 100;
	const unsigned long onesec = 1000;
	long long elapsed_millisec = 0;

	while (_run)
	{
		::Sleep(msleep);
		elapsed_millisec += msleep;
	}
}

int32_t sirius::library::cache::local::server::core::on_upload_begin(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, const char * packet, int32_t size)
{
	return _storage_manager->begin_uploading(sessionid, hash, fsize, width, height, packet, size);
}

int32_t sirius::library::cache::local::server::core::on_upload_processing(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, int32_t offset, const char * packet, int32_t size)
{
	return _storage_manager->processing_uploading(sessionid, hash, fsize, width, height, offset, packet, size);
}

int32_t sirius::library::cache::local::server::core::on_upload_end(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, int32_t offset, const char * packet, int32_t size)
{
	return _storage_manager->end_uploading(sessionid, hash, fsize, width, height, offset, packet, size);
}

void sirius::library::cache::local::server::core::on_download(const char * sessionid, const char * hash, std::shared_ptr<sirius::library::cache::local::session> session)
{
	_storage_manager->on_download(sessionid, hash, session);
}

void sirius::library::cache::local::server::core::on_ftell(const char * hash, int32_t fsize)
{
	_storage_manager->on_ftell(hash, fsize);
}