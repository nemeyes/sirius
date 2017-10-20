#include <stdarg.h>
#include <string>
#include <algorithm>
#include <codecvt>
#include <vector>
#include <locale>
#include "sirius_string.h"

sirius::string::string(void)
{
}

sirius::string::string(sirius::string & rh)
	: xxstring(rh)
{

}

sirius::string::string(xxstring & rh)
	: xxstring(rh)
{
}

sirius::string::string(const xxchar * rh)
	: xxstring(rh)
{
}

sirius::string::~string(void)
{
}

#ifdef _UNICODE
std::string sirius::string::wtoa(std::locale const& loc)
{
	std::wstring wstr = *this;

	typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt_t;
	std::codecvt<wchar_t, char, std::mbstate_t> const& codecvt = std::use_facet<codecvt_t>(loc);
	std::mbstate_t state = std::mbstate_t();
	std::vector<char> buff((wstr.size() + 1) * codecvt.max_length());

	wchar_t const* in_next = wstr.c_str();
	char* out_next = &buff[0];
	codecvt_t::result r = codecvt.out(state, wstr.c_str(), wstr.c_str() + wstr.size(), in_next, &buff[0], &buff[0] + buff.size(), out_next);
	return std::string(&buff[0]);
}

sirius::string sirius::string::atow(const char * str)
{
	std::string temp = str;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	return converter.from_bytes(temp);
}
#else
std::wstring cap_string::atow()
{
	std::string temp = *this;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	std::wstring wide = converter.from_bytes(temp);
	return wide;
}

cap_string cap_string::wtoa(const wchar_t * wstr, std::locale const& loc)
{
	std::wstring wstring = wstr;

	typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt_t;
	std::codecvt<wchar_t, char, std::mbstate_t> const& codecvt = std::use_facet<codecvt_t>(loc);
	std::mbstate_t state = std::mbstate_t();
	std::vector<char> buff((wstring.size() + 1) * codecvt.max_length());

	wchar_t const* in_next = wstring.c_str();
	char* out_next = &buff[0];
	codecvt_t::result r = codecvt.out(state, wstring.c_str(), wstring.c_str() + wstring.size(), in_next, &buff[0], &buff[0] + buff.size(), out_next);
	return cap_string(&buff[0]);
}
#endif



void sirius::string::format(const xxchar * fmt, ...)
{
	xxchar msg[max_size] = {0};

	va_list args;
	va_start(args, fmt);

#ifdef _UNICODE
	_vsnwprintf_s(msg, _countof(msg), fmt, args);
#else
	vsnprintf_s(msg, _countof(msg), fmt, args);
#endif
	va_end(args);

	append(msg);
}

sirius::string sirius::string::to_upper(void)
{
	sirius::string tmp = *this;
	std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
	return tmp;
}

sirius::string sirius::string::to_lower(void)
{
	sirius::string tmp = *this;
	std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
	return tmp;
}

bool sirius::string::is_number(void)
{
	sirius::string::const_iterator it = begin();
	while (it != end() && isdigit(*it)) 
		++it;
	return !empty() && it == end();
}

int sirius::string::to_int(int defult_val)
{
	int ret_val = 0;	
	
	if (!is_number())
		return defult_val;

#ifdef _UNICODE
	wchar_t* stopstring = nullptr;
	ret_val = wcstol(c_str(), &stopstring,0);
#else
	char* stopstring = nullptr;
	ret_val = strtol(c_str(), &stopstring,0);
#endif

	if (errno == ERANGE)
		ret_val = defult_val;

	return ret_val;
}

sirius::string & sirius::string::operator=(const sirius::string & rh)
{
	this->assign(rh);
	return *this;
}

sirius::string & sirius::string::operator=(const xxstring & rh)
{
	this->assign(rh);
	return *this;
}

sirius::string & sirius::string::operator=(const xxchar * rh)
{
	this->assign(rh);
	return *this;
}


sirius::string & sirius::string::operator<<(const sirius::string & rh)
{
	this->append(rh.c_str());
	return *this;
}

sirius::string & sirius::string::operator<<(const xxstring & rh)
{
	this->append(rh.c_str());
	return *this;

}

sirius::string & sirius::string::operator<<(const xxchar * rh)
{
	this->append(rh);
	return *this;
}
