#ifndef MAINWINDOWOUTPUTLOG_H
#define MAINWINDOWOUTPUTLOG_H

#include <QString>

class QDockWidget;
class QPlainTextEdit;

class MainWindowOutputLog
{
public:
    enum class Kind { Error, Info, Notification };

    MainWindowOutputLog(QPlainTextEdit* output, QDockWidget* outputDock);
    void Append(Kind kind, const QString& id, const QString& data);

private:
    QPlainTextEdit* Output;
    QDockWidget* OutputDock;
};

#endif // MAINWINDOWOUTPUTLOG_H
