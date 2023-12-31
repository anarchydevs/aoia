#include "StdAfx.h"
#include "dbmanager.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <shared/AODatabaseParser.h>
#include <shared/AODatabaseWriter.h>
#include <shared/FileUtils.h>
#include <sstream>
#include "ProgressDialog.h"
#include <ItemAssistantCore/AOManager.h>

namespace bfs = boost::filesystem;

#define CURRENT_DB_VERSION 4

/*********************************************************/
/* DB Manager Implementation                             */
/*********************************************************/


DBManager::DBManager(void)
{
}


DBManager::~DBManager(void)
{
}


bool DBManager::init(std::tstring dbfile)
{
    std::tstring aofolder = AOManager::instance().getAOFolder();
    if (aofolder.empty()) {
        return false;
    }

    if (dbfile.empty()) {
        dbfile = _T("ItemAssistant.db");
    }

    bool dbfileExists = bfs::exists(bfs::tpath(dbfile));

    if (!SQLite::Db::Init(dbfile)) {
        return false;
    }

    if (!dbfileExists) {
        createDBScheme();
    }

    unsigned int dbVersion = getDBVersion();
    if (dbVersion < CURRENT_DB_VERSION) {
        if (IDOK != MessageBox( NULL, _T("AO Item Assistant needs to update its database file to a newer version."), 
            _T("Question - AO Item Assistant"), MB_OKCANCEL | MB_ICONQUESTION))
        {
            return false;
        }
        updateDBVersion(dbVersion);
    }
    else if (dbVersion > CURRENT_DB_VERSION)
    {
        MessageBox( NULL, _T("AO Item Assistant has detected a too new version of its database file. You should upgrade the software to continue."),
            _T("Error - AO Item Assistant"), MB_OK | MB_ICONERROR);
        return false;
    }

    if (!syncLocalItemsDB(_T("aoitems.db"), aofolder)) {
        MessageBox( NULL, _T("AO Item Assistant can not start without a valid item database."),
            _T("Error - AO Item Assistant"), MB_OK | MB_ICONERROR);
        return false;
    }

    Exec(_T("ATTACH DATABASE \"aoitems.db\" AS aodb"));

    return true;
}


void DBManager::destroy()
{
    SQLite::Db::Term();
}


