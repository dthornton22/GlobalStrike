#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quit.h"
#include "sqldatabase.cpp"
#include "countryselection.h"
#include <QMovie>
#include <QBitmap>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	QDateTime now;
	file.setFileName("mainWindow_log.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "logging failed.";
		return;
	}
	file.write(QString(" ----- Start of Log " + now.currentDateTime().toString() + " ----- \n").toUtf8().constData());
	file.close();
	gsClient = new GlobalStrikeClient;
	gsServer = nullptr;
	d = new SQLDatabase;

	ui->setupUi(this);
	ui->tabWidget->setCurrentIndex(0);

	// CHANGE VIEW SIGNALS AND SLOTS
	connect(ui->StartLobbyButton, SIGNAL(clicked()), this, SLOT(StartLobby()));
	connect(ui->JoinLobbyButton, SIGNAL(clicked()), this, SLOT(JoinLobby()));
	connect(ui->homebutton1, SIGNAL(clicked()), this, SLOT(HomeLobby()));
	connect(ui->homebutton2, SIGNAL(clicked()), this, SLOT(HomeLobby()));
	connect(ui->leaveButton, SIGNAL(clicked()), this, SLOT(HomeLobby()));
	connect(ui->leaveButton2, SIGNAL(clicked()), this, SLOT(HomeLobby()));
	connect(ui->quitButtonGameLobby, SIGNAL(clicked()), this, SLOT(HomeLobby()));

	// SERVER/CLIENT MAIN WINDOW ONLY BASED SIGNALS AND SLOTS
	connect(ui->startServerButton, SIGNAL(clicked()), this, SLOT(StartServer()));
	connect(ui->closeServerButton, SIGNAL(clicked()), this, SLOT(CloseServer()));
	connect(ui->JoinButton, SIGNAL(clicked()), this, SLOT(JoinServer()));
	connect(ui->readyButton, SIGNAL(clicked()), this, SLOT(ReadyUp()));
	connect(gsClient, SIGNAL(lobbyIPUpdate(QString, quint16)), this, SLOT(updateLobbyIPData(QString, quint16)));
	connect(gsClient, SIGNAL(gameIsStarting()), this, SLOT(StartGame()));
	connect(gsClient, SIGNAL(connectionLost()), this, SLOT(Disconnect()));
	connect(gsClient->client, SIGNAL(disconnected()), this, SLOT(Disconnect()));

	// GAME BASED SIGNALS AND SLOTS
	// Connection for "Launch Missile" gameplay button to corresponding function
	connect(ui->LaunchMissile, SIGNAL(clicked()), this, SLOT(LaunchConfirmMissile()));
	// Connection for "Launch Interceptor" gameplay button to corresponding function
	connect(ui->LaunchInterceptor, SIGNAL(clicked()), this, SLOT(LaunchConfirmInterceptor()));

	connect(gsClient, SIGNAL(animateWhite()), this, SLOT(AnimationWhite()));
	connect(gsClient, SIGNAL(animateOrange()), this, SLOT(AnimationOrange()));
	connect(gsClient, SIGNAL(animateGreen()), this, SLOT(AnimationGreen()));
	connect(gsClient, SIGNAL(animateYellow()), this, SLOT(AnimationYellow()));
	connect(gsClient, SIGNAL(animateLaunchBlack(int, int)), this, SLOT(AnimationLaunchBlack(int, int)));
	connect(gsClient, SIGNAL(animateLaunchWhite(int, int)), this, SLOT(AnimationLaunchWhite(int, int)));
	connect(gsClient, SIGNAL(animateLaunchOrange(int, int)), this, SLOT(AnimationLaunchOrange(int, int)));
	connect(gsClient, SIGNAL(animateLaunchGreen(int, int)), this, SLOT(AnimationLaunchGreen(int, int)));
	connect(gsClient, SIGNAL(animateLaunchYellow(int, int)), this, SLOT(AnimationLaunchYellow(int, int)));
	connect(gsClient, SIGNAL(animateLaunchBlack(int, int)), this, SLOT(AnimationLaunchBlack(int, int)));
	connect(gsClient, SIGNAL(explodeMissile(QString)), this, SLOT(ExplodeMissile(QString)));

	// CLIENT/DATABASE SIGNALS AND SLOTS
	connect(gsClient, SIGNAL(newClientDB(QString)), this, SLOT(updatePlayerLobbyData(QString)));
	connect(gsClient, SIGNAL(readyDBToggle(QString)), this, SLOT(toggleReadyUpFlag(QString)));
	connect(gsClient, SIGNAL(scoreUpdate(QString)), this, SLOT(updatePlayerScoreData(QString)));
	connect(gsClient, SIGNAL(lobbyChanged()), this, SLOT(dropDatabase()));

	// QUIT BASED SIGNALS AND SLOTS
	connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(QuitWindow()));
	connect(ui->quitButton2, SIGNAL(clicked()), this, SLOT(QuitWindow()));
	connect(ui->quitButton3, SIGNAL(clicked()), this, SLOT(QuitWindow()));
	connect(ui->quitButton4, SIGNAL(clicked()), this, SLOT(QuitWindow()));
	connect(ui->quitButton5, SIGNAL(clicked()), this, SLOT(QuitWindow()));
	connect(ui->quitButton6, SIGNAL(clicked()), this, SLOT(QuitWindow()));

	countryNames.append("Atlantis");
	countryNames.append("Cobra Island");
	countryNames.append("Republic of Texas");
	countryNames.append("West Riverdale");
	countryNames.append("Zathura");

	MissileTimer = nullptr;
	InterceptorTimer = nullptr;

	this->show();
	ui->animationLabelGreen->hide();
	ui->animationLabelBlack->hide();
	ui->animationLabelWhite->hide();
	ui->animationLabelYellow->hide();
	ui->animationLabelOrange->hide();
	ui->animationLabelGreen_2->hide();
	ui->animationLabelBlack_2->hide();
	ui->animationLabelWhite_2->hide();
	ui->animationLabelYellow_2->hide();
	ui->animationLabelOrange_2->hide();
}

