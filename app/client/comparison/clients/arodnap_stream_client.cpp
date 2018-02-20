#include <winsock2.h>
#include <windows.h>
#include "arodnap_stream_client.h"
#include <tinyxml.h>

arodnap_stream_client::arodnap_stream_client(int recv_buffer_size)
	: base_client(recv_buffer_size)
	, _bstreaming(false)
	, _recv_buffer_index(0)
{

}

arodnap_stream_client::~arodnap_stream_client(void)
{

}

void arodnap_stream_client::on_connect_to_server(void)
{
	int packet_size = 0;
	const char * packet = make_port_info_req_packet(packet_size);
	send(packet, packet_size);
}

void arodnap_stream_client::on_disconnect_from_server(void)
{

}

void arodnap_stream_client::on_port_info(bool status)
{
	if (status)
		_bstreaming = true;
	else
		_bstreaming = false;
}

void arodnap_stream_client::on_recv(const char * packet, int packet_size)
{
	if (packet && packet_size > 0)
	{
		if (!_bstreaming)
		{
			char * real_packet = (char*)packet + sizeof(int);
			int real_packet_size = packet_size - sizeof(int);

			int command = 0;
			memmove(&command, packet, sizeof(int));
			command = ntohl(command);
			switch (command)
			{
			case 2021: //step1 success;
				on_port_info(true);
				break;

			case 2022: //step1 fail;
				on_port_info(false);
				break;
			}
		}
		else
		{
/*
0000   f2 01 00 86 e1 42 14 00 00 00 00 00 00 00 00 e1  .....B..........
0010   74 d6 5b 00 00 00 00 fa 5d 00 00 00 00 00 c1 3c  t.[.....]......<
0020   3f 78 6d 6c 20 76 65 72 73 69 6f 6e 3d 22 31 2e  ?xml version="1.
0030   30 22 3f 3e 0a 3c 49 4e 46 4f 3e 0a 09 3c 69 6d  0"?>.<INFO>..<im
0040   67 49 6e 66 6f 20 76 65 72 3d 22 32 2e 30 22 2f  gInfo ver="2.0"/
0050   3e 0a 09 3c 43 75 74 54 79 70 65 20 74 79 70 65  >..<CutType type
0060   3d 22 49 6d 67 22 2f 3e 0a 09 3c 52 45 43 54 20  ="Img"/>..<RECT
0070   58 3d 22 30 22 20 59 3d 22 30 22 20 57 49 44 54  X="0" Y="0" WIDT
0080   48 3d 22 32 35 36 22 20 48 45 49 47 48 54 3d 22  H="256" HEIGHT="
0090   31 38 30 22 2f 3e 0a 09 3c 4d 41 53 4b 20 66 6c  180"/>..<MASK fl
00a0   61 67 3d 22 44 49 53 41 42 4c 45 22 2f 3e 0a 09  ag="DISABLE"/>..
00b0   3c 54 59 50 45 20 66 6f 72 6d 61 74 3d 22 50 4e  <TYPE format="PN
00c0   47 22 2f 3e 0a 09 3c 43 4d 44 20 74 79 70 65 3d  G"/>..<CMD type=
00d0   22 4e 45 57 22 2f 3e 0a 3c 2f 49 4e 46 4f 3e 0a  "NEW"/>.</INFO>.
00e0   00 00 5d 31 89 50 4e 47 0d 0a 1a 0a 00 00 00 0d  ..]1.PNG........
00f0   49 48 44 52 00 00 01 00 00 00 00 b4 08 03 00 00  IHDR............
0100   00 4c 80 02 83 00 00 00 04 67 41 4d 41 00 00 b1  .L.......gAMA...
0110   8f 0b fc 61 05 00 00 00 01 73 52 47 42 00 ae ce  ...a.....sRGB...
0120   1c e9 00 00 03 00 50 4c 54 45 4d 4d 4d 13 1b 33  ......PLTEMMM..3
0130   0b 14 27 0a 13 24 0a 13 23 0f 19 31 12 1b 33 0f  ..'..$..#..1..3.
0140   18 30 11 1a 32 10 19 31 10 1a 32 09 11 1f 0a 12  .0..2..1..2.....
0150   22 4c 4c 4d 14 1d 34 0e 17 2f 0e 17 2e 0b 14 26  "LLM..4../.....&
0160   08 0f 1c 0c 15 28 0c 15 29 15 1d 34 08 0f 1b 09  .....(..)..4....
0170   10 1e 09 11 20 13 1c 34 13 1c 33 0e 17 2d 0a 13  .... ..4..3..-..
0180   25 0d 16 2c 0f 19 30 07 0d 18 0a 14 28 0d 17 2c  %..,..0.....(..,
0190   0a 12 21 0b 15 28 12 1a 32 0c 16 2a 0d 16 2b 12  ..!..(..2..*..+.
01a0   1b 32 0f 18 2f 16 1e 35 1b 23 38 14 1c 34 17 1f  .2../..5.#8..4..
01b0   35 0b 13 25 1e 26 3a 0c 16 2b 11 1b 32 07 0d 19  5..%.&:..+..2...
01c0   07 0c 17 0e 18 30 0d 17 2d 19 21 37 08 0e 1a 15  .....0..-.!7....
01d0   1e 35 0e 18 2f 18 20 36 09 12 23 0c 15 2a 17 1f  .5../. 6..#..*..
01e0   36 16 1f 35 17 20 36 0d 16 2d 1f 28 3b 11 1b 33  6..5. 6..-.(;..3
01f0   08 10 1f 07 0e 1a 0d 17 2e 1c 24 39 14 1c 33 1f  ..........$9..3.
0200   27 3b 09 12 21 18 21 37 10 1a 31 1d 26 3a 11 19  ';..!.!7..1.&:..
0210   31 0b 15 29 0b 13 24 1a 23 38 06 0b 14 15 1e 34  1..)..$.#8.....4
0220   0f 18 31 09 10 1d 0e 17 2c 0b 13 26 0d 17 30 1d  ..1.....,..&..0.
0230   25 39 0c 16 2c 08 0e 1b 0a 14 25 08 0f 1d 0b 14  %9..,.....%.....
0240   25 07 0d 19 1b 24 39 05 09 11 0c 14 27 12 1c 33  %....$9.....'..3
0250   13 1d 34 0a 11 20 16 1e 34 1e 27 3b 21 29 3c 1c  ..4.. ..4.';!)<.
0260   25 39 0d 16 2a 05 0a 13 06 0c 17 08 10 1e 1d 25  %9..*..........%
0270   3a 06 0b 16 08 10 1d 23 2c 3e 0c 15 2b 19 22 38  :......#,>..+."8
0280   23 2b 3d 20 29 3c 22 2a 3c 09 12 22 09 0f 1c 0c  #+= )<"*<.."....
0290   16 29 0f 19 30 11 19 31 49 4a 4c 1e 27 3a 19 21  .)..0..1IJL.':.!
02a0   37 0f 17 2f 1a 22 38 20 28 3c 0e 18 2e 13 1b 32  7../."8 (<.....2
02b0   0d 16 2b 0d 17 2f 28 30 40 26 2e 3f 0f 17 30 04  ..+../(0@&.?..0.
02c0   07 0f 09 0f 1d 0d 16 2e 0a 13 26 1a 22 37 27 2f  ..........&."7'/
02d0   3f 16 1d 34 10 18 31 4c 4d 4d 4a 4b 4c 0c 14 28  ?..4..1LMMJKL..(
02e0   14 1e 34 06 0c 16 09 11 21 45 47 4b 17 20 36 0a  ..4.....!EGK. 6.
02f0   13 22 1f 28 3c 20 28 3b 10 19 32 4b 4b 4d 10 18  .".(< (;..2KKM..
0300   30 1f 27 3a 0f 18 2e 42 45 49 16 1f 36 06 0a 14  0.':...BEI..6...
0310   0b 15 27 2e 33 41 48 49 4c 0f 17 2e 11 19 32 25  ..'.3AHIL.....2%
0320   2d 3d 14 1b 33 17 1e 35 24 2c 3d 10 1b 32 0d 15  -=..3..5$,=..2..
0330   29 0a 11 1f 15 1c 34 23 2a 3c 0b 13 26 0b 14 2a  ).....4#*<..&..*
0340   1c 25 3a 40 43 49 07 0c 18 2c 32 41 0a 12 20 0a  .%:@CI...,2A.. .
0350   11 21 07 0e 1b 0a 14 27 3f 42 48 25 2d 3f 38 3c  .!.....'?BH%-?8<
0360   46 21 2a 3c 18 1f 35 0c 15 2d 31 37 43 35 3a 45  F!*<..5..-17C5:E
0370   2e 35 42 21 2a 3d 0f 19 32 21 28 3b 47 48 4b 2b  .5B!*=..2!(;GHK+
0380   31 40 1f 26 39 4d 4c 4d 44 46 4a 22 2b 3d 2a 31  1@.&9MLMDFJ"+=*1
0390   40 1b 24 38 15 1f 35 09 12 20 0b 13 23 29 30 40  @.$8..5.. ..#)0@
03a0   2f 36 42 29 30 40 2b 32 41 07 0b 16 07 0f 1b 19  /6B)0@+2A.......
03b0   20 36 27 2e 3f 3b 3f 47 30 36 42 34 39 44 3e 41   6'.?;?G06B49D>A
03c0   48 32 38 43 27 2e 3d 09 10 1e 37 3b 45 2a 32 41  H28C'.=...7;E*2A
03d0   13 1a 32 45 47 4a 2c 34 42 0d 15 2b 3a 3e 47 1e  ..2EGJ,4B..+:>G.
03e0   25 39 1c 23 38 1b 23 39 28 2f 3f 3c 40 47 1c 24  %9.#8.#9(/?<@G.$
03f0   38 07 0d 17 0e 19 2f 0b 14 29 39 3d 45 33 39 45  8...../..)9=E39E
0400   18 22 37 0c 14 28 0e 16 2c 0d 15 2c 11 18 31 0e  ."7..(..,..,..1.
0410   18 2d 0c 15 2e 27 2d 3d 09 12 27 30 35 41 1b 23  .-...'-=..'05A.#
0420   36 0a 12 24 12 1b 30 0d 18 30 33 ad 8b e2 00 00  6..$..0..03.....
0430   20 00 49 44 41 54 78 01 c4 bd 09 70 93 e7 b9 36   .IDATx....p...6
0440   cc ab e5 5d 24 a1 7d df ac 7d 97 25 db 8a 25 59  ...]$.}..}.%..%Y
0450   d2 08 af d8 ca 67 1b db b2 b1 b1 87 78 8b 71 30  .....g......x.q0
0460   36 86 62 1b a7 32 e0 01 7c 00 03 cd 04 8e 38 50  6.b..2..|.....8P
0470   a0 04 c2 9a 00 d3 13 c2 92 a4 3d 24 29 49 26 4d  ..........=$)I&M
0480   0a 53 9a a5 69 9b 09 29 69 d2 2c 4d 8b 43 fb f7  .S..i..)i.,M.C..
0490   ef 39 df fc d7 cb 39 df fc 67 69 13 68 9b f9 04  .9....9..gi.h...
04a0   c4 c1 96 8c 9f fb 79 9e 7b b9 ae eb be 35 6f de  ......y.{....5o.
04b0   ff 78 cc ff e9 e6 7f 8a 4f 4e 52 b7 5d 6b 69 bf  .x......ONR.]ki.
04c0   2a 10 ef af 71 d4 10 4d de 78 f5 2b 9f ae f8 e6  *...q..M.x.+....
04d0   81 2b 85 eb 76 ec c8 05 4e e6 e2 fe 26 15 a9 13  .+..v...N...&...
04e0   bb 69 3d 41 3a e8 c9 49 86 de 49 93 62 46 4f ea  .i=A:..I..I.bFO.
04f0   c9 30 ed e8 a1 a7 c2 fb a7 d6 52 94 79 3f 43 fa  .0........R.y?C.
0500   a7 73 df 58 37 bb 77 f5 de d5 cb f2 62 46 72 f4  .s.X7.w.....bFr.
0510   a8 61 ae 7b 22 d8 60 eb 3b 7d 5a 5d 54 54 5a fc  .a.{".`.;}Z]TTZ.
0520   c6 a2 37 1e 2a 7e ac b8 f8 8d 37 8a 0b 0a 0a 4a  ..7.*~....7....J
0530   e5 1d 1d ea 0e 75 e9 ae 0b 26 86 0e 1f 23 68 01  .....u...&...#h.
0540   d5 2c e1 4b 62 53 e5 e6 c8 07 56 85 51 26 93 71  .,.KbS....V.Q&.q
0550   5a 13 13 4e ab 89 4a 7f d7 6a 68 6f 35 ca 78 3c  Z..N..J..jho5.x<
0560   ae 4d c4 6b b0 89 6c b6 32 2d cf 76 f5 bc b6 4f  .M.k..l.2-.v...O
0570   c4 2b 6b a8 15 d9 15 b2 6e ce a0 c5 5e 6f 09 25  .+k.....n...^o.%
0580   06 e5 c5 9f 8d 0c 89 82 9c d6 32 6e 57 c7 ae e2  ..........2nW...
0590   e1 5d cb 2b d5 1d ff 63 f9 f8 c4 23 bf 58 b9 ac  .].+...c...#.X..
05a0   66 72 92 36 50 93 84 ca 1b c8 05 fa 4f f8 fd ef  fr.6P.......O...
05b0   9f 38 19 4f                                      .8.O
*/
			memmove(_recv_buffer + _recv_buffer_index, packet, packet_size);
			_recv_buffer_index += packet_size;

			if (sizeof(header_t) > _recv_buffer_index)
				return;

			header_t header;
			memmove(&header, _recv_buffer, sizeof(header_t));
			header.packet_seq			= ntohs(header.packet_seq);
			header.packet_length		= ntohs(header.packet_length);
			header.adaptive_timestamp	= ntohll(header.adaptive_timestamp);
			if (header.packet_length > _recv_buffer_index)
				return;

			char * body = _recv_buffer + sizeof(header_t);
			if (header.packet_sop == 0xF2)
			{
				for (unsigned char contents_index = 0; contents_index < header.contents_count; contents_index++)
				{
					payload_t payload;
					memmove(&payload, body, sizeof(payload_t));
					payload.timestamp = ntohll(payload.timestamp);
					payload.contents_length = ntohl(payload.contents_length);
					body += sizeof(payload_t);

					char * contents = body + sizeof(payload_t);

					{
						//parse info
						unsigned int info_length = *(unsigned int*)(contents);
						info_length = ntohl(info_length);
						contents += sizeof(int);

						char xml[4096] = { 0, };
						memmove(xml, contents, info_length);
						payload_info_t info;
						parse_payload_info(xml, &info);
						contents += info_length;



						//data info
						unsigned int data_length = *(unsigned int*)(contents);
						data_length = ntohl(data_length);
						contents += sizeof(int);


					}
				}
			}
		}
	}
	else
	{
		OutputDebugStringA("arodnap_stream_client::on_recv : packet is NULL or packet size less than 1\n");
	}
}

