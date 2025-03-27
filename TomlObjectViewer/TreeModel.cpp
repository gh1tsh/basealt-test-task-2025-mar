#include "TreeModel.h"
#include "TreeItem.h"

#include <QComboBox>
#include <QFile>
#include <QLocale>
#include <QStringList>

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string_view>

using namespace Qt::StringLiterals;

TreeModel::TreeModel(QObject *parent) :
    QAbstractItemModel(parent),
    rootItem(std::make_unique<TreeItem>(TreeItem::ItemType::ObjectProperty, QVariantList())),
    m_tomlFilePath(), m_toml()
{
        m_systemLanguage = QLocale::system().name().split("_").first();
}

TreeModel::~TreeModel() = default;

int
TreeModel::columnCount(const QModelIndex &parent) const
{
        if (parent.isValid())
                return static_cast<TreeItem *>(parent.internalPointer())->columnCount();
        return rootItem->columnCount();
}

void
TreeModel::clear()
{
        beginResetModel();

        rootItem.reset();
        rootItem = std::make_unique<TreeItem>(TreeItem::ItemType::ObjectProperty, QVariantList());

        endResetModel();
}

void
TreeModel::checkToml(const toml::table &parsed)
{
        // Проверка таблицы [properties]
        if (!parsed.contains("properties")) {
                throw std::runtime_error("Таблица [properties] отсутствует.");
        }

        const auto &properties = parsed["properties"].as_table();

        // Проверка обязательных полей в [properties]
        if (!properties->contains("id") || !properties->get("id")->is<std::string>()) {
                throw std::runtime_error("Отсутствует обязательное поле 'id' в [properties].");
        }
        if (!properties->contains("type") || !properties->get("type")->is<std::string>()) {
                throw std::runtime_error("Отсутствует обязательное поле 'type' в [properties].");
        }
        if (!properties->contains("name") || !properties->get("name")->is<toml::table>() ||
            !properties->get("name")->as_table()->contains("default")) {
                throw std::runtime_error(
                    "Отсутствует обязательное поле 'default' в [properties.name].");
        }

        // Проверка на наличие таблицы [properties.name]
        const auto &propertiesName = properties->get("name")->as_table();
        if (!propertiesName->contains("default") ||
            !propertiesName->get("default")->is<std::string>()) {
                throw std::runtime_error(
                    "Отсутствует обязательное поле 'default' в [properties.name].");
        }

        // Проверка на существование параметров
        if (!parsed.contains("parameters") || !parsed["parameters"].is<toml::array>()) {
                throw std::runtime_error("Отсутствует обязательная таблица [parameters].");
        }

        const auto &parameters = parsed["parameters"].as_array();

        if (parameters->empty()) {
                throw std::runtime_error(
                    "В таблице [parameters] должен быть хотя бы один параметр.");
        }

        // Проверка каждого параметра
        for (const auto &param : *parameters) {
                const auto &paramTable = param.as_table();

                // Проверка обязательных полей для параметра
                if (!paramTable->contains("id") || !paramTable->get("id")->is<std::string>()) {
                        throw std::runtime_error("Отсутствует обязательное поле 'id' в параметре.");
                }
                if (!paramTable->contains("type") || !paramTable->get("type")->is<std::string>()) {
                        throw std::runtime_error(
                            "Отсутствует обязательное поле 'type' в параметре.");
                }
                if (!paramTable->contains("required") || !paramTable->get("required")->is<bool>()) {
                        throw std::runtime_error(
                            "Отсутствует обязательное поле 'required' в параметре.");
                }
                if (!paramTable->contains("default_value")) {
                        throw std::runtime_error(
                            "Отсутствует обязательное поле 'default_value' в параметре.");
                }
                if (!paramTable->contains("possible_values") ||
                    !paramTable->get("possible_values")->is<toml::array>()) {
                        throw std::runtime_error(
                            "Отсутствует обязательное поле 'possible_values' в параметре.");
                }
                if (!paramTable->contains("value")) {
                        throw std::runtime_error(
                            "Отсутствует обязательное поле 'value' в параметре.");
                }
        }
}

void
TreeModel::reset(const QString &t_tomlFilePath)
{
        beginResetModel();

        m_tomlFilePath = t_tomlFilePath;

        QFile f(m_tomlFilePath);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                throw std::runtime_error("Не удалось открыть файл '" +
                                         m_tomlFilePath.toStdString() + "'");
                return;
        }

        std::string fileContent = QString::fromUtf8(f.readAll()).toStdString();
        f.close();

        try {
                m_toml = toml::parse(fileContent);
        } catch (toml::parse_error &err) {
                throw toml::parse_error(err);
                return;
        }

        this->clear();

        rootItem.reset(
            new TreeItem(TreeItem::ItemType::ObjectProperty, QVariantList{ tr("Объект"), "", "" }));

        try {
                checkToml(m_toml);
        } catch (const std::runtime_error &e) {
                throw std::runtime_error(e);
        }

        try {
                setupModelData(rootItem.get());
        } catch (const std::runtime_error &e) {
                throw std::runtime_error(e);
        }

        endResetModel();
}

