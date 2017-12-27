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

		CefRefPtr<socket_win> socket_win::pThis = NULL;

		socket_win::socket_win() {
			create();
		}

		socket_win::~socket_win() {
			pThis = nullptr;
		}

		CefRefPtr<socket_win> socket_win::getInstance() {
			if (!pThis) {
				pThis = new socket_win;
			}

			return pThis;
		}

		void socket_win::release() {
			if (pThis) {
				pThis->Release();
			}
		}

		bool socket_win::create() {
			return socketbase::Create();
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
			int packetSize = recv(m_hSocket, buf, sizeof(int), 0);
			memcpy(&packetSize, buf, sizeof(int));
			packetSize = ntohl(packetSize);
			if (packetSize > SOCKET_BUF_LIMIT)
				return false;
			buffer.insert(buffer.end(), buf, buf + sizeof(int));

			do {
				int t = (packetSize - totalRecv);
				oneRead = (t >= MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : t;
				byteRecv = recv(m_hSocket, buf, oneRead, 0);
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
				sendToJSEngine(body);
				break;
			case CONTENTS_TYPE::TOAPP:
				OutputDebugStringA("[sirius->attendant]Receive Data From sirius & Bypass to V8 Engine\n");
				sendToJSEngine(body);
				break;
			case CONTENTS_TYPE::MENUID_URL:
				disconnectSTB();
				break;
			default:
				break;
			}
			return true;
		}

		void socket_win::sendToJSEngine(const CefString& xml) {
			if (!CefCurrentlyOn(TID_UI)) {
				CefPostTask(TID_UI, base::Bind(&socket_win::sendToJSEngine, this, xml));
				return;
			}
			OutputDebugStringA("[sirius->attendant]xml \n");

			RootWindowWin* rootWin =
				GetUserDataPtr<RootWindowWin*>(global::getInstance().getWindowHandle());
			DCHECK(rootWin);
			CefRefPtr<CefBrowser> browser = rootWin->GetBrowser();

			CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("AttendantToApp");
			msg->GetArgumentList()->SetString(0, xml);

			browser->SendProcessMessage(PID_RENDERER, msg);
		}

		bool socket_win::recvHeader(const BUFFER& buffer, HEADER& header) {
			memcpy(&header, &buffer[0], sizeof(header));
			header.bodyLength = ntohl(header.bodyLength);

			return true;
		}

		bool socket_win::connectSTB() {
			HWND hWnd = global::getInstance().getWindowHandle();
			SendMessage(hWnd, CI_SEND_URL, (WPARAM)0, (LPARAM)0);
			return true;
		}

		bool socket_win::disconnectSTB() {
			HWND hWnd = global::getInstance().getWindowHandle();
			SendMessage(hWnd, CI_PLAY_STOP, (WPARAM)0, (LPARAM)0);
			return true;
		}

		bool socket_win::sendXmlPacket(int contentsType, const std::string& utf8_xml) {
			int bufSize = utf8_xml.length() + 12;
			char* packet = new char[bufSize];
			memset(packet, 0x00, bufSize);
			OutputDebugStringA("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!sendXmlPacket");
			int size = 0;

			if (!makeXmlPacket(packet, size, contentsType, utf8_xml)) {
				OutputDebugStringA("[attendant->sirius] Message Send Failed\n");

				if (packet)
					delete[] packet;

				return false;
			}

			int rtn = Send(packet, size);
			if (rtn <= 0) {
				if (packet)
					delete[] packet;

				return false;
			}

			if (packet)
				delete[] packet;

			return true;
		}

		bool socket_win::makeXmlPacket(char* data, int& size, int& contentsType, const std::string& utf8_xml) {
			int index = 0, len = 0;
			len = utf8_xml.length();
			memcpy(&data[index], utf8_xml.c_str(), len);
			size = index + len;

			return true;
		}

		bool socket_win::SiriusToJSEngine(uint8_t * data, size_t size) {
			sendToJSEngine((char *)data);

			return true;
		}

	}  // namespace csb
}  // namespace client
#endif