MainWindow::~MainWindow()
{
	if (gsServer != nullptr)
	{
		delete gsServer;
		gsServer = nullptr;
	}
	if (gsClient != nullptr)
	{
		delete gsClient;
		gsClient = nullptr;
	}
	if (d != nullptr)
	{
		delete d;
		d = nullptr;
	}
	delete ui;
}

void MainWindow::StartServer()
{
	if (ui->pwordLineEdit->displayText().isEmpty() || ui->hnameLineEdit->displayText().isEmpty())
	{
		qDebug() << "Input a proper username and password.";
		return;
	}
	qDebug() << "MainWin: Starting server for Global Strike Game.";
	LogEvent(QString("MainWin: Starting server for Global Strike Game."));
	gsClient->password = ui->pwordLineEdit->displayText();
	gsClient->playerName = ui->hnameLineEdit->displayText();
	gsServer = new GlobalStrikeServer(gsClient);
	connect(ui->pushButton, SIGNAL(clicked()), gsServer, SLOT(debuggingReadyUp()));
	connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(CloseServer()));
	ui->tabWidget->setCurrentIndex(3);
}

void MainWindow::CloseServer()
{
	qDebug() << "Closing server";
	if (gsServer != nullptr)
	{
		delete gsServer;
		gsServer = nullptr;
	}
	d->DropTable();
	ui->tabWidget->setCurrentIndex(0);
	ui->tableView->setModel(d->GetLobby());
}

void MainWindow::JoinServer()
{
	if (ui->inputIPLineEdit->displayText().isEmpty() || ui->inputPNumLineEdit->displayText().isEmpty()
		|| ui->inputPNameLineEdit->displayText().isEmpty() || ui->inputPWordLineEdit->displayText().isEmpty())
	{
		qDebug() << "Input a proper username and password.";
		return;
	}
	QString ipAddress = ui->inputIPLineEdit->displayText();
	bool ok;
	quint16 portNum = ui->inputPNumLineEdit->displayText().toInt(&ok, 10);
	if (!ok)
	{
		qDebug() << "Input a decimal port.";
		return;
	}
	gsClient->playerName = ui->inputPNameLineEdit->displayText();
	gsClient->password = ui->inputPWordLineEdit->displayText();
	bool flag = gsClient->connectToServer(ipAddress, portNum);
	if (flag)
	{
		connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(Disconnect()));
	}
	ui->tabWidget->setCurrentIndex(3);
}

