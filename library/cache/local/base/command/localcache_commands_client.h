#ifndef _LOCALCACHE_COMMANDS_CLIENT_H_
#define _LOCALCACHE_COMMANDS_CLIENT_H_

#include <localcache_command.h>

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				class upload_res
					: public sirius::library::cache::local::abstract_command
				{
				public:
					upload_res(sirius::library::cache::local::localcache_processor * prcsr)
						: sirius::library::cache::local::abstract_command(prcsr, CMD_UPLOAD_RES)
					{}
					virtual ~upload_res(void)
					{}
					void execute(int32_t command_id, const char * packet, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
					{
						sirius::library::cache::local::cmd_upload_res_t res;
						int32_t header_size = sizeof(res);
						if (length == header_size)
						{
							memmove(&res, packet, length);
							_processor->on_upload(res.sessionid, res.code);
						}
					}
				};

				class download_begin_res
					: public sirius::library::cache::local::abstract_command
				{
				public:
					download_begin_res(sirius::library::cache::local::localcache_processor * prcsr)
						: sirius::library::cache::local::abstract_command(prcsr, CMD_DOWNLOAD_BEGIN_RES)
					{}
					virtual ~download_begin_res(void)
					{}
					void execute(int32_t command_id, const char * packet, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
					{
						sirius::library::cache::local::cmd_download_begin_res_t res;
						int32_t header_size = sizeof(res);
						if (length >= header_size)
						{
							memmove(&res, packet, header_size);
							_processor->on_download_begin(res.sessionid, res.hash, res.size, packet + header_size, length - header_size);
						}
					}
				};

				class download_processing_res
					: public sirius::library::cache::local::abstract_command
				{
				public:
					download_processing_res(sirius::library::cache::local::localcache_processor * prcsr)
						: sirius::library::cache::local::abstract_command(prcsr, CMD_DOWNLOAD_PROCESSING_RES)
					{}
					virtual ~download_processing_res(void)
					{}
					void execute(int32_t command_id, const char * packet, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
					{
						sirius::library::cache::local::cmd_download_processing_res_t res;
						int32_t header_size = sizeof(res);
						if (length >= header_size)
						{
							memmove(&res, packet, header_size);
							_processor->on_download_processing(res.sessionid, res.size, res.offset, packet + header_size, length - header_size);
						}
					}
				};

				class download_end_res
					: public sirius::library::cache::local::abstract_command
				{
				public:
					download_end_res(sirius::library::cache::local::localcache_processor * prcsr)
						: sirius::library::cache::local::abstract_command(prcsr, CMD_DOWNLOAD_END_RES)
					{}
					virtual ~download_end_res(void)
					{}
					void execute(int32_t command_id, const char * packet, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
					{
						sirius::library::cache::local::cmd_download_end_res_t res;
						int32_t header_size = sizeof(res);
						if (length >= header_size)
						{
							memmove(&res, packet, header_size);
							_processor->on_download_end(res.sessionid, res.code, res.size, res.offset, packet + header_size, length - header_size);
						}
					}
				};
			};
		};
	};
};















#endif
