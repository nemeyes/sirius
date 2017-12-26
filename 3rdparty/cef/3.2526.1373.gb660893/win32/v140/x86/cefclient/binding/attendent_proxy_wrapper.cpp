#include "attendent_proxy_wrapper.h"
#include "global.h"
#include <shellapi.h>

namespace client {
	namespace binding {


		attendent_proxy_wrapper* attendent_proxy_wrapper::sInstance = nullptr;


		attendent_proxy_wrapper::attendent_proxy_wrapper()
			: is_activated_(false) {
			wchar_t * command = GetCommandLine();
			int32_t argc = 0;
			LPWSTR * argv = CommandLineToArgvW(command, &argc);
			_proxy = new sirius::app::attendant::proxy;
			if (!_proxy->parse_argument(argc, argv, _proxy->context()))
				return;

		}

		attendent_proxy_wrapper::~attendent_proxy_wrapper()
		{
		}

		attendent_proxy_wrapper& attendent_proxy_wrapper::getInstance() {
			if (sInstance == nullptr)
				sInstance = new attendent_proxy_wrapper;

			return *sInstance;
		}

		void attendent_proxy_wrapper::Initialize()
		{
			OutputDebugString(TEXT("sirius attendent proxy loaded!!\n"));
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

		void attendent_proxy_wrapper::finalize()
		{
			OutputDebugString(TEXT("sirius attendent proxy finalized!!\n"));

			//sirius::app::attendant::proxy &attendant = sirius::app::attendant::proxy::instance();
			//sirius::app::attendant::proxy::context_t &context = sirius::app::attendant::proxy::context_t::instance();
			if (_proxy->initialize())
			{
				_proxy->stop();
				if (_proxy->context()->play_after_connect)
					_proxy->disconnect();
			}
			_proxy->release();

			delete sInstance;
			sInstance = nullptr;

			is_activated_ = false;

		}

	}  // namespace binding
}  // namespace client
