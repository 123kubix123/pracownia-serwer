#include "serwer.h"
#include "ui_serwer.h"
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

QString log1;
QString selectedNetwork;
QString selectedInterface;
QString distributionpass;
// domyślne opcje
QString ssh_port = "22";
QString client_ssh_port = "22";
QString database_port = "3306";
QString host = "localhost";
QString http_port = "80";
serwer::serwer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::serwer)
{
    ui->setupUi(this);
    counter = 0;
    id = 0;
    log_scrollbar = ui->log->verticalScrollBar();
    Panel_window = new Panel_Browser();
}

serwer::~serwer()
{
    delete ui;
}

void serwer::ui_manager()
{
    qInfo()<<"Pracownia-serwer - development release https://pracownia.tuxdev.com/";
    qInfo()<<"Autorzy Jakub Wolski i Jakub Poplawski";
    update_log("<img src=\"qrc:/loga/logo_420x53.png\"/><br/>Witaj w Pracowni!");
    bash_root = new QProcess(this);
    bash_root->start("/bin/bash");
    bash_root->waitForStarted();
        lock_interface(true);
        this->database_connect();
        this->onstart();
        //file_remover();
        QTimer *timer=new QTimer(this);
        connect(timer,SIGNAL(timeout()),this,SLOT(file_checker()));
        timer->start(500);
}

void serwer::file_remover()
{
    QProcess remover;
    remover.startDetached("/bin/bash", QStringList()<< "-c" << "rm /opt/pracownia/bin/show.interface");
}

void serwer::file_checker()
{
    //QFileinfo checker("/opt/pracownia/bin/show.interface");
    if(QFileInfo::exists("/opt/pracownia/bin/show.interface"))
    {
        //qInfo()<<"show interface";
        this->show();

    }
    else
    {
        //qInfo()<<"hide interface";
        this->hide();
    }
}

void serwer::create_file()
{
    QFile file1("/opt/pracownia/bin/show.interface");
       if (!file1.open(QIODevice::WriteOnly | QIODevice::Text))
       {
           //err
       }
       QTextStream out1(&file1);
       out1 << " ";
       file1.close();
}


void serwer::closeEvent(QCloseEvent *event)
{
    file_remover();
    event->ignore();
}

bool serwer::setup()
{
    ui->znajdz->setEnabled(false);
    ui->kopiuj->setEnabled(false);
    ui->wybierz->setEnabled(false);

    QProcess listNetworks;
    listNetworks.start("/bin/bash");
    listNetworks.waitForStarted();
    listNetworks.write("netstat -nr | grep -v ^0\\.0\\.0\\.0 | grep -v Kernel | grep -v Destination | awk '{ print $1 }' | sed ':a;N;$!ba;s/\\n/-/g' && exit || exit\n");
    listNetworks.waitForBytesWritten();
    listNetworks.waitForReadyRead();
    QString sieci = listNetworks.readAllStandardOutput();
    listNetworks.waitForFinished();
    listNetworks.close();
    sieci.replace(" ","");
    sieci.replace("\n","");
    QStringList listaSieci = sieci.split("-");
    listaSieci.removeDuplicates();
    foreach(QString siec, listaSieci)
    {
        QProcess networkInfo;
        networkInfo.start("/bin/bash");
        networkInfo.waitForStarted();
        std::string komendaInterfejs = "netstat -nr | grep '"+siec.toStdString()+"' | awk '{ print $8 }' | sed -n 1p \n";
        networkInfo.write(komendaInterfejs.c_str());
        networkInfo.waitForBytesWritten();
        networkInfo.waitForReadyRead();
        QString interfejs = networkInfo.readAllStandardOutput();
        interfejs.replace(" ","");
        interfejs.replace("\n","");
        std::string komendaMaska = "ip a | grep inet | grep -v inet6 | grep -v 127 | grep "+interfejs.toStdString()+" | awk '{ print $2 }' | cut -d/ -f2 && exit || exit\n";
        networkInfo.write(komendaMaska.c_str());
        networkInfo.waitForBytesWritten();
        networkInfo.waitForReadyRead();
        QString maska = networkInfo.readAllStandardOutput();
        networkInfo.waitForFinished();
        networkInfo.close();
        maska.replace(" ","");
        maska.replace("\n","");
        ui->siec->addItem(siec+"/"+maska+"-"+interfejs);
    }
    return true;
}
//netstat -nr | grep -v ^0\.0\.0\.0 | grep -v Kernel | grep -v Destination | awk '{ print $1 }' //adres sieci
//ip a | grep inet | grep -v inet6 | grep -v 127 | awk '{ print $2 }' | cut -d/ -f2 //maska
//nmap -sn $siec/$maska | grep "Nmap scan report" | cut -d"r" -f4 | sed ':a;N;$!ba;s/\n/-/g'  //ip w sieci
// && exit || exit\n


