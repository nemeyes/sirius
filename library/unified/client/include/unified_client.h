#ifndef _UNIFIED_MEDIA_CLIENT_H_
#define _UNIFIED_MEDIA_CLIENT_H_

#include <cstdint>
#include <sirius_unified_client.h>
#include "sirius_scsp_receiver.h"
#include "sirius_file_receiver.h"
#include <map>

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			class client::core
			{
			public:
				core(sirius::library::unified::client * front);
				virtual ~core(void);

				int32_t state(void);

				int32_t open(wchar_t * url, int32_t port, int32_t recv_option, bool repeat);
				int32_t play(void);
				int32_t stop(void);

			private:
				sirius::library::unified::client *	_front;
				int32_t						_state;
				char						_url[200];
				int32_t						_port;
				int32_t						_transport_option;
				int32_t						_recv_option;
				int32_t						_recv_timeout;
				bool						_repeat;

				sirius::library::unified::file::receiver * _file_receiver;
				sirius::library::unified::scsp::receiver * _scsp_receiver;

				CRITICAL_SECTION			_cs;

				bool						_bfile;
			};
		};
	};
};

#endif
