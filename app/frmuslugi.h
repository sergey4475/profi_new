
#ifndef FRMUSLUGI_H
#define FRMUSLUGI_H

#include <QtGui>
#include "sqlmodules.h"
#include "MyModel.h"

namespace Ui {
    class frmUslugi;
}

class frmUslugi : public QWidget
{
    Q_OBJECT

public:
    explicit frmUslugi(QWidget *parent = 0);
    ~frmUslugi();
    void init();
    PSqlTableModel *tabl;
private slots:

    void on_treeWidget_itemActivated(QTreeWidgetItem *item, int column);

    void on_add_usluga_clicked();

    void on_del_usluga_clicked();

    void on_add_gr_clicked();

    void on_treeWidget_itemPressed(QTreeWidgetItem *item, int column);

    void on_edit_gr_clicked();

    void editFinish(QModelIndex index);

private:
    Ui::frmUslugi *ui;
    QString idVID, idGR;
    bool bEdit;
    int edRow, edCol;
    QTreeWidgetItem *root, *folder, *currentItem;
    bool eventFilter(QObject * obj, QEvent * event);
    void updater(QTreeWidgetItem *item, int count_row,QObject *obj);
    void updater(QModelIndex item, int count_row,QObject *obj);
    void update_tree();
};

#endif // FRMUSLUGI_H
