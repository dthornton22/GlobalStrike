#ifndef GLOBALSTRIKECLIENT_H
#define GLOBALSTRIKECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QNetworkProxy>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <QDateTime>
#include <QQueue>
#include "sqldatabase.h"
#include "player.h"

class GlobalStrikeClient : public QObject
{
	Q_OBJECT

public:
	GlobalStrikeClient();
	~GlobalStrikeClient();

signals:
	void deny();
	void lobbyIPUpdate(QString, quint16);
	void newClientDB(QString);
	void readyDBToggle(QString);
	void scoreUpdate(QString);
	void lobbyChanged();
	void gameIsStarting();
	void animateWhite();
	void animateOrange();
	void animateGreen();
	void animateYellow();
	void animateBlack();
	void animateLaunchWhite(int, int);
	void animateLaunchOrange(int, int);
	void animateLaunchGreen(int, int);
	void animateLaunchYellow(int, int);
	void animateLaunchBlack(int, int);
	void explodeMissile(QString);
	void connectionLost();

public slots:
	bool connectToServer(QString ipAddress, quint16 portNum);
	void launchCommand(QString);
	void interceptCommand(QString);
	void yieldKillPoint(QString);
	void readyToggle();
	void handshake();
	void disconnectRequest();
	void processPendingMessages();
	void Dequeue();

private:
	void writeMessage(QByteArray message);
	void processLaunch(QTextStream&);
	void processIntercept(QTextStream&);
	void processScoreUpdates(QTextStream&);
	void processReadyToggleData(QTextStream&);
	void processGreeting(QTextStream&);
	void processDeny(QTextStream&);
	void processNewClient(QTextStream&);
	void processDisconnectRequest(QTextStream&);
	void LogEvent(QByteArray);
	void LogEvent(QString);
	void whichCountry(QString, bool, QString);

public:
	int playerID;
	QString playerName;
	QString password;
	QTcpSocket* client;
	QFile file;
	quint16 lineNum = 1;
	QDateTime now;
	quint16 portNum;
	QString ipAddress;
	QList<QString> takenCountryNames;
	QList<QString> playerNames;
	bool targetedBy[5];
	bool ingame;
	struct pendingMissile
	{
		QString originCountry;
		QTimer* shotTimer;
	};
	QQueue<pendingMissile> missileQueue;
};

#endif // GLOBALSTRIKECLIENT_H
