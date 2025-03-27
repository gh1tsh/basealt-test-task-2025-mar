// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include "TreeItem.h"

TreeItem::TreeItem(ItemType t_type, QVariantList t_data, TreeItem *t_parent) :
    m_type(t_type), m_itemData(std::move(t_data)), m_paramPossibleValues(), m_paramValue(),
    m_childItems(), m_parentItem(t_parent)
{}

TreeItem *
TreeItem::appendChild(std::unique_ptr<TreeItem> &&child)
{
        m_childItems.push_back(std::move(child));
        return m_childItems.back().get();
}

TreeItem *
TreeItem::child(int row)
{
        return row >= 0 && row < childCount() ? m_childItems.at(row).get() : nullptr;
}

int
TreeItem::childCount() const
{
        return int(m_childItems.size());
}

int
TreeItem::columnCount() const
{
        return int(m_itemData.count());
}

QVariant
TreeItem::data(int column) const
{
        return m_itemData.value(column);
}

TreeItem *
TreeItem::parentItem()
{
        return m_parentItem;
}

const TreeItem::ItemType
TreeItem::getType() const
{
        return m_type;
}

void
TreeItem::setParamPossibleValues(const QStringList values)
{
        m_paramPossibleValues = values;
}

QStringList
TreeItem::getParamPossibleValues() const
{
        return m_paramPossibleValues;
}

void
TreeItem::setParamValue(const QString &value)
{
        if (m_type == TreeItem::ItemType::ObjectParameterEditable) {
                m_paramValue = value;
        }
}

QString
TreeItem::getParamValue() const
{
        if (m_type == TreeItem::ItemType::ObjectParameterEditable) {
                return m_paramValue;
        } else {
                return QString();
        }
}

void
TreeItem::setParamDefaultValue(const QString &value)
{
        if (m_type == TreeItem::ItemType::ObjectParameterEditable) {
                m_paramDefaultValue = value;
        }
}

QString
TreeItem::getParamDefaultValue() const
{
        if (m_type == TreeItem::ItemType::ObjectParameterEditable) {
                return m_paramDefaultValue;
        } else {
                return QString();
        }
}

QString
TreeItem::getItemData() const
{
        QStringList stringList;
        for (const auto &var : m_itemData) {
                stringList << var.toString();
        }
        return stringList.join(", ");
}

int
TreeItem::row() const
{
        if (m_parentItem == nullptr)
                return 0;
        const auto it = std::find_if(
            m_parentItem->m_childItems.cbegin(),
            m_parentItem->m_childItems.cend(),
            [this](const std::unique_ptr<TreeItem> &treeItem) { return treeItem.get() == this; });

        if (it != m_parentItem->m_childItems.cend())
                return std::distance(m_parentItem->m_childItems.cbegin(), it);
        Q_ASSERT(false);    // should not happen
        return -1;
}

bool
TreeItem::setData(int column, const QVariant &value)
{
        if (column >= 0 && column < m_itemData.size()) {
                m_itemData[column] = value;
                return true;
        }
        return false;
}

