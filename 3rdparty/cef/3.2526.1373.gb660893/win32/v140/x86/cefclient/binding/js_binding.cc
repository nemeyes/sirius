#if defined(WITH_EXTERNAL_INTERFACE)
#include "js_Binding.h"
#include "handler_base.h"
#include "msg_handler.h"
#include <fstream>
#include <tlhelp32.h>
#define BUFF_SIZE 1024*1024*2

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
				else if (name == "syncSendMessage")
				{
					ExecuteSyncSendMessage(arguments, retval);
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

			bool ExecuteSyncSendMessage(const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval)
			{
				if ((arguments.size() == 1 || arguments.size() == 2) && arguments[0]->IsString())
				{
					CefRefPtr<CefBrowser> cef_browser = CefV8Context::GetCurrentContext()->GetBrowser();
					CefString name = arguments[0]->GetStringValue();
					CefString param = arguments[1]->GetStringValue();

					HANDLE hPipe;
					DWORD dwRead;
					char * debug = new char[BUFF_SIZE];
					memset(debug, 0x00, BUFF_SIZE);
					char * buffer = new char[BUFF_SIZE];
					memset(buffer, 0x00, BUFF_SIZE);
					int count = 0;
					int ppid;

					if (!name.empty())
					{
						CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(name);

						if (arguments.size() == 2 && arguments[1]->IsArray())
							message_handler::getInstance().set_list(arguments[1], msg->GetArgumentList());

						cef_browser->SendProcessMessage(PID_BROWSER, msg);


						HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
						PROCESSENTRY32 pe = { 0 };
						pe.dwSize = sizeof(PROCESSENTRY32);
						if (Process32First(h, &pe)) {
							do {
								if (pe.th32ProcessID == GetCurrentProcessId()) {
									ppid = pe.th32ParentProcessID;
									_snprintf(debug, MAX_PATH, "ExecuteSyncSendMessage pid=%d, ppid=%d \n", GetCurrentProcessId(), ppid);
									OutputDebugStringA(debug);
								}
							} while (Process32Next(h, &pe));
						}

						

						char pipe_name[MAX_PATH] = { 0, };
						_snprintf(pipe_name, MAX_PATH, "\\\\.\\pipe\\%d", ppid);
						OutputDebugStringA(pipe_name);

						wchar_t wtext[MAX_PATH];
						mbstowcs(wtext, pipe_name, strlen(pipe_name) + 1);
						LPCWSTR ptr = wtext;

						hPipe = CreateNamedPipe(ptr,
							PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
							PIPE_UNLIMITED_INSTANCES, BUFF_SIZE, BUFF_SIZE, NULL, NULL);

						OVERLAPPED ov = { 0 };
						ov.hEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
						while (hPipe != INVALID_HANDLE_VALUE)
						{
							BOOL ret = ::ConnectNamedPipe(hPipe, &ov);
							if (ret == 0)
							{
								DWORD err = ::GetLastError();
								if (count > 4)
								{
									_snprintf(buffer, MAX_PATH, "Response TimeOut");
									OutputDebugStringA(buffer);
									retval = CefV8Value::CreateString(buffer);
									break;
								}
								else if (err == ERROR_IO_PENDING)
								{
									OutputDebugStringA("JavaScript Sync Waiting... \n");
									count++;
									Sleep(1000);
									continue;
								}
							}

							while (ReadFile(hPipe, buffer, BUFF_SIZE - 1, &dwRead, NULL) != FALSE)
							{
								buffer[dwRead] = '\0';
								retval = CefV8Value::CreateString(buffer);

								_snprintf(debug, MAX_PATH, "hPipe buffer = %s \n", buffer);
								OutputDebugStringA(debug);
								break;
							}
							CloseHandle(hPipe);
							break;
						}
						DisconnectNamedPipe(hPipe);
						delete[] debug;
						delete[] buffer;

						OutputDebugStringA("exit!!!!!!!!!!!!! \n");
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
			render_delegate() 
			{

			}

			virtual void OnWebKitInitialized(CefRefPtr<ClientAppRenderer> app) 
			{
#if defined(DEBUG)
				/*DWORD threadid = GetCurrentThreadId();
				char debug[MAX_PATH] = { 0 };
				_snprintf(debug, MAX_PATH, "OnWebKitInitialized : threadid is %ld\n", threadid);
				OutputDebugStringA(debug);*/
#endif
				CefString code =
					"var __SIRIUS_APP__;"
					"if (!__SIRIUS_APP__)"
					"  __SIRIUS_APP__ = {};"
					"(function() {"
					"  __SIRIUS_APP__.sendMessage = function(name, arguments) {"
					"    native function sendMessage();"
					"    return sendMessage(name, arguments);"
					"  };"
					"  __SIRIUS_APP__.syncSendMessage = function(name, arguments) {"
					"    native function syncSendMessage();"
					"    return syncSendMessage(name, arguments);"
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
