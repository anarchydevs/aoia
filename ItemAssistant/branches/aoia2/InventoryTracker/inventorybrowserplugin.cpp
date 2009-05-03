#include "inventorybrowserplugin.h"
#include "inventorylistview.h"
#include <QtPlugin>
#include <QTreeView>


namespace aoia { namespace inventory {
    

    InventoryBrowserPlugin::InventoryBrowserPlugin()
        : QObject()
    {
    }


    InventoryBrowserPlugin::~InventoryBrowserPlugin()
    {
    }


    QWidget* InventoryBrowserPlugin::getMainWidget(QWidget* parent)
    {
        return new InventoryListView(parent);
    }


    QWidget* InventoryBrowserPlugin::getSidebarWidget(QWidget *parent)
    {
        return new QTreeView(parent);
    }


    void InventoryBrowserPlugin::activate()
    {
    }


    void InventoryBrowserPlugin::deactivate()
    {
    }

}}  // namespace
