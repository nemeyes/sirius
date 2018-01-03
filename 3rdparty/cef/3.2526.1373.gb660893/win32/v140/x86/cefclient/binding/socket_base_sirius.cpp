// SocketBase.cpp: implementation of the socketbase class.
//
//////////////////////////////////////////////////////////////////////
#if defined(WITH_EXTERNAL_INTERFACE)
#include <process.h>
#include <assert.h>
#include "socket_win.h"
#include "socket_base_sirius.h"
#include "attendant_proxy_wrapper.h"
#pragma comment(lib, "Ws2_32.lib")

namespace client {
	namespace binding {

#ifdef _DEBUG
#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

		socketbase::socketbase() {
			m_bThreadWhile = TRUE;
			m_hSocket = NULL;
			m_hEvent = NULL;
			client::binding::attendant_proxy_wrapper& apc = attendant_proxy_wrapper::getInstance();
			_attendant = apc._proxy;
			_attendant->set_attendant_cb(&AttendantToAppCalback);
		}

		socketbase::~socketbase() {

		}

		void socketbase::ProcessingThread()
		{

		}

		int socketbase::Send(const void *lpBuf, int nBuflen, int nFlags)
		{
			_attendant->app_to_attendant((uint8_t *)lpBuf, nBuflen);
			return true;
		}

		bool socketbase::Create(UINT nSocketPort, int nSocketType, LPCTSTR lpszSocketAddress)
		{
			return true;
		}

		void socketbase::AttendantToAppCalback(uint8_t* packet, size_t len)
		{
			socket_win::getInstance()->SiriusToJSEngine((uint8_t*)packet, len);
		}

	}  // namespace binding
}  // namespace client
#endif