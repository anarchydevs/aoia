#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <shared/SQLite.h>
#include <shared/Mutex.h>
#include <vector>


struct OwnedItemInfo
{
    std::tstring itemloid;
    std::tstring itemhiid;
    std::tstring itemql;
    std::tstring itemname;
    std::tstring ownername;
    std::tstring ownerid;
    std::tstring containername;
    std::tstring containerid;
    unsigned short flags;
    //std::tstring inventoryname;
    //std::tstring inventoryid;
};

typedef boost::shared_ptr<OwnedItemInfo> OwnedItemInfoPtr;


class DBManager : public SQLite::Db
{
public:
    DBManager(void);
    virtual ~DBManager(void);

    bool init(std::tstring dbfile);
    void destroy();

    /// Records an item with the specified properties as owned by a the character.
    void insertItem(unsigned int keylow, unsigned int keyhigh, unsigned short ql, unsigned short flags, unsigned short stack, 
        unsigned int parent, unsigned short slot, unsigned int children, unsigned int owner);

    /// Retrieve all known info about the item with the specified index.
    OwnedItemInfoPtr getOwnedItemInfo(unsigned int itemID);

    /// Store the specified name for the character ID
    void setToonName(unsigned int charid, std::tstring const& newName);

    /// Retrieve the stored name of a specified character ID
    std::tstring getToonName(unsigned int charid) const;

    /// Records the specified shop ID for the character.
    void setToonShopId(unsigned int charid, unsigned int shopid);

    /// Retrieves the recorded shop ID for the specified character.
    unsigned int getToonShopId(unsigned int charid) const;

    /// Retrieves the recorded (if any) character ID associated with the specified shop ID.
    unsigned int getShopOwner(unsigned int shopid);

    /// Assign a dimension ID to a specified character.
    void setToonDimension(unsigned int charid, unsigned int dimensionid);

    /// Retrieves the dimension a character belongs to. 0 means dimension is unknown.
    unsigned int getToonDimension(unsigned int charid) const;

    /// Retrieves all the dimension IDs and their descriptions. Returns false if query failed.
    bool getDimensions(std::map<unsigned int, std::tstring> &dimensions) const;

    /// Determines the first available container slot ID for a specified character and container.
    unsigned int findNextAvailableContainerSlot(unsigned int charId, unsigned int containerId);

	//returns the properties value in the AO db for an item in a particular slot in a container.
	unsigned int getItemProperties(unsigned int charId, unsigned int containerId, unsigned int slot);
	
	//searches for items in containerIdToSearchIn with the same keylow and ql as the item specified
	unsigned int findFirstItemOfSameType(unsigned int charId, unsigned int containerId, unsigned int slot, unsigned int containerIdToSearchIn);
	

    /// Lock database for access.
    void lock() { m_mutex.MutexOn(); }

    /// Release database lock.
    void unLock() { m_mutex.MutexOff(); }

protected:
    bool syncLocalItemsDB(std::tstring const& localfile, std::tstring const& aofolder);

    unsigned int getAODBSchemeVersion(std::tstring const& filename) const;
    unsigned int getDBVersion() const;
    void updateDBVersion(unsigned int fromVersion) const;
    void createDBScheme() const;

private:
    Mutex m_mutex;
};

#endif // DBMANAGER_H