//wykrywanie różnych sieci:
/*
 * adresy (oddzielane myślnikami): netstat -nr | grep -v ^0\.0\.0\.0 | grep -v Kernel | grep -v Destination | awk '{ print $1 }' | sed ':a;N;$!ba;s/\n/-/g'
 * interfejs do wybranej sieci: netstat -nr | grep '+adres_sieci+' | awk '{ print $8 }'
 * maski: ip a | grep inet | grep -v inet6 | grep -v 127 | grep +interfejs+ | awk '{ print $2 }' | cut -d/ -f2
 */

// fajna komenda z delimiterami
// echo "DUPCIA"`nmap -sn -n 192.168.1.0/24 | grep "Nmap scan report" | cut -d"r" -f4 | sed ':a;N;$!ba;s/\n/-/g'`"PO DUPCI"


bool serwer::runDiscovery()
{
    std::string komendaKomputery = "nmap -sn -n "+selectedNetwork.toStdString()+" | grep \"Nmap scan report\" | cut -d\"r\" -f4 | sed ':a;N;$!ba;s/\\n/-/g' \n";
    update_log(QString::fromStdString(komendaKomputery));
    bash_root->write(komendaKomputery.c_str());;
    bash_root->waitForBytesWritten();
    bash_root->waitForReadyRead();
    QString ips = bash_root->readAllStandardOutput();
    ips.replace(" ","");
    ips.replace("\n","");
    update_log(ips);
    QStringList ipslist = ips.split("-");

    QProcess ownHost;
    ownHost.start("/bin/bash");
    ownHost.waitForStarted();
    ownHost.write("hostname && exit || exit\n");
    ownHost.waitForBytesWritten();
    ownHost.waitForReadyRead();
    QString ownHostname = ownHost.readAllStandardOutput();
    ownHostname.replace("\n","");
    ownHost.waitForFinished();
    ownHost.close();
    foreach(QString ip, ipslist)
        {
        std::string sshHostname = "(ssh -o UserKnownHostsFile=/dev/null -o ConnectTimeout=2 -o BatchMode=yes -o StrictHostKeyChecking=no root@"+ip.toStdString()+" -p "+client_ssh_port.toStdString()+" cat /opt/pracownia/bin/config.conf | cut -d= -f2 || echo \"Brak configa\") 2>/dev/null; echo '4564563434534543553454345' \n";
        update_log(QString::fromStdString(sshHostname));
        bash_root->write(sshHostname.c_str());;
        bash_root->waitForBytesWritten();
        bash_root->waitForReadyRead();
        QString hostname = bash_root->readAllStandardOutput();
        hostname.replace("4564563434534543553454345","");
        hostname.replace("\n","");
        hostname.replace(" ","");
        update_log("znaleziony hostname: "+hostname);
        if(hostname!="" && hostname!=ownHostname)
        {
            update_log("dodawanie ip: "+ip+" hostname: "+hostname);
            ui->wybierz->addItem(ip+":"+hostname);
        }
// tu ustawianie było html do loga nie wiem czemu
        }
    if(ui->wybierz->count()!=0)
    {
        ui->wybierz->setEnabled(true);
        ui->znajdz->setEnabled(true);
        ui->kopiuj->setEnabled(true);
    }
    else
    {
            ui->kopiuj->setEnabled(false);
            ui->wybierz->setEnabled(false);

    }
    return true;
}



