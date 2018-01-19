#ifndef _CONTROL_CLIENT_H_
#define _CONTROL_CLIENT_H_

#include <sirius_client_framework.h>
#include <sirius_client_proxy.h>

class CSiriusClientDlg;
class client_controller
	: public sirius::app::client::proxy::handler
{
public:
	client_controller(CSiriusClientDlg * front, bool keepalive, bool tls);
	virtual ~client_controller(void);

	void on_pre_connect(wchar_t * address, int32_t portNumber, bool reconnection);
	void on_post_connect(wchar_t * address, int32_t portNumber, bool reconnection);
	void on_pre_disconnect(void);
	void on_post_disconnect(void);

	void on_pre_create_session(void);
	void on_create_session(void);
	void on_post_create_session(void);

	void on_pre_destroy_session(void);
	void on_destroy_session(void);
	void on_post_destroy_session(void);

	void on_pre_connect_client(int32_t code, wchar_t * msg);
	void on_connect_client(int32_t code, wchar_t * msg);
	void on_post_connect_client(int32_t code, wchar_t * msg);

	void on_pre_disconnect_client(int32_t code);
	void on_disconnect_client(int32_t code);
	void on_post_disconnect_client(int32_t code);

	void on_pre_attendant_info(int32_t code, wchar_t * container_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height);
	void on_post_attendant_info(int32_t code, wchar_t * container_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height);

	void on_open_streaming(wchar_t * container_uuid, int32_t streamer_portnumber, bool reconnection);
	void on_play_streaming(void);
	void on_stop_streaming(void);

	void on_pre_xml(const char * msg, size_t length);
	void on_xml(const char * msg, size_t length);
	void on_post_xml(const char * msg, size_t length);

	void on_pre_error(int32_t error_code);
	void on_error(int32_t error_code);
	void on_post_error(int32_t error_code);

private:
	CSiriusClientDlg *				_front;
	sirius::app::client::proxy *	_controller;
	HMODULE							_hmodule;
	wchar_t							_address[MAX_PATH];
	sirius::library::framework::client::base * _framework;
};

#endif
