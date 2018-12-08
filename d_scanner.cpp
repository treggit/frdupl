#include "d_scanner.h"
#include <QString>
#include <QVector>
#include <QByteArray>
#include <QHash>
#include <QDir>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QFile>
#include <iostream>
#include <QMessageBox>

d_scanner::d_scanner(QString const& dir) : root(dir) {}

d_scanner::~d_scanner() = default;

QByteArray d_scanner::get_file_hash(QString const& path) {
    QCryptographicHash qhash(QCryptographicHash::Md5);
    QFile file(path);

    if (file.open(QIODevice::ReadOnly)) {
        const size_t BUFFER_SIZE = 4096;
        QByteArray content;
        while (true) {
            content = file.read(BUFFER_SIZE);
            if (content.size() == 0 || isInterruptionRequested()) {
                break;
            }
            qhash.addData(content);
        }
    } else {
        emit throw_message(QString("Couldn't open file: ").append(path));
        return QByteArray();
    }

    return qhash.result();
}

QVector<QString> d_scanner::first_observe(QString const& dir) {
    QHash<qint64, QVector<QString>> clasters;
    QDirIterator it(dir, QDir::Hidden | QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        clasters[it.fileInfo().size()].append(std::move(path));
    }

    QVector<QString> res;

    for (auto&& claster : clasters) {
        if (claster.size() <= 1) {
            continue;
        }

        for (auto&& file : claster) {
            res.append(std::move(file));
        }
    }

    return res;
}

void d_scanner::find_duplicates(QString const& dir) {
    auto files = first_observe(dir);

    QHash<QByteArray, QVector<QString>> duplicates;
    QVector<QByteArray> duplicate_hashes;
    qint64 cur_size = 0, prev_size = 0;
    for (auto&& file : files) {
        cur_size = QFile(file).size();
        if (cur_size != prev_size && duplicate_hashes.size() > RELEASE_NUMBER) {
            release_duplicates(duplicate_hashes, duplicates);
        }
        prev_size = cur_size;

        QByteArray hash = get_file_hash(file);
        duplicates[hash].append(std::move(file));
        if (duplicates[hash].size() == 2) {
            duplicate_hashes.append(std::move(hash));
        }

        if (isInterruptionRequested()) {
            break;
        }
    }

    release_duplicates(duplicate_hashes, duplicates);

    if (isInterruptionRequested()) {
        emit interrupted();
    }
}

void d_scanner::run() {
    find_duplicates(root);
}

void d_scanner::release_duplicates(QVector<QByteArray>& hashes, QHash<QByteArray, QVector<QString>> const& duplicates) {
    if (isInterruptionRequested()) {
        return;
    }

    QVector<QVector<QString>> res;
    for (auto&& hash : hashes) {
        res.append(duplicates[hash]);
    }
    hashes.resize(0);

    emit return_duplicates(res);
}