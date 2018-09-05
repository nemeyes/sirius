#ifndef HANDLER_BASE_H_
#define HANDLER_BASE_H_

#include "include/cef_values.h"
#include "include/cef_v8.h"

namespace client {
		class message_handler_base /*: public CefBase*/ {
		public:
			message_handler_base() {};
			virtual ~message_handler_base() 
			{
			};

			void set_list_Value(CefRefPtr<CefListValue> list, int index,
				CefRefPtr<CefV8Value> value) {
				if (value->IsArray()) {
					CefRefPtr<CefListValue> new_list = CefListValue::Create();
					set_list(value, new_list);
					list->SetList(index, new_list);
				}
				else if (value->IsString()) {
					list->SetString(index, value->GetStringValue());
				}
				else if (value->IsBool()) {
					list->SetBool(index, value->GetBoolValue());
				}
				else if (value->IsInt()) {
					list->SetInt(index, value->GetIntValue());
				}
				else if (value->IsDouble()) {
					list->SetDouble(index, value->GetDoubleValue());
				}
			}

			void set_list_Value(CefRefPtr<CefV8Value> list, int index,
				CefRefPtr<CefListValue> value) {
				CefRefPtr<CefV8Value> new_value;
				CefValueType type = value->GetType(index);
				switch (type) {
				case VTYPE_LIST: {
					CefRefPtr<CefListValue> listTemp = value->GetList(index);
					new_value = CefV8Value::CreateArray(static_cast<int>(listTemp->GetSize()));
					set_list(listTemp, new_value);
				} break;
				case VTYPE_BOOL:
					new_value = CefV8Value::CreateBool(value->GetBool(index));
					break;
				case VTYPE_DOUBLE:
					new_value = CefV8Value::CreateDouble(value->GetDouble(index));
					break;
				case VTYPE_INT:
					new_value = CefV8Value::CreateInt(value->GetInt(index));
					break;
				case VTYPE_STRING:
				{
					std::string str = "";
					str = value->GetString(index);
					//::OutputDebugStringA(str.c_str());
					new_value = CefV8Value::CreateString(value->GetString(index));
					break;
				}
				default:
					break;
				}

				if (new_value.get()) {
					if(list.get()!=nullptr)
						list->SetValue(index, new_value);
				}
				else {
					if (list.get() != nullptr)
						list->SetValue(index, CefV8Value::CreateNull());
				}
			}
			void set_list(CefRefPtr<CefV8Value> source, CefRefPtr<CefListValue> target) {
				int arg_length = source->GetArrayLength();
				if (arg_length == 0)
					return;

				// Start with null types in all spaces.
				target->SetSize(arg_length);

				for (int i = 0; i < arg_length; ++i)
					set_list_Value(target, i, source->GetValue(i));
			}
			void set_list(CefRefPtr<CefListValue> source, CefRefPtr<CefV8Value> target) {
				for (int i = 0; i < static_cast<int>(source->GetSize()); ++i)
					set_list_Value(target, i, source);
			}
		private:
			//IMPLEMENT_REFCOUNTING(message_handler_base);
			DISALLOW_COPY_AND_ASSIGN(message_handler_base);
		};
}

#endif