#define  n_PRIHOD 1
#define  n_RASHOD 2

#include "frm_okazanie_uslug.h"
#include "ui_frm_okazanie_uslug.h"
#include "frmselect.h"
#include "frmclients.h"
#include "params.h"
#include "delegats.h"
#include "procedures.h"
#include "mainform.h"
#include <QTableView>
#include <QCompleter>
#include <QDirModel>

frm_okazanie_uslug::frm_okazanie_uslug(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::frm_okazanie_uslug)
{
    ui->setupUi(this);
}

frm_okazanie_uslug::~frm_okazanie_uslug()
{
    delete ui;
}

void frm_okazanie_uslug::InitForm(int nUslugi, WId w_ID){

    DateDoc = QDate::currentDate();
    ui->Date->setText(DateDoc.toString("dd.MM.yyyy"));
    if (w_ID != 0x0){
        QWidget *f = find(w_ID);
        f->deleteLater();
    }
    ID_sotr = 0;
    ID_client = 0;

    ui->Skidka->setVisible(ui->checkSkidka->isChecked());

    NumberUslugi = nUslugi;

    QSqlQuery sql;
    if (NumberUslugi != n_USL_MAG)
    sql.prepare("SELECT MAX(number) AS number "
                "FROM CLIENTS_HISTORY");
    else
        sql.prepare("SELECT MAX(number) AS number "
                    "FROM CLIENTS_PRODAJI");
    sql.exec();
    QSqlRecord record = sql.record();
    sql.next();

    Number = sql.value(record.indexOf("number")).toInt();

    Number++;

    ui->number->setText(QString::number(Number));

    clSqlQueryModel *model = new clSqlQueryModel;
    QSqlQuery query;
    query.prepare("SELECT FIO, POL, NOM_TEL, DEL FROM Clients ");
    query.exec();

    model->setQuery(query);
    model->setHeaderData(0,Qt::Horizontal,"���");
    model->setHeaderData(2,Qt::Horizontal,"�����");

    QCompleter *completer = new QCompleter(this);
    completer->setModel(model);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->Client->setCompleter(completer);

    sotSqlQueryModel *smodel = new sotSqlQueryModel;
    if (NumberUslugi != n_USL_MAG){
        query.prepare("SELECT Personal.FIO, "
                      "DOLJNOSTI.NAME, "
                      "Personal.Nom_Tel, "
                      "Personal.DEL, "
                      "Personal.POL, "
                      "Personal.ID, "
                      "DOLJNOSTI.ID AS Doljn "
                      "FROM Personal INNER JOIN DOLJNOSTI ON Personal.DOLJN = DOLJNOSTI.ID "
                      "WHERE DOLJNOSTI.VID_USLUGI = :Vid" );
        query.bindValue(":VID",NumberUslugi);
    }else{
        query.prepare("SELECT Personal.FIO, "
                      "DOLJNOSTI.NAME, "
                      "Personal.Nom_Tel, "
                      "Personal.DEL, "
                      "Personal.POL, "
                      "Personal.ID, "
                      "DOLJNOSTI.ID AS Doljn "
                      "FROM Personal INNER JOIN DOLJNOSTI ON Personal.DOLJN = DOLJNOSTI.ID ");
    }

    query.exec();
    smodel->setQuery(query);
    smodel->setHeaderData(0,Qt::Horizontal,"���");

    QCompleter *scompleter = new QCompleter(this);
    scompleter->setModel(smodel);
    scompleter->setCaseSensitivity(Qt::CaseInsensitive);
    ui->Sotrudnik->setCompleter(scompleter);

    sql.exec("SELECT VID_PLATEJA.ID, VID_PLATEJA.NAME "
             "FROM VID_PLATEJA ");

    ui->sposobOplati->clear();
    while (sql.next()){
        ui->sposobOplati->addItem(sql.value(1).toString(),sql.value(0).toInt());
    }
    if (NumberUslugi == n_USL_MAG){
        ui->USLUGI->setHidden(true);
        ui->Materials->setHidden(true);
        ui->groupMat->setVisible(false);
        ui->label_Mat->setVisible(false);
        ui->groupUsl->setVisible(false);
        ui->label_Usl->setVisible(false);

        ui->prodaja->setVisible(true);
        ui->groupMag->setVisible(true);
        ui->label_Mag->setVisible(true);
        ui->prodaja->setGeometry(ui->USLUGI->geometry());
        ui->groupMag->setGeometry(ui->groupUsl->geometry());
        ui->label_Mag->setGeometry(ui->label_Usl->geometry());

        QRect R = ui->groupOplati->geometry();
        R.setY(264);
        ui->groupOplati->setGeometry(R);

    }else{
        ui->prodaja->setVisible(false);
        ui->groupMag->setVisible(false);
        ui->label_Mag->setVisible(false);
        QRect R = ui->groupOplati->geometry();
        R.setY(384);
        ui->groupOplati->setGeometry(R);
    }
    ui->but_oplatit->setEnabled(true);
    ui->USLUGI->installEventFilter(this);
    ui->Materials->installEventFilter(this);
    updater();

}

