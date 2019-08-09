#include "jstructhelpergui.h"

jstructhelpergui::jstructhelpergui(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);

    align_.load();

    ui.lineEditArrLen->setText(QString(align_.reserve_arr_len_.c_str()));
    ui.lineEditStrLen->setText(QString(align_.reserve_str_len_.c_str()));
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

    align_.save();
}
