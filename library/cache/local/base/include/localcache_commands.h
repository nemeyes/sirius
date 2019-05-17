#ifndef _LOCALCACHE_COMMANDS_H_
#define _LOCALCACHE_COMMANDS_H_

#include <vector>
#include <stdint.h>

#define CMD_UPLOAD_BEGIN_REQ		1000
#define CMD_UPLOAD_PROCESSING_REQ	1001
#define CMD_UPLOAD_END_REQ			1002
#define CMD_UPLOAD_RES				1003

#define CMD_DOWNLOAD_REQ			1004
#define CMD_DOWNLOAD_BEGIN_RES		1005
#define CMD_DOWNLOAD_PROCESSING_RES	1006
#define CMD_DOWNLOAD_END_RES		1007

#define CMD_FTELL_IND				1008

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				typedef struct _cmd_upload_begin_req_t
				{
					char	sessionid[64];
					char	hash[130];
					int32_t size;
					int32_t width;
					int32_t height;
				} cmd_upload_begin_req_t;

				typedef struct _cmd_upload_processing_req_t
				{
					char	sessionid[64];
					char	hash[130];
					int32_t size;
					int32_t width;
					int32_t height;
					int32_t offset;
				} cmd_upload_processing_req_t;

				typedef struct _cmd_upload_end_req_t
				{
					char	sessionid[64];
					char	hash[130];
					int32_t size;
					int32_t width;
					int32_t height;
					int32_t offset;
				} cmd_upload_end_req_t;

				typedef struct _cmd_upload_res_t
				{
					char	sessionid[64];
					int32_t code;
				} cmd_upload_res_t;

				typedef struct _cmd_download_req_t
				{
					char	sessionid[64];
					char	hash[130];
				} cmd_download_req_t;

				typedef struct _cmd_download_begin_res_t
				{
					char	sessionid[64];
					char	hash[130];
					int32_t size;
				} cmd_download_begin_res_t;

				typedef struct _cmd_download_processing_res_t
				{
					char	sessionid[64];
					int32_t size;
					int32_t offset;
					int32_t reserved;
				} cmd_download_processing_res_t;

				typedef struct _cmd_download_end_res_t
				{
					char	sessionid[64];
					int32_t size;
					int32_t offset;
					int32_t code;
				} cmd_download_end_res_t;

				typedef struct _cmd_ftell_ind_t
				{
					char	hash[130];
					int32_t size;
				} cmd_ftell_ind_t;
			};
		};
	};
};

#endif