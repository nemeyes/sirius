
#include <string>
#include "D3D9.h"

using namespace std;
namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace backend
			{
				class device_manager
				{
				public:
					device_manager();
					~device_manager();
					static char* get_cpu_name();
					static char* get_operatingsystem_name();
					static int get_memory_info();
				};
			};
		};
	};
};


