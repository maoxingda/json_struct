#include "json2cxxstructhelper.h"
#include <QtGui/QApplication>
#include <fstream>
#include <boost/format.hpp>
#include <boost/xpressive/xpressive.hpp>

using namespace boost::xpressive;


struct field_info
{
	bool		nested;
	bool		array;
	std::string qualifier;
	std::string name;
};

std::list<field_info>						fields;

struct register_info
{
	std::string								struct_name;
	std::list<field_info>					fields;
	std::list<std::string>::const_iterator	iter_struct_beg;
	std::list<std::string>::const_iterator	iter_struct_end;
};

static std::string field_qualifier(std::string declaration)
{
    smatch qualifier;
    
    static sregex field_qualifier_regex = sregex::compile("^\\s*(REQUIRED|OPTIONAL|Y|N)\\s*(REQUIRED|OPTIONAL|Y|N)\\s+");
    
    if (regex_search(declaration, qualifier, field_qualifier_regex))
    {
        return qualifier[1] + qualifier[2];
    }
    
    return "";
}

static std::string field_name(std::string declaration, bool& nested, bool& array)
{
	smatch name;

	static sregex struct_field_regex				= sregex::compile("[a-zA-Z_$][a-zA-Z0-9_$]*\\s+([a-zA-Z_$][a-zA-Z0-9_$]*)(?:\\[\\d+\\])?\\s*;");
	static sregex nested_struct_field_regex			= sregex::compile("JSTRUCT_DECL_NESTED_FIELD\\(\\s*[a-zA-Z_$][a-zA-Z0-9_$]*\\s+([a-zA-Z_$][a-zA-Z0-9_$]*)\\)");
	static sregex nested_struct_field_array_regex	= sregex::compile("JSTRUCT_DECL_NESTED_FIELD\\(\\s*[a-zA-Z_$][a-zA-Z0-9_$]*\\s+([a-zA-Z_$][a-zA-Z0-9_$]*)\\[(\\d+)\\]\\)");

	if (regex_search(declaration, name, nested_struct_field_array_regex))
	{
		nested	= true;
		array	= true;

		return name[1];
	}

	if (regex_search(declaration, name, nested_struct_field_regex))
	{
		nested	= true;

		return name[1];
	}

	if (regex_search(declaration, name, struct_field_regex))
	{
		return name[1];
	}

	return "";
}

static std::string struct_name(std::string declaration)
{
	smatch name;

	if (regex_search(declaration, name, sregex::compile("JSTRUCT\\s*\\(\\s*([a-zA-Z_$][a-zA-Z0-9_$]*)\\s*\\)")))
	{
		return name[1];
	}

	return "";
}

static bool is_single_line_comment(std::string line)
{
	static sregex single_line_comment_re = sregex::compile("^\\s*//");

	return regex_search(line, single_line_comment_re);
}

static bool is_multiline_comment_beg(std::string line)
{
	static sregex multiline_comment_beg_re = sregex::compile("^\\s*/\\*");

	return regex_search(line, multiline_comment_beg_re);
}

static bool is_multiline_comment_end(std::string line)
{
	static sregex multiline_comment_end_re = sregex::compile("^\\s*\\*/");

	return regex_search(line, multiline_comment_end_re);
}

static bool is_struct_default_ctor(std::string struct_name, std::string line)
{
	static std::string ctor_decl_re_str = (boost::format("%1%\\(\\s*\\)\\s*;") % struct_name).str();

	static sregex ctor_decl_re			= sregex::compile(ctor_decl_re_str);

	return regex_search(line, ctor_decl_re);
}