/** 
 * Check to see if we have a local database already.
 * If it is, then check if it is up to date.
 * If local database is missing or obsolete then recreate it.
 * Return true if application has a local items database to run with, false otherwise.
*/
bool DBManager::syncLocalItemsDB(std::tstring const& localfile, std::tstring const& aofolder)
{
    bool hasLocalDB = false;
    std::time_t lastUpdateTime;

    bfs::path local(to_ascii_copy(localfile));
    bfs::path original(to_ascii_copy(aofolder));
    original = original / "cd_image/data/db/ResourceDatabase.dat";
    
    if (bfs::exists(local) && bfs::is_regular(local)) {
        hasLocalDB = true;
        lastUpdateTime = bfs::last_write_time(local);
    }

    if (!exists(original)) {
        Logger::instance().log(_T("Could not locate the original AO database."));
        return hasLocalDB;
    }

    if (hasLocalDB && getAODBSchemeVersion(localfile) == 2) {
        std::time_t lastOriginalUpdateTime = bfs::last_write_time(original);
        if (lastOriginalUpdateTime <= lastUpdateTime) {
                return true;
        }

        // Ask user if he wants to continue using the old (but compatible) DB or update it now.
        int answer = ::MessageBox(NULL, 
            _T("You items database is out of date. Do you wish to update it now?\r\nAnswering 'NO' will continue using the old one."),
            _T("Question - AO Item Assistant"), MB_ICONQUESTION | MB_YESNOCANCEL);
        if (answer == IDCANCEL) {
            exit(0);
        }
        else if (answer == IDNO) {
            return true;
        }
    }

    // If we come this far we need to update the DB.

    bfs::path tmpfile("tmp_" + local.string());
    bfs::remove(tmpfile);

    try {
        AODatabaseParser aodb(original.string());
        AODatabaseWriter writer(tmpfile.string());

        unsigned int itemCount = aodb.GetItemCount(AODB_TYP_ITEM);
        unsigned int nanoCount = aodb.GetItemCount(AODB_TYP_NANO);

        CProgressDialog dlg(itemCount + nanoCount, itemCount);
        dlg.SetWindowText(_T("Progress Dialog - Item Assistant"));
        dlg.setText(0, _T("Extracting data from the AO DB..."));
        dlg.setText(1, STREAM2STR("Finished " << 0 << " out of " << itemCount << " items."));
        dlg.setText(2, _T("Overall progress: 0%"));

        // Extract items
        boost::shared_ptr<ao_item> item = aodb.GetFirstItem(AODB_TYP_ITEM);
        if (item) {
            unsigned int count = 1;
            writer.BeginWrite();
            writer.WriteItem(item);
            while (item = aodb.GetNextItem()) { 
                ++count; 
                writer.WriteItem(item);
                if (count % 1000 == 0) {
                    if (dlg.userCanceled()) {
                        return false;
                    }
                    dlg.setTaskProgress(count, itemCount);
                    dlg.setText(1, STREAM2STR("Finished " << count << " out of " << itemCount << " items."));
                    dlg.setOverallProgress(count, itemCount + nanoCount);
                    dlg.setText(2, STREAM2STR("Overall progress: " << (count * 100) / (itemCount + nanoCount) << "%"));
                }
                if (count % 10000 == 0) {
                    writer.CommitItems();
                    writer.BeginWrite();
                }
            }
            dlg.setTaskProgress(count, itemCount);
            dlg.setText(1, STREAM2STR("Finished " << count << " out of " << itemCount << " items."));
            dlg.setOverallProgress(count, itemCount + nanoCount);
            dlg.setText(2, STREAM2STR("Overall progress: " << (count * 100) / (itemCount + nanoCount) << "%"));
            writer.CommitItems();
        }

        if (dlg.userCanceled()) {
            return false;
        }
        //dlg.setTaskProgress(0, nanoCount);
        //dlg.setText(1, STREAM2STR("Finished " << 0 << " out of " << nanoCount << " nanos."));
        //dlg.setText(2, STREAM2STR("Overall progress: " << (itemCount * 100) / (itemCount + nanoCount) << "%"));
        //dlg.setOverallProgress(itemCount, itemCount + nanoCount);

        // Extract nano programs
        boost::shared_ptr<ao_item> nano = aodb.GetFirstItem(AODB_TYP_NANO);
        if (nano) {
            unsigned int count = 1;
            writer.BeginWrite();
            writer.WriteItem(nano);
            while (nano = aodb.GetNextItem()) { 
                ++count; 
                writer.WriteItem(nano);
                if (count % 1000 == 0) {
                    if (dlg.userCanceled()) {
                        return false;
                    }
                    dlg.setTaskProgress(count, nanoCount);
                    dlg.setText(1, STREAM2STR("Finished " << count << " out of " << nanoCount << " nanos."));
                    dlg.setOverallProgress(itemCount + count, itemCount + nanoCount);
                    dlg.setText(2, STREAM2STR("Overall progress: " << ((itemCount + count) * 100) / (itemCount + nanoCount) << "%"));
                }
                if (count % 10000 == 0) {
                    writer.CommitItems();
                    writer.BeginWrite();
                }
            }
            dlg.setTaskProgress(count, nanoCount);
            dlg.setText(1, STREAM2STR("Finished " << count << " out of " << nanoCount << " nanos."));
            dlg.setOverallProgress(itemCount + count, itemCount + nanoCount);
            dlg.setText(2, STREAM2STR("Overall progress: " << ((itemCount + count) * 100) / (itemCount + nanoCount) << "%"));
            writer.CommitItems();
        }

        if (dlg.userCanceled()) {
            return false;
        }
        //dlg.setTaskProgress(nanoCount, nanoCount);
        //dlg.setOverallProgress(itemCount + nanoCount, itemCount + nanoCount);
        //dlg.setText(1, STREAM2STR("Finished " << nanoCount << " out of " << nanoCount << " nanos."));
        //dlg.setText(2, _T("Overall progress: 100%"));

        writer.PostProcessData();
    }
    catch (std::bad_alloc &e) {
        assert(false);
        Logger::instance().log(STREAM2STR(_T("Error creating item database. ") << e.what()));
        return false;
    }
    catch (AODatabaseParser::CTreeDbException &e) {
        assert(false);
        Logger::instance().log(STREAM2STR(_T("Error parsing the AO Database. ") << e.what()));
        MessageBox( NULL, _T("Unable to parse the AO database.\n\rPlease make sure Anarchy Online is not running and try again."),
            _T("Error - AO Item Assistant"), MB_OK | MB_ICONERROR);
        return false;
    }
    catch (std::exception &e) {
        assert(false);
        Logger::instance().log(STREAM2STR(_T("Error creating item database. ") << e.what()));
        return false;
    }

    remove(local);
    rename(tmpfile, local);

    return true;
}


