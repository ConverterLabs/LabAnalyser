#include "MainWindowSubplotDialog.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

bool MainWindowSubplotDialog::SelectDimensions(int& rows, int& columns)
{
    QDialog dialog(nullptr, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    QWidget content(&dialog);
    QHBoxLayout contentLayout(&content);
    QVBoxLayout dialogLayout(&dialog);
    QComboBox rowBox;
    rowBox.addItems(QStringList() << "1" << "2" << "3");
    QComboBox columnBox;
    columnBox.addItems(QStringList() << "1" << "2" << "3" << "4");
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QLabel rowLabel("Rows:");
    QLabel columnLabel("Columns:");

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    contentLayout.addWidget(&rowLabel);
    contentLayout.addWidget(&rowBox);
    contentLayout.addWidget(&columnLabel);
    contentLayout.addWidget(&columnBox);
    dialogLayout.addWidget(&content);
    dialogLayout.addWidget(&buttonBox);

    if (dialog.exec() != QDialog::Accepted)
        return false;

    rows = rowBox.currentText().toInt();
    columns = columnBox.currentText().toInt();
    return true;
}
