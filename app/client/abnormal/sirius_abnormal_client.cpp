#include <stdio.h>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <regex>
#include <json/json.h>

#define SERVER_UUID		"00000000-0000-0000-0000-000000000000"
#define BROADCAST_UUID	"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"
#define UNDEFINED_UUID	"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFE"

#define UUID_LENGTH 16
#define CMD_CREATE_SESSION_REQUEST			1001
#define CMD_CREATE_SESSION_RESPONSE			1002
#define CMD_DESTROY_SESSION_INDICATION		1003
#define CMD_KEEPALIVE_REQUEST				1004
#define CMD_KEEPALIVE_RESPONSE				1005
#define CMD_CONNECT_CLIENT_REQ							2101
#define CMD_CONNECT_CLIENT_RES							2102
#define CMD_ATTENDANT_INFO_IND							2104
#define CMD_DISCONNECT_CLIENT_REQ						2105
#define CMD_DISCONNECT_CLIENT_RES						2106

class uuid
	: GUID
{
private:
	std::string _uuid;

public:
	uuid(void)
	{
		Data1 = 0;
		Data2 = 0;
		Data3 = 0;
		ZeroMemory(Data4, 8);
	}

	uuid(uint8_t * puuid, int32_t size)
	{
		UUID uuid;

		uuid::uuid();

		if (size > 16)
			size = 16;

		memcpy(&uuid, puuid, size);

		Data1 = uuid.Data1;
		Data2 = uuid.Data2;
		Data3 = uuid.Data3;
		memcpy(Data4, uuid.Data4, 8);
	}

	uuid(UUID & uuid)
	{
		Data1 = uuid.Data1;
		Data2 = uuid.Data2;
		Data3 = uuid.Data3;
		memcpy(Data4, uuid.Data4, 8);
	}

	uuid(std::string & str)
	{
		std::string uuidstring = str;

		UUID uuid;

		UuidFromStringA((RPC_CSTR)uuidstring.c_str(), &uuid);

		Data1 = uuid.Data1;
		Data2 = uuid.Data2;
		Data3 = uuid.Data3;
		memcpy(Data4, uuid.Data4, 8);
	}

	virtual ~uuid(void) {}

	UUID create(void)
	{
		RPC_STATUS result = UuidCreate(this);
		return *this;
	}

	std::string to_string(void)
	{
		char buf[MAX_PATH] = { 0 };

		sprintf_s(buf, MAX_PATH, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
			Data1, Data2, Data3,
			Data4[0], Data4[1], Data4[2], Data4[3],
			Data4[4], Data4[5], Data4[6], Data4[7]);

		_uuid = buf;
		return _uuid;
	}

	std::string to_string_ntoh(void)
	{
		UUID hUuid = ntoh();

		std::string ret_uuid;

		RPC_CSTR pUuid = NULL;
		if (::UuidToStringA(&hUuid, (RPC_CSTR*)&pUuid) == RPC_S_OK)
		{
			ret_uuid = (char *)pUuid;
			::RpcStringFreeA(&pUuid);
		}
		std::transform(ret_uuid.begin(), ret_uuid.end(), ret_uuid.begin(), toupper);

		return ret_uuid;
	}

	char * c_str(void)
	{
		to_string();
		return (char *)_uuid.c_str();
	}

	UUID & get(void)
	{
		return *this;
	}

	LPGUID ptr(void)
	{
		return this;
	}

	UUID hton()
	{
		char buf[255] = { 0 };
		UUID ret;

		ret.Data1 = htonl(Data1);
		ret.Data2 = htons(Data2);
		ret.Data3 = htons(Data3);
		memcpy(ret.Data4, Data4, 8);

		return ret;
	}

	UUID ntoh()
	{
		char buf[MAX_PATH] = { 0 };
		UUID ret;

		ret.Data1 = ntohl(Data1);
		ret.Data2 = ntohs(Data2);
		ret.Data3 = ntohs(Data3);
		memcpy(ret.Data4, Data4, 8);

		return ret;
	}

	uuid & operator=(const UUID & rh)
	{
		Data1 = rh.Data1;
		Data2 = rh.Data2;
		Data3 = rh.Data3;
		memcpy(Data4, rh.Data4, 8);
		return *this;
	}

	uuid & operator=(const std::string &  rh)
	{
		UUID uuid;
		std::string uuid_string = rh;

		UuidFromStringA((RPC_CSTR)uuid_string.c_str(), &uuid);

		Data1 = uuid.Data1;
		Data2 = uuid.Data2;
		Data3 = uuid.Data3;
		memcpy(Data4, uuid.Data4, 8);
		return *this;
	}

	uuid & operator=(const char * rh)
	{
		unsigned char feUuid[] = { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe };

		UUID uuid;

		if (rh == (char *)-1) {
			memcpy(&uuid, feUuid, UUID_LENGTH);
			*this = uuid;
			return *this;
		}
		memcpy(&uuid, rh, UUID_LENGTH);

		Data1 = uuid.Data1;
		Data2 = uuid.Data2;
		Data3 = uuid.Data3;
		memcpy(Data4, uuid.Data4, 8);

		return *this;
	}

	bool operator== (uuid & rh)
	{
		if (rh.Data1 == this->Data1 &&
			rh.Data2 == this->Data2 &&
			rh.Data3 == this->Data3 &&
			rh.Data4[0] == this->Data4[0] &&
			rh.Data4[1] == this->Data4[1] &&
			rh.Data4[2] == this->Data4[2] &&
			rh.Data4[3] == this->Data4[3] &&
			rh.Data4[4] == this->Data4[4] &&
			rh.Data4[5] == this->Data4[5] &&
			rh.Data4[6] == this->Data4[6] &&
			rh.Data4[7] == this->Data4[7])
			return true;
		else
			return false;
	}
};

