#include "cefclient/common/client_switches.h"
#include "attendent_switches.h"
#include "global.h"
#include "include/cef_version.h"

namespace client {
	namespace binding {

		global* global::sInstance = nullptr;

		global::global() {}

		global& global::getInstance() {
			if (sInstance == nullptr)
				sInstance = new global;

			return *sInstance;
		}
	}  // namespace binding
}  // namespace client
