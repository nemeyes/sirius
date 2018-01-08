#include <winsock2.h>
#include "include/cef_base.h"
#include "include/base/cef_macros.h"
#include "include/base/cef_scoped_ptr.h"
#include "macro.h"
#include "sirius_attendant_proxy.h"
#if !defined(AFX_SOCKETBASE_H__617EC439_8FD1_4CB1_8129_BDB86169E8C3__INCLUDED_)
#define AFX_SOCKETBASE_H__617EC439_8FD1_4CB1_8129_BDB86169E8C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace client {
	namespace binding {

#define MAX_BUFFER_SIZE 1024*4

		struct ISocketThread
		{
			virtual void ProcessingThread() = 0;
		};

		class socketbase : public ISocketThread, CefBase {
		public:
			bool create(UINT nSocketPort = 0, int nSocketType = SOCK_STREAM, LPCTSTR lpszSocketAddress = NULL);
			int send_data(const void *lpBuf, int nBuflen, int nFlags = 0);
			virtual void ProcessingThread();
			static void calback_attendant_to_app(uint8_t* packet, size_t len);
			socketbase();
			virtual ~socketbase();
		public:

		protected:
			WSAEVENT _event;
			bool _thread_while;
			SOCKET _socket;
			DWORD _dw_last_error;
		private:

		private:
			sirius::app::attendant::proxy * _attendant;
			IMPLEMENT_REFCOUNTING(socketbase);
			DISALLOW_COPY_AND_ASSIGN(socketbase);
		};

	}  // namespace binding
}  // namespace client

#endif // !defined(AFX_SOCKETBASE_H__617EC439_8FD1_4CB1_8129_BDB86169E8C3__INCLUDED_)