//���������� ��������� �����
void frm_okazanie_uslug::updater(){
    int VidPlateja = ui->sposobOplati->itemData(ui->sposobOplati->currentIndex()).toInt();
    ui->Skidka->setText("0");
    ui->Skidka->setDisabled(false);
    ui->checkSkidka->setVisible(true);
    ui->checkSkidka->setChecked(false);
    ui->spisanie_so_scheta->setChecked(g_spisanie_so_scheta);
    if (VidPlateja == 1){
        ui->spisanie_so_scheta->setVisible(true);
        ui->spisanie_so_scheta->setChecked(g_spisanie_so_scheta);
    }else{
        ui->spisanie_so_scheta->setVisible(false);
        ui->spisanie_so_scheta->setChecked(false);
    }
    if (VidPlateja == 4){
        ui->spisanie_so_scheta->setVisible(false);
        ui->spisanie_so_scheta->setChecked(false);
        ui->checkSkidka->setVisible(false);
        ui->checkSkidka->setChecked(true);
        ui->Skidka->setText("100");
        ui->Skidka->setDisabled(true);
    }
}

// ��������� ������ ������
void frm_okazanie_uslug::editFinish(QModelIndex index){
    double cena = uslModel->itemData(uslModel->index(index.row(),2)).value(0).toDouble();
    int count   = uslModel->itemData(uslModel->index(index.row(),4)).value(0).toInt();

    double summa = cena * count;
    uslModel->setData(uslModel->index(index.row(),5),summa);

    double sum_uslugi = 0;
    for (int ind = 0;ind < uslModel->rowCount(); ind++){
        sum_uslugi += uslModel->itemData(uslModel->index(ind,5)).value(0).toDouble();
    }
    frm->SetSumma(sum_uslugi - setProcent(sum_uslugi));

}

// ��������� ������ �������
void frm_okazanie_uslug::magSelectFinish(QModelIndex index){
    double cena = magModel->itemData(magModel->index(index.row(),2)).value(0).toDouble();
    int count   = magModel->itemData(magModel->index(index.row(),4)).value(0).toInt();

    double summa = cena * count;
    magModel->setData(magModel->index(index.row(),5),summa);

    double sum_uslugi = 0;
    for (int ind = 0;ind < magModel->rowCount(); ind++){
        sum_uslugi += magModel->itemData(magModel->index(ind,5)).value(0).toDouble();
    }
    frm->SetSumma(sum_uslugi - setProcent(sum_uslugi));

}


// ���������� ������
void frm_okazanie_uslug::on_add_usluga_clicked()
{
    NotEditableDelegate *DelegatNotEditCol = new NotEditableDelegate;
    if ((ui->Client->text() != "" && ui->Sotrudnik->text() != "")||(ID_client != 0 && ID_sotr != 0) ){
        QString str = QString("%1").arg(NumberUslugi);
        frmSelect *fSelect = new frmSelect();
        uslModel = new PStandardItemModel;
        uslModel->frm = frm;
        ui->USLUGI->setModel(uslModel);
        ui->USLUGI->setColumnHidden(0,true);
        ui->USLUGI->setColumnHidden(3,true);
        ui->USLUGI->setItemDelegateForColumn(1,DelegatNotEditCol);
        ui->USLUGI->setItemDelegateForColumn(5,DelegatNotEditCol);
        ui->USLUGI->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        connect(uslModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(editFinish(QModelIndex)));
        fSelect->frm = frm;
        fSelect->type_select = n_USLUGI;
        fSelect->type_uslugi_= NumberUslugi;
        fSelect->tempModel   = uslModel;
        fSelect->Id_Client   = ID_client;
        fSelect->init(DateDoc);
        ui->USLUGI->setColumnHidden(0,true);
        ui->USLUGI->setColumnHidden(3,true);
        fSelect->show();
    }else{
        QMessageBox::question(0,"��������!!!","�� ��������� ������������ ����!!!",QMessageBox::Yes);
    }
}


