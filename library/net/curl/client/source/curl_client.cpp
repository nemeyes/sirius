#include "curl_client.h"
#include <memory.h>

typedef size_t(*CURL_WRITEFUNCTION_PTR)(void*, size_t, size_t, void*);

sirius::library::net::curl::client::core::core(int sending_timeout)
	:_list(NULL)
{
	memset(_url, 0, sizeof(_url));
	_response = 123;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	_core = curl_easy_init();
	if (sending_timeout > 0)
	{
		_sending_timeout = sending_timeout;
		set_method(sirius::library::net::curl::client::HTTP_METHOD_GET);
	}
	else
	{
		set_method(sirius::library::net::curl::client::HTTP_METHOD_POST);
	}

	if (_core)
	{
		curl_easy_setopt(_core, CURLOPT_WRITEFUNCTION, sirius::library::net::curl::client::core::response_callback);
		curl_easy_setopt(_core, CURLOPT_WRITEDATA, this);
	}
}

sirius::library::net::curl::client::core::~core(void)
{
	curl_slist_free_all(_list);
	curl_easy_cleanup(_core);
	curl_global_cleanup();

}

bool sirius::library::net::curl::client::core::set_url(char* url, int len)
{
	memset(_url, 0, sizeof(_url));
	memcpy(_url, url, len);
	curl_easy_setopt(_core, CURLOPT_URL, url);
	return true;
}

void sirius::library::net::curl::client::core::set_method(sirius::library::net::curl::client::HTTP_METHOD_T method)
{
	_method = method;
	switch (method)
	{
	case sirius::library::net::curl::client::HTTP_METHOD_POST:
	{
		_list = curl_slist_append(_list, "Content-Type: text/xml");
		curl_easy_setopt(_core, CURLOPT_HTTPHEADER, _list);
		curl_easy_setopt(_core, CURLOPT_POST, 1);
	}
	break;
	case sirius::library::net::curl::client::HTTP_METHOD_GET:
	{
		//curl_slist_append(_list, "Content-Type: text/xml");
		curl_easy_setopt(_core, CURLOPT_HTTPGET, 1);
	}
	break;
	}
}

bool sirius::library::net::curl::client::core::set_post_data(char* data)
{
	if (_method == sirius::library::net::curl::client::HTTP_METHOD_POST)
	{
		curl_easy_setopt(_core, CURLOPT_POST, 1);
		curl_easy_setopt(_core, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(_core, CURLOPT_POSTFIELDSIZE, strlen(data));
		curl_easy_setopt(_core, CURLOPT_CONNECTTIMEOUT, 1);
		curl_easy_setopt(_core, CURLOPT_TIMEOUT, 1);
		return true;
	}
	return false;
}

void sirius::library::net::curl::client::core::set_get_data(char* parameters, int stat_type)
{
	//if (_method == sirius::library::net::curl::client::HTTP_METHOD_POST)
		set_method(sirius::library::net::curl::client::HTTP_METHOD_GET);
		curl_easy_setopt(_core, CURLOPT_TIMEOUT_MS, _sending_timeout);

	curl_easy_setopt(_core, CURLOPT_URL, parameters);
	//curl_easy_setopt(_core, CURLOPT_TCP_KEEPALIVE, 1);
	//curl_easy_setopt(_core, CURLOPT_TCP_KEEPIDLE, 10);
	//curl_easy_setopt(_core, CURLOPT_TCP_KEEPINTVL, 5);
	//curl_easy_setopt(_core, CURLOPT_NOSIGNAL, 1);
}

bool sirius::library::net::curl::client::core::send()
{
	CURLcode res;

	res = curl_easy_perform(_core);
	if (res != CURLE_OK) 
	{
		err = res;
		return false;
	}

	return true;
}

void sirius::library::net::curl::client::core::set_callback_function(void(*func) (int message_id, int status_code, void* data, int length))
{
	_callback = func;
}

size_t sirius::library::net::curl::client::core::response_callback(void *data, size_t size, size_t count, void * curl)
{
	//((string*)rsp)->append((char*)data,0, size * count);
	static_cast<sirius::library::net::curl::client::core*>(curl)->_callback(size, count, data, size * count);
	return size * count;
}
