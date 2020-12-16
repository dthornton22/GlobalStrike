#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include <QtSql>
#include "player.h"

class SQLDatabase
{
private:
	QSqlDatabase db;
public:
	SQLDatabase();
	QSqlQueryModel* GetModel();
	QSqlQueryModel* GetLobby();
	void AddPlayer(Player p);
	void UpdateDb();
	void DropTable();
public slots:
	int GetNumPlayers();
	void newClientAddition(QString name);
	void RemovePlayer(QString name);
	void UpdateReadyStatus(QString name);
	void IncrementScore(QString name);
};

#endif // SQLDATABASE_H
