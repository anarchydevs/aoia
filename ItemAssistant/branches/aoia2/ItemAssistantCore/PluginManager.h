#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <ItemAssistantCore/PluginInterface.h>
#include <ItemAssistantCore/Singleton.h>
#include <ItemAssistantCore/itemassistantcore_global.h>
#include <QVector>


namespace aoia {

    class ITEMASSISTANTCORE_EXPORT PluginManager
    {
        SINGLETON(PluginManager);
    public:
        ~PluginManager();

        void loadPlugins();
        void initPlugins();

    private:
        QVector<PluginInterface*> m_plugins;
    };

}   // namespace

#endif // PLUGINMANAGER_H
