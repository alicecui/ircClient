#include "IrcClient.h"
#include "SimpleIrcProtocol.h"

IrcClient::IrcClient(QWidget *parent)
    : QMainWindow(parent)
    , m_ircProtocol(NULL)
{
    ui.setupUi(this);

    initIrc();
    m_channel = "irc-channel";
    connectToHost();

    QObject::connect(ui.sendBut, SIGNAL(pressed()), this, SLOT(on_sendBtn_pressed()));
}

IrcClient::~IrcClient()
{
    deleteIrc();
}

void IrcClient::initIrc()
{
    m_ircProtocol = new SimpleIrcProtocol(this);
    connect(m_ircProtocol, SIGNAL(privMsgToMe(QString, QString)) , this, SLOT(onPrivMsgToMe(QString, QString)));
    connect(m_ircProtocol, SIGNAL(privMsgToChannel(QString, QString, QString)) , this, SLOT(onPrivMsgToChannel(QString, QString, QString)));
    connect(m_ircProtocol, SIGNAL(noticeMsgToMe(QString, QString)) , this, SLOT(onNoticeMsgToMe(QString, QString)));
    connect(m_ircProtocol, SIGNAL(noticeMsgToChannel(QString, QString, QString)) , this, SLOT(onNoticeMsgToChannel(QString, QString, QString)));
    connect(m_ircProtocol, SIGNAL(namesList(QStringList)) , this, SLOT(onNamesList(QStringList)));
    connect(m_ircProtocol, SIGNAL(topic(QString, QString)) , this, SLOT(onTopic(QString, QString)));
    connect(m_ircProtocol, SIGNAL(connectTimeout(bool, QString, QString)) , this, SLOT(onConnectToChatServer(bool, QString, QString)));
    connect(m_ircProtocol, SIGNAL(join(QString)), this, SLOT(onJoin(QString)));
    connect(m_ircProtocol, SIGNAL(quit(QString)), this, SLOT(onQuit(QString)));
    connect(m_ircProtocol, SIGNAL(reconnect()), this, SLOT(onReconnect()));
    connect(m_ircProtocol, SIGNAL(reGetChatHost()), SLOT(onReGetChatHost()));
    connect(m_ircProtocol, SIGNAL(dirtyWordRecognised(QString, QString)), this, SLOT(onDirtyWordRecognised(QString, QString)));
}

void IrcClient::deleteIrc()
{
    if (m_ircProtocol) {
        m_ircProtocol->quit();
        m_ircProtocol->close();
        m_ircProtocol->deleteLater();
        m_ircProtocol = NULL;
    }
}

void IrcClient::connectToHost()
{
    QList<IrcServer> ircServerList;
    IrcServer ircServer;
    ircServer.host = "10.99.2.200";
    ircServer.port = 6666;
    ircServer.passwd = "cuijie";
    ircServerList.append(ircServer);

    m_ircProtocol->connectToHost(ircServerList, QSysInfo::machineHostName(), QStringList(m_channel));//channels: ("#2L11113")//name: "s_2_11113_2383553_2"//
}

void IrcClient::disconnect()
{
    if (m_ircProtocol) {
        m_ircProtocol->close();
    }
}

void IrcClient::on_sendBtn_pressed()
{
    if (ui.sendEdit->text().isEmpty()) {
        return;
    }

    if (m_ircProtocol) {
        m_ircProtocol->sendMsg(ui.sendEdit->text());
        ui.listWidget->addItem(ui.sendEdit->text());
        ui.sendEdit->clear();
    }
}

void IrcClient::onDirtyWordRecognised(const QString nick, const QString word)
{
    qDebug() << Q_FUNC_INFO << nick << word;
}

void IrcClient::onConnectToChatServer(bool flag, const QString text, const QString errStr)
{
    qDebug() << Q_FUNC_INFO << flag << text << errStr;
}

void IrcClient::onIrcGslbTimeout(int code)
{
    qDebug() << Q_FUNC_INFO << code;
}

void IrcClient::onGetChatHostPollingAgain()
{
    qDebug() << Q_FUNC_INFO;
}

void IrcClient::onGetChatServerResult(bool result, const QJsonValue &value, const QString &err)
{
    qDebug() << Q_FUNC_INFO << result << value << err;
}

void IrcClient::onReGetChatHost()
{
    qDebug() << Q_FUNC_INFO ;
}

void IrcClient::onReconnect()
{
    qDebug() << Q_FUNC_INFO;
}

void IrcClient::onNamesList(const QStringList &nicks)
{
    qDebug() << Q_FUNC_INFO << nicks;
}

void IrcClient::onJoin(const QString &name)
{
    qDebug() << Q_FUNC_INFO << name;
}

void IrcClient::onQuit(const QString &name)
{
    qDebug() << Q_FUNC_INFO << name;
}

void IrcClient::onTopic(const QString  &content, const QString &channel)
{
    ui.listWidget->addItem(channel + content);
}

void IrcClient::onPrivMsgToMe(const QString &senderNick, const QString &content)
{
    ui.listWidget->addItem(senderNick + content);
}

void IrcClient::onPrivMsgToChannel(const QString &senderNick, const QString &content, const QString &channel)
{
    ui.listWidget->addItem(channel + senderNick + content);
}

void IrcClient::onNoticeMsgToMe(const QString &senderNick, const QString &content)
{
    ui.listWidget->addItem(senderNick + content);
}

void IrcClient::onNoticeMsgToChannel(const QString &senderNick, const QString &content, const QString &channel)
{
    ui.listWidget->addItem(channel + senderNick + content);
}