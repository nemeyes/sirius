#ifndef ATTENDANT_PROXY_WRAPPER_H_
#define ATTENDANT_PROXY_WRAPPER_H_
#pragma once

#include "include/cef_base.h"
#include "include/base/cef_macros.h"
#include "sirius_attendant_proxy.h"

namespace client {
namespace binding {

class attendant_proxy_wrapper
{
public:

	static attendant_proxy_wrapper& getInstance();

private:
	//static attendant_proxy_wrapper* _instance;

	attendant_proxy_wrapper();
	~attendant_proxy_wrapper();
	DISALLOW_COPY_AND_ASSIGN(attendant_proxy_wrapper);

public:
	//void Initialize();
	//void finalize();
	HWND _proxy_handle;
	sirius::app::attendant::proxy * _proxy;
private:
	
	bool is_activated_;
};

}  // namespace binding
}  // namespace client

#endif  // ATTENDANT_PROXY_WRAPPER_H_