void serwer::on_znajdz_clicked()
{
    ui->wybierz->clear();
    if(runDiscovery()==true)
    {
       // ui->znajdz->setEnabled(false);
        //todo log info about successful discovery result
    }
    else
    {
      //  ui->znajdz->setEnabled(false);
        // todo failed discovery handling
    }
}

bool serwer::copyHome(QString ip, QString name)
{

    rsyncHome = new QProcess(this);
    rsyncHome->start("/bin/bash");
    rsyncHome->waitForStarted();
    rsyncHome->write("pkexec su; exit \n");
    rsyncHome->waitForBytesWritten();
    rsyncHome->write("whoami \n");
    rsyncHome->waitForBytesWritten();
    rsyncHome->waitForReadyRead();
    QString whoami = rsyncHome->readAllStandardOutput();
    whoami.replace(" ","");
    whoami.replace("\n","");
    if(whoami == "root")
    {
        ui->kopiuj->setEnabled(false);
        ui->znajdz->setEnabled(false);
        ui->siec->setEnabled(false);
        ui->wybierz->setEnabled(false);
        ui->Synchronizuj->setEnabled(false);
        ui->synchronizuj_wszystkie->setEnabled(false);
        update_log("Rozpoczęto kopiowanie z "+name);
        ping_database();
        QSqlQuery gdzie_home;
        gdzie_home.exec("SELECT home_dir FROM komputery WHERE hostname='"+name+"';");
        gdzie_home.next();
        QString home_path = gdzie_home.value(0).toString();
        std::string rsyncCommand = "rsync -aogu -e 'ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -p "+client_ssh_port.toStdString()+"' root@"+ip.toStdString()+":"+home_path.toStdString()+"/ "+getHome(name)+"/ --delete-before --ignore-errors; setfacl -R -m u:distribution:r-x "+getHome(name)+" && exit|| exit\n";
        rsyncHome->write(rsyncCommand.c_str());
        rsyncHome->waitForBytesWritten();
        connect(rsyncHome, SIGNAL(finished(int)), this, SLOT(rsync_finished()));
    }
    else
    {
        rsyncHome->close();

    }
   // connect(rsyncHome, SIGNAL(finished(int)), rsyncHome, SLOT(deleteLater()));
    return true;
}

void serwer::rsync_finished()
{
           QString copyresult = rsyncHome->readAllStandardOutput();
           copyresult += rsyncHome->readAllStandardError();
           update_log(copyresult);
           update_log("Zakończono kopiowanie");
           rsyncHome->close();
           //rsyncHome->deleteLater();
           //delete rsyncHome;
           ui->znajdz->setEnabled(true);
           ui->kopiuj->setEnabled(true);
           ui->siec->setEnabled(true);
           ui->wybierz->setEnabled(true);
           ui->Synchronizuj->setEnabled(true);
           ui->synchronizuj_wszystkie->setEnabled(true);
}



void serwer::on_kopiuj_clicked()
{
    QString toSplit = ui->wybierz->currentText();
    QStringList ipExtractor = toSplit.split(":");
    QString ip = ipExtractor.first();
    QString name = ipExtractor.at(1);
    if(copyHome(ip, name)==true)
    {
        //todo log info about successful copy result
    }
    else
    {
        // todo failed copy handling
    }
}

void serwer::on_siec_activated(const QString &arg1)
{
    QString no_wunused = arg1;
    ui->znajdz->setEnabled(true);
    QStringList separateAddress = arg1.split("-");
    selectedNetwork = separateAddress.first();
    selectedInterface = separateAddress.last();
}
/*
QString serverIp="ip a | grep "+interfejs+" | grep inet | cut -d't' -f2 | cut -d'/' -f1";
QString base="struktura";
QString port="22";

std::string config = "ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no root@"+ip.toStdString()+" -p 22 'echo server="+serverIp.toStdString()+" > /etc/freeze.conf; echo base="+base.toStdString+" >> /etc/freeze.conf; echo port="+port.toStdString+" >> /etc/freeze.conf && exit || exit\n";

 *Do zrobienia:
 * kopiowanie klucza distribution (ALBO WCIEP GO CAŁEGO DO CLIENTA I ZRZUCAJ TYLKO DO WYKONANIA POLECENIA GDZIEŚ DO /TMP I POTEM KASUJ), ale to potem chyba, na razie niech bedzie w /root
 *
 */


