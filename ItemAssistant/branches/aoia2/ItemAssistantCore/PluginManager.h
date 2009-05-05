#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <ItemAssistantCore/Singleton.h>
#include <ItemAssistantCore/itemassistantcore_global.h>
#include <vector>


namespace aoia {

    class ITEMASSISTANTCORE_EXPORT PluginManager
    {
        SINGLETON(PluginManager);
    public:
        ~PluginManager();

        void loadPlugins();

    private:
    };

}   // namespace

#endif // PLUGINMANAGER_H
