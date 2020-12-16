#include "globalstrikeserver.h"

GlobalStrikeServer::GlobalStrikeServer()
{
	file.setFileName("server_log.txt");
	server = new QTcpServer(this);
	hostClient = nullptr;
}

GlobalStrikeServer::GlobalStrikeServer(GlobalStrikeClient* client)
{
	QDateTime now;
	file.setFileName("server_log.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "logging failed.";
		return;
	}
	file.write(QString(" ----- Start of Log " + now.currentDateTime().toString() + " ----- \n").toUtf8().constData());
	file.close();
	server = new QTcpServer(this);
	hostClient = client;
	password = hostClient->password;
	pLength = password.length();
	//setDatabase(d2);
	countrySelections.append(QString("Atlantis"));
	countrySelections.append(QString("Cobra"));
	countrySelections.append(QString("Republic"));
	countrySelections.append(QString("West"));
	countrySelections.append(QString("Zathura"));
	for (int i = 0; i < 5; i++)
	{
		readyStatusFlags.append(false);
		scores[i] = 0;
	}
	startServer(); // maybe remove this one?
}

GlobalStrikeServer::~GlobalStrikeServer()
{
	closeServer();
	delete server;
}

void GlobalStrikeServer::startServer()
{
	QList<QHostAddress> possibleAddresses = QNetworkInterface::allAddresses();
	const QHostAddress& localhost = QHostAddress::LocalHost; // if this fails try other version
	for (int i = 0; i < possibleAddresses.size(); i++)
	{
		//for (const QHostAddress &address: QNetworkInterface::allAddresses())
		if (possibleAddresses.at(i).toIPv4Address() && possibleAddresses.at(i) != localhost)
		{
			//if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
				 //ipAddress = address.toString();
			ipAddress = possibleAddresses.at(i).toString();
			qDebug() << "Your IP is:" << ipAddress;
			LogEvent(ipAddress + " was found.");
			break;
		}
	}
	if (ipAddress.isEmpty())
	{
		ipAddress = localhost.toString();
		qDebug() << "Your IP is:" << ipAddress;
		LogEvent(ipAddress + " was found and used.");
	}
	if (!server->listen(QHostAddress(ipAddress)))
	{
		closeServer();
		qDebug() << "Server cannot listen on IP Address/Port Number";
		LogEvent(QString("Server cannot listen on IP Address/Port Number"));
		return;
	}
	else
	{
		connect(this, SIGNAL(serverReady(QString, quint16)),
			hostClient, SLOT(connectToServer(QString, quint16)));
		connect(server, SIGNAL(newConnection()), this, SLOT(processPendingConnection()));
		qDebug() << "Server successfully created.";
		LogEvent(QString("Server successfully created."));
		portNum = server->serverPort();
		//d = new SQLDatabase;
		emit(serverReady(ipAddress, server->serverPort()));
	}
}

void GlobalStrikeServer::closeServer()
{
	closingServer = true;
	qDebug() << "Beginning server closure.";
	LogEvent(QString("Beginning server closure."));
	LogEvent(QString("Disconnecting all players."));
	qDebug() << "Disconnecting all players.";
	for (int i = playerSockets.size() - 1; i >= 1; i--)
	{
		if (playerSockets.at(i) != nullptr)
		{
			playerSockets.at(i)->disconnectFromHost();
			LogEvent(QString("Disconnected player " + QString::number(playerSockets.size())));
			delete playerSockets.at(i);
			playerSockets.removeAt(i);
		}
	}
	server->close();
	closingServer = false;
}

void GlobalStrikeServer::resetServer()
{
	// do stuff
}

void GlobalStrikeServer::writeMessage(QByteArray message, QTcpSocket* serverclient)
{
	if (serverclient->state() == QAbstractSocket::ConnectedState)
	{
		serverclient->write(message);
		serverclient->flush();
		LogEvent(message);
	}
}

