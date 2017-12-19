#ifndef _COMMANDS_ATTENDANT_H_
#define _COMMANDS_ATTENDANT_H_

#include <inttypes.h>
#include <json\json.h>

#include <sicp_command.h>
#include "commands_payload.h"
#include "sirius_attendant_proxy.h"
#include "sirius_log4cplus_logger.h"

namespace sirius
{
	namespace app
	{
		namespace attendant
		{
			class attendant_cmd : public sirius::library::net::sicp::abstract_command
			{
			public:
				attendant_cmd(sirius::app::attendant::proxy * attendant, int32_t command_id)
					: sirius::library::net::sicp::abstract_command(command_id)
					, _attendant(attendant)
				{}
				virtual ~attendant_cmd(void)
				{}

			protected:
				sirius::app::attendant::proxy * _attendant;
			};

			class connect_attendant_res : public sirius::app::attendant::attendant_cmd
			{
			public:
				connect_attendant_res(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_CONNECT_ATTENDANT_RES)
				{}
				virtual ~connect_attendant_res(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value rpacket("");
					std::string rerr = "";
					Json::CharReaderBuilder rbuilder;
					std::shared_ptr<Json::CharReader> const reader(rbuilder.newCharReader());
					reader->parse(msg, msg + length, &rpacket, &rerr);

					int32_t code = -1;
					if (rpacket["rcode"].isInt())
						code = rpacket["rcode"].asInt();

					_attendant->connect_attendant_callback(code);
				}
			};

			class disconnect_attendant_req : public sirius::app::attendant::attendant_cmd
			{
			public:
				disconnect_attendant_req(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_DISCONNECT_ATTENDANT_REQ)
				{}
				virtual ~disconnect_attendant_req(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_attendant->disconnect_attendant_callback();
				}
			};

			class start_attendant_req : public sirius::app::attendant::attendant_cmd
			{
			public:
				start_attendant_req(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_START_ATTENDANT_REQ)
				{}
				virtual ~start_attendant_req(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value rpacket("");
					std::string rerr = "";
					Json::CharReaderBuilder rbuilder;
					std::shared_ptr<Json::CharReader> const reader(rbuilder.newCharReader());
					reader->parse(msg, msg + length, &rpacket, &rerr);

					std::string client_uuid = rpacket["client_uuid"].asString();
					std::string client_id = rpacket["client_id"].asString();

					_attendant->start_attendant_callback(client_uuid.c_str(), client_id.c_str());
				}
			};

			class stop_attendant_req : public sirius::app::attendant::attendant_cmd
			{
			public:
				stop_attendant_req(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_STOP_ATTENDANT_REQ)
				{}
				virtual ~stop_attendant_req(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value rpacket("");
					std::string rerr = "";
					Json::CharReaderBuilder rbuilder;
					std::shared_ptr<Json::CharReader> const reader(rbuilder.newCharReader());
					reader->parse(msg, msg + length, &rpacket, &rerr);

					std::string client_uuid = rpacket["client_uuid"].asString();

					_attendant->stop_attendant_callback(client_uuid.c_str());
				}
			};

