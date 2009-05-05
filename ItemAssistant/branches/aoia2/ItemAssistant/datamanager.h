#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QString>
#include "DataManagerInterface.h"

namespace aoia {

    class DataManager
        : public QObject
        , public DataManagerInterface
    {
        Q_OBJECT

    public:
        DataManager(QObject *parent);
        ~DataManager();

        bool init(QString dbfile);

        // Implementation of DataManagerInterface
        virtual void insertItem(unsigned int keylow, unsigned int keyhigh, unsigned short ql, 
            unsigned short flags, unsigned short stack, unsigned int parent, unsigned short slot, 
            unsigned int children, unsigned int owner);
        virtual OwnedItemInfo getOwnedItemInfo(unsigned int itemID);
        virtual void setToonName(unsigned int charid, Qstring const& newName);
        virtual Qstring getToonName(unsigned int charid) const;
        virtual void setToonShopId(unsigned int charid, unsigned int shopid);
        virtual unsigned int getToonShopId(unsigned int charid) const;
        virtual unsigned int getShopOwner(unsigned int shopid);
        virtual void setToonDimension(unsigned int charid, unsigned int dimensionid);
        virtual unsigned int getToonDimension(unsigned int charid) const;
        virtual bool getDimensions(std::map<unsigned int, std::tstring> &dimensions) const;
        virtual unsigned int findNextAvailableContainerSlot(unsigned int charId, unsigned int containerId);
        virtual unsigned int getItemProperties(unsigned int charId, unsigned int containerId, 
            unsigned int slot);
        virtual unsigned int findFirstItemOfSameType(unsigned int charId, unsigned int containerId, 
            unsigned int slot, unsigned int containerIdToSearchIn);
        virtual void lock();
        virtual void unLock();
    };

}   // namespace

#endif // DATAMANAGER_H
