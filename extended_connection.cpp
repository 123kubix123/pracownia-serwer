#include "extended_connection.h"

extended_connection::extended_connection(QObject *parent) : QObject(parent)
{
     this->is_disconnected = false;
    this->data_avilable = false;
}

extended_connection::~extended_connection()
{
    // zakomentowane bo narusza ochronę pamięci
    //delete this->socket;
}

void extended_connection::connector()
{
    connect(socket, SIGNAL(disconnected()), this, SLOT(sig_emiter()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(read_emiter()));
}

void extended_connection::sig_emiter()
{
    this->is_disconnected = true;
    emit connection_ended();
}

void extended_connection::read_emiter()
{
    this->data_avilable = true;
    emit ready_read();
}
