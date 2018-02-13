#ifndef _IOCP_SESSION_H_
#define _IOCP_SESSION_H_

#include <iocp_io_context.h>
#include <memory>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace iocp
			{
#if defined(WITH_WORKING_AS_SERVER)
#define			processor server
#else
#define			processor client
#endif
				class processor;
				class session :	public std::enable_shared_from_this<sirius::library::net::iocp::session>
				{
					friend class sirius::library::net::iocp::processor;
				public:
					typedef struct _address_type_t
					{
						static const int32_t local	= 0;
						static const int32_t remote	= 1;
					} address_type_t;

					typedef struct _status_t
					{
						static const int32_t none			= 0x00;
						static const int32_t accepting		= 0x01;
						static const int32_t connecting		= 0x02;
						static const int32_t handshaking	= 0x04;
						static const int32_t connected		= 0x08;
						static const int32_t receiving		= 0x10;
						static const int32_t sending		= 0x20;
						static const int32_t closing		= 0x40;
						static const int32_t closed			= 0x80;
						static const int32_t operating		= accepting | connecting | handshaking | receiving | sending;
					} status_t;

					typedef struct _io_context_t
						: public sirius::library::net::iocp::io_context_t
						, public std::enable_shared_from_this<sirius::library::net::iocp::session::_io_context_t>
					{
						char *									packet;
						int32_t									packet_capacity;
						int32_t									packet_size;
						BOOL									tls;
						char *									ssl_packet;
						int32_t									ssl_packet_capacity;
						int32_t									ssl_packet_size;
						//BOOL									working;
						//DWORD									bytes_transfered;
						sirius::library::net::iocp::session *	session;
						CRITICAL_SECTION						lock;
						SOCKADDR_IN								peer;
						_io_context_t(size_t size, BOOL tls)
							: sirius::library::net::iocp::io_context_t(sirius::library::net::iocp::io_context_t::operation_t::send)
							, packet(nullptr)
							, packet_capacity(size)
							, packet_size(0)
							, tls(tls)
							, ssl_packet(nullptr)
							, ssl_packet_capacity(size)
							, ssl_packet_size(0)
							//, working(FALSE)
							//, bytes_transfered(0)
						{
							packet = static_cast<char*>(malloc(packet_capacity));
							::memset(packet, 0x00, packet_capacity);

							if (tls)
							{
								ssl_packet = static_cast<char*>(malloc(ssl_packet_capacity));
								::memset(ssl_packet, 0x00, ssl_packet_capacity);
							}

							::InitializeCriticalSection(&lock);
						}

						~_io_context_t(void)
						{
							if (packet)
							{
								free(packet);
								packet = nullptr;
								packet_capacity = 0;
								packet_size = 0;
							}

							if (ssl_packet)
							{
								free(ssl_packet);
								ssl_packet = nullptr;
								ssl_packet_capacity = 0;
								ssl_packet_size = 0;
							}

							::DeleteCriticalSection(&lock);
						}

						void resize(int32_t size)
						{
							if (packet_capacity >= size)
								return;

							if (packet)
							{
								packet = static_cast<char*>(realloc(packet, size));
								::memset(packet + packet_capacity, 0x00, size - packet_capacity);
							}
							packet_capacity = size;
						}

						void ssl_resize(int32_t size)
						{
							if (ssl_packet_capacity >= size)
								return;

							if (ssl_packet)
							{
								ssl_packet = static_cast<char*>(realloc(ssl_packet, size));
								::memset(ssl_packet + ssl_packet_capacity, 0x00, size - ssl_packet_capacity);
							}
							ssl_packet_capacity = size;
						}
					} io_context_t;

				public:
					session(sirius::library::net::iocp::processor * prcsr, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls = FALSE, SSL_CTX * ssl_ctx = nullptr, BOOL reconnection = FALSE);
					virtual ~session(void);

					SOCKET			listen_socket(void);
					void			listen_socket(SOCKET s);

					SOCKET			socket(void);
					void			socket(SOCKET s);
					BOOL			pending(void);
					BOOL			is_session_destroy(void);
					void			increase_session_destroy_count(void);
					
					uint32_t		status(void);
					void			status(uint32_t value);
					uint64_t		timestamp(void);
					void			update_timestamp(void);

					std::shared_ptr<sirius::library::net::iocp::session::io_context_t> recv_context(void);

					void			close(void);

					void			connect(const char * address = nullptr, int32_t portnumber = -1);
					void			accept(void);

					void			send(const char * packet, int32_t packet_size);
					void			recv(int32_t packet_size);

					BOOL			ssl_is_fatal_error(int32_t error);
					int32_t			ssl_get_error(SSL *ssl, int32_t result);

					virtual int32_t on_recv(const char * packet, int32_t packet_size) = 0;
					virtual int32_t packet_header_size(void) = 0;
				
				private:
					void			on_connect(std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context);
					void			on_accept(std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context);
					void			on_recv(std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context);
					void			on_send(std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context);
					void			on_completed(DWORD bytes_transfered, LPOVERLAPPED overlapped);

					static unsigned __stdcall	secure_send_process_cb(void * param);
					void						secure_send_process(void);
					static unsigned __stdcall	secure_recv_process_cb(void * param);
					void						secure_recv_process(void);


				protected:
					sirius::library::net::iocp::processor *								_processor;
					uint32_t															_status;
					uint64_t															_timestamp;
					SOCKET																_socket;
					SOCKET																_socket_listen;
					sockaddr_storage													_addresses[2]; // local and remote address
					std::shared_ptr<sirius::library::net::iocp::session::io_context_t>	_cnct_io_context;
					std::shared_ptr<sirius::library::net::iocp::session::io_context_t>	_recv_io_context;
					std::shared_ptr<sirius::library::net::iocp::session::io_context_t>	_send_io_context[64];
					int32_t																_nsend_io_context;
					//DWORD																_bytes_transfered[2];
					DWORD																_wsa_flags[2];

					BOOL																_tls;
					SSL_CTX *															_ssl_ctx;
					SSL *																_ssl;
					BIO *																_bio[2];

					HANDLE																_secure_send_thread;
					BOOL																_secure_send_run;
					HANDLE																_secure_recv_thread;
					BOOL																_secure_recv_run;

					LPFN_CONNECTEX														_pfn_connect;
					char																_address[MAX_PATH];
					int32_t																_portnumber;
					BOOL																_reconnection;

					uint32_t															_so_recv_buffer_size;
					uint32_t															_so_send_buffer_size;
					uint32_t															_recv_buffer_size;
					uint32_t															_send_buffer_size;

					CRITICAL_SECTION													_lock;
					volatile long														_ndestroy_session;
				};
			};
		};
	};
};

#endif