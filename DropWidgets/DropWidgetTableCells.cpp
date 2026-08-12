#include "DropWidgetTableCells.h"

#include "QCheckBox.h"
#include "QLed.h"
#include "QLineEdit.h"
#include "../DataManagement/DataManagementSetClass.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qtablewidget.h>

namespace
{
QString CellObjectName(QTableWidget* table, int column)
{
    return table->objectName() + "r" + QString::number(table->rowCount())
           + "c" + QString::number(column);
}

QWidget* CreateCellContainer(QWidget* editor)
{
    QWidget* container = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->addWidget(editor);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    return container;
}
}

namespace DropWidgetTableCells
{
QWidget* CreateBoundCell(QTableWidget* table, DataManagementSetClass* manager,
                         const QString& id, int column)
{
    ToFormMapper* data = manager->GetContainer(id);
    const QString objectName = CellObjectName(table, column);

    if (data->IsSigedNumber() || data->IsFloatingPointNumber() || data->IsUnsigedNumber())
    {
        QLineEditD* editor = new QLineEditD;
        if (!data->IsEditable())
            editor->setReadOnly(true);

        QWidget* container = CreateCellContainer(editor);
        editor->setObjectName(objectName);
        manager->AddElementToContainerEntry(editor->objectName(), id, editor->metaObject()->className(), editor);
        editor->ConnectToID(manager, id);

        const QFontMetrics metrics(editor->font());
        editor->setFixedWidth(metrics.horizontalAdvance("1234678.1234"));
        editor->adjustSize();
        return container;
    }

    if (!data->IsBool())
        return NULL;

    QWidget* container = NULL;
    if (data->IsEditable())
    {
        QCheckBoxD* editor = new QCheckBoxD(0, 0);
        container = CreateCellContainer(editor);
        editor->setObjectName(objectName);
        manager->AddElementToContainerEntry(editor->objectName(), id, editor->metaObject()->className(), editor);
        editor->ConnectToID(manager, id);
    }
    else
    {
        QLed* editor = new QLed;
        container = CreateCellContainer(editor);
        editor->setObjectName(objectName);
        manager->AddElementToContainerEntry(editor->objectName(), id, editor->metaObject()->className(), editor);
        editor->ConnectToID(manager, id);
    }
    return container;
}
}
