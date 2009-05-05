#include "inventorytrackerplugin.h"
#include <QtPlugin>
#include "inventorybrowserplugin.h"


namespace aoia { namespace inventory {

    Q_EXPORT_PLUGIN2(inventorytracker, InventoryTrackerPlugin);

    InventoryTrackerPlugin::InventoryTrackerPlugin()
    {
    }


    InventoryTrackerPlugin::~InventoryTrackerPlugin()
    {
    }


    void InventoryTrackerPlugin::initPlugin()
    {
    }


    QList<MessageHandlerInterface*> InventoryTrackerPlugin::getMessageHandlers() const
    {
        QList<MessageHandlerInterface*> handlers;

        return handlers;
    }


    QList<GuiPluginInterface*> InventoryTrackerPlugin::getGuiPlugins() const
    {
        QList<GuiPluginInterface*> plugins;

        plugins += new InventoryBrowserPlugin();

        return plugins;
    }

}}  // namespace