void DBManager::insertItem(unsigned int keylow,
                           unsigned int keyhigh,
                           unsigned short ql,
                           unsigned short flags,
                           unsigned short stack,
                           unsigned int parent,
                           unsigned short slot,
                           unsigned int children,
                           unsigned int owner)
{
    std::tstringstream sql;
    sql << _T("INSERT INTO tItems (keylow, keyhigh, ql, flags, stack, parent, slot, children, owner) VALUES (")
        << (unsigned int) keylow      << _T(", ")
        << (unsigned int) keyhigh     << _T(", ")
        << (unsigned int) ql          << _T(", ")
        << (unsigned int) flags       << _T(", ")
        << (unsigned int) stack       << _T(", ")
        << (unsigned int) parent      << _T(", ")
        << (unsigned int) slot        << _T(", ")
        << (unsigned int) children    << _T(", ")
        << (unsigned int) owner       << _T(")");
    Exec(sql.str());
}


std::tstring DBManager::getToonName(unsigned int charid) const
{
    std::tstring result;

    SQLite::TablePtr pT = g_DBManager.ExecTable(STREAM2STR("SELECT charid, charname FROM tToons WHERE charid = " << charid));

    if (pT != NULL && pT->Rows())
    {
        if (!pT->Data()[1].empty())
        {
            result = from_ascii_copy(pT->Data()[1]);
        }
    }

    return result;
}


void DBManager::setToonName(unsigned int charid, std::tstring const& newName)
{
    g_DBManager.Begin();

    boost::format sql("INSERT INTO tToons (charid, charname) VALUES (%1%, '%2%')");
    sql % charid % to_ascii_copy(newName);

    if (!g_DBManager.Exec(sql.str()))
    {
        // Insert failed, so update existing record instead.
        sql = boost::format("UPDATE OR IGNORE tToons SET charname='%1%' WHERE charid=%2%");
        sql % to_ascii_copy(newName) % charid;
        g_DBManager.Exec(sql.str());
    }

    g_DBManager.Commit();
}


void DBManager::setToonShopId(unsigned int charid, unsigned int shopid)
{
    assert(charid != 0);
    assert(shopid != 0);

    boost::format sql("UPDATE OR IGNORE tToons SET shopid=%1% WHERE charid=%2%");
    sql % shopid % charid;

    g_DBManager.Begin();
    g_DBManager.Exec(sql.str());
    g_DBManager.Commit();
}


unsigned int DBManager::getToonShopId(unsigned int charid) const
{
    assert(charid != 0);

    unsigned int result = 0;

    SQLite::TablePtr pT = g_DBManager.ExecTable(STREAM2STR("SELECT shopid FROM tToons WHERE charid = " << charid));
    if (pT != NULL && pT->Rows())
    {
        try
        {
            result = boost::lexical_cast<unsigned int>(pT->Data(0,0));
        }
        catch (boost::bad_lexical_cast &/*e*/)
        {
            // Wierd.. lets debug!
            assert(false);
        }
    }

    return result;
}