//"ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no root@"+ip.toStdString()+" rm /sbin/pracownia-client && exit || exit \n";
//scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no /home/distribution/pracownia-client root@IP:/sbin/pracownia-client
//ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no root@"+ip.toStdString()+" chmod +x /sbin/pracownia-client && exit || exit \n";

std::string serwer::getHome(QString name)
{
    ping_database();
    //qInfo()<< "Accessing database to run query......"<<ok;
    QSqlQuery get_path;
    get_path.exec("SELECT struktura FROM komputery WHERE hostname='"+name+"';");
    get_path.next();
    QString path = get_path.value(0).toString();

    QProcess hm;
    hm.start("/bin/bash");
    hm.waitForStarted();
    std::string cmd= "grep distribution /etc/passwd | cut -d: -f6 && exit || exit \n";
    hm.write(cmd.c_str());
    hm.waitForBytesWritten();
    hm.waitForReadyRead();
    hm.waitForFinished();
    QString returnuj = hm.readAllStandardOutput();
    hm.close();
    returnuj.replace("\n","");

    return returnuj.toStdString()+"/"+path.toStdString();

    // stara wersja funkcji
    /*
    QProcess hm;
    hm.start("/bin/bash");
    hm.waitForStarted();
    std::string cmd= "grep distribution /etc/passwd | cut -d: -f6 && exit || exit \n";
    hm.write(cmd.c_str());
    hm.waitForBytesWritten();
    hm.waitForReadyRead();
    hm.waitForFinished();
    QString returnuj = hm.readAllStandardOutput();
    hm.close();
    returnuj.replace("\n","");
    return returnuj.toStdString();
    */
}

void serwer::onstart()
{
    serv = new QTcpServer(this);
    connect(serv, SIGNAL(newConnection()), this, SLOT(newConnection()));
    int port = 9999;
    if(!serv->listen(QHostAddress::Any,port))
    {
        qInfo() << "Could not start server";
    }
    else
    {
        qInfo() << "Listening on port " << port << "...";
    }

    QProcess hm;
    hm.start("/bin/bash");
    hm.waitForStarted();
    std::string cmd= "grep distribution /etc/passwd | cut -d: -f6 && exit || exit \n";
    hm.write(cmd.c_str());
    hm.waitForBytesWritten();
    hm.waitForReadyRead();
    hm.waitForFinished();
    distri_home = hm.readAllStandardOutput();
    hm.close();
    distri_home.replace("\n","");
}

bool serwer::database_connect()
{
    QString dbname;
    QString dbusername;
    QString dbpasswd;
    QFile file("/opt/pracownia/www/conn.php");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            update_log("Błąd odczytu pliku /opt/pracownia/www/conn.php..");
        }
        else
        {
            int count = 0;
        QTextStream in(&file);
           while (!in.atEnd()) {
               QString line = in.readLine();
               QStringList split = line.split("=");
               QString value = split.last();
               value.replace("\"","");
               value.replace(";","");
               value.replace(" ","");
               value.replace("?","");
               value.replace(">","");
               value.replace("<","");
               if(value != "")
               {
               if(line.startsWith("$db_user"))
               {
                   dbusername = value;
               }
               else if(line.startsWith("$db_name"))
               {
                   dbname = value;
               }
               else if(line.startsWith("$db_password"))
               {
                   dbpasswd = value;
               }
               else if(line.startsWith("$dstpw"))
               {
                   distributionpass = value;
               }
               else if(line.startsWith("$ssh_port"))
               {
                   ssh_port = value;
               }
               else if(line.startsWith("$client_ssh_port"))
               {
                   client_ssh_port = value;
               }
               else if(line.startsWith("$database_port"))
               {
                   database_port = value;
               }
               else if(line.startsWith("$host"))
               {
                    host = value;
               }
               else if(line.startsWith("$http_port"))
               {
                    http_port = value;
               }
               }
               count++;
           }
        }
        file.close();
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setPort(database_port.toInt());
    db.setDatabaseName(dbname);
    db.setUserName(dbusername);
    db.setPassword(dbpasswd);
    bool ok = ping_database();
    qInfo()<< "Connected to database......"<<ok;
    return ok;
}

