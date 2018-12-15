#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "d_scanner.h"
#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QDesktopWidget>
#include <iostream>
#include <QtWidgets>
#include <QGraphicsScene>
#include <QTreeWidget>
#include <fcntl.h>
#include <QPushButton>
#include <QAbstractButton>
#include <memory>
#include <QCommonStyle>
#include <QProgressBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scanner(new d_scanner)
{
    ui->setupUi(this);

    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setStretchLastSection(false);

    QCommonStyle style;
    ui->action_scan->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));

    qRegisterMetaType<QVector<QVector<QString>>>();
    qRegisterMetaType<size_t>("size_t");
    ui->pushButton->setVisible(false);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->progressBar->reset();
    init_connections();
}

MainWindow::~MainWindow()
{

}

void MainWindow::init_connections() {
    connect(scanner.get(), SIGNAL(return_duplicates(QVector<QVector<QString>>, size_t, bool)),
            this, SLOT(add_duplicates(QVector<QVector<QString>>, size_t, bool)));

    connect(scanner.get(), SIGNAL(throw_message(QString)), this, SLOT(message_handler(QString)));

    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(on_treeWidget_customContextMenuRequested(const QPoint&)));

    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            this, SLOT(open_file()));

    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(try_exit_scanning()));
    connect(scanner.get(), SIGNAL(return_files_number(size_t)), this, SLOT(set_progress_bar(size_t)));
}

void MainWindow::on_action_scan_triggered()
{
    current_dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                     QString(),
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (current_dir.isEmpty()) {
        return;
    }
    if (scanning) {
        try_exit_scanning();
    }
    scanner->set_root(current_dir);
    prepare_window_for_scanning();
    scanner->start();
}

void MainWindow::prepare_window_for_scanning() {
    ui->treeWidget->clear();
    ui->pushButton->setVisible(true);
    ui->progressBar->reset();
    ui->label->setText("In progress...");
    scanning = true;
    duplicate_groups_cnt = 0;
}

void MainWindow::show_info() {
    ui->pushButton->setVisible(false);
    ui->label->setText(QString("In progress... Done! %1 groups of duplicates were found.").arg(QString::number(duplicate_groups_cnt)));
    scanning = false;
    setWindowTitle(QString("Duplicates - %1").arg(current_dir));
}

void MainWindow::add_duplicates(QVector<QVector<QString>> duplicates, size_t counter, bool last) {
    duplicate_groups_cnt += duplicates.size();
    if (last) {
        show_info();
    }
    ui->progressBar->setValue(counter);
    for (auto&& d_group : duplicates) {
        auto item = new QTreeWidgetItem(ui->treeWidget);
        qint64 sz = QFile(d_group[0]).size();

        item->setText(0, d_group[0]);
        item->setText(1, QString::number(d_group.size()));
        item->setText(2, QString::number(sz));

        for (qint32 i = 1; i < d_group.size(); i++) {
            auto child = new QTreeWidgetItem(item);
            child->setText(0, d_group[i]);
        }

        ui->treeWidget->addTopLevelItem(item);
    }

}

void MainWindow::on_treeWidget_customContextMenuRequested(const QPoint& pos) {
    auto menu = new QMenu(this);

    menu->addAction(QString("Open"), this, SLOT(open_file()));
    menu->addAction(QString("Open in folder"), this, SLOT(open_file_in_folder()));
    menu->addAction(QString("Delete"), this, SLOT(delete_file()));
    menu->popup(ui->treeWidget->viewport()->mapToGlobal(pos));
}

QString command_with_escapes(QString const& command, QString const& path) {
    QString res = command;
    for (auto&& c : path) {
        if (c.isSpace()) {
            res.append('\\');
        }
        res.append(c);
    }
    return res;
}

void MainWindow::open_file() {
    auto path = ui->treeWidget->currentItem()->data(0, 0).toString();
    QString command = QString("open ");
    system(command_with_escapes(command, path).toStdString().c_str());
}

void MainWindow::open_file_in_folder() {
    auto path = ui->treeWidget->currentItem()->data(0, 0).toString();
    QString command = QString("open ");
    system(command_with_escapes(command, path.mid(0, path.lastIndexOf('/'))).toStdString().c_str());
}

void MainWindow::delete_file() {
    auto current_item = ui->treeWidget->currentItem();
    auto path = current_item->data(0, 0).toString();
    auto res = exec_message_box(QString("Are you sure you want to delete the following file: ").append(path));

    if (res == QMessageBox::Yes) {
        QFile::remove(path);
        current_item->setForeground(0, QBrush(Qt::gray));
    }
}

int MainWindow::exec_message_box(QString const& message) {
    QMessageBox msg_box;
    msg_box.setText(message);
    msg_box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msg_box.setDefaultButton(QMessageBox::Yes);
    return msg_box.exec();
}

void MainWindow::message_handler(QString text) {
    QMessageBox msgB;
    msgB.setText(text);
    msgB.exec();
}

void MainWindow::try_exit_scanning() {
    if (scanner->isInterruptionRequested()) {
        return;
    }
    auto res = exec_message_box(QString("Are you sure you want to cancel scanning?"));

    if (res == QMessageBox::Yes) {
        ui->pushButton->setVisible(false);
        ui->label->setText("Cancelling...");
        kill_scanner();
        ui->label->setText("Cancelled");
    }
}

void MainWindow::kill_scanner() {
    scanner->requestInterruption();
    scanner->wait();
    scanning = false;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    kill_scanner();
}

void MainWindow::set_progress_bar(size_t max) {
    ui->progressBar->setMaximum(max);
}
