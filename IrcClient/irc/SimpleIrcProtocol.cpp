#include "SimpleIrcProtocol.h"

#include <QRegExp>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

#include "IrcMessage.h"

SimpleIrcProtocol::SimpleIrcProtocol(QObject *parent)
    : QObject(parent)
    , m_socket(NULL)
    , m_timeout(true)
{
    connect(&m_heartBeatTimer, SIGNAL(timeout()), this, SLOT(onHeartbeat()));
    m_heartBeatTimer.setInterval(5000);
}

SimpleIrcProtocol::~SimpleIrcProtocol()
{
    if (m_heartBeatTimer.isActive()) {
        m_heartBeatTimer.stop();
    }

    finiSocket();
}

void SimpleIrcProtocol::connectToHost(const QList<IrcServer> &ircServerList
                                      , const QString& userName
                                      , const QStringList& channels)
{
    m_serverIndex = -1;
    m_serverList.clear();
    for (int i = 0; i < ircServerList.size(); i++) {
        IrcServer ircServer;
        ircServer.host = ircServerList.at(i).host;
        ircServer.port = ircServerList.at(i).port;
        ircServer.passwd = ircServerList.at(i).passwd;
        m_serverList.append(ircServer);
    }
    m_userName = userName;
    m_channels << channels;
    connectToHostInternal();
}

void SimpleIrcProtocol::connectToHostInternal()
{
    m_serverIndex++;
    if (m_serverIndex >= m_serverList.size()) {
        close();
        emit reGetChatHost();  // All server in lists has been tried and failed
        return;
    } else {
        if (m_serverIndex > 0) {
            emit reconnect();  // Send reconnect signal
        }

        finiSocket();
        initSocket();

        //m_heartBeatTimer.start();
        m_elapsedTimer.restart();
        m_lastTime = 0;

        m_host = m_serverList.at(m_serverIndex).host;
        m_port = m_serverList.at(m_serverIndex).port;
        m_userPass = m_serverList.at(m_serverIndex).passwd;
        m_socket->connectToHost(m_host, m_port);

        qDebug() << "Connect to irc server(" << m_host << ":" << m_port <<")";
    }
}


void SimpleIrcProtocol::disconnectFromHost()
{
    if (m_heartBeatTimer.isActive()) {
        m_heartBeatTimer.stop();
    }

    finiSocket();
}

void SimpleIrcProtocol::close()
{
    if (m_heartBeatTimer.isActive()) {
        m_heartBeatTimer.stop();
    }

    finiSocket();
}


void SimpleIrcProtocol::sendMsgToChannel(const QByteArray &data, const QString &channel)
{
    QByteArray out = IrcMessage::encode(IMT_PRIVMSG, channel.toLatin1(), data);
    writeLine(out);
}

void SimpleIrcProtocol::sendNoticeToChannel(const QByteArray &data, const QString &channel)
{
    QByteArray out = IrcMessage::encode(IMT_NOTICE, channel.toLatin1(), data);
    writeLine(out);
}

void SimpleIrcProtocol::sendNoticeToPerson(const QByteArray &nick, const QByteArray &self, const QByteArray &data)
{
    QByteArray out = IrcMessage::decodeNoticeP2PMessage(nick, self, data);
    writeLine(out);
}

void SimpleIrcProtocol::sendPrivMsgToPerson(const QByteArray &nick, const QByteArray &self, const QByteArray &data)
{
    QByteArray out = IrcMessage::decodePrivMsgP2PMessage(nick, self, data);
    writeLine(out);
}

