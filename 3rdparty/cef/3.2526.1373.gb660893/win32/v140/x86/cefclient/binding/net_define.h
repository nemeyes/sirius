#ifndef NET_DEFINE_H_
#define NET_DEFINE_H_

#include <vector>

namespace client {
	namespace binding {
		typedef unsigned char byte;

		enum COMMAND_TYPE {
			REQUEST = 0,
			RESPONSE = 1,
			REPORT = 2
		};
		enum DATA_TYPE {
			STRING = 0,
			NUMBER
		};
		enum CONTENTS_TYPE {
			CONNECT = 0,
			DISCONNECT,
			HISTORYREQUESTINFO,
			CONTENTS_URL,
			MENUID_URL,
			MENU_HISTORY,
			APPID,
			APPHISTORY,
			RESPONSE_CTS,
			TOAPP = 10,			// STB -> APP	
			TOSTB,				// App -> STB
			SEND_TO_AND_STOP_ON_SIRIUS = 23	  // SEND TO sirius And Stop on sirius
		};
		typedef struct _HEADER {
			int bodyLength;
			byte commandType;
			byte contentsType;
			byte dataType;
			_HEADER() {
				bodyLength = 0;
				commandType = 0;
				contentsType = 0;
				dataType = 0;
			}
		}HEADER;
		typedef struct _XML_DATA {
			CefString command;
			CefString group;
			CefString gtype;
			CefString data;
		}XML_DATA;
	}  // namespace binding
}  // namespace client

#endif  // NET_DEFINE_H_
