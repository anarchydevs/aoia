#ifndef AOMANAGER_H
#define AOMANAGER_H

#include <QString>
#include <ItemAssistantCore/itemassistantcore_global.h>
#include <ItemAssistantCore/Singleton.h>
#include <vector>
#include <exception>


class ITEMASSISTANTCORE_EXPORT AOManager
{
    SINGLETON(AOManager);
public:
    ~AOManager();

    struct ITEMASSISTANTCORE_EXPORT AOManagerException : public std::exception {
        AOManagerException(QString const& message) : std::exception(message.toStdString().c_str()) {}
    };

    QString getAOFolder() const;
    bool createAOItemsDB(QString const& localfile, bool showProgress = true);
    QString getCustomBackpackName(unsigned int charid, unsigned int containerid) const;
    std::vector<QString> getAccountNames() const;

private:
    mutable QString m_aofolder;
};

#endif // AOMANAGER_H
