#ifndef _COMMANDS_SERVER_H_
#define _COMMANDS_SERVER_H_

#include <inttypes.h>
#include <json/json.h>

# include <sicp_command.h>
#include "commands_payload.h"
#include "scsp_server.h"
#include <sirius_log4cplus_logger.h>

namespace sirius
{
	namespace library 
	{
		namespace net 
		{
			namespace scsp
			{
				class abstract_server_cmd : public sirius::library::net::sicp::abstract_command
				{
				public:
					abstract_server_cmd(sirius::library::net::scsp::server::core * cmes, int32_t command_id)
						: sirius::library::net::sicp::abstract_command(command_id)
						, _cmes(cmes) {}
					virtual ~abstract_server_cmd(void) {}

				protected:
					sirius::library::net::scsp::server::core * _cmes;
				};

				class play_req_cmd : public sirius::library::net::scsp::abstract_server_cmd
				{
				public:
					play_req_cmd(sirius::library::net::scsp::server::core * cmes)
						: sirius::library::net::scsp::abstract_server_cmd(cmes, CMD_PLAY_REQ) {}
					virtual ~play_req_cmd(void) {}

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						int32_t type = 0;

						Json::Value rpacket;
						Json::Reader reader;
						reader.parse(msg, rpacket);
						if (rpacket["type"].isInt())
							type = rpacket.get("type", 0).asInt();

						_cmes->play_callback(session->uuid(), type, "");
					}
				};
			};
		};
	};
};

#endif