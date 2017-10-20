#ifndef _SIRIUS_XML_PARSER_H_
#define _SIRIUS_XML_PARSER_H_

#include <tinyxml.h>

#include <string>
#if defined(EXPORT_XML_PARSER_LIB)
#define EXP_XML_PARSER_CLASS __declspec(dllexport)
#else
#define EXP_XML_PARSER_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace misc
		{
			namespace xml
			{
				class EXP_XML_PARSER_CLASS parser
				{
				private:
					TiXmlDocument* _doc;

				public:
					parser(void);
					virtual ~parser(void);

					//Create
					TiXmlDocument*	create_new_instance(void);
					TiXmlDocument*	create_new_instance(const char* version, const char* encoding, const char* standalone);
					TiXmlElement*	create_root_element(TiXmlDocument* pRoot, const char* name);
					TiXmlElement*	create_child_element(TiXmlElement* pElem, const char* name);

					//Find
					TiXmlElement*	get_element_handle(TiXmlElement* pElem, const char* pName);
					int				get_child_element_count_by_name(TiXmlElement* pElem, char* name);

					//element
					void			set_element(TiXmlElement* pElem, const char* pName);
					const char*		get_element(TiXmlElement* pElem);

					//attribute
					const char*		get_attribute(TiXmlElement* pElem, const char* key);
					void			set_attribute(TiXmlElement* pElem, const char* key, const char* value);

					//text
					const char*		get_text(TiXmlElement* pElem);
					void			set_text(TiXmlElement* pElem, const char* text);

					//xml <--> string
					std::string		convert_to_string(TiXmlNode* pNode);
					TiXmlDocument*	convert_to_xml(const char* str);
				};
			};
		};
	};
};


#endif