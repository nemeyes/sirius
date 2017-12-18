#ifndef _CASP_SERVER_H_
#define _CASP_SERVER_H_

#include <string>
#include <map>
#include <vector>
#include <time.h>
#include <memory>
#include <sirius_sicp_server.h>
#include "sirius_casp_server.h"

#define MAX_STREAM_QUEUE			30
#define MAX_VIDEO_STREAM_QUEUE		10
#define THREAD_STACK_SIZE			1024 * 128

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace casp
			{
				class server::core
					: public sirius::library::net::sicp::server
				{
				public:

					typedef struct _send_buffer_t
					{
						uint8_t * packet;
						size_t size;
					} send_buffer_t;

					template<class T>
					class stream_queue
					{
					public:
						stream_queue(void);
						virtual ~stream_queue(void);

						bool initialize(T *pItems, uint32_t size);
						T* get_available(void);
						T* get_pending(void);
						T* pop_pending(void);
						T* set_available(uint8_t *bitstream, size_t size);
						uint32_t get_count(void);

					protected:
						T** _buffer;
						uint32_t _size;
						uint32_t _pending_count;
						uint32_t _available_index;
						uint32_t _pending_index;
					};

					void allocate_send_buffer(int32_t cnt);
					void release_send_buffer(void);


					typedef struct _stream_conf
					{
						size_t stream_type;
						std::vector<std::string> uuids;
						int32_t state;
						CRITICAL_SECTION cs;
#if defined(WITH_STREAMING_BLOCKING_MODE)
						std::vector<SOCKET> fds;
						HANDLE thread;
						uint32_t thread_id;
						HANDLE	send_event;
						bool thread_run;
#endif
						uint8_t * bitstream;
						size_t packet_length;

						size_t	_send_queue_size;
						send_buffer_t _send_buffers[MAX_STREAM_QUEUE];
						stream_queue<send_buffer_t>	_send_queue;

						_stream_conf(void)
						{
							uuids.clear();
#if defined(WITH_STREAMING_BLOCKING_MODE)
							fds.clear();
#endif
							::InitializeCriticalSection(&cs);
						}

						virtual ~_stream_conf(void)
						{
							::DeleteCriticalSection(&cs);
							uuids.clear();
#if defined(WITH_STREAMING_BLOCKING_MODE)
							fds.clear();
#endif
						}

					} stream_conf_t;

					core(const char * uuid, sirius::library::net::casp::server * front);
					virtual ~core(void);

					int32_t publish_begin(int32_t vsmt, int32_t asmt, const char * address, int32_t port_number, const char * uuid, sirius::library::net::casp::server::proxy * sc);
					int32_t publish_end(void);

					int32_t publish_video(uint8_t * bitstream, size_t nb, long long timestamp);

					int32_t play(int32_t flags);
					int32_t pause(int32_t flags);
					int32_t stop(int32_t flags);

					void create_session_callback(const char * uuid);
					void destroy_session_callback(const char * uuid);
					void polling_callback() {};
					int32_t client_request_stream_res(const char * client_uuid, const char * msg, int length, SOCKET clientsocket);
					
					int32_t state_start(int32_t type);

#if defined(WITH_STREAMING_BLOCKING_MODE)
					static unsigned __stdcall process_cb(void * param);
					int32_t send_packet(SOCKET fd, uint8_t * packet, size_t length);
					void process(void);
#endif

					send_buffer_t* get_stream_buffer(int32_t stream_type);
					int32_t set_stream_buffer(int32_t stream_type, uint8_t *bitstream, size_t size);

					int32_t deinit_stream(core::stream_conf_t * stream_conf);

				private:
					sirius::library::net::casp::server *						_front;
					core::stream_conf_t				_video_conf;
					int32_t									_vsmt;
					int32_t									_asmt;
					int32_t									_port_number;
					int32_t									_slot_number;
					sirius::library::net::casp::server::proxy *	_controller;
					size_t									_mtu;
					char									_address[MAX_PATH];
					char									_uuid[MAX_PATH];

					uint32_t								_video_index;
				};
			};
		};
	};
};
#endif
