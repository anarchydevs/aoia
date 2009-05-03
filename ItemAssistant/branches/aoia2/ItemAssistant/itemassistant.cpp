#include "itemassistant.h"
#include <QPluginLoader>
#include <QDir>
#include <QPushButton>
#include "PluginInterface.h"


namespace aoia {

    ItemAssistant::ItemAssistant(QWidget *parent, Qt::WFlags flags)
        : QMainWindow(parent, flags)
        , m_dataManager(new DataManager(parent))
    {
        ui.setupUi(this);

        // Clear out empty pages from the UI designer.
        while (ui.mainWidgetStack->count())
        {
            ui.mainWidgetStack->removeWidget(ui.mainWidgetStack->widget(0));
        }
        while (ui.sidebarWidgetStack->count())
        {
            ui.sidebarWidgetStack->removeWidget(ui.sidebarWidgetStack->widget(0));
        }

        loadPlugins();
    }


    ItemAssistant::~ItemAssistant()
    {
    }


    void ItemAssistant::loadPlugins()
    {
        foreach (QObject *plugin, QPluginLoader::staticInstances())
        {
            PluginInterface* aoiaPlugin = qobject_cast<PluginInterface*>(plugin);
            if (!aoiaPlugin)
            {
                continue;
            }
            registerPlugin(aoiaPlugin);
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
            initPlugin(aoiaPlugin);
        }
    }


    void ItemAssistant::initPlugin(PluginInterface* plugin)
    {
        plugin->initPlugin(m_dataManager);
        registerPlugin(plugin);
    }


    void ItemAssistant::registerPlugin(PluginInterface* plugin)
    {
        foreach (GuiPluginInterface* guiPlugin, plugin->getGuiPlugins())
        {
            QWidget* widget = guiPlugin->getMainWidget(ui.mainWidgetStack);
            QWidget* sidebar = guiPlugin->getSidebarWidget(ui.sidebarWidgetStack);
            if (!widget || !sidebar)
            {
                continue;
            }
            int mainIdx = ui.mainWidgetStack->addWidget(widget);
            int sidebarIdx = ui.sidebarWidgetStack->addWidget(sidebar);

            ui.mainWidgetStack->setCurrentIndex(mainIdx);
            ui.sidebarWidgetStack->setCurrentIndex(sidebarIdx);

            QPushButton* button = new QPushButton(guiPlugin->getLabel(), ui.buttonPanel);
            ui.buttonPanel->layout()->addWidget(button);
            button->setCheckable(true);
            button->setAutoExclusive(true);
            button->setChecked(true);
            button->show();
        }
    }

}   // namespace
