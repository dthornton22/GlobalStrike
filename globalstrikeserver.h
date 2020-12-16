#ifndef GLOBALSTRIKESERVER_H
#define GLOBALSTRIKESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QTextStream>
#include <QFile>
#include <QAbstractSocket>
#include <QDateTime>
#include <globalstrikeclient.h>

class GlobalStrikeServer : public QObject
{
	Q_OBJECT

public:
	GlobalStrikeServer();
	GlobalStrikeServer(GlobalStrikeClient* client);
	~GlobalStrikeServer();
	void processNewConnection(QString);  // QString while testing
	void setDatabase(SQLDatabase*);

signals:
	void serverReady(QString, quint16);

public slots:
	void processMessages(QTcpSocket*);
	void processPendingConnection();
	void processDisconnectRequest();
	void debuggingReadyUp();
	void endgame();

private:
	void startServer();
	void closeServer();
	void resetServer();
	void writeMessage(QByteArray, QTcpSocket*);
	void writeGreeting(QTcpSocket*, int);
	void writeReadyData(QTcpSocket*, QString);
	void writeDeny(int, QTcpSocket*);
	void writeLaunch(QTcpSocket*, QString, QString);
	void writeIntercept(QTcpSocket*, QString, QString);
	void writeScoreBoard(QString);
	void writeNewConnection(QTcpSocket*, QString);
	void writeDisconnectRequest(QTcpSocket*, QString);
	void writeStartGame();
	bool processHandshake(QByteArray&, QTextStream&, QTcpSocket*);
	void processLaunchCommand(QTextStream&);
	void processInterceptCommand(QTextStream&);
	void processYieldedKillPoint(QTextStream&);
	void processReadyToggleCommand(QTextStream&);
	void processDisconnectRequest(QTextStream&);
	void LogEvent(QByteArray);
	void LogEvent(QString);
	QTcpServer* server;
	GlobalStrikeClient* hostClient;
	QString ipAddress;
	quint16 portNum;
	QString password;
	qint64 pLength;
	QList<QString> playerNames;
	QFile file;
	quint16 lineNum = 1;
	QDateTime now;
	QList<QTcpSocket*> playerSockets;
	bool closingServer = false;
	bool ingame = false;
	QList<bool> readyStatusFlags;
	QList<QString> countrySelections;
	int scores[5];
	SQLDatabase* d;
};

#endif // GLOBALSTRIKESERVER_H
