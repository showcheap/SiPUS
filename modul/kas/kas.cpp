#include <modul/kas/kas.h>
#include "ui_kas.h"
#include <QStandardItemModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDate>
#include <QSqlError>
#include <QDebug>
#include <QLocale>
#include <QMessageBox>

Kas::Kas(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Kas)
{
    ui->setupUi(this);
    this->tataLayout();
    this->tataTabel();
    this->refreshData();
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

Kas::~Kas()
{
    qDebug() << this << "destroyed!";
    delete ui;
}

void Kas::on_pSimpan_clicked()
{
    QSqlQuery query;
    QString ket,keluar,masuk,tgl,saldo;

    tgl = QDate().currentDate().toString("yyyy-MM-dd");
    ket = ui->lKet->text();
    if(ui->comboJenis->currentIndex() == 0){
        keluar = "0";
        masuk = ui->lUang->text();
    }else if(ui->comboJenis->currentIndex() == 1){
        masuk = "0";
        keluar = ui->lUang->text();
    }else{
        masuk = "0";
        keluar = "0";
    }
    query.exec("SELECT saldo FROM tbl_kas ORDER BY id DESC");
    query.next();

    saldo = QString::number(query.value(0).toInt() + masuk.toInt() - keluar.toInt());

    if(!ui->lUang->text().isEmpty()){

        if(query.exec("INSERT INTO tbl_kas SET ket=\""+ket+"\", tgl=\""+tgl+"\", keluar=\""+keluar+"\", masuk=\""+masuk+"\", saldo=\""+saldo+"\"")){
            modelKas->clear();
            this->refreshData();
            this->tataTabel();

            ui->lUang->clear();
            ui->lKet->clear();
            ui->lUang->setFocus();
        }else{
            qDebug()<<query.lastError().text();
        }

    }else{
        QMessageBox::warning(this,"Masukkan Uang","Masukkan sejumlah uang terlebih dahulu");
    }


}

void Kas::tataLayout(){
    //Combo jenis
    ui->comboJenis->addItems(QStringList()<<"Masuk"<<"Keluar");

    //Combo Bulan
    ui->comboBulan->addItems(QStringList()<<"Januari"<<"Pebruari"<<"Maret"<<"April"<<"Mei"<<"Juni"<<"Juli"<<"Agustus"<<"September"<<"Oktober"<<"November"<<"Desember");
    ui->comboBulan->setCurrentIndex(QDate().currentDate().toString("M").toInt()-1);
    //Combo Tahun
    this->initComboTahun();

    //Atur Layout Tabel Peminjaman
    modelKas = new QStandardItemModel(0,4,this);
    ui->tblKas->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblKas->setSortingEnabled(true);
    ui->tblKas->resizeColumnsToContents();
    ui->tblKas->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblKas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblKas->horizontalHeader()->setStretchLastSection(true);
    ui->tblKas->setModel(modelKas);
}

void Kas::tataTabel(){
    modelKas->setHorizontalHeaderItem(0, new QStandardItem(QString("Uraian")));
    modelKas->setHorizontalHeaderItem(1, new QStandardItem(QString("Tanggal")));
    modelKas->setHorizontalHeaderItem(2, new QStandardItem(QString("Masuk")));
    modelKas->setHorizontalHeaderItem(3, new QStandardItem(QString("Keluar")));
    modelKas->setHorizontalHeaderItem(4, new QStandardItem(QString("Saldo")));

    ui->tblKas->setColumnWidth(0, 400);//Uraian
    ui->tblKas->setColumnWidth(1, 200);//Tanggal
    ui->tblKas->setColumnWidth(2, 200);//Masuk
    ui->tblKas->setColumnWidth(3, 200);//Keluar
    ui->tblKas->setColumnWidth(4, 200);//Saldo
}

void Kas::refreshData(){
    QSqlQuery query;
    QString keluar, masuk, saldo;

    QString bulan = QString::number(ui->comboBulan->currentIndex()+1);
    QString tahun = ui->comboTahun->currentText();

    if(query.exec(" SELECT * FROM tbl_kas WHERE MONTH(tgl) = '"+bulan+"' AND YEAR(tgl) = '"+tahun+"' ")){
        int counter=0;
				
				// index column
				int ket 		= query.record().indexOf("ket");
				int tgl_		= query.record().indexOf("tgl");
				int masuk_	= query.record().indexOf("masuk");
				int keluar_	= query.record().indexOf("keluar");
				int saldo_	= query.record().indexOf("saldo");
				
        while(query.next()){
            modelKas->setItem(counter,0,new QStandardItem(QString(query.value(ket).toString())));
            modelKas->setItem(counter,1,new QStandardItem(QString(query.value(tgl_).toString())));
            modelKas->setItem(counter,2,new QStandardItem(QString(query.value(masuk_).toString())));
            modelKas->setItem(counter,3,new QStandardItem(QString(query.value(keluar_).toString())));
            modelKas->setItem(counter,4,new QStandardItem(QString(query.value(saldo_).toString())));

            counter++;
        }
    }
    query.exec("SELECT SUM(masuk) FROM tbl_kas");
    query.next();
        masuk = QLocale::system().toString(query.value(0).toInt());
    query.exec("SELECT SUM(keluar) FROM tbl_kas");
    query.next();
        keluar = QLocale::system().toString(query.value(0).toInt());
    query.exec("SELECT saldo FROM tbl_kas ORDER BY id DESC");
    query.next();
        saldo = QLocale::system().toString(query.value(0).toInt());

    ui->lTotalPemasukan->setText("Rp. "+masuk);
    ui->lTotalKeluar->setText("Rp. "+keluar);
    ui->lSaldoAkhir->setText("Rp. "+saldo);

}

void Kas::on_comboBulan_currentIndexChanged(int index)
{
    QString::number(index);

    //this->on_bRefresh_clicked();
}

void Kas::on_bRefresh_clicked()
{
    modelKas->clear();
    this->refreshData();
    this->tataTabel();
}

void Kas::initComboTahun(){
    QSqlQuery querytahun;
    QString sql = "SELECT DISTINCT YEAR(tgl) FROM tbl_kas WHERE YEAR(tgl) != YEAR(NOW())";

    if( querytahun.exec(sql) ){
        while(querytahun.next()){
            ui->comboTahun->addItem(querytahun.value(0).toString());
        }
    }else{
        qDebug()<<"Sql Query Error "<<__FILE__<<__LINE__;
    }
    QString curYear = QDate().currentDate().toString("yyyy");
    ui->comboTahun->addItem(curYear);
    ui->comboTahun->setCurrentIndex(ui->comboTahun->count()-1);
}
