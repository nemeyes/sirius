#if defined(WITH_EXTERNAL_INTERFACE)
#include <vector>
#include "include/cef_browser.h"
#include "include/cef_v8.h"
#include "cefclient/browser/root_window_win.h"
#include "cefclient/browser/util_win.h"
#include "cefclient/common/client_switches.h"
#include "global.h"
#include "socket_win.h"
#include "cefclient/binding/attendant_interface.h"

namespace client {
	namespace binding {

		CefRefPtr<socket_win> socket_win::_ptr_this = NULL;

		socket_win::socket_win() {
			create();
			OutputDebugStringA("socket_win!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
		}

		socket_win::~socket_win() {
			_ptr_this = nullptr;
		}

		CefRefPtr<socket_win> socket_win::get_instance() {
			if (!_ptr_this) {
				_ptr_this = new socket_win;
			}

			return _ptr_this;
		}

		void socket_win::release() {
			if (_ptr_this) {
				_ptr_this->Release();
			}
		}

		bool socket_win::create() {
			return socketbase::create();
		}

		void socket_win::OnReceive(int nErrorCode) {
			HEADER header;
			BUFFER buffer;

			if (!recvData(buffer))
				return;
			OutputDebugStringA("Socket Recevied\n");

			if (!recvHeader(buffer, header))
				return;

			if (!recvBody(buffer, header))
				return;
		}

#define SOCKET_BUF_LIMIT 1024 * 1024 * 10

		bool socket_win::recvData(BUFFER& buffer) {
			char buf[MAX_BUFFER_SIZE];
			int byteRecv = 0;
			int totalRecv = 0;
			int oneRead = 0;

			//First, get one packet size
			int packetSize = recv(_socket, buf, sizeof(int), 0);
			memcpy(&packetSize, buf, sizeof(int));
			packetSize = ntohl(packetSize);
			if (packetSize > SOCKET_BUF_LIMIT)
				return false;
			buffer.insert(buffer.end(), buf, buf + sizeof(int));

			do {
				int t = (packetSize - totalRecv);
				oneRead = (t >= MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : t;
				byteRecv = recv(_socket, buf, oneRead, 0);
				if (byteRecv > 0) {
					buffer.insert(buffer.end(), buf, buf + byteRecv);
					totalRecv += byteRecv;
					//if totalRecv are bigger than 10M, return false
					if (totalRecv >= SOCKET_BUF_LIMIT)
						return false;
				}
				else
					break;
			} while (totalRecv < packetSize);

			if (totalRecv != packetSize)
				return false;

			return true;
		}

		int socket_win::recvBody_getDataLen(const BUFFER& buffer, int sizeOfHeader, int& dataLen) {
			memcpy(&dataLen, &buffer[sizeOfHeader - 1], sizeof(dataLen));
			dataLen = ntohl(dataLen);
			return sizeof(dataLen);
		}

		bool socket_win::getBody(const int len, const int index,
			const BUFFER& buffer, CefString& body) {
			if (len > 1) {
				char* data = new char[len + 1];
				memset(data, 0x00, len + 1);
				memcpy(data, (const char*)&buffer[index], len);
				body = data;
				delete[] data;
			}
			return true;
		}

		bool socket_win::recvBody(const BUFFER& buffer, const HEADER& header) {
			int index = sizeof(header) - 1; //because buffer start 0
			int dataLen = 0;
			CefString body;
			index += recvBody_getDataLen(buffer, sizeof(header), dataLen);

			getBody(dataLen, index, buffer, body);
			OutputDebugStringA("[sirius->attendant]\n");
			switch (header.commandType) {
			case COMMAND_TYPE::RESPONSE:
				recvBody_cmdType_response(header);
				break;
			case COMMAND_TYPE::REQUEST:
				recvBody_cmdType_request(header, body);
				break;
			default:
				break;
			}

			return true;
		}

		bool socket_win::recvBody_cmdType_response(const HEADER& header) {
			switch (header.contentsType) {
			case CONTENTS_TYPE::CONNECT:
				OutputDebugStringA("[sirius->attendnet]sirius Connect\n");
				break;
			case CONTENTS_TYPE::TOAPP:
				OutputDebugStringA("[sirius->attendnet]Receive Data From sirius\n");
				break;
			default:
				break;
			}
			return true;
		}

		bool socket_win::recvBody_cmdType_request(const HEADER& header,
			const CefString& body) {
			switch (header.contentsType) {
			case CONTENTS_TYPE::CONTENTS_URL:
				connectSTB();
				send_to_javascript(body);
				break;
			case CONTENTS_TYPE::TOAPP:
				OutputDebugStringA("[sirius->attendant]Receive Data From sirius & Bypass to V8 Engine\n");
				send_to_javascript(body);
				break;
			case CONTENTS_TYPE::MENUID_URL:
				disconnectSTB();
				break;
			default:
				break;
			}
			return true;
		}

		void socket_win::send_to_javascript(const CefString& data) {
			if (!CefCurrentlyOn(TID_UI)) {
				CefPostTask(TID_UI, base::Bind(&socket_win::send_to_javascript, this, data));
				return;
			}
			RootWindowWin* rootWin =
				GetUserDataPtr<RootWindowWin*>(global::get_instance().get_window_handle());
			DCHECK(rootWin);
			CefRefPtr<CefBrowser> browser = rootWin->GetBrowser();

			CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("AttendantToApp");
			msg->GetArgumentList()->SetString(0, data);

			browser->SendProcessMessage(PID_RENDERER, msg);
		}

		bool socket_win::recvHeader(const BUFFER& buffer, HEADER& header) {
			memcpy(&header, &buffer[0], sizeof(header));
			header.bodyLength = ntohl(header.bodyLength);

			return true;
		}

		bool socket_win::connectSTB() {
			HWND hWnd = global::get_instance().get_window_handle();
			SendMessage(hWnd, CI_SEND_URL, (WPARAM)0, (LPARAM)0);
			return true;
		}

		bool socket_win::disconnectSTB() {
			HWND hWnd = global::get_instance().get_window_handle();
			SendMessage(hWnd, CI_PLAY_STOP, (WPARAM)0, (LPARAM)0);
			return true;
		}

		bool socket_win::send_bypass_packet(int contentsType, const std::string& utf8_data) {
			int bufSize = utf8_data.length() + 12;
			char* send_packet = new char[bufSize];
			memset(send_packet, 0x00, bufSize);
			OutputDebugStringA("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!send_bypass_packet");
			int size = 0;

			if (!make_json_packet(send_packet, size, contentsType, utf8_data)) {
				OutputDebugStringA("[attendant->sirius] message send failed\n");

				if (send_packet)
					delete[] send_packet;

				return false;
			}

			int rtn = send_data(send_packet, size);
			binding::socketbase::calback_attendant_to_app((uint8_t *)send_packet, strlen(send_packet));
			if (rtn <= 0) {
				if (send_packet)
					delete[] send_packet;

				return false;
			}

			if (send_packet)
				delete[] send_packet;

			return true;
		}

		bool socket_win::make_json_packet(char* data, int& size, int& contentsType, const std::string& utf8_json) {
			int index = 0, len = 0;
			len = utf8_json.length();
			memcpy(&data[index], utf8_json.c_str(), len);
			size = index + len;

			return true;
		}

		bool socket_win::sirius_to_javascript(uint8_t * data, size_t size) {
			if (strcmp((const char *)data, "reload") != 0)
			{
				send_to_javascript((char *)data);
			}
			else
			{
				RootWindowWin* rootWin =
					GetUserDataPtr<RootWindowWin*>(global::get_instance().get_window_handle());
				DCHECK(rootWin);
				CefRefPtr<CefBrowser> browser = rootWin->GetBrowser();
				browser->Reload();
				OutputDebugStringA("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Attendant Reload");
			}

			return true;
		}

	}  // namespace binding
}  // namespace client
#endif