void DBManager::setToonDimension(unsigned int charid, unsigned int dimensionid)
{
    assert(charid != 0);
    assert(dimensionid != 0);
    g_DBManager.Begin();
    g_DBManager.Exec(STREAM2STR("UPDATE OR IGNORE tToons SET dimensionid = " << dimensionid << " WHERE charid = " << charid));
    g_DBManager.Commit();
}


unsigned int DBManager::getToonDimension(unsigned int charid) const
{
    assert(charid != 0);

    unsigned int result = 0;

    SQLite::TablePtr pT = g_DBManager.ExecTable(STREAM2STR("SELECT dimensionid FROM tToons WHERE charid = " << charid));
    if (pT != NULL && pT->Rows())
    {
        try
        {
            result = boost::lexical_cast<unsigned int>(pT->Data(0,0));
        }
        catch (boost::bad_lexical_cast &/*e*/)
        {
            // Wierd.. lets debug!
            assert(false);
        }
    }

    return result;
}


bool DBManager::getDimensions(std::map<unsigned int, std::tstring> &dimensions) const
{
    SQLite::TablePtr pT = g_DBManager.ExecTable(STREAM2STR("SELECT dimensionid, dimensionname FROM tDimensions"));

    if (pT != NULL && pT->Rows())
    {
        for (unsigned int i = 0; i < pT->Rows(); ++i)
        {
            std::tstring name = from_ascii_copy(pT->Data(i, 1));
            dimensions[boost::lexical_cast<unsigned int>(pT->Data(i, 0))] = name;
        }
        return true;
    }
    return false;
}


unsigned int DBManager::findNextAvailableContainerSlot(unsigned int charId, unsigned int containerId)
{
	assert(charId != 0);
	assert(containerId != 0);

	unsigned short posSlot = 0;

	if (containerId == 2)
		posSlot = 64; //start at slot 64 for inventory!

	SQLite::TablePtr pT = g_DBManager.ExecTable(STREAM2STR("SELECT slot FROM tItems WHERE parent = " << containerId << " AND slot >= " << posSlot
		<< " AND owner = " << charId << " ORDER by slot"));
	
	if (pT == NULL || !pT->Rows())
		return 0; //empty backpack

	unsigned int count = pT->Rows();

	for (unsigned i=0;i<count;i++)
	{
		try
		{
			if (posSlot < boost::lexical_cast<unsigned short>(pT->Data(i,0)))
			{
				return posSlot; //we found a free slot in-between
			}
		}
		catch (boost::bad_lexical_cast &/*e*/)
		{
			//return 0xff; //Can't really imagine this to be possible
		}

		posSlot++;
	}

	return posSlot; //we return the next available slot
}

//returns the properties value in the AO db for an item in a particular slot in a container.
unsigned int DBManager::getItemProperties(unsigned int charId, unsigned int containerId, unsigned int slot)
{
	assert(charId != 0);
	assert(containerId != 0);
	assert(slot >= 0);

    unsigned int result = 0;

	std::tstringstream sql;
    sql << _T("SELECT properties FROM tItems JOIN tblAO ON keylow = aoid WHERE owner = ") << charId
	    << _T(" AND parent = ") << containerId << _T(" AND slot = ") << slot;

	//OutputDebugString(sql.str().c_str());

    SQLite::TablePtr pT = ExecTable(sql.str());

    try
    {
	    if (pT != NULL && pT->Rows()) {
	        result = boost::lexical_cast<unsigned int>(pT->Data()[0]);
	    }
    }
    catch (boost::bad_lexical_cast &/*e*/)
    {
    }

    return result;
}
	
