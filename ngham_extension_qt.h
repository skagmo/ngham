#pragma once

#include <QString>
#include <QVector>
#include <QStandardItemModel>

extern "C" {
    #include "ngham_extension.h"
}

class NghamExtension
{
    QMap<int, QByteArray> m_pkts;
    QByteArrayList m_txpkts;

public:
    explicit NghamExtension();
    ~NghamExtension();

    void parse(uint8_t*, uint16_t);
    bool hasExtensionType(uint8_t type);
    void getConvertedExtensionData(QStandardItemModel* model);
    QString getSource(void);
    QStringList getExtensionTypeStrings(void);
    QString getExtensionCmdReply(void);
    QByteArray getData(void);
    static QString secondsToDhms(unsigned int sec);

    void addDestination(QString call);
    void addSource(QString call, uint8_t seq);
    void addCommand(QString command);
    void addData(QByteArray in);
    QByteArray getPacket(void);

    static QByteArray packCommand(QString destination, QString source, QString command);
};
