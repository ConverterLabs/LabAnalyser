#pragma once

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>

#include"../mainwindow.h"

MainWindow* GetMainWindow();
QString CreateID(QObject *Tree);
