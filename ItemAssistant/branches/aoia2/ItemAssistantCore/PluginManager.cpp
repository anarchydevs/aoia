#include "PluginManager.h"
#include <QPluginLoader>
#include <QDir>
#include <QApplication>
#include "Logger.h"


namespace aoia {

    SINGLETON_IMPL(PluginManager);


    PluginManager::PluginManager()
    {
        LOG("PluginManager: created");
    }


    PluginManager::PluginManager(const PluginManager&)
    {
    }


    PluginManager::~PluginManager()
    {
        LOG("PluginManager: destroyed");
    }


    void PluginManager::loadPlugins()
    {
        foreach (QObject *plugin, QPluginLoader::staticInstances())
        {
            PluginInterface* aoiaPlugin = qobject_cast<PluginInterface*>(plugin);
            if (!aoiaPlugin)
            {
                continue;
            }
            m_plugins += aoiaPlugin;
        }

        QDir pluginsDir(qApp->applicationDirPath());
        pluginsDir.cd("plugins");

        foreach (QString fileName, pluginsDir.entryList(QDir::Files))
        {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            QObject *plugin = loader.instance();
            if (!plugin)
            {
                continue;
            }
            PluginInterface* aoiaPlugin = qobject_cast<PluginInterface*>(plugin);
            if (!aoiaPlugin)
            {
                continue;
            }
            m_plugins += aoiaPlugin;
        }
        LOG(QString("PluginManager: %1 plugins loaded").arg(m_plugins.size()));
    }


    void PluginManager::initPlugins()
    {
        LOG("PluginManager: initializing plugins");
        foreach(PluginInterface* plugin, m_plugins)
        {
            plugin->initPlugin();
        }
    }

}   // namespace