bool serwer::ping_database()
{
    bool ok = db.open();
    qInfo()<< "Accessing database......"<<ok;
    if(ok == true)
    {
        ui->red_circle->hide();
        ui->green_circle->show();
    }
    else
    {
        ui->green_circle->hide();
        ui->red_circle->show();
    }
    return ok;
}

bool serwer::database_auth(QString token)
{
    ping_database();
    //qInfo()<< "Accessing database to run query......"<<ok;
    QSqlQuery tokeny;
    tokeny.exec("SELECT token FROM komputery;");
    while(tokeny.next())
    {
        QString compare = tokeny.value(0).toString();
        if(compare == token)
        {
            return true;
        }
    }
    return false;
}

void serwer::newConnection()
{
    qInfo()<<"New Connection..";
    extended_connection *connection = new extended_connection(this);
    connection->unique_ID = QString::number(id);
    connection->socket = serv->nextPendingConnection();
    connection->socket->waitForReadyRead(2);
    QString received = connection->socket->readLine();
        if(received == "")
        {
            // zakomentowane bo installer wyswietlał Unauthorized connection jezeli przed udaną prubą połączenia nastąpiły nieudane
            //connection->socket->write("Unauthorized connection");
            //connection->socket->flush();
            connection->socket->disconnect();
            connection->is_disconnected = true;
            qInfo()<<"Unauthorized connection disconnected";
            delete connection;
        }
        else if(database_auth(received) == true)
        {
            connection->socket->write("Authorized");
            connection->socket->flush();
            connection->socket->waitForReadyRead();
            QString Client_Name = connection->socket->readLine();
            connection->Client_Name = Client_Name;
            connection->connector();
            connect(connection, SIGNAL(connection_ended()), this, SLOT(list_manager()));
            connect(connection, SIGNAL(ready_read()), this, SLOT(reader()));

            // run command from panel
            ping_database();
            //qInfo()<< "Accessing database to run query......"<<ok;
            QSqlQuery get_command_for_client;
            get_command_for_client.exec("SELECT bash FROM komputery WHERE hostname='"+connection->Client_Name+"';");
            get_command_for_client.next();
            QString command_for_client = get_command_for_client.value(0).toString();
            if(command_for_client!="")
            {
                update_log("Komputer: "+connection->Client_Name+" zlecono wykonanie: "+command_for_client);
                command_for_client.prepend("z_panelu:");
                command_for_client.append(" && exit || exit\n");
                connection->socket->write(command_for_client.toUtf8());
                connection->socket->flush();

                QSqlQuery clear_command_for_client;
                clear_command_for_client.exec("UPDATE komputery set bash='' WHERE hostname='"+connection->Client_Name+"';");
                get_command_for_client.next();
            }
            clients.append(connection);
            id++;
        }
        else if(received == ui->install_pass->text())
        {
            QString komenda_kopiuj_klucze = "cp /root/.ssh/id_rsa.pub /tmp/.FMsbinQl; chmod 777 /tmp/.FMsbinQl; cp "+distri_home+"/.ssh/id_rsa /tmp/.QSlx32; chmod 744 /tmp/.QSlx32; echo done \n";
            bash_root->write(komenda_kopiuj_klucze.toStdString().c_str());
            bash_root->waitForBytesWritten();
            bash_root->waitForReadyRead();
            QString root_klucz;
            QFile file("/tmp/.FMsbinQl");
                if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    update_log("Błąd odczytywania klucza do instalacji..");
                }
                else
                {
                QTextStream in(&file);
                   while (!in.atEnd()) {
                       QString line = in.readLine();
                       root_klucz += line;
                       root_klucz += "[enter]";
                   }
                }
                file.close();
                QString dist_klucz;
                QFile file1("/tmp/.QSlx32");
                    if(!file1.open(QIODevice::ReadOnly | QIODevice::Text))
                    {
                        update_log("Błąd odczytywania klucza do instalacji..");
                    }
                    else
                    {
                    QTextStream in1(&file1);
                       while (!in1.atEnd()) {
                           QString line = in1.readLine();
                           dist_klucz += line;
                           dist_klucz += "[enter]";
                       }
                    }
                file1.close();
                QString komenda_usun_klucze = "rm -rf /tmp/.FMsbinQl;rm -rf /tmp/.QSlx32; echo done \n";
                bash_root->write(komenda_usun_klucze.toStdString().c_str());
                bash_root->waitForBytesWritten();
                bash_root->waitForReadyRead();
            QString dane_komputerow;
            ping_database();
            //qInfo()<< "Accessing database to run query......"<<ok;
            QSqlQuery pobierz_komputery;
            pobierz_komputery.exec("SELECT hostname, server_ip, token FROM komputery;");
            while(pobierz_komputery.next())
            {
                QString jeden_komputer = pobierz_komputery.value(0).toString()+"^"+pobierz_komputery.value(1).toString()+"^"+pobierz_komputery.value(2).toString()+"^"+root_klucz+"^"+dist_klucz;
                dane_komputerow+=jeden_komputer+"`";
            }
            dane_komputerow.chop(1);
            //log1+=dane_komputerow+"<br/>";
            //ui->log->setHtml(log1);
            //update_log(dane_komputerow);
            connection->socket->write(dane_komputerow.toUtf8());
            //qInfo()<<dane_komputerow;
            connection->socket->flush();
            connection->socket->waitForBytesWritten();
            connection->socket->disconnect();
            delete connection->socket;
            connection->is_disconnected = true;
            delete connection;
        }
        else
        {
            // zakomentowane bo installer wyswietlał Unauthorized connection jezeli przed udaną prubą połączenia nastąpiły nieudane
            //connection->socket->write("Unauthorized connection");
            //connection->socket->flush();
            connection->socket->disconnect();
            connection->is_disconnected = true;
            qInfo()<<"Unauthorized connection disconnected";
            delete connection;
        }
        list_manager();
}

