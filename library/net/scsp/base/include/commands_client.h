#ifndef _COMMANDS_MEDIA_CLIENT_H_
#define _COMMANDS_MEDIA_CLIENT_H_

#include <sicp_command.h>
#include <commands_payload.h>
#include "scsp_client.h"

#include <sirius_locks.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace scsp
			{
				class client_cmd  : public sirius::library::net::sicp::abstract_command
				{
				public:
					client_cmd(sirius::library::net::scsp::client::core * processor, int32_t command_id)
						: abstract_command(command_id)
						, _processor(processor) {}
					virtual ~client_cmd(void) {}

				protected:
					sirius::library::net::scsp::client::core * _processor;
				};


				class play_res_cmd : public client_cmd
				{
				public:
					play_res_cmd(sirius::library::net::scsp::client::core * processor)
						: sirius::library::net::scsp::client_cmd(processor, CMD_PLAY_RES)
					{};
					virtual ~play_res_cmd(void) {};
					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						_processor->av_stream_callback(msg, length);
					};
				};

				class video_stream_data_cmd : public client_cmd
				{
				public:
					video_stream_data_cmd(sirius::library::net::scsp::client::core * processor)
						: sirius::library::net::scsp::client_cmd(processor, CMD_VIDEO_STREAM_DATA)
					{
						::InitializeSRWLock(&_lock);
					}
					virtual ~video_stream_data_cmd(void) {};

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						sirius::exclusive_scopedlock mutex(&_lock);
						_processor->push_video_packet((sirius::library::net::scsp::cmd_stream_data_t*)msg, (uint8_t*)msg + sizeof(sirius::library::net::scsp::cmd_stream_data_t), length);
					};

				private:
					SRWLOCK _lock;
				};
			};
		};
	};
};

#endif