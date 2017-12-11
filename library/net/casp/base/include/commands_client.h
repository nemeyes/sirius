#ifndef _COMMANDS_MEDIA_CLIENT_H_
#define _COMMANDS_MEDIA_CLIENT_H_

#include <sicp_command.h>
#include <commands_payload.h>
#include "casp_client.h"

#include <sirius_locks.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace casp
			{
				class abstract_media_client_cmd : public sirius::library::net::sicp::abstract_command
				{
				public:
					abstract_media_client_cmd(sirius::library::net::casp::client::core * processor, int32_t command_id)
						: abstract_command(command_id)
						, _processor(processor) {}
					virtual ~abstract_media_client_cmd(void) {}

				protected:
					sirius::library::net::casp::client::core * _processor;
				};


				class play_res_cmd : public  abstract_media_client_cmd
				{
				public:
					play_res_cmd(sirius::library::net::casp::client::core *processor)
						:sirius::library::net::casp::abstract_media_client_cmd(processor, CMD_PLAY_RES)
					{};
					virtual ~play_res_cmd(void) {};
					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						_processor->av_stream_callback(msg, length);
					};
				};

				class video_stream_data_cmd : public abstract_media_client_cmd
				{
				public:
					video_stream_data_cmd(sirius::library::net::casp::client::core * processor)
						:sirius::library::net::casp::abstract_media_client_cmd(processor, CMD_VIDEO_STREAM_DATA)
					{
						::InitializeSRWLock(&_lock);
					}
					virtual ~video_stream_data_cmd(void) {};

					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
					{
						sirius::exclusive_scopedlock mutex(&_lock);

						uint8_t * packet = (uint8_t*)msg;
						_processor->push_video_packet((sirius::library::net::casp::cmd_stream_data_t*)msg, packet, length);
					};

				private:
					SRWLOCK _lock;
				};
			};
		};
	};
};

#endif