typedef struct _CMD_PAYLOAD_T
{
	int	code;
} CMD_PAYLOAD_T;

typedef struct _CMD_CREATE_SESSION_RES_T
	: public _CMD_PAYLOAD_T
{
	char uuid[64];
} CMD_CREATE_SESSION_RES_T;

typedef struct _packet_header_t
{
	char			pid;
	char			dst[16];
	char			src[16];
	unsigned char	version;
	unsigned short	command;
	unsigned int	length;
} packet_header_t;


char g_send_buffer[1500] = { 0 };
char g_recv_buffer[1500] = { 0 };
char g_uuid[64] = { 0 };

int serialize(const char * dst, const char * src, int command, const char * payload, int payload_size)
{
	const char *	pkt_payload = payload;
	unsigned int	pkt_header_size = sizeof(packet_header_t);
	unsigned int	pkt_payload_size = payload_size;
	unsigned int	pkt_size = pkt_header_size + pkt_payload_size;

	uuid src_uuid = std::string(src);
	uuid dst_uuid = std::string(dst);

	packet_header_t header;
	header.pid = 'S';
	memcpy(header.dst, (void *)&dst_uuid.hton(), sizeof(header.dst));
	memcpy(header.src, (void *)&src_uuid.hton(), sizeof(header.src));
	header.version = 0x0;
	header.command = htons(command);
	header.length = htonl(pkt_payload_size);

	memmove(g_send_buffer, &header, pkt_header_size);
	if (pkt_payload_size > 0)
		memmove(g_send_buffer + pkt_header_size, pkt_payload, pkt_payload_size);

	return pkt_size;
}

