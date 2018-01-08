#include "cefclient/common/client_switches.h"
#include "attendant_switches.h"
#include "global.h"
#include "include/cef_version.h"

namespace client {
	namespace binding {

		global* global::_instance = nullptr;

		global::global() {}

		global& global::get_instance() {
			if (_instance == nullptr)
				_instance = new global;

			return *_instance;
		}
	}  // namespace binding
}  // namespace client
