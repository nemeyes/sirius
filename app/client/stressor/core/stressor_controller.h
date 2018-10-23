#ifndef _CONTROL_STRESSOR_H_
#define _CONTROL_STRESSOR_H_

#include <sirius_client_framework.h>
#include <sirius_client_proxy.h>
#include "sirius_native_stressor_framework.h"

class CSiriusStressorDlg;
class stressor_controller
	: public sirius::app::client::proxy::handler
{
public:
	stressor_controller(CSiriusStressorDlg * front, int32_t index, bool keepalive, int32_t keepalive_timeout, bool tls);
	virtual ~stressor_controller(void);

	void on_pre_connect(wchar_t * address, int32_t portNumber, bool reconnection, bool keepalive, int32_t keepalive_timeout);
	void on_post_connect(wchar_t * address, int32_t portNumber, bool reconnection, bool keepalive, int32_t keepalive_timeout);
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

	void on_open_streaming(wchar_t * container_uuid, int32_t streamer_portnumber, bool reconnection, bool keepalive, int32_t keepalive_timeout);
	void on_play_streaming(void);
	void on_stop_streaming(void);

	void on_pre_end2end_data(const char * packet, int32_t packet_size);
	void on_end2end_data(const char * packet, int32_t packet_size);
	void on_post_end2end_data(const char * packet, int32_t packet_size);

	void on_pre_error(int32_t error_code);
	void on_error(int32_t error_code);
	void on_post_error(int32_t error_code);

	void on_connect_stream(void);
	void on_disconnect_stream(void);
	void on_recv_stream(void);

	void check_stream_state(void);

	HANDLE							_key_event_thread;
	static unsigned __stdcall		key_event_process_cb(void * param);
	void							key_event_process(void);
	void							close_key_event_thread_wait();

private:
	CSiriusStressorDlg *						_front;
	int32_t										_index;
	int32_t										_recv_stream_count;
	DWORD										_latency;
	sirius::app::client::proxy *				_controller;
	HMODULE										_hmodule;
	wchar_t										_address[MAX_PATH];
	sirius::library::framework::client::base *	_framework;
	bool										_key_event_run;
};
#endif
