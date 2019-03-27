#ifndef SERWER_H
#define SERWER_H

#include <QWidget>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QDateEdit>
#include <QDir>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include "extended_connection.h"
#include <QList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QtSql>
#include <QCloseEvent>
#include <QProcess>
#include <QScrollBar>
#include "panel_browser.h"
namespace Ui {
class serwer;
}

class serwer : public QWidget
{
    Q_OBJECT

public:
    explicit serwer(QWidget *parent = 0);
    ~serwer();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    bool runDiscovery();

    bool copyHome(QString ip, QString name);

    void on_znajdz_clicked();

    void on_kopiuj_clicked();

    void on_siec_activated(const QString &arg1);

    std::string getHome(QString name);

    void on_Synchronizuj_clicked();

    bool database_auth(QString token);

    void file_checker();

    void file_remover();

    void rsync_finished();

    void on_informacje_clicked();

    void on_open_panel_clicked();

    void on_wykonaj_komende_clicked();

    void on_fix_permisions_clicked();

    void on_log_textChanged();

    void on_synchronizuj_wszystkie_clicked();

    void on_wykonaj_wszystkie_clicked();

    void on_aktualizuj_clicked();

    void on_autoryzuj_clicked();

public slots:
    bool setup();

    void onstart();
    void newConnection();
    void list_manager();
    void reader();
    void ui_updater();
    bool database_connect();
    void ui_manager();
    void create_file();
    void update_log(QString new_content);
    bool ping_database();
    void lock_interface(bool czy);


private:
    Ui::serwer *ui;
    QTcpServer *serv;
    QList<extended_connection*> clients;
    QProcess *rsyncHome;
    QProcess *bash_root;
    double counter;
    double id;
    QSqlDatabase db;
    QString distri_home;
    QScrollBar *log_scrollbar;
    Panel_Browser *Panel_window;
};

#endif // SERWER_H