void deserialize(char * packet, int packet_size, char * dst, char * src, int & command, char ** payload, int & payload_size)
{
	int32_t pkt_header_size = sizeof(packet_header_t);
	packet_header_t header;
	memcpy(&header, packet, pkt_header_size);
	command = ntohs(header.command);
	payload_size = ntohl(header.length);

	uuid src_uuid((uint8_t*)header.src, 16);
	uuid dst_uuid((uint8_t*)header.dst, 16);

	strcpy(dst, dst_uuid.c_str());
	strcpy(src, src_uuid.c_str());

	(*payload) = packet + pkt_header_size;
}

int main(int argc, char const * argv[])
{
	if (argc < 4)
		return 0;

	WSADATA wsd;
	WSAStartup(MAKEWORD(1, 0), &wsd);

	int sock = 0;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n socket creation error \n");
		return -1;
	}

	int mode = atoi(argv[3]);
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address[%s]/ Address not supported \n", argv[1]);
		return -1;
	}
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	if (mode == 1)
	{
		closesocket(sock);
		::getchar();
		WSACleanup();
		return -1;
	}

	memcpy(g_uuid, UNDEFINED_UUID, sizeof(g_uuid));
	char recv_dst_uuid[64] = { 0 };
	char recv_src_uuid[64] = { 0 };
	int recv_command = 0;
	char * recv_payload = nullptr;
	int recv_payload_size = 0;
	int wsa_error = 0;


	//create session
	{
		//create session req
		int nsend = serialize(SERVER_UUID, g_uuid, CMD_CREATE_SESSION_REQUEST, NULL, 0);
		int rsend = send(sock, g_send_buffer, nsend, 0);
		if (rsend < 0)
		{
			wsa_error = WSAGetLastError();
		}


		if (mode == 2)
		{
			closesocket(sock);
			::getchar();
			WSACleanup();
			return -1;
		}

		//create session res
		int nread = 0;
		do
		{
			nread = recv(sock, g_recv_buffer, 1500, 0);
			::Sleep(10);
		} while (nread < 1);

		deserialize(g_recv_buffer, nread, recv_dst_uuid, recv_src_uuid, recv_command, &recv_payload, recv_payload_size);
		CMD_CREATE_SESSION_RES_T create_session_res;
		memset(&create_session_res, 0x00, sizeof(CMD_CREATE_SESSION_RES_T));
		memcpy(&create_session_res, recv_payload, sizeof(CMD_CREATE_SESSION_RES_T));
		memcpy(g_uuid, create_session_res.uuid, sizeof(g_uuid));

		if (mode == 3)
		{
			closesocket(sock);
			::getchar();
			WSACleanup();
			return -1;
		}
	}

	//connect client
	{
		//connect client req
		Json::Value wpacket;
		Json::StyledWriter writer;
		wpacket["id"] = "11:bb:cc:dd:ee";
		std::string request = writer.write(wpacket);
		int nsend = serialize(SERVER_UUID, g_uuid, CMD_CONNECT_CLIENT_REQ, request.c_str(), request.size() + 1);
		int rsend = send(sock, g_send_buffer, nsend, 0);
		if (rsend < 0)
		{
			wsa_error = WSAGetLastError();
		}

		if (mode == 4)
		{
			closesocket(sock);
			::getchar();
			WSACleanup();
			return -1;
		}

		//connect client res
		int nread = 0;
		do
		{
			nread = recv(sock, g_recv_buffer, 1500, 0);
			::Sleep(10);
		} while (nread < 1);

		deserialize(g_recv_buffer, nread, recv_dst_uuid, recv_src_uuid, recv_command, &recv_payload, recv_payload_size);

		Json::Value rpacket;
		Json::Reader reader;
		reader.parse(recv_payload, rpacket);

		int32_t rcode = -1;
		if (rpacket["rcode"].isInt())
		rcode = rpacket["rcode"].asInt();

		//std::string rmsg = rpacket["msg"].asString();
		printf(recv_payload);

		if (mode == 5)
		{
			closesocket(sock);
			::getchar();
			WSACleanup();
			return -1;
		}
	}


	::getchar();

	::closesocket(sock);

	WSACleanup();
}