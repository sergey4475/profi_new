#include "frmdocument.h"
#include "params.h"
#include "ui_frmdocument.h"

frmDocument::frmDocument(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmDocument)
{
    ui->setupUi(this);
}

frmDocument::~frmDocument()
{
    delete ui;
}

void frmDocument::GetOstaok(){
    QSqlQuery sql;
    int id_group = ui->Group->itemData(ui->Group->currentIndex()).toInt();
    if (id_group <=0)
    sql.prepare("SELECT "
                "   materials.name, "
                "   SUM(o_sklad.count) AS count, "
                "   ed_izm.name "
                "FROM "
                "   public.o_sklad, "
                "   public.materials, "
                "   public.ed_izm "
                "WHERE "
                "   o_sklad.id_material = materials.id "
                "   AND o_sklad.DATE <= :DATE "
                "   AND materials.id_ed_izm = ed_izm.id "
                "GROUP BY "
                "   ed_izm.name, "
                "   materials.name ");
    else{
        sql.prepare("SELECT "
                    "   materials.name, "
                    "   SUM(o_sklad.count) AS count, "
                    "   ed_izm.name "
                    "FROM "
                    "   o_sklad, "
                    "   materials, "
                    "   ed_izm "
                    "WHERE "
                    "   o_sklad.id_material = materials.id "
                    "   AND o_sklad.DATE <= :DATE "
                    "   AND materials.id_ed_izm = ed_izm.id "
                    "   AND O_SKLAD.id_group_o_sklad = :id_group_o_sklad "
                    "GROUP BY "
                    "   ed_izm.name, "
                    "   materials.name ");
        sql.bindValue(":id_group_o_sklad",id_group);
    }

    sql.bindValue(":DATE",ui->DateDoc->date().toString("dd.MM.yyyy"));
    Ost_model *model_ = new Ost_model;
    sql.exec();
    qDebug() << sql.lastError();
    model_->setQuery(sql);
    model_->setHeaderData(0,Qt::Horizontal,"��������");
    model_->setHeaderData(1,Qt::Horizontal,"���-��");
    model_->setHeaderData(2,Qt::Horizontal,"��.���");

    ui->tableView->setModel(model_);
}

void frmDocument::initForm(PStandardItemModel *model, int vid_form, int type_doc){
    QSqlQuery sql;
    this->setWindowFlags(Qt::Tool);
    this->setWindowModality(Qt::ApplicationModal);
    NotEditableDelegate *del = new NotEditableDelegate;
    ui->DateDoc->setDateCalendar(QDate::currentDate());
    QSqlRecord record;

    type_doc_ = type_doc;
    vid_form_ = vid_form;
    // --- �������� �� ������ ---
    if (type_doc_ == d_oskald){

        //************ ��������� ������ ������
        sql.prepare("SELECT ID, Name "
                    "FROM group_o_sklad "
                    "ORDER BY ID");
        sql.exec();
        while (sql.next()){
            ui->Group->addItem(sql.value(1).toString(),sql.value(0).toInt());
        }

        ui->l_group->setText("����� ������:");
        ui->l_vid_zatrat->setHidden(true);
        ui->vid_zatrat->setHidden(true);
        if (vid_form_ == f_document){ // -- ��� ����� ��������
            tempModel = model;
            sql.prepare("SELECT MAX(number) AS number "
                        "FROM O_SKLAD");
            sql.exec();
            record = sql.record();
            sql.next();

            Number = sql.value(record.indexOf("number")).toInt();

            Number++;

            ui->Number->setText(QString::number(Number));

            ui->tableView->setModel(tempModel);
            ui->tableView->setColumnHidden(0,true);
            ui->tableView->setColumnHidden(1,true);
            ui->tableView->setColumnHidden(4,true);
            ui->tableView->setColumnHidden(5,true);

            ui->tableView->setColumnWidth(2,280);
            ui->tableView->setColumnWidth(3,50);
            ui->tableView->setColumnWidth(6,50);

        }
        if (vid_form_ == f_ostatki){ // -- ��� ����� �������
            ui->Number->setDisabled(true);
            ui->ApplyBut->setDisabled(true);
            GetOstaok();
        }
    }
    // --- �������� �� ������������� ��������� ---
    if (type_doc_ == d_raspred){
        ui->l_group->setText("����� ������:");
        ui->l_vid_zatrat->setText("�������:");
        if (vid_form == 0){
            tempModel = model;
            sql.prepare("SELECT MAX(number) AS number "
                     "FROM SKLAD");
            sql.exec();
            record = sql.record();
            sql.next();

            Number = sql.value(record.indexOf("number")).toInt();

            Number++;

            // ��������� ������ ������ ���������
            sql.prepare("SELECT vidi_zatrat.id, vidi_zatrat.Name "
                        "FROM vidi_zatrat "
                        "ORDER BY vidi_zatrat.id");
            sql.exec();
            record = sql.record();
            while (sql.next()){
                ui->vid_zatrat->addItem(sql.value(1).toString(),sql.value(0).toInt());
            }

            // ��������� ��������� �� ������
            sql.prepare("SELECT ID, Name "
                        "FROM group_o_sklad "
                        "ORDER BY ID");
            sql.exec();
            record = sql.record();
            while (sql.next()){
                ui->Group->addItem(sql.value(1).toString(),sql.value(0).toInt());
            }

            ui->Number->setText(QString::number(Number));

            ui->tableView->setModel(tempModel);
            ui->tableView->setColumnHidden(0,true);
            ui->tableView->setColumnHidden(1,true);
            ui->tableView->setColumnHidden(4,true);
            ui->tableView->setColumnHidden(5,true);

            ui->tableView->setColumnWidth(2,280);
            ui->tableView->setColumnWidth(3,50);
            ui->tableView->setColumnWidth(6,50);

        }
    }

    ui->tableView->setItemDelegateForColumn(0,del);
    ui->tableView->setItemDelegateForColumn(1,del);
    ui->tableView->setItemDelegateForColumn(2,del);

}