void SimpleIrcProtocol::onSocketReadyRead()
{
    QByteArray line = m_socket->readAll();
    QString ircMsg = line;
    qDebug() << "Original irc msg:" << ircMsg;
    emit privMsg(ircMsg);
    return;
    while (m_socket && m_socket->canReadLine()) {
        QString line = m_socket->readLine().trimmed();
        QString ircMsg = line;
        QString prefix;
        QString command;
        QStringList params;

        qDebug() << "Original irc msg:" << ircMsg;

        parseIrcMsg(line, prefix, command, params);

        if (IRC_COMMAND_PING == command) {

            QRegExp rx("(:)(.*)");
            int pos = 0;
            if ((pos = rx.indexIn(ircMsg, pos)) != -1) {
                QString data = rx.cap(2);
                QByteArray pong = IrcMessage::encode(IMT_PONG, data.toLatin1());
                writeLine(pong);
                m_lastTime = m_elapsedTimer.elapsed();
                qDebug() << "Update last heart-beat time to" << m_lastTime;
            }

        } else if (IRC_COMMAND_PONG == command) {

            m_lastTime = m_elapsedTimer.elapsed();
            qDebug() << "Update last heart-beat time to" << m_lastTime;

        } else if (IRC_COMMAND_DIRWORD_BLOCK == command && 2 == params.size()){

            emit dirtyWordRecognised(params.at(0), params.at(1));

        } else if (IRC_COMMAND_PRIVMSG_NAME == command) {

            processPrivMsg(prefix, command, params);

        } else if (IRC_COMMAND_NOTICE_NAME == command) {

            processNoticeMsg(prefix, command, params);

        } else if (IRC_COMMAND_WELCOME == command) {

            connectChannels();
            emit connectTimeout(true,  tr("The chat server is connected successfully!"), m_errString);
            m_timeout = false;

        } else if (IRC_COMMAND_NAME_LIST == command) {

            processNamesList(prefix, command, params);

        } else if (IRC_COMMAND_TOPIC == command || IRC_COMMAND_TOPIC_NAME == command) {

            processTopic(prefix, command, params);

        } else if (IRC_COMMAND_JOIN_NAME == command) {

            emit join(prefix);

        } else if (IRC_COMMAND_QUIT_NAME == command) {

            emit quit(prefix);

        } else if (IRC_COMMAND_USED_NAME == command && params.contains("Nickname is already in use.")) {

            modifyTheNick(m_userName);
            connectToHostInternal();

        } else {
            qDebug() << "Parse irc replied msg succeed but ignored, prefix:" << prefix
                     << " command:" << command << " params:" << params;
        }
    }
}

void SimpleIrcProtocol::onSocketConnected()
{
    qDebug() << QString("Connected to irc server(%1:%2) succeed.").arg(m_host).arg(m_port);

    //if (m_userPass.isEmpty()) {
    //    m_userPass = "talwx001";  // use defaut password
    //}

    //QByteArray pass = IrcMessage::encode(IMT_PASS, m_userPass.toUtf8());
    //writeLine(pass);

    //QByteArray nick = IrcMessage::encode(IMT_NICK, m_userName.toLatin1());
    //writeLine(nick);

    //QString userStr = QString("%1 0 * :xes").arg(m_userName);
    //QByteArray user = IrcMessage::encode(IMT_USER, userStr.toLatin1());
    //writeLine(user);

    writeLine(QString("nick|%1").arg(m_userName).toUtf8());
}

void SimpleIrcProtocol::onSocketError(QAbstractSocket::SocketError socketError)
{    
    m_errString = QString("Chat socket error: ") + socketError;
    qDebug() << __FUNCTION__ << m_errString;
    emit reconnect();
    return;
    if (m_heartBeatTimer.isActive()) {
        m_heartBeatTimer.stop();
    }

    QTimer::singleShot(1000, this, SLOT(connectToHostInternal()));
    emit connectTimeout(false, tr("The chat server connection is broken! Reconnecting..."), m_errString);
}

void SimpleIrcProtocol::finiSocket()
{
    if (m_socket) {
        m_socket->abort();
        m_socket->deleteLater();
    }
}

void SimpleIrcProtocol::initSocket()
{
    m_socket = new QTcpSocket(this);

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    connect(m_socket, &QTcpSocket::destroyed, this, [=](){m_socket = NULL;});
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError))
            , this, SLOT(onSocketError(QAbstractSocket::SocketError)));
}

