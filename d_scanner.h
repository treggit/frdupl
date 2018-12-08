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
    explicit d_scanner(QString const& dir);
    ~d_scanner();

    void find_duplicates(QString const& dir);

    void run() override;

private:
    QByteArray get_file_hash(QString const& path);
    QVector<QString> first_observe(QString const& dir);

    QHash<QByteArray, QVector<QString>> buffer;
    void release_duplicates(QVector<QByteArray>& hashes, QHash<QByteArray, QVector<QString>> const& duplicates);
    const size_t RELEASE_NUMBER = 30;

    QString root;

signals:
    void return_duplicates(QVector<QVector<QString>> duplicates);
    void throw_message(QString text);
    void interrupted();
};

#endif // SCANNER_H
