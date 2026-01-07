#include "utils.h"

#include <QUuid>
#include <QCryptographicHash>

namespace Utils {
    QString generateUUID() {
        QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        return uuid;
    }

    QString hashPassword(const QString& plainPassword) {
        QByteArray data = plainPassword.toUtf8();
        QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
        return hash.toHex();
    }
}
