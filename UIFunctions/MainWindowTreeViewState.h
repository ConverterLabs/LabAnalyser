#ifndef MAINWINDOWTREEVIEWSTATE_H
#define MAINWINDOWTREEVIEWSTATE_H

class QDockWidget;
class QTreeWidget;

class MainWindowTreeViewState
{
public:
    MainWindowTreeViewState(QTreeWidget* parameter, QTreeWidget* data, QTreeWidget* state,
                            QDockWidget* parameterDock, QDockWidget* dataDock, QDockWidget* stateDock);
    void BeginPublish();
    void EndPublish();

private:
    static void SetColumns(QTreeWidget* tree, QDockWidget* dock);

    QTreeWidget* Parameter;
    QTreeWidget* Data;
    QTreeWidget* State;
    QDockWidget* ParameterDock;
    QDockWidget* DataDock;
    QDockWidget* StateDock;
};

#endif // MAINWINDOWTREEVIEWSTATE_H
