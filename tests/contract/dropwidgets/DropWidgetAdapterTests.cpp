#include <QtTest>
#include <QSignalSpy>
#include <QPointer>
#include <QFile>
#include <QDragEnterEvent>
#include <QAction>
#include <QImage>
#include <QTimer>
#include "DropWidgets/CreateID.h"
#include "DropWidgets/DropWidgetDragSource.h"
#include "DropWidgets/DropWidgetBinding.h"
#include "DropWidgets/DropWidgetIndicatorBinding.h"
#include "DropWidgets/DropWidgetTreePath.h"
#include "DropWidgets/DropWidgetsUiLoader.h"
#include "TreeWidgetCustomDrop.h"
#include "DropWidgets/QBLed.h"
#include "DropWidgets/QCheckBox.h"
#include "DropWidgets/QComboBox.h"
#include "DropWidgets/QDoubleSpinBox.h"
#include "DropWidgets/QLCDNumber.h"
#include "DropWidgets/QLabel.h"
#include "DropWidgets/QLed.h"
#include "DropWidgets/QLineEdit.h"
#include "DropWidgets/QListView.h"
#include "DropWidgets/QProgressBar.h"
#include "DropWidgets/QPushButton.h"
#include "DropWidgets/QSlider.h"
#include "DropWidgets/QSpinBox.h"
#include "DropWidgets/QTableWidgeD.h"
#include "DropWidgets/QTSLed.h"

Q_DECLARE_METATYPE(InterfaceData)

class DropWidgetAdapterTests : public QObject
{
    Q_OBJECT
    static ToFormMapper mapper(const QString& type) { ToFormMapper value(type, "Parameter"); value.SetDataType(type); return value; }
private slots:
    void initTestCase();
    void DW_001_construction_parenting_and_defaults();
    void DW_002_numeric_set_get_and_signal_suppression();
    void DW_003_string_selection_and_list_adapters();
    void DW_004_bit_and_xml_adapter_contracts();
    void DW_005_xml_stubs_and_widget_special_cases();
    void DW_006_loader_maps_standard_widgets_and_properties();
    void DW_007_loader_maps_dropwidgets_and_parent_hierarchy();
    void DW_008_loader_rejects_missing_malformed_and_unknown_ui();
    void DW_009_tree_mime_and_invalid_drag_contract();
    void DW_010_connect_and_table_row_actions();
    void DW_011_manager_to_widget_and_widget_to_message();
    void DW_012_duplicate_and_removed_connections();
    void DW_013_table_createrow_bindings_and_context_action();
    void DW_014_indicator_colors_state_and_offscreen_render();
    void DW_015_blink_timer_repeats_and_is_owned();
    void DW_016_data_driven_adapter_signals_ranges_and_values();
    void DW_017_isolated_xml_properties_and_list_mutation();
    void DW_018_equal_slider_bounds_preserve_value();
    void DW_019_drag_source_rejects_empty_and_nonleaf_selection();
    void DW_020_shared_indicator_initialization_preserves_checkbox_state_contract();
    void DW_021_shared_tree_item_paths_preserve_multi_selection_ids();
    void DW_022_unbound_button_timeout_is_safe_noop();
    void DW_023_remove_connection_preserves_widget_resets();
    void DW_024_missing_mainwindow_context_is_safe_noop();
};

class ExposedTreeWidget : public TreeWidgetCustomDrop
{
public:
    explicit ExposedTreeWidget(QWidget* parent = nullptr) : TreeWidgetCustomDrop(parent) {}
    using TreeWidgetCustomDrop::mimeData;
    using TreeWidgetCustomDrop::mimeTypes;
};

static QString fixture(const QString& name)
{
    return QFINDTESTDATA("../../fixtures/gui/" + name);
}

static QWidget* loadFixture(const QString& name, DropWidgetsUiLoader& loader, QFile& file)
{
    file.setFileName(fixture(name));
    if (!file.open(QIODevice::ReadOnly))
        return nullptr;
    return loader.load(&file);
}

void DropWidgetAdapterTests::initTestCase()
{
    QCOMPARE(qEnvironmentVariable("QT_QPA_PLATFORM"), QString("offscreen"));
    qRegisterMetaType<InterfaceData>("InterfaceData");
    new MainWindow;
}

