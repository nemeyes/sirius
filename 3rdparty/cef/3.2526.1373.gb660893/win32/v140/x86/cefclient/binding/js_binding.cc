#if defined(WITH_EXTERNAL_INTERFACE)
#include "js_Binding.h"
#include "handler_base.h"
#include "msg_handler.h"

namespace client {
	namespace binding {

		class CSBV8Handler : public CefV8Handler {
		public:
			CSBV8Handler() 
			{
			}

			~CSBV8Handler() {
			}
			virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) 
			{
				if (name == "sendMessage") 
				{
					ExecuteSendMessage(arguments);
				}
				else if (name == "setMessageCallback") 
				{
					ExecuteSetMessageCallBack(arguments);
				}
				else if (name == "removeMessageCallback") 
				{
					retval = CefV8Value::CreateBool(ExecuteRemoveMessageCallBack(arguments));
				}
				return true;
			}

			bool ExecuteRemoveMessageCallBack(const CefV8ValueList& arguments) 
			{
				if (arguments.size() == 1 && arguments[0]->IsString()) 
				{
					std::string name = arguments[0]->GetStringValue();
					CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
					int browser_id = context->GetBrowser()->GetIdentifier();
					return message_handler::getInstance().removeMessageCallback(name, browser_id);
				}
				return false;
			}

			bool ExecuteSetMessageCallBack(const CefV8ValueList& arguments) 
			{
				if (arguments.size() == 2 && arguments[0]->IsString() && arguments[1]->IsFunction())
				{
					std::string name = arguments[0]->GetStringValue();
					CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
					int browser_id = context->GetBrowser()->GetIdentifier();
					
					message_handler::getInstance().removeMessageCallback(name, browser_id);
					message_handler::getInstance().setMessageCallback(name, browser_id, context, arguments[1]);
				}
				return true;
			}

			bool ExecuteSendMessage(const CefV8ValueList& arguments) 
			{
				if ((arguments.size() == 1 || arguments.size() == 2) && arguments[0]->IsString()) 
				{
					CefRefPtr<CefBrowser> cef_browser = CefV8Context::GetCurrentContext()->GetBrowser();
					CefString name = arguments[0]->GetStringValue();
					if (!name.empty()) 
					{
						CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(name);

						// Translate the arguments, if any.
						if (arguments.size() == 2 && arguments[1]->IsArray())
							message_handler::getInstance().set_list(arguments[1], msg->GetArgumentList());

						cef_browser->SendProcessMessage(PID_BROWSER, msg);
					}
				}
				return true;
			}
		private:
			IMPLEMENT_REFCOUNTING(CSBV8Handler);
		};

		// Handle bindings in the render process.
		class render_delegate : public ClientAppRenderer::Delegate {
		public:
			render_delegate() {
			}
			virtual void OnWebKitInitialized(CefRefPtr<ClientAppRenderer> app) {
				DWORD threadid = GetCurrentThreadId();
				char debug[MAX_PATH] = { 0 };
				_snprintf(debug, MAX_PATH, "OnWebKitInitialized : threadid is %ld\n", threadid);
				OutputDebugStringA(debug);

				CefString code =
					"var __SIRIUS_APP__;"
					"if (!__SIRIUS_APP__)"
					"  __SIRIUS_APP__ = {};"
					"(function() {"
					"  __SIRIUS_APP__.sendMessage = function(name, arguments) {"
					"    native function sendMessage();"
					"    return sendMessage(name, arguments);"
					"  };"
					"  __SIRIUS_APP__.setMessageCallback = function(name, callback) {"
					"    native function setMessageCallback();"
					"    return setMessageCallback(name, callback);"
					"  };"
					"  __SIRIUS_APP__.removeMessageCallback = function(name) {"
					"    native function removeMessageCallback();"
					"    return removeMessageCallback(name);"
					"  };"
					"})();";
				CefRegisterExtension("v8/app", code, new CSBV8Handler());
			}
		private:
			IMPLEMENT_REFCOUNTING(render_delegate);
		};

		void CreateDelegates(ClientAppRenderer::DelegateSet& delegates) {
			delegates.insert(new render_delegate);
		}

	}  // namespace binding
}  // namespace client
#endif
