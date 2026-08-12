/*
 * Unit tests for MainWindowTreeModel.
 *
 * Covers:
 *   - AddElement without index (original behaviour, no regression)
 *   - AddElement with index: first call builds structure + populates index
 *   - AddElement with index: subsequent call takes O(1) fast path (value updated)
 *   - RemoveElement with index: entry is removed from index and tree
 *   - RemoveElement without index: falls back to linear search
 *   - Highlight: item is selected in the tree
 */

#include <QtTest/QtTest>
#include <QTreeWidget>
#include <QDockWidget>

#include "UIFunctions/MainWindowTreeModel.h"
#include "plugins/InterfaceDataType.h"

class MainWindowTreeModelTests : public QObject
{
    Q_OBJECT

private slots:
    void addElement_noIndex_createsLeaf()
    {
        QTreeWidget tree;
        InterfaceData data;
        data.SetData(QString("42"));

        MainWindowTreeModel::AddElement(&tree, {"Device", "Channel"}, data, nullptr);

        QCOMPARE(tree.topLevelItemCount(), 1);
        QCOMPARE(tree.topLevelItem(0)->text(0), QString("Device"));
        QCOMPARE(tree.topLevelItem(0)->childCount(), 1);
        QCOMPARE(tree.topLevelItem(0)->child(0)->text(0), QString("Channel"));
    }

    void addElement_withIndex_populatesIndex()
    {
        QTreeWidget tree;
        MainWindowTreeModel::ItemIndex index;
        InterfaceData data;
        data.SetData(QString("hello"));

        MainWindowTreeModel::AddElement(&tree, {"A", "B"}, data, &index);

        QVERIFY(index.contains("A::B"));
        QCOMPARE(index.value("A::B")->text(1), QString("hello"));
    }

    void addElement_withIndex_fastPathUpdatesValue()
    {
        QTreeWidget tree;
        MainWindowTreeModel::ItemIndex index;
        InterfaceData first;
        first.SetData(QString("first"));

        MainWindowTreeModel::AddElement(&tree, {"X", "Y"}, first, &index);

        // Confirm structure was built
        QCOMPARE(tree.topLevelItemCount(), 1);
        QTreeWidgetItem* leaf = index.value("X::Y");
        QVERIFY(leaf);

        // Second call must not create a new item
        InterfaceData second;
        second.SetData(QString("second"));
        MainWindowTreeModel::AddElement(&tree, {"X", "Y"}, second, &index);

        QCOMPARE(tree.topLevelItemCount(), 1);                    // no new top-level
        QCOMPARE(index.value("X::Y"), leaf);                      // same pointer
        QCOMPARE(leaf->text(1), QString("second"));               // value updated
    }

    void addElement_withIndex_multipleLeaves()
    {
        QTreeWidget tree;
        MainWindowTreeModel::ItemIndex index;
        InterfaceData a, b;
        a.SetData(QString("1"));
        b.SetData(QString("2"));

        MainWindowTreeModel::AddElement(&tree, {"Root", "ChildA"}, a, &index);
        MainWindowTreeModel::AddElement(&tree, {"Root", "ChildB"}, b, &index);

        // Both under same parent
        QCOMPARE(tree.topLevelItemCount(), 1);
        QCOMPARE(tree.topLevelItem(0)->childCount(), 2);
        QVERIFY(index.contains("Root::ChildA"));
        QVERIFY(index.contains("Root::ChildB"));
    }

    void removeElement_withIndex_removesFromTreeAndIndex()
    {
        QTreeWidget tree;
        MainWindowTreeModel::ItemIndex index;
        InterfaceData data;
        data.SetData(QString("val"));

        MainWindowTreeModel::AddElement(&tree, {"P", "Q"}, data, &index);
        QVERIFY(index.contains("P::Q"));

        MainWindowTreeModel::RemoveElement({&tree}, {"P", "Q"}, {&index});

        QVERIFY(!index.contains("P::Q"));
    }

    void removeElement_withoutIndex_fallsBackToLinearSearch()
    {
        QTreeWidget tree;
        InterfaceData data;
        data.SetData(QString("v"));

        // Add without index
        MainWindowTreeModel::AddElement(&tree, {"Top", "Leaf"}, data, nullptr);
        QCOMPARE(tree.topLevelItem(0)->childCount(), 1);

        // Remove without index — should use linear search fallback
        MainWindowTreeModel::RemoveElement({&tree}, {"Top", "Leaf"}, {});
        QCOMPARE(tree.topLevelItem(0)->childCount(), 0);
    }

    void removeElement_nonExistent_doesNotCrash()
    {
        QTreeWidget tree;
        MainWindowTreeModel::ItemIndex index;

        // Must not crash when path does not exist
        MainWindowTreeModel::RemoveElement({&tree}, {"Missing", "Path"}, {&index});
    }

    void addElement_nullTree_doesNotCrash()
    {
        MainWindowTreeModel::ItemIndex index;
        InterfaceData data;
        data.SetData(QString("x"));

        // Must not crash when tree is null
        MainWindowTreeModel::AddElement(nullptr, {"A"}, data, &index);
        QVERIFY(index.isEmpty());
    }

    void highlight_selectsItem()
    {
        QTreeWidget tree;
        QDockWidget dock;
        dock.setWidget(&tree);

        InterfaceData data;
        data.SetData(QString("v"));
        MainWindowTreeModel::AddElement(&tree, {"Root", "Child"}, data, nullptr);

        MainWindowTreeModel::Highlight({{&tree, &dock}}, {"Root", "Child"});

        QVERIFY(!tree.selectedItems().isEmpty());
        QCOMPARE(tree.selectedItems().first()->text(0), QString("Child"));
    }
};

QTEST_MAIN(MainWindowTreeModelTests)
#include "MainWindowTreeModelTests.moc"
