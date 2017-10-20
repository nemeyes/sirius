#ifndef _SIRIUS_ABSTRACT_SICP_SERVER_H_
#define _SIRIUS_ABSTRACT_SICP_SERVER_H_

#include <abstract_server.h>
#include <sicp_base.h>
#include <sicp_session.h>
#include <sicp_command.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace sicp
			{
				class abstract_server
					: public sirius::library::net::server
					, public sirius::library::net::sicp::base
				{
				public:
					abstract_server(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,const char * uuid, int32_t command_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast);
					virtual ~abstract_server(void);

					const char * uuid(void);
					void uuid(const char * uuid);

					bool check_alive_session(const char * uuid);
					std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> get_assoc_clients(void);
					//void set_keep_alive_flag(bool flag) { _enable_keep_alive = flag; }

					void data_indication_callback(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, size_t length, std::shared_ptr<sirius::library::net::sicp::session> session);
					void data_request(char * dst, int32_t command_id, char * msg, size_t length);
					void data_request(char * dst, char * src, int32_t command_id, char * msg, size_t length);

					//implement virtual function of sirius::library::net::abstract_server
					std::shared_ptr<sirius::library::net::session> create_session_callback(SOCKET client_socket, int32_t mtu,int32_t recv_buffer_size, bool dynamic_alloc = false);

					//implement virtual function of sirius::library::net::abstract_server
					void	destroy_session_callback(std::shared_ptr<sirius::library::net::session> session);

					//implement virtual function of sirius::library::net::abstract_server
					int32_t clean_conn_client(bool force_clean = false); 
					int32_t clean_assoc_session(bool force_clean = false);

					bool register_assoc_client(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session);
					bool unregister_assoc_client(std::shared_ptr<sirius::library::net::sicp::session> session);

					void add_command(sirius::library::net::sicp::abstract_command * command);
					void remove_command(int32_t command_id);

					void create_session_completion_callback(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session);
					void destroy_session_completion_callback(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session);

					virtual void create_session_callback(const char * uuid) = 0;
					virtual void destroy_session_callback(const char * uuid) = 0;

				protected:
					void clear_command_list(void);

					//implement virtual function of sirius::library::net::abstract_server
					void process(void);


				protected:
					bool	_use_keep_alive;
					char	_uuid[64];
					int32_t	_sequence;

					CRITICAL_SECTION _assoc_sessions_cs;
					std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>	_assoc_sessions;
					std::map<int32_t, sirius::library::net::sicp::abstract_command*>			_commands;

				private:
					abstract_server(sirius::library::net::sicp::abstract_server & clone);
				};	
			};
		};
	};
};







#endif