// ���������� ���������
void frm_okazanie_uslug::on_add_material_clicked()
{
    NotEditableDelegate *DelegatNotEditCol = new NotEditableDelegate;
    if ((ui->Client->text() != "" && ui->Sotrudnik->text() != "")||(ID_client != 0 && ID_sotr != 0) ){
        //disconnect(tempModel);
        frmSelect *fSelect = new frmSelect();
        matModel = new PStandardItemModel;
        //tempModel->frm = frm;
        ui->Materials->setModel(matModel);
        ui->Materials->setItemDelegateForColumn(1,DelegatNotEditCol);
        ui->Materials->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        //connect(tempModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),,SLOT(editFinish(QModelIndex)));
        //fSelect->frm = frm;
        fSelect->type_select = n_OSTATKI_SKALD;
        fSelect->type_uslugi_= NumberUslugi;
        fSelect->tempModel   = matModel;
        fSelect->Id_Client   = ID_client;
        fSelect->init(DateDoc);
        fSelect->show();
        ui->Materials->setColumnHidden(0,true);
    }else{
        QMessageBox::question(0,"��������!!!","�� ��������� ������������ ����!!!",QMessageBox::Yes);
    }

}
// ���������� �������
void frm_okazanie_uslug::on_add_prodaja_clicked()
{
    NotEditableDelegate *DelegatNotEditCol = new NotEditableDelegate;
    if ((ui->Client->text() != "" && ui->Sotrudnik->text() != "")||(ID_client != 0 && ID_sotr != 0) ){
        //disconnect(tempModel);
        frmSelect *fSelect = new frmSelect();
        magModel = new PStandardItemModel;
        //tempModel->frm = frm;
        ui->prodaja->setModel(magModel);
        ui->prodaja->setItemDelegateForColumn(1,DelegatNotEditCol);
        ui->prodaja->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        connect(magModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(magSelectFinish(QModelIndex)));
        fSelect->frm = frm;
        fSelect->type_select = n_MAGAZIN;
        fSelect->type_uslugi_= NumberUslugi;
        fSelect->tempModel   = magModel;
        fSelect->Id_Client   = ID_client;
        fSelect->init(DateDoc);
        fSelect->show();
        ui->prodaja->setColumnHidden(0,true);
    }else{
        QMessageBox::question(0,"��������!!!","�� ��������� ������������ ����!!!",QMessageBox::Yes);
    }

}


// �������� ��������� �� ������
void frm_okazanie_uslug::on_del_material_clicked()
{
    ui->Materials->model()->removeRow(ui->Materials->currentIndex().row());
}

// �������� ������ �� ������
void frm_okazanie_uslug::on_del_usluga_clicked()
{
    ui->USLUGI->model()->removeRow(ui->USLUGI->currentIndex().row());
}

// ������ ������ �������
void frm_okazanie_uslug::on_Client_buttonClicked()
{
    selModel = new PStandardItemModel;
    frmSelect *frm_client = new frmSelect;
    connect(selModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(selClientsFinish(QModelIndex)));
    frm_client->tempModel = selModel;
    frm_client->type_select = n_CLIENTS;
    frm_client->frm = frm;
    frm_client->init(DateDoc);
    frm_client->show();

    //    ui->Client->setText(model->item(0,0)->text());
}

// ��������� ������ �������
void frm_okazanie_uslug::selClientsFinish(QModelIndex index){
    ui->Client->setText(selModel->itemData(selModel->index(index.row(),0)).value(0).toString());
    on_Client_editingFinished();
}


// ��������� ������ ��� ����� �������
void frm_okazanie_uslug::on_Client_editingFinished()
{
    ID_client = 0;
    QSqlQuery sql;
    sql.prepare("SELECT Clients.ID "
                "FROM Clients "
                "WHERE Clients.FIO = :FIO ");

    sql.bindValue(":FIO",ui->Client->text());
    sql.exec();

    while (sql.next()){
        ID_client = sql.value(0).toInt();
    }
    if (ID_client != 0)
        frm->UpdateClients(ID_client);
}

