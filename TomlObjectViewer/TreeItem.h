// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>

class TreeItem
{
public:
        enum class ItemType
        {
                ObjectProperty,
                ObjectParameter,
                ObjectParameterEditable
        };
        explicit TreeItem(ItemType t_type, QVariantList t_data, TreeItem *parentItem = nullptr);

        TreeItem *appendChild(std::unique_ptr<TreeItem> &&child);

        TreeItem      *child(int row);
        int            childCount() const;
        int            columnCount() const;
        QVariant       data(int column) const;
        bool           setData(int column, const QVariant &value);
        int            row() const;
        TreeItem      *parentItem();
        const ItemType getType() const;
        void           setParamPossibleValues(const QStringList values);
        QStringList    getParamPossibleValues() const;
        void           setParamValue(const QString &value);
        QString        getParamValue() const;
        void           setParamDefaultValue(const QString &value);
        QString        getParamDefaultValue() const;
        QString        getItemData() const;

private:
        ItemType     m_type;
        QVariantList m_itemData;
        // Поля для работы со значениями параметров
        QStringList  m_paramPossibleValues;
        QString      m_paramValue;
        QString      m_paramDefaultValue;

        std::vector<std::unique_ptr<TreeItem>> m_childItems;
        TreeItem                              *m_parentItem;
};

#endif    // TREEITEM_H
