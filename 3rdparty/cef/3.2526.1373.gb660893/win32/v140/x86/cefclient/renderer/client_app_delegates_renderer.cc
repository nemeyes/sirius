// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/renderer/client_app_renderer.h"
#include "cefclient/renderer/client_renderer.h"
#include "cefclient/renderer/performance_test.h"
#if defined(WITH_EXTERNAL_INTERFACE)
#include "cefclient/binding/js_binding.h"
#else
#include "cefclient/renderer/performance_test.h"
#endif
namespace client {

// static
void ClientAppRenderer::CreateDelegates(DelegateSet& delegates) {
  renderer::CreateDelegates(delegates);
#if defined(WITH_EXTERNAL_INTERFACE)
  binding::CreateDelegates(delegates);
#else
  performance_test::CreateDelegates(delegates);
#endif
}

}  // namespace client