void GlobalStrikeServer::writeGreeting(QTcpSocket* serverclient, int pID)
{
	QByteArray greet;
	QTextStream out(&greet, QIODevice::WriteOnly);
	out << QString("G") << endl;
	out << ipAddress << endl;
	out << (unsigned int)portNum << endl;
	out << pID << endl;
	out << "Z" << endl;
	// once was out << "G" << endl << "Welcome! -ipAddress- -portNum-" << endl << pID << endl;"
	writeMessage(greet, serverclient);
	for (int i = 0; i < playerNames.size(); i++)
	{
		if (readyStatusFlags.at(i))
		{
			writeNewConnection(serverclient, playerNames.at(i));
			writeReadyData(serverclient, playerNames.at(i));
		}
		else
		{ // need to revisit write greeting
			writeNewConnection(serverclient, playerNames.at(i));
		}
		if (playerSockets.at(i) != serverclient)
		{
			writeNewConnection(playerSockets.at(i), playerNames.last());
		}
	}
}

void GlobalStrikeServer::writeReadyData(QTcpSocket* serverclient,
	QString playerName)
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "R" << endl;
	out << playerName << endl;
	out << "Z" << endl;
	writeMessage(message, serverclient);
}

void GlobalStrikeServer::writeDeny(int i, QTcpSocket* serverclient)
{
	QByteArray deny;
	QTextStream out(&deny, QIODevice::WriteOnly);
	out << (QString("D")) << endl;
	switch (i)
	{
	case 0:
		out << (QString("Denied. Incorrect Password.")) << endl;
		LogEvent(QString("Socket was denied on account of incorrect password."));
		qDebug() << "Socket was denied on account of incorrect password.";
		break;
	case 1:
		out << (QString("Denied. Non-unique Username.")) << endl;
		LogEvent(QString("Socket was denied on account of nonunique username."));
		qDebug() << "Socket was denied on account of nonunique username.";
		break;
	case 2:
		out << (QString("Denied. Max Server Capacity Reached.")) << endl;
		LogEvent(QString("Socket was denied on account of max server capacity."));
		qDebug() << "Socket was denied on account of max server capacity.";
		break;
	case 3:
		out << (QString("Denied. Game in progress.")) << endl;
		LogEvent(QString("Socket was denied on account of game in progress."));
		qDebug() << "Socket was denied on account of game in progress.";
	}
	out << "Z" << endl;
	writeMessage(deny, serverclient);
}

void GlobalStrikeServer::writeDisconnectRequest(QTcpSocket* serverclient, QString pname)
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "B" << endl;
	out << pname << endl;
	out << "Z" << endl;
	writeMessage(message, serverclient);
	for (int i = 0; i < playerSockets.size(); i++)
	{
		if (playerNames.at(i) == pname)
			continue;
		if (readyStatusFlags.at(i))
		{
			writeNewConnection(serverclient, playerNames.at(i));
			writeReadyData(serverclient, playerNames.at(i));
		}
		else
		{
			writeNewConnection(serverclient, playerNames.at(i));
		}
		if (scores[i] > 0)
			writeScoreBoard(playerNames.at(i));
	}
}

void GlobalStrikeServer::writeLaunch(QTcpSocket* serverclient, QString tCountry, QString oCountry)
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "L" << endl;
	out << tCountry << endl;
	out << oCountry << endl;
	out << "Z" << endl;
	writeMessage(message, serverclient);
}

void GlobalStrikeServer::writeIntercept(QTcpSocket* serverclient, QString tCountry, QString oCountry)
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "I" << endl;
	out << tCountry << endl;
	out << oCountry << endl;
	out << "Z" << endl;
	writeMessage(message, serverclient);
}

void GlobalStrikeServer::writeScoreBoard(QString target)
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "S" << endl;
	out << target << endl;
	out << "Z" << endl;
	for (int i = 0; i < playerSockets.size(); i++)
		writeMessage(message, playerSockets.at(i));
}

void GlobalStrikeServer::writeNewConnection(QTcpSocket* serverclient, QString pname)
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "N" << endl;
	out << pname << endl;
	out << "Z" << endl;
	writeMessage(message, serverclient);
}

void GlobalStrikeServer::writeStartGame()
{
	QByteArray message;
	QTextStream out(&message, QIODevice::WriteOnly);
	out << "X" << endl;
	out << "Z" << endl;
	for (int i = 0; i < playerSockets.size(); i++)
	{
		if (playerSockets.at(i) != nullptr)
			writeMessage(message, playerSockets.at(i));
	}
}

