#include "UiLayoutEditMode.h"

#include "mainwindow.h"

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QKeyEvent>
#include <QKeySequence>
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
constexpr auto LayoutParentProperty = "LabAnalyserLayoutParent";

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

QDomElement LayoutForParent(QDomDocument& document, const QString& parentName)
{
    QDomElement parent = FindWidget(document.documentElement(), parentName);
    if (parent.isNull())
        return {};
    return parent.firstChildElement(QStringLiteral("layout"));
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

QLayout* EditableLayoutAt(QWidget& form, const QPoint& globalPosition)
{
    QWidget* candidate = form.childAt(form.mapFromGlobal(globalPosition));
    while (candidate) {
        if (QLayout* layout = candidate->layout())
            return layout;
        if (candidate == &form)
            break;
        candidate = candidate->parentWidget();
    }
    return nullptr;
}

QWidget* LayoutItemForWidget(QWidget* widget, QLayout** owningLayout)
{
    for (QWidget* candidate = widget; candidate; candidate = candidate->parentWidget()) {
        QWidget* parent = candidate->parentWidget();
        if (!parent)
            continue;
        QLayout* layout = parent->layout();
        if (layout && layout->indexOf(candidate) >= 0) {
            if (owningLayout)
                *owningLayout = layout;
            return candidate;
        }
    }
    return nullptr;
}

void MarkGridPosition(QWidget& widget, QWidget* parent, int row, int column,
                      int rowSpan, int columnSpan)
{
    widget.setProperty(MovedProperty, true);
    widget.setProperty(LayoutParentProperty,
                       parent ? parent->property(OriginalNameProperty).toString() : QString());
    widget.setProperty(GridRowProperty, row);
    widget.setProperty(GridColumnProperty, column);
    widget.setProperty(GridRowSpanProperty, rowSpan);
    widget.setProperty(GridColumnSpanProperty, columnSpan);
    widget.setProperty(BeforeProperty, {});
}

void MarkBoxPosition(QWidget& widget, QWidget* parent, QWidget* after)
{
    widget.setProperty(MovedProperty, true);
    widget.setProperty(LayoutParentProperty,
                       parent ? parent->property(OriginalNameProperty).toString() : QString());
    widget.setProperty(BeforeProperty,
                       after ? after->property(OriginalNameProperty).toString() : QString());
    widget.setProperty(GridRowProperty, {});
    widget.setProperty(GridColumnProperty, {});
    widget.setProperty(GridRowSpanProperty, {});
    widget.setProperty(GridColumnSpanProperty, {});
}

bool MoveInGrid(QLayout& sourceLayout, QGridLayout& layout, QWidget& widget,
                const QPoint& position)
{
    const int sourceIndex = sourceLayout.indexOf(&widget);
    if (sourceIndex < 0)
        return false;

    int sourceRow = 0;
    int sourceColumn = 0;
    int sourceRowSpan = 1;
    int sourceColumnSpan = 1;
    if (auto* sourceGrid = qobject_cast<QGridLayout*>(&sourceLayout))
        sourceGrid->getItemPosition(sourceIndex, &sourceRow, &sourceColumn,
                                    &sourceRowSpan, &sourceColumnSpan);

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
    if (&sourceLayout == &layout && targetRow == sourceRow && targetColumn == sourceColumn)
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
        sourceLayout.removeWidget(&widget);
        layout.removeWidget(target);
        if (auto* sourceGrid = qobject_cast<QGridLayout*>(&sourceLayout))
            sourceGrid->addWidget(target, sourceRow, sourceColumn, targetRowSpan, targetColumnSpan);
        else if (auto* sourceBox = qobject_cast<QBoxLayout*>(&sourceLayout))
            sourceBox->insertWidget(sourceIndex, target);
        layout.addWidget(&widget, targetWidgetRow, targetWidgetColumn,
                         sourceRowSpan, sourceColumnSpan);
        if (auto* sourceGrid = qobject_cast<QGridLayout*>(&sourceLayout))
            MarkGridPosition(*target, sourceGrid->parentWidget(), sourceRow, sourceColumn,
                             targetRowSpan, targetColumnSpan);
        else if (auto* sourceBox = qobject_cast<QBoxLayout*>(&sourceLayout))
            MarkBoxPosition(*target, sourceBox->parentWidget(),
                            sourceIndex + 1 < sourceBox->count()
                                ? sourceBox->itemAt(sourceIndex + 1)->widget() : nullptr);
    } else {
        sourceLayout.removeWidget(&widget);
        layout.addWidget(&widget, targetRow, targetColumn, sourceRowSpan, sourceColumnSpan);
    }
    MarkGridPosition(widget, layout.parentWidget(), targetRow, targetColumn,
                     sourceRowSpan, sourceColumnSpan);
    return true;
}

