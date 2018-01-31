#pragma once
#include <cstdint>

//#include "event_log_parser.h"

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace manager
			{
				class monitor
				{
				public:

					monitor();
					~monitor();

					bool init();
					bool uninit();

					bool is_run() { return _run; };

				private:

					static unsigned __stdcall process_cb(void * param);
					void process();

					HANDLE	_thread;
					bool _run;
				};
			};
		};
	};
};