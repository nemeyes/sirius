#ifndef _SESSOIN_H_
#define _SESSOIN_H_

#include <sirius.h>

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{			
				class session					
				{
				public:
					session(int32_t id);
					~session(void);

					int32_t			id(void);
					int32_t			pid(void);
					int32_t			state(void);
					uint32_t		total_bandwidth_byte(void);
					const char *	attendant_uuid(void);		
					const char *	client_uuid(void);
					const char *	client_id(void);

					void			pid(int32_t pid);
					void			state(int32_t state);
					void			total_bandwidth_byte(uint32_t total_bandwidth_byte);
					void			attendant_uuid(const char * uuid);
					void			client_uuid(const char * uuid);					
					void			client_id(const char * client_id);	
					
				private:
					int32_t		_id;
					int32_t		_pid;
					int32_t		_state;
					int32_t		_total_bandwidth_byte;
					char		_attendant_uuid[64];
					char		_client_uuid[64];
					char		_client_id[MAX_PATH];	

					CRITICAL_SECTION _lock;
				};				
			};
		};
	};
};










#endif