void GlobalStrikeServer::processPendingConnection()
{
	QTcpSocket* temp = server->nextPendingConnection();
	qDebug() << "Pending connection detected.";
	LogEvent(QString("Pending connection detected."));
	temp->waitForReadyRead(2000);
	QByteArray input = temp->readAll();
	QTextStream textData(&input, QIODevice::ReadOnly);
	if (processHandshake(input, textData, temp))
	{
		qDebug() << "Server:New connection successful.";
		LogEvent(QString("Server:New connection successful."));
		connect(temp, &QTcpSocket::readyRead, [=] { processMessages(temp);});
		connect(temp, SIGNAL(disconnected()), this, SLOT(processDisconnectRequest()));
	}
	else
	{
		temp->disconnectFromHost();
	}
}

bool GlobalStrikeServer::processHandshake(QByteArray& input,
	QTextStream& textData, QTcpSocket* temp)
{
	QString pword, pname, endlBuffer;
	LogEvent(QString("New handshake received at " + now.currentDateTime().toString() + ". Details following:"));
	textData >> pword;

	if (pword != password)
	{
		writeDeny(0, temp); // wrong password
		return false;
	}
	textData >> pname;
	if (playerNames.contains(pname))
	{
		writeDeny(1, temp); // recurring username
		return false;
	}
	LogEvent(pname + " " + pword);
	if (playerNames.size() >= 5)
	{
		writeDeny(2, temp); // full lobby
		return false;
	}
	if (ingame)
	{
		writeDeny(3, temp);
		return false;
	}
	playerNames.append(pname);
	playerSockets.append(temp);
	writeGreeting(temp, playerNames.size());
	return true;
}

void GlobalStrikeServer::processLaunchCommand(QTextStream& textData)
{
	QString target, tCountry;
	QString origin, oCountry;
	textData >> target;
	textData >> origin;
	textData >> tCountry;
	textData >> oCountry;
	if (playerNames.contains(target) && playerNames.contains(origin))
	{
		for (int i = 0; i < playerSockets.size(); i++)
		{
			writeLaunch(playerSockets.at(i), tCountry, oCountry);
		}
	}
}

void GlobalStrikeServer::processInterceptCommand(QTextStream& textData)
{
	QString target, tCountry;
	QString origin, oCountry;
	textData >> target;
	textData >> origin;
	textData >> tCountry;
	textData >> oCountry;
	if (playerNames.contains(target) && playerNames.contains(origin))
	{
		for (int i = 0; i < playerSockets.size(); i++)
			writeIntercept(playerSockets.at(i), tCountry, oCountry);
	}
}

void GlobalStrikeServer::processYieldedKillPoint(QTextStream& textData)
{
	QString playerName, target;
	textData >> target;
	textData >> playerName;
	scores[playerNames.indexOf(playerName)]++;
	writeScoreBoard(playerName);
}

void GlobalStrikeServer::processReadyToggleCommand(QTextStream& textData)
{
	QString pname;
	QString countryName;
	textData >> pname;
	if (!countrySelections.contains(countryName))
	{
		//qDebug() << playerNames.indexOf(pname);
		//qDebug() << countrySelections.size();
		readyStatusFlags.replace(playerNames.indexOf(pname),
			!readyStatusFlags.at(playerNames.indexOf(pname)));
		for (int i = 0; i < playerNames.size(); i++)
		{
			writeReadyData(playerSockets.at(i), pname);
		}
	}
	if (!readyStatusFlags.contains(false))
	{
		writeStartGame();
		ingame = true;
		QTimer::singleShot(100000, this, SLOT(endgame()));
	}
}

void GlobalStrikeServer::processDisconnectRequest()
{
	if (closingServer)
		return;
	QTcpSocket* temp = (QTcpSocket*)sender();
	int index = playerSockets.indexOf(temp);
	QString pname = playerNames.at(index);
	playerNames.removeAt(index);
	playerSockets.removeAt(index);
	qDebug() << QString("Player " + pname + " has left the game server.");
	LogEvent("Player " + pname + " has left the game server.");
	for (int i = index; i < 4; i++)
	{
		readyStatusFlags.replace(index, readyStatusFlags.at(index + 1));
	}
	readyStatusFlags.replace(4, false);
	for (int i = 0; i < playerSockets.size(); i++)
	{
		writeDisconnectRequest(playerSockets.at(i), pname);
	}
}

