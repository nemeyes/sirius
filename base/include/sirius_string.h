#ifndef _SIRIUS_STRING_H_
#define _SIRIUS_STRING_H_

#include <string>
#include <tchar.h>

#ifdef _UNICODE
#define xxstring std::wstring
#define xxchar   wchar_t
#else
#define xxstring std::string
#define xxchar   char
#endif

namespace sirius
{
	class string : public xxstring
	{
	public:
		sirius::string(void);
		sirius::string(sirius::string & rh);
		sirius::string(xxstring & rh);
		sirius::string(const xxchar * rh);
		virtual ~string(void);

#ifdef _UNICODE
		std::string wtoa(std::locale const& loc = std::locale("KOR"));
		static sirius::string atow(const char * str);
#else
		std::wstring atow();
		static cap_string wtoa(const wchar_t * wstr, std::locale const& loc = std::locale("KOR"));
#endif

		void format(const xxchar * fmt, ...);
		sirius::string to_upper(void);
		sirius::string to_lower(void);
		bool is_number(void);
		int to_int(int defult_val = -1);

		sirius::string & operator= (const sirius::string & rh);
		sirius::string & operator= (const xxstring & rh);
		sirius::string & operator= (const xxchar * rh);

		sirius::string & operator<<(const sirius::string & rh);
		sirius::string & operator<<(const xxstring & rh);
		sirius::string & operator<<(const xxchar * rh);
	private:
		static const int max_size = 1000;
	};
};

#endif
