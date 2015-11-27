

#include <QtNetwork>
#include <QtGui>
#include "MyClient.h"
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QChar>
//-----------------------------------------------------------------------
//вычисляет число num  в степени step
int stepen(int num,int step)
{
    int    ans = 1;

    for(int j = 0; j < step;j++)
        ans = ans*num;

    return ans;
}

//с помощью этой функции кодируется сообщение
int* getMessage(int* code,QString* symbol)
{
int l = 0;

int ex;

char sy = ' ';

QChar* s = symbol->data();//получаем указатель на первый символ в строке

if( !s->isNull() )
 sy = s->toLatin1();//приводим его к стандартным символам си
else return NULL;

int sym =static_cast<int>(sy);//получаем его числовое значение

int* message = new int[32];

for(int j = 7; j > -1;j--)//вычисляем числа для сумматора
    {
        ex = stepen(2,j);

        ex = ex & sym;

        if (ex == 0)
            ex = -1;
        else
            ex = 1;

        for(int k = 0; k < 4;k++)
        {
            message[l]=ex*code[l];
            l++;
        }


    }
return message;
}





// ----------------------------------------------------------------------
MyClient::MyClient(const QString& strHost , int nPort, QWidget* pwgt /*=0*/) : QWidget(pwgt), m_nNextBlockSize(0)
{
  //создаем сокет и коннектимся к хосту по адресу strHost к порту nPort
    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost(strHost, nPort);

    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));

    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));

    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)) );

    connect(this ,SIGNAL(destroyed()),SLOT(CloseSocket));

    m_ptxtInfo  = new QTextEdit;

// объект ввода текста. В программе отсутствует проверка входных данных - вводить надо только латинские буквы!!!
    m_ptxtInput = new QLineEdit;

    connect(m_ptxtInput, SIGNAL(returnPressed()), this, SLOT(slotSendToServer()));

    m_ptxtInfo->setReadOnly(true);

//настраиваем диалоговое окно
    QPushButton* pcmd = new QPushButton("&Send");

    connect(pcmd, SIGNAL(clicked()), SLOT(slotSendToServer()));

    QVBoxLayout* pvbxLayout = new QVBoxLayout;

    pvbxLayout->addWidget(new QLabel("<H1>Client</H1>"));

    pvbxLayout->addWidget(m_ptxtInfo);

    pvbxLayout->addWidget(m_ptxtInput);

    pvbxLayout->addWidget(pcmd);

    setLayout(pvbxLayout);
}

// ----------------------------------------------------------------------
void MyClient::slotReadyRead()
{
//считываем код, отправленный сумматором
    QDataStream in(m_pTcpSocket);

    in.setVersion(QDataStream::Qt_4_7);

//проверяем, а все ли данные нам пришли?
    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < (int)sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }

        QTime time;

//считываем пришедшие данные
        code = new int[32];

        qDebug()<<"Code: ";

        for( int i =0 ; i < 32 ; i++ )
        {

        in >>  code[i];

        qDebug() <<code[i]<<" ";

        }
        qDebug()<<endl;

        m_ptxtInfo->append(time.toString() + " Пришел код ");

        m_nNextBlockSize = 0;
    }
}

// ----------------------------------------------------------------------
void MyClient::slotError(QAbstractSocket::SocketError err)
{
    QString strError = 
        "Error: " + (err == QAbstractSocket::HostNotFoundError ? 
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ? 
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ? 
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );
    m_ptxtInfo->append(strError);
}

// ----------------------------------------------------------------------
void MyClient::slotSendToServer()
{
//отправляем серверу закадированное сообщение
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_4_7);

    out << quint16(0);

    message = getMessage(code,&m_ptxtInput->text());

    qDebug()<<"Message: ";

    for(int j = 0; j < 32;j++)
    {
        out << message[j];
        qDebug() << message[j]<<" ";
    }

    qDebug()<<endl;

    out.device()->seek(0);

    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);

    m_ptxtInput->setText("");
}

// ------------------------------------------------------------------
void MyClient::slotConnected()
{
    m_ptxtInfo->append("Received the connected() signal");
}
void MyClient::CloseSocket()
{
  m_pTcpSocket->disconnect();
}
