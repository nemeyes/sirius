#include "sirius_xml_parser.h"

sirius::library::misc::xml::parser::parser(void)
	: _doc(nullptr)
{
}

sirius::library::misc::xml::parser::~parser(void)
{
	if (_doc)
	{
		_doc->Clear();
		delete _doc;
    }
}

TiXmlDocument* sirius::library::misc::xml::parser::create_new_instance(void)
{
	if (_doc) return _doc;

	TiXmlDocument* root = new TiXmlDocument;
	//TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	//root->LinkEndChild(decl);

	_doc = root;
	return root;
}

TiXmlDocument* sirius::library::misc::xml::parser::create_new_instance(const char* version, const char* encoding, const char* standalone)
{
	if (_doc) return _doc;

	TiXmlDocument* root = new TiXmlDocument;
	TiXmlDeclaration* decl = new TiXmlDeclaration(version, encoding, standalone);
	root->LinkEndChild(decl);

	_doc = root;
	return root;
}

TiXmlElement* sirius::library::misc::xml::parser::create_root_element(TiXmlDocument* pRoot, const char* name)
{
	TiXmlElement* elem = new TiXmlElement(name);
	pRoot->LinkEndChild(elem);
	return elem;
}

TiXmlElement* sirius::library::misc::xml::parser::create_child_element(TiXmlElement* pElem, const char* name)
{
	TiXmlElement* elem = new TiXmlElement(name);
	pElem->LinkEndChild(elem);
	return elem;
}

TiXmlElement* sirius::library::misc::xml::parser::get_element_handle(TiXmlElement* pElem, const char* pName)
{
	return pElem->FirstChildElement(pName);
}
int	sirius::library::misc::xml::parser::get_child_element_count_by_name(TiXmlElement* pElem, char* name)
{
	int count = 0 ;
	for (TiXmlNode* node = pElem->FirstChildElement(name);	node; node = node->NextSiblingElement(name))
		count++;
	return count;
}

const char* sirius::library::misc::xml::parser::get_element(TiXmlElement* pElem)
{
	return pElem->Value();
}

void sirius::library::misc::xml::parser::set_element(TiXmlElement* pElem, const char* pName)
{
	return pElem->SetValue(pName);
}

const char*	sirius::library::misc::xml::parser::get_attribute(TiXmlElement* pElem, const char* key)
{
	return pElem->Attribute(key);
}

void sirius::library::misc::xml::parser::set_attribute(TiXmlElement* element, const char* key, const char* value)
{
	element->SetAttribute(key, value);

}

const char*	sirius::library::misc::xml::parser::get_text(TiXmlElement* pElem)
{
	return pElem->GetText();
}

void sirius::library::misc::xml::parser::set_text(TiXmlElement* pElem, const char* text)
{
	pElem->LinkEndChild(new TiXmlText(text));
}

std::string sirius::library::misc::xml::parser::convert_to_string(TiXmlNode* pNode)
{
	TiXmlPrinter printer;
	pNode->Accept(&printer);
	return printer.CStr();
}

TiXmlDocument* sirius::library::misc::xml::parser::convert_to_xml(const char* str)
{
	TiXmlDocument* doc = new TiXmlDocument;
	doc->Parse(str);
	TiXmlElement* a = doc->FirstChild()->ToElement();
	TiXmlElement* b = doc->FirstChildElement();
	return doc;
}