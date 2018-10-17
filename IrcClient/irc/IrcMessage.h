#ifndef IRCMESSAGE_H
#define IRCMESSAGE_H

#include <QByteArray>
#include <QString>

#define IRC_COMMAND_PING            ( "PING" )
#define IRC_COMMAND_PONG            ( "PONG" )
#define IRC_COMMAND_WELCOME         ( "004" )
#define IRC_COMMAND_NAME_LIST       ( "353" )
#define IRC_COMMAND_TOPIC           ( "332" )
#define IRC_COMMAND_TOPIC_NAME      ( "TOPIC" )
#define IRC_COMMAND_PRIVMSG_NAME    ( "PRIVMSG" )
#define IRC_COMMAND_NOTICE_NAME     ( "NOTICE" )
#define IRC_COMMAND_JOIN_NAME       ( "JOIN" )
#define IRC_COMMAND_QUIT_NAME       ( "QUIT" )
#define IRC_COMMAND_USED_NAME       ( "433"  )
#define IRC_COMMAND_DIRWORD_BLOCK   ( "BLOCK")

enum IrcMessageType {
    IMT_INVALID,
    IMT_PASS,
    IMT_NICK,
    IMT_USER,
    IMT_JOIN,
    IMT_PRIVMSG,
    IMT_NOTICE,
    IMT_TOPIC,
    IMT_PING,
    IMT_PONG
};

class IrcMessage
{
public:
    IrcMessage(IrcMessageType type, const QByteArray &data);
    IrcMessage();

    void setType(IrcMessageType type);
    void setContent(const QByteArray &data);

    QByteArray encode();
    static QByteArray encode(IrcMessageType type, const QByteArray &data);
    static QByteArray encode(IrcMessageType type, const QByteArray &channel, const QByteArray &data);
    static QByteArray decodeNoticeP2PMessage(const QByteArray &nick, const QByteArray &self, const QByteArray &data);
    static QByteArray decodePrivMsgP2PMessage(const QByteArray &nick, const QByteArray &self, const QByteArray &data);

private:
    IrcMessageType  m_type;
    QByteArray      m_body;
};

#endif // IRCMESSAGE_H
