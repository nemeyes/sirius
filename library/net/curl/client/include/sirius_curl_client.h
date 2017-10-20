#ifndef _SIRIUS_CURL_CLIENT_H_
#define _SIRIUS_CURL_CLIENT_H_

#include "interface.h"

#if defined(EXPORT_CURL_LIB)
#define EXP_CURL_CLASS __declspec(dllexport)
#else
#define EXP_CURL_CLASS __declspec(dllimport)
#endif


namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace curl
			{
				class EXP_CURL_CLASS client
				{
				public:
					class core;
				public:
					typedef enum _HTTP_METHOD_T
					{
						HTTP_METHOD_POST,
						HTTP_METHOD_GET,
					} HTTP_METHOD_T;

					typedef enum _STAT_TYPE_T
					{
						MIN_STAT,
						ETC_STAT
					} STAT_TYPE_T;

					client(int);
					~client(void);

					bool	set_url(char* url, int len);
					void	set_method(HTTP_METHOD_T method);
					void	set_callback_function(void(*func) (int message_id, int status_code, void* data, int length));
					bool	set_post_data(char* data);
					void	set_get_data(char* parameters, int stat_type);

					bool	send(void);
					int		get_send_err();
					int		get_sending_timeout();

				private:
					sirius::library::net::curl::client::core * _core;
				};
			};
		};
	};
};


#endif