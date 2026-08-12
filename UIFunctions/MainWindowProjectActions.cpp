#include "MainWindowProjectActions.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QWidget>

namespace {

QString Text(const char* sourceText)
{
    return QCoreApplication::translate("MainWindow", sourceText);
}

QString ExperimentFilter()
{
    return Text("Expermiment Files (*.LAexp)");
}

}

void MainWindowProjectActions::SaveExperiment(QWidget& parent, QString& standardSavePath,
                                              bool& changeDetected,
                                              const std::function<void(const QString&)>& save)
{
    const QString path = QFileDialog::getSaveFileName(&parent, Text("Save Experiment"),
                                                      standardSavePath, ExperimentFilter());
    const QFileInfo fileInfo(path);
    standardSavePath = fileInfo.absolutePath();
    save(path);
    changeDetected = false;
}

void MainWindowProjectActions::LoadExperiment(QWidget& parent, QString& standardSavePath,
                                              QString& savePath, bool& changeDetected,
                                              bool& isLoading,
                                              const std::function<void()>& closeProject,
                                              const std::function<void(const QString&)>& save,
                                              const std::function<bool(const QString&)>& load)
{
    if (changeDetected) {
        QMessageBox::StandardButton answer = QMessageBox::Discard;
        if (changeDetected) {
            answer = QMessageBox::question(&parent, "Load other Project", "Save the actual Project?",
                                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        }
        if (answer == QMessageBox::Save) {
            const QString path = QFileDialog::getSaveFileName(&parent, Text("Save Experiment"),
                                                               standardSavePath, ExperimentFilter());
            const QFileInfo fileInfo(path);
            standardSavePath = fileInfo.absolutePath();
            save(path);
        }
        if (answer == QMessageBox::Cancel)
            return;
    }

    const QString path = QFileDialog::getOpenFileName(&parent, Text("Load Experiment"),
                                                       standardSavePath, ExperimentFilter());
    if (!path.size())
        return;

    closeProject();
    const QFileInfo fileInfo(path);
    standardSavePath = fileInfo.absolutePath();
    isLoading = true;
    if (!load(path))
        savePath = path;
    isLoading = false;
}

void MainWindowProjectActions::CloseProjectWithPrompt(
        QWidget& parent, const QString& standardSavePath, bool& changeDetected,
        const std::function<bool()>& hasForms,
        const std::function<void(const QString&)>& save,
        const std::function<void()>& closeProject)
{
    if (hasForms()) {
        QMessageBox::StandardButton answer = QMessageBox::Discard;
        if (changeDetected) {
            answer = QMessageBox::question(&parent, "Close Project", "Do you want to save the Project?",
                                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        }
        if (answer == QMessageBox::Save) {
            const QString path = QFileDialog::getSaveFileName(&parent, Text("Save Experiment"),
                                                               standardSavePath, ExperimentFilter());
            save(path);
            closeProject();
        }
        if (answer == QMessageBox::Discard)
            closeProject();
    } else {
        closeProject();
    }
    changeDetected = false;
}
