#ifndef _ARODNAP_STREAM_CLIENT_H_
#define _ARODNAP_STREAM_CLIENT_H_

#include "base_client.h"

class arodnap_stream_client
	: public base_client
{
public:
#pragma pack(1)
	typedef struct _header_t
	{
		unsigned char		packet_sop;
		unsigned short		packet_seq;
		unsigned short		packet_length;
		unsigned char		contents_type;
		unsigned char		contents_count;
		unsigned long long	adaptive_timestamp;
	} header_t;

	typedef struct _payload_t 
	{
		unsigned long long	timestamp;
		unsigned int		contents_length;
	} payload_t;
#pragma pack()

	typedef struct _payload_info_t
	{
		int		x;
		int		y;
		int		w;
		int		h;
		bool	mask;
		char	format[100];
		char	cmd[100];
	} payload_info_t;

	arodnap_stream_client(int recv_buffer_size);
	virtual ~arodnap_stream_client(void);

	void	on_connect_to_server(void);
	void	on_disconnect_from_server(void);

	void	on_port_info(bool status);
	//void	on_disconnect(void);


	void	on_recv(const char * packet, int packet_size);

private:
	const char *	make_port_info_req_packet(int & packet_size);
	int				parse_payload_info(const char * xml, payload_info_t * pinfo);

private:
	char	_send_buffer[1024 * 1024 * 2];
	char	_recv_buffer[1024 * 1024 * 4];
	int		_recv_buffer_index;
	bool	_bstreaming;
};


#endif