#if defined(WITH_EXTERNAL_INTERFACE)
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
			static CefRefPtr<socket_win> get_instance();
			static void release();
			virtual void OnReceive(int nErrorCode);
			bool create();
			bool send_bypass_packet(int contentsType, const std::string& utf8_data);
			bool sirius_to_javascript(uint8_t * data, size_t size);
			std::string start_url;
			std::string get_url;

		protected:
			bool make_json_packet(char* data, int& size, int& contentsType, const std::string& utf8_json);
			bool recvData(BUFFER& buffer);
			bool recvHeader(const BUFFER& buffer, HEADER& header);
			bool recvBody(const BUFFER& buffer, const HEADER& header);
			bool getBody(const int len, const int index, const BUFFER& buffer, CefString& body);
			int recvBody_getDataLen(const BUFFER& buffer, int sizeOfHeader, int& dataLen);
			bool recvBody_cmdType_response(const HEADER& header);
			bool recvBody_cmdType_request(const HEADER& header, const CefString& body);
			void send_to_javascript(const CefString& data);
			bool connectSTB();
			bool disconnectSTB();

		private:
			socket_win();
			virtual ~socket_win();

			static CefRefPtr<socket_win> _ptr_this;

			IMPLEMENT_REFCOUNTING(socket_win);
			DISALLOW_COPY_AND_ASSIGN(socket_win);
		};

	}  // namespace binding
}  // namespace client

#endif  // SOCKET_WIN_H_
#endif