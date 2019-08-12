#include "jstructhelpergui.h"
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../util/jutil_common_path.h"
#include <boost/lexical_cast.hpp>

using namespace boost::property_tree;

jstructhelpergui::jstructhelpergui(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);

    align_.load();
    dbg_.load();

    ui.lineEditArrLen->setText(QString(align_.reserve_arr_len_.c_str()));
    ui.lineEditStrLen->setText(QString(align_.reserve_str_len_.c_str()));
    dbg_.throw_ ? ui.checkBoxThrow->setCheckState(Qt::Checked) : 0;
    "true" == dbg_.add_output_file_ ? ui.checkBoxAdd->setCheckState(Qt::Checked) : 0;
}

jstructhelpergui::~jstructhelpergui()
{

}

void jstructhelpergui::on_pushButtonSave_clicked()
{
    unsigned arr_len = ui.lineEditArrLen->text().toUInt();
    unsigned str_len = ui.lineEditStrLen->text().toUInt();

    if (2 > arr_len || (1 << 7 ) < arr_len) return;
    if (2 > str_len || (1 << 10) < str_len) return;

    align_.reserve_arr_len_ = boost::lexical_cast<string>(arr_len);//(boost::format("%1%") % arr_len).str();
    align_.reserve_str_len_ = boost::lexical_cast<string>(str_len);//(boost::format("%1%") % str_len).str();

    dbg_.throw_             = Qt::Checked == ui.checkBoxThrow->checkState();
    dbg_.add_output_file_   = Qt::Checked == ui.checkBoxAdd->checkState() ? "true" : "false";

    align_.save();
    dbg_.save();

    close();
}

void jstructhelpergui::debug_conf::load()
{
    ptree tree;

    read_xml(jutil_common_path().my_documents() + "\\Visual Studio 2010\\Addins\\debugconf.xml", tree);

    throw_           = tree.get<bool>("debug.throw");
    add_output_file_ = tree.get<string>("debug.add_output_file");
}

void jstructhelpergui::debug_conf::save()
{
    ptree tree;

    tree.put("debug.throw", throw_);
    tree.put("debug.add_output_file", add_output_file_);

    write_xml(jutil_common_path().my_documents() + "\\Visual Studio 2010\\Addins\\debugconf.xml", tree, std::locale(), xml_writer_settings<typename ptree::key_type>(' ', 4));
}
