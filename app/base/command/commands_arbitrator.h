#ifndef _COMMANDS_SERVER_H_
#define _COMMANDS_SERVER_H_

#include <inttypes.h>
#include <json/json.h>

#include <sicp_command.h>
#include "commands_payload.h"
#include "arbitrator_proxy.h"
#include "sirius_log4cplus_logger.h"

#define MAX_BANDWIDTH_BYTES 18000000000000000000
namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{
				class arbitrator_cmd : public sirius::library::net::sicp::abstract_command
				{
				public:
					arbitrator_cmd(sirius::app::server::arbitrator::proxy * prxy, int32_t command_id)
						: sirius::library::net::sicp::abstract_command(command_id)
						, _proxy(prxy)
					{}

					virtual ~arbitrator_cmd(void)
					{}

				protected:
					sirius::app::server::arbitrator::proxy * _proxy;
				};


				//begin client <-> arbitrator
				class connect_client_req : public sirius::app::server::arbitrator::arbitrator_cmd
				{
				public:
					connect_client_req(sirius::app::server::arbitrator::proxy * prxy)
						: sirius::app::server::arbitrator::arbitrator_cmd(prxy, CMD_CONNECT_CLIENT_REQ)
					{}
					virtual ~connect_client_req(void)
					{}

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						Json::Value rpacket("");
						Json::Reader reader;
						reader.parse(msg, rpacket);

						std::string id = rpacket.get("id", "").asString();
						int32_t status = _proxy->connect_client(session->uuid(), id.c_str());

						Json::Value wpacket;
						Json::StyledWriter writer;
						wpacket["rcode"] = status;
						std::string response = writer.write(wpacket);

						session->send(session->uuid(), uuid(), CMD_CONNECT_CLIENT_RES, (char*)response.c_str(), response.size() + 1);
					}
				};

				class disconnect_client_req : public sirius::app::server::arbitrator::arbitrator_cmd
				{
				public:
					disconnect_client_req(sirius::app::server::arbitrator::proxy * prxy)
						: sirius::app::server::arbitrator::arbitrator_cmd(prxy, CMD_DISCONNECT_CLIENT_REQ)
					{}
					virtual ~disconnect_client_req(void)
					{}

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						int32_t status = _proxy->disconnect_client(session->uuid());

						Json::Value wpacket;
						Json::StyledWriter writer;
						wpacket["rcode"] = status;
						std::string response = writer.write(wpacket);

						session->send(session->uuid(), uuid(), CMD_DISCONNECT_CLIENT_RES, (char*)response.c_str(), response.size() + 1);
					}
				};
				//end client <-> arbitrator

				//begin attendant <-> arbitrator
				class connect_attendant_req : public sirius::app::server::arbitrator::arbitrator_cmd
				{
				public:
					connect_attendant_req(sirius::app::server::arbitrator::proxy * prxy)
						: sirius::app::server::arbitrator::arbitrator_cmd(prxy, CMD_CONNECT_ATTENDANT_REQ)
					{}
					virtual ~connect_attendant_req(void)
					{}

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						Json::Value rpacket("");
						std::string rerr = "";
						Json::CharReaderBuilder rbuilder;
						std::shared_ptr<Json::CharReader> const reader(rbuilder.newCharReader());
						reader->parse(msg, msg + length, &rpacket, &rerr);

						int32_t id = -1;
						int32_t pid = -1;

						if (rpacket["id"].isInt())
							id = rpacket.get("id", -1).asInt();

						if (rpacket["pid"].isInt())
							pid = rpacket.get("pid", -1).asInt();
						
						int32_t status = _proxy->connect_attendant_callback(session->uuid(), id, pid);

						Json::Value wpacket;
						Json::StyledWriter writer;
						wpacket["rcode"] = status;
						std::string response = writer.write(wpacket);

						session->send(session->uuid(), uuid(), CMD_CONNECT_ATTENDANT_RES, (char*)response.c_str(), response.size() + 1);
					}
				};

				class disconnect_attendant_res : public sirius::app::server::arbitrator::arbitrator_cmd
				{
				public:
					disconnect_attendant_res(sirius::app::server::arbitrator::proxy * prxy)
						: sirius::app::server::arbitrator::arbitrator_cmd(prxy, CMD_DISCONNECT_ATTENDANT_RES)
					{}
					virtual ~disconnect_attendant_res(void)
					{}

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						_proxy->disconnect_attendant_callback(session->uuid());
					}
				};


				class start_attendant_res : public sirius::app::server::arbitrator::arbitrator_cmd
				{
				public:
					start_attendant_res(sirius::app::server::arbitrator::proxy * prxy)
						: sirius::app::server::arbitrator::arbitrator_cmd(prxy, CMD_START_ATTENDANT_RES)
					{}
					virtual ~start_attendant_res(void)
					{}

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						Json::Value rpacket("");
						Json::Reader reader;
						reader.parse(msg, rpacket);

						int32_t id = rpacket.get("id", "").asInt();
						std::string client_id = rpacket.get("client_id", "").asString();
						std::string client_uuid = rpacket.get("client_uuid", "").asString();
						int32_t rcode = rpacket.get("rcode", -1).asInt();
						_proxy->start_attendant_callback(session->uuid(), id, client_id.c_str(),client_uuid.c_str(), rcode);
					}
				};

				class stop_attendant_res : public sirius::app::server::arbitrator::arbitrator_cmd
				{
				public:
					stop_attendant_res(sirius::app::server::arbitrator::proxy * prxy)
						: sirius::app::server::arbitrator::arbitrator_cmd(prxy, CMD_STOP_ATTENDANT_RES)
					{}
					virtual ~stop_attendant_res(void)
					{}

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						Json::Value rpacket("");
						Json::Reader reader;
						reader.parse(msg, rpacket);

						int32_t rcode = rpacket.get("rcode", -1).asInt();
						_proxy->stop_attendant_callback(session->uuid(), rcode);
					}
				};
				//end attendant <-> arbitrator
			};
		};
	};
};






#endif