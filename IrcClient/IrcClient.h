#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_IrcClient.h"

class SimpleIrcProtocol;
class IrcClient : public QMainWindow
{
    Q_OBJECT

public:
    IrcClient(QWidget *parent = Q_NULLPTR);
    ~IrcClient();

private slots:
    void on_sendBtn_pressed();

private slots:
    void onDirtyWordRecognised(const QString nick, const QString word);
    void onConnectToChatServer(bool flag, const QString text, const QString errStr);
    void onIrcGslbTimeout(int code = 0);
    void onGetChatHostPollingAgain();
    void onGetChatServerResult(bool result, const QJsonValue &value, const QString &err);
    void onReGetChatHost();
    void onReconnect();
    void onNamesList(const QStringList &nicks);
    void onJoin(const QString &name);
    void onQuit(const QString &name);
    void onTopic(const QString  &content, const QString &channel);
    void onPrivMsgToMe(const QString &senderNick, const QString &content);
    void onPrivMsgToChannel(const QString &senderNick, const QString &content, const QString &channel);
    void onNoticeMsgToMe(const QString &senderNick, const QString &content);
    void onNoticeMsgToChannel(const QString &senderNick, const QString &content, const QString &channel);
    void onPrivMsg(const QString &content);

private:
    void initIrc();
    void deleteIrc();

    void connectToHost();
    void disconnect();

private:
    Ui::IrcClient ui;
    SimpleIrcProtocol*  m_ircProtocol;          // irc协议接口
    QString             m_channel;              // channel
};
