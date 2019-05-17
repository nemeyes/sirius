#ifndef _LOCALCACHE_SESSION_H_
#define _LOCALCACHE_SESSION_H_

#include <iocp_session.h>
#if defined(WITH_WORKING_AS_SERVER)
# include "sirius_localcache_server.h"
#else
# include "sirius_localcache_client.h"
#endif

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
#if defined(WITH_WORKING_AS_SERVER)
# define		localcache_processor server::core
#else
# define		localcache_processor client::core
#endif
				class localcache_processor;
				class session
					: public sirius::library::net::iocp::session
				{
				public:
					static const uint8_t protocol_version = 0x0;
					typedef struct _status_t
						: public sirius::library::net::iocp::session::status_t
					{
						static const int32_t waiting_request = 0x0080;
						static const int32_t request_received = 0x0100;
						static const int32_t sending_response = 0x0200;
						static const int32_t response_sended = 0x0400;
					} status_t;

				public:
					session(sirius::library::cache::local::localcache_processor * prcsr, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls = FALSE, SSL_CTX * ssl_ctx = NULL, BOOL reconnection = FALSE);
					virtual ~session(void);

					void send(int32_t cmd, const char * payload, int32_t payload_size);
					virtual int32_t	on_recv(const char * packet, int32_t packet_size);
					virtual int32_t	packet_header_size(void);

					BOOL			disconnect_flag(void) const;
					void			disconnect_flag(BOOL flag);

				protected:
					void			on_data_indication(int32_t command_id, const char * payload, int32_t payload_size, std::shared_ptr<sirius::library::cache::local::session> session);

				protected:
					sirius::library::cache::local::localcache_processor * _localcache_processor;

					BOOL _disconnect;
					CRITICAL_SECTION _send_lock;
					CRITICAL_SECTION _recv_lock;

					char * _send_buffer;
					char * _recv_buffer;
					uint32_t _recv_buffer_index;
				};
			};
		};
	};
};

#endif
