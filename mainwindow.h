#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "d_scanner.h"
#include <memory>

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
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<d_scanner> scanner;

    QString current_dir = QString();
    size_t duplicate_groups_cnt = 0;
    bool scanning = false;


    void init_connections();
    int exec_message_box(QString const& message);
    void kill_scanner();
    void prepare_window_for_scanning();
    void closeEvent(QCloseEvent* event);


        //void create_treeWidgetItems_actions(QTreeWidgetItem* item);

};

#endif // MAINWINDOW_H
