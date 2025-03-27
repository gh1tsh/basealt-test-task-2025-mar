// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <toml++/toml.h>

class TreeItem;

class TreeModel : public QAbstractItemModel
{
        Q_OBJECT

public:
        Q_DISABLE_COPY_MOVE(TreeModel)

        explicit TreeModel(QObject *parent = nullptr);
        ~TreeModel() override;

        QVariant      data(const QModelIndex &index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant      headerData(int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole) const override;
        QModelIndex   index(int row, int column, const QModelIndex &parent = {}) const override;
        QModelIndex   parent(const QModelIndex &index) const override;
        int           rowCount(const QModelIndex &parent = {}) const override;
        int           columnCount(const QModelIndex &parent = {}) const override;
        void          clear();
        void          checkToml(const toml::table &parsedToml);
        void          reset(const QString &);
        bool          setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
        void setupModelData(TreeItem *parent);

        std::unique_ptr<TreeItem> rootItem;

        QString     m_tomlFilePath;
        QString     m_systemLanguage;
        toml::table m_toml;
};

#endif    // TREEMODEL_H
