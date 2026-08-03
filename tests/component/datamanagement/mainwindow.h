#pragma once

// Test-only link seam for DataManagementSetClass.  The characterization tests
// deliberately use a plain QObject parent, so qobject_cast<MainWindow*> fails
// and no member of this stand-in is invoked.  This avoids pulling the GUI and
// its unrelated IO dependencies into a component test.
#include <QObject>
#include <QStatusBar>

class MainWindow : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    QStatusBar* GetStatusBar() { return nullptr; }

public slots:
    void PublishFinished() {}
    void PublishStart() {}
};
