#ifndef _COMMANDS_CLIENT_H_
#define _COMMANDS_CLIENT_H_

#include <json\json.h>
#include <sicp_command.h>
#include "client_proxy.h"
#include "sirius_log4cplus_logger.h"

namespace sirius
{
	namespace app
	{
		namespace client
		{
			class abstract_client_cmd : public sirius::library::net::sicp::abstract_command
			{
			public:
				abstract_client_cmd(sirius::app::client::proxy::core * prxy, int32_t command_id)
					: abstract_command(command_id)
					, _proxy(prxy)
				{}

				virtual ~abstract_client_cmd(void)
				{}

			protected:
				sirius::app::client::proxy::core * _proxy;
			};

			class connect_client_res : public sirius::app::client::abstract_client_cmd
			{
			public:
				connect_client_res(sirius::app::client::proxy::core * prxy)
					: sirius::app::client::abstract_client_cmd(prxy, CMD_CONNECT_CLIENT_RES)
				{}
				virtual ~connect_client_res(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value rpacket;
					Json::Reader reader;
					reader.parse(msg, rpacket);

					int32_t rcode = -1;
					if (rpacket["rcode"].isInt())
						rcode = rpacket["rcode"].asInt();

					std::string rmsg = rpacket["msg"].asString();
					_proxy->connect_client_callback(rcode, rmsg.c_str());
				}
			};

			class disconnect_client_res : public sirius::app::client::abstract_client_cmd
			{
			public:
				disconnect_client_res(sirius::app::client::proxy::core * prxy)
					: sirius::app::client::abstract_client_cmd(prxy, CMD_DISCONNECT_CLIENT_RES)
				{}
				virtual ~disconnect_client_res(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value rpacket;
					Json::Reader reader;
					reader.parse(msg, rpacket);

					int32_t rcode = -1;
					if (rpacket["rcode"].isInt())
						rcode = rpacket["rcode"].asInt();

					_proxy->disconnect_client_callback(rcode);
				}
			};

			class attendant_info_noti : public sirius::app::client::abstract_client_cmd
			{
			public:
				attendant_info_noti(sirius::app::client::proxy::core * prxy)
					: sirius::app::client::abstract_client_cmd(prxy, CMD_ATTENDANT_INFO_IND)
				{}
				virtual ~attendant_info_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					int32_t rcode = -1;
					std::string attendant_uuid = "";
					int32_t streamer_portnumber = -1;
					int32_t video_width = -1;
					int32_t video_height = -1;

					Json::Value rpacket;
					Json::Reader reader;
					reader.parse(msg, rpacket);

					rcode = rpacket.get("rcode", sirius::app::client::proxy::err_code_t::fail).asInt();
					attendant_uuid = rpacket.get("attendant_uuid", -1).asString();
					streamer_portnumber = rpacket.get("streamer_portnumber", -1).asInt();
					video_width = rpacket.get("video_width", -1).asInt();
					video_height = rpacket.get("video_height", -1).asInt();

					_proxy->attendant_info_callback(rcode, attendant_uuid.c_str(), streamer_portnumber, video_width, video_height);
				}
			};

			class end2end_data_noti : public sirius::app::client::abstract_client_cmd
			{
			public:
				end2end_data_noti(sirius::app::client::proxy::core * prxy)
					: sirius::app::client::abstract_client_cmd(prxy, CMD_END2END_DATA_IND)
				{}
				virtual ~end2end_data_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					LOGGER::make_info_log(SAC, "%s, %d client end2end_data_noti data=%s", __FUNCTION__, __LINE__, msg);
					//_proxy->end2end_data_callback(msg, length);
				}
			};

			class error_noti : public sirius::app::client::abstract_client_cmd
			{
			public:
				error_noti(sirius::app::client::proxy::core * prxy)
					: sirius::app::client::abstract_client_cmd(prxy, CMD_ERROR_IND)
				{}
				virtual ~error_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					int32_t value = 0;
					Json::Value root;
					Json::Reader reader;
					reader.parse(msg, root);
					if (root["value"].isInt())
						value = root.get("value", 0).asInt();

					_proxy->error_callback(value);
				}
			};
		};
	};
};
#endif