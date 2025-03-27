#include "TreeItemDelegate.h"
#include "TreeItem.h"

#include <QComboBox>

TreeItemDelegate::TreeItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QWidget *
TreeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
        const auto *item = static_cast<const TreeItem *>(index.internalPointer());

        if (index.column() == 2 && item->getType() == TreeItem::ItemType::ObjectParameterEditable) {
                auto *comboBox = new QComboBox(parent);

                QStringList possibleValues = item->getParamPossibleValues();
                for (const auto &val : possibleValues) {
                        comboBox->addItem(val);
                }

                return comboBox;
        }
        return TreeItemDelegate::createEditor(parent, option, index);
}

void
TreeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
        auto *comboBox = qobject_cast<QComboBox *>(editor);
        if (!comboBox)
                return;

        QString currentValue = index.model()->data(index, Qt::EditRole).toString();
        comboBox->setCurrentText(currentValue);
}

void
TreeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const
{
        auto *comboBox = qobject_cast<QComboBox *>(editor);
        if (!comboBox)
                return;

        model->setData(index, comboBox->currentText(), Qt::EditRole);
}
