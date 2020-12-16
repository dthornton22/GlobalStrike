#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QRegularExpression>
#include <QRegExpValidator>
#include <QDialog>
#include <QFile>
#include <QVBoxLayout>
#include <QIntValidator>
#include <QObject>
#include <QDateTime>
#include <QPropertyAnimation>
#include <globalstrikeclient.h>
#include <globalstrikeserver.h>
#include "sqldatabase.h"
#include "player.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
	class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	int time = 100;
	int Missile_cd_time = 100;
	int Interceptor_cd_time = 100;
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow* ui;
	Player* ThisPlayer;
	bool playersReady[5];
	QStringList playerIDList;
	QStringList playerNameList;
	QStringList countryNames;
	SQLDatabase* d;
	QTimer* game_timer;
	QTimer* MissileTimer;
	QTimer* InterceptorTimer;
	GlobalStrikeServer* gsServer;
	GlobalStrikeClient* gsClient;
	QString pword;
	QString pname;
	QString ipAdd;
	QString pnum;
	quint16 lineNum = 1;
	QFile file;
	QPropertyAnimation* animationBlack = nullptr;
	QPropertyAnimation* animationWhite = nullptr;
	QPropertyAnimation* animationOrange = nullptr;
	QPropertyAnimation* animationYellow = nullptr;
	QPropertyAnimation* animationGreen = nullptr;
	void LogEvent(QByteArray);
	void LogEvent(QString);

public slots:
	void updateLobbyIPData(QString, quint16);
	void updatePlayerLobbyData(QString);
	void dropDatabase();
	void toggleReadyUpFlag(QString);
	void updatePlayerScoreData(QString);
	void StartLobby();
	void JoinLobby();
	void Disconnect();
	void HomeLobby();
	void QuitWindow();
	void ReadyUp();
	void StartGame();
	void UpdateGameTime();
	void UpdateMissileCooldownTime();
	void UpdateInterceptorCooldownTime();
	void DisplayDb();
	void DisplayLobby();
	void EndGame();
	void LaunchMissile(QString target);
	void LaunchInterceptor(QString target);
	void LaunchConfirmMissile();
	void LaunchConfirmInterceptor();
	void AnimationGreen();
	void AnimationBlack();
	void AnimationWhite();
	void AnimationYellow();
	void AnimationOrange();
	void cleanGreen();
	void cleanWhite();
	void cleanYellow();
	void cleanOrange();
	void cleanBlack();
	void AnimationLaunchGreen(int x, int y);
	void AnimationLaunchBlack(int x, int y);
	void AnimationLaunchWhite(int x, int y);
	void AnimationLaunchYellow(int x, int y);
	void AnimationLaunchOrange(int x, int y);
	void cleanLaunchGreen();
	void cleanLaunchWhite();
	void cleanLaunchYellow();
	void cleanLaunchOrange();
	void cleanLaunchBlack();
	void ExplodeMissile(QString);

private slots:
	void StartServer();
	void CloseServer();
	void JoinServer();
};

#endif // MAINWINDOW_H