void DropWidgetAdapterTests::DW_001_construction_parenting_and_defaults()
{
    QWidget parent;
    QList<QWidget*> widgets { new QBLed(&parent), new QCheckBoxD(&parent), new QComboBoxD(&parent),
        new QDoubleSpinBoxD(&parent), new QLCDNumberD(&parent), new QLabelD(&parent), new QLed(&parent),
        new QLineEditD(&parent), new QListViewD(&parent), new QProgressBarD(&parent), new QPushButtonD(&parent),
        new QSliderD(&parent), new QSpinBoxD(&parent), new QTableWidgeD(&parent), new QTSLed(&parent) };
    for (QWidget* widget : widgets) {
        QCOMPARE(widget->parentWidget(), &parent);
        QVERIFY(widget->isEnabled());
        QCOMPARE(widget->objectName(), QString());
    }
    QCOMPARE(qobject_cast<QComboBoxD*>(widgets.at(2))->count(), 0);
    QCOMPARE(qobject_cast<QListViewD*>(widgets.at(8))->model->rowCount(), 0);
    QCOMPARE(qobject_cast<QTableWidgeD*>(widgets.at(13))->rowCount(), 0);
    QTreeWidget tree; QTreeWidgetItem root(&tree, QStringList("root")); QTreeWidgetItem leaf(&root, QStringList("leaf")); tree.setCurrentItem(&leaf);
    QCOMPARE(CreateID(&tree), QString("root::leaf")); QCOMPARE(CreateIDs(&tree), QStringList({"root::leaf"})); tree.clearSelection(); QCOMPARE(CreateID(&tree), QString());
    QPointer<QLineEditD> owned;
    { QWidget owner; owned = new QLineEditD(&owner); QVERIFY(owned); }
    QVERIFY(owned.isNull());
}

void DropWidgetAdapterTests::DW_019_drag_source_rejects_empty_and_nonleaf_selection()
{
    QWidget foreignSource;
    QVERIFY(!DropWidgetDragSource::HasFirstSelectedLeaf(&foreignSource));

    QTreeWidget tree;
    QTreeWidgetItem root(&tree, QStringList("root"));
    QTreeWidgetItem leaf(&root, QStringList("leaf"));
    QVERIFY(!DropWidgetDragSource::HasFirstSelectedLeaf(&tree));

    root.setSelected(true);
    QVERIFY(!DropWidgetDragSource::HasFirstSelectedLeaf(&tree));

    root.setSelected(false);
    leaf.setSelected(true);
    QVERIFY(DropWidgetDragSource::HasFirstSelectedLeaf(&tree));
    const QString id("root::leaf");
    GetMainWindow()->GetLogic()->AddContainerElement(id, "bool", "Parameter", "");
    QCOMPARE(DropWidgetDragSource::ContainerForFirstSelectedLeaf(&tree),
             GetMainWindow()->GetLogic()->GetContainer(id));
}

void DropWidgetAdapterTests::DW_020_shared_indicator_initialization_preserves_checkbox_state_contract()
{
    QWidget parent;
    ToFormMapper value = mapper("bool");
    value.SetData(true);
    uint32_t bit = 7;
    uint32_t bitCounter = 11;
    bool state = false;

    DropWidgetIndicatorBinding::InitializeState(&parent, &value, bit, bitCounter,
                                                 QStringLiteral("title"), QStringLiteral("prompt"),
                                                 [&state](bool updated) { state = updated; });

    QCOMPARE(bit, uint32_t(0));
    QCOMPARE(bitCounter, uint32_t(11));
    QVERIFY(state);
}

void DropWidgetAdapterTests::DW_021_shared_tree_item_paths_preserve_multi_selection_ids()
{
    QTreeWidget tree;
    QTreeWidgetItem root(&tree, QStringList("root"));
    QTreeWidgetItem branch(&root, QStringList("branch"));
    QTreeWidgetItem leaf(&branch, QStringList("leaf"));
    QCOMPARE(DropWidgetTreePath::IdForItem(&leaf), QString("root::branch::leaf"));
}

void DropWidgetAdapterTests::DW_022_unbound_button_timeout_is_safe_noop()
{
    QPushButtonD button;
    button.setDown(false);
    QSignalSpy released(&button, &QPushButton::released);
    button.TimeOut();
    QCOMPARE(released.count(), 0);
}

void DropWidgetAdapterTests::DW_023_remove_connection_preserves_widget_resets()
{
    QCheckBoxD check;
    check.setChecked(true);
    check.setToolTip("bound");
    check.RemoveConnection();
    QVERIFY(!check.isChecked());
    QCOMPARE(check.text(), QString("CheckBox"));
    QVERIFY(check.toolTip().isEmpty());

    QComboBoxD combo;
    combo.addItem("entry");
    combo.setToolTip("bound");
    combo.RemoveConnection();
    QCOMPARE(combo.count(), 0);
    QVERIFY(combo.toolTip().isEmpty());

    QLineEditD line;
    line.setText("entry");
    line.setToolTip("bound");
    line.RemoveConnection();
    QVERIFY(line.text().isEmpty());
    QVERIFY(line.toolTip().isEmpty());

    QSliderD slider;
    slider.setValue(37);
    slider.setToolTip("bound");
    slider.RemoveConnection();
    QCOMPARE(slider.value(), 0);
    QVERIFY(slider.toolTip().isEmpty());
}