QVariant
TreeModel::data(const QModelIndex &index, int role) const
{
        if (!index.isValid() || role != Qt::DisplayRole)
                return {};

        const auto *item = static_cast<const TreeItem *>(index.internalPointer());
        return item->data(index.column());
}

Qt::ItemFlags
TreeModel::flags(const QModelIndex &index) const
{
        if (!index.isValid())
                return Qt::NoItemFlags;

        const auto *item = static_cast<const TreeItem *>(index.internalPointer());

        if (index.column() == 2 && item->getType() == TreeItem::ItemType::ObjectParameterEditable) {
                return Qt::ItemFlags(QAbstractItemModel::flags(index) | Qt::ItemIsEditable);
        }

        return QAbstractItemModel::flags(index);
}

QVariant
TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
        return orientation == Qt::Horizontal && role == Qt::DisplayRole ? rootItem->data(section)
                                                                        : QVariant{};
}

QModelIndex
TreeModel::index(int row, int column, const QModelIndex &parent) const
{
        if (!hasIndex(row, column, parent))
                return {};

        TreeItem *parentItem =
            parent.isValid() ? static_cast<TreeItem *>(parent.internalPointer()) : rootItem.get();

        if (auto *childItem = parentItem->child(row))
                return createIndex(row, column, childItem);
        return {};
}

QModelIndex
TreeModel::parent(const QModelIndex &index) const
{
        if (!index.isValid())
                return {};

        auto     *childItem  = static_cast<TreeItem *>(index.internalPointer());
        TreeItem *parentItem = childItem->parentItem();

        return parentItem != rootItem.get() ? createIndex(parentItem->row(), 0, parentItem)
                                            : QModelIndex{};
}

int
TreeModel::rowCount(const QModelIndex &parent) const
{
        if (parent.column() > 0)
                return 0;

        const TreeItem *parentItem = parent.isValid()
                                         ? static_cast<const TreeItem *>(parent.internalPointer())
                                         : rootItem.get();

        return parentItem->childCount();
}

