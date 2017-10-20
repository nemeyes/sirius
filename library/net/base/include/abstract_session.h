#ifndef _ABSTRACT_SESSION_H_
#define _ABSTRACT_SESSION_H_

#if defined(WIN32)
# include <iocp_io_context.h>
#elif defined(__linux__)
# include <epoll_io_context.h>
#elif defined(__MACOS__) || defined(__FreeBSD__)
# include <kqueue_io_context.h>
#endif

namespace sirius
{
	class multiple_inheritable_enable_shared_from_this : public std::enable_shared_from_this<multiple_inheritable_enable_shared_from_this>
	{
	public:
		virtual ~multiple_inheritable_enable_shared_from_this(void) {}
	};

	template <class T>
	class inheritable_enable_shared_from_this : virtual public sirius::multiple_inheritable_enable_shared_from_this
	{
	public:
		std::shared_ptr<T> shared_from_this(void)
		{
			return std::dynamic_pointer_cast<T>(sirius::multiple_inheritable_enable_shared_from_this::shared_from_this());
		}

		template <class Down>
		std::shared_ptr<Down> downcasted_shared_from_this(void)
		{
			return std::dynamic_pointer_cast<Down>(sirius::multiple_inheritable_enable_shared_from_this::shared_from_this());
		}
	};

	namespace library
	{
		namespace net
		{
			class session :
				public sirius::inheritable_enable_shared_from_this<sirius::library::net::session>
			{
			public:

				typedef struct _send_buffer_t
#if defined(WIN32)
					: public sirius::library::net::iocp::io_context_t
#elif defined(__linux__)
					: public sirius::library::net::epoll::io_context_t
#elif defined(__MACOS__) || defined(__FreeBSD__)
					: public sirius::library::net::kqueue::io_context_t
#endif
					, public std::enable_shared_from_this<sirius::library::net::session::_send_buffer_t>
				{
					char *		packet;
					int32_t		packet_size;
					int32_t		send_size;
					SOCKADDR_IN peer;
					_send_buffer_t(size_t size)
#if defined(WIN32)
						: sirius::library::net::iocp::io_context_t(sirius::library::net::iocp::io_context_t::operation_t::send)
#elif defined(__linux__)
						: sirius::library::net::epoll::io_context_t(sirius::library::net::epoll::io_context_t::operation_t::send)
#elif defined(__MACOS__) || defined(__FreeBSD__)
						: sirius::library::net::kqueue::io_context_t(sirius::library::net::kqueue::io_context_t::operation_t::send)
#endif
						, packet_size(size)
						, send_size(0)
					{
						packet = static_cast<char*>(malloc(packet_size));
						::memset(packet, 0x00, sizeof(packet));
					}

					~_send_buffer_t(void)
					{
						if (packet)
						{
							free(packet);
							packet = nullptr;
							packet_size = 0;
						}
						send_size = 0;
					}

					void resize(int32_t size)
					{
						if (packet_size >= size)
							return;
						if (packet)
						{
							packet = static_cast<char*>(realloc(packet, size));
							::memset(packet + packet_size, 0x00, size - packet_size);
							packet_size = size;
						}
					}
				} send_buffer_t;

				typedef struct _recv_buffer_t
#if defined(WIN32)
					: public sirius::library::net::iocp::io_context_t
#elif defined(__linux__)
					: public sirius::library::net::epoll::io_context_t
#elif defined(__MACOS__) || defined(__FreeBSD__)
					: public sirius::library::net::kqueue::io_context_t
#endif
					, public std::enable_shared_from_this<sirius::library::net::session::_recv_buffer_t>
				{
					char *		packet;
					int32_t		packet_size;
					int32_t		recv_size;
					SOCKADDR_IN	peer;
					_recv_buffer_t(size_t size)
#if defined(WIN32)
						: sirius::library::net::iocp::io_context_t(sirius::library::net::iocp::io_context_t::operation_t::recv)
#elif defined(__linux__)
						: sirius::library::net::epoll::io_context_t(sirius::library::net::epoll::io_context_t::operation_t::recv)
#elif defined(__MACOS__) || defined(__FreeBSD__)
						: sirius::library::net::kqueue::io_context_t(sirius::library::net::kqueue::io_context_t::operation_t::recv)
#endif
						, packet_size(size)
						, recv_size(0)
					{
						packet = static_cast<char*>(malloc(packet_size));
						::memset(packet, 0x00, sizeof(packet));
					}

					~_recv_buffer_t(void)
					{
						if (packet)
						{
							free(packet);
							packet = nullptr;
							packet_size = 0;
						}
						recv_size = 0;
					}

					void resize(int32_t size)
					{
						if (packet_size >= size)
							return;
						if (packet)
						{
							packet = static_cast<char*>(realloc(packet, size));
							::memset(packet + packet_size, 0x00, size - packet_size);
							packet_size = size;
						}
					}
				} recv_buffer_t;

			public:
				session(SOCKET fd, int32_t mtu,int32_t recv_buffer_size, bool dynamic_alloc = false);
				virtual ~session(void);
				
				void			push_back_send_buffer(std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ);
				void			push_back_send_buffer(std::shared_ptr<sirius::library::net::session::send_buffer_t> send_buffer);
				void			pop_send_buffer(std::shared_ptr<sirius::library::net::session::send_buffer_t> send_buffer);

				SOCKET			fd(void);
				void			fd(SOCKET fd);

				uint64_t		get_last_access_tm(void);
				void			update_last_access_tm(void);

				std::shared_ptr<sirius::library::net::session::recv_buffer_t> recv_context(void);
				bool			close(void);

				virtual int32_t push_recv_packet(const char * msg, int32_t length) { return 0; };
				virtual int32_t get_first_recv_packet_size(void) { return 0; };

			protected:
				bool			shutdown_fd(void);

			protected:
				bool					_dynamic_alloc;
				uint64_t				_last_access_tm;
				char *					_precv_buffer;

				uint32_t				_recv_buffer_index;
				uint32_t				_recv_buffer_size;

				CRITICAL_SECTION		_cs;
				CRITICAL_SECTION		_ready_send_cs_lock;
				CRITICAL_SECTION		_send_cs_lock;
				CRITICAL_SECTION		_recv_cs_lock;
				int32_t					_mtu;
				SOCKET					_fd;

				std::deque<std::shared_ptr<sirius::library::net::session::send_buffer_t>>	_send_ready_queue;
				std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>>	_send_queue;
				std::shared_ptr<sirius::library::net::session::recv_buffer_t>				_recv_context;
			};
		};
	};
};

#endif