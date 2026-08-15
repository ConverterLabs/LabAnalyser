#include "MainWindowFormLoader.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "DropWidgets/DropWidgetsUiLoader.h"
#include "DropWidgets/Plots/PlotWidget.h"
#include "UIFunctions/UiLayoutEditMode.h"

#include <QApplication>
#include <QDate>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QPalette>
#include <QScrollArea>
#include <QTime>

namespace {

void AppendWidgetNames(MainWindow& mainWindow, const QObjectList& children, const QString& suffix)
{
    for (QObject* child : children) {
        if (!child)
            continue;
        if (!child->children().isEmpty())
            AppendWidgetNames(mainWindow, child->children(), suffix);
        const QString name = child->objectName();
        if (!name.isEmpty()) {
            child->setProperty("LabAnalyserOriginalObjectName", name);
            child->setObjectName(name + "_" + suffix);
        }
        if (qobject_cast<PlotWidget*>(child))
            mainWindow.GetLogic()->AddPlotPointer(child->objectName(), child);
    }
}

} // namespace

void MainWindowFormLoader::Load(MainWindow& mainWindow, QString uiFileName, QString lastFormName, bool skip)
{
    uiFileName.replace("\\", "/");
    const QStringList pathParts = uiFileName.split("/");
    const QString filePart = pathParts.at(pathParts.size() - 1);
    const QStringList fileNameParts = filePart.split(".");
    if (fileNameParts.size() < 2) {
        mainWindow.Error("Corrupt Form File");
        return;
    }
    const QString fileName = fileNameParts.at(fileNameParts.size() - 2);

    QWidget* tab = new QWidget;
    const QFileInfo fileInfo(uiFileName);
    QDir::setCurrent(fileInfo.absoluteDir().absolutePath());
    DropWidgetsUiLoader loader(&mainWindow);
    QFile file(uiFileName);
    loader.setWorkingDirectory(fileInfo.absoluteDir().absolutePath());
    QWidget* formWidget = loader.load(&file, tab);
    file.close();

    if (!formWidget) {
        delete tab;
        mainWindow.Error("Corrupt Form File");
        return;
    }

    const QString dateTime = QDate::currentDate().toString("_yyyy_MM_dd_")
        + QTime::currentTime().toString("hh_mm_ss");
    const QString formName = lastFormName.isEmpty() ? fileName + dateTime : lastFormName;

    AppendWidgetNames(mainWindow, formWidget->children(), formName);
    tab->setObjectName(formName);

    QGridLayout* clientLayout = new QGridLayout;
    clientLayout->setSpacing(6);
    clientLayout->setContentsMargins(11, 35, 11, 11);
    QWidget* clientContainer = new QWidget;
    clientContainer->setLayout(clientLayout);

    QScrollArea* scrollArea = new QScrollArea;
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(0, 0, 0, 0));
    scrollArea->setPalette(palette);
    scrollArea->setBackgroundRole(QPalette::Window);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(clientContainer);
    clientContainer->layout()->addWidget(formWidget);

    QGridLayout* pageLayout = new QGridLayout;
    pageLayout->setSpacing(6);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    tab->setLayout(pageLayout);
    tab->layout()->addWidget(scrollArea);
    QPalette pagePalette;
    pagePalette.setColor(QPalette::Window, Qt::white);
    tab->setPalette(pagePalette);
    tab->setAutoFillBackground(true);

    mainWindow.UI()->centralWidget->hide();
    QDockWidget* dock = new QDockWidget(&mainWindow);
    dock->setObjectName(formName);
    dock->setWindowTitle(fileName);
    dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    dock->setWidget(tab);
    dock->setAttribute(Qt::WA_DeleteOnClose);
    dock->installEventFilter(&mainWindow);
    dock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainWindow.addDockWidget(Qt::LeftDockWidgetArea, dock);
    for (QDockWidget* existingDock : mainWindow.findChildren<QDockWidget*>()) {
        if (mainWindow.dockWidgetArea(existingDock) == Qt::LeftDockWidgetArea
            && !existingDock->isFloating() && mainWindow.isVisible() && dock != existingDock) {
            mainWindow.tabifyDockWidget(existingDock, dock);
            dock->show();
            dock->raise();
        }
    }
    QObject::connect(dock, SIGNAL(topLevelChanged(bool)), &mainWindow, SLOT(dockWidget_topLevelChanged(bool)));
    QObject::connect(dock, SIGNAL(destroyed(QObject*)), &mainWindow, SLOT(dockWidget_destroyed(QObject*)));

    mainWindow.GetLogic()->AddFormFile(std::pair<QString, QString>(formName, uiFileName));
    mainWindow.GetLogic()->AddSkipFormFile(formName, skip);
    UiLayoutEditMode::For(mainWindow)->RegisterForm(formWidget, uiFileName);
    QApplication::processEvents();
}
