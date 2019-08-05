#pragma once
#include "../inc/jmacro.h"
#include "../inc/jqualifier.h"
#include <boost/xpressive/xpressive.hpp>


using namespace boost::xpressive;


// sub group
mark_tag alias_name(1);
mark_tag field_name(1);
mark_tag struct_name(1);
mark_tag qualifier_name(1);

// common regex expressions
static const sregex alpha_underscore    = (alpha | '_');
static const sregex identifier          = (alpha_underscore >> *(_d | alpha_underscore));
static const sregex number              = (as_xpr(ESTR(jint)) | ESTR(juint) | ESTR(jint64) | ESTR(juint64) | ESTR(jfloat) | ESTR(jdouble));

static const sregex qualifier           = (bos >> *_s >> "public" >> +_s >> (qualifier_name = (as_xpr("jreq") | "jopt")) >> *_s >> ':');

static const sregex field               = (bos >> *_s >> identifier >> +_s >> (field_name = identifier) >> repeat<0, 2>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex bool_field          = (bos >> *_s >> ESTR(jbool) >> +_s >> identifier >> *_s >> ';');

static const sregex number_field        = (bos >> *_s >> number >> +_s >> identifier >> *_s >> ';');
static const sregex number_array_field  = (bos >> *_s >> number >> +_s >> identifier >> repeat<1, 1>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex wchar_array_field   = (bos >> *_s >> ESTR(jwchar) >> +_s >> identifier >> repeat<1, 1>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');
static const sregex wchar_table_field   = (bos >> *_s >> ESTR(jwchar) >> +_s >> identifier >> repeat<2, 2>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex struct_field        = (bos >> *_s >> (struct_name = identifier) >> +_s >> identifier >> *_s >> ';');
static const sregex struct_array_field  = (bos >> *_s >> (struct_name = identifier) >> +_s >> identifier >> repeat<1, 1>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');
