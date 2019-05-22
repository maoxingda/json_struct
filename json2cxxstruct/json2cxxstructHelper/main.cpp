#include "json2cxxstructhelper.h"
#include <QtGui/QApplication>
#include <fstream>
#include <boost/format.hpp>
#include <boost/xpressive/xpressive.hpp>


struct register_info
{
	std::string								struct_name;
	std::list<std::string>					fields;
	std::list<std::string>::const_iterator	iter_struct_beg;
	std::list<std::string>::const_iterator	iter_struct_end;
};

static std::string field_name(std::string declaration)
{
	boost::xpressive::smatch name;

	if (regex_search(declaration, name, boost::xpressive::sregex::compile("[a-zA-Z_$][a-zA-Z0-9_$]*\\s+([a-zA-Z_$][a-zA-Z0-9_$]*)")))
	{
		return name[1];
	}

	return "";
}

static std::string struct_name(std::string declaration)
{
	boost::xpressive::smatch name;

	if (regex_search(declaration, name, boost::xpressive::sregex::compile("JSON_STRUCT\\s*\\(\\s*([a-zA-Z_$][a-zA-Z0-9_$]*)\\s*\\)")))
	{
		return name[1];
	}

	return "";
}

static bool already_register_field(std::list<std::string>::const_iterator beg, std::list<std::string>::const_iterator end)
{
	for (auto iter = beg; iter != end; ++iter)
	{
		if (std::string::npos != iter->find("JSON_REGISTER_FIELD") || std::string::npos != iter->find("JSON_REGISTER_MAP_FIELD"))
		{
			return true;
		}
	}

	return false;
}

void register_fields(std::string in_file_name, std::string out_file_name)
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

		boost::xpressive::sregex struct_end_re = boost::xpressive::sregex::compile("\\}\\s*;");
		boost::xpressive::sregex struct_beg_re = boost::xpressive::sregex::compile("JSON_STRUCT\\s*\\([a-zA-Z_$][a-zA-Z0-9_$]*\\)");

		for (auto iter = lines.begin(); iter != lines.end(); ++iter)
		{
			if (regex_search(*iter, struct_beg_re))
			{
				reg_info.iter_struct_beg = iter;
			}
			else if (regex_search(*iter, struct_end_re))
			{
				reg_info.iter_struct_end = iter;

				if (already_register_field(reg_info.iter_struct_beg , reg_info.iter_struct_end)) continue;

				reg_infos.push_back(reg_info);
			}
		}

		for (auto iter1 = reg_infos.begin(); iter1 != reg_infos.end(); ++iter1)
		{
			std::string st_name = struct_name(*iter1->iter_struct_beg);

			if (struct_name(*iter1->iter_struct_beg).empty()) continue;

			iter1->struct_name = st_name;

			for (auto iter2 = iter1->iter_struct_beg; iter2 != iter1->iter_struct_end; ++iter2)
			{
				std::string name = field_name(*iter2);

				if (!name.empty()) iter1->fields.push_back(name);
			}
		}

		for (auto iter1 = reg_infos.begin(); iter1 != reg_infos.end(); ++iter1)
		{
			lines.insert(iter1->iter_struct_end, (boost::format("\n\t%1%()") % iter1->struct_name).str());
			lines.insert(iter1->iter_struct_end, "\t{");
			
			//bool first = true;
			//std::string first_field_name;
			//for (auto iter2 = iter1->fields.begin(); iter2 != iter1->fields.end(); ++iter2)
			//{
			//	if (iter2->empty()) continue;

			//	if (first)
			//	{
			//		first = false;
			//		first_field_name = *iter2;
			//	}
			//}

			//if (!first_field_name.empty())
			//{
			//	//initialize child struct fields
			//	boost::format zero_fill_fields("\t\tZeroMemory((byte*)this + offsetof(%1%, %2%), sizeof(%1%) - sizeof(json_struct_base));");

			//	zero_fill_fields % iter1->struct_name;
			//	zero_fill_fields % first_field_name;

			//	lines.insert(iter1->iter_struct_end, zero_fill_fields.str());
			//}
			for (auto iter2 = iter1->fields.begin(); iter2 != iter1->fields.end(); ++iter2)
			{
				if (iter2->empty()) continue;
			}
			//////////////////////////////////////////////////////////////////////////
			//TODO:
			for (auto iter2 = iter1->fields.begin(); iter2 != iter1->fields.end(); ++iter2)
			{
				if (iter2->empty()) continue;

				lines.insert(iter1->iter_struct_end, (boost::format("\t\tJSON_REGISTER_FIELD(%1%);") % *iter2).str());
			}

			lines.insert(iter1->iter_struct_end, "\t}");
		}

		std::fstream out(out_file_name, std::ios_base::out);

		if (out)
		{
			int count = 0;
			for (auto iter = lines.begin(); iter != lines.end(); ++iter)
			{
				++count < lines.size() ? out << *iter << "\n" : out << *iter;
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