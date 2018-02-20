#ifndef _ARODNAP_STREAM_CLIENT_H_
#define _ARODNAP_STREAM_CLIENT_H_

#include "base_client.h"

class arodnap_stream_client
	: public base_client
{
public:
	arodnap_stream_client(int recv_buffer_size);
	virtual ~arodnap_stream_client(void);

	void	on_connect_to_server(void);
	void	on_disconnect_from_server(void);

	void	on_connect(void);
	//void	on_disconnect(void);


	void	on_recv(const char * packet, int packet_size);

private:


private:
	char	_buffer[1024 * 1024 * 2];

};


#endif