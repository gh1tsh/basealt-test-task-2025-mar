#include "MainWindow.h"
#include "TreeItemDelegate.h"

#include "./ui_MainWindow.h"

#include <QFileDialog>
#include <QLocale>
#include <QMessageBox>

#include <toml++/toml.hpp>

#include <filesystem>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), model(this)
{
        ui->setupUi(this);

        connect(ui->actionQuitProgram, &QAction::triggered, qApp, &QApplication::quit);

        connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openFile);

        connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);

        ui->treeView->setModel(&model);
        m_treeItemDelegate = new TreeItemDelegate(ui->treeView);
        ui->treeView->setItemDelegateForColumn(2, m_treeItemDelegate);
        ui->treeView->setEditTriggers(QAbstractItemView::DoubleClicked |
                                      QAbstractItemView::SelectedClicked);
        for (int c = 0; c < model.columnCount(); ++c)
                ui->treeView->resizeColumnToContents(c);
        ui->treeView->expandAll();
}

MainWindow::~MainWindow()
{
        delete m_treeItemDelegate;

        delete ui;
}

void
MainWindow::openFile()
{
        const QString filePath =
            QFileDialog::getOpenFileName(this,
                                         tr("Выберите TOML-файл для просмотра"),
                                         QDir::homePath(),
                                         tr("Текстовые файлы (*.toml)"));
        if (filePath.isEmpty()) {
                showErrorMessage("Не удалось открыть файл. Причина: передан пустой путь.");
                return;
        }

        std::filesystem::path t(filePath.toStdString());

        if (t.extension() != std::filesystem::path(".toml")) {
                showErrorMessage(
                    "Недопустимый формат файла. Следует использовать файлы в формате TOML.");
                return;
        }

        try {
                model.reset(filePath);
        } catch (const toml::parse_error &err) {
                std::string what(err.description().begin(), err.description().end());
                showErrorMessage("Не удалось выполнить разбор файла TOML. Причина: '" +
                                 QString::fromStdString(what) + "'.");
        } catch (const std::runtime_error &e) {
                showErrorMessage(QString::fromStdString(e.what()));
        }

        ui->treeView->setModel(&model);
        m_treeItemDelegate = new TreeItemDelegate(ui->treeView);
        ui->treeView->setItemDelegateForColumn(2, m_treeItemDelegate);
        ui->treeView->expandAll();
        for (int c = 0; c < model.columnCount(); ++c)
                ui->treeView->resizeColumnToContents(c);
}

void
MainWindow::about()
{
        QMessageBox::about(this,
                           tr("О программе"),
                           tr("<b>TomlObjectViewer</b><br>"
                              "Версия: 0.2.0<br>"
                              "Автор: san<br><br>"
                              "Программа для просмотра объектов, описанных в формате TOML."));
}

void
MainWindow::showErrorMessage(const QString &message)
{
        QMessageBox::critical(this, "Ошибка", message);
}
