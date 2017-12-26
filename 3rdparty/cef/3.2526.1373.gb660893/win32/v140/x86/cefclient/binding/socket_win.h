#if defined(WITH_JAVASCRIPT)
#ifndef SOCKET_WIN_H_
#define SOCKET_WIN_H_
#pragma once

#include "include/base/cef_macros.h"
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_base.h"
#include "macro.h"
#include "net_define.h"
#include "socket_base_sirius.h"

namespace client {
	namespace binding {

		typedef std::vector<unsigned char> BUFFER;

		class socket_win : public socketbase {
		public:
			static CefRefPtr<socket_win> getInstance();
			static void release();
			virtual void OnReceive(int nErrorCode);
			bool create();
			bool sendXmlPacket(int contentsType, const std::string& utf8_xml);
			bool AmadeusToJSEngine(uint8_t * data, size_t size);

		protected:
			bool makeXmlPacket(char* data, int& size, int& contentsType, const std::string& utf8_xml);
			bool recvData(BUFFER& buffer);
			bool recvHeader(const BUFFER& buffer, HEADER& header);
			bool recvBody(const BUFFER& buffer, const HEADER& header);
			bool getBody(const int len, const int index, const BUFFER& buffer, CefString& body);
			int recvBody_getDataLen(const BUFFER& buffer, int sizeOfHeader, int& dataLen);
			bool recvBody_cmdType_response(const HEADER& header);
			bool recvBody_cmdType_request(const HEADER& header, const CefString& body);
			void sendToJSEngine(const CefString& xml);
			bool connectSTB();
			bool disconnectSTB();

		private:
			socket_win();
			virtual ~socket_win();

			static CefRefPtr<socket_win> pThis;

			IMPLEMENT_REFCOUNTING(socket_win);
			DISALLOW_COPY_AND_ASSIGN(socket_win);
		};

	}  // namespace binding
}  // namespace client

#endif  // SOCKET_WIN_H_
#endif