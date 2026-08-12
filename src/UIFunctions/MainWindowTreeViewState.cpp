#include "MainWindowTreeViewState.h"

#include <QDockWidget>
#include <QTreeWidget>

MainWindowTreeViewState::MainWindowTreeViewState(QTreeWidget* parameter, QTreeWidget* data,
                                                 QTreeWidget* state, QDockWidget* parameterDock,
                                                 QDockWidget* dataDock, QDockWidget* stateDock)
    : Parameter(parameter), Data(data), State(state), ParameterDock(parameterDock),
      DataDock(dataDock), StateDock(stateDock)
{
}

void MainWindowTreeViewState::BeginPublish()
{
    for (QTreeWidget* tree : {Parameter, Data, State}) {
        if (tree) {
            tree->setUpdatesEnabled(false);
            tree->setSortingEnabled(false);
        }
    }
}

void MainWindowTreeViewState::EndPublish()
{
    for (QTreeWidget* tree : {Parameter, Data, State}) {
        if (tree)
            tree->setUpdatesEnabled(true);
    }
    SetColumns(Parameter, ParameterDock);
    if (Parameter) {
        Parameter->sortByColumn(0, Qt::AscendingOrder);
        Parameter->setSortingEnabled(true);
    }
    SetColumns(Data, DataDock);
    if (Data) {
        Data->sortByColumn(0, Qt::AscendingOrder);
        Data->setSortingEnabled(true);
    }
    SetColumns(State, StateDock);
}

void MainWindowTreeViewState::SetColumns(QTreeWidget* tree, QDockWidget* dock)
{
    if (!tree || !dock)
        return;
    const int width = dock->width();
    tree->setColumnWidth(0, width * 0.45);
    tree->setColumnWidth(1, width * 0.3);
    tree->setColumnWidth(2, width * 0.2);
}
