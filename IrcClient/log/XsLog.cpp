#include "XsLog.hpp"

#include <QDateTime>
#include <QMutexLocker>

XsLog::XsLog()
    : m_logLevel(MLogLevel::Trace)
    , m_enableCache(false)
    , m_log2Console(true)
    , m_log2File(false)
    , m_enablFILE(false)
    , m_enableLINE(false)
    , m_enableFUNTION(true)
    , m_enableColorPrint(true)
    , m_file(NULL)
    , m_buffer(NULL)
    , m_bufferLength(4096*100)
    , m_timeFormat("yyyy-MM-dd hh:mm:ss")
{
    m_filePath = QString("log.txt");
    m_buffer = new char[m_bufferLength];
}

XsLog::~XsLog()
{
    delete m_file;
    delete [] m_buffer;
}

void XsLog::setLogLevel(int level)
{
    m_logLevel = level;
}

void XsLog::setEnableCache(bool enabled)
{
    m_enableCache = enabled;
}

void XsLog::setLog2Console(bool enabled)
{
    m_log2Console = enabled;
}

bool XsLog::setLog2File(bool enabled, QString &err)
{
    m_log2File = enabled;
    m_file = new QFile(m_filePath);
    if (!m_file->open(QIODevice::WriteOnly)) {
        err = m_file->errorString();
        delete m_file;
        m_file = NULL;

        return false;
    }

    return true;
}

void XsLog::setEnableFILE(bool enabled)
{
    m_enablFILE = enabled;
}

void XsLog::setEnableLINE(bool enabled)
{
    m_enableLINE = enabled;
}

void XsLog::setEnableFUNCTION(bool enabled)
{
    m_enableFUNTION = enabled;
}

void XsLog::setEnableColorPrint(bool enabled)
{
    m_enableColorPrint = enabled;
}

void XsLog::setTimeFormat(const QString &fmt)
{
    m_timeFormat = fmt;
}

void XsLog::setFilePath(const QString &path)
{
    m_filePath = path;
}

void XsLog::verbose(const char *file, quint16 line, const char *function
                     , const char *tag, const char *fmt, ...)
{
    if (m_logLevel > MLogLevel::Verbose) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    log(MLogLevel::Verbose, file, line, function, tag, fmt, ap);
    va_end(ap);
}

void XsLog::info(const char *file, quint16 line, const char *function
                  , const char *tag, const char *fmt, ...)
{
    if (m_logLevel > MLogLevel::Info) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    log(MLogLevel::Info, file, line, function, tag, fmt, ap);
    va_end(ap);
}

void XsLog::trace(const char *file, quint16 line, const char *function
                   , const char *tag, const char *fmt, ...)
{
    if (m_logLevel > MLogLevel::Trace) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    log(MLogLevel::Trace, file, line, function, tag, fmt, ap);
    va_end(ap);
}

void XsLog::warn(const char *file, quint16 line, const char *function
                  , const char *tag, const char *fmt, ...)
{
    if (m_logLevel > MLogLevel::Warn) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    log(MLogLevel::Warn, file, line, function, tag, fmt, ap);
    va_end(ap);
}

void XsLog::error(const char *file, quint16 line, const char *function
                   , const char *tag, const char *fmt, ...)
{
    if (m_logLevel > MLogLevel::Error) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    log(MLogLevel::Error, file, line, function, tag, fmt, ap);
    va_end(ap);
}

void XsLog::log(int level, const char *file, quint16 line, const char *function, const char *tag, const char *fmt, va_list ap)
{
    QMutexLocker locker(&m_mutex);

    const char *p;

    switch (level) {
    case MLogLevel::Verbose:
        p = "verbose";
        break;
    case MLogLevel::Info:
        p = "info";
        break;
    case MLogLevel::Trace:
        p = "trace";
        break;
    case MLogLevel::Warn:
        p = "warn";
        break;
    case MLogLevel::Error:
        p = "error";
        break;
    default:
        p = "default";
        break;
    }

    QString time = QDateTime::currentDateTime().toString(m_timeFormat);
    int size = 0;

    size += qsnprintf(m_buffer+size, m_bufferLength-size, "[%s]", time.toStdString().c_str());
    size += qsnprintf(m_buffer+size, m_bufferLength-size, "[%s]", p);

    if (m_enablFILE) {
        size += qsnprintf(m_buffer+size, m_bufferLength-size, "[%s]", file);
    }

    if (m_enableLINE) {
        size += qsnprintf(m_buffer+size, m_bufferLength-size, "[%d]", line);
    }

    if (m_enableFUNTION) {
        size += qsnprintf(m_buffer+size, m_bufferLength-size, "[%s]", function);
    }

    if (tag) {
        size += qsnprintf(m_buffer+size, m_bufferLength-size, "[%s]", tag);
    }

    size += vsnprintf(m_buffer+size, m_bufferLength-size, fmt, ap);

    size += qsnprintf(m_buffer+size, m_bufferLength-size, "\r\n");

    // log to console
    if (m_log2Console && m_enableColorPrint) {
        if (level <= MLogLevel::Trace) {
                  printf("%s", m_buffer);
              } else if (level == MLogLevel::Warn) {
                  printf("%s", m_buffer);
              } else if (level == MLogLevel::Error){
                  printf("%s", m_buffer);
              }
    } else if (m_log2Console && !m_enableColorPrint) {
        printf("%s", m_buffer);
    }

    QByteArray data(m_buffer, size);
    // log to file
    if (m_log2File) {
        m_file->write(data.data(), data.length());
        m_file->flush();
    }

    fflush(stdout);
}

XsLog *g_logCtx = new XsLog;
