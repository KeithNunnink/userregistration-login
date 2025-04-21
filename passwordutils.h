#ifndef PASSWORDUTILS_H
#define PASSWORDUTILS_H

#include <QString>
#include <QCryptographicHash>

namespace PasswordUtils {
    inline QString hashPassword(const QString &password) {
        // Generate a SHA-256 hash of the password
        QByteArray hash = QCryptographicHash::hash(
            password.toUtf8(), 
            QCryptographicHash::Sha256
        );
        
        // Return the hash as a hex string
        return QString(hash.toHex());
    }
}

#endif // PASSWORDUTILS_H