/*
0000   00 00 07 e4 00 00 00 00 00 00 00 1c 38 44 42 34
0010   46 42 43 35 2d 38 33 43 38 2d 34 36 32 30 2d 31
0020   34 33 33 33 38 39 36 30

0000   00 00 07 e4 00 00 00 00 00 00 00 1c 38 44 42 34  ............8DB4
0010   46 42 43 35 2d 38 33 43 38 2d 34 36 32 30 2d 31  FBC5-83C8-4620-1
0020   34 33 33 33 38 39 36 30                          43338960
*/
const char * arodnap_stream_client::make_port_info_req_packet(int & packet_size)
{
	int command = 2020;
	int stream_type = 0;
	int stb_id_length = 0;
	char stb_id[] = "8DB4FBC5-83C8-4619-160133653";
	stb_id_length = strlen(stb_id);

	int command_nl = htonl(command);
	int stream_type_nl = htonl(stream_type);
	int stb_id_length_nl = htonl(stb_id_length);

	packet_size = 0;
	memmove(_send_buffer + packet_size, &command_nl, sizeof(command));
	packet_size += sizeof(command);
	memmove(_send_buffer + packet_size, &stream_type_nl, sizeof(stream_type));
	packet_size += sizeof(stream_type);
	memmove(_send_buffer + packet_size, &stb_id_length_nl, sizeof(stb_id_length));
	packet_size += sizeof(stb_id_length);
	memmove(_send_buffer + packet_size, stb_id, stb_id_length);
	packet_size += stb_id_length;

	return _send_buffer;
}

