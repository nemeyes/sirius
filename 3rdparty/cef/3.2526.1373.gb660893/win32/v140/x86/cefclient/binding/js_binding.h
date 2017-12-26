#if defined(WITH_JAVASCRIPT)
#ifndef JSBINDING_H_
#define JSBINDING_H_

#include "cefclient/renderer/client_app_renderer.h"

namespace client {
	namespace binding {
		void CreateDelegates(ClientAppRenderer::DelegateSet& delegates);

	}  // namespace binding
}  // namespace client

#endif  // JSBINDING_H_
#endif
