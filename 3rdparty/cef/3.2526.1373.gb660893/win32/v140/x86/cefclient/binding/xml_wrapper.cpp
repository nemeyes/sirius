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
*  File:            $Workfile:    xmlWrapper.cpp  $
*
*  Date:            $Date: 2015/11/05
*
*  Description:     Contain functions for wrapper of xpath
*
******************************************************************************
*/

#include <sstream>
#include "Windows.h"
#include "xml_wrapper.h"
#include "libxml/xpathInternals.h"
#include "libxml/xpath.h"

using namespace std;

namespace client {
	namespace binding {

		const string NAMESPACE_XML_SIGNATURE = "http://www.w3.org/2000/09/xmldsig#";
		const string NAMESPACE_ALIAS_XML_SIGNATURE = "ns";
		const string NAMESPACE_XML_SIGNATURE_PROPERTIES = "http://www.w3.org/2009/xmldsig-properties";
		const string NAMESPACE_ALIAS_XML_SIGNATURE_PROPERTIES = "dsp";

		inline bool CXmlWrapper::ready() {
			if ((m_xmlDocPtr == NULL) || (xmlDocGetRootElement(m_xmlDocPtr) == NULL))
				return false;
			if (m_xpathCtx == NULL)
				return false;
			return true;
		}

		inline void ReleaseXmlChar(xmlChar* str) {
			if (str) {
				xmlFree(str);
				str = NULL;
			}
		}

		CXmlWrapper::CXmlWrapper()
			: m_xmlDocPtr(NULL)
			, m_xpathCtx(NULL)
			, m_xpathObj(NULL)
			, m_nsXml("")
			, m_nsProperty("") {
		}

		//Usage-----------------------------------------------------------------
		//xmlDocPtr doc = xmlParseFile(xmlPath);
		//CXmlWrapper p(doc);
		//Usage-----------------------------------------------------------------
		CXmlWrapper::CXmlWrapper(xmlDocPtr doc) {
			m_xmlDocPtr = doc;
			init();
		}

		CXmlWrapper::~CXmlWrapper() {
			reset();
			if (m_xpathCtx)
				xmlXPathFreeContext(m_xpathCtx);
		}

		void CXmlWrapper::setXmlDoc(const xmlDocPtr doc) {
			m_xmlDocPtr = doc;
			init();
			if (!ready())
				OutputDebugStringA("AMCore CXmlWrapper::setXmlDoc : failed ready");
		}

		//Usage-----------------------------------------------------------------
		//xmlDocPtr doc = xmlParseFile(xmlPath);
		//CXmlWrapper p(doc);
		//p.init();
		//Usage-----------------------------------------------------------------
		bool CXmlWrapper::init() {
			m_xpathCtx = xmlXPathNewContext(m_xmlDocPtr);

			xmlNsPtr* ns = xmlGetNsList(m_xmlDocPtr, xmlDocGetRootElement(m_xmlDocPtr));
			if (ns) {
				m_nsXml = (const char*)(*ns)->href;
				if (!compareNamespace(m_nsXml, NAMESPACE_XML_SIGNATURE))
					return false;
				registerNamespace(m_nsXml.c_str(), NAMESPACE_ALIAS_XML_SIGNATURE.c_str());
				xmlFree(ns);
			}

			ostringstream os;
			os << "/" << NAMESPACE_ALIAS_XML_SIGNATURE << ":Signature";
			os << "/" << NAMESPACE_ALIAS_XML_SIGNATURE << ":Object";
			os << "/" << NAMESPACE_ALIAS_XML_SIGNATURE << ":SignatureProperties";
			xmlNodeSetPtr nodeSet;
			if (RTN_SUCCESS != selectNode(os.str(), nodeSet))
				return false;

			if ((nodeSet) && (nodeSet->nodeTab)) {
				xmlNsPtr* ns1 = xmlGetNsList(m_xmlDocPtr, *(nodeSet->nodeTab));
				if (ns1) {
					m_nsProperty = (const char*)(*ns1)->href;
					if (!compareNamespace(m_nsProperty, NAMESPACE_XML_SIGNATURE_PROPERTIES))
						return false;
					registerNamespace(m_nsProperty.c_str(), NAMESPACE_ALIAS_XML_SIGNATURE_PROPERTIES);
					xmlFree(ns1);
				}
			}
			else {
				OutputDebugStringA("AMCore CXmlWrapper::init Initialize Error");
				return false;
			}

			return true;
		}

