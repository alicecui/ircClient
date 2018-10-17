#ifndef STCLOG_HPP
#define STCLOG_HPP

#include <stdarg.h>
#include <QFile>
#include <QMutex>

class MLogLevel
{
public:
    static const int Verbose    = 0x01;
    static const int Info       = 0x02;
    static const int Trace      = 0x04;
    static const int Warn       = 0x08;
    static const int Error      = 0x0F;
};

class XsLog
{
public:
    XsLog();
    ~XsLog();

    void setLogLevel(int level);

    void setEnableCache(bool enabled);
    void setLog2Console(bool enabled);
    bool setLog2File(bool enabled, QString &err);
    void setEnableFILE(bool enabled);
    void setEnableLINE(bool enabled);
    void setEnableFUNCTION(bool enabled);
    void setEnableColorPrint(bool enabled);

    void setTimeFormat(const QString &fmt);
    void setFilePath(const QString &path);

    virtual void verbose(const char *file, quint16 line, const char *function, const char *tag, const char* fmt, ...);
    virtual void info(const char *file, quint16 line, const char *function, const char *tag,  const char* fmt, ...);
    virtual void trace(const char *file, quint16 line, const char *function, const char *tag,  const char* fmt, ...);
    virtual void warn(const char *file, quint16 line, const char *function, const char *tag,  const char* fmt, ...);
    virtual void error(const char *file, quint16 line, const char *function, const char *tag,  const char* fmt, ...);

private:
    void log(int level, const char *file, quint16 line, const char *function, const char *tag, const char* fmt, va_list ap);

private:
    int m_logLevel;
    bool m_enableCache;
    bool m_log2Console;
    bool m_log2File;
    bool m_enablFILE;
    bool m_enableLINE;
    bool m_enableFUNTION;
    bool m_enableColorPrint;
    QFile *m_file;
    char *m_buffer;
    quint32 m_bufferLength;

    QString m_timeFormat;
    QString m_filePath;

    QMutex m_mutex;
};

extern XsLog *g_logCtx;



#define xs_log_verbose(msg, ...) \
    g_logCtx->verbose(__FILE__, __LINE__, __FUNCTION__, NULL, msg, ##__VA_ARGS__)

#define xs_log_info(msg, ...) \
    g_logCtx->info(__FILE__, __LINE__, __FUNCTION__, NULL, msg, ##__VA_ARGS__)

#define xs_log_trace(msg, ...) \
    g_logCtx->trace(__FILE__, __LINE__, __FUNCTION__, NULL, msg, ##__VA_ARGS__)

#define xs_log_warn(msg, ...) \
    g_logCtx->warn(__FILE__, __LINE__, __FUNCTION__, NULL, msg, ##__VA_ARGS__)

#define xs_log_error(msg, ...) \
    g_logCtx->error(__FILE__, __LINE__, __FUNCTION__, NULL, msg, ##__VA_ARGS__)



#endif // STCLOG_HPP
