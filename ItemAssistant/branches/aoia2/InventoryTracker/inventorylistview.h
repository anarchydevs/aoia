#ifndef INVENTORYLISTVIEW_H
#define INVENTORYLISTVIEW_H

#include <QWidget>
#include "ui_inventorylistview.h"

class InventoryListView : public QWidget
{
    Q_OBJECT

public:
    InventoryListView(QWidget *parent = 0);
    ~InventoryListView();

private:
    Ui::InventoryListViewClass ui;
};

#endif // INVENTORYLISTVIEW_H
