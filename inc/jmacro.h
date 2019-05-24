#pragma once


#define ESTR(expression)																#expression

#define JSTRUCT(struct_name)															struct struct_name : public jstruct_base

#define JTRUCT_INIT_FIELD_ZERO(struct_name, field_name)									memset((byte*)this + offsetof(struct_name, field_name), 0, sizeof(field_name))

#define JSTRUCT_REGISTER_FIELD(qualifier, field_name)									register_field(&typeid(field_name), #qualifier, #field_name, &field_name, 0)

#define JSTRUCT_REGISTER_FIELD_ALIAS(qualifier, field_name, alias_name)					register_field(&typeid(field_name), #qualifier, #field_name##"@"##alias_name, &field_name, 0)

#define JSTRUCT_REGISTER_NESTED_FIELD_ARRAY(qualifier, field_name)						register_field(&typeid(field_name), #qualifier, #field_name, &field_name, sizeof(field_name[0]))

#define JSTRUCT_REGISTER_NESTED_FIELD_ARRAY_ALIAS(qualifier, field_name, alias_name)	register_field(&typeid(field_name), #qualifier, #field_name##"@"##alias_name, &field_name, sizeof(field_name[0]))
