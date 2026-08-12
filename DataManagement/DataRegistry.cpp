#include "DataRegistry.h"

#include <QObject>

#include <algorithm>

int DataRegistry::PlotCount() const
{
    return static_cast<int>(PlotObjects.size());
}

int DataRegistry::GetUniquePlotNumber() const
{
    int lastNumber = 0;
    if (!PlotObjectsNumbers.size())
        return 0;
    if (PlotObjectsNumbers[0] != 0)
        return 0;
    for (auto index = 1; index < PlotObjectsNumbers.size(); index++) {
        if (PlotObjectsNumbers[index] - lastNumber > 1)
            break;
        lastNumber = PlotObjectsNumbers[index];
    }
    return lastNumber + 1;
}

void DataRegistry::AddPlotPointer(QString id, QObject* pointer)
{
    PlotObjects[id] = pointer;
}

void DataRegistry::AddPlotPointer(QString id, QObject* pointer, int number)
{
    PlotObjects[id] = pointer;
    PlotObjectsNumber[id] = number;
    PlotObjectsNumbers.push_back(number);
    std::sort(PlotObjectsNumbers.begin(), PlotObjectsNumbers.end());
}

void DataRegistry::RenamePlotPointer(QString oldId, QString newId)
{
    auto it = PlotObjects.find(oldId);
    auto itNumber = PlotObjectsNumber.find(oldId);

    if (it != PlotObjects.end()) {
        auto data = PlotObjects[oldId];
        PlotObjects.erase(it);

        if (itNumber != PlotObjectsNumber.end()) {
            int number = PlotObjectsNumber[oldId];
            PlotObjectsNumber.erase(itNumber);
            auto numberVectorIt = std::find(PlotObjectsNumbers.begin(), PlotObjectsNumbers.end(), number);
            if (numberVectorIt != PlotObjectsNumbers.end())
                PlotObjectsNumbers.erase(numberVectorIt);

            const QStringList nameParts = newId.split("#");
            if (nameParts.size() > 1)
                number = nameParts.at(1).toInt() - 1;
            AddPlotPointer(newId, data, number);
        } else {
            AddPlotPointer(newId, data);
        }
    }
}

void DataRegistry::DeletePlotPointer(QString id)
{
    auto it = PlotObjects.find(id);
    if (it != PlotObjects.end())
        PlotObjects.erase(it);

    auto itNumber = PlotObjectsNumber.find(id);
    if (itNumber != PlotObjectsNumber.end()) {
        auto number = PlotObjectsNumber[id];
        PlotObjectsNumber.erase(itNumber);
        auto numberVectorIt = std::find(PlotObjectsNumbers.begin(), PlotObjectsNumbers.end(), number);
        if (numberVectorIt != PlotObjectsNumbers.end())
            PlotObjectsNumbers.erase(numberVectorIt);
    }
}

QObject* DataRegistry::GetPlotByName(QString name) const
{
    for (auto entry : PlotObjects)
        if (entry.second->objectName().compare(name) == 0)
            return entry.second;

    return nullptr;
}

void DataRegistry::AddPlotWindow(QString id, int rows, int cols, int number)
{
    PlotWindows[id] = std::pair<int, int>(rows, cols);
    PlotWindowNumber[id] = number;
    PlotWindowNumbers.push_back(number);
    std::sort(PlotWindowNumbers.begin(), PlotWindowNumbers.end());
    PlotWindowsIncrementer++;
}

void DataRegistry::AddPlotWindow(QString id, int rows, int cols)
{
    PlotWindows[id] = std::pair<int, int>(rows, cols);
    PlotWindowsIncrementer++;
}

void DataRegistry::DeletePlotWindow(QString id)
{
    auto it = PlotWindows.find(id);
    if (it != PlotWindows.end())
        PlotWindows.erase(it);

    auto itNumber = PlotWindowNumber.find(id);
    if (itNumber != PlotWindowNumber.end()) {
        auto number = PlotWindowNumber[id];
        PlotWindowNumber.erase(itNumber);
        auto numberVectorIt = std::find(PlotWindowNumbers.begin(), PlotWindowNumbers.end(), number);
        PlotWindowNumbers.erase(numberVectorIt);
    }
}

std::pair<int, int> DataRegistry::GetPlotWindowRowsCols(QString name)
{
    return PlotWindows[name];
}

int DataRegistry::GetPlotWindowsIncrementer() const
{
    int lastNumber = 0;
    if (!PlotWindowNumbers.size())
        return 0;
    if (PlotWindowNumbers[0] != 0)
        return 0;
    for (auto index = 1; index < PlotWindowNumbers.size(); index++) {
        if (PlotWindowNumbers[index] - lastNumber > 1)
            break;
        lastNumber = PlotWindowNumbers[index];
    }
    return lastNumber + 1;
}

void DataRegistry::AddFormFile(std::pair<QString, QString> file)
{
    FormFiles.push_back(file);
}

void DataRegistry::AddSkipFormFile(QString fileName, bool skip)
{
    SkipFormFiles[fileName] = skip;
}

bool DataRegistry::GetSkipFormFile(QString fileName) const
{
    auto it = SkipFormFiles.find(fileName);
    if (it != SkipFormFiles.end())
        return it->second;
    return false;
}

void DataRegistry::RemoveFormFile(QString fileName)
{
    int foundIndex = -1;
    for (int index = 0; index < FormFiles.size() && foundIndex == -1; index++) {
        if (FormFiles[index].first.compare(fileName) == 0)
            foundIndex = index;
    }
    if (foundIndex != -1)
        FormFiles.erase(FormFiles.begin() + foundIndex);
}

int DataRegistry::GetFormFileCount() const
{
    return static_cast<int>(FormFiles.size());
}

std::pair<QString, QString> DataRegistry::GetFormFileEntry(int index) const
{
    if (index < 0 || index >= static_cast<int>(FormFiles.size()))
        return std::pair<QString, QString>();
    return FormFiles[index];
}

void DataRegistry::SetAlias(QString id, QString alias)
{
    AliasMap[id] = alias;
}

QString DataRegistry::GetAlias(QString id) const
{
    auto it = AliasMap.find(id);
    if (it != AliasMap.end())
        return it->second;
    return id;
}

void DataRegistry::Clear()
{
    FormFiles.clear();
    SkipFormFiles.clear();
    PlotObjects.clear();
    PlotObjectsNumber.clear();
    PlotObjectsNumbers.clear();
    PlotWindowsIncrementer = 0;
    PlotWindows.clear();
    PlotWindowNumber.clear();
    PlotWindowNumbers.clear();
    AliasMap.clear();
}
