#include "json2cxxstructhelper.h"

json2cxxstructHelper::json2cxxstructHelper(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);

	lineEditFileName->setText("C:/Users/Administrator/Desktop/json2cxxstruct/json2cxxstruct/json2cxxstructDemo/test/json2cxxstruct1.h");
	lineEditStructName->setText("anchor_info");
}

json2cxxstructHelper::~json2cxxstructHelper()
{

}