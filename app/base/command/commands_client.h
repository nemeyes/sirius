#ifndef _COMMANDS_CLIENT_H_
#define _COMMANDS_CLIENT_H_

#include <sicp_command.h>
#include "client_proxy.h"

namespace sirius
{
	namespace app
	{
		namespace client
		{
			class client_cmd : public sirius::library::net::sicp::abstract_command
			{
			public:
				client_cmd(sirius::app::client::proxy::core * prxy, int32_t command_id)
					: abstract_command(command_id)
					, _proxy(prxy)
				{}

				virtual ~client_cmd(void)
				{}

			protected:
				sirius::app::client::proxy::core * _proxy;
			};

			class connect_attendant_res_cmd : public client_cmd
			{
			public:
				connect_attendant_res_cmd(sirius::app::client::proxy::core * prxy)
					: client_cmd(prxy, CMD_CONNECT_ATTENDANT_RES)
				{}
				virtual ~connect_attendant_res_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_proxy->connect_attendant_callback(dst, src, msg, length);
				}
			};

			class disconnect_attendant_res_cmd : public client_cmd
			{
			public:
				disconnect_attendant_res_cmd(sirius::app::client::proxy::core * prxy)
					: client_cmd(prxy, CMD_DISCONNECT_ATTENDANT_RES)
				{}
				virtual ~disconnect_attendant_res_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_proxy->disconnect_attendant_callback(dst, src, msg, length);
				}
			};

			class attendant_info_ind_cmd : public client_cmd
			{
			public:
				attendant_info_ind_cmd(sirius::app::client::proxy::core * prxy)
					: client_cmd(prxy, CMD_ATTENDANT_INFO_IND)
				{}
				virtual ~attendant_info_ind_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_proxy->attendant_info_callback(dst, src, msg, length);
				}

			};

			class xml_ind_cmd : public client_cmd
			{
			public:
				xml_ind_cmd(sirius::app::client::proxy::core * prxy)
					: client_cmd(prxy, CMD_CLIENT_INFO_XML_IND)
				{}
				virtual ~xml_ind_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_proxy->xml_callback(dst, src, msg, length);
				}

			};

			class error_ind_cmd : public client_cmd
			{
			public:
				error_ind_cmd(sirius::app::client::proxy::core * prxy)
					: client_cmd(prxy, CMD_ERROR_IND)
				{}
				virtual ~error_ind_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_proxy->error_callback(dst, src, msg, length);
				}
			};
		};
	};
};
#endif