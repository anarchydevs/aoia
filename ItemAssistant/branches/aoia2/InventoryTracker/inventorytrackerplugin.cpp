#include "inventorytrackerplugin.h"
#include <QtPlugin>
#include "inventorybrowserplugin.h"
#include <ItemAssistantCore/Logger.h>
#include <ItemAssistantCore/guimanager.h>
#include "inventorylistview.h"
#include <QTreeView>


namespace aoia { namespace inventory {

    Q_EXPORT_PLUGIN2(inventorytracker, InventoryTrackerPlugin);

    InventoryTrackerPlugin::InventoryTrackerPlugin()
    {
        LOG("Inventory Tracker Plugin loaded");
    }


    InventoryTrackerPlugin::~InventoryTrackerPlugin()
    {
    }


    void InventoryTrackerPlugin::initPlugin()
    {
        LOG("Inventory Tracker Plugin initialized");
        GuiManager::instance().extendGui(new InventoryListView(), new QTreeView(), "Inventory");
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