//������ ������ ����������
void frm_okazanie_uslug::on_Sotrudnik_buttonClicked()
{
    selModel = new PStandardItemModel;
    frmSelect *frm_client = new frmSelect;
    connect(selModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(selSotrFinish(QModelIndex)));
    frm_client->tempModel = selModel;
    frm_client->type_uslugi_= NumberUslugi;
    frm_client->type_select = n_MASTER;
    frm_client->frm = frm;
    frm_client->init(DateDoc);
    frm_client->show();

}

// ��������� ������ �������
void frm_okazanie_uslug::selSotrFinish(QModelIndex index){
    ui->Sotrudnik->setText(selModel->itemData(selModel->index(index.row(),0)).value(0).toString());
    on_Sotrudnik_editingFinished();
}


// ��������� ������ ��� ����� ����������
void frm_okazanie_uslug::on_Sotrudnik_editingFinished()
{
    QSqlQuery query;
    query.prepare("SELECT PERSONAL.ID "
                  "FROM PERSONAL "
                  "WHERE PERSONAL.FIO = :FIO ");
    query.bindValue(":FIO",ui->Sotrudnik->text());
    query.exec();
    QSqlRecord record = query.record();
    query.next();

    ID_sotr = query.value(record.indexOf("ID")).toInt();
}
// ������ ��������
double frm_okazanie_uslug::setProcent(double summa){
    int skidka_p = ui->Skidka->text().toInt();
    double summa_slidki = (summa * skidka_p)/100;

    return summa_slidki;
}

