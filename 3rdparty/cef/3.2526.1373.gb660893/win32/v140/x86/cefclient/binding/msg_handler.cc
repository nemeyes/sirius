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
		message_handler* message_handler::sInstance = nullptr;
		message_handler::message_handler() {
		}
		message_handler& message_handler::getInstance() {
			if (sInstance == nullptr)
				sInstance = new message_handler;

			return *sInstance;
		}

		void message_handler::release() {
			if (sInstance) {
				sInstance->deleteCallbackMap();
				//sInstance->Release();
			}
		}

		void message_handler::deleteCallbackMap() {
			if (!callback_map_.empty()) {
				CallbackMap::iterator it = callback_map_.begin();
				for (; it != callback_map_.end();) {
					callback_map_.erase(it++);
				}
			}
		}
		bool message_handler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
			CefProcessId source_process,
			CefRefPtr<CefProcessMessage> message) {
			CefRefPtr<CefListValue> args = message->GetArgumentList();
			CefString msgName = message->GetName();
			bool ret = false;

			if (msgName == kAppToAttendant) {
				msgAppToAttendant(message);
			}
			else if (msgName == kAttendantToApp) {
				msgAttendantToApp(browser, message);
			}
			return ret;
		}

		bool message_handler::msgAppToAttendant(CefRefPtr<CefProcessMessage> message) {
			CefRefPtr<CefListValue> args = message->GetArgumentList();
			bool ret = false;

			switch (args->GetType(0)) {
			case VTYPE_STRING:
				ret = msgAppToAttendantGeneralMode(args);
				break;
			default:
				break;
			}

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

		void message_handler::msgAttendantToApp(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefProcessMessage> message) {
			if (!callback_map_.empty()) {
				CefString message_name = message->GetName();

				CallbackMap::const_iterator it = callback_map_.find(
					std::make_pair(message_name.ToString(),
						browser->GetIdentifier()));

				if (it != callback_map_.end()) {
					CefRefPtr<CefV8Context> context = it->second.first;
					CefRefPtr<CefV8Value> callback = it->second.second;

					// Enter the context.
					context->Enter();

					CefV8ValueList arguments;

					// First argument is the message name.
					arguments.push_back(CefV8Value::CreateString(message_name));

					// Second argument is the list of message arguments.
					CefRefPtr<CefListValue> list = message->GetArgumentList();
					CefRefPtr<CefV8Value> args =
						CefV8Value::CreateArray(static_cast<int>(list->GetSize()));
					SetList(list, args);
					arguments.push_back(args);

					CefRefPtr<CefV8Value> retval = callback->ExecuteFunction(NULL, arguments);

					context->Exit();
				}
			}
		}

		bool message_handler::msgAppToAttendantGeneralMode(CefRefPtr<CefListValue> args) {
			bool ret = false;
			CefString result = args->GetString(0);

			switch (processByGroup(result)) {
			case RETURN::FAIL:
				break;
			case RETURN::TO_APP:
				ret = true;
				break;//aleady sended message to webapp, so it is passed here.
			case RETURN::TO_SIRIUS:
				ret = socket_win::getInstance()->sendXmlPacket(CONTENTS_TYPE::TOAPP, result.ToString());
				break;
			case RETURN::TO_AND_STOP_ON_SIRIUS:
				ret = socket_win::getInstance()->sendXmlPacket(CONTENTS_TYPE::SEND_TO_AND_STOP_ON_SIRIUS, result.ToString());
				break;
			}
			//LOG_INFO("return = %d", ret);
			return ret;

		}
		int message_handler::processByGroup(const CefString& xml) {
			appmsg msg;
			int rtn = 0;
			if (!msg.parse(xml))
				return RETURN::FAIL;

			if (msg.getGroupString() == GROUP_ANIMATION_INFO) {
				rtn = FAIL;
			}

			else if (msg.getGroupString() == GROUP_SIRIUS) {
				rtn = RETURN::TO_AND_STOP_ON_SIRIUS;
			}
			else {
				rtn = RETURN::TO_SIRIUS;
			}

			return rtn;
		}
	}
}
#endif