//searches for items in containerIdToSearchIn with the same keylow and ql as the item specified
unsigned int DBManager::findFirstItemOfSameType(unsigned int charId, unsigned int containerId, unsigned int slot, unsigned int containerIdToSearchIn)
{
	assert(charId != 0);
	assert(containerId != 0);
	assert(slot >= 0);
	assert(containerIdToSearchIn != 0);

    unsigned int result = 0;

	std::tstringstream sql;
    sql << _T("SELECT tTarget.slot FROM tItems tTarget, tItems tSource WHERE tSource.owner = ") << charId
	    << _T(" AND tSource.parent = ") << containerId 
		<< _T(" AND tSource.slot = ") << slot
		<< _T(" AND tTarget.keylow = tSource.keylow AND tTarget.ql = tSource.ql")
		<< _T(" AND tTarget.owner = ") << charId
		<< _T(" AND tTarget.parent = ") << containerIdToSearchIn
		<< _T(" ORDER BY tTarget.slot LIMIT 1");

	//OutputDebugString(sql.str().c_str());

    SQLite::TablePtr pT = ExecTable(sql.str());

    try
    {
	    if (pT != NULL && pT->Rows()) {
	        result = boost::lexical_cast<unsigned int>(pT->Data()[0]);
	    }
    }
    catch (boost::bad_lexical_cast &/*e*/)
    {
    }

    return result;
}



unsigned int DBManager::getShopOwner(unsigned int shopid)
{


    assert(shopid != 0);

    unsigned int result = 0;

    SQLite::TablePtr pT = g_DBManager.ExecTable(STREAM2STR("SELECT charid FROM tToons WHERE shopid = " << shopid));

    try
    {
	    if (pT != NULL && pT->Rows()) {
	        result = boost::lexical_cast<unsigned int>(pT->Data()[0]);
	    }
    }
    catch (boost::bad_lexical_cast &/*e*/)
    {
    }

    return result;
}


OwnedItemInfoPtr DBManager::getOwnedItemInfo(unsigned int itemID)
{
    OwnedItemInfoPtr pRetVal(new OwnedItemInfo());

    std::tstringstream sql;
    sql << _T("SELECT tItems.keylow AS itemloid, tItems.keyhigh AS itemhiid, tItems.ql AS itemql, name AS itemname, ")
        << _T("(SELECT tToons.charname FROM tToons WHERE tToons.charid = owner) AS ownername, owner AS ownerid, ")
        << _T("parent AS containerid, tItems.flags ")
        << _T("FROM tItems JOIN tblAO ON keylow = aoid WHERE itemidx = ") << (int)itemID;

    SQLite::TablePtr pT = ExecTable(sql.str());

    pRetVal->itemloid = from_ascii_copy(pT->Data(0, 0));
    pRetVal->itemhiid = from_ascii_copy(pT->Data(0, 1));
    pRetVal->itemql = from_ascii_copy(pT->Data(0, 2));
    pRetVal->itemname = from_ascii_copy(pT->Data(0, 3));
    pRetVal->ownername = from_ascii_copy(pT->Data(0, 4));
    pRetVal->ownerid = from_ascii_copy(pT->Data(0, 5));
    pRetVal->containerid = from_ascii_copy(pT->Data(0, 6));
    pRetVal->flags = boost::lexical_cast<unsigned short>(pT->Data(0, 7));
    unsigned int containerid = boost::lexical_cast<unsigned int>(pRetVal->containerid);
    unsigned int ownerid = boost::lexical_cast<unsigned int>(pRetVal->ownerid);
    pRetVal->containername = ServicesSingleton::Instance()->GetContainerName(ownerid, containerid);

    return pRetVal;
}


unsigned int DBManager::getAODBSchemeVersion(std::tstring const& filename) const
{
    unsigned int retval = 0;
    SQLite::Db db;

    if (db.Init(filename)) {
        try {
            SQLite::TablePtr pT = db.ExecTable(_T("SELECT Version FROM vSchemeVersion"));
            retval = boost::lexical_cast<unsigned int>(pT->Data(0,0));
        }
        catch(Db::QueryFailedException &/*e*/) {
            retval = 0;
        }
        catch (boost::bad_lexical_cast &/*e*/) {
            retval = 0;
        }
    }

    return retval;
}