void serwer::list_manager()
{
    int ile = clients.size();
    for(int i = 0; i<ile; i++)
    {
        if(clients[i]->is_disconnected == true)
        {
            clients.removeAt(i);
            list_manager();
            break;
        }
    }
    ui_updater();
}

void serwer::ui_updater()
{
    ui->clients->clear();
    int ile = clients.size();
    for(int i = 0; i<ile; i++)
        {
            ui->clients->addItem(clients[i]->Client_Name);
        }
}

void serwer::reader()
{
    int ile = clients.size();
    for(int i = 0; i<ile; i++)
    {
        if(clients[i]->data_avilable == true)
        {
            QString output = clients[i]->socket->readLine();
            output.replace("\n","");
            //output.replace(" ","");
            update_log(output);
        }
    }
}

void serwer::on_Synchronizuj_clicked()
{
    QString ktory = ui->clients->currentText();
    ping_database();
    //qInfo()<< "Accessing database to run query......"<<ok;
    QSqlQuery data_for_client;
    data_for_client.exec("SELECT katalog FROM komputery WHERE hostname='"+ktory+"';");
    data_for_client.next();
    QString struktura = QString::fromStdString(getHome(ktory));
    QString katalog = data_for_client.value(0).toString();
    int ile = clients.size();
    for(int i = 0; i<ile; i++)
        {
           if(ktory == clients[i]->Client_Name)
           {
               QString polecenie = "mount "+katalog+" /pracownia; id=$(btrfs sub list /pracownia | grep 'home$' | awk '{ print $2 }'); umount /pracownia; mount -t btrfs -o subvolid=$id "+katalog+" /pracownia; sshpass -p '"+distributionpass+"' rsync -aogu -e 'ssh -i /root/id_rsa -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -p "+ssh_port+"' distribution@`:"+struktura+"/ /pracownia/ --delete-before --ignore-errors 2> /var/log/freeze; umount /pracownia && exit || exit\n";
            // QString polecenie = "mount "+katalog+" /pracownia; id=`btrfs sub list /pracownia | grep 'home$' | awk '{ print $2 }'`; umount /pracownia; mount -t btrfs -o subvolid=$id "+katalog+" /pracownia; sshpass -p '"+distributionpass+"' rsync -aogu -e 'ssh -i /root/id_rsa -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -p "+ssh_port+"' distribution@`:"+struktura+"/ /pracownia/ --delete-before --ignore-errors 2> /var/log/freeze; umount /pracownia && exit || exit\n";
               qInfo()<<polecenie;
                //przed updatem: QString polecenie = "mount -t btrfs -o subvol=@/home "+katalog+" /pracownia; sshpass -p '"+distributionpass+"' rsync -aogu -e 'ssh -i /root/id_rsa -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -p "+ssh_port+"' distribution@`:"+struktura+"/ /pracownia/ --delete-before --ignore-errors 2> /var/log/freeze; umount /pracownia && exit || exit\n";
              // update_log(polecenie);
                clients[i]->socket->write(polecenie.toUtf8());
               clients[i]->socket->flush();
           }
        }
}

