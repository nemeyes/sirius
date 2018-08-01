#include "attendant_proxy_wrapper.h"
#include "global.h"
#include <shellapi.h>

namespace client {
	namespace binding {
		attendant_proxy_wrapper::attendant_proxy_wrapper()
			: is_activated_(false) {
		}

		attendant_proxy_wrapper::~attendant_proxy_wrapper()
		{
		}

		attendant_proxy_wrapper& attendant_proxy_wrapper::getInstance() {
			static attendant_proxy_wrapper instance_;
			return instance_;
		}

		/*void attendant_proxy_wrapper::Initialize()
		{
			OutputDebugString(TEXT("sirius attendant proxy loaded!!\n"));
			_proxy->initialize();
			if (_proxy->is_initialized())
			{
				if (_proxy->context()->play_after_connect)
				{
					_proxy->connect();
				}
				else
				{
					_proxy->play();
				}
				_proxy_handle = _proxy->context()->hwnd;
				is_activated_ = true;
			}
		}

		void attendant_proxy_wrapper::finalize()
		{
			OutputDebugString(TEXT("sirius attendant proxy finalized!!\n"));
			if (_proxy->initialize())
			{
				_proxy->stop();
				if (_proxy->context()->play_after_connect)
					_proxy->disconnect();
			}
			_proxy->release();

			delete _instance;
			_instance = nullptr;

			is_activated_ = false;

		}*/

	}  // namespace binding
}  // namespace client