unsigned int DBManager::getDBVersion() const
{
    unsigned int retval = 0;

    try {
        SQLite::TablePtr pT = ExecTable(_T("SELECT Version FROM vSchemeVersion"));
        retval = boost::lexical_cast<unsigned int>(pT->Data(0,0));
    }
    catch(Db::QueryFailedException &/*e*/) {
        retval = 0;
    }
    catch (boost::bad_lexical_cast &/*e*/) {
        retval = 0;
    }

    return retval;
}


void DBManager::updateDBVersion(unsigned int fromVersion) const
{
    switch (fromVersion)
    {
    case 0:
        {
            Begin();
            Exec(_T("CREATE TABLE tToons2 (charid, charname)"));
            Exec(_T("INSERT INTO tToons2 (charid, charname) SELECT charid, charname FROM tToons"));
            Exec(_T("DROP TABLE tToons"));
            Exec(_T("CREATE TABLE tToons (charid, charname)"));
            Exec(_T("CREATE UNIQUE INDEX iCharId ON tToons (charid)"));
            Exec(_T("INSERT INTO tToons (charid, charname) SELECT charid, charname FROM tToons2"));
            Exec(_T("DROP TABLE tToons2"));
            Exec(_T("CREATE VIEW vSchemeVersion AS SELECT '1' AS Version"));
            Commit();
        }
        // Dropthrough

    case 1:
        {
            Begin();
            Exec(_T("CREATE TABLE tToons2 (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR)"));
            Exec(_T("INSERT INTO tToons2 (charid, charname) SELECT charid, charname FROM tToons"));
            Exec(_T("DROP TABLE tToons"));
            Exec(_T("CREATE TABLE tToons (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR)"));
            Exec(_T("INSERT INTO tToons (charid, charname) SELECT charid, charname FROM tToons2"));
            Exec(_T("DROP TABLE tToons2"));
            Exec(_T("CREATE UNIQUE INDEX iCharId ON tToons (charid)"));
            Exec(_T("DROP VIEW vSchemeVersion"));
            Exec(_T("CREATE VIEW vSchemeVersion AS SELECT '2' AS Version"));
            Commit();
        }
        // Dropthrough

    case 2: // Update from v2 is the added shopid column in the toons table
        {
            Begin();
            Exec(_T("CREATE TABLE tToons2 (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0')"));
            Exec(_T("INSERT INTO tToons2 (charid, charname) SELECT charid, charname FROM tToons"));
            Exec(_T("DROP TABLE tToons"));
            Exec(_T("CREATE TABLE tToons (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0')"));
            Exec(_T("INSERT INTO tToons (charid, charname) SELECT charid, charname FROM tToons2"));
            Exec(_T("DROP TABLE tToons2"));
            Exec(_T("CREATE UNIQUE INDEX iCharId ON tToons (charid)"));
            Exec(_T("DROP VIEW vSchemeVersion"));
            Exec(_T("CREATE VIEW vSchemeVersion AS SELECT '3' AS Version"));
            Commit();
        }
        // Dropthrough

    case 3: // Update from v3 is the added flags column in tItems as well as the new dimension table and a new dimensionid column in tToons.
        {
            Begin();
            Exec(_T("CREATE TABLE tItems2 (itemidx INTEGER NOT NULL PRIMARY KEY ON CONFLICT REPLACE AUTOINCREMENT UNIQUE DEFAULT '1', keylow INTEGER, keyhigh INTEGER, ql INTEGER, flags INTEGER DEFAULT '0', stack INTEGER DEFAULT '1', parent INTEGER NOT NULL DEFAULT '2', slot INTEGER, children INTEGER, owner INTEGER NOT NULL)"));
            Exec(_T("INSERT INTO tItems2 (itemidx, keylow, keyhigh, ql, stack, parent, slot, children, owner) SELECT itemidx, keylow, keyhigh, ql, stack, parent, slot, children, owner FROM tItems"));
            Exec(_T("DROP TABLE tItems"));
            Exec(_T("CREATE TABLE tItems (itemidx INTEGER NOT NULL PRIMARY KEY ON CONFLICT REPLACE AUTOINCREMENT UNIQUE DEFAULT '1', keylow INTEGER, keyhigh INTEGER, ql INTEGER, flags INTEGER DEFAULT '0', stack INTEGER DEFAULT '1', parent INTEGER NOT NULL DEFAULT '2', slot INTEGER, children INTEGER, owner INTEGER NOT NULL)"));
            Exec(_T("INSERT INTO tItems (itemidx, keylow, keyhigh, ql, stack, parent, slot, children, owner) SELECT itemidx, keylow, keyhigh, ql, stack, parent, slot, children, owner FROM tItems2"));
            Exec(_T("DROP TABLE tItems2"));
            Exec(_T("CREATE TABLE tDimensions (dimensionid INTEGER NOT NULL PRIMARY KEY UNIQUE, dimensionname VARCHAR)"));
            Exec(_T("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (11, 'Atlantean (Rubi-Ka 1)')"));
            Exec(_T("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (12, 'Rimor (Rubi-Ka 2)')"));
            Exec(_T("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (13, 'Die Neue Welt (German Server)')"));
            Exec(_T("CREATE TABLE tToons2 (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0', dimensionid INTEGER DEFAULT '0')"));
            Exec(_T("INSERT INTO tToons2 (charid, charname, shopid) SELECT charid, charname, shopid FROM tToons"));
            Exec(_T("DROP TABLE tToons"));
            Exec(_T("CREATE TABLE tToons (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0', dimensionid INTEGER DEFAULT '0')"));
            Exec(_T("INSERT INTO tToons (charid, charname, shopid) SELECT charid, charname, shopid FROM tToons2"));
            Exec(_T("DROP TABLE tToons2"));
            Exec(_T("CREATE UNIQUE INDEX iCharId ON tToons (charid)"));
            Exec(_T("DROP VIEW vSchemeVersion"));
            Exec(_T("CREATE VIEW vSchemeVersion AS SELECT '4' AS Version"));
            Commit();
        }
        // Dropthrough

    default:
        break;
    }
}