static void register_fields(std::string in_file_name, std::string out_file_name)
{
	std::list<std::string> lines;

	std::fstream in(in_file_name, std::ios_base::in);

	if (in)
	{
		std::string line;

		while (getline(in, line))
		{
			lines.push_back(line);
		}

		in.close();
	}

	if (!lines.empty())
	{
		register_info				reg_info;
		std::list<register_info>	reg_infos;

		sregex struct_end_re = sregex::compile("\\}\\s*;");
		sregex struct_beg_re = sregex::compile("JSTRUCT\\s*\\([a-zA-Z_$][a-zA-Z0-9_$]*\\)");

		bool in_multiline_comment = false;

		for (auto iter = lines.begin(); iter != lines.end(); ++iter)
		{
			if (is_single_line_comment(*iter)) continue;

			if (is_multiline_comment_beg(*iter))
			{
				in_multiline_comment = true;

				continue;
			}
			else if (is_multiline_comment_end(*iter))
			{
				in_multiline_comment = false;

				continue;
			}
			else if (in_multiline_comment)
			{
				continue;
			}

			if (regex_search(*iter, struct_beg_re))
			{
				reg_info.iter_struct_beg = iter;
			}
			else if (regex_search(*iter, struct_end_re))
			{
				reg_info.iter_struct_end = iter;

				reg_infos.push_back(reg_info);
			}
		}

		for (auto iter1 = reg_infos.begin(); iter1 != reg_infos.end(); ++iter1)
		{
			std::string st_name = struct_name(*iter1->iter_struct_beg);

			if (st_name.empty()) continue;

			iter1->struct_name = st_name;

			bool in_multiline_comment = false;

			for (auto iter2 = iter1->iter_struct_beg; iter2 != iter1->iter_struct_end; ++iter2)
			{
				if (is_single_line_comment(*iter2)) continue;

				if (is_multiline_comment_beg(*iter2))
				{
					in_multiline_comment = true;

					continue;
				}
				else if (is_multiline_comment_end(*iter2))
				{
					in_multiline_comment = false;

					continue;
				}
				else if (in_multiline_comment)
				{
					continue;
				}
				else if (is_struct_default_ctor(st_name, *iter2))
				{
					continue;
				}

				field_info f_info;

				bool nested			= false;
				bool array			= false;
				std::string name	= field_name(*iter2, nested, array);

				f_info.nested		= nested;
				f_info.array		= array;
				f_info.name			= name;
				f_info.qualifier	= field_qualifier(*iter2);

				if (!name.empty()) iter1->fields.push_back(f_info);
			}
		}

		std::fstream out(out_file_name, std::ios_base::out);

		if (out)
		{
			smatch base_file_name_sm;
			sregex base_file_name_regex = sregex::compile("(\\w+\\.h)");
			if (regex_search(in_file_name, base_file_name_sm, base_file_name_regex))
			{
				out << "#include \"stdafx.h\"\n";
				out << boost::format("#include <%1%>\n\n\n") % base_file_name_sm[1];

				int count = 0;
				for (auto iter1 = reg_infos.begin(); iter1 != reg_infos.end(); ++iter1, ++count)
				{
					out << boost::format("%1%::%1%()\n") % iter1->struct_name;
					out << "{\n";
					//////////////////////////////////////////////////////////////////////////
					for (auto iter2 = iter1->fields.begin(); iter2 != iter1->fields.end(); ++iter2)
					{
						if (iter2->name.empty()) continue;

						if (!iter2->array)
						{
							out << boost::format("\tJSON_STRUCT_REGISTER_FIELD(%1%, %2%);\n") % iter2->qualifier % iter2->name;
						}
					}
					//////////////////////////////////////////////////////////////////////////
					for (auto iter2 = iter1->fields.begin(); iter2 != iter1->fields.end(); ++iter2)
					{
						if (iter2->name.empty()) continue;

						if (iter2->array)
						{
							out << boost::format("\tJSON_STRUCT_REGISTER_NESTED_FIELD(%1%, %2%);\n") % iter2->qualifier % iter2->name;
						}
					}
					out << "\n";
					//////////////////////////////////////////////////////////////////////////
					for (auto iter2 = iter1->fields.begin(); iter2 != iter1->fields.end(); ++iter2)
					{
						if (iter2->name.empty()) continue;

						if (!iter2->nested)
						{
							out << boost::format("\tJSON_STRUCT_FIELD_FILL_ZERO(%1%, %2%);\n") % iter1->struct_name % iter2->name;
						}
					}

					count + 1 == reg_infos.size() ? out << "}\n" : out << "}\n\n";
				}
			}
			out.close();
		}
	}
}

int main(int argc, char *argv[])
{
	//return false;
	QApplication a(argc, argv);

	if (3 > argc) return -1;

	register_fields(argv[1], argv[2]);

	return 0;
}