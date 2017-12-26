/*
 * Copyright (C) Entrix, 2015-20151.
 * All rights reserved.
 *
 * This software is covered by the license agreement between
 * the end user and Entrix, and may be used and copied only in
 * accordance with the terms of the
 * said agreement.
 *
 * Entrix assumes no responsibility or liability for any errors
 * or inaccuracies in this software, or any consequential,
 * incidental or indirect damage arising out of the use of the software.
 *
 ******************************************************************************
 *
 *  File:            $Workfile:    xmlWrapper.h  $
 *
 *  Date:            $Date: 2015/11/05
 *
 *  Description:     Contain functions for wrapper of xpath
 *
 ******************************************************************************
 */

#ifndef XMLWRAPPER_H_
#define XMLWRAPPER_H_

#include <string>
#include "libxml/tree.h"
#include "libxml/xpath.h"

namespace client {
	namespace binding {

		using namespace std;

		class CXmlWrapper {
		public:
			CXmlWrapper();
			CXmlWrapper(xmlDocPtr doc);
			virtual ~CXmlWrapper();

		public:
			enum XPathRtn {
				RTN_FAIL = -1,
				RTN_SUCCESS = 0,
				RTN_NOT_MATCH_DATA_TYPE = 1
			};

		protected:
			bool init();
			bool ready();
			bool selectInside(const string& xpathExp);
			bool registerNamespace(const string& nmspace, const string& alias);
			bool compareNamespace(const string& nmspace, const string& comp) const;
			bool insertChildNode(const string& xpathParent, const string& newNodeName, const string& newNodeValue);
			bool changeFileDataNodeValue(const string& xpath, const string& nodeVal);

		public:
			bool parse(const string& path);
			bool parse(const char* buf, const int& bufLen);
			void reset();
			const string getSignatureNamespace() const;
			const string getSignatureNamespaceAlias() const;
			const string getSigPropertiesNamespace() const;
			const string getSigPropertiesNamespaceAlias() const;

			const XPathRtn selectNode(const string& xpathExp, xmlNodeSetPtr& rtn);
			const XPathRtn selectBool(const string& xpathExp, bool& rtn);
			const XPathRtn selectString(const string& xpathExp, string& rtn);
			const XPathRtn selectCount(const string& xpathExp, int& rtn);
			const XPathRtn selectInt(const string& xpathExp, int& rtn);
			bool setNodeValue(const string& xpathExp,
				const string& xpathParent,
				const string& nodeName,
				const string& nodeVal);

			void setXmlDoc(const xmlDocPtr doc);
			string dumpXml();
		protected:
			xmlDocPtr m_xmlDocPtr;
			xmlXPathContextPtr m_xpathCtx;
			xmlXPathObjectPtr m_xpathObj;
			string m_nsXml;
			string m_nsProperty;
		};

	}  // namespace binding
}  // namespace client

#endif /* XMLWRAPPER_H_ */
