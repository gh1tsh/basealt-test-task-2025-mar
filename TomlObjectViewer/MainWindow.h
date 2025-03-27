#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TreeItemDelegate.h"
#include "TreeModel.h"

#include <QContextMenuEvent>
#include <QMainWindow>

QT_BEGIN_NAMESPACE

namespace Ui
{
class MainWindow;
}

QT_END_NAMESPACE

class MainWindow final : public QMainWindow
{
        Q_OBJECT

public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

private slots:
        void openFile();

        void about();

private:
        void showErrorMessage(const QString &message);

        Ui::MainWindow   *ui;
        TreeModel         model;
        TreeItemDelegate *m_treeItemDelegate;
};
#endif    // MAINWINDOW_H
