#ifndef JSON2CXXSTRUCTHELPER_H
#define JSON2CXXSTRUCTHELPER_H

#include <QtGui/QWidget>
#include "ui_json2cxxstructhelper.h"

class json2cxxstructHelper : public QWidget, public Ui::json2cxxstructHelperClass
{
	Q_OBJECT

public:
	json2cxxstructHelper(QWidget *parent = 0, Qt::WFlags flags = 0);
	~json2cxxstructHelper();
};

#endif // JSON2CXXSTRUCTHELPER_H
