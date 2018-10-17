#include "IrcMessage.h"

static QByteArray typeToString(IrcMessageType type)
{
    QByteArray cmd;

    switch (type) {
    case IMT_PASS:
        cmd = "PASS";
        break;
    case IMT_NICK:
        cmd = "NXICK";
        break;
    case IMT_USER:
        cmd = "USER";
        break;
    case IMT_JOIN:
        cmd = "JOIN";
        break;
    case IMT_PRIVMSG:
        cmd = "PRIVMSG";
        break;
    case IMT_NOTICE:
        cmd = "NOTICE";
        break;
    case IMT_TOPIC:
        cmd = "TOPIC";
        break;
    case IMT_PING:
        cmd = "PING";
        break;
    case IMT_PONG:
        cmd = "PONG";
        break;
    default:
        cmd = "PRIVMSG";
        break;
    }

    return cmd;
}

IrcMessage::IrcMessage(IrcMessageType type, const QByteArray &data)
    : m_type(type)
    , m_body(data)
{

}

IrcMessage::IrcMessage()
    : m_type(IMT_INVALID)
{

}

void IrcMessage::setType(IrcMessageType type)
{
    m_type = type;
}

void IrcMessage::setContent(const QByteArray &data)
{
    m_body = data;
}

QByteArray IrcMessage::encode()
{
    return encode(m_type, m_body);
}

QByteArray IrcMessage::encode(IrcMessageType type, const QByteArray &data)
{
    QString typeStr = typeToString(type);
    QString pkt = QString("%1 %2\r\n").arg(typeStr).arg(QString(data));
    return pkt.toUtf8();
}

QByteArray IrcMessage::encode(IrcMessageType type, const QByteArray &channel, const QByteArray &data)
{
    QString typeStr = typeToString(type);
    QString pkt = QString("%1 %2 %3\r\n").arg(typeStr).arg(QString(channel)).arg(QString(data));
    return pkt.toUtf8();
}

QByteArray IrcMessage::decodeNoticeP2PMessage(const QByteArray &nick, const QByteArray &self, const QByteArray &data)
{
    QString pkt = QString(":%1 NOTICE %2 :%3\r\n").arg(QString(self)).arg(QString(nick)).arg(QString(data));
    return pkt.toUtf8();
}

QByteArray IrcMessage::decodePrivMsgP2PMessage(const QByteArray &nick, const QByteArray &self, const QByteArray &data)
{
    QString pkt = QString(":%1 PRIVMSG %2 :%3\r\n").arg(QString(self)).arg(QString(nick)).arg(QString(data));
    return pkt.toUtf8();
}
