#include "guimanager.h"
#include <QPushButton>
#include <QLayout>

namespace aoia {

    SINGLETON_IMPL(GuiManager);


    GuiManager::GuiManager()
    {

    }


    GuiManager::~GuiManager()
    {

    }


    void GuiManager::setParents(QWidget* mainParent, QWidget* sidebarParent, QWidget* labelParent)
    {
        m_main = mainParent;
        m_sidebar = sidebarParent;
        m_labels = labelParent;
    }


    bool GuiManager::extendGui(QWidget* mainWidget, QWidget* sidebarWidget, QString const& label)
    {
        QPushButton* button = new QPushButton(label);
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setChecked(m_labels->layout()->count() == 0 ? true : false);

        m_main->layout()->addWidget(mainWidget);
        m_sidebar->layout()->addWidget(sidebarWidget);
        m_labels->layout()->addWidget(button);

        return true;
    }

}   // namespace
