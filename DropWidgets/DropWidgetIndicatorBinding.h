#ifndef DROPWIDGETINDICATORBINDING_H
#define DROPWIDGETINDICATORBINDING_H

#include "CreateID.h"
#include "DropWidgetBinding.h"

#include <QtGui/qevent.h>
#include <QtWidgets/qinputdialog.h>

// Shared legacy drop transaction for the three Designer LED adapters.  The
// caller retains its own static bit counter and concrete SetState operation;
// this helper only removes their byte-for-byte duplicated manager sequence.
namespace DropWidgetIndicatorBinding
{
template <typename StateSetter>
inline void InitializeState(QWidget* parent, ToFormMapper* container, uint32_t& bit,
                            uint32_t& bitCounter, const QString& title,
                            const QString& prompt, StateSetter setState)
{
    if (!container)
        return;
    if (container->IsBool())
    {
        bit = 0;
        setState(container->GetBool());
    }
    else if (container->IsUnsigedNumber())
    {
        bool ok = false;
        const int selectedBit = QInputDialog::getInt(parent, title, prompt,
                                                     bitCounter, 0, 63, 1, &ok);
        bitCounter = selectedBit + 1;
        if (bitCounter > 63)
            bitCounter = 0;
        if (ok)
            bit = selectedBit;

        setState((container->GetUnsignedData() & (1ULL << bit)) != 0);
    }
}

template <typename Indicator>
inline void BindFromDrop(Indicator* indicator, QDropEvent* event, uint32_t& bit, uint32_t& bitCounter)
{
    MainWindow* mainWindow = GetMainWindow();
    if (!mainWindow)
        return;

    const QString id = CreateID(event->source());
    DataManagementSetClass* manager = mainWindow->GetLogic();
    ToFormMapper* container = manager->GetContainer(id);
    if (!container)
        return;

    indicator->disconnect();
    QObject::connect(indicator, SIGNAL(customContextMenuRequested(QPoint)), indicator, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(indicator, SIGNAL(RequestUpdate()));

    // The legacy adapters query the type before the actual type predicates.
    // Keep that lookup order even though the textual result is unused.
    container->GetDataType();

    InitializeState(indicator, container, bit, bitCounter,
                    QObject::tr("Index of Bit to be set"),
                    QObject::tr("Index of Bit to be set (zero based):"),
                    [indicator](bool state) { indicator->SetState(state); });

    indicator->setToolTip(id + ":" + QString::number(bit));
    indicator->setToolTipDuration(2000);
    manager->AddElementToContainerEntry(indicator->objectName(), id,
                                        indicator->metaObject()->className(), indicator);
    mainWindow->ChangeForSaveDetected = true;
}
}

#endif // DROPWIDGETINDICATORBINDING_H