		void CXmlWrapper::reset() {
			if (m_xpathObj)
				xmlXPathFreeObject(m_xpathObj);
		}

		//Usage-----------------------------------------------------------------
		//xmlDocPtr doc = xmlParseFile(xmlPath);
		//CXmlWrapper p(doc);
		//p.init();
		//p.registerNamespace("http://www.w3.org/2000/09/xmldsig#","ns");
		//p.registerNamespace("http://www.w3.org/2009/xmldsig-properties#","ds");   //namespace add
		//Usage-----------------------------------------------------------------
		bool CXmlWrapper::registerNamespace(const string& nmspace, const string& alias) {
			xmlChar* sigNm = xmlCharStrdup(nmspace.c_str());
			xmlChar* sigNmAlias = xmlCharStrdup(alias.c_str());
			if (0 > xmlXPathRegisterNs(m_xpathCtx, sigNmAlias, sigNm)) {
				return false;
			}
			ReleaseXmlChar(sigNm);
			ReleaseXmlChar(sigNmAlias);
			return true;
		}

		bool CXmlWrapper::parse(const string& path) {
			m_xmlDocPtr = xmlParseFile(path.c_str());
			m_xpathCtx = xmlXPathNewContext(m_xmlDocPtr);
			//init();
			if (!ready()) {
				OutputDebugStringA("CXmlWrapper::setXmlDoc : failed ready");
				return false;
			}
			return true;
		}

		bool CXmlWrapper::parse(const char* buf, const int& bufLen) {
			m_xmlDocPtr = xmlParseMemory(buf, bufLen);
			m_xpathCtx = xmlXPathNewContext(m_xmlDocPtr);
			//init();
			if (!ready()) {
				OutputDebugStringA("CXmlWrapper::setXmlDoc : failed ready");
				return false;
			}
			return true;
		}

		//Usage-----------------------------------------------------------------
		//xmlDocPtr doc = xmlParseFile(xmlPath);
		//CXmlWrapper p(doc);
		//p.registerNamespace("http://www.w3.org/2000/09/xmldsig#","ns");
		//p.registerNamespace("http://www.w3.org/2009/xmldsig-properties#","ds");   //namespace add
		//xmlNodeSetPtr pt = p.select("/ds:Signature/ds:Object[@Id='prop']");
		//if(!pt)
		//    return NULL;
		//p.reset();
		//pt = p.select(xpathExpression);
		//Usage-----------------------------------------------------------------
		bool CXmlWrapper::selectInside(const string& xpathExp) {
			xmlChar* xpathExpr = xmlCharStrdup(xpathExp.c_str());
			m_xpathObj = xmlXPathEvalExpression(xpathExpr, m_xpathCtx);
			ReleaseXmlChar(xpathExpr);
			if (m_xpathObj == NULL) {
				return false;
			}
			return true;
		}

		const CXmlWrapper::XPathRtn CXmlWrapper::selectNode(const string& xpathExp, xmlNodeSetPtr& rtn) {
			if (!selectInside(xpathExp))
				return RTN_FAIL;

			if (m_xpathObj->type != XPATH_NODESET) {
				reset();
				return RTN_NOT_MATCH_DATA_TYPE;
			}

			rtn = m_xpathObj->nodesetval;

			return RTN_SUCCESS;
		}

		const CXmlWrapper::XPathRtn CXmlWrapper::selectBool(const string& xpathExp, bool& rtn) {
			if (!selectInside(xpathExp))
				return RTN_FAIL;

			if (m_xpathObj->type != XPATH_BOOLEAN) {
				reset();
				return RTN_NOT_MATCH_DATA_TYPE;
			}
			m_xpathObj->boolval == 0 ? rtn = false : rtn = true;
			return RTN_SUCCESS;
		}

		const CXmlWrapper::XPathRtn CXmlWrapper::selectString(const string& xpathExp, string& rtn) {
			if (!selectInside(xpathExp))
				return RTN_FAIL;

			if (m_xpathObj->type != XPATH_STRING) {
				reset();
				return RTN_NOT_MATCH_DATA_TYPE;
			}

			if (string((const char*)m_xpathObj->stringval).empty()) {
				return RTN_FAIL;
			}

			rtn = (const char*)m_xpathObj->stringval;

			return RTN_SUCCESS;
		}

