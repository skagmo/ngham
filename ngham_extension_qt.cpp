#include "ngham_extension_qt.h"
#include <QStandardItemModel>


NghamExtension::NghamExtension(){
}

NghamExtension::~NghamExtension(){
}

void NghamExtension::parse(uint8_t* d, uint16_t d_len)
{
    int start = 0;
    for (int j=0; j<ngh_ext_numpkts(d, d_len); j++){
        if ( ((int)d[start] >= EXT_TYPES) || ((d[start+1]+2) > d_len) ){
            m_pkts.clear();
            return;
        }

        // Insert extension packet type as key and bytearray with data as value
        m_pkts.insert((int)d[start], QByteArray((char*)&d[start+2], (uint8_t)d[start+1]));

        // Go to next sub-packet start
        start += d[start+1] + 2;
    }
}

bool NghamExtension::hasExtensionType(uint8_t type){
    return (m_pkts.contains(type));
}

QString NghamExtension::getSource(void){
    QMapIterator<int, QByteArray> i (m_pkts);
    while (i.hasNext()){
        i.next();
        if (i.key() == EXT_TYPE_SRC){
            const ngham_src_t* id = (ngham_src_t*)(i.value().data());
            char call[11];
            ngh_ext_decode_callsign(call, (uint8_t*)id->call_ssid);
            return (QString(call));
        }
    }
    return QString("");
}

QByteArray NghamExtension::getData(void){
    QMapIterator<int, QByteArray> i (m_pkts);
    while (i.hasNext()){
        i.next();
        if (i.key() == EXT_TYPE_DATA) return i.value();
    }
    return QByteArray();
}

QStringList NghamExtension::getExtensionTypeStrings(void){
    QStringList types;
    QMapIterator<int, QByteArray> i (m_pkts);
    while (i.hasNext()){
        i.next();
        if (i.key() >= EXT_TYPES) return QStringList("INVALID");
        types.append(EXT_TYPE_STRINGS[i.key()]);
    }
    return types;
}



QString NghamExtension::getExtensionCmdReply(void){
    QMapIterator<int, QByteArray> i (m_pkts);
    while (i.hasNext()){
        i.next();
        if (i.key() == EXT_TYPE_TEXT) return QString::fromLatin1(i.value());
    }
    return QString("");
}

void appendItemRow(QStandardItem* item, QString name, QString value){
   QList<QStandardItem*> row;
   row << new QStandardItem(name) << new QStandardItem(value);
   item->appendRow(row);
}

QString NghamExtension::secondsToDhms(unsigned int sec){
    int days = sec / 86400; sec %= 86400;
    int hours = sec / 3600; sec %= 3600;
    int min = sec / 60; sec %= 60;
    if (days) return QString().sprintf("%u day(s) %02u:%02u:%02u", days, hours, min, sec);
    else return QString().sprintf("%02u:%02u:%02u", hours, min, sec);
}

