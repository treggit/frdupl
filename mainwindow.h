#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "d_scanner.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_scan_triggered();
    void add_duplicates(QVector<QVector<QString>> duplicates, bool last);
    void on_treeWidget_customContextMenuRequested(QPoint const& pos);
    void open_file();
    void open_file_in_folder();
    void delete_file();
    void message_handler(QString text);
    void try_exit_scanning();
    void show_info();


private:
    Ui::MainWindow* ui;
    QString current_dir = QString();
    QAction* open_action;
    QAction* open_in_folder_action;
    QAction* delete_file_action;
    size_t duplicate_groups_cnt = 0;
    d_scanner* scanner;
    bool interrupted = false;
    bool scanning = false;


    void init_connections();
    int exec_message_box(QString const& message);

    //void create_treeWidgetItems_actions(QTreeWidgetItem* item);

};

#endif // MAINWINDOW_H
