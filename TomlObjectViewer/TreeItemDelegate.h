#ifndef TREEITEMDELEGATE_H
#define TREEITEMDELEGATE_H

#include <QComboBox>
#include <QStyledItemDelegate>

class TreeItemDelegate : public QStyledItemDelegate
{
        Q_OBJECT

public:
        explicit TreeItemDelegate(QObject *parent = nullptr);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

        void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model,
                          const QModelIndex &index) const override;
};

#endif    // TREEITEMDELEGATE_H
