#ifndef _COMMANDS_SERVER_H_
#define _COMMANDS_SERVER_H_

#include <inttypes.h>
#include <command.h>
#include "commands_payload.h"
#include "server_controller.h"
#include "cap_log4cplus_logger.h"
#include "json/json.h"
#include "core/csr_adapter.h"
#include "slot_manager.h"

#define MAX_BANDWIDTH_BYTES 18000000000000000000
namespace ic
{
	class abstract_server_controller_cmd : public abstract_command
	{
	public:
		abstract_server_controller_cmd(ic::server_controller * controller, int32_t command_id)
			: abstract_command(command_id)
			, _controller(controller) 
		{}

		virtual ~abstract_server_controller_cmd(void)
		{}

	protected:
		ic::server_controller * _controller;
	};

	class ca_alive_check_req_cmd : public abstract_server_controller_cmd
	{
	public:
		ca_alive_check_req_cmd(ic::server_controller * controller)
			: abstract_server_controller_cmd(controller, CMD_CA_ALIVE_CHECK_REQ)
		{}
		virtual ~ca_alive_check_req_cmd(void)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			session->push_send_packet(session->uuid(), uuid(), CMD_AC_ALIVE_CHECK_RES, ic::session::msg_type_t::binary, nullptr, 0);
			session->update_heart_beat();
			_controller->ca_client_alive_check_noti(session->uuid());
		}
	};

	class ca_create_slot_req_cmd : public abstract_server_controller_cmd
	{
	public:
		ca_create_slot_req_cmd(ic::server_controller * controller)
			: abstract_server_controller_cmd(controller, CMD_CA_CREATE_SLOT_REQ)
		{}
		virtual ~ca_create_slot_req_cmd(void)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			int ret_slotnum;
			int32_t return_code = _controller->create_slot(dst, src, msg, length,ret_slotnum);
			Json::Value res_packet;
			Json::StyledWriter writer;
			
			res_packet["resCode"] = return_code;

			switch (return_code)
			{
			case ic::server_controller::err_code_t::success:
				res_packet["resMsg"] = "success";
				break;
			case ic::server_controller::err_code_t::fail:
				res_packet["resMsg"] = "fail";
				break;
			case ic::server_controller::err_code_t::not_implemented:
				res_packet["resMsg"] = "not implemented";
				break;
			case ic::server_controller::err_code_t::unsupported_function:
				res_packet["resMsg"] = "unsupported function";
				break;
			case ic::server_controller::err_code_t::invalid_encoding_device:
				res_packet["resMsg"] = "invalid encoding_device";
				break;
			case ic::server_controller::err_code_t::encoding_under_processing:
				res_packet["resMsg"] = "encoding under processing";
				break;
			case ic::server_controller::err_code_t::invalid_parameter:
				res_packet["resMsg"] = "invalid parameter";
				break;
			case ic::server_controller::err_code_t::slot_full:
				res_packet["resMsg"] = "slot full";
				break;
			case ic::server_controller::err_code_t::slot_launcher_fail:
				res_packet["resMsg"] = "slot launcher fail";
				break;
			case ic::server_controller::err_code_t::invaild_appid:
				res_packet["resMsg"] = "invaild appid";
				break;
			case ic::server_controller::err_code_t::duplicate_uuid:
				res_packet["resMsg"] = "duplicate uuid";
				break;
			case ic::server_controller::err_code_t::invalid_device_type:
				res_packet["resMsg"] = "invalid device_type";
				break;
			case ic::server_controller::err_code_t::unsupported_resolution:
				res_packet["resMsg"] = "unsupported resolution";
				break;
			case ic::server_controller::err_code_t::max_es_size_over:
				res_packet["resMsg"] = "max es size over";
				break;
			case ic::server_controller::err_code_t::slot_count_danger:
				res_packet["resMsg"] = "slot count danger";
				break;
			case ic::server_controller::err_code_t::gpu_load_danger:
				res_packet["resMsg"] = "gpu load danger";
				break;
			case ic::server_controller::err_code_t::gpu_error_detection:
				res_packet["resMsg"] = "gpu error detection";
				break;
			case ic::server_controller::err_code_t::duplicate_device_id:
				res_packet["resMsg"] = "duplicate device id";
				break;
			case ic::server_controller::err_code_t::invalid_file_path:
				res_packet["resMsg"] = "invalid file path";
				break;
			}

			std::string res_msg = writer.write(res_packet);
			session->push_send_packet(session->uuid(), uuid(), CMD_AC_CREATE_SLOT_RES, ic::session::msg_type_t::json,(char*)res_msg.c_str(), res_msg.size() + 1);
			
			Json::Reader reader;
			Json::Value packet;
			std::string res_massage;
			reader.parse((char*)res_msg.c_str(), res_packet);
			res_massage = res_packet.get("resMsg", "nullptr").asString();
					
			if (return_code == ic::server_controller::err_code_t::success)
			{
				reader.parse(msg, packet);
				std::string stb_id = packet.get("stbId", "unknown").asString();

				session->name(stb_id.c_str());
				int slotnum = ret_slotnum;//_controller->get_slot_num();
				session->slot_num(slotnum);
				//LOGGER::make_info_log(CAPA, "[Create Slot Response] - %s(), %d, Command:%d, resCode:%d, resMsg:%s, stb_id:%s, slotnum:%d src:%s", __FUNCTION__, __LINE__, CMD_AC_CREATE_SLOT_RES, return_code, res_massage.c_str(), stb_id.c_str(), slotnum, src);
			}
		}
	};

	class ca_destroy_slot_req_cmd : public abstract_server_controller_cmd
	{
	public:
		ca_destroy_slot_req_cmd(ic::server_controller * controller)
			: abstract_server_controller_cmd(controller, CMD_CA_DESTROY_SLOT_REQ)
		{}
		virtual ~ca_destroy_slot_req_cmd(void)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			int32_t code = _controller->destroy_slot(dst, src, msg, length);

			Json::Value res_packet;
			Json::StyledWriter writer;
			res_packet["resCode"] = code;
			std::string res_msg = writer.write(res_packet);
			session->push_send_packet(session->uuid(), uuid(), CMD_AC_DESTROY_SLOT_RES, ic::session::msg_type_t::json,(char*)res_msg.c_str(), res_msg.size() + 1);
		}
	};

	class slot_connect_req_cmd : public abstract_server_controller_cmd
	{
	public:
		slot_connect_req_cmd(ic::server_controller * controller)
			: abstract_server_controller_cmd(controller, CMD_SA_CONNECT_REQ) {}
		virtual ~slot_connect_req_cmd(void) {}
		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			//Slot connect request 
			int resCode = _controller->sa_slot_connect_req(session->uuid(), msg, length);
			log_info(CAPS, "%s, %d sa_slot_connect_req", __FUNCTION__, __LINE__);
			//Slot connect response
			_controller->as_slot_connect_res(session->uuid(), resCode);

			if (resCode == server_controller::err_code_t::success)
			{
				//Slot info notification
				_controller->ac_slot_info_noti(session->uuid(), resCode);
			}
		}
	};

	class slot_disconnect_req_cmd : public abstract_server_controller_cmd
	{
	public:
		slot_disconnect_req_cmd(ic::server_controller * controller)
			: abstract_server_controller_cmd(controller, CMD_SA_DISCONNECT_REQ) 
		{}
		virtual ~slot_disconnect_req_cmd(void) 
		{}

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			log_info(CAPA, "%s, %d dst=%s, src=%s, cmd=%d", __FUNCTION__, __LINE__, dst, src, command_id);

			//Slot disconnect response
			_controller->as_slot_disconnect_res(session->uuid(),0);

			//Slot disconnect request porcess
			_controller->sa_slot_disconnect_req(session->uuid(), msg, length);
		}
	};
	
	class slot_disconnect_res_cmd : public abstract_server_controller_cmd
	{
	public:
		slot_disconnect_res_cmd(ic::server_controller * controller)
			: abstract_server_controller_cmd(controller, CMD_SA_DISCONNECT_RES) 
		{}
		virtual ~slot_disconnect_res_cmd(void) 
		{}

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{	
			log_info(CAPA, "%s, %d, dst=%s, src=%s, cmd=%d", __FUNCTION__, __LINE__, dst, src, command_id);

			//slot disconnect response process
			_controller->sa_slot_disconnect_res(session->uuid(),msg, length);
		}
	};

	class slot_alive_check_req_cmd : public abstract_server_controller_cmd
	{
	public:
		slot_alive_check_req_cmd(server_controller * cmes)
			: abstract_server_controller_cmd(cmes, CMD_SA_ALIVE_CHECK_REQ) {}
		virtual ~slot_alive_check_req_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			log_debug(CAPA, "%s, %d, dst=%s, src=%s, cmd=%d", __FUNCTION__, __LINE__, dst, src, command_id);

			char req_msg[128] = { 0 };
			memcpy(req_msg, msg, length);

			Json::Value packet;
			Json::Reader reader;
			reader.parse(req_msg, packet);

			uint32_t pid = -1;
			uint64_t audio_transferred_bytes = 0;
			uint64_t video_transferred_bytes = 0;
			uint64_t total_transferred_bytes = 0;
			uint64_t last_total_transferred_bytes = 0;
			if (packet["pid"].isInt())						pid = packet.get("pid", -1).asInt();
			if (packet["audio_trans_bytes"].isInt64())		audio_transferred_bytes = packet.get("audio_trans_bytes", 0).asInt64();
			if (packet["video_trans_bytes"].isInt64())		video_transferred_bytes = packet.get("video_trans_bytes", 0).asInt64();

			if (Config.is_statistice_use() && Config.is_csr_use() && Config.is_soc_use())
			{
				slot_stateinfo *pslot = SLTMGR.find(src);
				if (pslot != nullptr)
				{
					std::string device_id = pslot->get_stb_id();
					if (device_id.size() > 0)
					{
						std::string device_id_start_string, device_id_end_string;
						device_id_start_string.push_back(device_id[0]);
						device_id_end_string.push_back(device_id[device_id.size() - 1]);
						if (device_id_start_string.compare("{") == 0 && device_id_end_string.compare("}") == 0)
						{
							total_transferred_bytes = audio_transferred_bytes + video_transferred_bytes;
							last_total_transferred_bytes = pslot->get_last_total_bandwidth_bytes();

							CSR_ADT._new_total_bandwidth_bytes += (total_transferred_bytes - last_total_transferred_bytes);
							pslot->set_last_total_bandwidth_bytes(total_transferred_bytes);
							log_debug(CAPA, "%s, %d, total_transferred_bytes:%I64d(bytes), last_total_transferred_bytes:%I64d(bytes), _new_total_bandwidth_bytes:%I64d(bytes)", __FUNCTION__, __LINE__, total_transferred_bytes, last_total_transferred_bytes, CSR_ADT._new_total_bandwidth_bytes);

							if (CSR_ADT._last_total_bandwidth_bytes > MAX_BANDWIDTH_BYTES)
							{
								CSR_ADT._last_total_bandwidth_bytes = CSR_ADT._last_total_bandwidth_bytes - MAX_BANDWIDTH_BYTES;
								CSR_ADT._new_total_bandwidth_bytes = CSR_ADT._new_total_bandwidth_bytes - MAX_BANDWIDTH_BYTES;
								log_info(CAPA, "%s, %d, _last_total_bandwidth_bytes:%I64d(bytes), _new_total_bandwidth_bytes:%I64d(bytes)) ", __FUNCTION__, __LINE__, CSR_ADT._last_total_bandwidth_bytes, CSR_ADT._new_total_bandwidth_bytes);

							}
							log_debug(CAPA, "%s, %d, BandWidth(Audio_Paket:%I64d(bytes), Video_Paket:%I64d(bytes)) ", __FUNCTION__, __LINE__, audio_transferred_bytes, video_transferred_bytes);
						}
					}
					else
						log_error(CAPA, "%s, %d, device_id size error size=(%d)", __FUNCTION__, __LINE__, device_id.size());

				}
			}
			//slot alive check response response process
			_controller->sa_alive_check_req(session->uuid(), pid, 0);
			session->update_heart_beat();
			_controller->sa_slot_alive_check_noti(session->uuid());
		}
	};

