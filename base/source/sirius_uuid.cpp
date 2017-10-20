
#include <stdio.h>
#include <string>
#include <regex>

#include <sirius_uuid.h>

sirius::uuid::uuid(void)
{
	Data1 = 0;
	Data2 = 0;
	Data3 = 0;
	ZeroMemory(Data4, 8);
}

sirius::uuid::uuid(uint8_t * puuid, int32_t size)
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

sirius::uuid::uuid(UUID & uuid)
{
	Data1 = uuid.Data1;
	Data2 = uuid.Data2;
	Data3 = uuid.Data3;
	memcpy(Data4, uuid.Data4, 8);
}

sirius::uuid::uuid(std::string & str)
{
#if 0
	std::regex rx_uuid("[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}");

	std::cmatch cmatch;
	if (!std::regex_search(in_string.c_str(), cmatch, rx_uuid)) {
		Data1 = 0;
		Data2 = 0;
		Data3 = 0;
		ZeroMemory(Data4, 8);
		return; 
	}

	std::string uuidstring(cmatch[0]);
#endif
	std::string uuidstring = str;
	
	UUID uuid;

	UuidFromStringA((RPC_CSTR)uuidstring.c_str(), &uuid);

	Data1 = uuid.Data1;
	Data2 = uuid.Data2;
	Data3 = uuid.Data3;
	memcpy(Data4, uuid.Data4, 8);
}

sirius::uuid::~uuid(void)
{
}

UUID sirius::uuid::create(void)
{
	RPC_STATUS result = UuidCreate(this);
//	if (result != RPC_S_OK)
//	{
//		//TODO 예외 처리.
//	}

	return *this;
}

std::string sirius::uuid::to_string(void)
{
	char buf[MAX_PATH] = { 0 };

	sprintf_s(buf, MAX_PATH, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
		Data1, Data2, Data3,
		Data4[0], Data4[1], Data4[2], Data4[3],
		Data4[4], Data4[5], Data4[6], Data4[7]);

	_uuid = buf;
	return _uuid;
}

char * sirius::uuid::c_str(void)
{
	to_string();
	return (char *)_uuid.c_str();
}

#if 0
bool cap_uuid::uuid::validate(const char * ptr_bytestream,int32_t length)
{
	if (ptr_bytestream == NULL)
		return false;

	if (length > 16)
		length = 16;

	std::string uuid(ptr_bytestream, length);
	return cap_uuid::validate(uuid);
}

bool cap_uuid::uuid::validate(std::string & in_uuid)
{
	std::regex regex_uuid("[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}");

	std::cmatch cmatch;
	if (!std::regex_search(in_uuid.c_str(), cmatch, regex_uuid))
		return false; 
	return false;
}
#endif 

UUID sirius::uuid::hton(void)
{
	char buf[255] = { 0 };
	UUID ret;

	ret.Data1 = htonl(Data1);
	ret.Data2 = htons(Data2);
	ret.Data3 = htons(Data3);
	memcpy(ret.Data4, Data4, 8);

	return ret;
}

UUID sirius::uuid::ntoh(void)
{
	char buf[MAX_PATH] = { 0 };
	UUID ret;

	ret.Data1 = ntohl(Data1);
	ret.Data2 = ntohs(Data2);
	ret.Data3 = ntohs(Data3);
	memcpy(ret.Data4, Data4, 8);

	return ret;
}

std::string sirius::uuid::to_string_ntoh(void)
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

sirius::uuid & sirius::uuid::operator=(const UUID & rh)
{
	Data1 = rh.Data1;
	Data2 = rh.Data2;
	Data3 = rh.Data3;
	memcpy(Data4, rh.Data4, 8);
	return *this;
}

sirius::uuid & sirius::uuid::operator=(const char * rh)
{
	unsigned char feUuid[] = { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe };

	UUID uuid;

	if (rh ==(char *) -1) {
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

sirius::uuid & sirius::uuid::operator=(const std::string  & rh)
{
	UUID uuid;

#if 0
///	std::regex rx_uuid("[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}");
//	std::cmatch cmatch;
//	if (!std::regex_search(rh.c_str(), cmatch, rx_uuid))
//		return *this; // todo excecption 

//	std::string uuid_string(cmatch[0]);
#endif
	
	std::string uuid_string = rh;

	UuidFromStringA((RPC_CSTR)uuid_string.c_str(), &uuid);

	Data1 = uuid.Data1;
	Data2 = uuid.Data2;
	Data3 = uuid.Data3;
	memcpy(Data4, uuid.Data4, 8);
	return *this;
}

bool sirius::uuid::operator == (sirius::uuid & rh)
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