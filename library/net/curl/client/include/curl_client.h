#ifndef _CURL_CORE_H_
#define _CURL_CORE_H_

#include <string>
using namespace std;

#include <curl.h>
#include "interface.h"

#include "sirius_curl_client.h"

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace curl
			{
				class client::core
				{
				public:
					core(int);
					~core(void);

					bool	set_url(char* url, int len);
					void	set_method(sirius::library::net::curl::client::HTTP_METHOD_T method);
					void	set_callback_function(void(*func) (int message_id, int status_code, void* data, int length));
					bool	set_post_data(char* data);
					void	set_get_data(char* parameters, int stat_type);
					int		get_send_err() { return err; }
					bool	send();
					int		get_sending_timeout() { return _sending_timeout; }

				private:
					CURL*	_core;
					struct curl_slist * _list;
					string _response;
					sirius::library::net::curl::client::HTTP_METHOD_T _method;
					char _url[100];
					int err;
					int _sending_timeout;

					//callback
					static size_t response_callback(void *data, size_t size, size_t count, void *rsp);
					void(*_callback) (int message_id, int status_code, void* data, int length);
				};
			};
		};
	};
};

#endif