#ifndef _ARODNAP_CONTROL_CLIENT_H_
#define _ARODNAP_CONTROL_CLIENT_H_

#include "base_client.h"

class arodnap_control_client
	: public base_client
{
public:
	arodnap_control_client(int recv_buffer_size);
	virtual ~arodnap_control_client(void);

	void	on_connect_to_server(void);
	void	on_disconnect_from_server(void);

	void	on_connect(void);
	//void	on_disconnect(void);


	void	on_recv(const char * packet, int packet_size);

private:
	const char * make_connect_req_packet(int & packet_size);
	const char * make_stbinfo_packet(int & packet_size);


private:
	char	_buffer[1024 * 1024 * 2];

};


#endif