void DropWidgetAdapterTests::DW_024_missing_mainwindow_context_is_safe_noop()
{
    const QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget* widget : topLevelWidgets)
        if (qobject_cast<MainWindow*>(widget))
            delete widget;

    QCOMPARE(GetMainWindow(), nullptr);
    QVERIFY(!DropWidgetBinding::CurrentManager());

    QObject foreignSource;
    QCOMPARE(CreateID(&foreignSource), QString());
    QVERIFY(CreateIDs(&foreignSource).isEmpty());

    QLineEditD lineEdit;
    lineEdit.setText("unchanged");
    lineEdit.RemoveConnection();
    QCOMPARE(lineEdit.text(), QString());

    QPushButtonD button;
    QSignalSpy released(&button, &QPushButton::released);
    button.TimeOut();
    QCOMPARE(released.count(), 0);
}

void DropWidgetAdapterTests::DW_002_numeric_set_get_and_signal_suppression()
{
    QSpinBoxD spin; spin.setRange(-5, 5); QSignalSpy spinSpy(&spin, &QSpinBox::valueChanged);
    ToFormMapper signedValue = mapper("int32_t"); signedValue.SetData(int32_t(8)); spin.SetVariantData(signedValue);
    QCOMPARE(spin.value(), 5); QCOMPARE(spinSpy.count(), 0);
    ToFormMapper signedOut = mapper("int32_t"); signedOut.SetData(int32_t(0)); spin.GetVariantData(&signedOut); QCOMPARE(signedOut.GetInt(), 5);
    spin.setValue(4); QCOMPARE(spinSpy.count(), 1); QCOMPARE(spinSpy.at(0).at(0).toInt(), 4); spin.setValue(4); QCOMPARE(spinSpy.count(), 1);

    QDoubleSpinBoxD decimal; decimal.setRange(-2.0, 2.0); decimal.setDecimals(1); QSignalSpy decimalSpy(&decimal, &QDoubleSpinBox::valueChanged);
    ToFormMapper doubleValue = mapper("double"); doubleValue.SetData(1.26); decimal.SetVariantData(doubleValue);
    QCOMPARE(decimal.value(), 1.3); QCOMPARE(decimalSpy.count(), 0);
    ToFormMapper doubleOut = mapper("double"); doubleOut.SetData(0.0); decimal.GetVariantData(&doubleOut); QCOMPARE(doubleOut.GetDouble(), 1.3);

    QProgressBarD progress; progress.setRange(0, 10); ToFormMapper floatValue = mapper("double"); floatValue.SetData(11.9); progress.SetVariantData(floatValue); QCOMPARE(progress.value(), -1);
    QLCDNumberD lcd; lcd.SetVariantData(doubleValue); QVERIFY(lcd.value() > 1.2 && lcd.value() < 1.3);
}

void DropWidgetAdapterTests::DW_003_string_selection_and_list_adapters()
{
    QLineEditD line; QSignalSpy textSpy(&line, &QLineEdit::textChanged);
    ToFormMapper text("", "Parameter"); text.SetData(QString::fromUtf8("Grüße")); line.SetVariantData(text);
    QCOMPARE(line.text(), QString::fromUtf8("Grüße")); QCOMPARE(textSpy.count(), 0);
    ToFormMapper textOut("", "Parameter"); textOut.SetData(QString()); line.GetVariantData(&textOut); QCOMPARE(textOut.GetString(), line.text());

    QComboBoxD combo; ToFormMapper selection = mapper("GuiSelection"); selection.SetData(GuiSelection("two", QStringList{"one", "two"})); combo.SetVariantData(selection);
    QCOMPARE(combo.count(), 2); QCOMPARE(combo.currentText(), QString("two")); combo.setCurrentIndex(0); combo.GetVariantData(&selection); QCOMPARE(selection.GetGuiSelection().first, QString("one"));

    QListViewD list; ToFormMapper listValue = mapper("QStringList"); listValue.SetData(QStringList{"eins", "zwei"}); list.SetVariantData(listValue);
    QCOMPARE(list.model->rowCount(), 2); ToFormMapper listOut = mapper("QStringList"); listOut.SetData(QStringList{"seed"}); list.GetVariantData(&listOut); QCOMPARE(listOut.GetStringList(), QStringList({"eins", "zwei"}));
}

