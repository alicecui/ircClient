#ifndef MESSAGEINFO_H
#define MESSAGEINFO_H

#include <QString>

struct ChatMessage
{
    bool    isPriv;         // 是否是私聊消息
    QString senderId;       // 发送者的nick
    QString senderName;     // 发送者名字
    QString msg;            // 文本消息
    QString channel;        // 频道名
    QString version;        // 头像版本
    QString path;           // 头像路径
    double  msecond;        // 接收消息的微秒值
    QString from;           // 发送来源
    QString to;             // 发送目标
    QString content;        // 原始内容，用于站立式直播
    QString own;            // 用于记录用户的名字
};

#endif // MESSAGEINFO_H
