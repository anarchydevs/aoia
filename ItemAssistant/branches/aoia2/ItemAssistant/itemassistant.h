#ifndef ITEMASSISTANT_H
#define ITEMASSISTANT_H

#include <QtGui/QMainWindow>
#include "ui_itemassistant.h"
#include "PluginInterface.h"


namespace aoia {

    class ItemAssistant : public QMainWindow
    {
        Q_OBJECT

    public:
        ItemAssistant(QWidget *parent = 0, Qt::WFlags flags = 0);
        ~ItemAssistant();

    private:
        void loadPlugins();
        void registerPlugin(PluginInterface* plugin);

        Ui::ItemAssistantClass ui;
    };

}   // namespace

#endif // ITEMASSISTANT_H