void DBManager::createDBScheme() const
{
    Begin();
    Exec(_T("CREATE TABLE tItems (itemidx INTEGER NOT NULL PRIMARY KEY ON CONFLICT REPLACE AUTOINCREMENT UNIQUE DEFAULT '1', keylow INTEGER, keyhigh INTEGER, ql INTEGER, flags INTEGER DEFAULT '0', stack INTEGER DEFAULT '1', parent INTEGER NOT NULL DEFAULT '2', slot INTEGER, children INTEGER, owner INTEGER NOT NULL)"));
    Exec(_T("CREATE VIEW vBankItems AS SELECT * FROM tItems WHERE parent=1"));
    Exec(_T("CREATE VIEW vContainers AS SELECT * FROM tItems WHERE children > 0"));
    Exec(_T("CREATE VIEW vInvItems AS SELECT * FROM tItems WHERE parent=2"));
    Exec(_T("CREATE INDEX iOwner ON tItems (owner)"));
    Exec(_T("CREATE INDEX iParent ON tItems (parent)"));
    Exec(_T("CREATE TABLE tToons (charid INTEGER NOT NULL PRIMARY KEY UNIQUE, charname VARCHAR, shopid INTEGER DEFAULT '0', dimensionid INTEGER DEFAULT '0')"));
    Exec(_T("CREATE UNIQUE INDEX iCharId ON tToons (charid)"));
    Exec(_T("CREATE TABLE tDimensions (dimensionid INTEGER NOT NULL PRIMARY KEY UNIQUE, dimensionname VARCHAR)"));
    Exec(_T("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (11, 'Atlantean (Rubi-Ka 1)')"));
    Exec(_T("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (12, 'Rimor (Rubi-Ka 2)')"));
    Exec(_T("INSERT INTO tDimensions (dimensionid, dimensionname) VALUES (13, 'Die Neue Welt (German Server)')"));
    Exec(STREAM2STR(_T("CREATE VIEW vSchemeVersion AS SELECT '") << CURRENT_DB_VERSION << _T("' AS Version")));
    Commit();
}