void serwer::on_synchronizuj_wszystkie_clicked()
{
    int ilosc = ui->clients->count();
    for(int i = 0; i<ilosc; i++)
    {
        QString ktory = ui->clients->itemText(i);
        ping_database();
        //qInfo()<< "Accessing database to run query......"<<ok;
        QSqlQuery data_for_client;
        data_for_client.exec("SELECT katalog FROM komputery WHERE hostname='"+ktory+"';");
        data_for_client.next();
        QString struktura = QString::fromStdString(getHome(ktory));
        QString katalog = data_for_client.value(0).toString();
        int ile = clients.size();
        for(int i = 0; i<ile; i++)
            {
               if(ktory == clients[i]->Client_Name)
               {
                   QString polecenie = "mount "+katalog+" /pracownia; id=$(btrfs sub list /pracownia | grep 'home$' | awk '{ print $2 }'); umount /pracownia; mount -t btrfs -o subvolid=$id "+katalog+" /pracownia; sshpass -p '"+distributionpass+"' rsync -aogu -e 'ssh -i /root/id_rsa -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -p "+ssh_port+"' distribution@`:"+struktura+"/ /pracownia/ --delete-before --ignore-errors 2> /var/log/freeze; umount /pracownia && exit || exit\n";
                   // QString polecenie = "mount -t btrfs -o subvol=@/home "+katalog+" /pracownia; sshpass -p '"+distributionpass+"' rsync -aogu -e 'ssh -i /root/id_rsa -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -p "+ssh_port+"' distribution@`:"+struktura+"/ /pracownia/ --delete-before --ignore-errors 2> /var/log/freeze; umount /pracownia && exit || exit\n";
                 // update_log(polecenie);
                    clients[i]->socket->write(polecenie.toUtf8());
                   clients[i]->socket->flush();
               }
            }
    }
}

void serwer::on_informacje_clicked()
{
  QMessageBox::information(this,tr("Information"),tr("Autorzy Jakub Wolski i Jakub Popławski.<br/>Kod źródłowy na licencji GPLv3: <a href=\"https://github.com/123kubix123/pracownia-serwer\">https://github.com/123kubix123/pracownia-serwer</a> <br/>Development-release <a href=\"https://pracownia.tuxdev.com/\">https://pracownia.tuxdev.com/</a>"));
}

void serwer::on_wykonaj_komende_clicked()
{
     // run command from panel
    QString ktory = ui->clients->currentText();
    if(ktory != "")
    {
    ping_database();
    //qInfo()<< "Accessing database to run query......"<<ok;
    QSqlQuery get_command_for_client;
    get_command_for_client.exec("SELECT bash FROM komputery WHERE hostname='"+ktory+"';");
    get_command_for_client.next();
    QString command_for_client = get_command_for_client.value(0).toString();
    if(command_for_client!="")
    {
        int ile = clients.size();
        for(int i = 0; i<ile; i++)
            {
               if(ktory == clients[i]->Client_Name)
               {
                   update_log("Komputer: "+ktory+" zlecono wykonanie: "+command_for_client);
                   command_for_client.prepend("z_panelu:");
                   command_for_client.append(" && exit || exit\n");
                   clients[i]->socket->write(command_for_client.toUtf8());
                   clients[i]->socket->flush();

                   QSqlQuery clear_command_for_client;
                   clear_command_for_client.exec("UPDATE komputery set bash='' WHERE hostname='"+ktory+"';");
                   get_command_for_client.next();
               }
            }
    }
    }
}

