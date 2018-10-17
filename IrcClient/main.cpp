#include "IrcClient.h"
#include <QtWidgets/QApplication>
#include <qdebug.h>

#include "XsLog.hpp"

static void QtMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    QByteArray localMsg = msg.toLocal8Bit();
    static int size = 4096;
    static char *buf = new char[size];

    switch (type) {
    case QtDebugMsg:
        qsnprintf(buf, size, "Debug: %s", localMsg.constData());
        break;
    case QtInfoMsg:
        qsnprintf(buf, size, "Info: %s", localMsg.constData());
        break;
    case QtWarningMsg:
        qsnprintf(buf, size, "Warning: %s", localMsg.constData());
        break;
    case QtCriticalMsg:
        qsnprintf(buf, size, "Critical: %s", localMsg.constData());
        break;
    case QtFatalMsg:
        qsnprintf(buf, size, "Fatal: %s", localMsg.constData());
        abort();
    }

    xs_log_trace("%s", buf);
}

int main(int argc, char *argv[])
{
    QString logErrStr;
    if (!g_logCtx->setLog2File(true, logErrStr)) {
        qDebug() << "log err";
        return -1;
    }

    qInstallMessageHandler(QtMessageOutput);

    QApplication a(argc, argv);
    IrcClient w;
    w.show();
    return a.exec();
}
