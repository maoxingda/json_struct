#pragma once


#define ESTR(expression) #expression

#define JSTRUCT_INIT_FIELD_ZERO(      struct_name, field_name) memset((char*)this + offsetof(struct_name, field_name), 0, sizeof(field_name))

//////////////////////////////////////////////////////////////////////////

#define JSTRUCT_REG_BASIC_FIELD(       qualifier, field_name) register_field(&typeid(field_name), #qualifier, #field_name, "", &field_name, 0, 0)

#define JSTRUCT_REG_CUSTOM_FIELD(      qualifier, field_name) register_field(&typeid(field_name), #qualifier, #field_name, "", &field_name, 0, 0)

#define JSTRUCT_REG_BASIC_ARRAY_FIELD( qualifier, field_name) register_field(&typeid(field_name), #qualifier, #field_name, "", &field_name, &field_name##_size, sizeof(field_name[0]))

#define JSTRUCT_REG_CUSTOM_ARRAY_FIELD(qualifier, field_name) register_field(&typeid(field_name), #qualifier, #field_name, "", &field_name, &field_name##_size, sizeof(field_name[0]))

//////////////////////////////////////////////////////////////////////////

#define ALIAS(                                                      alias_name)

#define JSTRUCT_REG_BASIC_FIELD_ALIAS(       qualifier, field_name, alias_name) register_field(&typeid(field_name), #qualifier, #field_name, #alias_name, &field_name, 0, 0)

#define JSTRUCT_REG_CUSTOM_FIELD_ALIAS(      qualifier, field_name, alias_name) register_field(&typeid(field_name), #qualifier, #field_name, #alias_name, &field_name, 0, 0)

#define JSTRUCT_REG_BASIC_ARRAY_FIELD_ALIAS( qualifier, field_name, alias_name) register_field(&typeid(field_name), #qualifier, #field_name, #alias_name, &field_name, &field_name##_size, sizeof(field_name[0]))

#define JSTRUCT_REG_CUSTOM_ARRAY_FIELD_ALIAS(qualifier, field_name, alias_name) register_field(&typeid(field_name), #qualifier, #field_name, #alias_name, &field_name, &field_name##_size, sizeof(field_name[0]))
