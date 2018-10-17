#ifndef SIMPLEIRCPROTOCOL_H
#define SIMPLEIRCPROTOCOL_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QList>
#include <QElapsedTimer>
#include <QAbstractSocket>

struct IrcServer
{
    QString host;
    qint16  port;
    QString passwd;
};

class SimpleIrcProtocol : public QObject
{
    Q_OBJECT
public:
    explicit SimpleIrcProtocol(QObject *parent = 0);
    ~SimpleIrcProtocol();

    void connectToHost(const QList<IrcServer>& ircServerList, const QString &userName, const QStringList &channels);
    void disconnectFromHost();
    void close();
    void sendMsgToChannel(const QByteArray &data, const QString &channel);
    void sendNoticeToChannel(const QByteArray &data, const QString &channel);
    void sendNoticeToPerson(const QByteArray &nick, const QByteArray &self, const QByteArray &data);
    void sendPrivMsgToPerson(const QByteArray &nick, const QByteArray &self, const QByteArray &data);
    void sendT();
    void sendMsg(const QString &data, const QString &channel);

signals:
    void privMsgToMe(const QString &senderNick, const QString &content);
    void privMsgToChannel(const QString &senderNick, const QString &content, const QString &channel);
    void noticeMsgToMe(const QString &senderNick, const QString &content);
    void noticeMsgToChannel(const QString &senderNick, const QString &content, const QString &channel);
    void namesList(const QStringList &nicks);
    void topic(const QString  &content, const QString &channel);
    void join(const QString &name);
    void quit(const QString &name);
    void reconnect();
    void connectTimeout(bool flag, const QString &text, const QString& errStr);
    void dirtyWordRecognised(const QString& nick, const QString& word);
    void reGetChatHost();

private slots:
    void onSocketReadyRead();
    void onSocketConnected();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onHeartbeat();
    void connectToHostInternal();

private:
    void finiSocket();
    void initSocket();
    void writeLine(const QByteArray &line);
    void parseIrcMsg(QString &line, QString &prefix, QString &command, QStringList &params);
    void connectChannels();
    void processPrivMsg(QString &prefix, QString &command, QStringList &params);
    void processNoticeMsg(QString &prefix, QString &command, QStringList &params);
    void processNamesList(QString &prefix, QString &command, QStringList &params);
    void processTopic(QString &prefix, QString &command, QStringList &params);
    void modifyTheNick(QString &nick);
    QString getWord(QString& line);

private:
    QTcpSocket*      m_socket;          // socekt used to connect irc server
    QString          m_errString;       // socket error string
    int              m_serverIndex;     // index of server lists for current server
    QList<IrcServer> m_serverList;      // irc server lists client may connect to
    QString          m_host;            // current irc server address
    qint32           m_port;            // current irc server port
    QString          m_userPass;        // current irc server password
    QString          m_userName;        // user nickname
    QString          m_oldUserName;     // user old nickname
    QStringList      m_channels;        // channels user may join
    QTimer           m_heartBeatTimer;  // heart beat timer
    QElapsedTimer    m_elapsedTimer;    // elapsed timer
    qint64           m_lastTime;        // time since connected to server
    bool             m_timeout;         // indicate if timeout when connecting to server
};

#endif // SIMPLEIRCPROTOCOL_H
