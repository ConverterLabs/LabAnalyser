#ifndef DROPWIDGETUPDATE_H
#define DROPWIDGETUPDATE_H

#include <QObject>

// Preserves the adapters' historic programmatic-update convention: updates
// suppress their own signals and explicitly leave the object unblocked.
// This intentionally does not restore a pre-existing blocked state.
template <typename Update>
inline void ApplyDropWidgetUpdate(QObject* object, Update update)
{
    object->blockSignals(true);
    update();
    object->blockSignals(false);
}

#endif // DROPWIDGETUPDATE_H
