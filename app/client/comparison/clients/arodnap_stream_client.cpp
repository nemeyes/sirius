#include <winsock2.h>
#include <windows.h>
#include "arodnap_stream_client.h"

arodnap_stream_client::arodnap_stream_client(int recv_buffer_size)
	: base_client(recv_buffer_size)
{

}

arodnap_stream_client::~arodnap_stream_client(void)
{

}

void arodnap_stream_client::on_connect_to_server(void)
{
	int packet_size = 0;
	const char * packet = make_connect_req_packet(packet_size);
	send(packet, packet_size);
}

void arodnap_stream_client::on_disconnect_from_server(void)
{

}

void arodnap_stream_client::on_connect(void)
{
	int packet_size = 0;
	const char * packet = make_stbinfo_packet(packet_size);
	send(packet, packet_size);
}

void arodnap_stream_client::on_recv(const char * packet, int packet_size)
{
	char * real_packet = (char*)packet + sizeof(int);
	int real_packet_size = packet_size - sizeof(int);

	int command = 0;
	memmove(&command, packet, sizeof(int));
	command = ntohl(command);
	switch (command)
	{
	case 2001: //connect success;
		on_connect();
		break;

	case 2002: //connect fail;
		break;
	}


}

/*
0000   00 00 07 d0 00 00 00 00 00 00 00 00 00 00 11 40
0010   00 00 00 1c 38 44 42 34 46 42 43 35 2d 38 33 43
0020   38 2d 34 36 31 39 2d 31 36 30 31 33 33 36 35 33
0030   00 00 00 b8 3c 3f 78 6d 6c 20 76 65 72 73 69 6f
0040   6e 3d 27 31 2e 30 27 20 3f 3e 3c 49 4e 54 45 52
0050   46 41 43 45 3e 3c 43 4f 4d 4d 41 4e 44 3e 72 65
0060   71 75 65 73 74 3c 2f 43 4f 4d 4d 41 4e 44 3e 3c
0070   47 52 4f 55 50 3e 42 59 50 41 53 53 3c 2f 47 52
0080   4f 55 50 3e 3c 47 54 59 50 45 3e 4d 45 4e 55 2d
0090   4e 41 56 49 47 41 54 49 4f 4e 2d 43 53 3c 2f 47
00a0   54 59 50 45 3e 3c 44 41 54 41 3e 3c 6d 65 6e 75
00b0   54 79 70 65 3e 68 6f 6d 65 43 56 3c 2f 6d 65 6e
00c0   75 54 79 70 65 3e 3c 65 78 74 49 6e 66 6f 3e 3c
00d0   2f 65 78 74 49 6e 66 6f 3e 3c 2f 44 41 54 41 3e
00e0   3c 2f 49 4e 54 45 52 46 41 43 45 3e
*/
const char * arodnap_stream_client::make_connect_req_packet(int & packet_size)
{
	int		command = 2000;	//00 00 07 d0
	int		video_port = 0; //00 00 00 00
	int		audio_port = 0; //00 00 00 00
	int		udp_packet_size = 0; //00 00 11 40
	int		stb_id_length = 0; //00 00 00 1c -> 28				//4
	char	stb_id[] = "8DB4FBC5-83C8-4619-160133653";		//28
	int		history_length = 0; //00 00 00 b8 -> 184				//4
	char	history_xml[] = "<?xml version='1.0' ?><INTERFACE><COMMAND>request</COMMAND><GROUP>BYPASS</GROUP><GTYPE>MENU-NAVIGATION-CS</GTYPE><DATA><menuType>homeCV</menuType><extInfo></extInfo></DATA></INTERFACE>";

	udp_packet_size = 4416;
	stb_id_length = strlen(stb_id);
	history_length = strlen(history_xml);

	/*
	packet_size		= sizeof(command);
	packet_size		+= sizeof(video_port);
	packet_size		+= sizeof(audio_port);
	packet_size		+= sizeof(udp_packet_size);
	packet_size		+= sizeof(stb_id_length);
	packet_size		+= stb_id_length;
	packet_size		+= sizeof(history_length);
	packet_size		+= history_length;
	*/

	int command_nl = htonl(command);
	int video_port_nl = htonl(video_port);
	int audio_port_nl = htonl(audio_port);
	int udp_packet_size_nl = htonl(udp_packet_size);
	int stb_id_length_nl = htonl(stb_id_length);
	int history_length_nl = htonl(history_length);

	memmove(_buffer, &command_nl, sizeof(command));
	packet_size = sizeof(command);
	memmove(_buffer + packet_size, &video_port_nl, sizeof(video_port));
	packet_size += sizeof(video_port);
	memmove(_buffer + packet_size, &audio_port_nl, sizeof(audio_port));
	packet_size += sizeof(audio_port);
	memmove(_buffer + packet_size, &udp_packet_size_nl, sizeof(udp_packet_size));
	packet_size += sizeof(udp_packet_size);
	memmove(_buffer + packet_size, &stb_id_length_nl, sizeof(stb_id_length));
	packet_size += sizeof(stb_id_length);
	memmove(_buffer + packet_size, stb_id, stb_id_length);
	packet_size += stb_id_length;
	memmove(_buffer + packet_size, &history_length_nl, sizeof(history_length));
	packet_size += sizeof(history_length);
	memmove(_buffer + packet_size, history_xml, history_length);
	packet_size += history_length;


	return _buffer;
	//................r<?xml version='1.0' ?><INTERFACE><COMMAND>request</COMMAND><GROUP>CJ_VOD</GROUP><GTYPE>MOVE</GTYPE><DATA><historyList><history><menuId>102</menuId><contextId></contextId><categoryId></categoryId><pageIndex></pageIndex></history><history><menuId>23</menuId><contextId></contextId><categoryId></categoryId><pageIndex></pageIndex></history></historyList></DATA></INTERFACE>

}

