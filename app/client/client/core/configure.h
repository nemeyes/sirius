#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#include <locale>
#include <codecvt>
#include <string>
#include <sirius_string.h>

class configure
{

public:	
	virtual ~configure(void);

	static configure & get_instance()
	{
		static configure _configure;
		return _configure;
	}

	sirius::string & get_server_address() {	return _server_address;}
	sirius::string & get_server_port() { return _server_port; }
	
	sirius::string & get_attendant_app_id() 
	{ 
		return _attendant_app_id; 
	}
	
	sirius::string & get_attendant_device_id();
	sirius::string & get_url() { return _url; };
	sirius::string & get_port() { return _port; };
	
	sirius::string & get_csr_url() { return _csr_url; };
	sirius::string & get_csr_so_code() { return _csr_so_code; };
	

	void set_server_address(sirius::string & server_address) { _server_address = server_address; }
	void set_server_port(sirius::string & server_port) { _server_port = server_port; }
	void set_attendant_app_id(sirius::string & attendant_app_id) { _attendant_app_id = attendant_app_id; }
	void set_attendant_device_id(sirius::string & attendant_device_id) { _attendant_device_id = attendant_device_id; };
	void set_url(sirius::string & url) { _url = url; };
	void set_port(sirius::string & port) { _port = port; };
	void set_csr_url(sirius::string & csr_url){ _csr_url = csr_url; };
	void set_csr_so_code(sirius::string & csr_so_code) { _csr_so_code = csr_so_code; };

	bool load();
	bool save();

private:
	configure(void);

	sirius::string _server_address;
	sirius::string _server_port;
	
	sirius::string _attendant_app_id;
	sirius::string _attendant_device_id;

	sirius::string _url;
	sirius::string _port;
		
	sirius::string _csr_url;
	sirius::string _csr_so_code;	

};

#define Config configure::get_instance()

#endif