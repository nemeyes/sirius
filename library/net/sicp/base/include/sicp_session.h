#ifndef _SIRIUS_SICP_SESSION_H_
#define _SIRIUS_SICP_SESSION_H_

#include <abstract_session.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace sicp
			{
#if defined(WITH_WORKING_AS_SERVER)
# define		base_processor server
# define		processor abstract_server
#else
# define		base_processor client
# define		processor abstract_client
#endif
				class base_processor;
				class processor;
				class session
					: public sirius::library::net::session
				{
				public:
					typedef struct _msg_type_t
					{
						static const int32_t binary = 0x00;
						static const int32_t json = 0x01;
						static const int32_t xml = 0x02;
						static const int32_t max_type = 0x03;
					} msg_type_t;

					static const uint8_t protocol_version = 0x0;

				public:
					session(sirius::library::net::sicp::processor * prcsr, SOCKET fd, int32_t mtu, int32_t recv_buffer_size,bool dynamic_alloc = false);
					virtual ~session(void);

					void	push_send_packet(const char * dst, const char * src, int32_t cmd, char * msg, int32_t length);

					//implement virtual function of sirius::library::net::session
					void	push_send_packet(const char * msg, int32_t length);
					//implement virtual function of sirius::library::net::session
					int32_t push_recv_packet(const char * msg, int32_t length);
					////implement virtual function of sirius::library::net::session
					int32_t get_first_recv_packet_size(void);

					void	update_heart_beat(void);

					const char *	ip(void);
					const char *	uuid(void);
					const char *	name(void);

					void			ip(const char * ip);
					void			uuid(const char * uuid);
					void			name(const char * name);

					bool			disconnect_flag(void) const;
					void			disconnect_flag(bool flag);
					bool			connected_flag(void) const;
					void			connected_flag(bool flag);
					bool			assoc_flag(void) const;
					void			assoc_flag(bool flag);

					int32_t			get_hb_period_sec(void) const;
					void			set_hb_period_sec(int32_t sec);

					void			update_hb_start_time(void);
					void			update_hb_end_time(void);
					bool			check_hb(void);

				protected:
					void			data_indication_callback(const char * dst, const char * src,  int32_t command_id, uint8_t version, 
															 const char * msg, size_t length,  std::shared_ptr<sirius::library::net::sicp::session> session);


				protected:
					sirius::library::net::sicp::processor * _processor;


					bool	_disconnect;
					bool	_connected;
					bool	_associated;

					char	_ip[20];
					char	_uuid[100];
					char	_name[100];

					uint64_t	_heart_beat;
					uint64_t	_hb_start;
					uint64_t	_hb_end;
					uint32_t	_hb_period;
					int32_t		_hb_retry_count;
				};
			};
		};
	};
};










#endif
