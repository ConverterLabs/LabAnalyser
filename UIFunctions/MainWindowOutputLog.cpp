#include "MainWindowOutputLog.h"

#include <QDockWidget>
#include <QAction>
#include <QMenu>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextDocument>
#include <QTime>

MainWindowOutputLog::MainWindowOutputLog(QPlainTextEdit* output, QDockWidget* outputDock)
    : Output(output), OutputDock(outputDock)
{
}

void MainWindowOutputLog::ShowContextMenu(QPlainTextEdit& output, const QPoint& position)
{
    QMenu* menu = output.createStandardContextMenu();
    menu->addSeparator();
    QAction* clearAction = new QAction("Clear Output", menu);
    QObject::connect(clearAction, &QAction::triggered, &output, &QPlainTextEdit::clear);
    menu->addAction(clearAction);
    menu->exec(output.mapToGlobal(position));
    delete menu;
}

void MainWindowOutputLog::Append(Kind kind, const QString& id, const QString& data)
{
    if (!Output)
        return;

    QString line = QTime::currentTime().toString("hh:mm:ss") % " " % id % ":&nbsp;&nbsp;&nbsp; ";
    QString lineData = data;
    if (lineData.size() > 10000) {
        lineData = lineData.left(1000);
        lineData.prepend("<font color=\"Red\"><b> WARNING: The Data was trimmed by LabAnalyser, because of its size. It might be due to a babbling idiot.</b></font><br>");
    }
    lineData.prepend("<br>");

    QString color;
    switch (kind) {
    case Kind::Error:
        color = "Red";
        break;
    case Kind::Info:
        color = "Black";
        break;
    case Kind::Notification:
        color = "DarkBlue";
        break;
    }

    const QString htmlPrefix = "<font color=\"" % color % "\">";
    const QString htmlSuffix = kind == Kind::Error ? "</font> " : "</font>";
    line = htmlPrefix % "<b>" % line % "</b>" % htmlSuffix;

    const bool atBottom = Output->verticalScrollBar()->value() == Output->verticalScrollBar()->maximum();
    QTextCursor cursor(Output->document());
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    cursor.insertHtml(line);
    cursor.insertHtml(htmlPrefix % lineData % htmlSuffix);
    cursor.insertBlock();
    cursor.endEditBlock();

    if (atBottom)
        Output->verticalScrollBar()->setValue(Output->verticalScrollBar()->maximum());

    if (kind != Kind::Info && OutputDock)
        OutputDock->raise();
}
