#ifndef _COMMANDS_ATTENDANT_H_
#define _COMMANDS_ATTENDANT_H_

#include <inttypes.h>
#include <sicp_command.h>
#include "commands_payload.h"
#include "sirius_attendant_proxy.h"
#include "json\json.h"
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

			class stop_attendant_req_cmd : public attendant_cmd
			{
			public:
				stop_attendant_req_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_STOP_ATTENDANT_REQ)
				{}
				virtual ~stop_attendant_req_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					LOGGER::make_info_log(SLNS, "%s(), attendant disconnect request", __FUNCTION__, __LINE__);
					_attendant->on_destroy();
				}
			};

			class keyup_noti_cmd : public attendant_cmd
			{
			public:
				keyup_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_KEY_UP_IND)
				{}
				virtual ~keyup_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_KEY_UP_IND_T))
						return;

					CMD_KEY_UP_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_KEY_UP_IND_T));
					memcpy(&noti, msg, sizeof(CMD_KEY_UP_IND_T));
					noti.key_code = ntohl(noti.key_code);
					_attendant->on_key_up(noti.input_type, noti.key_code);
				}
			};

			class keydown_noti_cmd : public attendant_cmd
			{
			public:
				keydown_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_KEY_DOWN_IND)
				{}
				virtual ~keydown_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_KEY_DOWN_IND_T))
						return;

					CMD_KEY_DOWN_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_KEY_DOWN_IND_T));
					memcpy(&noti, msg, sizeof(CMD_KEY_DOWN_IND_T));
					noti.key_code = ntohl(noti.key_code);
					_attendant->on_key_down(noti.input_type, noti.key_code);
				}
			};

			class mouse_move_noti_cmd : public attendant_cmd
			{
			public:
				mouse_move_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_MOUSE_MOVE_IND)
				{}
				virtual ~mouse_move_noti_cmd(void)
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
					_attendant->on_mouse_move(noti.x_translation, noti.y_translation);
				}
			};

			class mouse_lbd_noti_cmd : public attendant_cmd
			{
			public:
				mouse_lbd_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_MOUSE_LBD_IND)
				{}
				virtual ~mouse_lbd_noti_cmd(void)
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
					_attendant->on_mouse_lbd(noti.x, noti.y);
				}
			};

			class mouse_lbu_noti_cmd : public attendant_cmd
			{
			public:
				mouse_lbu_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_MOUSE_LBU_IND)
				{}
				virtual ~mouse_lbu_noti_cmd(void)
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
					_attendant->on_mouse_lbu(noti.x, noti.y);
				}
			};

			class mouse_rbd_noti_cmd : public attendant_cmd
			{
			public:
				mouse_rbd_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_MOUSE_RBD_IND)
				{}
				virtual ~mouse_rbd_noti_cmd(void)
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
					_attendant->on_mouse_rbd(noti.x, noti.y);
				}
			};

			class mouse_rbu_noti_cmd : public attendant_cmd
			{
			public:
				mouse_rbu_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_MOUSE_RBU_IND)
				{}
				virtual ~mouse_rbu_noti_cmd(void)
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
					_attendant->on_mouse_rbu(noti.x, noti.y);
				}
			};

			class mouse_lb_dclick_noti_cmd : public attendant_cmd
			{
			public:
				mouse_lb_dclick_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_MOUSE_LB_DCLICK_IND)
				{}
				virtual ~mouse_lb_dclick_noti_cmd(void)
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
					_attendant->on_mouse_lb_dclick(noti.x, noti.y);
				}
			};

			class mouse_rb_dclick_noti_cmd : public attendant_cmd
			{
			public:
				mouse_rb_dclick_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_MOUSE_RB_DCLICK_IND)
				{}
				virtual ~mouse_rb_dclick_noti_cmd(void)
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
					_attendant->on_mouse_rb_dclick(noti.x, noti.y);
				}
			};

			class mouse_wheel_noti_cmd : public attendant_cmd
			{
			public:
				mouse_wheel_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_MOUSE_WHEEL_IND)
				{}
				virtual ~mouse_wheel_noti_cmd(void)
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
					_attendant->on_mouse_wheel(noti.x, noti.y, noti.z_delta);
				}
			};

			class seek_key_down_noti_cmd : public attendant_cmd
			{
			public:
				seek_key_down_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_SEEK_KEY_DOWN)
				{}
				virtual ~seek_key_down_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_SEEK_KEY_DOWN_T))
						return;

					CMD_SEEK_KEY_DOWN_T noti;
					memset(&noti, 0x00, sizeof(CMD_SEEK_KEY_DOWN_T));
					memcpy(&noti, msg, sizeof(CMD_SEEK_KEY_DOWN_T));
					noti.diff = ntohl(noti.diff);
					//_attendant->Seek(noti.diff);
				}
			};
			
			class seek_key_up_noti_cmd : public attendant_cmd
			{
			public:
				seek_key_up_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_SEEK_KEY_UP)
				{}
				virtual ~seek_key_up_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					//_attendant->SeekStop();
				}
			};

			class play_toggle_cmd : public attendant_cmd
			{
			public:
				play_toggle_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_PLAY_TOGGLE)
				{}
				virtual ~play_toggle_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_attendant->play_toggle();
				}
			};

			class backward_cmd : public attendant_cmd
			{
			public:
				backward_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_BACKWARD)
				{}
				virtual ~backward_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_attendant->backward();
				}
			};

			class forward_cmd : public attendant_cmd
			{
			public:
				forward_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_FORWARD)
				{}
				virtual ~forward_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_attendant->forward();
				}
			};

			class reverse_cmd : public attendant_cmd
			{
			public:
				reverse_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_REVERSE)
				{}
				virtual ~reverse_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_attendant->reverse();
				}
			};

			class gyro_noti_cmd : public attendant_cmd
			{
			public:
				gyro_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_IND)
				{}
				virtual ~gyro_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_GYRO_IND_T))
						return;

					CMD_GYRO_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_GYRO_IND_T));
					memcpy(&noti, msg, sizeof(CMD_GYRO_IND_T));

					//WCHAR str[1024];
					//_stprintf_s(str, _countof(str), L"be %f, %f, %f\n", noti.x.f, noti.y.f, noti.z.f);
					//OutputDebugString(str);		

					noti.x.ui = ntohl(noti.x.ui);
					noti.y.ui = ntohl(noti.y.ui);
					noti.z.ui = ntohl(noti.z.ui);

					_attendant->on_gyro(noti.x.f, noti.y.f, noti.z.f);
				}
			};

			class pinch_zoom_noti_cmd : public attendant_cmd
			{
			public:
				pinch_zoom_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_PINCH_ZOOM_IND)
				{}
				virtual ~pinch_zoom_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_PINCH_ZOOM_IND_T))
						return;

					CMD_PINCH_ZOOM_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_PINCH_ZOOM_IND_T));
					memcpy(&noti, msg, sizeof(CMD_PINCH_ZOOM_IND_T));
					int value = ntohl(noti.delta.ui);
					_attendant->on_pinch_zoom((float)value);
				}
			};

			/*
			class alive_check_res_cmd : public attendant_cmd
			{
			public:
				alive_check_res_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_AS_ALIVE_CHECK_RES)
				{}
				virtual ~alive_check_res_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					LOGGER::make_info_log(SLNS, "[CMD_AS_ALIVE_CHECK_RES] -  %s(), %d, Command:%d", __FUNCTION__, __LINE__, command_id);
					session->update_heart_beat();
				}
			};
			*/
			class connect_attendant_res_cmd : public attendant_cmd
			{
			public:
				connect_attendant_res_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_CONNECT_ATTENDANT_RES)
				{}
				virtual ~connect_attendant_res_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value root;
					Json::Reader reader;
					std::string client_uuid;
					reader.parse(msg, root);

					client_uuid = root.get("clientUuid", "nullptr").asString();
					int32_t res_code = root.get("resCode", 0).asInt();
					LOGGER::make_info_log(SLNS, "[Slot Connect Response] -  %s(), %d, Command:%d, clientUuid:%s, resCode:%d", __FUNCTION__, __LINE__, CMD_CONNECT_ATTENDANT_RES, client_uuid.c_str(), res_code);
					//	session->update_heart_beat();
				}
			};

			class infoxml_noti_cmd : public attendant_cmd
			{
			public:
				infoxml_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_CLIENT_INFO_XML_IND)
				{}
				virtual ~infoxml_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					_attendant->on_container_to_app((char *)msg, length);
				}
			};

			class gyro_attitude_noti_cmd : public attendant_cmd
			{
			public:
				gyro_attitude_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_ATTITUDE)
				{}
				virtual ~gyro_attitude_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_GYRO_ROT_IND_T))
						return;

					CMD_GYRO_ROT_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_GYRO_ROT_IND_T));
					memcpy(&noti, msg, sizeof(CMD_GYRO_ROT_IND_T));

					noti.x.ui = ntohl(noti.x.ui);
					noti.y.ui = ntohl(noti.y.ui);
					noti.z.ui = ntohl(noti.z.ui);
					noti.w.ui = ntohl(noti.w.ui);

					_attendant->on_gyro_attitude(noti.x.f, noti.y.f, noti.z.f, noti.w.f);
				}
			};

			class gyro_gravity_noti_cmd : public attendant_cmd
			{
			public:
				gyro_gravity_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_GRAVITY)
				{}
				virtual ~gyro_gravity_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_GYRO_IND_T))
						return;

					CMD_GYRO_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_GYRO_IND_T));
					memcpy(&noti, msg, sizeof(CMD_GYRO_IND_T));

					noti.x.ui = ntohl(noti.x.ui);
					noti.y.ui = ntohl(noti.y.ui);
					noti.z.ui = ntohl(noti.z.ui);

					_attendant->on_gyro_gravity(noti.x.f, noti.y.f, noti.z.f);
				}
			};

			class gyro_rotation_rate_noti_cmd : public attendant_cmd
			{
			public:
				gyro_rotation_rate_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_ROTATION_RATE)
				{}
				virtual ~gyro_rotation_rate_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_GYRO_IND_T))
						return;

					CMD_GYRO_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_GYRO_IND_T));
					memcpy(&noti, msg, sizeof(CMD_GYRO_IND_T));

					noti.x.ui = ntohl(noti.x.ui);
					noti.y.ui = ntohl(noti.y.ui);
					noti.z.ui = ntohl(noti.z.ui);

					_attendant->on_gyro_rotation_rate(noti.x.f, noti.y.f, noti.z.f);
				}
			};

			class gyro_rotation_rate_unbiased_noti_cmd : public attendant_cmd
			{
			public:
				gyro_rotation_rate_unbiased_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_ROTATION_RATE_UNBIASED)
				{}
				virtual ~gyro_rotation_rate_unbiased_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_GYRO_IND_T))
						return;

					CMD_GYRO_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_GYRO_IND_T));
					memcpy(&noti, msg, sizeof(CMD_GYRO_IND_T));

					noti.x.ui = ntohl(noti.x.ui);
					noti.y.ui = ntohl(noti.y.ui);
					noti.z.ui = ntohl(noti.z.ui);

					_attendant->on_gyro_rotation_rate_unbiased(noti.x.f, noti.y.f, noti.z.f);
				}
			};

			class gyro_user_acceleration_noti_cmd : public attendant_cmd
			{
			public:
				gyro_user_acceleration_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_ACCELERATION)
				{}
				virtual ~gyro_user_acceleration_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_GYRO_IND_T))
						return;

					CMD_GYRO_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_GYRO_IND_T));
					memcpy(&noti, msg, sizeof(CMD_GYRO_IND_T));

					noti.x.ui = ntohl(noti.x.ui);
					noti.y.ui = ntohl(noti.y.ui);
					noti.z.ui = ntohl(noti.z.ui);

					_attendant->on_gyro_user_acceleration(noti.x.f, noti.y.f, noti.z.f);
				}
			};

			class gyro_enabled_attitude_cmd : public attendant_cmd
			{
			public:
				gyro_enabled_attitude_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_ENABLED_ATTITUDE)
				{}
				virtual ~gyro_enabled_attitude_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value packet;
					Json::Reader reader;
					reader.parse(msg, packet);

					int32_t result = FALSE;
					if (packet["value"].isInt())
						result = packet.get("value", FALSE).asInt();

					_attendant->on_gyro_enabled_attitude(result != FALSE ? true : false);
				}
			};

			class gyro_enabled_gravity_cmd : public attendant_cmd
			{
			public:
				gyro_enabled_gravity_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_ENABLED_GRAVITY)
				{}
				virtual ~gyro_enabled_gravity_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value packet;
					Json::Reader reader;
					reader.parse(msg, packet);

					int32_t result = FALSE;
					if (packet["value"].isInt())
						result = packet.get("value", FALSE).asInt();

					_attendant->on_gyro_enabled_gravity(result != FALSE ? true : false);
				}
			};

			class gyro_enabled_rotation_rate_cmd : public attendant_cmd
			{
			public:
				gyro_enabled_rotation_rate_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_ENABLED_ROTATION_RATE)
				{}
				virtual ~gyro_enabled_rotation_rate_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value packet;
					Json::Reader reader;
					reader.parse(msg, packet);

					int32_t result = FALSE;
					if (packet["value"].isInt())
						result = packet.get("value", FALSE).asInt();

					_attendant->on_gyro_enabled_rotation_rate(result != FALSE ? true : false);
				}
			};

			class gyro_enabled_rotation_rate_unbiased_cmd : public attendant_cmd
			{
			public:
				gyro_enabled_rotation_rate_unbiased_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_ENABLED_ROTATION_RATE_UNBIASED)
				{}
				virtual ~gyro_enabled_rotation_rate_unbiased_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value packet;
					Json::Reader reader;
					reader.parse(msg, packet);

					int32_t result = FALSE;
					if (packet["value"].isInt())
						result = packet.get("value", FALSE).asInt();

					_attendant->on_gyro_enabled_rotation_rate_unbiased(result != FALSE ? true : false);
				}
			};

			class gyro_enabled_user_acceleration_cmd : public attendant_cmd
			{
			public:
				gyro_enabled_user_acceleration_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_ENABLED_USER_ACCELERATION)
				{}
				virtual ~gyro_enabled_user_acceleration_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value packet;
					Json::Reader reader;
					reader.parse(msg, packet);

					int32_t result = FALSE;
					if (packet["value"].isInt())
						result = packet.get("value", FALSE).asInt();

					_attendant->on_gyro_enabled_user_acceleration(result != FALSE ? true : false);
				}
			};

			class gyro_updateinterval_cmd : public attendant_cmd
			{
			public:
				gyro_updateinterval_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_GYRO_UPDATEINTERVAL)
				{}
				virtual ~gyro_updateinterval_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					Json::Value packet;
					Json::Reader reader;
					reader.parse(msg, packet);
					float result = packet.get("value", 0.15f).asFloat();

					_attendant->on_gyro_update_interval(result);
				}
			};

			class ar_view_mat_noti_cmd : public attendant_cmd
			{
			public:
				ar_view_mat_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_AR_VIEW_MAT)
				{}
				virtual ~ar_view_mat_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MAT4X4_IND_T))
						return;

					CMD_MAT4X4_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MAT4X4_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MAT4X4_IND_T));

					for (int i = 0; i < 4; i++)
					{
						for (int j = 0; j < 4; j++)
						{
							noti.m[i][j].ui = ntohl(noti.m[i][j].ui);
						}
					}

					_attendant->on_ar_view_mat(noti.m[0][0].f, noti.m[0][1].f, noti.m[0][2].f, noti.m[0][3].f,
						noti.m[1][0].f, noti.m[1][1].f, noti.m[1][2].f, noti.m[1][3].f,
						noti.m[2][0].f, noti.m[2][1].f, noti.m[2][2].f, noti.m[2][3].f,
						noti.m[3][0].f, noti.m[3][1].f, noti.m[3][2].f, noti.m[3][3].f);
				}
			};

			class ar_proj_mat_noti_cmd : public attendant_cmd
			{
			public:
				ar_proj_mat_noti_cmd(sirius::app::attendant::proxy * attendant)
					: attendant_cmd(attendant, CMD_AR_PROJ_MAT)
				{}
				virtual ~ar_proj_mat_noti_cmd(void)
				{}

				void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
				{
					if (length != sizeof(CMD_MAT4X4_IND_T))
						return;

					CMD_MAT4X4_IND_T noti;
					memset(&noti, 0x00, sizeof(CMD_MAT4X4_IND_T));
					memcpy(&noti, msg, sizeof(CMD_MAT4X4_IND_T));

					for (int i = 0; i < 4; i++)
					{
						for (int j = 0; j < 4; j++)
						{
							noti.m[i][j].ui = ntohl(noti.m[i][j].ui);
						}
					}

					_attendant->on_ar_proj_mat(noti.m[0][0].f, noti.m[0][1].f, noti.m[0][2].f, noti.m[0][3].f,
						noti.m[1][0].f, noti.m[1][1].f, noti.m[1][2].f, noti.m[1][3].f,
						noti.m[2][0].f, noti.m[2][1].f, noti.m[2][2].f, noti.m[2][3].f,
						noti.m[3][0].f, noti.m[3][1].f, noti.m[3][2].f, noti.m[3][3].f);
				};
			};
		};
	};
};

#endif