void GlobalStrikeServer::processDisconnectRequest(QTextStream& textData)
{
	QString pname;
	textData >> pname;
	int index = playerNames.indexOf(pname);
	for (int i = 0; i < playerSockets.size(); i++)
	{
		if (playerSockets.at(i) == playerSockets.at(index))
			continue;
		writeDisconnectRequest(playerSockets.at(i), pname);
	}
	playerSockets.removeAt(index);
	qDebug() << QString("Player " + playerNames.at(index) + " has left the game server.");
	LogEvent("Player " + playerNames.at(index) + " has left the game server.");
	playerNames.removeAt(index);
	for (int i = index; i < 4; i++)
	{
		readyStatusFlags.replace(index, readyStatusFlags.at(index + 1));
	}
	readyStatusFlags.replace(4, false);
}

void GlobalStrikeServer::processMessages(QTcpSocket* temp)
{
	char ch;
	QByteArray input = temp->readAll();
	LogEvent(QString("New message received at " + now.currentDateTime().toString()));
	LogEvent(input);
	QTextStream textData(&input, QIODevice::ReadOnly);
	textData >> ch;
	switch (ch)
	{
	case 'L':
		processLaunchCommand(textData);
		break;
	case 'I':
		processInterceptCommand(textData);
		break;
	case 'K':
		processYieldedKillPoint(textData);
		break;
	case 'R':
		processReadyToggleCommand(textData);
		break;
	case 'S':
		processYieldedKillPoint(textData);
		break;
	case 'B':
		processDisconnectRequest(textData);
		break;
	case 'Z':
		qDebug() << "Client: Parsing input concluded.";
		LogEvent(QString("Client: Parsing input concluded."));
	default:
		break;
	}
}

void GlobalStrikeServer::LogEvent(QByteArray data)
{
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
	{
		qDebug() << "logging failed.";
		return;
	}
	file.write(QString("|" + QString::number(lineNum++) + "| ").toUtf8().constData());
	file.write(data.replace('\n', ' '));
	file.write(QString("\n").toUtf8().constData());
	file.close();
}

void GlobalStrikeServer::LogEvent(QString data)
{
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
	{
		qDebug() << "logging failed.";
		return;
	}
	file.write(QString("|" + QString::number(lineNum++) + "| " + data.replace('\n', ' ') + "\n").toUtf8().constData());
	file.close();
}

void GlobalStrikeServer::processNewConnection(QString name)
{
	//QString name;
	//get name from textData
	//textData >> name;
	int numPlayers = d->GetNumPlayers();
	Player p(0, "", "");
	if (numPlayers < 1)
	{
		p.setUUID(1);
		p.setName(name);
		p.setCountry("Unset");
	}
	else if (numPlayers < 2)
	{
		p.setUUID(2);
		p.setName(name);
		p.setCountry("Unset");
	}
	else if (numPlayers < 3)
	{
		p.setUUID(3);
		p.setName(name);
		p.setCountry("Unset");
	}
	else if (numPlayers < 4)
	{
		p.setUUID(4);
		p.setName(name);
		p.setCountry("Unset");
	}
	else if (numPlayers < 5)
	{
		p.setUUID(5);
		p.setName(name);
		p.setCountry("Unset");
	}
	else
	{
		qDebug() << "Lobby is full";
		return;
	}
	d->AddPlayer(p);
}

void GlobalStrikeServer::setDatabase(SQLDatabase* d2)
{
	d = d2;
}

void GlobalStrikeServer::debuggingReadyUp()
{
	qDebug() << "testing Ready Up";
	for (int i = 0; i < 5; i++)
	{
		readyStatusFlags.replace(i, true);
	}
	writeStartGame();
}

void GlobalStrikeServer::endgame()
{
	ingame = false;
}
