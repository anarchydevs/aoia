#include "itemassistant.h"
#include <QDir>
#include <QPushButton>
#include <ItemAssistantCore/guimanager.h>

namespace aoia {

    ItemAssistant::ItemAssistant(QWidget *parent, Qt::WFlags flags)
        : QMainWindow(parent, flags)
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

        GuiManager::instance().setParents(ui.mainWidgetStack, ui.sidebarWidgetStack, ui.buttonPanel);
    }


    ItemAssistant::~ItemAssistant()
    {
    }

}   // namespace