void MainWindow::updateLobbyIPData(QString ipAddressLE, quint16 portNumLE)
{
	LogEvent(QString("MainWin: Updating lobby IP data for Global Strike Game."));
	ui->connectionIPLineEdit->setText(ipAddressLE);
	ui->connectionIPLineEdit->setReadOnly(true);
	ui->portNumLineEdit->setText(QString(QString::number(portNumLE)));
	ui->portNumLineEdit->setReadOnly(true);
	qDebug() << "Textboxes appended";
}

void MainWindow::updatePlayerLobbyData(QString name)
{
	d->newClientAddition(name);
	ui->tableView->setModel(d->GetLobby());
}

void MainWindow::dropDatabase()
{
	d->DropTable();
	ui->tableView->setModel(d->GetLobby());
}

void MainWindow::toggleReadyUpFlag(QString name)
{
	LogEvent(QString("MainWin: Player Ready Status was updated."));
	d->UpdateReadyStatus(name);
	ui->tableView->setModel(d->GetLobby());
}

void MainWindow::updatePlayerScoreData(QString name)
{
	d->IncrementScore(name);
	ui->tableView->setModel(d->GetModel());
}

void MainWindow::StartLobby()
{
	ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::JoinLobby()
{
	QIntValidator* intValidator = new QIntValidator(ui->inputPNumLineEdit);
	ui->inputPNumLineEdit->setValidator(intValidator);
	QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
	QRegExp ipRegex("^" + ipRange + "\\." + ipRange + "\\." + ipRange + "\\." + ipRange + "$");
	QRegExpValidator* ipValidator = new QRegExpValidator(ipRegex, this);
	ui->inputIPLineEdit->setValidator(ipValidator);
	ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::HomeLobby()
{
	ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::QuitWindow()
{
	quit* quitWin = new quit();
	quitWin->exec();
}

// Executes when "Ready" button is clicked
void MainWindow::ReadyUp()
{
	gsClient->readyToggle();
}

// Executes when All players have selected "Ready button"
void MainWindow::StartGame()
{
	// TODO: need to print "Starting game" to textbox
	qDebug() << "Starting Game";
	// System moves to Gameplay Tab
	ui->tabWidget->setCurrentIndex(4);
	ui->lcdDisplay->display(time);

	// Game timer
	game_timer = new QTimer(this);
	connect(game_timer, &QTimer::timeout, this, &MainWindow::UpdateGameTime);
	game_timer->start(1000);
}

void MainWindow::DisplayDb()
{
	ui->ResultsTable->setModel(d->GetModel());
}

void MainWindow::DisplayLobby()
{
	ui->tableView->setModel(d->GetLobby());
}

// A function to keep track of the status of the game
void MainWindow::UpdateGameTime()
{
	time--;
	ui->lcdDisplay->display(time);
	if (time == 0)
	{
		EndGame();
	}
}

// launches missile from launching player to desired player
void MainWindow::LaunchMissile(QString target)
{
	if (Missile_cd_time != 100 || Interceptor_cd_time != 100)
	{
		qDebug() << "Cannot launch missile!";
		return;
	}
	qDebug() << "Launching missile to " + target;

	if ((gsClient->takenCountryNames.indexOf(target) + 1) > gsClient->playerNames.size())
	{
		qDebug() << "Cannot launch, player is not in game.";
		return;
	}
	QString pname_target = gsClient->playerNames.at(gsClient->takenCountryNames.indexOf(target));
	if (int(gsClient->playerID - 1) == int(gsClient->takenCountryNames.indexOf(target)))
	{
		qDebug() << "Cannot launch missile at yourself!!";
		return;
	}
	if (pname_target == "Unset") {
		qDebug() << "Player was disconnected. Invalid target";
		return;
	}
	gsClient->launchCommand(pname_target);
	MissileTimer = new QTimer(this);
	connect(MissileTimer, &QTimer::timeout, this, &MainWindow::UpdateMissileCooldownTime);
	MissileTimer->start(50);
}

void MainWindow::LaunchConfirmMissile()
{
	CountrySelection* pick = new CountrySelection();
	pick->exec();
	if (pick->target == "NO_COUNTRY")
	{
		qDebug() << "No country selected";
		return;
	}
	else
	{
		LaunchMissile(pick->target);
	}
}

void MainWindow::UpdateMissileCooldownTime()
{
	Missile_cd_time--;
	ui->progressBar->setValue(Missile_cd_time);
	if (Missile_cd_time == 0)
	{
		MissileTimer->stop();
		Missile_cd_time = 100;
		ui->progressBar->setValue(Missile_cd_time);
	}
}

void MainWindow::LaunchInterceptor(QString target)
{
	if (Missile_cd_time != 100 || Interceptor_cd_time != 100)
	{
		qDebug() << "Cannot launch interceptor!";
		return;
	}

	qDebug() << QString("Launching interceptor to " + target);

	if ((gsClient->takenCountryNames.indexOf(target) + 1) > gsClient->playerNames.size())
	{
		qDebug() << "cannot launch, player is not in game.";
		return;
	}
	QString pname_target = gsClient->playerNames.at(gsClient->takenCountryNames.indexOf(target));
	if (int(gsClient->playerID - 1) == int(gsClient->takenCountryNames.indexOf(target)))
	{
		qDebug() << "Cannot launch interceptor at yourself!!";
		return;
	}
	if (pname_target == "Unset")
	{
		qDebug() << "Player was disconnected. Invalid target";
		return;
	}
	gsClient->interceptCommand(pname_target);
	InterceptorTimer = new QTimer(this);
	connect(InterceptorTimer, &QTimer::timeout, this, &MainWindow::UpdateInterceptorCooldownTime);
	InterceptorTimer->start(50);
}

void MainWindow::LaunchConfirmInterceptor()
{
	CountrySelection* pick = new CountrySelection();
	pick->exec();
	if (pick->target == "NO_COUNTRY")
	{
		qDebug() << "No country selected";
		return;
	}
	else
	{
		MainWindow::LaunchInterceptor(pick->target);
	}
}

void MainWindow::UpdateInterceptorCooldownTime()
{
	Interceptor_cd_time--;
	ui->progressBar_2->setValue(Interceptor_cd_time);
	if (Interceptor_cd_time == 0)
	{
		InterceptorTimer->stop();
		Interceptor_cd_time = 100;
		ui->progressBar_2->setValue(Interceptor_cd_time);
	}
}

void MainWindow::EndGame()
{
	qDebug() << "Ending Game...";
	game_timer->stop();
	if (MissileTimer != nullptr)
		MissileTimer->stop();
	if (InterceptorTimer != nullptr)
		InterceptorTimer->stop();
	ui->tabWidget->setCurrentIndex(5);
	DisplayDb();
	while (gsClient->playerNames.contains("Unset"))
		gsClient->playerNames.removeAt(gsClient->playerNames.indexOf("Unset"));
	d->DropTable();
}

void MainWindow::Disconnect()
{
	gsClient->client->disconnectFromHost();
	gsClient->playerNames.clear();
	d->DropTable();
	disconnect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(Disconnect()));
	ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::AnimationGreen()
{
	ui->animationLabelGreen->show();
	QMovie* movie = new QMovie(":animationImage.gif");
	ui->animationLabelGreen->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelGreen->setMovie(movie);
	ui->animationLabelGreen->setScaledContents(true);
	movie->start();

	QTimer::singleShot(3000, this, SLOT(cleanGreen()));
}

void MainWindow::cleanGreen()
{
	ui->animationLabelGreen->hide();
}

void MainWindow::AnimationBlack()
{
	ui->animationLabelBlack->show();
	QMovie* movie = new QMovie(":animationImage.gif");
	ui->animationLabelBlack->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelBlack->setMovie(movie);
	ui->animationLabelBlack->setScaledContents(true);
	movie->start();

	QTimer::singleShot(3000, this, SLOT(cleanBlack()));
}

void MainWindow::cleanBlack()
{
	ui->animationLabelBlack->hide();
}

void MainWindow::AnimationWhite()
{
	ui->animationLabelWhite->show();
	QMovie* movie = new QMovie(":animationImage.gif");
	ui->animationLabelWhite->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelWhite->setMovie(movie);
	ui->animationLabelWhite->setScaledContents(true);
	movie->start();

	QTimer::singleShot(3000, this, SLOT(cleanWhite()));
}

void MainWindow::cleanWhite()
{
	ui->animationLabelWhite->hide();
}

void MainWindow::AnimationYellow()
{
	ui->animationLabelYellow->show();
	QMovie* movie = new QMovie(":animationImage.gif");
	ui->animationLabelYellow->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelYellow->setMovie(movie);
	ui->animationLabelYellow->setScaledContents(true);
	movie->start();

	QTimer::singleShot(3000, this, SLOT(cleanYellow()));
}

void MainWindow::cleanYellow()
{
	ui->animationLabelYellow->hide();
}

void MainWindow::AnimationOrange()
{
	ui->animationLabelOrange->show();
	QMovie* movie = new QMovie(":animationImage.gif");
	ui->animationLabelOrange->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelOrange->setMovie(movie);
	ui->animationLabelOrange->setScaledContents(true);
	movie->start();

	QTimer::singleShot(3000, this, SLOT(cleanOrange()));
}

void MainWindow::cleanOrange()
{
	ui->animationLabelOrange->hide();
}

void MainWindow::AnimationLaunchBlack(int x, int y)
{
	ui->animationLabelBlack_2->show();
	QMovie* movie = new QMovie(":ezgif_missile_launch_toNE.gif");
	animationBlack = new QPropertyAnimation(ui->animationLabelBlack_2, "geometry");
	animationBlack->setDuration(3000);
	animationBlack->setStartValue(QRect(80, 129, 71, 41));
	animationBlack->setEndValue(QRect(x, y, 71, 41));

	ui->animationLabelBlack_2->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelBlack_2->setMovie(movie);
	ui->animationLabelBlack_2->setScaledContents(true);
	movie->start();
	animationBlack->start();

	QTimer::singleShot(3000, this, SLOT(cleanLaunchBlack()));
}

void MainWindow::AnimationLaunchWhite(int x, int y)
{
	ui->animationLabelWhite_2->show();
	QMovie* movie = new QMovie(":ezgif_missile_launch_toNW.gif");
	if (!movie->isValid())
	{
		qDebug() << "failed to run gif.";
		qDebug() << movie->lastErrorString();
	}
	animationWhite = new QPropertyAnimation(ui->animationLabelWhite_2, "geometry");
	animationWhite->setDuration(3000);
	animationWhite->setStartValue(QRect(640, 140, 71, 41));
	animationWhite->setEndValue(QRect(x, y, 71, 41));

	ui->animationLabelWhite_2->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelWhite_2->setMovie(movie);
	ui->animationLabelWhite_2->setScaledContents(true);
	movie->start();
	animationWhite->start();

	QTimer::singleShot(3000, this, SLOT(cleanLaunchWhite()));
}

void MainWindow::AnimationLaunchOrange(int x, int y)
{
	ui->animationLabelOrange_2->show();
	QMovie* movie = new QMovie(":ezgif_missile_launch_toNW.gif");
	animationOrange = new QPropertyAnimation(ui->animationLabelOrange_2, "geometry");
	animationOrange->setDuration(3000);
	animationOrange->setStartValue(QRect(499, 329, 71, 41));
	animationOrange->setEndValue(QRect(x, y, 71, 41));

	ui->animationLabelOrange_2->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelOrange_2->setMovie(movie);
	ui->animationLabelOrange_2->setScaledContents(true);
	movie->start();
	animationOrange->start();

	QTimer::singleShot(3000, this, SLOT(cleanLaunchOrange()));
}

void MainWindow::AnimationLaunchYellow(int x, int y)
{
	ui->animationLabelYellow_2->show();
	QMovie* movie = new QMovie(":ezgif_missile_launch_toNE.gif");
	animationYellow = new QPropertyAnimation(ui->animationLabelYellow_2, "geometry");
	animationYellow->setDuration(3000);
	animationYellow->setStartValue(QRect(250, 289, 71, 41));
	animationYellow->setEndValue(QRect(x, y, 71, 41));

	ui->animationLabelYellow_2->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelYellow_2->setMovie(movie);
	ui->animationLabelYellow_2->setScaledContents(true);
	movie->start();
	animationYellow->start();

	QTimer::singleShot(3000, this, SLOT(cleanLaunchYellow()));
}

void MainWindow::AnimationLaunchGreen(int x, int y)
{
	ui->animationLabelGreen_2->show();
	QMovie* movie = new QMovie(":ezgif_missile_launch_toNW.gif");
	animationGreen = new QPropertyAnimation(ui->animationLabelGreen_2, "geometry");
	animationGreen->setDuration(3000);
	animationGreen->setStartValue(QRect(409, 79, 71, 41));
	animationGreen->setEndValue(QRect(x, y, 71, 41));

	ui->animationLabelGreen_2->setAttribute(Qt::WA_NoSystemBackground);
	ui->animationLabelGreen_2->setMovie(movie);
	ui->animationLabelGreen_2->setScaledContents(true);
	movie->start();
	animationGreen->start();

	QTimer::singleShot(3000, this, SLOT(cleanLaunchGreen()));
}

void MainWindow::cleanLaunchBlack()
{
	ui->animationLabelBlack_2->hide();
}

void MainWindow::cleanLaunchOrange()
{
	ui->animationLabelOrange_2->hide();
}

void MainWindow::cleanLaunchWhite()
{
	ui->animationLabelWhite_2->hide();
}

void MainWindow::cleanLaunchGreen()
{
	ui->animationLabelGreen_2->hide();
}

void MainWindow::cleanLaunchYellow()
{
	ui->animationLabelYellow_2->hide();
}

void MainWindow::ExplodeMissile(QString missileOrigin)
{
	QMovie* movie = new QMovie(":ezgif_explosion.gif");
	if (!movie->isValid())
	{
		qDebug() << "failed to run gif.";
		qDebug() << movie->lastErrorString();
		return;
	}
	if (missileOrigin == "Atlantis")
	{
		ui->animationLabelWhite_2->setAttribute(Qt::WA_NoSystemBackground);
		ui->animationLabelWhite_2->setMovie(movie);
		ui->animationLabelWhite_2->setScaledContents(true);
	}
	else if (missileOrigin == "Cobra")
	{
		ui->animationLabelOrange_2->setAttribute(Qt::WA_NoSystemBackground);
		ui->animationLabelOrange_2->setMovie(movie);
		ui->animationLabelOrange_2->setScaledContents(true);
	}
	else if (missileOrigin == "Republic")
	{
		ui->animationLabelGreen_2->setAttribute(Qt::WA_NoSystemBackground);
		ui->animationLabelGreen_2->setMovie(movie);
		ui->animationLabelGreen_2->setScaledContents(true);
	}
	else if (missileOrigin == "West")
	{
		ui->animationLabelYellow_2->setAttribute(Qt::WA_NoSystemBackground);
		ui->animationLabelYellow_2->setMovie(movie);
		ui->animationLabelYellow_2->setScaledContents(true);
	}
	else if (missileOrigin == "Zathura")
	{
		ui->animationLabelBlack_2->setAttribute(Qt::WA_NoSystemBackground);
		ui->animationLabelBlack_2->setMovie(movie);
		ui->animationLabelBlack_2->setScaledContents(true);
	}
	movie->start();
}

void MainWindow::LogEvent(QByteArray data)
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

void MainWindow::LogEvent(QString data)
{
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
	{
		qDebug() << "Logging failed.";
		return;
	}
	file.write(QString("|" + QString::number(lineNum++) + "| " + data + "\n").toUtf8().constData());
	file.close();
}