		const CXmlWrapper::XPathRtn CXmlWrapper::selectCount(const string& xpathExp, int& rtn) {
			if (!selectInside(xpathExp))
				return RTN_FAIL;

			if (m_xpathObj->type != XPATH_NUMBER) {
				reset();
				return RTN_NOT_MATCH_DATA_TYPE;
			}
			rtn = m_xpathObj->floatval;
			return RTN_SUCCESS;
		}

		const CXmlWrapper::XPathRtn CXmlWrapper::selectInt(const string& xpathExp, int& rtn) {
			if (!selectInside(xpathExp))
				return RTN_FAIL;

			if (m_xpathObj->type != XPATH_STRING) {
				reset();
				return RTN_NOT_MATCH_DATA_TYPE;
			}

			if (string((const char*)m_xpathObj->stringval).empty()) {
				return RTN_FAIL;
			}

			stringstream((const char*)m_xpathObj->stringval) >> rtn;
			return RTN_SUCCESS;
		}

		const string CXmlWrapper::getSignatureNamespace() const {
			return m_nsXml;
		}

		const string CXmlWrapper::getSignatureNamespaceAlias() const {
			return NAMESPACE_ALIAS_XML_SIGNATURE;
		}

		const string CXmlWrapper::getSigPropertiesNamespace() const {
			return m_nsProperty;
		}

		const string CXmlWrapper::getSigPropertiesNamespaceAlias() const {
			return NAMESPACE_ALIAS_XML_SIGNATURE_PROPERTIES;
		}

		bool CXmlWrapper::compareNamespace(const string& nmspace, const string& comp) const {
			if (nmspace.empty())
				return false;
			if (nmspace != comp)
				return false;

			return true;
		}

		bool CXmlWrapper::setNodeValue(const string& xpathExp,
			const string& xpathParent,
			const string& nodeName,
			const string& nodeVal) {
			xmlNodeSetPtr nodeSet;
			if (XPathRtn::RTN_SUCCESS != selectNode(xpathExp, nodeSet)) {
				return false;
			}

			return (nodeSet->nodeNr == 0) ?
				insertChildNode(xpathParent, nodeName, nodeVal) :
				changeFileDataNodeValue(xpathExp, nodeVal);
		}

		bool CXmlWrapper::changeFileDataNodeValue(const string& xpath,
			const string& nodeVal) {
			xmlNodeSetPtr nodeSet;
			if (XPathRtn::RTN_SUCCESS != selectNode(xpath, nodeSet)) {
				return false;
			}
			if (nodeSet->nodeNr == 0)
				return false;

			xmlChar* nodeValue = xmlCharStrdup(nodeVal.c_str());
			xmlNodePtr newNode = xmlNewCDataBlock(m_xmlDocPtr, nodeValue, nodeVal.length());
			xmlUnlinkNode(nodeSet->nodeTab[0]->children->next);
			xmlAddChild(nodeSet->nodeTab[0], newNode);

			return true;
		}

		bool CXmlWrapper::insertChildNode(const string& xpathParent,
			const string& newNodeName,
			const string& newNodeValue) {
			xmlNodeSetPtr nodeSet;
			if (XPathRtn::RTN_SUCCESS != selectNode(xpathParent, nodeSet)) {
				return false;
			}

			if (nodeSet->nodeNr == 0)
				return false;

			xmlChar* nodeName = xmlCharStrdup(newNodeName.c_str());
			xmlChar* nodeValue = xmlCharStrdup(newNodeValue.c_str());
			xmlNodePtr child = xmlNewChild(nodeSet->nodeTab[0], nullptr, nodeName, nodeValue);

			return (child == nullptr) ? false : true;
		}

		string CXmlWrapper::dumpXml() {
			if (!m_xmlDocPtr)
				return "";

			xmlChar* xmlbuff;
			int buffersize = 0;
			xmlDocDumpFormatMemory(m_xmlDocPtr, &xmlbuff, &buffersize, 1);
			return (char*)xmlbuff;
		}

	}  // namespace binding
}  // namespace client
