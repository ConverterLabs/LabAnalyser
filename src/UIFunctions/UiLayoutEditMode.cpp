#include "UiLayoutEditMode.h"

#include "mainwindow.h"

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLayout>
#include <QMouseEvent>
#include <QSaveFile>
#include <QWidget>

#include <limits>

namespace {
constexpr auto OriginalNameProperty = "LabAnalyserOriginalObjectName";
constexpr auto MovedProperty = "LabAnalyserLayoutMoved";
constexpr auto GridRowProperty = "LabAnalyserLayoutGridRow";
constexpr auto GridColumnProperty = "LabAnalyserLayoutGridColumn";
constexpr auto GridRowSpanProperty = "LabAnalyserLayoutGridRowSpan";
constexpr auto GridColumnSpanProperty = "LabAnalyserLayoutGridColumnSpan";
constexpr auto BeforeProperty = "LabAnalyserLayoutBefore";

QDomElement FindWidget(QDomElement element, const QString& name)
{
    if (element.tagName() == QStringLiteral("widget")
        && element.attribute(QStringLiteral("name")) == name)
        return element;
    for (QDomElement child = element.firstChildElement(); !child.isNull();
         child = child.nextSiblingElement()) {
        QDomElement result = FindWidget(child, name);
        if (!result.isNull())
            return result;
    }
    return {};
}

QDomElement ItemForWidget(QDomElement widget)
{
    const QDomElement item = widget.parentNode().toElement();
    return item.tagName() == QStringLiteral("item") ? item : QDomElement{};
}

QWidget* GridWidgetAt(QGridLayout& layout, int row, int column)
{
    for (int index = 0; index < layout.count(); ++index) {
        int itemRow = 0;
        int itemColumn = 0;
        int rowSpan = 0;
        int columnSpan = 0;
        layout.getItemPosition(index, &itemRow, &itemColumn, &rowSpan, &columnSpan);
        if (row >= itemRow && row < itemRow + rowSpan
            && column >= itemColumn && column < itemColumn + columnSpan)
            return layout.itemAt(index)->widget();
    }
    return nullptr;
}

bool MoveInGrid(QGridLayout& layout, QWidget& widget, const QPoint& position)
{
    const int sourceIndex = layout.indexOf(&widget);
    if (sourceIndex < 0)
        return false;

    int sourceRow = 0;
    int sourceColumn = 0;
    int sourceRowSpan = 1;
    int sourceColumnSpan = 1;
    layout.getItemPosition(sourceIndex, &sourceRow, &sourceColumn, &sourceRowSpan, &sourceColumnSpan);

    int targetRow = sourceRow;
    int targetColumn = sourceColumn;
    qint64 bestDistance = std::numeric_limits<qint64>::max();
    for (int row = 0; row < layout.rowCount(); ++row) {
        for (int column = 0; column < layout.columnCount(); ++column) {
            const QRect cell = layout.cellRect(row, column);
            if (cell.isEmpty())
                continue;
            if (cell.contains(position)) {
                targetRow = row;
                targetColumn = column;
                row = layout.rowCount();
                break;
            }
            const QPoint delta = cell.center() - position;
            const qint64 distance = qint64(delta.x()) * delta.x() + qint64(delta.y()) * delta.y();
            if (distance < bestDistance) {
                bestDistance = distance;
                targetRow = row;
                targetColumn = column;
            }
        }
    }
    if (targetRow == sourceRow && targetColumn == sourceColumn)
        return false;

    QWidget* target = GridWidgetAt(layout, targetRow, targetColumn);
    if (target && target != &widget) {
        const int targetIndex = layout.indexOf(target);
        int targetWidgetRow = 0;
        int targetWidgetColumn = 0;
        int targetRowSpan = 1;
        int targetColumnSpan = 1;
        layout.getItemPosition(targetIndex, &targetWidgetRow, &targetWidgetColumn,
                               &targetRowSpan, &targetColumnSpan);
        layout.removeWidget(&widget);
        layout.removeWidget(target);
        layout.addWidget(target, sourceRow, sourceColumn, targetRowSpan, targetColumnSpan);
        layout.addWidget(&widget, targetWidgetRow, targetWidgetColumn,
                         sourceRowSpan, sourceColumnSpan);
        target->setProperty(MovedProperty, true);
        target->setProperty(GridRowProperty, sourceRow);
        target->setProperty(GridColumnProperty, sourceColumn);
        target->setProperty(GridRowSpanProperty, targetRowSpan);
        target->setProperty(GridColumnSpanProperty, targetColumnSpan);
    } else {
        layout.removeWidget(&widget);
        layout.addWidget(&widget, targetRow, targetColumn, sourceRowSpan, sourceColumnSpan);
    }
    widget.setProperty(MovedProperty, true);
    widget.setProperty(GridRowProperty, targetRow);
    widget.setProperty(GridColumnProperty, targetColumn);
    widget.setProperty(GridRowSpanProperty, sourceRowSpan);
    widget.setProperty(GridColumnSpanProperty, sourceColumnSpan);
    return true;
}

bool MoveInBox(QBoxLayout& layout, QWidget& widget, const QPoint& position)
{
    const int sourceIndex = layout.indexOf(&widget);
    if (sourceIndex < 0)
        return false;
    int destination = layout.count();
    for (int index = 0; index < layout.count(); ++index) {
        QWidget* candidate = layout.itemAt(index)->widget();
        if (!candidate || candidate == &widget)
            continue;
        const QPoint center = candidate->geometry().center();
        const bool before = layout.direction() == QBoxLayout::LeftToRight
                            || layout.direction() == QBoxLayout::RightToLeft
            ? position.x() < center.x() : position.y() < center.y();
        if (before) {
            destination = index;
            break;
        }
    }
    layout.removeWidget(&widget);
    if (destination > sourceIndex)
        --destination;
    if (destination == sourceIndex) {
        layout.insertWidget(sourceIndex, &widget);
        return false;
    }
    layout.insertWidget(destination, &widget);
    QWidget* after = destination + 1 < layout.count()
        ? layout.itemAt(destination + 1)->widget() : nullptr;
    widget.setProperty(MovedProperty, true);
    widget.setProperty(BeforeProperty,
                       after ? after->property(OriginalNameProperty).toString() : QString());
    return true;
}
}