void DropWidgetAdapterTests::DW_004_bit_and_xml_adapter_contracts()
{
    QLed led; QBLed blink; QTSLed traffic; QCheckBoxD check;
    std::vector<std::pair<QString, QString>> attributes {{"Bit", "3"}}; QString text;
    QVERIFY(led.LoadFromXML(attributes, text)); QVERIFY(blink.LoadFromXML(attributes, text)); QVERIFY(traffic.LoadFromXML(attributes, text)); QVERIFY(check.LoadFromXML(attributes, text));
    QCOMPARE(led.GetBit(), uint32_t(3)); QCOMPARE(blink.GetBit(), uint32_t(3)); QCOMPARE(traffic.GetBit(), uint32_t(3)); QCOMPARE(check.GetBit(), uint32_t(3));
    attributes.clear(); QVERIFY(check.SaveToXML(attributes, text)); QCOMPARE(attributes.size(), size_t(1)); QCOMPARE(attributes.front().second, QString("3"));
    ToFormMapper bits = mapper("uint64_t"); bits.SetData(uint64_t(8)); check.setChecked(true); check.GetVariantData(&bits); QCOMPARE(bits.GetUnsignedData(), uint64_t(8)); check.setChecked(false); check.GetVariantData(&bits); QCOMPARE(bits.GetUnsignedData(), uint64_t(0));
}

void DropWidgetAdapterTests::DW_005_xml_stubs_and_widget_special_cases()
{
    QComboBoxD combo; QLineEditD line; QSpinBoxD spin; QProgressBarD progress; QListViewD list; QPushButtonD button;
    std::vector<std::pair<QString, QString>> attributes; QString text;
    QVERIFY(!combo.LoadFromXML(attributes, text)); QVERIFY(!line.SaveToXML(attributes, text)); QVERIFY(!spin.LoadFromXML(attributes, text)); QVERIFY(!progress.SaveToXML(attributes, text)); QVERIFY(!list.LoadFromXML(attributes, text)); QVERIFY(!button.SaveToXML(attributes, text));
    QLabelD label; static_cast<QLabel&>(label).setText("original"); label.setText("override"); QVERIFY(label.SaveToXML(attributes, text)); QCOMPARE(text, QString("override")); label.RemoveUserText(); QCOMPARE(label.text(), QString("original"));
    QTableWidgeD table; QVERIFY(table.SaveToXML(attributes, text)); QCOMPARE(table.rowCount(), 0);
}

void DropWidgetAdapterTests::DW_006_loader_maps_standard_widgets_and_properties()
{
    DropWidgetsUiLoader loader; QFile file; std::unique_ptr<QWidget> form(loadFixture("standard-widgets.ui", loader, file));
    QVERIFY2(form, qPrintable(loader.errorString())); QCOMPARE(form->objectName(), QString("fixtureForm"));
    auto* button = form->findChild<QPushButtonD*>("pushButton"); auto* line = form->findChild<QLineEditD*>("lineEdit"); auto* spin = form->findChild<QSpinBoxD*>("spinBox");
    QVERIFY(button); QVERIFY(line); QVERIFY(spin); QVERIFY(!button->isEnabled()); QCOMPARE(line->text(), QString("initial")); QVERIFY(line->isReadOnly()); QCOMPARE(spin->minimum(), -2); QCOMPARE(spin->maximum(), 7); QCOMPARE(spin->value(), 3);
    QVERIFY(button->acceptDrops()); QVERIFY(line->acceptDrops()); QVERIFY(spin->acceptDrops());
}

void DropWidgetAdapterTests::DW_007_loader_maps_dropwidgets_and_parent_hierarchy()
{
    DropWidgetsUiLoader loader; QFile file; std::unique_ptr<QWidget> form(loadFixture("dropwidgets-nested.ui", loader, file));
    QVERIFY2(form, qPrintable(loader.errorString())); auto* panel=form->findChild<QWidget*>("nestedPanel"); QVERIFY(panel);
    auto* combo=form->findChild<QComboBoxD*>("selection"); auto* led=form->findChild<QLed*>("statusLed"); auto* traffic=form->findChild<QTSLed*>("trafficLed"); auto* blink=form->findChild<QBLed*>("blinkLed"); auto* table=form->findChild<QTableWidgeD*>("dataTable");
    QVERIFY(combo); QVERIFY(led); QVERIFY(traffic); QVERIFY(blink); QVERIFY(table); QCOMPARE(combo->parentWidget(), panel); QCOMPARE(table->parentWidget(), form.get()); QVERIFY(combo->acceptDrops()); QVERIFY(table->acceptDrops());
}

