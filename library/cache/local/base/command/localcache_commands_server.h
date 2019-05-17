#ifndef _LOCALCACHE_COMMANDS_SERVER_H_
#define _LOCALCACHE_COMMANDS_SERVER_H_

#include <localcache_command.h>

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				class upload_begin_req
					: public sirius::library::cache::local::abstract_command
				{
				public:
					upload_begin_req(sirius::library::cache::local::localcache_processor * prcsr)
						: sirius::library::cache::local::abstract_command(prcsr, CMD_UPLOAD_BEGIN_REQ)
					{}
					virtual ~upload_begin_req(void)
					{}
					void execute(int32_t command_id, const char * packet, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
					{
						sirius::library::cache::local::cmd_upload_begin_req_t req;
						sirius::library::cache::local::cmd_upload_res_t res;
						int32_t header_size = sizeof(req);
						if (length >= header_size)
						{
							memmove(&req, packet, header_size);
							res.code = _processor->on_upload_begin(req.sessionid, req.hash, req.size, req.width, req.height, packet + header_size, length - header_size);
							if (res.code != sirius::library::cache::local::localcache_processor::err_code_t::success)
							{
								::memmove(res.sessionid, req.sessionid, 64);
								session->send(CMD_UPLOAD_RES, (const char*)&res, sizeof(res));
							}
						}
						else
						{
							res.code = sirius::library::cache::local::localcache_processor::err_code_t::fail;
							session->send(CMD_UPLOAD_RES, (const char*)&res, sizeof(res));
						}
					}
				};

				class upload_processing_req
					: public sirius::library::cache::local::abstract_command
				{
				public:
					upload_processing_req(sirius::library::cache::local::localcache_processor * prcsr)
						: sirius::library::cache::local::abstract_command(prcsr, CMD_UPLOAD_PROCESSING_REQ)
					{}
					virtual ~upload_processing_req(void)
					{}
					void execute(int32_t command_id, const char * packet, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
					{
						sirius::library::cache::local::cmd_upload_processing_req_t req;
						sirius::library::cache::local::cmd_upload_res_t res;
						int32_t header_size = sizeof(req);
						if (length >= header_size)
						{
							memmove(&req, packet, header_size);
							res.code = _processor->on_upload_processing(req.sessionid, req.hash, req.size, req.width, req.height, req.offset, packet + header_size, length - header_size);
							if (res.code != sirius::library::cache::local::localcache_processor::err_code_t::success)
							{
								::memmove(res.sessionid, req.sessionid, 64);
								session->send(CMD_UPLOAD_RES, (const char*)&res, sizeof(res));
							}
						}
						else
						{
							res.code = sirius::library::cache::local::localcache_processor::err_code_t::fail;
							session->send(CMD_UPLOAD_RES, (const char*)&res, sizeof(res));
						}
					}
				};

				class upload_end_req
					: public sirius::library::cache::local::abstract_command
				{
				public:
					upload_end_req(sirius::library::cache::local::localcache_processor * prcsr)
						: sirius::library::cache::local::abstract_command(prcsr, CMD_UPLOAD_END_REQ)
					{}
					virtual ~upload_end_req(void)
					{}
					void execute(int32_t command_id, const char * packet, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
					{
						sirius::library::cache::local::cmd_upload_end_req_t req;
						sirius::library::cache::local::cmd_upload_res_t res;
						int32_t header_size = sizeof(req);
						if (length >= header_size)
						{
							memmove(&req, packet, header_size);
							res.code = _processor->on_upload_end(req.sessionid, req.hash, req.size, req.width, req.height, req.offset, packet + header_size, length - header_size);
							::memmove(res.sessionid, req.sessionid, 64);
							session->send(CMD_UPLOAD_RES, (const char*)&res, sizeof(res));
						}
						else
						{
							res.code = sirius::library::cache::local::localcache_processor::err_code_t::fail;
							session->send(CMD_UPLOAD_RES, (const char*)&res, sizeof(res));
						}
					}
				};

				class download_req
					: public sirius::library::cache::local::abstract_command
				{
				public:
					download_req(sirius::library::cache::local::localcache_processor * prcsr)
						: sirius::library::cache::local::abstract_command(prcsr, CMD_DOWNLOAD_REQ)
					{}
					virtual ~download_req(void)
					{}
					void execute(int32_t command_id, const char * packet, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
					{
						sirius::library::cache::local::cmd_download_req_t req;
						int32_t header_size = sizeof(req);
						if (length == header_size)
						{
							memmove(&req, packet, length);
							_processor->on_download(req.sessionid, req.hash, session);
						}
					}
				};

				class ftell_ind
					: public sirius::library::cache::local::abstract_command
				{
				public:
					ftell_ind(sirius::library::cache::local::localcache_processor * prcsr)
						: sirius::library::cache::local::abstract_command(prcsr, CMD_FTELL_IND)
					{}
					virtual ~ftell_ind(void)
					{}
					void execute(int32_t command_id, const char * packet, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session)
					{
						sirius::library::cache::local::cmd_ftell_ind_t ind;
						int32_t header_size = sizeof(ind);
						if (length == header_size)
						{
							memmove(&ind, packet, length);
							_processor->on_ftell(ind.hash, ind.size);
						}
					}
				};
			};
		};
	};
};















#endif
