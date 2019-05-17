#ifndef _LOCALCACHE_SERVER_H_
#define _LOCALCACHE_SERVER_H_

#include "sirius_localcache_server.h"

#include <iocp_session.h>
#include <iocp_server.h>
#include <localcache_base.h>
#include <localcache_session.h>
#include <localcache_command.h>
#include <storage_manager.h>

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				class server::core
					: public sirius::library::net::iocp::server
					, public sirius::library::cache::local::base
				{
					friend class sirius::library::cache::local::session;
				public:
					core(sirius::library::cache::local::server * front, int32_t max_sessions = 1000);
					virtual ~core(void);

					int32_t			initialize(sirius::library::cache::local::server::context_t * context);
					int32_t			release(void);

					int32_t			start(int32_t portnumber, int32_t threadpool);
					int32_t			stop(void);

					int32_t			on_upload_begin(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, const char * packet, int32_t size);
					int32_t			on_upload_processing(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, int32_t offset, const char * packet, int32_t size);
					int32_t			on_upload_end(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, int32_t offset, const char * packet, int32_t size);

					void			on_download(const char * sessionid, const char * hash, std::shared_ptr<sirius::library::cache::local::session> session);
					void			on_ftell(const char * hash, int32_t fsize);

				private:
					virtual std::shared_ptr<sirius::library::net::iocp::session>	create_session(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls, SSL_CTX * ssl_ctx, BOOL reconnection=FALSE);
					virtual void	destroy_session(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_start(void);
					virtual void	on_stop(void);
					virtual void	on_running(void);

					void			on_data_indication(int32_t command_id, const char * packet, int32_t packet_size, std::shared_ptr<sirius::library::cache::local::session> session);
					void			add_command(sirius::library::cache::local::abstract_command * command);
					void			remove_command(int32_t command_id);

					void			clean_command_list(void);

					virtual void	on_app_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_app_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_app_session_close(std::shared_ptr<sirius::library::net::iocp::session> session);

				protected:
					sirius::library::cache::local::server * _front;
					sirius::library::cache::local::server::context_t * _context;
					CRITICAL_SECTION _slock;
					std::vector<std::shared_ptr<sirius::library::cache::local::session>> _sessions;
					std::map<int32_t, sirius::library::cache::local::abstract_command*> _commands;
					int32_t _max_sessions;

					sirius::library::cache::local::storage::manager * _storage_manager;

				private:
					core(sirius::library::cache::local::server::core & clone);
				};
			};
		};
	};
};

#endif