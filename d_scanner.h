#ifndef SCANNER_H
#define SCANNER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QByteArray>
#include <QHash>
#include <QThread>

class d_scanner : public QThread {
    Q_OBJECT

public:
    explicit d_scanner();
    ~d_scanner();

    void find_duplicates(QString const& dir);

    void run() override;
    void set_root(QString const& root);

private:
    QByteArray get_file_hash(QString const& path);
    QVector<QString> first_observe(QString const& dir);

    void release_duplicates(QVector<QByteArray>& hashes, QHash<QByteArray, QVector<QString>> const& duplicates, size_t counter, bool last = false);
    const size_t RELEASE_NUMBER = 100;

    QString root;

signals:
    void return_duplicates(QVector<QVector<QString>> duplicates, size_t counter, bool last = false);
    void throw_message(QString text);
    void return_files_number(size_t num);
};

#endif // SCANNER_H