void
TreeModel::setupModelData(TreeItem *parent)
{
        TreeItem    *currParent = parent;
        QVariantList columnData;

        columnData << tr("Свойства объекта") << "" << "";
        currParent = currParent->appendChild(
            std::make_unique<TreeItem>(TreeItem::ItemType::ObjectProperty, columnData, currParent));

        columnData.clear();
        columnData << "" << tr("Идентификатор")
                   << "\"" +
                          QString::fromStdString(
                              m_toml["properties"]["id"].value_or<std::string>("")) +
                          "\"";
        currParent->appendChild(
            std::make_unique<TreeItem>(TreeItem::ItemType::ObjectProperty, columnData, currParent));

        columnData.clear();
        columnData << "" << tr("Тип")
                   << "\"" +
                          QString::fromStdString(
                              m_toml["properties"]["type"].value_or<std::string>("")) +
                          "\"";
        currParent->appendChild(
            std::make_unique<TreeItem>(TreeItem::ItemType::ObjectProperty, columnData, currParent));

        columnData.clear();
        QString objectDefaultName;
        QString objectNameL10n;

        auto objectNameTable = m_toml["properties"]["name"].as_table();
        for (const auto &[key, val] : *objectNameTable) {
                QString strKey = QString::fromStdString(key.data());
                QString strVal = QString::fromStdString(val.value<std::string>().value());
                if (strKey == QString("default")) {
                        objectDefaultName = strVal;
                }
                if (strKey == m_systemLanguage) {
                        objectNameL10n = strVal;
                }
        }

        QString objectName;
        if (objectNameL10n.isEmpty()) {
                objectName = objectDefaultName;
        } else {
                objectName = objectNameL10n;
        }

        columnData << "" << tr("Наименование объекта") << "\"" + objectName + "\"";
        currParent->appendChild(
            std::make_unique<TreeItem>(TreeItem::ItemType::ObjectProperty, columnData, currParent));

        currParent = rootItem.get();

        columnData.clear();
        columnData << tr("Параметры объекта") << "" << "";
        currParent =
            currParent->appendChild(std::make_unique<TreeItem>(TreeItem::ItemType::ObjectParameter,
                                                               columnData,
                                                               currParent));

        auto objectParams = m_toml["parameters"].as_array();
        for (const auto &param : *objectParams) {
                columnData.clear();
                columnData << tr("Параметр") << "" << "";
                auto parent = currParent->appendChild(
                    std::make_unique<TreeItem>(TreeItem::ItemType::ObjectParameter,
                                               columnData,
                                               currParent));

                const toml::table paramTable = *param.as_table();

                columnData.clear();
                columnData << "" << tr("Идентификатор")
                           << "\"" +
                                  QString::fromStdString(
                                      paramTable["id"].value_or<std::string>("")) +
                                  "\"";
                parent->appendChild(std::make_unique<TreeItem>(TreeItem::ItemType::ObjectParameter,
                                                               columnData,
                                                               parent));

                columnData.clear();
                QString paramType =
                    QString::fromStdString(paramTable["type"].value_or<std::string>(""));
                columnData << "" << tr("Тип") << paramType;
                parent->appendChild(std::make_unique<TreeItem>(TreeItem::ItemType::ObjectParameter,
                                                               columnData,
                                                               parent));

                bool paramTypeFlag;
                if (paramType == "integer") {
                        paramTypeFlag = false;
                } else {
                        paramTypeFlag = true;
                }

                columnData.clear();
                columnData << "" << tr("Признак обязательности")
                           << QString(paramTable["required"].value<bool>().value() ? "true"
                                                                                   : "false");
                parent->appendChild(std::make_unique<TreeItem>(TreeItem::ItemType::ObjectParameter,
                                                               columnData,
                                                               parent));

                columnData.clear();
                QString strParamDefaultValue;
                if (!paramTypeFlag) {
                        // integer
                        strParamDefaultValue =
                            QString::number(paramTable["default_value"].value<int>().value());
                        columnData << "" << tr("Значение по умолчанию") << strParamDefaultValue;
                } else {
                        // string
                        strParamDefaultValue = QString::fromStdString(
                            paramTable["default_value"].value<std::string>().value());
                        columnData << "" << tr("Значение по умолчанию")
                                   << "\"" + strParamDefaultValue + "\"";
                }
                parent->appendChild(std::make_unique<TreeItem>(TreeItem::ItemType::ObjectParameter,
                                                               columnData,
                                                               parent));

                columnData.clear();
                auto               paramPossibleValues = paramTable["possible_values"].as_array();
                std::ostringstream oss;
                QStringList        strParamPossibleValues;
                QString            strVal;
                oss << "[ ";
                for (size_t i = 0; i < paramPossibleValues->size(); ++i) {
                        if (!paramTypeFlag) {
                                // integer
                                auto val = (*paramPossibleValues)[i].value<int>();
                                strVal   = QString::number(*val);
                                oss << *val;
                        } else {
                                // string
                                auto val = (*paramPossibleValues)[i].value<std::string>();
                                strVal   = QString::fromStdString(*val);
                                oss << "\"" << *val << "\"";
                        }
                        if (i < paramPossibleValues->size() - 1) {
                                oss << ", ";
                        }
                        strParamPossibleValues.append(strVal);
                }
                oss << " ]";
                std::string result = oss.str();
                columnData << "" << tr("Возможные значения") << QString::fromStdString(result);
                parent->appendChild(std::make_unique<TreeItem>(TreeItem::ItemType::ObjectParameter,
                                                               columnData,
                                                               parent));

                columnData.clear();
                QString strParamVal;
                if (!paramTypeFlag) {
                        // integer
                        strParamVal = QString::number(paramTable["value"].value<int>().value());
                        columnData << "" << tr("Значение") << strParamVal;
                } else {
                        // string
                        strParamVal = QString::fromStdString(
                            paramTable["value"].value<std::string>().value());
                        columnData << "" << tr("Значение") << "\"" + strParamVal + "\"";
                }
                TreeItem *t = parent->appendChild(
                    std::make_unique<TreeItem>(TreeItem::ItemType::ObjectParameterEditable,
                                               columnData,
                                               parent));
                t->setParamPossibleValues(strParamPossibleValues);
                t->setParamValue(strParamVal);
        }
}

bool
TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
        if (role == Qt::EditRole) {
                auto *item = static_cast<TreeItem *>(index.internalPointer());

                if (index.column() == 2 &&
                    item->getType() == TreeItem::ItemType::ObjectParameterEditable) {
                        QString newValue = value.toString();

                        bool isValidValue = false;

                        if (item->getParamPossibleValues().contains(newValue)) {
                                isValidValue = true;
                        }

                        if (isValidValue) {
                                item->setData(index.column(), newValue);
                                return true;
                        }
                }
        }
        return false;
}


