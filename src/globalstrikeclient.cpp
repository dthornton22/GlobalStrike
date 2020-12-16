#include "globalstrikeclient.h"

GlobalStrikeClient::GlobalStrikeClient()
{
	QDateTime now;
	file.setFileName("client_log.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "Logging failed.";
		return;
	}
	file.write(QString(" ----- Start of Log " + now.currentDateTime().toString() + " ----- \n").toUtf8().constData());
	file.close();
	client = new QTcpSocket;
	client->setProxy(QNetworkProxy::NoProxy); // network patch
	takenCountryNames.append(QString("Atlantis"));
	takenCountryNames.append(QString("Cobra"));
	takenCountryNames.append(QString("Republic"));
	takenCountryNames.append(QString("West"));
	takenCountryNames.append(QString("Zathura"));
	for (int i = 0; i < 5; i++)
	{
		targetedBy[i] = false;
	}
	connect(client, SIGNAL(readyRead()), this, SLOT(processPendingMessages())); // receipt from server
}

GlobalStrikeClient::~GlobalStrikeClient()
{
	if (client->state() == QAbstractSocket::ConnectedState)
		client->close();
	delete client;
}

bool GlobalStrikeClient::connectToServer(QString ipAddress, quint16 portNum)
{
	qDebug() << "Client: Attempting connection to" << ipAddress << "at port" << portNum;
	LogEvent(QString("Client: Attempting connection to " + ipAddress));
	if (client == nullptr)
	{
		qDebug() << "Client: Connection failed because client was not a valid object.";
		LogEvent(QString("Client: Connection failed because client was not a valid object."));
		return false;
	}
	client->connectToHost(ipAddress, portNum);
	if (!client->waitForConnected())
	{
		qDebug() << "Client: Failed to connect to server."
			<< " Attempting disconnect now.";
		LogEvent(QString("Client: Failed to connect to server. Attempting disconnect now."));
		emit(connectionLost());
	}
	handshake();
	qDebug() << "Client: Connected to" << ipAddress << "at port" << portNum;
	LogEvent(QString("Client: Connected to " + ipAddress + "at port" + QString::number(portNum)));
	connect(client, SIGNAL(readyRead()), this, SLOT(processPendingMessages()));
	ingame = false;
	return true;
}

void GlobalStrikeClient::launchCommand(QString target)
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "L" << endl;
	out << target << endl; // target
	out << playerName << endl; // origin
	out << takenCountryNames.at(playerNames.indexOf(target)) << endl; // target country
	out << takenCountryNames.at(playerNames.indexOf(playerName)) << endl; // origin country
	out << "Z" << endl;
	writeMessage(message);
}

void GlobalStrikeClient::interceptCommand(QString target)
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "I" << endl;
	out << target << endl;
	out << playerName << endl;
	out << takenCountryNames.at(playerNames.indexOf(target)) << endl;
	out << takenCountryNames.at(playerNames.indexOf(playerName)) << endl;
	out << "Z" << endl;
	writeMessage(message);
}

void GlobalStrikeClient::yieldKillPoint(QString target)
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "S" << endl;
	out << target << endl;
	out << playerNames.at(takenCountryNames.indexOf(target)) << endl;
	out << "Z" << endl;
	writeMessage(message);
}

void GlobalStrikeClient::readyToggle()
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "R" << endl;
	out << playerName << endl;
	out << "Z" << endl;
	writeMessage(message);
}

void GlobalStrikeClient::handshake()
{
	QByteArray hand;
	QTextStream out(&hand, QIODevice::WriteOnly);
	out << password << endl;
	out << playerName << endl;
	LogEvent(playerName + " " + password);
	writeMessage(hand);
}

void GlobalStrikeClient::disconnectRequest()
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "B" << endl;
	out << playerName << endl;
	out << "Z" << endl;
	writeMessage(message);
}