UiLayoutEditMode::UiLayoutEditMode(MainWindow& mainWindow)
    : QObject(&mainWindow)
{
    setObjectName(QStringLiteral("UiLayoutEditMode"));
}

UiLayoutEditMode* UiLayoutEditMode::For(MainWindow& mainWindow)
{
    if (auto* existing = mainWindow.findChild<UiLayoutEditMode*>(QStringLiteral("UiLayoutEditMode")))
        return existing;
    return new UiLayoutEditMode(mainWindow);
}

void UiLayoutEditMode::RegisterForm(QWidget* form, const QString& sourceFile)
{
    if (!form)
        return;
    forms.push_back({form, sourceFile, false});
    form->setProperty(OriginalNameProperty, form->objectName());
    form->installEventFilter(this);
    for (QWidget* child : form->findChildren<QWidget*>())
        child->installEventFilter(this);
    SetEditCursor(form, editEnabled);
}

void UiLayoutEditMode::SetEnabled(bool enabled)
{
    editEnabled = enabled;
    if (enabled)
        qApp->installEventFilter(this);
    else
        qApp->removeEventFilter(this);
    for (const Form& form : forms)
        if (form.widget)
            SetEditCursor(form.widget, enabled);
}

void UiLayoutEditMode::SetEditCursor(QWidget* form, bool enabled)
{
    for (QWidget* child : form->findChildren<QWidget*>()) {
        if (child->property(OriginalNameProperty).isValid())
            child->setCursor(enabled ? Qt::SizeAllCursor : Qt::ArrowCursor);
    }
}

UiLayoutEditMode::Form* UiLayoutEditMode::FindForm(QWidget* widget)
{
    for (Form& form : forms) {
        if (form.widget && (widget == form.widget || form.widget->isAncestorOf(widget)))
            return &form;
    }
    return nullptr;
}