void frmDocument::on_add_button_clicked()
{
    // --- �������� �� ������ ---
    if (type_doc_ == d_oskald) {
        if (ui->Group->currentIndex() == 0){
            QMessageBox::question(0,"��������!!!","�������� ����� ������!",QMessageBox::Ok);
            return;
        }

        frmSelect *fSelect = new frmSelect();
        fSelect->type_select = n_MATERIAL;
        fSelect->type_uslugi_= 0;
        fSelect->tempModel   = tempModel;
        fSelect->Id_Client   = 0;
        fSelect->init(ui->DateDoc->date());
        fSelect->setWindowModality(Qt::ApplicationModal);
        fSelect->show();
    }

    // --- �������� �� ������������� ��������� ---
    if (type_doc_ == d_raspred){
        if (ui->Group->currentIndex() == 0){
            QMessageBox::question(0,"��������!!!","�������� �������!",QMessageBox::Ok);
            return;
        }
        frmSelect *fSelect = new frmSelect();
        fSelect->type_select = n_SKALD;
        fSelect->type_uslugi_= ui->Group->itemData(ui->Group->currentIndex()).toInt();
        fSelect->tempModel   = tempModel;
        fSelect->Id_Client   = 0;
        fSelect->init(ui->DateDoc->date());

        fSelect->setWindowModality(Qt::ApplicationModal);
        fSelect->show();
    }

}

void frmDocument::on_del_button_clicked()
{
    if (tempModel->rowCount() > 0)
        tempModel->removeRow(ui->tableView->currentIndex().row());
}

void frmDocument::on_ApplyBut_clicked()
{
    int countRow = tempModel->rowCount();
    for (int ind = 0; ind < countRow; ind++){
        QString DATE        = tempModel->itemData(tempModel->index(ind,1)).value(0).toString();
        int ID_MATERIAL     = tempModel->itemData(tempModel->index(ind,0)).value(0).toInt();
        int COUNT           = tempModel->itemData(tempModel->index(ind,3)).value(0).toDouble();
        int type_operacii   = tempModel->itemData(tempModel->index(ind,4)).value(0).toInt();
        int id_group        = ui->Group->itemData(ui->Group->currentIndex()).toInt();
        int vid_zatrat      = ui->vid_zatrat->itemData(ui->vid_zatrat->currentIndex()).toInt();
        int NUMBER          = Number;
        QSqlQuery sql;
        // --- �������� �� ��������� ����� ---
        if (type_doc_ == d_oskald){

            sql.prepare("INSERT INTO O_SKLAD(DATE,ID_MATERIAL,COUNT,type_operacii,NUMBER,id_group_o_sklad) "
                        "VALUES(:DATE,:ID_MATERIAL,:COUNT,:type_operacii,:NUMBER,:id_group_o_sklad) ");
            sql.bindValue(":DATE",DATE);
            sql.bindValue(":ID_MATERIAL",ID_MATERIAL);
            sql.bindValue(":COUNT",COUNT);
            sql.bindValue(":type_operacii",type_operacii);
            sql.bindValue(":NUMBER",NUMBER);
            sql.bindValue(":id_group_o_sklad",id_group);
            sql.exec();

        }
        // --- �������� �� ��������� ������������� ���������� ---
        if (type_doc_ == d_raspred){
            double ostatok = GetOstatokNaSklade(ID_MATERIAL,id_group,DATE,N_O_SKLAD);
            if (COUNT > ostatok){
                QMessageBox::warning(0,"��������!!!!!!!!!!!","��������� "+tempModel->itemData(tempModel->index(ind,2)).value(0).toString()+ "������������ �� �������� ������ ",QMessageBox::Ok);
                return;
            }

            sql.prepare("INSERT INTO SKLAD(DATE,ID_MATERIAL,COUNT,type_operacii,id_VID_ZATRAT,NUMBER) "
                        "VALUES(:DATE,:ID_MATERIAL,:COUNT,:type_operacii,:vid_zatrat,:NUMBER) ");

            sql.bindValue(":DATE",DATE);
            sql.bindValue(":ID_MATERIAL",ID_MATERIAL);
            sql.bindValue(":COUNT",COUNT);
            sql.bindValue(":type_operacii",n_PRIHOD);
            sql.bindValue(":vid_zatrat",vid_zatrat);
            sql.bindValue(":NUMBER",NUMBER);
            sql.exec();

            //��������� ������ � ������ ������
            sql.prepare("INSERT INTO O_SKLAD(DATE,ID_MATERIAL,COUNT,type_operacii,NUMBER,id_group_o_sklad) "
                        "VALUES(:DATE,:ID_MATERIAL,:COUNT,:type_operacii,:NUMBER,:id_group_o_sklad)");
            sql.bindValue(":DATE",DATE);
            sql.bindValue(":ID_MATERIAL",ID_MATERIAL);
            sql.bindValue(":COUNT",COUNT * (-1));
            sql.bindValue(":type_operacii",n_RASHOD);
            sql.bindValue(":NUMBER",NUMBER);
            sql.bindValue(":id_group_o_sklad",id_group);
            sql.exec();
        }
    }
    if (countRow > 0)
        this->close();
}

void frmDocument::on_DateDoc_dateChanged(QDate &Date)
{
    if (vid_form_ == f_ostatki) // -- ��� ����� �������
        GetOstaok();
}

void frmDocument::on_Group_currentIndexChanged(const QString &arg1)
{
    if (vid_form_ == f_ostatki) // -- ��� ����� �������
        GetOstaok();
}