void SimpleIrcProtocol::writeLine(const QByteArray &line)
{
    if (m_socket) {
        m_socket->write(line);
        qDebug() << "Write:" << QString(line);
    }
}

void SimpleIrcProtocol::parseIrcMsg(QString &line, QString &prefix, QString &command, QStringList &params)
{
    /* from irc rfc 1459
    2.3.1 Message format in 'pseudo' BNF

         The protocol messages must be extracted from the contiguous stream of
         octets. The current solution is to designate two characters, CR and
         LF, as message separators. Empty messages are silently ignored,
         which permits use of the sequence CR-LF between messages
         without extra problems.

         The extracted message is parsed into the components <prefix>,
         <command> and list of parameters matched either by <middle> or
         <trailing> components.

         The BNF representation for this is:

         <message> ::= [':' <prefix> <SPACE> ] <command> <params>      <crlf>
         <prefix> ::= <servername> | <nick> [ '!' <user> ]      [ '@' <host> ]
         <command> ::= <letter> { <letter> } | <number> <number>      <number>
         <SPACE> ::= ' ' { ' ' }
         <params> ::= <SPACE> [ ':' <trailing> | <middle> <params>      ]

         <middle> ::= <Any *non-empty* sequence of octets not including SPACE
         or NUL or CR or LF, the first of which may not be ':'>
         <trailing> ::= <Any, possibly *empty*, sequence of octets not including
         NUL or CR or LF>

         <crlf> ::= CR LF
    */

    bool ret = true;
    line = line.trimmed();
    if (!line.startsWith(':')) {
        prefix = "";
    }else{
        line.remove(0, 1);
        prefix = getWord(line);
    }
    command = getWord(line);

    // There are only two cases that will jump out of the loop:
    // one is that line is an empty string;
    // the other is that processing to line starts with a ':' character
    // If the line is an empty string, the message format is incorrect.
    while (!line.startsWith(':')) {
        QString word = getWord(line);
        if (!word.isEmpty()) {
            params << word;
        } else {
            ret = false;
            break;
        }
    }

    if (ret) {
        line.remove(0, 1);
        if (!line.isEmpty()) {
            params << line;
        }
    }
}

void SimpleIrcProtocol::connectChannels()
{
    QString str;
    for (int i = 0; i < m_channels.size(); ++i) {
        str.append(m_channels.at(i));
        str.append(",");
    }

    if (str.endsWith(',')) {
        str.remove(str.size()-1, 1);
    }

    QByteArray data = IrcMessage::encode(IMT_JOIN, str.toLatin1());
    writeLine(data);
}

void SimpleIrcProtocol::onHeartbeat()
{
    qint64 etime = m_elapsedTimer.elapsed();
    if ((etime - m_lastTime) >= 8000) {

        qDebug() << "Heart beat timeout, reconnect to server. last time=" << m_lastTime << ", now time=" << etime;

        if (m_heartBeatTimer.isActive()) {
            m_heartBeatTimer.stop();
        }

        if (!m_timeout) {
            m_timeout = true;
        }

        QTimer::singleShot(1000, this, SLOT(connectToHostInternal()));
        return;
    }

    QByteArray ping = IrcMessage::encode(IMT_PING, ":student-client");
    writeLine(ping);
}



void SimpleIrcProtocol::processPrivMsg(QString &prefix, QString &command, QStringList &params)
{
    Q_UNUSED(command);

    if (params.size() != 2) {
        qDebug() << "Priv msg params size error.";
        return;
    }

    QString dst = params.at(0);
    QString content = params.at(1);
    if (dst == m_userName) {
        // Private chat message
        emit privMsgToMe(prefix, content);
    } else if (m_channels.contains(dst)) {
        // Room public message
        emit privMsgToChannel(prefix, content, dst);
    } else {
        qDebug() << "Should be irc internal error";
    }
}