void GlobalStrikeClient::writeMessage(QByteArray message)
{
	if (client->state() == QAbstractSocket::ConnectedState)
	{
		client->write(message);
		client->flush();
		return;
		// LogEvent(message);
	}
	qDebug() << "Connection Lost - Ending Game";
	LogEvent(QString("Connection lost, ending game/leaving lobby"));
	emit(connectionLost());
}

void GlobalStrikeClient::processPendingMessages()
{
	QString ch, temp, temp2;
	QByteArray input = client->readAll();
	LogEvent(input);
	QTextStream textData(&input, QIODevice::ReadOnly);
	while (!textData.atEnd())
	{
		textData >> ch;
		qDebug() << ch;
		switch (ch.front().toLatin1())
		{
		case 'L':
			processLaunch(textData);
			break;
		case 'I':
			processIntercept(textData);
			break;
		case 'S':
			processScoreUpdates(textData);
			break;
		case 'R':
			processReadyToggleData(textData);
			break;
		case 'G':
			processGreeting(textData);
			break;
		case 'D':
			processDeny(textData);
			break;
		case 'N':
			processNewClient(textData);
			// processing new connections, to extract a String and no whitespace
			// textData >> qstringVariable; reads exactly one string worth and no \n's
			break;
		case 'B':
			processDisconnectRequest(textData);
			break;
		case 'X':
			ingame = true;
			emit(gameIsStarting());
			break;
		case 'Z':
			qDebug() << "Client: Parsing input concluded.";
			LogEvent(QString("Client: Parsing input concluded."));
		default:
			break;
		}
	}
}

void GlobalStrikeClient::processLaunch(QTextStream& textData)
{
	QString targetCountry, originCountry;
	textData >> targetCountry;
	textData >> originCountry;
	LogEvent(QString("Client: Launched at " + targetCountry + "."));
	whichCountry(targetCountry, true, targetCountry);
	whichCountry(originCountry, false, targetCountry);
	if (takenCountryNames.at(playerNames.indexOf(playerName)) == targetCountry)
	{
		qDebug() << "Youre being targeted!!!";
		pendingMissile missile;
		missile.shotTimer = new QTimer;
		connect(missile.shotTimer, SIGNAL(timeout()), this, SLOT(Dequeue()));
		missile.shotTimer->setSingleShot(true);
		missile.shotTimer->start(3000);
		targetedBy[takenCountryNames.indexOf(originCountry)] = true;
		missile.originCountry = originCountry;
		missileQueue.enqueue(missile);
	}
}

void GlobalStrikeClient::processDisconnectRequest(QTextStream& textData)
{
	QString pname;
	textData >> pname;
	qDebug() << QString("Client: Player " + pname + " has left the game server.");
	LogEvent("Client: Player " + pname + " has left the game server.");
	if (ingame)
	{
		playerNames.replace(playerNames.indexOf(pname), "Unset");
		return;
	}
	playerNames.clear();
	emit(lobbyChanged());
}

