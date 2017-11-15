#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <cstdint>
#include <memory>
#include <deque>
#include <sirius_commands.h>
#include <abstract_session.h>

#if defined(WITH_WORKING_AS_SERVER)
# include <rpc.h>
# include <sicp_abstract_server.h>
#else
# include <sicp_abstract_client.h>
#endif

#define SERVER_WORKER_THREAD	80
#define CLIENT_WORKER_THREAD	3

#define SERVER_UUID		"00000000-0000-0000-0000-000000000000"
#define BROADCAST_UUID	"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"
#define UNDEFINED_UUID	"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFE"

#define COMMAND_SIZE sizeof(int)

#define CMD_ERR_CODE_SUCCESS		0
#define CMD_ERR_CODE_FAIL			1

#define CMD_CREATE_SESSION_REQUEST			1001
#define CMD_CREATE_SESSION_RESPONSE			1002
//#define CMD_LEAVE_REQUEST			1031
//#define CMD_LEAVE_RESPONSE		1032
#define CMD_DESTROY_SESSION_INDICATION		1003
#define CMD_KEEPALIVE_REQUEST				1004
#define CMD_KEEPALIVE_RESPONSE				1005

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace sicp
			{
				typedef struct _CMD_PAYLOAD_T
				{
					int32_t	code;
				} CMD_PAYLOAD_T;

				typedef struct _CMD_CREATE_SESSION_RES_T
					: public _CMD_PAYLOAD_T
				{
					char uuid[64];
				} CMD_CREATE_SESSION_RES_T;

				class processor;
				class abstract_command
				{
					friend class sirius::library::net::sicp::base;
				public:
					class tp_worker
						: public std::enable_shared_from_this<sirius::library::net::sicp::abstract_command::tp_worker>
					{
					public:
						tp_worker(void)
							: command(nullptr)
							, version(0)
							, msg_type(0)
							, length(0)
							, msg(nullptr)
							, session(nullptr)
						{
							memset(destination, 0x00, sizeof(destination));
							memset(source, 0x00, sizeof(source));
						}

						~tp_worker(void)
						{
							if (msg)
								free(msg);
							msg = nullptr;
							length = 0;
						}

					public:
						sirius::library::net::sicp::abstract_command * command;
						char		destination[64];
						char		source[64];
						uint8_t		version;
						uint8_t		msg_type;
						int32_t		length;
						char *		msg;
						std::shared_ptr<sirius::library::net::sicp::session> session;
					};

					class tp_worker_scopped_lock
					{
					public:
						tp_worker_scopped_lock(SRWLOCK * srwl)
							: _srwl(srwl)
						{
							::AcquireSRWLockExclusive(_srwl);
						}
						~tp_worker_scopped_lock(void)
						{
							::ReleaseSRWLockExclusive(_srwl);
						}
					private:
						SRWLOCK * _srwl;
					};
					abstract_command(sirius::library::net::sicp::processor * prcsr, int32_t command_id);
					abstract_command(int32_t command_id);
					virtual ~abstract_command(void);


					void set_processor(sirius::library::net::sicp::processor * prcsr);
					sirius::library::net::sicp::processor * get_processor(void);

					const char *	uuid(void);
					int32_t			command_id(void);
					bool			is_running(void);

					void			_execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session);
					static void __stdcall tp_work_callback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work);

				protected:
					void __execute(void);
					virtual void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session) = 0;

					sirius::library::net::sicp::processor * _processor;
					int32_t		_command_id;
					PTP_WORK	_work;
					SRWLOCK		_srwl;
					std::deque<std::shared_ptr<sirius::library::net::sicp::abstract_command::tp_worker>> _tp_ready_workers;
				};

#if defined(WITH_WORKING_AS_SERVER)
				class create_session_req_cmd : public abstract_command
				{
				public:
					create_session_req_cmd(sirius::library::net::sicp::processor * prcsr);
					virtual ~create_session_req_cmd(void);
					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session);
				};
#else
				class create_session_res_cmd : public abstract_command
				{
				public:
					create_session_res_cmd(sirius::library::net::sicp::processor * prcsr);
					virtual ~create_session_res_cmd(void);
					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session);
				};
#endif
				class destroy_session_ind_cmd : public abstract_command
				{
				public:
					destroy_session_ind_cmd(sirius::library::net::sicp::processor * prcsr);
					virtual ~destroy_session_ind_cmd(void);
					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session);
				};



				class keepalive_req_cmd : public abstract_command
				{
				public:
					keepalive_req_cmd(sirius::library::net::sicp::processor * prcsr);
					virtual ~keepalive_req_cmd(void);
					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session);
				};

				class keepalive_res_cmd : public abstract_command
				{
				public:
					keepalive_res_cmd(sirius::library::net::sicp::processor * prcsr);
					virtual ~keepalive_res_cmd(void);
					void execute(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, int32_t length, std::shared_ptr<sirius::library::net::sicp::session> session);
				};
			};
		};
	};
};

#endif