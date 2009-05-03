#ifndef INVENTORYTRACKERPLUGIN_H
#define INVENTORYTRACKERPLUGIN_H

#include "inventorytracker_global.h"
#include <QObject>
#include <ItemAssistant/PluginInterface.h>


namespace aoia { namespace inventory {

    class InventoryTrackerPlugin
        : public QObject
        , public PluginInterface
    {
        Q_OBJECT;
        Q_INTERFACES(aoia::PluginInterface);

    public:
        InventoryTrackerPlugin();
        ~InventoryTrackerPlugin();

        virtual void initPlugin(DataManagerInterface* dbManager);
        virtual QList<MessageHandlerInterface*> getMessageHandlers() const;
        virtual QList<GuiPluginInterface*> getGuiPlugins() const;
    };

}}  // namespace

#endif // INVENTORYTRACKERPLUGIN_H
