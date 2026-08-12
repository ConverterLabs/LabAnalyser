#ifndef MAINWINDOWOUTPUTLOG_H
#define MAINWINDOWOUTPUTLOG_H

#include <QString>

class QDockWidget;
class QPlainTextEdit;
class QPoint;

class MainWindowOutputLog
{
public:
    enum class Kind { Error, Info, Notification };

    MainWindowOutputLog(QPlainTextEdit* output, QDockWidget* outputDock);
    void Append(Kind kind, const QString& id, const QString& data);
    static void ShowContextMenu(QPlainTextEdit& output, const QPoint& position);

private:
    QPlainTextEdit* Output;
    QDockWidget* OutputDock;
};

#endif // MAINWINDOWOUTPUTLOG_H
