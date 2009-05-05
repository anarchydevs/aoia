#ifndef LOGGER_H
#define LOGGER_H

#include <ItemAssistantCore/Singleton.h>
#include <ItemAssistantCore/itemassistantcore_global.h>
#include <QTextStream>
#include <QString>
#include <QFile>
#include <Windows.h>


namespace aoia {

    class ITEMASSISTANTCORE_EXPORT Logger
    {
        SINGLETON(Logger);
    public:
        ~Logger();

        void init(QString const& filename, QString const& versioninfo);
        void destroy();
        void log(QString const& msg);

    private:
        QFile m_file;
        QTextStream m_out;
        bool m_enabled;
    };

}   // namespace


#ifdef DEBUG

#define TRACE(streamdef) \
{ \
    QString msg; \
    QTextStream stream(&msg); \
    stream << (streamdef) << "\n"; \
    stream.flush(); \
}
//    OutputDebugString(msg.toStdWString().c_str()); \
//}

#define LOG(streamdef) \
{ \
    QString msg; \
    QTextStream stream(&msg); \
    stream << (streamdef); \
    stream.flush(); \
    Logger::instance().log(msg); \
}
//    msg += "\n"; \
//    OutputDebugString(msg.toStdWString().c_str()); \
//}

#else

#define TRACE(streamdef) \
    __noop;

#define LOG(streamdef) \
{ \
    QString msg; \
    QTextStream stream(&msg); \
    stream << (streamdef); \
    stream.flush(); \
    Logger::instance().log(msg); \
}

#endif

#endif // LOGGER_H