bool MoveInBox(QLayout& sourceLayout, QBoxLayout& layout, QWidget& widget,
               const QPoint& position)
{
    const int sourceIndex = sourceLayout.indexOf(&widget);
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
    sourceLayout.removeWidget(&widget);
    if (&sourceLayout == &layout && destination > sourceIndex)
        --destination;
    if (&sourceLayout == &layout && destination == sourceIndex) {
        layout.insertWidget(sourceIndex, &widget);
        return false;
    }
    QWidget* target = destination < layout.count() ? layout.itemAt(destination)->widget() : nullptr;
    if (target && &sourceLayout != &layout) {
        layout.removeWidget(target);
        if (auto* sourceGrid = qobject_cast<QGridLayout*>(&sourceLayout)) {
            int row = 0;
            int column = 0;
            int rowSpan = 1;
            int columnSpan = 1;
            sourceGrid->getItemPosition(sourceIndex, &row, &column, &rowSpan, &columnSpan);
            sourceGrid->addWidget(target, row, column, rowSpan, columnSpan);
            MarkGridPosition(*target, sourceGrid->parentWidget(), row, column, rowSpan, columnSpan);
        } else if (auto* sourceBox = qobject_cast<QBoxLayout*>(&sourceLayout)) {
            sourceBox->insertWidget(sourceIndex, target);
            MarkBoxPosition(*target, sourceBox->parentWidget(),
                            sourceIndex + 1 < sourceBox->count()
                                ? sourceBox->itemAt(sourceIndex + 1)->widget() : nullptr);
        }
    }
    layout.insertWidget(destination, &widget);
    QWidget* after = destination + 1 < layout.count()
        ? layout.itemAt(destination + 1)->widget() : nullptr;
    MarkBoxPosition(widget, layout.parentWidget(), after);
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

UiLayoutEditMode::LayoutState UiLayoutEditMode::CaptureLayout(QLayout* layout) const
{
    LayoutState state;
    state.layout = layout;
    if (!layout)
        return state;
    for (int index = 0; index < layout->count(); ++index) {
        QWidget* itemWidget = layout->itemAt(index)->widget();
        if (!itemWidget)
            continue;
        LayoutItemState item;
        item.widget = itemWidget;
        item.index = index;
        if (auto* grid = qobject_cast<QGridLayout*>(layout))
            grid->getItemPosition(index, &item.row, &item.column, &item.rowSpan, &item.columnSpan);
        state.items.push_back(item);
    }
    return state;
}

void UiLayoutEditMode::MarkLayoutForSave(const LayoutState& state)
{
    if (!state.layout)
        return;
    if (auto* grid = qobject_cast<QGridLayout*>(state.layout.data())) {
        for (const LayoutItemState& item : state.items)
            if (item.widget)
                MarkGridPosition(*item.widget, grid->parentWidget(), item.row, item.column,
                                 item.rowSpan, item.columnSpan);
        return;
    }
    if (auto* box = qobject_cast<QBoxLayout*>(state.layout.data())) {
        for (int index = 0; index < state.items.size(); ++index) {
            QWidget* after = index + 1 < state.items.size()
                ? state.items.at(index + 1).widget.data() : nullptr;
            if (state.items.at(index).widget)
                MarkBoxPosition(*state.items.at(index).widget, box->parentWidget(), after);
        }
    }
}

void UiLayoutEditMode::UndoLastMove()
{
    if (undoHistory.isEmpty())
        return;
    const Move move = undoHistory.takeLast();
    for (const LayoutState& state : move.layouts)
        for (const LayoutItemState& item : state.items)
            if (item.widget && item.widget->parentWidget() && item.widget->parentWidget()->layout())
                item.widget->parentWidget()->layout()->removeWidget(item.widget);

    for (const LayoutState& state : move.layouts) {
        if (!state.layout)
            continue;
        if (auto* grid = qobject_cast<QGridLayout*>(state.layout.data())) {
            for (const LayoutItemState& item : state.items)
                if (item.widget)
                    grid->addWidget(item.widget, item.row, item.column, item.rowSpan, item.columnSpan);
        } else if (auto* box = qobject_cast<QBoxLayout*>(state.layout.data())) {
            for (const LayoutItemState& item : state.items)
                if (item.widget)
                    box->insertWidget(item.index, item.widget);
        }
        MarkLayoutForSave(state);
    }
    if (!move.layouts.isEmpty() && !move.layouts.first().items.isEmpty())
        if (QWidget* widget = move.layouts.first().items.first().widget)
            if (Form* form = FindForm(widget))
                form->dirty = true;
}

bool UiLayoutEditMode::eventFilter(QObject* watched, QEvent* event)
{
    auto* widget = qobject_cast<QWidget*>(watched);
    while (widget && !widget->property(OriginalNameProperty).isValid())
        widget = widget->parentWidget();
    if (!editEnabled || (!widget && !draggedWidget))
        return QObject::eventFilter(watched, event);

    if (event->type() == QEvent::KeyPress) {
        const auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->matches(QKeySequence::Undo)) {
            UndoLastMove();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (widget && mouseEvent->button() == Qt::LeftButton && widget->parentWidget()) {
            QLayout* owningLayout = nullptr;
            if (QWidget* layoutItem = LayoutItemForWidget(widget, &owningLayout)) {
                if (owningLayout) {
                    draggedWidget = layoutItem;
                    dragOffset = layoutItem->mapFromGlobal(mouseEvent->globalPosition().toPoint());
                    if (Form* form = FindForm(layoutItem)) {
                        dragForm = form->widget;
                        if (dragForm) {
                            auto* preview = new QLabel(dragForm);
                            preview->setPixmap(layoutItem->grab());
                            preview->resize(layoutItem->size());
                            preview->setAttribute(Qt::WA_TransparentForMouseEvents);
                            preview->setStyleSheet(QStringLiteral("border: 2px dashed #268bd2;"));
                            auto* opacity = new QGraphicsOpacityEffect(preview);
                            opacity->setOpacity(0.72);
                            preview->setGraphicsEffect(opacity);
                            preview->move(dragForm->mapFromGlobal(mouseEvent->globalPosition().toPoint())
                                          - dragOffset);
                            preview->show();
                            preview->raise();
                            dragPreview = preview;
                        }
                    }
                }
            }
            // Edit mode must not trigger the embedded control when that
            // control cannot be reordered by its current layout.
            return true;
        }
    } else if (event->type() == QEvent::MouseMove && draggedWidget
               && (static_cast<QMouseEvent*>(event)->buttons() & Qt::LeftButton)) {
        if (dragPreview && dragForm) {
            const auto* mouseEvent = static_cast<QMouseEvent*>(event);
            dragPreview->move(dragForm->mapFromGlobal(mouseEvent->globalPosition().toPoint()) - dragOffset);
            dragPreview->raise();
        }
        return true;
    } else if (event->type() == QEvent::MouseButtonRelease && draggedWidget) {
        QWidget* dragged = draggedWidget;
        bool moved = false;
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (dragPreview)
            dragPreview->hide();
        if (QWidget* sourceParent = dragged->parentWidget()) {
            if (QLayout* sourceLayout = sourceParent->layout()) {
                if (dragForm && FindForm(dragged) == FindForm(dragForm)) {
                    if (QLayout* targetLayout = EditableLayoutAt(*dragForm,
                        mouseEvent->globalPosition().toPoint())) {
                        Move undo;
                        undo.layouts.push_back(CaptureLayout(sourceLayout));
                        if (targetLayout != sourceLayout)
                            undo.layouts.push_back(CaptureLayout(targetLayout));
                        const QPoint targetPosition = targetLayout->parentWidget()->mapFromGlobal(
                            mouseEvent->globalPosition().toPoint());
                        if (auto* grid = qobject_cast<QGridLayout*>(targetLayout))
                            moved = MoveInGrid(*sourceLayout, *grid, *dragged, targetPosition);
                        else if (auto* box = qobject_cast<QBoxLayout*>(targetLayout))
                            moved = MoveInBox(*sourceLayout, *box, *dragged, targetPosition);
                        if (moved)
                            undoHistory.push_back(undo);
                    }
                }
            }
        }
        if (moved) {
            if (Form* form = FindForm(dragged))
                form->dirty = true;
        }
        if (dragPreview)
            dragPreview->deleteLater();
        dragPreview = nullptr;
        dragForm = nullptr;
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
            QDomElement targetLayout = LayoutForParent(document,
                child->property(LayoutParentProperty).toString());
            if (!targetLayout.isNull() && item.parentNode() != targetLayout)
                targetLayout.appendChild(item);
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
            widget->setProperty(LayoutParentProperty, {});
        }
    return true;
}
