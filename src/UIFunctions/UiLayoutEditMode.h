#pragma once

#include <QObject>
#include <QPointer>
#include <QPoint>
#include <QVector>

class MainWindow;
class QWidget;

// A deliberately small runtime layout editor for loaded user forms.  It only
// moves existing widgets; it never changes widget classes, names or bindings.
class UiLayoutEditMode final : public QObject
{
    Q_OBJECT
public:
    static UiLayoutEditMode* For(MainWindow& mainWindow);
    void RegisterForm(QWidget* form, const QString& sourceFile);
    void SetEnabled(bool enabled);
    bool SaveDirtyForms(QString* errorMessage = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    explicit UiLayoutEditMode(MainWindow& mainWindow);
    struct Form {
        QPointer<QWidget> widget;
        QString sourceFile;
        bool dirty = false;
    };
    Form* FindForm(QWidget* widget);
    static bool SaveLayout(const QString& fileName, QWidget& form, QString* errorMessage);
    void SetEditCursor(QWidget* form, bool enabled);

    bool editEnabled = false;
    QPointer<QWidget> draggedWidget;
    QPoint dragOffset;
    QVector<Form> forms;
};
