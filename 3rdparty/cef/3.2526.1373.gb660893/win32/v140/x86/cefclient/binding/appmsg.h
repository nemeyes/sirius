#ifndef APP_MSG_H_
#define APP_MSG_H_
#pragma once

#include "include/cef_base.h"
#include "xml_wrapper.h"

namespace client {
	namespace binding {

#define GROUP_ANIMATION_INFO "Animation_Info"
#define GROUP_LFS "File_System"
#define GTYPE_LFS "LFS"
#define GROUP_SIRIUS "SIRIUS"

		class appmsg : public virtual CefBase {
		public:
			appmsg();
			virtual ~appmsg();

			bool parse(const CefString& xml);
			CefString getGroupString();
			CefString getGtypeString();
			CefString getString(CefString& xpath);
			int countNode(CefString& nodeExp);
			bool setNodeValue(const string& xpathExp,
				const string& xpathParent,
				const string& nodeName,
				const string& nodeVal);
			CefString dumpXml();

		protected:
			CXmlWrapper xmlWrapper_;

			IMPLEMENT_REFCOUNTING(appmsg);
			DISALLOW_COPY_AND_ASSIGN(appmsg);
		};

	}  // namespace binding
}  // namespace client

#endif  // APP_MSG_H_
