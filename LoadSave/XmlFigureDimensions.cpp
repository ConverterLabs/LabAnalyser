#include "XmlFigureDimensions.h"

namespace XmlFigureDimensions
{
bool ParseAndValidate(const QString& rowsText, const QString& colsText,
                      int* rows, int* cols, QString* errorText)
{
    bool rowsOk = false;
    bool colsOk = false;
    const int parsedRows = rowsText.toInt(&rowsOk);
    const int parsedCols = colsText.toInt(&colsOk);
    if (!rowsOk || !colsOk || parsedRows < 0 || parsedCols < 0 || parsedRows > 32 || parsedCols > 32)
    {
        if (errorText)
            *errorText = QStringLiteral("Figure window Rows and Cols must be integers between 0 and 32.");
        return false;
    }

    if (parsedRows != 0 && parsedCols > 256 / parsedRows)
    {
        if (errorText)
            *errorText = QStringLiteral("Figure window Rows x Cols must not exceed 256.");
        return false;
    }

    if (rows)
        *rows = parsedRows;
    if (cols)
        *cols = parsedCols;
    return true;
}
}
