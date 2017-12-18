#ifndef _CONFIGURATION_DAO_H_
#define _CONFIGURATION_DAO_H_

#include <sirius_uuid.h>
#include "data_access_object.h"
#include "configuration_entity.h"

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{
				namespace db
				{
					class configuration_dao
						: public sirius::app::server::arbitrator::db::data_access_object
					{
					public:
						configuration_dao(const char * path);
						virtual ~configuration_dao(void);

						int32_t update(sirius::app::server::arbitrator::entity::configuration_t * entity, sqlite3 * connection=nullptr);
						int32_t retrieve(sirius::app::server::arbitrator::entity::configuration_t * entity, sqlite3 * connection = nullptr);

						int32_t create(sirius::app::server::arbitrator::entity::configuration_t * entity, sqlite3 * connection = nullptr);
					private:
						int32_t retrieve_count(sqlite3 * connection = nullptr);
					};
				};
			};
		};
	};
};















#endif