bool UiLayoutEditMode::eventFilter(QObject* watched, QEvent* event)
{
    auto* widget = qobject_cast<QWidget*>(watched);
    while (widget && !widget->property(OriginalNameProperty).isValid())
        widget = widget->parentWidget();
    if (!editEnabled || !widget)
        return QObject::eventFilter(watched, event);

    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton && widget->parentWidget()) {
            if (QLayout* layout = widget->parentWidget()->layout()) {
                if (layout->indexOf(widget) >= 0) {
                    draggedWidget = widget;
                    dragOffset = mouseEvent->position().toPoint();
                }
            }
            // Edit mode must not trigger the embedded control when that
            // control cannot be reordered by its current layout.
            return true;
        }
    } else if (event->type() == QEvent::MouseMove && draggedWidget == widget
               && (static_cast<QMouseEvent*>(event)->buttons() & Qt::LeftButton)) {
        // The owning layout remains authoritative while dragging.  The move is
        // committed on release, so no widget becomes a free-floating child.
        return true;
    } else if (event->type() == QEvent::MouseButtonRelease && draggedWidget == widget) {
        bool moved = false;
        if (QWidget* parent = widget->parentWidget()) {
            if (QLayout* layout = parent->layout()) {
                const QPoint position = parent->mapFromGlobal(
                    static_cast<QMouseEvent*>(event)->globalPosition().toPoint());
                if (auto* grid = qobject_cast<QGridLayout*>(layout))
                    moved = MoveInGrid(*grid, *widget, position);
                else if (auto* box = qobject_cast<QBoxLayout*>(layout))
                    moved = MoveInBox(*box, *widget, position);
            }
        }
        if (moved) {
            if (Form* form = FindForm(widget))
            form->dirty = true;
        }
        draggedWidget = nullptr;
        return true;
    }
    return QObject::eventFilter(watched, event);
}

bool UiLayoutEditMode::SaveDirtyForms(QString* errorMessage)
{
    for (Form& form : forms) {
        if (!form.dirty || !form.widget)
            continue;
        if (!SaveLayout(form.sourceFile, *form.widget, errorMessage))
            return false;
        form.dirty = false;
    }
    return true;
}

bool UiLayoutEditMode::SaveLayout(const QString& fileName, QWidget& form, QString* errorMessage)
{
    QFile input(fileName);
    if (!input.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = QObject::tr("Could not open UI file: %1").arg(fileName);
        return false;
    }
    QDomDocument document;
    QString parseError;
    if (!document.setContent(&input, &parseError)) {
        if (errorMessage) *errorMessage = QObject::tr("Could not parse UI file %1: %2").arg(fileName, parseError);
        return false;
    }

    QVector<QPointer<QWidget>> savedWidgets;
    for (QWidget* child : form.findChildren<QWidget*>()) {
        if (!child->property(MovedProperty).toBool())
            continue;
        const QString originalName = child->property(OriginalNameProperty).toString();
        if (originalName.isEmpty())
            continue;
        QDomElement sourceWidget = FindWidget(document.documentElement(), originalName);
        if (!sourceWidget.isNull()) {
            QDomElement item = ItemForWidget(sourceWidget);
            if (item.isNull())
                continue;
            if (child->property(GridRowProperty).isValid()) {
                item.setAttribute(QStringLiteral("row"), child->property(GridRowProperty).toInt());
                item.setAttribute(QStringLiteral("column"), child->property(GridColumnProperty).toInt());
                item.setAttribute(QStringLiteral("rowspan"), child->property(GridRowSpanProperty).toInt());
                item.setAttribute(QStringLiteral("colspan"), child->property(GridColumnSpanProperty).toInt());
            } else if (child->property(BeforeProperty).isValid()) {
                const QString beforeName = child->property(BeforeProperty).toString();
                QDomNode layout = item.parentNode();
                if (!beforeName.isEmpty()) {
                    QDomElement before = FindWidget(document.documentElement(), beforeName);
                    QDomElement beforeItem = ItemForWidget(before);
                    if (!beforeItem.isNull() && beforeItem.parentNode() == layout)
                        layout.insertBefore(item, beforeItem);
                    else
                        layout.appendChild(item);
                } else {
                    layout.appendChild(item);
                }
            }
            savedWidgets.push_back(child);
        }
    }

    QSaveFile output(fileName);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = QObject::tr("Could not write UI file: %1").arg(fileName);
        return false;
    }
    output.write(document.toByteArray(2));
    if (!output.commit()) {
        if (errorMessage) *errorMessage = QObject::tr("Could not replace UI file: %1").arg(fileName);
        return false;
    }
    for (const QPointer<QWidget>& widget : savedWidgets)
        if (widget) {
            widget->setProperty(MovedProperty, false);
            widget->setProperty(GridRowProperty, {});
            widget->setProperty(GridColumnProperty, {});
            widget->setProperty(GridRowSpanProperty, {});
            widget->setProperty(GridColumnSpanProperty, {});
            widget->setProperty(BeforeProperty, {});
        }
    return true;
}