void SimpleIrcProtocol::processNoticeMsg(QString &prefix, QString &command, QStringList &params)
{
    Q_UNUSED(command);

    if (params.size() != 2) {
        qDebug() << "Notice msg params size error.";
        return;
    }

    QString dst = params.at(0);
    QString content = params.at(1);
    if (dst == m_userName) {
        // Private chat message
        emit noticeMsgToMe(prefix, content);
    } else if (m_channels.contains(dst)) {
        // Room public message
        emit noticeMsgToChannel(prefix, content, dst);
    } else {
        qDebug() << "Should irc internal error";
    }
}

void SimpleIrcProtocol::processNamesList(QString &prefix, QString &command, QStringList &params)
{
    Q_UNUSED(prefix)
    Q_UNUSED(command)

    if (params.size() <= 3) {
        qDebug() << "No valid names found.";
        return;
    }

    QStringList names;

    for (int i = 3; i < params.size(); ++i) {
        const QString &nick = params.at(i);
        QStringList nameList = nick.split(" ");
        for (int j = 0; j < nameList.size(); ++j) {
            QString key = nameList.at(j);
            if (key.startsWith('@')) {
                QString _nick = key.mid(1, key.size() - 1);
                names << _nick;
            } else {
                names << key;
            }
        }
    }

    emit namesList(names);
}

void SimpleIrcProtocol::processTopic(QString &prefix, QString &command, QStringList &params)
{
    Q_UNUSED(prefix)

    if (command == IRC_COMMAND_TOPIC) {
        if (params.size() != 3) {
            qDebug() << "No valid topic found, " << IRC_COMMAND_TOPIC;
            return;
        }

        QString nick = params.at(0);
        QString channel = params.at(1);
        QString content = params.at(2);

        if (nick != m_userName) {
            qDebug() << "Should irc internal error, " << IRC_COMMAND_TOPIC;
            return;
        }

        emit topic(content, channel);
    } else if (command == IRC_COMMAND_TOPIC_NAME) {
        if (params.size() != 2) {
            qDebug() << "No valid topic found, " << IRC_COMMAND_TOPIC_NAME << "params:" << params;
            return;
        }

        QString channel = params.at(0);
        QString content = params.at(1);

        emit topic(content, channel);
    }
}

void SimpleIrcProtocol::modifyTheNick(QString &nick)
{
    m_oldUserName = nick;
    if (nick.startsWith("ws_")) {
        nick.remove(0, 1);
    } else if (nick.startsWith("s_")) {
        nick = "w" + nick;
    }
}

QString SimpleIrcProtocol::getWord(QString &line)
{
    if (line.isEmpty()) {
        return QString();
    }

    int idx = line.indexOf(' ');
    // Process the last word
    if (idx == -1) {
        idx = line.size();
    }
    QString word = line.mid(0, idx);
    line.remove(0, idx+1);

    return word.trimmed();
}

void SimpleIrcProtocol::sendT()
{
    QString array = "T";

    if (m_oldUserName.isEmpty()) {
        m_oldUserName = m_userName;
        if (m_oldUserName.startsWith("ws_")) {
            m_oldUserName.remove(0, 1);
        } else if (m_oldUserName.startsWith("s_")) {
            m_oldUserName = "w" + m_oldUserName;
        }
        sendPrivMsgToPerson(m_oldUserName.toUtf8(), m_userName.toUtf8(), array.toUtf8());
    } else {
        sendPrivMsgToPerson(m_oldUserName.toUtf8(), m_userName.toUtf8(), array.toUtf8());
    }

    qDebug() << m_userName << " Send T to " << m_oldUserName;
}

void SimpleIrcProtocol::sendMsg(const QString &data)
{
    writeLine(QString("say|%1|%2").arg(m_userName).arg(data).toUtf8());
}

void SimpleIrcProtocol::quit()
{
    writeLine(QString("quit|%1").arg(m_userName).toUtf8());
}