void NghamExtension::getConvertedExtensionData(QStandardItemModel* model){
    QMapIterator<int, QByteArray> i (m_pkts);
    while (i.hasNext()){
        i.next();
        QList<QStandardItem*> list = model->findItems(EXT_TYPE_STRINGS[i.key()]);
        QStandardItem* subtree;

        if (list.empty()){
            subtree = new QStandardItem(EXT_TYPE_STRINGS[i.key()]);
            model->appendRow(subtree);
        }
        else if (list.size() == 1){
            subtree = list[0];
            subtree->removeRows(0, subtree->rowCount()); // Removing entire subtree
        }

        switch (i.key()){
            case EXT_TYPE_SRC:
                {
                    const ngham_src_t* id = reinterpret_cast<const ngham_src_t*>(i.value().data());
                    char call[11];
                    ngh_ext_decode_callsign(call, (uint8_t*)id);
                    appendItemRow(subtree, "Callsign", QString(call));
                    appendItemRow(subtree, "Seq. nr", QString::number((unsigned int)id->sequence));
                }
                break;
            case EXT_TYPE_DEST:
                {
                    char call[11];
                    ngh_ext_decode_callsign(call, (uint8_t*)i.value().data());
                    appendItemRow(subtree, "Callsign", QString(call));
                }
                break;
            case EXT_TYPE_STAT:
                {
                    const ngham_stat_t* stat = reinterpret_cast<const ngham_stat_t*>(i.value().data());
                    appendItemRow(subtree, "Hardware version", QString("%1/%2").arg((stat->hw_ver&0xffc0)>>6).arg(stat->hw_ver&0x003f));
                    appendItemRow(subtree, "Serial number", QString::number(stat->serial));
                    appendItemRow(subtree, "Software version", QString("%1.%2.%3").arg((stat->sw_ver&0xf000)>>12).arg((stat->sw_ver&0x0f00)>>8).arg(stat->sw_ver&0xff));
                    appendItemRow(subtree, "Uptime", secondsToDhms(stat->uptime_s));
                    appendItemRow(subtree, "Voltage (V)", QString::number(((float)stat->voltage)/10, 'f', 2));
                    appendItemRow(subtree, "Temperature (C)", QString::number(stat->temp));
                    appendItemRow(subtree, "Signal (dBm)", QString::number((int)stat->signal-200));
                    appendItemRow(subtree, "Noise (dBm)", QString::number((int)stat->noise-200));
                    appendItemRow(subtree, "RX count OK", QString::number(stat->cntr_rx_ok));
                    appendItemRow(subtree, "RX count fixed", QString::number(stat->cntr_rx_fix));
                    appendItemRow(subtree, "RX count errors", QString::number(stat->cntr_rx_err));
                    appendItemRow(subtree, "TX count", QString::number(stat->cntr_tx));
                }
                break;
            case EXT_TYPE_TOH:
                {
                    const ngham_toh_t* toh = reinterpret_cast<const ngham_toh_t*>(i.value().data());
                    appendItemRow(subtree, "Time of hour (us)", QString::number(toh->toh_us));
                    appendItemRow(subtree, "Validity", QString::number(toh->toh_val));
                }
                break;
            case EXT_TYPE_TEXT:
                appendItemRow(subtree, "Reply", QString::fromLatin1(i.value()));
                break;
            case EXT_TYPE_SIMPLEHOP:
                {
                    const ngham_simplehop_t* hop = reinterpret_cast<const ngham_simplehop_t*>(i.value().data());
                    appendItemRow(subtree, "Hops remaining", QString::number(hop->hops_remaining));
                    appendItemRow(subtree, "Hops total", QString::number(hop->hops_total));
                }
                break;
        }
    }
}

QByteArray NghamExtension::packCommand(QString destination, QString source, QString command)
{
    const unsigned int MAX_SIZE = 220 - 4 - sizeof(ngham_src_t) - sizeof(ngham_dest_t);
    if (command.size() > MAX_SIZE) return QByteArray();

    uint8_t d[MAX_SIZE];
    d[0] = EXT_TYPE_SRC;
    d[1] = 7;
    ngh_ext_encode_callsign(&d[2], source.toLatin1().data());
    d[8] = 123; // Seq
    d[9] = EXT_TYPE_DEST;
    d[10] = 6;
    ngh_ext_encode_callsign(&d[11], destination.toLatin1().data());
    d[17] = EXT_TYPE_CMD_REQ;
    d[18] = command.size();
    memcpy(d+19, command.toLatin1().data(), command.size());

    return QByteArray((char*)d, 19+command.size());
}

void NghamExtension::addDestination(QString call){
    QByteArray data;
    data.resize(2 + sizeof(ngham_dest_t));
    data.data()[0] = EXT_TYPE_DEST;
    data.data()[1] = sizeof(ngham_dest_t);
    ngh_ext_encode_callsign((uint8_t*)(data.data()+2), call.toLatin1().data());
    m_txpkts.append(data);
}

void NghamExtension::addSource(QString call, uint8_t seq){
    QByteArray data;
    data.resize(2 + sizeof(ngham_src_t));
    data.data()[0] = EXT_TYPE_SRC;
    data.data()[1] = sizeof(ngham_src_t);
    ngham_src_t* src = (ngham_src_t*)(data.data()+2);
    ngh_ext_encode_callsign((uint8_t*)&(src->call_ssid), call.toLatin1().data());
    src->sequence = seq;
    m_txpkts.append(data);
}

void NghamExtension::addCommand(QString command){
    QByteArray data;
    data.resize(2);
    data.data()[0] = EXT_TYPE_CMD_REQ;
    data.data()[1] = command.toLatin1().size();
    data.append(command.toLatin1());
    m_txpkts.append(data);
}

void NghamExtension::addData(QByteArray in){
    QByteArray data;
    data.resize(2);
    data.data()[0] = EXT_TYPE_DATA;
    data.data()[1] = in.size();
    data.append(in);
    m_txpkts.append(data);
}

QByteArray NghamExtension::getPacket(void){
    return m_txpkts.join();
}

