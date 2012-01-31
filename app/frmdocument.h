#ifndef FRMDOCUMENT_H
#define FRMDOCUMENT_H

#define f_document  0
#define f_ostatki   1
#define d_oskald    1
#define d_raspred   2

#include <QWidget>
#include "MyModel.h"
#include "include/delegats.h"
#include "frmselect.h"
#include "procedures.h"

namespace Ui {
    class frmDocument;
}

class frmDocument : public QWidget
{
    Q_OBJECT

public:
    explicit frmDocument(QWidget *parent = 0);
    void initForm(PStandardItemModel *model,int vid_form, int type_doc);
    PStandardItemModel *tempModel;
    int Number,type_doc_,vid_form_;

    ~frmDocument();

private slots:
    void on_add_button_clicked();

    void on_del_button_clicked();

    void on_ApplyBut_clicked();

    void on_DateDoc_dateChanged(QDate &Date);

    void on_Group_currentIndexChanged(const QString &arg1);

private:
    void GetOstaok();

private:
    Ui::frmDocument *ui;
};

#endif // FRMDOCUMENT_H