void GlobalStrikeClient::whichCountry(QString target, bool flag, QString lastTarget)
{
	qDebug() << "Selecting animation for " + target;
	LogEvent(QString("Selecting animation for " + target));
	int x, y;
	if (flag)
	{
		lastTarget = target;
		if (target == "Atlantis")
		{
			qDebug() << "Atlantis was targeted";
			LogEvent(QString("Atlantis was targeted"));
			emit(animateWhite());
		}
		else if (target == "Cobra")
		{
			qDebug() << "Cobra was targeted";
			LogEvent(QString("Cobra was targeted"));
			emit(animateOrange());
		}
		else if (target == "Republic")
		{
			qDebug() << "Republic was targeted";
			LogEvent(QString("Republic was targeted"));
			emit(animateGreen());
		}
		else if (target == "West")
		{
			qDebug() << "West was targeted";
			LogEvent(QString("West was targeted"));
			emit(animateYellow());
		}
		else if (target == "Zathura")
		{
			qDebug() << "Zathura was targeted";
			LogEvent(QString("Zathura was targeted"));
			emit(animateBlack());
		}
		return;
	}
	if (lastTarget == "Atlantis")
	{
		x = 680;
		y = 120;
		qDebug() << "Atlantis fired a missile";
		LogEvent(QString("Atlantis fired a missile"));
	}
	else if (lastTarget == "Cobra")
	{
		x = 530;
		y = 350;
		qDebug() << "Cobra fired a missile";
		LogEvent(QString("Cobra fired a missile"));
	}
	else if (lastTarget == "Republic")
	{
		x = 400;
		y = 90;
		qDebug() << "Republic fired a missile";
		LogEvent(QString("Republic fired a missile"));
	}
	else if (lastTarget == "West")
	{
		x = 270;
		y = 360;
		qDebug() << "West fired a missile";
		LogEvent(QString("West fired a missile"));
	}
	else if (lastTarget == "Zathura")
	{
		x = 100;
		y = 140;
		qDebug() << "Zathura fired a missile";
		LogEvent(QString("Zathura fired a missile"));
	}
	if (target == "Atlantis")
		emit(animateLaunchWhite(x, y));
	else if (target == "Cobra")
		emit(animateLaunchOrange(x, y));
	else if (target == "Republic")
		emit(animateLaunchGreen(x, y));
	else if (target == "West")
		emit(animateLaunchYellow(x, y));
	else if (target == "Zathura")
		emit(animateLaunchBlack(x, y));
}

void GlobalStrikeClient::processIntercept(QTextStream& textData)
{
	QString targetCountry, originCountry;
	textData >> targetCountry; // original launcher of missile
	textData >> originCountry; // launcher of intercept
	LogEvent(QString("Client: Interceptor launched at " + targetCountry + "."));
	for (int i = 0; i < missileQueue.size(); i++)
	{
		if (missileQueue.at(i).originCountry == targetCountry)
		{
			pendingMissile missileEvent = missileQueue.at(i);
			missileEvent.shotTimer->stop();
			missileQueue.removeAt(i);
			delete missileEvent.shotTimer;
			targetedBy[takenCountryNames.indexOf(originCountry)] = false;
			break;
		}
	}
	emit(explodeMissile(targetCountry));
}

void GlobalStrikeClient::processScoreUpdates(QTextStream& textData)
{
	QString pname;
	textData >> pname;
	emit(scoreUpdate(pname));
}

void GlobalStrikeClient::processReadyToggleData(QTextStream& textData)
{
	QString pname;
	textData >> pname;
	if (!ingame)
	{
		emit(readyDBToggle(pname));
	}
}

void GlobalStrikeClient::processGreeting(QTextStream& textData)
{
	QString ipAddress;
	quint16 portNum;
	textData >> ipAddress;
	textData >> portNum;
	textData >> playerID;
	emit(lobbyIPUpdate(ipAddress, portNum));
}

void GlobalStrikeClient::processDeny(QTextStream& textData)
{
	emit(deny());
}

void GlobalStrikeClient::processNewClient(QTextStream& textData)
{
	QString pname;
	textData >> pname;
	playerNames.append(pname);
	if (!ingame)
	{
		emit(newClientDB(pname));
	}
}

void GlobalStrikeClient::LogEvent(QByteArray data)
{
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
	{
		qDebug() << "Logging failed.";
		return;
	}
	file.write(QString("|" + QString::number(lineNum++) + "| ").toUtf8().constData());
	file.write(data.replace('\n', ' '));
	file.write(QString("\n").toUtf8().constData());
	file.close();
}

void GlobalStrikeClient::LogEvent(QString data)
{
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
	{
		qDebug() << "Logging failed.";
		return;
	}
	file.write(QString("|" + QString::number(lineNum++) + "| " + data + "\n").toUtf8().constData());
	file.close();
}

void GlobalStrikeClient::Dequeue()
{
	pendingMissile missile = missileQueue.dequeue();
	missile.shotTimer->stop();
	delete missile.shotTimer;
	yieldKillPoint(missile.originCountry);
}
