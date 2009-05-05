#include "datamanager.h"

namespace aoia {

    DataManager::DataManager(QObject *parent)
        : QObject(parent)
    {

    }

    DataManager::~DataManager()
    {

    }


    bool DataManager::init(QString dbfile)
    {
        return false;
    }


    void DataManager::insertItem(unsigned int keylow, unsigned int keyhigh, unsigned short ql, 
        unsigned short flags, unsigned short stack, unsigned int parent, unsigned short slot, 
        unsigned int children, unsigned int owner)
    {

    }


    OwnedItemInfo DataManager::getOwnedItemInfo(unsigned int itemID)
    {
        return OwnedItemInfo();
    }


    void DataManager::setToonName(unsigned int charid, QString const& newName)
    {

    }


    QString DataManager::getToonName(unsigned int charid) const
    {
        return "";
    }


    void DataManager::setToonShopId(unsigned int charid, unsigned int shopid)
    {

    }


    unsigned int DataManager::getToonShopId(unsigned int charid) const
    {
        return 0;
    }


    unsigned int DataManager::getShopOwner(unsigned int shopid)
    {
        return 0;
    }


    void DataManager::setToonDimension(unsigned int charid, unsigned int dimensionid)
    {

    }


    unsigned int DataManager::getToonDimension(unsigned int charid) const
    {
        return 0;
    }


    bool DataManager::getDimensions(QMap<unsigned int, QString> &dimensions) const
    {
        return false;
    }


    unsigned int DataManager::findNextAvailableContainerSlot(unsigned int charId, unsigned int containerId)
    {
        return 0;
    }


    unsigned int DataManager::getItemProperties(unsigned int charId, unsigned int containerId, 
        unsigned int slot)
    {
        return 0;
    }


    unsigned int DataManager::findFirstItemOfSameType(unsigned int charId, unsigned int containerId, 
        unsigned int slot, unsigned int containerIdToSearchIn)
    {
        return 0;
    }


    void DataManager::lock()
    {

    }


    void DataManager::unLock()
    {

    }

}   // namespace
