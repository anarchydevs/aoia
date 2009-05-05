#ifndef DATAMANAGERINTERFACE_H
#define DATAMANAGERINTERFACE_H

#include <QString>

namespace aoia {

    struct OwnedItemInfo
    {
        QString itemloid;
        QString itemhiid;
        QString itemql;
        QString itemname;
        QString ownername;
        QString ownerid;
        QString containername;
        QString containerid;
        unsigned short flags;
    };

    struct DataManagerInterface
    {
        virtual ~DataManagerInterface() {}

        /// Records an item with the specified properties as owned by a the character.
        virtual void insertItem(unsigned int keylow, unsigned int keyhigh, unsigned short ql, 
            unsigned short flags, unsigned short stack, unsigned int parent, unsigned short slot, 
            unsigned int children, unsigned int owner) = 0;

        /// Retrieve all known info about the item with the specified index.
        virtual OwnedItemInfo getOwnedItemInfo(unsigned int itemID) = 0;

        /// Store the specified name for the character ID
        virtual void setToonName(unsigned int charid, QString const& newName) = 0;

        /// Retrieve the stored name of a specified character ID
        virtual QString getToonName(unsigned int charid) const = 0;

        /// Records the specified shop ID for the character.
        virtual void setToonShopId(unsigned int charid, unsigned int shopid) = 0;

        /// Retrieves the recorded shop ID for the specified character.
        virtual unsigned int getToonShopId(unsigned int charid) const = 0;

        /// Retrieves the recorded (if any) character ID associated with the specified shop ID.
        virtual unsigned int getShopOwner(unsigned int shopid) = 0;

        /// Assign a dimension ID to a specified character.
        virtual void setToonDimension(unsigned int charid, unsigned int dimensionid) = 0;

        /// Retrieves the dimension a character belongs to. 0 means dimension is unknown.
        virtual unsigned int getToonDimension(unsigned int charid) const = 0;

        /// Retrieves all the dimension IDs and their descriptions. Returns false if query failed.
        virtual bool getDimensions(std::map<unsigned int, QString> &dimensions) const = 0;

        /// Determines the first available container slot ID for a specified character and container.
        virtual unsigned int findNextAvailableContainerSlot(unsigned int charId, unsigned int containerId) = 0;

        /// Returns the properties value in the AO db for an item in a particular slot in a container.
        virtual unsigned int getItemProperties(unsigned int charId, unsigned int containerId, 
            unsigned int slot) = 0;

        /// Searches for items in containerIdToSearchIn with the same keylow and ql as the item specified
        virtual unsigned int findFirstItemOfSameType(unsigned int charId, unsigned int containerId, 
            unsigned int slot, unsigned int containerIdToSearchIn) = 0;

        /// Lock database for access.
        virtual void lock() = 0;

        /// Release database lock.
        virtual void unLock() = 0;
    };

}   // namespace

#endif // DATAMANAGERINTERFACE_H
