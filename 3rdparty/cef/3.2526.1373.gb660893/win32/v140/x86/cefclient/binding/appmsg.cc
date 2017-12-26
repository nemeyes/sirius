#include "appmsg.h"

using namespace std;

namespace client {
	namespace binding {

		appmsg::appmsg() {
		}

		appmsg::~appmsg() {
		}

		bool appmsg::parse(const CefString& xml) {
			std::string utf8_string = xml.ToString();
			return xmlWrapper_.parse(utf8_string.c_str(), utf8_string.length());
		}

		CefString appmsg::getGroupString() {
			CefString xpath = "string(/INTERFACE/GROUP/text())";
			return getString(xpath);
		}

		CefString appmsg::getGtypeString() {
			CefString xpath = "string(/INTERFACE/GTYPE/text())";
			return getString(xpath);
		}

		CefString appmsg::getString(CefString& xpath) {
			string xpathExp = xpath.ToString();
			string tmpVal;
			if (CXmlWrapper::XPathRtn::RTN_SUCCESS !=
				xmlWrapper_.selectString(xpathExp, tmpVal)) {
				return "";
			}

			CefString val;
			val.FromString(tmpVal);
			return val;
		}

		int appmsg::countNode(CefString& nodeExp) {
			string xpathExp = nodeExp.ToString();
			int tmpVal;
			if (CXmlWrapper::XPathRtn::RTN_SUCCESS !=
				xmlWrapper_.selectCount(xpathExp, tmpVal)) {
				return 0;
			}
			return tmpVal;
		}

		bool appmsg::setNodeValue(const string& xpathExp,
			const string& xpathParent,
			const string& nodeName,
			const string& nodeVal) {
			return xmlWrapper_.setNodeValue(xpathExp, xpathParent, nodeName, nodeVal);
		}

		CefString appmsg::dumpXml() {
			CefString xml;
			xml.FromString(xmlWrapper_.dumpXml());
			return xml;
		}

	}  // namespace binding
}  // namespace client