int arodnap_stream_client::parse_payload_info(const char * xml, payload_info_t * pinfo)
{
	TiXmlDocument doc;
	doc.Parse(xml);

	if (doc.Error())
		return arodnap_stream_client::err_code_t::fail;

	TiXmlElement* root = NULL, *next = NULL;
	TiXmlNode* pRoot = NULL;
	TiXmlAttribute *pAttrib = NULL;

	pRoot = doc.FirstChild("INFO");

	TiXmlNode *pNode = pRoot->FirstChild("RECT");
	for (pNode; pNode; pNode = pNode->NextSibling())
	{
		const char * pValue = pNode->Value();
		if (strcmp(pValue, "RECT") == 0)
		{
			pAttrib = pNode->ToElement()->FirstAttribute();
			while (pAttrib)
			{
				char * pszAttrib = (char *)pAttrib->Name();
				if (strcmp(pszAttrib, "X") == 0) {
					char * pszText = (char *)pAttrib->Value();
					pinfo->x = atoi(pszText);
				}
				if (strcmp(pszAttrib, "Y") == 0) {
					char * pszText = (char *)pAttrib->Value();
					pinfo->y = atoi(pszText);
				}
				if (strcmp(pszAttrib, "WIDTH") == 0) {
					char * pszText = (char *)pAttrib->Value();
					pinfo->w = atoi(pszText);
				}

				if (strcmp(pszAttrib, "HEIGHT") == 0) {
					char * pszText = (char *)pAttrib->Value();
					pinfo->h = atoi(pszText);
				}
				pAttrib = pAttrib->Next();
			}

		}

		if (strcmp(pValue, "MASK") == 0)
		{
			pAttrib = pNode->ToElement()->FirstAttribute();
			char * pszAttrib = (char *)pAttrib->Name();
			if (strcmp(pszAttrib, "flag") == 0) {
				char * pszText = (char *)pAttrib->Value();
				pinfo->mask = atoi(pszText)>0 ? true : false;
			}
		}

		if (strcmp(pValue, "TYPE") == 0)
		{
			pAttrib = pNode->ToElement()->FirstAttribute();
			char * pszAttrib = (char *)pAttrib->Name();
			if (strcmp(pszAttrib, "format") == 0) {
				char * pszText = (char *)pAttrib->Value();
				strcpy(pinfo->format, pszText);
			}
		}

		if (strcmp(pValue, "CMD") == 0)
		{
			pAttrib = pNode->ToElement()->FirstAttribute();
			char * pszAttrib = (char *)pAttrib->Name();
			if (strcmp(pszAttrib, "type") == 0) {
				char * pszText = (char *)pAttrib->Value();
				strcpy(pinfo->cmd, pszText);
			}
		}
	}

	return arodnap_stream_client::err_code_t::success;
}