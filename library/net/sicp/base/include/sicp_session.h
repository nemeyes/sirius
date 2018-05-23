#ifndef _SIRIUS_SICP_SESSION_H_
#define _SIRIUS_SICP_SESSION_H_

#include <iocp_session.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace sicp
			{
#if defined(WITH_WORKING_AS_SERVER)
# define		sicp_processor abstract_server
#else
# define		sicp_processor abstract_client
#endif
				class sicp_processor;
				class session
					: public sirius::library::net::iocp::session
				{
				public:
					static const uint8_t protocol_version = 0x0;

				public:
					session(sirius::library::net::sicp::sicp_processor * prcsr, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls= FALSE, SSL_CTX * ssl_ctx = NULL, BOOL reconnection = FALSE);
					virtual ~session(void);

					void			send(const char * dst, const char * src, int32_t cmd, const char * payload, int32_t payload_size);

					virtual int32_t	on_recv(const char * packet, int32_t packet_size);
					virtual int32_t	packet_header_size(void);

					const char *	ip(void);
					const char *	uuid(void);
					const char *	name(void);

					void			ip(const char * ip);
					void			uuid(const char * uuid);
					void			name(const char * name);

					BOOL			disconnect_flag(void) const;
					void			disconnect_flag(BOOL flag);
					BOOL			register_flag(void) const;
					void			register_flag(BOOL flag);

					void			keepalive_flag(BOOL flag);
					BOOL			keepalive_flag(void) const;

				protected:
					void			on_data_indication(const char * dst, const char * src,  int32_t command_id, uint8_t version, 
													   const char * payload, int32_t payload_size,  std::shared_ptr<sirius::library::net::sicp::session> session);


				protected:
					sirius::library::net::sicp::sicp_processor * _sicp_processor;

					BOOL		_disconnect;
					BOOL		_registerd;
					BOOL		_keepalive;

					char		_ip[MAX_PATH];
					char		_uuid[MAX_PATH];
					char		_name[MAX_PATH];

					CRITICAL_SECTION	_send_lock;
					CRITICAL_SECTION	_recv_lock;

					char *		_send_buffer;
					char *		_recv_buffer;
					uint32_t	_recv_buffer_index;
				};
			};
		};
	};
};










#endif
