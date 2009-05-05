#ifndef INVENTORYBROWSERPLUGIN_H
#define INVENTORYBROWSERPLUGIN_H

#include <QObject>
#include <ItemAssistantCore/PluginInterface.h>

namespace aoia { namespace inventory {

    class InventoryBrowserPlugin
        : public QObject
        , public GuiPluginInterface
    {
        Q_OBJECT;
        Q_INTERFACES(aoia::GuiPluginInterface);

    public:
        InventoryBrowserPlugin();
        ~InventoryBrowserPlugin();

        virtual QString getLabel() const { return "Inventory Browser"; }
        virtual QWidget* getMainWidget(QWidget* parent);
        virtual QWidget* getSidebarWidget(QWidget* parent);
        virtual void activate();
        virtual void deactivate();

    private:
    };

}}  // namespace

#endif // INVENTORYBROWSERPLUGIN_H
