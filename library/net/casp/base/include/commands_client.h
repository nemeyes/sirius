#ifndef _COMMANDS_MEDIA_CLIENT_H_
#define _COMMANDS_MEDIA_CLIENT_H_

#if defined(WITH_BRANDNEW_IPC)

#include <ipc_command.h>
#include <commands_payload.h>
#include "casp_client.h"

#include <cap_auto_lock.h>

namespace amadeus
{
	namespace library
	{
		namespace net
		{
			namespace casp
			{
				class abstract_media_client_cmd : public amadeus::library::net::ipc::abstract_command
				{
				public:
					abstract_media_client_cmd(amadeus::library::net::casp::client::core * processor, int32_t command_id)
						: abstract_command(command_id)
						, _processor(processor) {}
					virtual ~abstract_media_client_cmd(void) {}

				protected:
					amadeus::library::net::casp::client::core * _processor;
				};


				class sc_stream_res_cmd : public abstract_media_client_cmd
				{
				public:
					sc_stream_res_cmd(amadeus::library::net::casp::client::core * processor)
						: amadeus::library::net::casp::abstract_media_client_cmd(processor, CMD_SC_STREAM_RES)
					{};
					virtual ~sc_stream_res_cmd(void) {};
					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<amadeus::library::net::ipc::session> session)
					{
						_processor->av_stream_callback(msg, length);
					};
				};
				//CMD_SC_AUDIO_INFO_NOTI
				class sc_audio_info_noti : public abstract_media_client_cmd
				{
				public:
					sc_audio_info_noti(amadeus::library::net::casp::client::core * processor)
						: abstract_media_client_cmd(processor, CMD_SC_AUDIO_INFO_NOTI)
					{
						::InitializeSRWLock(&_lock);
					}
					virtual ~sc_audio_info_noti(void) {};

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<amadeus::library::net::ipc::session> session)
					{
						cap_exclusive_scoped_lock mutex(&_lock);
						_processor->audio_info_noti_callback(msg, length);
					};

				private:
					SRWLOCK _lock;

				};

				class sc_video_data_cmd : public abstract_media_client_cmd
				{
				public:
					sc_video_data_cmd(amadeus::library::net::casp::client::core * processor)
						:abstract_media_client_cmd(processor, CMD_SC_VIDEO_DATA)
					{
						::InitializeSRWLock(&_lock);
					}
					virtual ~sc_video_data_cmd(void) {};

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<amadeus::library::net::ipc::session> session)
					{
						cap_exclusive_scoped_lock mutex(&_lock);
						_processor->push_video_packet((amadeus::library::net::casp::cmd_stream_data_t*)msg, (uint8_t*)msg + sizeof(amadeus::library::net::casp::cmd_stream_data_t), length);
					};

				private:
					SRWLOCK _lock;
				};

				class sc_audio_data_cmd : public abstract_media_client_cmd
				{
				public:
					sc_audio_data_cmd(amadeus::library::net::casp::client::core * processor)
						:abstract_media_client_cmd(processor, CMD_SC_AUDIO_DATA)
					{
						::InitializeSRWLock(&_lock);
					};
					virtual ~sc_audio_data_cmd(void) {};

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<amadeus::library::net::ipc::session> session)
					{
						cap_exclusive_scoped_lock mutex(&_lock);
						_processor->push_audio_packet((amadeus::library::net::casp::cmd_stream_data_t*)msg, (uint8_t*)msg + sizeof(amadeus::library::net::casp::cmd_stream_data_t), length);
					};
				private:
					SRWLOCK _lock;
				};
			};
		};
	};
};

#else

#include <command.h>
#include <commands_payload.h>
#include "cap_casp_client.h"

#include <cap_auto_lock.h>

namespace ic
{
	class abstract_media_client_cmd : public ic::abstract_command
	{
	public:
		abstract_media_client_cmd(casp_client * processor, int32_t command_id)
			: abstract_command(command_id)
			, _processor(processor) {}
		virtual ~abstract_media_client_cmd(void) {}

	protected:
		casp_client * _processor;
	};


	class sc_stream_res_cmd : public ic::abstract_media_client_cmd
	{
	public:
		sc_stream_res_cmd(casp_client *processor)
			:ic::abstract_media_client_cmd(processor, CMD_SC_STREAM_RES)
		{};
		virtual ~sc_stream_res_cmd(void) {};
		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			_processor->av_stream_callback(msg, length);
			//_processor->set_state(cap_casp_client::state_t::streaming);
		};
	};

	class sc_video_data_cmd : public ic::abstract_media_client_cmd
	{
	public:
		sc_video_data_cmd(casp_client * processor)
			:abstract_media_client_cmd(processor, CMD_SC_VIDEO_DATA)
		{
			::InitializeSRWLock(&_lock);
		}
		virtual ~sc_video_data_cmd(void) {};

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			cap_exclusive_scoped_lock mutex(&_lock);
			_processor->push_video_packet((ic::cmd_stream_data_t*)msg, (uint8_t*)msg + sizeof(ic::cmd_stream_data_t), length);
		};

	private:
		SRWLOCK _lock;
	};

	class sc_audio_data_cmd : public ic::abstract_media_client_cmd
	{
	public:
		sc_audio_data_cmd(casp_client * processor)
			:abstract_media_client_cmd(processor, CMD_SC_AUDIO_DATA)
		{
			::InitializeSRWLock(&_lock);
		};
		virtual ~sc_audio_data_cmd(void) {};

		void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, uint8_t msg_type, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			cap_exclusive_scoped_lock mutex(&_lock);
			_processor->push_audio_packet((ic::cmd_stream_data_t*)msg, (uint8_t*)msg + sizeof(ic::cmd_stream_data_t), length);
		};
	private:
		SRWLOCK _lock;
	};
};

#endif

#endif