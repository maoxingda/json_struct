/********************************************************************************
** Form generated from reading UI file 'jstructhelpergui.ui'
**
** Created: Mon Aug 12 15:25:00 2019
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_JSTRUCTHELPERGUI_H
#define UI_JSTRUCTHELPERGUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFormLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_jstructhelperguiClass
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelArrLen;
    QLineEdit *lineEditArrLen;
    QLabel *labelStrLen;
    QLineEdit *lineEditStrLen;
    QLabel *labelThro;
    QLabel *labelAdd;
    QCheckBox *checkBoxThrow;
    QCheckBox *checkBoxAdd;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButtonSave;

    void setupUi(QMainWindow *jstructhelperguiClass)
    {
        if (jstructhelperguiClass->objectName().isEmpty())
            jstructhelperguiClass->setObjectName(QString::fromUtf8("jstructhelperguiClass"));
        jstructhelperguiClass->resize(400, 300);
        jstructhelperguiClass->setMinimumSize(QSize(400, 300));
        jstructhelperguiClass->setMaximumSize(QSize(400, 300));
        centralWidget = new QWidget(jstructhelperguiClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(-1, -1, -1, 9);
        formLayout = new QFormLayout();
        formLayout->setSpacing(6);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        labelArrLen = new QLabel(centralWidget);
        labelArrLen->setObjectName(QString::fromUtf8("labelArrLen"));

        formLayout->setWidget(0, QFormLayout::LabelRole, labelArrLen);

        lineEditArrLen = new QLineEdit(centralWidget);
        lineEditArrLen->setObjectName(QString::fromUtf8("lineEditArrLen"));

        formLayout->setWidget(0, QFormLayout::FieldRole, lineEditArrLen);

        labelStrLen = new QLabel(centralWidget);
        labelStrLen->setObjectName(QString::fromUtf8("labelStrLen"));

        formLayout->setWidget(1, QFormLayout::LabelRole, labelStrLen);

        lineEditStrLen = new QLineEdit(centralWidget);
        lineEditStrLen->setObjectName(QString::fromUtf8("lineEditStrLen"));

        formLayout->setWidget(1, QFormLayout::FieldRole, lineEditStrLen);

        labelThro = new QLabel(centralWidget);
        labelThro->setObjectName(QString::fromUtf8("labelThro"));

        formLayout->setWidget(2, QFormLayout::LabelRole, labelThro);

        labelAdd = new QLabel(centralWidget);
        labelAdd->setObjectName(QString::fromUtf8("labelAdd"));

        formLayout->setWidget(3, QFormLayout::LabelRole, labelAdd);

        checkBoxThrow = new QCheckBox(centralWidget);
        checkBoxThrow->setObjectName(QString::fromUtf8("checkBoxThrow"));

        formLayout->setWidget(2, QFormLayout::FieldRole, checkBoxThrow);

        checkBoxAdd = new QCheckBox(centralWidget);
        checkBoxAdd->setObjectName(QString::fromUtf8("checkBoxAdd"));

        formLayout->setWidget(3, QFormLayout::FieldRole, checkBoxAdd);


        verticalLayout->addLayout(formLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 0, -1, -1);
        pushButtonSave = new QPushButton(centralWidget);
        pushButtonSave->setObjectName(QString::fromUtf8("pushButtonSave"));
        pushButtonSave->setAutoDefault(true);
        pushButtonSave->setDefault(true);

        horizontalLayout->addWidget(pushButtonSave, 0, Qt::AlignRight);


        verticalLayout->addLayout(horizontalLayout);

        jstructhelperguiClass->setCentralWidget(centralWidget);

        retranslateUi(jstructhelperguiClass);

        QMetaObject::connectSlotsByName(jstructhelperguiClass);
    } // setupUi

    void retranslateUi(QMainWindow *jstructhelperguiClass)
    {
        jstructhelperguiClass->setWindowTitle(QApplication::translate("jstructhelperguiClass", "jstructhelpergui", 0, QApplication::UnicodeUTF8));
        labelArrLen->setText(QApplication::translate("jstructhelperguiClass", "\351\242\204\347\225\231\346\225\260\347\273\204\345\244\247\345\260\217", 0, QApplication::UnicodeUTF8));
        labelStrLen->setText(QApplication::translate("jstructhelperguiClass", "\351\242\204\347\225\231\345\255\227\347\254\246\344\270\262\345\244\247\345\260\217", 0, QApplication::UnicodeUTF8));
        labelThro->setText(QApplication::translate("jstructhelperguiClass", "\346\230\257\345\220\246\346\212\233\350\260\203\350\257\225\345\274\202\345\270\270", 0, QApplication::UnicodeUTF8));
        labelAdd->setText(QApplication::translate("jstructhelperguiClass", "\346\230\257\345\220\246\346\267\273\345\212\240\345\244\264\346\226\207\344\273\266", 0, QApplication::UnicodeUTF8));
        checkBoxThrow->setText(QString());
        checkBoxAdd->setText(QString());
        pushButtonSave->setText(QApplication::translate("jstructhelperguiClass", "\344\277\235\345\255\230", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class jstructhelperguiClass: public Ui_jstructhelperguiClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_JSTRUCTHELPERGUI_H
