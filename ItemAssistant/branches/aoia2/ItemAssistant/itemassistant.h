#ifndef ITEMASSISTANT_H
#define ITEMASSISTANT_H

#include <QtGui/QMainWindow>
#include "ui_itemassistant.h"
#include "PluginInterface.h"
#include "datamanager.h"

namespace aoia {

    class ItemAssistant : public QMainWindow
    {
        Q_OBJECT

    public:
        ItemAssistant(QWidget *parent = 0, Qt::WFlags flags = 0);
        ~ItemAssistant();

    private:
        void loadPlugins();
        void initPlugin(PluginInterface* plugin);
        void registerPlugin(PluginInterface* plugin);

        Ui::ItemAssistantClass ui;
        DataManager* m_dataManager;
    };

}   // namespace

#endif // ITEMASSISTANT_H
