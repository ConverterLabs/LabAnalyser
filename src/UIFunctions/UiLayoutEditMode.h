#pragma once

#include <QObject>
#include <QPointer>
#include <QPoint>
#include <QVector>

class MainWindow;
class QWidget;
class QLabel;
class QLayout;

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
    struct LayoutState;
    struct Form {
        QPointer<QWidget> widget;
        QString sourceFile;
        bool dirty = false;
    };
    Form* FindForm(QWidget* widget);
    static bool SaveLayout(const QString& fileName, QWidget& form, QString* errorMessage);
    void SetEditCursor(QWidget* form, bool enabled);
    LayoutState CaptureLayout(QLayout* layout) const;
    void MarkLayoutForSave(const LayoutState& state);
    void UndoLastMove();

    struct LayoutItemState {
        QPointer<QWidget> widget;
        int index = -1;
        int row = 0;
        int column = 0;
        int rowSpan = 1;
        int columnSpan = 1;
    };
    struct LayoutState {
        QPointer<QLayout> layout;
        QVector<LayoutItemState> items;
    };
    struct Move {
        QVector<LayoutState> layouts;
    };

    bool editEnabled = false;
    QPointer<QWidget> draggedWidget;
    QPointer<QWidget> dragForm;
    QPointer<QLabel> dragPreview;
    QPoint dragOffset;
    QVector<Form> forms;
    QVector<Move> undoHistory;
};