void serwer::on_fix_permisions_clicked()
{
    std::string cmd= "setfacl -R -m u:distribution:r-x $(grep distribution /etc/passwd | cut -d: -f6) && echo true || echo false \n";
    bash_root->write(cmd.c_str());;
    bash_root->waitForBytesWritten();
    bash_root->waitForReadyRead();
    QString zwrot = bash_root->readAllStandardOutput();
    zwrot.replace(" ","");
    zwrot.replace("\n","");
    if(zwrot == "true")
    {
        update_log("Naprawiono uprawnienia struktury");
    }
    else
    {
        update_log("Naprawa uprawnień struktury nie powiodła się");
    }
}

void serwer::on_log_textChanged()
{
    // narazie nie używana bo nie pozwalało na scrolowanie zmieniłem na serwer::update_log(QString new_content)
}

void serwer::on_open_panel_clicked()
{
    Panel_window->load(host,http_port);
    Panel_window->show();
}




void serwer::on_wykonaj_wszystkie_clicked()
{
    int ilosc = ui->clients->count();
    for(int i = 0; i<ilosc; i++)
    {
        QString ktory = ui->clients->itemText(i);
        if(ktory != "")
        {
        ping_database();
        //qInfo()<< "Accessing database to run query......"<<ok;
        QSqlQuery get_command_for_client;
        get_command_for_client.exec("SELECT bash FROM komputery WHERE hostname='"+ktory+"';");
        get_command_for_client.next();
        QString command_for_client = get_command_for_client.value(0).toString();
        if(command_for_client!="")
            {
                int ile = clients.size();
                for(int i = 0; i<ile; i++)
                    {
                       if(ktory == clients[i]->Client_Name)
                       {
                           update_log("Komputer: "+ktory+" zlecono wykonanie: "+command_for_client);
                           command_for_client.prepend("z_panelu:");
                           command_for_client.append(" && exit || exit\n");
                           clients[i]->socket->write(command_for_client.toUtf8());
                           clients[i]->socket->flush();

                           QSqlQuery clear_command_for_client;
                           clear_command_for_client.exec("UPDATE komputery set bash='' WHERE hostname='"+ktory+"';");
                           get_command_for_client.next();
                       }
                    }
            }
        }
    }
}

void serwer::update_log(QString new_content)
{
    if(new_content != "")
    {
    log1+= new_content+"<br/>";
    ui->log->setHtml(log1);
    log_scrollbar->setValue(log_scrollbar->maximum());
    }
}

void serwer::on_aktualizuj_clicked()
{

}

void serwer::on_autoryzuj_clicked()
{
    bash_root->write("pkexec su \n");
    bash_root->waitForBytesWritten();
    bash_root->write("whoami \n");
    bash_root->waitForBytesWritten();
    bash_root->waitForReadyRead();
    QString whoami = bash_root->readAllStandardOutput();
    whoami.replace(" ","");
    whoami.replace("\n","");
    if(whoami == "root")
    {
        qInfo()<< "Autoryzacja zakonczona..."<<whoami;
        lock_interface(false);
        setup();
        // odblokowanie interfejsu
    }
    else
    {
        qInfo()<< "Autoryzacja nieudana..."<<whoami;
        lock_interface(true);
        //interfejs zablokowany
    }
}

void serwer::lock_interface(bool czy)
{
    if(czy == true)
    {
       qInfo()<< "blokada interfejsu...";
        ui->siec->setEnabled(false);
        ui->znajdz->setEnabled(false);
        ui->wybierz->setEnabled(false);
        ui->kopiuj->setEnabled(false);
        ui->install_pass->setEnabled(false);
        ui->fix_permisions->setEnabled(false);
        ui->aktualizuj->setEnabled(false);

        ui->autoryzuj->setEnabled(true);
    }
    else
    {
        qInfo()<< "odblokowanie interfejsu...";
        ui->siec->setEnabled(true);
        ui->znajdz->setEnabled(true);
        ui->wybierz->setEnabled(true);
        ui->kopiuj->setEnabled(true);
        ui->install_pass->setEnabled(true);
        ui->fix_permisions->setEnabled(true);
        ui->aktualizuj->setEnabled(true);

        ui->autoryzuj->setEnabled(false);
    }
}