void DropWidgetAdapterTests::DW_008_loader_rejects_missing_malformed_and_unknown_ui()
{
    DropWidgetsUiLoader loader; QFile missing(fixture("missing.ui")); QVERIFY(!missing.open(QIODevice::ReadOnly));
    QFile malformed; QVERIFY(!loadFixture("malformed.ui", loader, malformed)); QVERIFY(!loader.errorString().isEmpty());
    QFile unknown; std::unique_ptr<QWidget> partial(loadFixture("unsupported-widget.ui", loader, unknown)); QVERIFY(partial); QVERIFY(!partial->findChild<QWidget*>("unsupported"));
}

void DropWidgetAdapterTests::DW_009_tree_mime_and_invalid_drag_contract()
{
    ExposedTreeWidget tree; auto* root=new QTreeWidgetItem(&tree, QStringList("root")); auto* first=new QTreeWidgetItem(root, QStringList("first")); auto* second=new QTreeWidgetItem(root, QStringList("second"));
    first->setSelected(true); second->setSelected(true); std::unique_ptr<QMimeData> mime(tree.mimeData(tree.selectedItems()));
    QCOMPARE(tree.mimeTypes(), QStringList({"text/uri-list"})); QCOMPARE(mime->text(), QString("root::first\r\nroot::second")); QVERIFY(!mime->hasFormat("text/uri-list"));
    QLineEditD target; QMimeData foreign; foreign.setText("root::first"); QDragEnterEvent event(QPoint(1,1), Qt::CopyAction, &foreign, Qt::LeftButton, Qt::NoModifier); target.dragEnterEvent(&event); QVERIFY(!event.isAccepted());
}

void DropWidgetAdapterTests::DW_010_connect_and_table_row_actions()
{
    QComboBoxD combo; QSignalSpy updateSpy(&combo, &QComboBoxD::RequestUpdate); combo.ConnectToID(GetMainWindow()->GetLogic(), "root::selection"); combo.ConnectToID(GetMainWindow()->GetLogic(), "root::selection"); QCOMPARE(combo.toolTip(), QString("root::selection")); QCOMPARE(updateSpy.count(), 2);
    QTableWidgeD table; table.setRowCount(1); table.setColumnCount(1); table.setVerticalHeaderItem(0, new QTableWidgetItem("root::value")); table.setItem(0,0,new QTableWidgetItem("42")); std::vector<std::pair<QString,QString>> attributes; QString text; QVERIFY(table.SaveToXML(attributes,text)); QCOMPARE(attributes.size(), size_t(1)); QCOMPARE(attributes.front().second, QString("root::value")); table.selectRow(0); table.RemoveSelectedRows(); QCOMPARE(table.rowCount(), 0);
}

void DropWidgetAdapterTests::DW_011_manager_to_widget_and_widget_to_message()
{
    auto* manager = GetMainWindow()->GetLogic(); const QString id("DW11::text");
    manager->AddContainerElement(id, "QString", "Parameter", "");
    QLineEditD line; line.setObjectName("dw11Line"); manager->AddElementToContainerEntry(line.objectName(), id, line.metaObject()->className(), &line);
    MessengerClass messenger(manager); QObject::connect(manager, &DataManagementClass::MessageSender, &messenger, &MessengerClass::MessageReceiver);
    InterfaceData incoming("", "Parameter"); incoming.SetData(QString("from-manager")); messenger.MessageReceiver("set", id, incoming);
    QCOMPARE(line.text(), QString("from-manager")); QCOMPARE(manager->GetContainer(id)->GetString(), QString("from-manager"));
    QSignalSpy sent(manager, &DataManagementClass::MessageSender); line.ConnectToID(manager, id); QCOMPARE(line.toolTip(), id); QCOMPARE(sent.count(), 1); QCOMPARE(sent.at(0).at(0).toString(), QString("get")); QCOMPARE(sent.at(0).at(1).toString(), id);
    line.setText("from-widget"); emit line.editingFinished(); QCOMPARE(sent.count(), 2); QCOMPARE(sent.at(1).at(0).toString(), QString("set")); QCOMPARE(sent.at(1).at(1).toString(), id); QCOMPARE(sent.at(1).at(2).value<InterfaceData>().GetString(), QString("from-widget"));
}

void DropWidgetAdapterTests::DW_012_duplicate_and_removed_connections()
{
    auto* manager = GetMainWindow()->GetLogic(); const QString id("DW12::value"); manager->AddContainerElement(id, "QString", "Parameter", "");
    QLineEditD line; line.setObjectName("dw12Line"); manager->AddElementToContainerEntry(line.objectName(), id, line.metaObject()->className(), &line);
    QSignalSpy sent(manager, &DataManagementClass::MessageSender); QObject::connect(&line, &QLineEdit::editingFinished, manager, &DataManagementSetClass::SendNewValue); QObject::connect(&line, &QLineEdit::editingFinished, manager, &DataManagementSetClass::SendNewValue);
    line.setText("twice"); emit line.editingFinished(); QCOMPARE(sent.count(), 2); QCOMPARE(sent.at(0).at(1).toString(), id); QCOMPARE(sent.at(1).at(1).toString(), id);
    manager->DeleteEntryOfObject(&line); line.setText("detached"); emit line.editingFinished(); QCOMPARE(sent.count(), 2); QVERIFY(manager->GetContainerID(&line).isEmpty());
}

