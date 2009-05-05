#include "itemassistant.h"
#include <QDir>
#include <QPushButton>


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
    }


    ItemAssistant::~ItemAssistant()
    {
    }

}   // namespace
