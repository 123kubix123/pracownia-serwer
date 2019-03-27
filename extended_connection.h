#ifndef EXTENDED_CONNECTION_H
#define EXTENDED_CONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>

class extended_connection : public QObject
{
    Q_OBJECT
public:
    explicit extended_connection(QObject *parent = 0);
    ~extended_connection();

public:
    QTcpSocket *socket;
    QString unique_ID;
    QString Client_Name;
    bool is_disconnected;
    bool data_avilable;

signals:
    void connection_ended();
    void ready_read();

public slots:
    void connector();
    void sig_emiter();
    void read_emiter();
};

#endif // EXTENDED_CONNECTION_H
