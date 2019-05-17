#ifndef _LOCALCACHE_CLIENT_H_
#define _LOCALCACHE_CLIENT_H_

#include <iocp_session.h>
#include <iocp_client.h>
#include <localcache_base.h>
#include <localcache_session.h>
#include <localcache_command.h>

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				class client::core
					: public sirius::library::net::iocp::client
					, public sirius::library::cache::local::base
				{
					friend class sirius::library::cache::local::session;
				public:
					typedef struct _uplink_state_t
					{
						BOOL	finalized;
						int32_t code;
						CRITICAL_SECTION lock;
						_uplink_state_t(void)
							: finalized(FALSE)
							, code(sirius::library::cache::local::client::err_code_t::unknown)
						{
							::InitializeCriticalSection(&lock);
						}

						~_uplink_state_t(void)
						{
							::DeleteCriticalSection(&lock);
						}
					} uplink_state_t;

					typedef struct _downlink_state_t
					{
						char	hash[MAX_PATH];
						HANDLE	finalized;
						int32_t code;
						char *	buffer;
						int32_t nbuffer;
						int32_t nprocessed;
						SRWLOCK lock;
						_downlink_state_t(void)
							: finalized(INVALID_HANDLE_VALUE)
							, code(sirius::library::cache::local::client::err_code_t::unknown)
							, buffer(NULL)
							, nbuffer(0)
							, nprocessed(0)
						{
							::memset(hash, 0x00, MAX_PATH);
							finalized = ::CreateEventA(NULL, FALSE, FALSE, NULL);
							::InitializeSRWLock(&lock);
						}

						~_downlink_state_t(void)
						{
							if (buffer)
							{
								free(buffer);
								buffer = NULL;
							}
							nbuffer = 0;
							if (finalized != INVALID_HANDLE_VALUE)
							{
								::CloseHandle(finalized);
								finalized = INVALID_HANDLE_VALUE;
							}
									
						}
					} downlink_state_t;

					core(void);
					virtual ~core(void);

					int32_t			initialize(void);
					int32_t			release(void);

					int32_t			connect(const char * address, int32_t portnumber, int32_t io_thread_pool_count, BOOL reconnection = TRUE);
					int32_t			disconnect(void);
					void			disconnect(BOOL enable);
					BOOL			active(void) const;

					int32_t			upload(const char * hash, const char * image, int32_t size, int32_t width, int32_t height);
					int32_t			download(const char * hash, char * image, int32_t capacity, int32_t & size);
					int32_t			ftell(const char * hash, int32_t fsize);

					void			on_upload(const char * sessionid, int32_t code);
					void			on_download_begin(const char * sessionid, const char * hash, int32_t size, const char * packet, int32_t npacket);
					void			on_download_processing(const char * sessionid, int32_t size, int32_t offset, const char * packet, int32_t npacket);
					void			on_download_end(const char * sessionid, int32_t code, int32_t size, int32_t offset, const char * packet, int32_t npacket);

				private:
					void			on_data_indication(int32_t command_id, const char * packet, int32_t packet_size, std::shared_ptr<sirius::library::cache::local::session> session);
					void			send(int32_t command_id, const char * packet, int32_t packet_size);

					void			add_command(abstract_command * command);
					void			wait_command_thread_end(void);
					void			remove_command(int32_t command_id);

					virtual std::shared_ptr<sirius::library::net::iocp::session> create_session(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls = FALSE, SSL_CTX * ssl_ctx = NULL, BOOL reconnection = FALSE);
					virtual void	destroy_session(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_start(void);
					virtual void	on_stop(void);
					virtual void	on_running(void);

					void			clear_command_list(void);
							
					virtual void	on_app_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_app_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_app_session_close(std::shared_ptr<sirius::library::net::iocp::session> session);

				protected:
					CRITICAL_SECTION _slock;
					std::map<int32_t, sirius::library::cache::local::abstract_command*> _commands;

					CRITICAL_SECTION _ulock;
					std::map<std::string, std::shared_ptr<sirius::library::cache::local::client::core::uplink_state_t>> _uplink_states;

					CRITICAL_SECTION _dlock;
					std::map<std::string, std::shared_ptr<sirius::library::cache::local::client::core::downlink_state_t>> _downlink_states;

					std::shared_ptr<sirius::library::cache::local::session> _session;
				};
			};
		}
	};
};

#endif
