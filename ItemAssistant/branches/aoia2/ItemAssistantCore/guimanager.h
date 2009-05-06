#ifndef GUIMANAGER_H
#define GUIMANAGER_H

#include <ItemAssistantCore/itemassistantcore_global.h>
#include <ItemAssistantCore/Singleton.h>
#include <QWidget>


namespace aoia {

    class ITEMASSISTANTCORE_EXPORT GuiManager
    {
        SINGLETON(GuiManager);
    public:
        ~GuiManager();

        // Assignes the container widgets to use
        void setParents(QWidget* mainParent, QWidget* sidebarParent, QWidget* labelParent);

        // Registers widgets to be associated with a labeled activation 
        // control in the GUI. Plugins should use this to register their 
        // GUI extensions.
        bool extendGui(QWidget* mainWidget, QWidget* sidebarWidget, QString const& label);

    private:
        QWidget* m_sidebar;
        QWidget* m_main;
        QWidget* m_labels;
    };

}   // namespace 

#endif // GUIMANAGER_H