void DropWidgetAdapterTests::DW_013_table_createrow_bindings_and_context_action()
{
    auto* manager = GetMainWindow()->GetLogic();
    manager->AddContainerElement("DW13::0", "int32_t", "Parameter", ""); manager->GetContainer("DW13::0")->SetData(int32_t(0));
    manager->AddContainerElement("DW13::1", "double", "Parameter", ""); manager->GetContainer("DW13::1")->SetData(0.0);
    QTableWidgeD table; table.setObjectName("dw13Table"); std::vector<std::pair<QString,QString>> attributes {{"Connected_ID0", "DW13"}, {"Connected_ID1", "DW13"}}; QString text;
    QVERIFY(table.LoadFromXML(attributes, text)); QTRY_COMPARE_WITH_TIMEOUT(table.rowCount(), 2, 3000); QCOMPARE(table.columnCount(), 2);
    QWidget* firstCell = table.cellWidget(0, 0); QVERIFY(firstCell); auto* first = firstCell->findChild<QLineEditD*>(); QVERIFY(first); InterfaceData update("", "Parameter"); update.SetData(int32_t(42)); manager->DataManagementClass::SetData("DW13::0", update); manager->SetData("DW13::0"); QCOMPARE(first->text(), QString("42"));
    table.contextMenu(QPoint(1,1)); QAction* clear = nullptr; for (QAction* action : table.findChildren<QAction*>()) if (action->text() == "Clear Table") clear = action; QVERIFY(clear); clear->trigger(); QCOMPARE(table.rowCount(), 0); QCOMPARE(table.columnCount(), 0);
}

void DropWidgetAdapterTests::DW_014_indicator_colors_state_and_offscreen_render()
{
    QLedIndicator led(nullptr); QBLedIndicator blink(nullptr); QTSLedIndicator traffic(nullptr);
    led.setOnColor(Qt::blue); led.setOffColor(Qt::darkBlue); QCOMPARE(led.getOnColor(), QColor(Qt::blue)); QCOMPARE(led.getOffColor(), QColor(Qt::darkBlue));
    blink.setOnColor(Qt::cyan); blink.setOffColor(Qt::darkCyan); QCOMPARE(blink.getOnColor(), QColor(Qt::cyan)); QCOMPARE(blink.getOffColor(), QColor(Qt::darkCyan));
    traffic.setOnColor1(Qt::green); traffic.setOnColor2(Qt::darkGreen); traffic.setOffColor1(Qt::yellow); traffic.setOffColor2(Qt::darkYellow);
    QCOMPARE(traffic.getOnColor1(), QColor(Qt::green)); QCOMPARE(traffic.getOnColor2(), QColor(Qt::darkGreen)); QCOMPARE(traffic.getOffColor1(), QColor(Qt::yellow)); QCOMPARE(traffic.getOffColor2(), QColor(Qt::darkYellow));
    traffic.SetColor("Red"); QCOMPARE(traffic.GetColor(), QString("Red")); traffic.SetColor("Green"); QCOMPARE(traffic.GetColor(), QString("Green")); traffic.SetColor("Yellow"); QCOMPARE(traffic.GetColor(), QString("Green")); traffic.YellowColor(); QCOMPARE(traffic.GetColor(), QString()); traffic.SetColor("unsupported"); QCOMPARE(traffic.GetColor(), QString());
    const QList<QAbstractButton*> indicators {&led, &blink, &traffic};
    for (QAbstractButton* indicator : indicators) {
        indicator->resize(32, 24); indicator->setEnabled(false); QVERIFY(!indicator->isEnabled()); indicator->setEnabled(true); QVERIFY(indicator->isEnabled());
        QImage image(indicator->size(), QImage::Format_ARGB32_Premultiplied); image.fill(Qt::transparent); indicator->render(&image); QVERIFY(!image.isNull());
    }
    QImage stateImage(32, 24, QImage::Format_ARGB32_Premultiplied);
    led.SetState(true); led.render(&stateImage); led.SetState(false); led.render(&stateImage);
    blink.SetState(true); blink.TimeOut(); blink.render(&stateImage); blink.TimeOut(); blink.render(&stateImage); blink.SetState(false); blink.TimeOut(); blink.render(&stateImage);
    for (int state : {-1, 0, 1, 2}) { traffic.SetState(state); traffic.render(&stateImage); }
}