// ������
void frm_okazanie_uslug::on_but_oplatit_clicked()
{
    int countRow = 0;
    int vid_doc = 0;
    QAbstractItemModel *model = 0x0;
    QDate date_usl = DateDoc;
    int VidPlateja = ui->sposobOplati->itemData(ui->sposobOplati->currentIndex()).toInt();

    double sum_uslugi = 0;

    if (NumberUslugi != n_USL_MAG){
        model = ui->USLUGI->model();
        vid_doc = 1;
    }else if (NumberUslugi == n_USL_MAG){
        model = ui->prodaja->model();
        vid_doc = 2;
    }

    // ������ �����
    if (model == 0x0)
        return;


    countRow = model->rowCount();

    if (ID_sotr != 0 && ID_client != 0 && countRow !=0 ){
        QSqlDatabase::database().transaction();
        for (int ind = 0;ind < countRow; ind++){
            if (NumberUslugi != n_USL_MAG)
                sum_uslugi += model->itemData(model->index(ind,5)).value(0).toDouble();
            else if (NumberUslugi == n_USL_MAG)
                sum_uslugi += model->itemData(model->index(ind,4)).value(0).toDouble();
        }
        double summa_skidki = setProcent(sum_uslugi);
        sum_uslugi = sum_uslugi - summa_skidki;


        // ���� ������ ������, ���� �������, �� ��������� �� ����� ����� �������
        if (VidPlateja == 5){
            if (GetOstatokNaSchete(ID_client,date_usl.toString("dd.MM.yyyy")) < sum_uslugi){
                QMessageBox::question(0,"��������!!!!","�� ����� � ������� �� ���������� �������!!! \n "
                                      "�������� ������ ������ ������ ��� ��������� ���� �������",QMessageBox::Ok);
                return;
            }
            bool result = EditChetClienta(ID_client,N_CH_RASHOD,sum_uslugi,date_usl.toString("dd.MM.yyyy"));

            if (result){
                // ��������� ������

                QSqlQuery sql;
                sql.prepare("INSERT INTO OPLATI_CLIENTS(NUMBER,DATE,TYPE_OPERACII,ID_CLIENT,SUMMA_K_OPL,SUMMA_OPL, type_oplati, vid_doc) "
                            "VALUES(:NUMBER,:DATE,:TYPE_OPERACII,:ID_CLIENT,:SUMMA_K_OPL,:SUMMA_OPL,:type_oplati, :vid_doc) ");
                sql.bindValue(":NUMBER",Number);
                sql.bindValue(":DATE",DateDoc.toString("dd.MM.yyyy"));
                sql.bindValue(":TYPE_OPERACII",n_PL_OSN);
                sql.bindValue(":ID_CLIENT",ID_client);
                sql.bindValue(":SUMMA_K_OPL",sum_uslugi);
                sql.bindValue(":SUMMA_OPL",sum_uslugi);
                sql.bindValue(":type_oplati",VidPlateja);
                sql.bindValue(":vid_doc",vid_doc);
                sql.exec();

                if (sql.lastError().isValid()){
                    QSqlDatabase::database().rollback();
                    return;
                }
            }
        }
        // ���� ������ ������, �� ���� ������� � ���������� �������� ��������� �� ����� ��� ������
        // ��������� ������ �������
        if (VidPlateja != 5 && VidPlateja != 4){

            double ostatok = 0;
            double summa_Oplati = sum_uslugi;
            double summa_spisan = sum_uslugi;

            if (ui->spisanie_so_scheta->checkState()){
                ostatok = GetOstatokNaSchete(ID_client,date_usl.toString("dd.MM.yyyy"));
                summa_Oplati = 0;

                if (ostatok < sum_uslugi){
                    summa_spisan = ostatok;
                    summa_Oplati = QInputDialog::getDouble(this,"������� ����� �������: ","�����:",sum_uslugi - ostatok,0,99999999,2,0,0);
                    if (summa_Oplati == 0)
                        return;
                }

                bool result = EditChetClienta(ID_client,N_CH_RASHOD,summa_spisan,date_usl.toString("dd.MM.yyyy"));

                if (result){
                    QSqlQuery sql;
                    sql.prepare("INSERT INTO OPLATI_CLIENTS(NUMBER,DATE,TYPE_OPERACII,ID_CLIENT,SUMMA_K_OPL,SUMMA_OPL, type_oplati, vid_doc) "
                                "VALUES(:NUMBER,:DATE,:TYPE_OPERACII,:ID_CLIENT,:SUMMA_K_OPL,:SUMMA_OPL,:type_oplati, :vid_doc) ");
                    sql.bindValue(":NUMBER",Number);
                    sql.bindValue(":DATE",DateDoc.toString("dd.MM.yyyy"));
                    sql.bindValue(":TYPE_OPERACII",n_PL_OSN);
                    sql.bindValue(":ID_CLIENT",ID_client);
                    sql.bindValue(":SUMMA_K_OPL",summa_spisan);
                    sql.bindValue(":SUMMA_OPL",summa_spisan);
                    sql.bindValue(":type_oplati",5);
                    sql.bindValue(":vid_doc",vid_doc);
                    sql.exec();

                    if (sql.lastError().isValid()){
                        QSqlDatabase::database().rollback();
                        return;
                    }
                }
            }else{
                summa_Oplati = QInputDialog::getDouble(this,"������� ����� ������: ","�����:",sum_uslugi,0,99999999,2,0,0);
                if (summa_Oplati == 0)
                    return;
            }
            if(summa_Oplati > 0){
                QSqlQuery sql;
                sql.prepare("INSERT INTO OPLATI_CLIENTS(NUMBER,DATE,TYPE_OPERACII,ID_CLIENT,SUMMA_K_OPL,SUMMA_OPL, type_oplati, vid_doc) "
                            "VALUES(:NUMBER,:DATE,:TYPE_OPERACII,:ID_CLIENT,:SUMMA_K_OPL,:SUMMA_OPL,:type_oplati, :vid_doc) ");
                sql.bindValue(":NUMBER",Number);
                sql.bindValue(":DATE",DateDoc.toString("dd.MM.yyyy"));
                sql.bindValue(":TYPE_OPERACII",n_PL_OSN);
                sql.bindValue(":ID_CLIENT",ID_client);
                sql.bindValue(":SUMMA_K_OPL",sum_uslugi - ostatok);
                sql.bindValue(":SUMMA_OPL",summa_Oplati);
                sql.bindValue(":type_oplati",VidPlateja);
                sql.bindValue(":vid_doc",vid_doc);
                sql.exec();

                qDebug() << sql.lastError();
                if (sql.lastError().isValid()){
                    QSqlDatabase::database().rollback();
                    return;
                }

                if (VidPlateja == 1){

                    sql.prepare("INSERT INTO KASSA(NUMBER, DATE, type_operacii, summa) "
                                "VALUES(:NUMBER, :DATE, :type_operacii, :SUMMA) ");
                    sql.bindValue(":NUMBER",Number);
                    sql.bindValue(":DATE",DateDoc.toString("dd.MM.yyyy"));
                    sql.bindValue(":TYPE_OPERACII",n_KASSA_PRIHOD);
                    sql.bindValue(":SUMMA",summa_Oplati);
                    sql.exec();
                    qDebug() << sql.lastError();
                    if (sql.lastError().isValid()){
                        QSqlDatabase::database().rollback();
                        return;
                    }
                }
            }
        }

        for (int ind = 0; ind < countRow; ind++){
            QSqlQuery sql;
            int IDUsl   = 0;
            int count   = 0;
            double summa= 0;
            double cena = 0;
            int skidka_p = 0;
            double summa_slidki = setProcent(summa);
            double summa_vsego = 0;

            if (NumberUslugi != n_USL_MAG){
                IDUsl   = model->itemData(model->index(ind,0)).value(0).toInt();
                count   = model->itemData(model->index(ind,4)).value(0).toInt();
                summa= model->itemData(model->index(ind,5)).value(0).toDouble();
                cena = model->itemData(model->index(ind,2)).value(0).toDouble();
                skidka_p = ui->Skidka->text().toInt();
                summa_slidki = setProcent(summa);
                summa_vsego = summa - summa_slidki;
                sql.prepare("INSERT INTO CLIENTS_HISTORY(NUMBER,DATE_USLUGI,ID_CLIENT,ID_SOTRUDNIK,ID_USLUGA,count,cena,SUMMA,oplacheno,vid_oplati,skidka_p,summa_skidki,summa_vsego) "
                            "VALUES(:NUMBER,:DATE_USLUGI,:ID_CLIENT,:ID_SOTRUDNIK,:ID_USLUGA,:count,:cena,:SUMMA,:oplacheno,:vid_oplati,:skidka_p,:summa_skidki,:summa_vsego) ");
            }
            else{
                IDUsl   = model->itemData(model->index(ind,0)).value(0).toInt();
                count   = model->itemData(model->index(ind,2)).value(0).toInt();
                summa= model->itemData(model->index(ind,4)).value(0).toDouble();
                cena = 0;
                skidka_p = ui->Skidka->text().toInt();
                summa_slidki = setProcent(summa);
                summa_vsego = summa - summa_slidki;

                sql.prepare("INSERT INTO clients_prodaji(NUMBER,DATE,ID_CLIENT,ID_SOTRUDNIK,ID_MATERIAL,count,cena,SUMMA,oplacheno,vid_oplati,skidka_p,summa_skidki,summa_vsego) "
                            "VALUES(:NUMBER,:DATE_USLUGI,:ID_CLIENT,:ID_SOTRUDNIK,:ID_USLUGA,:count,:cena,:SUMMA,:oplacheno,:vid_oplati,:skidka_p,:summa_skidki,:summa_vsego) ");

                QSqlQuery sql_;
                sql_.prepare("INSERT INTO SKLAD(DATE,ID_MATERIAL,COUNT,type_operacii,id_VID_ZATRAT,NUMBER,vid_doc) "
                            "VALUES(:DATE,:ID_MATERIAL,:COUNT,:type_operacii,:vid_zatrat,:NUMBER,:vid_doc) ");

                sql_.bindValue(":DATE",date_usl.toString("dd.MM.yyyy"));
                sql_.bindValue(":ID_MATERIAL",IDUsl);
                sql_.bindValue(":COUNT",count * (-1));
                sql_.bindValue(":type_operacii",n_RASHOD);
                sql_.bindValue(":vid_zatrat",NumberUslugi);
                sql_.bindValue(":NUMBER",Number);
                sql_.bindValue(":vid_doc",vid_doc);
                sql_.exec();

            }


            sql.bindValue(":NUMBER",Number);
            sql.bindValue(":DATE_USLUGI",date_usl.toString("dd.MM.yyyy"));
            sql.bindValue(":ID_CLIENT",ID_client);
            sql.bindValue(":ID_SOTRUDNIK",ID_sotr);
            sql.bindValue(":ID_USLUGA",IDUsl);
            sql.bindValue(":count",count);
            sql.bindValue(":cena",cena);
            sql.bindValue(":SUMMA",summa);
            sql.bindValue(":oplacheno",true);
            sql.bindValue(":vid_oplati",VidPlateja);
            sql.bindValue(":skidka_p",skidka_p);
            sql.bindValue(":summa_skidki",summa_slidki);
            sql.bindValue(":summa_vsego",summa_vsego);

            sql.exec();
            qDebug() << sql.lastError();
            if (sql.lastError().isValid()){
                QSqlDatabase::database().rollback();
                return;
            }

            ui->USLUGI->setDisabled(true);
            ui->prodaja->setDisabled(true);
            ui->but_oplatit->setDisabled(true);
        }
        // �������� ����������
        countRow = 0;
        if (ui->Materials->model() != NULL)
            countRow = ui->Materials->model()->rowCount();

        for (int ind = 0; ind < countRow; ind++){
            int ID_MATERIAL   = ui->Materials->model()->itemData(ui->Materials->model()->index(ind,0)).value(0).toInt();
            int COUNT   = ui->Materials->model()->itemData(ui->Materials->model()->index(ind,2)).value(0).toInt();

            double ostatok = GetOstatokNaSklade(ID_MATERIAL,NumberUslugi,DateDoc.toString("dd.MM.yyyy"),N_SKLAD);
            if (COUNT > ostatok){
                QMessageBox::warning(0,"��������!!!!!!!!!!!",ui->Materials->model()->itemData(ui->Materials->model()->index(ind,1)).value(0).toString()+ " �� ������� �� ������!",QMessageBox::Ok);
                return;
            }

            QSqlQuery sql;
            sql.prepare("INSERT INTO SKLAD(DATE,ID_MATERIAL,COUNT,type_operacii,id_VID_ZATRAT,NUMBER) "
                        "VALUES(:DATE,:ID_MATERIAL,:COUNT,:type_operacii,:vid_zatrat,:NUMBER) ");

            sql.bindValue(":DATE",date_usl.toString("dd.MM.yyyy"));
            sql.bindValue(":ID_MATERIAL",ID_MATERIAL);
            sql.bindValue(":COUNT",COUNT * (-1));
            sql.bindValue(":type_operacii",n_RASHOD);
            sql.bindValue(":vid_zatrat",NumberUslugi);
            sql.bindValue(":NUMBER",Number);
            sql.exec();

            if (sql.lastError().isValid()){
                QSqlDatabase::database().rollback();
                return;
            }

            if (ID_client != 0)
                frm->UpdateClients(ID_client);
        }
        frm->UpdateClients(0);
        QSqlDatabase::database().commit();
        on_closeButton_clicked();
    }else if (NumberUslugi == n_MAGAZIN){

    }
    else{
        QMessageBox::question(0,"��������!!!","�� ��������� ������������ ����!!!",QMessageBox::Ok);
    }

}

