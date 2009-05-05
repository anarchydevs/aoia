#include "Logger.h"
#include <QDir>
#include <QTime>

#define LB "\r\n"

namespace aoia {

    SINGLETON_IMPL(Logger);


    Logger::Logger()
        : m_enabled(false)
    {
    }


    Logger::Logger(const Logger&)
    {
    }


    Logger::~Logger()
    {
    }


    void Logger::init(QString const& filename, QString const& versioninfo)
    {
        m_enabled = !filename.isEmpty();
        if (m_enabled)
        {
            m_file.setFileName(filename);
            m_file.open(QFile::WriteOnly | QFile::Truncate);
            m_out.setDevice(&m_file);

            log("****************************************");
            log(QString("Logging Started %1 %2").arg(QDate::currentDate().toString()).arg(QTime::currentTime().toString()));
            log(QString("Version: %1").arg(versioninfo));
            log(QString("Current Directory: %1").arg(QDir::currentPath()));
            log("****************************************");
        }
    }


    void Logger::destroy()
    {
        log("****************************************");
        log("Logging Terminated");
        log("****************************************");

        m_out.setDevice(NULL);
        m_file.close();
        m_enabled = false;
    }


    void Logger::log(QString const& msg)
    {
        if (m_enabled && m_out.status() == QTextStream::Ok)
        {
            m_out << msg << LB << flush;
        }
    }

}   // namespace
