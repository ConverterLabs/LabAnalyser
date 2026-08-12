#ifndef MAINWINDOWPROJECTACTIONS_H
#define MAINWINDOWPROJECTACTIONS_H

#include <functional>

class QString;
class QWidget;

class MainWindowProjectActions
{
public:
    static void SaveExperiment(QWidget& parent, QString& standardSavePath,
                               bool& changeDetected,
                               const std::function<void(const QString&)>& save);
    static void LoadExperiment(QWidget& parent, QString& standardSavePath,
                               QString& savePath, bool& changeDetected, bool& isLoading,
                               const std::function<void()>& closeProject,
                               const std::function<void(const QString&)>& save,
                               const std::function<bool(const QString&)>& load);
    static void CloseProjectWithPrompt(QWidget& parent, const QString& standardSavePath,
                                       bool& changeDetected,
                                       const std::function<bool()>& hasForms,
                                       const std::function<void(const QString&)>& save,
                                       const std::function<void()>& closeProject);
};

#endif // MAINWINDOWPROJECTACTIONS_H
