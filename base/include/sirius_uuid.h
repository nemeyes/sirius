#ifndef _SIRIUS_SICP_UUID_H_
#define _SIRIUS_SICP_UUID_H_

#include <winsock2.h>
#include <windows.h>

#include <string>

#define UUID_LENGTH 16

namespace sirius
{
	class uuid 
		: GUID
	{
	private:
		std::string _uuid;

	public:
		uuid(void);
		uuid(uint8_t * uuid, int32_t size);
		uuid(UUID & uuid);
		uuid(std::string & str);

		virtual ~uuid(void);

		UUID		create(void);
		std::string to_string(void);
		std::string to_string_ntoh(void);
		char *		c_str(void);

		UUID &		get(void) 
		{
			return *this;
		}

		LPGUID		ptr(void) 
		{
			return this;
		}

#if 0
		static bool validate(std::string & in_uuid);
		static bool validate(const char * ptr_bytestream, int32_t length);
#endif

		UUID hton();
		UUID ntoh();

		sirius::uuid & operator=(const UUID & rh);
		sirius::uuid & operator=(const std::string &  rh);
		sirius::uuid & operator=(const char * rh);

		bool operator== (sirius::uuid & rh);
	};
};

#endif