/*
000000EC  00 00 0b c2 00 00 01 72  3c 3f 78 6d 6c 20 76 65   .......r <?xml ve
000000FC  72 73 69 6f 6e 3d 27 31  2e 30 27 20 3f 3e 3c 49   rsion='1 .0' ?><I
0000010C  4e 54 45 52 46 41 43 45  3e 3c 43 4f 4d 4d 41 4e   NTERFACE ><COMMAN
0000011C  44 3e 72 65 71 75 65 73  74 3c 2f 43 4f 4d 4d 41   D>reques t</COMMA
0000012C  4e 44 3e 3c 47 52 4f 55  50 3e 43 4a 5f 56 4f 44   ND><GROU P>CJ_VOD
0000013C  3c 2f 47 52 4f 55 50 3e  3c 47 54 59 50 45 3e 4d   </GROUP> <GTYPE>M
0000014C  4f 56 45 3c 2f 47 54 59  50 45 3e 3c 44 41 54 41   OVE</GTY PE><DATA
0000015C  3e 3c 68 69 73 74 6f 72  79 4c 69 73 74 3e 3c 68   ><histor yList><h
0000016C  69 73 74 6f 72 79 3e 3c  6d 65 6e 75 49 64 3e 31   istory>< menuId>1
0000017C  30 32 3c 2f 6d 65 6e 75  49 64 3e 3c 63 6f 6e 74   02</menu Id><cont
0000018C  65 78 74 49 64 3e 3c 2f  63 6f 6e 74 65 78 74 49   extId></ contextI
0000019C  64 3e 3c 63 61 74 65 67  6f 72 79 49 64 3e 3c 2f   d><categ oryId></
000001AC  63 61 74 65 67 6f 72 79  49 64 3e 3c 70 61 67 65   category Id><page
000001BC  49 6e 64 65 78 3e 3c 2f  70 61 67 65 49 6e 64 65   Index></ pageInde
000001CC  78 3e 3c 2f 68 69 73 74  6f 72 79 3e 3c 68 69 73   x></hist ory><his
000001DC  74 6f 72 79 3e 3c 6d 65  6e 75 49 64 3e 32 33 3c   tory><me nuId>23<
000001EC  2f 6d 65 6e 75 49 64 3e  3c 63 6f 6e 74 65 78 74   /menuId> <context
000001FC  49 64 3e 3c 2f 63 6f 6e  74 65 78 74 49 64 3e 3c   Id></con textId><
0000020C  63 61 74 65 67 6f 72 79  49 64 3e 3c 2f 63 61 74   category Id></cat
0000021C  65 67 6f 72 79 49 64 3e  3c 70 61 67 65 49 6e 64   egoryId> <pageInd
0000022C  65 78 3e 3c 2f 70 61 67  65 49 6e 64 65 78 3e 3c   ex></pag eIndex><
0000023C  2f 68 69 73 74 6f 72 79  3e 3c 2f 68 69 73 74 6f   /history ></histo
0000024C  72 79 4c 69 73 74 3e 3c  2f 44 41 54 41 3e 3c 2f   ryList>< /DATA></
0000025C  49 4e 54 45 52 46 41 43  45 3e                     INTERFAC E>
*/
const char * arodnap_stream_client::make_stbinfo_packet(int & packet_size)
{
	int command = 3010;
	char history[] = "<?xml version='1.0' ?><INTERFACE><COMMAND>request</COMMAND><GROUP>CJ_VOD</GROUP><GTYPE>MOVE</GTYPE><DATA><historyList><history><menuId>102</menuId><contextId></contextId><categoryId></categoryId><pageIndex></pageIndex></history><history><menuId>23</menuId><contextId></contextId><categoryId></categoryId><pageIndex></pageIndex></history></historyList></DATA></INTERFACE>";
	int size = strlen(history);

	int command_nl = htonl(command);
	int size_nl = htonl(size);

	memmove(_buffer, &command_nl, sizeof(command));
	packet_size = sizeof(command);
	memmove(_buffer + packet_size, &size_nl, sizeof(size));
	packet_size += sizeof(size);
	memmove(_buffer + packet_size, history, size);
	packet_size += size;

	return _buffer;
}