			class keyup_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				keyup_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_KEY_UP_IND)
				{}
				virtual ~keyup_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_KEY_UP_IND_T))
						return;

					CMD_KEY_UP_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_KEY_UP_IND_T));
					memcpy(&noti, msg, sizeof(CMD_KEY_UP_IND_T));
					noti.key_code = ntohl(noti.key_code);
					_attendant->key_up_callback(noti.input_type, noti.key_code);
				}
			};

			class keydown_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				keydown_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_KEY_DOWN_IND)
				{}
				virtual ~keydown_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_KEY_DOWN_IND_T))
						return;

					CMD_KEY_DOWN_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_KEY_DOWN_IND_T));
					memcpy(&noti, msg, sizeof(CMD_KEY_DOWN_IND_T));
					noti.key_code = ntohl(noti.key_code);
					_attendant->key_down_callback(noti.input_type, noti.key_code);
				}
			};

			class mouse_move_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				mouse_move_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_MOUSE_MOVE_IND)
				{}
				virtual ~mouse_move_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MOUSE_MOVE_IND_T))
						return;

					CMD_MOUSE_MOVE_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MOUSE_MOVE_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MOUSE_MOVE_IND_T));
					noti.x_translation = ntohl(noti.x_translation);
					noti.y_translation = ntohl(noti.y_translation);
					_attendant->mouse_move_callback(noti.x_translation, noti.y_translation);
				}
			};

			class mouse_lbd_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				mouse_lbd_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_MOUSE_LBD_IND)
				{}
				virtual ~mouse_lbd_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MOUSE_LBD_IND_T))
						return;

					CMD_MOUSE_LBD_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MOUSE_LBD_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MOUSE_LBD_IND_T));
					noti.x = ntohl(noti.x);
					noti.y = ntohl(noti.y);
					_attendant->mouse_lbd_callback(noti.x, noti.y);
				}
			};

			class mouse_lbu_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				mouse_lbu_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_MOUSE_LBU_IND)
				{}
				virtual ~mouse_lbu_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MOUSE_LBU_IND_T))
						return;

					CMD_MOUSE_LBU_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MOUSE_LBU_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MOUSE_LBU_IND_T));
					noti.x = ntohl(noti.x);
					noti.y = ntohl(noti.y);
					_attendant->mouse_lbu_callback(noti.x, noti.y);
				}
			};

			class mouse_rbd_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				mouse_rbd_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_MOUSE_RBD_IND)
				{}
				virtual ~mouse_rbd_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MOUSE_RBD_IND_T))
						return;

					CMD_MOUSE_RBD_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MOUSE_RBD_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MOUSE_RBD_IND_T));
					noti.x = ntohl(noti.x);
					noti.y = ntohl(noti.y);
					_attendant->mouse_rbd_callback(noti.x, noti.y);
				}
			};

			class mouse_rbu_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				mouse_rbu_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_MOUSE_RBU_IND)
				{}
				virtual ~mouse_rbu_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MOUSE_RBU_IND_T))
						return;

					CMD_MOUSE_RBU_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MOUSE_RBU_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MOUSE_RBU_IND_T));
					noti.x = ntohl(noti.x);
					noti.y = ntohl(noti.y);
					_attendant->mouse_rbu_callback(noti.x, noti.y);
				}
			};

			class mouse_lb_dclick_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				mouse_lb_dclick_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_MOUSE_LB_DCLICK_IND)
				{}
				virtual ~mouse_lb_dclick_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MOUSE_LB_DCLICK_IND_T))
						return;

					CMD_MOUSE_LB_DCLICK_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MOUSE_LB_DCLICK_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MOUSE_LB_DCLICK_IND_T));
					noti.x = ntohl(noti.x);
					noti.y = ntohl(noti.y);
					_attendant->mouse_lb_dclick_callback(noti.x, noti.y);
				}
			};

			class mouse_rb_dclick_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				mouse_rb_dclick_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_MOUSE_RB_DCLICK_IND)
				{}
				virtual ~mouse_rb_dclick_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MOUSE_RB_DCLICK_IND_T))
						return;

					CMD_MOUSE_RB_DCLICK_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MOUSE_RB_DCLICK_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MOUSE_RB_DCLICK_IND_T));
					noti.x = ntohl(noti.x);
					noti.y = ntohl(noti.y);
					_attendant->mouse_rb_dclick_callback(noti.x, noti.y);
				}
			};

			class mouse_wheel_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				mouse_wheel_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_MOUSE_WHEEL_IND)
				{}
				virtual ~mouse_wheel_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MOUSE_WHEEL_IND_T))
						return;

					CMD_MOUSE_WHEEL_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MOUSE_WHEEL_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MOUSE_WHEEL_IND_T));
					noti.x = ntohl(noti.x);
					noti.y = ntohl(noti.y);
					noti.z_delta = ntohl(noti.z_delta);
					_attendant->mouse_wheel_callback(noti.x, noti.y, noti.z_delta);
				}
			};


			class infoxml_noti : public sirius::app::attendant::attendant_cmd
			{
			public:
				infoxml_noti(sirius::app::attendant::proxy * attendant)
					: sirius::app::attendant::attendant_cmd(attendant, CMD_CLIENT_INFO_XML_IND)
				{}
				virtual ~infoxml_noti(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_attendant->attendant_to_app_callback((uint8_t*)msg, length);
				}
			};
		};
	};
};

#endif


