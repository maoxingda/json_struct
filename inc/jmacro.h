#pragma once


#define ESTR(expression) #expression

#define JSTRUCT_INIT_FIELD_ZERO(      struct_name, field_name) memset((char*)this + offsetof(struct_name, field_name), 0, sizeof(field_name))

//////////////////////////////////////////////////////////////////////////

#define JSTRUCT_REG_BOOL_FIELD(       qualifier, field_name) register_field(typeid(field_name).name(), #qualifier, #field_name, "", &field_name, 0, 0)

#define JSTRUCT_REG_NUMBER_FIELD(       qualifier, field_name) register_field(typeid(field_name).name(), #qualifier, #field_name, "", &field_name, 0, 0)

#define JSTRUCT_REG_WCHAR_ARRAY_FIELD(       qualifier, field_name) register_field(typeid(field_name).name(), #qualifier, #field_name, "", &field_name, 0, 0)

#define JSTRUCT_REG_STRUCT_FIELD(      qualifier, field_name) register_field(typeid(field_name).name(), #qualifier, #field_name, "", &field_name, 0, 0)

#define JSTRUCT_REG_NUMBER_ARRAY_FIELD( qualifier, field_name) register_field(typeid(field_name).name(), #qualifier, #field_name, "", &field_name, &field_name##_size, sizeof(field_name[0]))

#define JSTRUCT_REG_WCHAR_TABLE_FIELD( qualifier, field_name) register_field(typeid(field_name).name(), #qualifier, #field_name, "", &field_name, &field_name##_size, sizeof(field_name[0]))

#define JSTRUCT_REG_STRUCT_ARRAY_FIELD(qualifier, field_name) register_field(typeid(field_name).name(), #qualifier, #field_name, "", &field_name, &field_name##_size, sizeof(field_name[0]))

//////////////////////////////////////////////////////////////////////////

#define JSTRUCT_REG_BOOL_FIELD_ALIAS(       qualifier, field_name, alias_name) register_field(typeid(field_name).name(), #qualifier, #field_name, #alias_name, &field_name, 0, 0)

#define JSTRUCT_REG_NUMBER_FIELD_ALIAS(       qualifier, field_name, alias_name) register_field(typeid(field_name).name(), #qualifier, #field_name, #alias_name, &field_name, 0, 0)

#define JSTRUCT_REG_WCHAR_ARRAY_FIELD_ALIAS(       qualifier, field_name, alias_name) register_field(typeid(field_name).name(), #qualifier, #field_name, #alias_name, &field_name, 0, 0)

#define JSTRUCT_REG_STRUCT_FIELD_ALIAS(      qualifier, field_name, alias_name) register_field(typeid(field_name).name(), #qualifier, #field_name, #alias_name, &field_name, 0, 0)

#define JSTRUCT_REG_NUMBER_ARRAY_FIELD_ALIAS( qualifier, field_name, alias_name) register_field(typeid(field_name).name(), #qualifier, #field_name, #alias_name, &field_name, &field_name##_size, sizeof(field_name[0]))

#define JSTRUCT_REG_WCHAR_TABLE_FIELD_ALIAS( qualifier, field_name, alias_name) register_field(typeid(field_name).name(), #qualifier, #field_name, #alias_name, &field_name, &field_name##_size, sizeof(field_name[0]))

#define JSTRUCT_REG_STRUCT_ARRAY_FIELD_ALIAS( qualifier, field_name, alias_name) register_field(typeid(field_name).name(), #qualifier, #field_name, #alias_name, &field_name, &field_name##_size, sizeof(field_name[0]))
