#if defined(WITH_EXTERNAL_INTERFACE)
#include "cefclient/browser/root_window_win.h"
#include "cefclient/browser/util_win.h"
#include "global.h"
#include "msg_handler.h"
#include "net_define.h"
#include "socket_win.h"
#include "attendant_interface.h"

namespace client {
	namespace binding {
		message_handler::message_handler() 
		{
		
		}

		message_handler::~message_handler()
		{
			deleteCallbackMap();
		}

		message_handler& message_handler::getInstance() 
		{
			static message_handler _instance;
			return _instance;
		}

		/*
		void message_handler::release() 
		{
			if (_instance) {
				_instance->deleteCallbackMap();
				//_instance->Release();
			}
		}
		*/

		void message_handler::deleteCallbackMap() {
			if (!callback_map_.empty()) {
				CallbackMap::iterator it = callback_map_.begin();
				for (; it != callback_map_.end();) {
					callback_map_.erase(it++);
				}
			}
		}
		void message_handler::send_to_javascript(const CefString & data)
		{
			RootWindowWin* rootWin =
				GetUserDataPtr<RootWindowWin*>(binding::global::get_instance().get_window_handle());
			DCHECK(rootWin);
			CefRefPtr<CefBrowser> browser = rootWin->GetBrowser();

			CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("AttendantToApp");
			msg->GetArgumentList()->SetString(0, data);

			browser->SendProcessMessage(PID_RENDERER, msg);
		}

		bool message_handler::external_interface_message_received(CefRefPtr<CefBrowser> browser,
			CefProcessId source_process,
			CefRefPtr<CefProcessMessage> message) {
			CefRefPtr<CefListValue> args = message->GetArgumentList();
			CefString message_name = message->GetName();
			bool ret = false;

			if (message_name == msg_app_to_attendant) {
				msg_from_app_to_attendant(message);
			}
			else if (message_name == msg_attendant_to_app) {
				msg_from_attendant_to_app(browser, message);
			}
			return ret;
		}

		bool message_handler::msg_from_app_to_attendant(CefRefPtr<CefProcessMessage> message) {
			CefRefPtr<CefListValue> args = message->GetArgumentList();
			bool ret = false;
			CefString result = args->GetString(0);
			ret = socket_win::get_instance()->send_bypass_packet(CONTENTS_TYPE::TOAPP, result.ToString());
			return ret;
		}

		bool message_handler::setMessageCallback(const CefString& message_name,
			int browser_id,
			CefRefPtr<CefV8Context> context,
			CefRefPtr<CefV8Value> function) {
			callback_map_.insert(
				std::make_pair(std::make_pair(message_name, browser_id),
					std::make_pair(context, function)));
			return true;
		}

		bool message_handler::removeMessageCallback(const CefString& message_name,
			int browser_id) {
			CallbackMap::iterator it =
				callback_map_.find(std::make_pair(message_name, browser_id));
			if (it != callback_map_.end()) {
				callback_map_.erase(it);
				return true;
			}
			return false;
		}

		void message_handler::msg_from_attendant_to_app(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefProcessMessage> message) {
			if (!callback_map_.empty()) {
				CefString message_name = message->GetName();

				CallbackMap::const_iterator it = callback_map_.find(
					std::make_pair(message_name.ToString(),
						browser->GetIdentifier()));

				if (it != callback_map_.end()) {
					CefRefPtr<CefV8Context> context = it->second.first;
					CefRefPtr<CefV8Value> callback = it->second.second;
					context->Enter();
					CefV8ValueList arguments;
					arguments.push_back(CefV8Value::CreateString(message_name));
					CefRefPtr<CefListValue> list = message->GetArgumentList();
					CefRefPtr<CefV8Value> args =
						CefV8Value::CreateArray(static_cast<int>(list->GetSize()));
					set_list(list, args);
					arguments.push_back(args);

					CefRefPtr<CefV8Value> retval = callback->ExecuteFunction(NULL, arguments);
					context->Exit();
				}
			}
		}
	}
}
#endif