void DropWidgetAdapterTests::DW_015_blink_timer_repeats_and_is_owned()
{
    auto* blink = new QBLedIndicator(nullptr); QPointer<QTimer> timer = blink->findChild<QTimer*>(); QVERIFY(timer); QCOMPARE(timer->interval(), 500); QVERIFY(timer->isActive());
    QSignalSpy timeoutSpy(timer, &QTimer::timeout); QTRY_VERIFY_WITH_TIMEOUT(timeoutSpy.count() >= 2, 1300);
    blink->SetState(true); blink->TimeOut(); blink->SetState(false); blink->TimeOut();
    delete blink; QVERIFY(timer.isNull());
}

void DropWidgetAdapterTests::DW_016_data_driven_adapter_signals_ranges_and_values()
{
    QPushButtonD button; QSignalSpy pressed(&button, &QPushButton::pressed); QSignalSpy released(&button, &QPushButton::released); QSignalSpy clicked(&button, &QPushButton::clicked); button.click(); QCOMPARE(pressed.count(), 1); QCOMPARE(released.count(), 1); QCOMPARE(clicked.count(), 1); QCOMPARE(clicked.at(0).at(0).toBool(), false);
    ToFormMapper buttonOut = mapper("bool"); buttonOut.SetData(false); button.setDown(true); button.GetVariantData(&buttonOut); QVERIFY(buttonOut.GetBool());

    QSliderD slider; slider.setRange(0, 100); QSignalSpy sliderSpy(&slider, &QSlider::valueChanged); slider.setValue(-1); QCOMPARE(slider.value(), 0); slider.setValue(55); slider.setValue(55); QCOMPARE(sliderSpy.count(), 1); QCOMPARE(sliderSpy.at(0).at(0).toInt(), 55);
    ToFormMapper sliderOut = mapper("double"); sliderOut.SetData(0.0); sliderOut.MinValue = 10.0; sliderOut.MaxValue = 20.0; slider.setValue(50); slider.GetVariantData(&sliderOut); QCOMPARE(sliderOut.GetDouble(), 15.0);

    auto* manager = GetMainWindow()->GetLogic();
    const QString sliderId("DW16::numeric-range");
    manager->AddContainerElement(sliderId, "double", "Parameter", "");
    manager->SetMinMaxValue(sliderId, 0.0, 100.0);
    QSliderD connectedSlider;
    connectedSlider.setRange(0, 100);
    connectedSlider.ConnectToID(manager, sliderId);
    connectedSlider.setValue(37);
    QSignalSpy connectedSliderSpy(&connectedSlider, &QSlider::valueChanged);
    ToFormMapper stringValue = mapper("QString"); stringValue.SetData(QString("not-a-number"));
    ToFormMapper listValue = mapper("QStringList"); listValue.SetData(QStringList({"not", "numeric"}));
    ToFormMapper selectionValue = mapper("GuiSelection"); selectionValue.SetData(GuiSelection("not-numeric", QStringList({"not-numeric"})));
    const QList<ToFormMapper> nonnumericValues {stringValue, listValue, selectionValue};
    for (ToFormMapper data : nonnumericValues) {
        QVERIFY(data.IsEditable()); QVERIFY(!data.IsFloatingPointNumber()); QVERIFY(!data.IsSigedNumber()); QVERIFY(!data.IsUnsigedNumber());
        connectedSlider.SetVariantData(data);
        QCOMPARE(connectedSlider.value(), 37); QCOMPARE(connectedSliderSpy.count(), 0); QVERIFY(!connectedSlider.signalsBlocked());
    }
    connectedSlider.setValue(38); QCOMPARE(connectedSliderSpy.count(), 1); QCOMPARE(connectedSliderSpy.at(0).at(0).toInt(), 38);

    QSpinBoxD spin; spin.setRange(-2, 2); QSignalSpy spinSpy(&spin, &QSpinBox::valueChanged); spin.setValue(9); spin.setValue(2); QCOMPARE(spin.value(), 2); QCOMPARE(spinSpy.count(), 1); QCOMPARE(spinSpy.at(0).at(0).toInt(), 2);
    QCheckBoxD check; QSignalSpy checkSpy(&check, &QCheckBox::clicked); check.click(); QCOMPARE(checkSpy.count(), 1); QCOMPARE(checkSpy.at(0).at(0).toBool(), true); check.click(); QCOMPARE(checkSpy.count(), 2); QCOMPARE(checkSpy.at(1).at(0).toBool(), false);
    QComboBoxD combo; combo.addItems({"one", "two"}); QSignalSpy comboSpy(&combo, qOverload<int>(&QComboBox::currentIndexChanged)); combo.setCurrentIndex(1); combo.setCurrentIndex(1); QCOMPARE(comboSpy.count(), 1); QCOMPARE(comboSpy.at(0).at(0).toInt(), 1);
    QLineEditD line; QSignalSpy editSpy(&line, &QLineEdit::editingFinished); line.setText("same"); emit line.editingFinished(); QCOMPARE(editSpy.count(), 1); QCOMPARE(line.text(), QString("same"));
}

