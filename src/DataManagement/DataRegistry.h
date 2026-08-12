/***************************************************************************
** Internal value registry used exclusively by DataManagementClass.
**
** This type deliberately preserves the current facade semantics, including
** ordered duplicate form entries, alias fallback and duplicate plot-number
** history. It owns no QObject and has no GUI, IO, plugin, network or
** Messenger dependency; stored QObject pointers are non-owning observations.
***************************************************************************/

#ifndef DATAREGISTRY_H
#define DATAREGISTRY_H

#include <QString>

#include <map>
#include <utility>
#include <vector>

class QObject;

class DataRegistry
{
public:
    int PlotCount() const;
    int GetUniquePlotNumber() const;
    void AddPlotPointer(QString id, QObject* pointer);
    void AddPlotPointer(QString id, QObject* pointer, int number);
    void RenamePlotPointer(QString oldId, QString newId);
    void DeletePlotPointer(QString id);
    QObject* GetPlotByName(QString name) const;

    void AddPlotWindow(QString id, int rows, int cols);
    void AddPlotWindow(QString id, int rows, int cols, int number);
    void DeletePlotWindow(QString id);
    std::pair<int, int> GetPlotWindowRowsCols(QString name);
    int GetPlotWindowsIncrementer() const;

    void AddFormFile(std::pair<QString, QString> file);
    void AddSkipFormFile(QString fileName, bool skip);
    bool GetSkipFormFile(QString fileName) const;
    void RemoveFormFile(QString fileName);
    int GetFormFileCount() const;
    std::pair<QString, QString> GetFormFileEntry(int index) const;

    void SetAlias(QString id, QString alias);
    QString GetAlias(QString id) const;

    void Clear();

private:
    std::vector<std::pair<QString, QString>> FormFiles;
    std::map<QString, bool> SkipFormFiles;
    std::map<QString, QObject*> PlotObjects;
    std::map<QString, int> PlotObjectsNumber;
    std::vector<int> PlotObjectsNumbers;
    std::map<QString, std::pair<int, int>> PlotWindows;
    int PlotWindowsIncrementer = 0;
    std::map<QString, int> PlotWindowNumber;
    std::vector<int> PlotWindowNumbers;
    std::map<QString, QString> AliasMap;
};

#endif