#if 0
	class client_alive_check_noti_cmd : public abstract_server_controller_cmd
	{
	public:
		client_alive_check_noti_cmd(ic::server_controller * controller)
			: abstract_server_controller_cmd(controller, CMD_CA_ALIVECHECK_NOTI) 
		{}
		virtual ~client_alive_check_noti_cmd(void) 
		{}

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{	
			//cap_log4cplus_logger::make_trace_log("cloud.app.platform.appmanager", "%s(), dst=%s, src=%s, cmd=%d", __FUNCTION__, dst, src, command_id);

			_controller->ca_client_alive_check_noti(session->uuid());
		}
	};


	class slot_alive_check_noti_cmd : public abstract_server_controller_cmd
	{
	public:
		slot_alive_check_noti_cmd(ic::server_controller * controller)
			: abstract_server_controller_cmd(controller, CMD_SA_ALIVECHECK_NOTI) 
		{}
		virtual ~slot_alive_check_noti_cmd(void) 
		{}

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			//cap_log4cplus_logger::make_trace_log("cloud.app.platform.appmanager", "%s(), dst=%s, src=%s, cmd=%d", __FUNCTION__, dst, src, command_id);

			_controller->sa_slot_alive_check_noti(session->uuid());
		}
	};
#endif
};















#endif