void frm_okazanie_uslug::on_sposobOplati_activated(const QString &arg1)
{
    updater();
}

bool frm_okazanie_uslug::eventFilter(QObject *obj, QEvent *event){
    if (event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        // ��������� ������� enter ��� return
        if (obj->objectName() == "USLUGI") {
            if (keyEvent->key() == Qt::Key_Insert)
                on_add_usluga_clicked();
        }
        if (obj->objectName() == "Materials") {
            if (keyEvent->key() == Qt::Key_Insert)
                on_add_material_clicked();
        }

    }
    return QWidget::eventFilter(obj , event);
}

void frm_okazanie_uslug::on_closeButton_clicked()
{
    frm->EnableButton(NumberUslugi);
    this->~frm_okazanie_uslug();
}

void frm_okazanie_uslug::on_checkSkidka_clicked()
{
    ui->Skidka->setVisible(ui->checkSkidka->isChecked());
    ui->Skidka->setText(0);
    if (ui->USLUGI->model() != 0x0){
        double sum_uslugi = 0;
        for (int ind = 0;ind < uslModel->rowCount(); ind++){
            sum_uslugi += uslModel->itemData(uslModel->index(ind,5)).value(0).toDouble();
        }
        frm->SetSumma(sum_uslugi - setProcent(sum_uslugi));

    }


}

void frm_okazanie_uslug::on_Skidka_textChanged(const QString &arg1)
{
    if (ui->USLUGI->model() != 0x0){
        double sum_uslugi = 0;
        for (int ind = 0;ind < uslModel->rowCount(); ind++){
            sum_uslugi += uslModel->itemData(uslModel->index(ind,5)).value(0).toDouble();
        }
        frm->SetSumma(sum_uslugi - setProcent(sum_uslugi));

    }

}