void DropWidgetAdapterTests::DW_017_isolated_xml_properties_and_list_mutation()
{
    QLed led; QBLed blink; QTSLed traffic; std::vector<std::pair<QString, QString>> attributes {{"Bit", "2"}, {"Ignored", "x"}}; QString text;
    QVERIFY(led.LoadFromXML(attributes, text)); QVERIFY(blink.LoadFromXML(attributes, text)); QVERIFY(traffic.LoadFromXML(attributes, text)); QCOMPARE(led.GetBit(), uint32_t(2)); QCOMPARE(blink.GetBit(), uint32_t(2)); QCOMPARE(traffic.GetBit(), uint32_t(2));
    for (VariantDropWidget* widget : QList<VariantDropWidget*> {&led, &blink, &traffic}) { std::vector<std::pair<QString, QString>> saved; QVERIFY(widget->SaveToXML(saved, text)); QCOMPARE(saved.size(), size_t(1)); QCOMPARE(saved.front().first, QString("Bit")); QCOMPARE(saved.front().second, QString("2")); }
    QListViewD list; list.model->setStringList({"a", "b"}); QSignalSpy newEntry(&list, &QListViewD::NewEntry); list.setCurrentIndex(list.model->index(0, 0)); list.DeleteEntry(); QCOMPARE(newEntry.count(), 1); QCOMPARE(list.model->stringList(), QStringList({"b"})); list.DeleteAllEntries(); QCOMPARE(newEntry.count(), 2); QCOMPARE(list.model->rowCount(), 0);
    QWidget parent; QLineEditD* line = new QLineEditD(&parent); line->setReadOnly(true); QVERIFY(line->isReadOnly()); QCOMPARE(line->parentWidget(), &parent); delete line;
}

void DropWidgetAdapterTests::DW_018_equal_slider_bounds_preserve_value()
{
    auto* manager = GetMainWindow()->GetLogic();
    const QString sliderId("DW18::equal-range");
    manager->AddContainerElement(sliderId, "double", "Parameter", "");
    manager->SetMinMaxValue(sliderId, 5.0, 5.0);
    manager->GetContainer(sliderId)->SetData(5.0);

    QSliderD slider;
    slider.setRange(0, 100);
    slider.setValue(37);
    slider.setObjectName("dw18Slider");
    manager->AddElementToContainerEntry(slider.objectName(), sliderId, slider.metaObject()->className(), &slider);
    slider.ConnectToID(manager, sliderId);
    QSignalSpy valueChanged(&slider, &QSlider::valueChanged);
    QSignalSpy messages(manager, &DataManagementClass::MessageSender);

    ToFormMapper floating = mapper("double");
    floating.SetData(7.5);
    ToFormMapper signedValue = mapper("int32_t");
    signedValue.SetData(int32_t(-4));
    ToFormMapper unsignedValue = mapper("uint64_t");
    unsignedValue.SetData(uint64_t(42));
    const QList<ToFormMapper> numericValues {floating, signedValue, unsignedValue};
    for (ToFormMapper data : numericValues) {
        QVERIFY(data.IsEditable());
        QVERIFY(data.IsFloatingPointNumber() || data.IsSigedNumber() || data.IsUnsigedNumber());
        slider.SetVariantData(data);
        QCOMPARE(slider.value(), 37);
        QCOMPARE(valueChanged.count(), 0);
        QCOMPARE(messages.count(), 0);
        QVERIFY(!slider.signalsBlocked());
    }

    slider.setValue(38);
    QCOMPARE(valueChanged.count(), 1);
    QCOMPARE(valueChanged.at(0).at(0).toInt(), 38);
    QCOMPARE(messages.count(), 1);
    QCOMPARE(messages.at(0).at(0).toString(), QString("set"));
    QCOMPARE(messages.at(0).at(1).toString(), sliderId);

    manager->SetMinMaxValue(sliderId, 0.0, 100.0);
    valueChanged.clear();
    messages.clear();
    slider.setValue(37);
    valueChanged.clear();
    messages.clear();
    slider.SetVariantData(floating);
    QCOMPARE(slider.value(), 8);
    QCOMPARE(valueChanged.count(), 0);
    QCOMPARE(messages.count(), 0);
}

int main(int argc, char** argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    if (GetMainWindow())
        return 2;
    DropWidgetAdapterTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "DropWidgetAdapterTests.moc"
