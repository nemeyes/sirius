#include "include/cef_base.h"
#include "include/cef_browser.h"
#include "include/cef_process_message.h"
#include "include/base/cef_macros.h"
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_v8.h"
#include "handler_base.h"
namespace client {
	namespace binding {
		const char msg_app_to_attendant[] = "AppToAttendant";
		const char msg_attendant_to_app[] = "AttendantToApp";
		const char kRequestPID[] = "RequestPID";
		const char kRequestedPID[] = "RequestedPID";

		class message_handler : virtual public message_handler_base
			{
		public:
			static message_handler& getInstance();
			//static void release();
			bool external_interface_message_received(CefRefPtr<CefBrowser> browser,
				CefProcessId source_process,
				CefRefPtr<CefProcessMessage> message);
			bool removeMessageCallback(const CefString& message_name,
				int browser_id);
			bool setMessageCallback(const CefString& message_name,
				int browser_id,
				CefRefPtr<CefV8Context> context,
				CefRefPtr<CefV8Value> function);
			bool msg_from_app_to_attendant(CefRefPtr<CefProcessMessage> message);
			void msg_from_attendant_to_app(CefRefPtr<CefBrowser> browser,
				CefRefPtr<CefProcessMessage> message);
		private:
			//static message_handler _instance;
			typedef enum _RETURN {
				FAIL = -1,
				TO_APP,
				TO_SIRIUS,
				TO_AND_STOP_ON_SIRIUS,
			}RETURN;

			typedef std::map<std::pair<std::string, int>,
				std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>>
				CallbackMap;
			CallbackMap callback_map_;
			void deleteCallbackMap();
			void send_to_javascript(const CefString& data);
			message_handler();
			~message_handler();
			IMPLEMENT_REFCOUNTING(message_handler);
			DISALLOW_COPY_AND_ASSIGN(message_handler);
		};
	}
}