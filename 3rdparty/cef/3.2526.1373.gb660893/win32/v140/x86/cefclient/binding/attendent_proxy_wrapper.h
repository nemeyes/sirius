#ifndef ATTENDENT_PROXY_WRAPPER_H_
#define ATTENDENT_PROXY_WRAPPER_H_
#pragma once

#include "include/cef_base.h"
#include "include/base/cef_macros.h"
#include "sirius_attendant_proxy.h"

namespace client {
namespace binding {

class attendent_proxy_wrapper
{
public:

	static attendent_proxy_wrapper& getInstance();

private:
	static attendent_proxy_wrapper* sInstance;

	attendent_proxy_wrapper();
	~attendent_proxy_wrapper();
	DISALLOW_COPY_AND_ASSIGN(attendent_proxy_wrapper);

public:
	void Initialize();
	void finalize();
	HWND _proxy_handle;
	sirius::app::attendant::proxy * _proxy;
private:
	
	bool is_activated_;
};

}  // namespace binding
}  // namespace client

#endif  // ATTENDENT_PROXY_WRAPPER_H_
