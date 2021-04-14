/********************************************************************************
** Form generated from reading UI file 'About.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_About
{
public:
    QPushButton *closeButton;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;

    void setupUi(QDialog *About)
    {
        if (About->objectName().isEmpty())
            About->setObjectName(QString::fromUtf8("About"));
        About->resize(518, 460);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(About->sizePolicy().hasHeightForWidth());
        About->setSizePolicy(sizePolicy);
        About->setMinimumSize(QSize(518, 460));
        About->setMaximumSize(QSize(518, 460));
        closeButton = new QPushButton(About);
        closeButton->setObjectName(QString::fromUtf8("closeButton"));
        closeButton->setGeometry(QRect(420, 410, 81, 27));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/icons/sym.png"), QSize(), QIcon::Normal, QIcon::Off);
        closeButton->setIcon(icon);
        closeButton->setAutoDefault(true);
        closeButton->setFlat(false);
        label = new QLabel(About);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 10, 131, 141));
        label_2 = new QLabel(About);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(180, 50, 291, 111));
        label_2->setOpenExternalLinks(true);
        label_3 = new QLabel(About);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(30, 230, 391, 201));
        label_3->setWordWrap(true);
        label_3->setOpenExternalLinks(true);
        label_4 = new QLabel(About);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(30, 180, 471, 41));
        label_4->setWordWrap(true);
        label_5 = new QLabel(About);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(180, 10, 331, 31));
        label_5->setOpenExternalLinks(true);

        retranslateUi(About);
        QObject::connect(closeButton, SIGNAL(clicked()), About, SLOT(close()));

        closeButton->setDefault(true);


        QMetaObject::connectSlotsByName(About);
    } // setupUi

    void retranslateUi(QDialog *About)
    {
        About->setWindowTitle(QCoreApplication::translate("About", "Dialog", nullptr));
        closeButton->setText(QCoreApplication::translate("About", "close", nullptr));
        label->setText(QCoreApplication::translate("About", "<html><head/><body><p><img src=\":/icons/icons/sym_about.png\"/></p></body></html>", nullptr));
        label_2->setText(QCoreApplication::translate("About", "<html><head/><body><p><span style=\" font-size:10pt; font-style:italic;\">a plugin based open source data modification <br/>and visualization tool. &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span><a href=\"https://github.com/ConverterLabs/LabAnalyser/releases\"><span style=\" font-size:9pt; text-decoration: underline; color:#0000ff;\">&lt;Releases&gt;</span></a></p><p><span style=\" font-size:10pt;\">based on Qt-Version 5.12.6 (MinGW 64-bit)</span></p><p><span style=\" font-size:10pt;\">licenced under:<br/></span><a href=\"https://www.gnu.org/licenses/gpl-3.0.html\"><span style=\" font-size:10pt; text-decoration: underline; color:#0000ff;\">GNU General Public License Version 3</span></a></p><p><span style=\" font-size:11pt;\"><br/><br/></span></p></body></html>", nullptr));
        label_3->setText(QCoreApplication::translate("About", "<html><head/><body><p><span style=\" font-size:10pt; font-weight:600;\">Copyright (c) :</span></p><p><a href=\"https://github.com/ConverterLabs/LabAnalyser/\"><span style=\" font-size:10pt; text-decoration: underline; color:#3333ff;\">LabAnalyser:</span></a><span style=\" font-size:10pt;\"> (c) 2016-2021 Andreas Hoffmann</span></p><p><span style=\" font-size:10pt;\">LabAnalyser uses code of:<br/></span><a href=\"www.boost.org/\"><span style=\" font-size:10pt; text-decoration: underline; color:#000000;\">Boost C++ Libraries </span></a><span style=\" font-size:10pt;\">(c) 2015 (Boost Software License)<br/></span><a href=\"https://github.com/EyNuel/matOut/\"><span style=\" font-size:10pt; text-decoration: underline; color:#000000;\">matOut:</span></a><span style=\" font-size:10pt;\"> (c) 2011 Emanuel Ey (LGPLV3)<br/></span><a href=\"https://www.qcustomplot.com/\"><span style=\" font-size:10pt; text-decoration: underline; color:#000000;\">QCustomPlot:</span></a><span style=\" font-size:10pt;\"> (c) 2015 Emanuel Ei"
                        "chhammer (GPLV3)<br/></span><a href=\"https://github.com/BlueBrain/HighFive/\"><span style=\" font-size:10pt; text-decoration: underline; color:#000000;\">HighFive </span></a><span style=\" font-size:10pt;\">(c) 2017 (Boost Software License)<br/></span><a href=\"https://github.com/HDFGroup/hdf5/\"><span style=\" font-size:10pt; text-decoration: underline; color:#000000;\">HDF5 Software Library and Utilities </span></a><span style=\" font-size:10pt;\">(c) The HDF Group.</span></p><p><span style=\" font-size:10pt; color:#000000;\">LabAnalyser uses icons from:<br/></span><a href=\"https://remixicon.com/\"><span style=\" font-size:10pt; text-decoration: underline; color:#000000;\">Remix Icon </span></a><span style=\" font-size:10pt;\">(Apache License Version 2.0)</span></p></body></html>", nullptr));
        label_4->setText(QCoreApplication::translate("About", "<html><head/><body><p align=\"center\"><span style=\" font-size:10pt; font-weight:600;\">THE COPYRIGHTHOLDER AND/OR OTHER PARTIES PROVIDE THE SOFTWARE IS PROVIDED &quot;AS IS&quot;, WITHOUT WARRANTY OF ANY KIND.</span></p></body></html>", nullptr));
        label_5->setText(QCoreApplication::translate("About", "<html><head/><body><p><span style=\" font-size:12pt; font-weight:600;\">LabAnalyser X.X.X alpha (git: XXXXXXX)</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class About: public Ui_About {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_H
