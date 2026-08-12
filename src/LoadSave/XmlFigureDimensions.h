#ifndef XMLFIGUREDIMENSIONS_H
#define XMLFIGUREDIMENSIONS_H

#include <QString>

// Internal XML-reader boundary: validates figure dimensions before any GUI
// object is created. It has no QObject or MainWindow dependency.
namespace XmlFigureDimensions
{
bool ParseAndValidate(const QString& rowsText, const QString& colsText,
                      int* rows, int* cols, QString* errorText);
}

#endif // XMLFIGUREDIMENSIONS_H
