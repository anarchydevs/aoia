#include "AOManager.h"
#include <QDir>


SINGLETON_IMPL(AOManager);


AOManager::AOManager()
{
}


AOManager::AOManager(const AOManager&)
{
}


AOManager::~AOManager()
{
}


QString AOManager::getAOFolder() const
{
    //if (m_aofolder.isEmpty())
    //{
    //    bfs::tpath AODir;
    //    bool requestFolder = true;

    //    // Get AO folder from registry
    //    ATL::CRegKey regKey;
    //    if (regKey.Open(HKEY_CURRENT_USER, _T("Software\\AOItemAssistant"), KEY_ALL_ACCESS) == ERROR_SUCCESS)
    //    {
    //        TCHAR ao_dir[MAX_PATH];
    //        ULONG ao_dir_size = MAX_PATH;
    //        ZeroMemory(ao_dir, MAX_PATH * sizeof(TCHAR));

    //        if (regKey.QueryStringValue(_T("AOPath"), ao_dir, &ao_dir_size) == ERROR_SUCCESS)
    //        {
    //            AODir = QString(ao_dir);
    //            if (bfs::exists(AODir / _T("anarchy.exe"))) {
    //                requestFolder = false;
    //            }
    //        }
    //        regKey.Close();
    //    }
    //    else
    //    {
    //        if (regKey.Create(HKEY_CURRENT_USER, _T("Software\\AOItemAssistant")) == ERROR_SUCCESS)
    //        {
    //            regKey.Close();
    //        }
    //    }

    //    if (requestFolder)
    //    {
    //        AODir = BrowseForFolder(NULL, _T("Please locate the AO directory:"));
    //        if (AODir.empty()) {
    //            return _T("");
    //        }

    //        if (!bfs::exists(AODir / _T("anarchy.exe"))) {
    //            MessageBox( NULL, _T("This is not AO's directory."), _T("ERROR"), MB_OK | MB_ICONERROR);
    //            return _T("");
    //        }

    //        if (regKey.Open(HKEY_CURRENT_USER, _T("Software\\AOItemAssistant"), KEY_ALL_ACCESS) == ERROR_SUCCESS)
    //        {
    //            regKey.SetStringValue(_T("AOPath"), AODir.string().c_str());
    //            regKey.Close();
    //        }
    //    }

    //    m_aofolder = AODir.string();
    //}

    return m_aofolder;
}


bool AOManager::createAOItemsDB(QString const& localfile, bool showProgress)
{
    return false;
}


QString AOManager::getCustomBackpackName(unsigned int charid, unsigned int containerid) const
{
    return "";
}


std::vector<QString> AOManager::getAccountNames() const
{
    std::vector<QString> result;

    //bfs::path path(to_ascii_copy(getAOFolder()));
    //path = path / "Prefs";

    //if (bfs::exists(path)) {
    //    bfs::directory_iterator end_itr; // default construction yields past-the-end
    //    for (bfs::directory_iterator itr(path); itr != end_itr; ++itr ) {
    //        if (bfs::is_directory(itr->status())) {
    //            result.push_back(from_ascii_copy(itr->leaf()));
    //        }
    //    }
    //}

    return result;
}
