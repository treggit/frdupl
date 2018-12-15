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

d_scanner::d_scanner() = default;

d_scanner::~d_scanner() = default;

void d_scanner::set_root(QString const& r) {
    root = r;
}

QByteArray d_scanner::get_file_hash(QString const& path) {
    QCryptographicHash qhash(QCryptographicHash::Md5);
    QFile file(path);

    if (file.open(QIODevice::ReadOnly)) {
        qhash.addData(&file);
    } else {
        if (!isInterruptionRequested()) {
            emit throw_message(QString("Couldn't open file: ").append(path));
        }
        return QByteArray();
    }

    return qhash.result();
}

QVector<QVector<QString>> d_scanner::first_observe(QString const& dir) {
    QHash<qint64, QVector<QString>> clusters;
    QDirIterator it(dir, QDir::Hidden | QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        clusters[it.fileInfo().size()].append(std::move(path));
        if (isInterruptionRequested()) {
            return {};
        }
    }

    QVector<QVector<QString>> res;

    for (auto&& cluster : clusters) {
        if (cluster.size() <= 1) {
            continue;
        }

        res.append(std::move(cluster));

        if (isInterruptionRequested()) {
            return {};
        }
    }

    return res;
}

void d_scanner::find_duplicates(QString const& dir) {
    auto clusters = first_observe(dir);

    QHash<QByteArray, QVector<QString>> duplicates;
    QVector<QByteArray> duplicate_hashes;

    size_t counter = 0;
    for (auto&& cluster : clusters) {
        counter += cluster.size();
    }
    emit return_files_number(counter);
    counter = 0;
    for (auto&& cluster : clusters) {
        counter += cluster.size();
        for (auto&& file : cluster) {
            QByteArray hash = get_file_hash(file);
            duplicates[hash].append(std::move(file));

            if(duplicates[hash].size() == 2) {
                duplicate_hashes.append(std::move(hash));
            }

            if(isInterruptionRequested()) {
                return;
            }
        }
        if (duplicate_hashes.size() > RELEASE_NUMBER) {
            release_duplicates(duplicate_hashes, duplicates, counter);
        }
    }

    release_duplicates(duplicate_hashes, duplicates, counter, true);
}

void d_scanner::run() {
    find_duplicates(root);
}

void d_scanner::release_duplicates(QVector<QByteArray>& hashes, QHash<QByteArray, QVector<QString>>& duplicates, size_t counter, bool last) {
    if (isInterruptionRequested()) {
        return;
    }

    QVector<QVector<QString>> res;
    for (auto&& hash : hashes) {
        res.append(duplicates[hash]);
    }
    hashes.clear();
    duplicates.clear();


    emit return_duplicates(res, counter, last);
}