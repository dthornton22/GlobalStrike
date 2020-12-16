#include "sqldatabase.h"
#include <QtSql>

SQLDatabase::SQLDatabase()
{
	const QString DRIVER("QSQLITE");
	if (QSqlDatabase::isDriverAvailable(DRIVER))
	{
		db = QSqlDatabase::addDatabase(DRIVER);
		db.setDatabaseName(":memory:");
		if (!db.open())
		{
			qWarning() << "ERROR: " << db.lastError();
		}
		else
		{
			QSqlQuery q;
			if (!q.exec("CREATE TABLE results (id INT, name TEXT, country TEXT, score INT, Ready_Status TEXT)"))
			{
				qDebug() << db.lastError();
				qDebug() << "Error: Unable to create table due to above error";
			}
		}
	}
}

QSqlQueryModel* SQLDatabase::GetModel()
{
	QSqlQueryModel* model = new QSqlQueryModel;
	model->setQuery("SELECT * FROM results");
	return model;
}

QSqlQueryModel* SQLDatabase::GetLobby()
{
	QSqlQueryModel* model = new QSqlQueryModel;
	model->setQuery("SELECT name, country, Ready_Status, score FROM results");
	return model;
}

void SQLDatabase::UpdateDb()
{
	QSqlQuery q("UPDATE results SET score = score + 1 WHERE id = 1");
	if (!q.exec())
	{
		qDebug() << db.lastError();
		qDebug() << "Error: Unable to create table due to above error";
	}
}

void SQLDatabase::AddPlayer(Player p)
{
	QSqlQuery q("INSERT INTO results (id, name, country, score, Ready_Status) VALUES(?, ?, ?, ?, ?)");
	q.addBindValue(p.UUID);
	q.addBindValue(p.name);
	q.addBindValue(p.country);
	q.addBindValue(0);
	q.addBindValue("Not Ready");
	if (!q.exec())
	{
		qDebug() << db.lastError();
		qDebug() << "Error: Unable to create table due to above error";
	}
}

void SQLDatabase::newClientAddition(QString name)
{
	int numPlayers = GetNumPlayers();
	Player p(0, "", "");
	if (numPlayers < 1)
	{
		p.setUUID(1);
		p.setName(name);
		p.setCountry("Atlantis");
	}
	else if (numPlayers < 2)
	{
		p.setUUID(2);
		p.setName(name);
		p.setCountry("Cobra Island");
	}
	else if (numPlayers < 3)
	{
		p.setUUID(3);
		p.setName(name);
		p.setCountry("Republic of Texas");
	}
	else if (numPlayers < 4)
	{
		p.setUUID(4);
		p.setName(name);
		p.setCountry("West Riverdale");
	}
	else if (numPlayers < 5)
	{
		p.setUUID(5);
		p.setName(name);
		p.setCountry("Zathura");
	}
	else
	{
		qDebug() << "Lobby is full";
		return;
	}
	AddPlayer(p);
}

int SQLDatabase::GetNumPlayers()
{
	QSqlQuery q("SELECT COUNT(*) FROM results");
	if (!q.exec())
	{
		qDebug() << db.lastError();
		qDebug() << "Error: Unable to create table due to above error";
	}
	q.next();
	return q.value(0).toInt();
}

void SQLDatabase::UpdateReadyStatus(QString name)
{
	QSqlQuery q1;
	q1.prepare("SELECT Ready_Status FROM results WHERE name = ?");
	q1.addBindValue(name.toUtf8().data());
	if (!q1.exec())
	{
		qDebug() << db.lastError();
		qDebug() << "Error: Unable to create table due to above error";
	}
	q1.next();
	QString readyStatus = q1.value(0).toString();

	if (readyStatus.contains("Not Ready"))
	{
		QSqlQuery q2;
		q2.prepare("UPDATE results SET Ready_Status = ? WHERE name = ?");
		q2.addBindValue("Ready");
		q2.addBindValue(name.toUtf8().data());
		if (!q2.exec())
		{
			qDebug() << db.lastError();
			qDebug() << "Error: Unable to create table due to above error";
		}
	}
	else
	{
		QSqlQuery q3;
		q3.prepare("UPDATE results SET Ready_Status = ? WHERE name = ?");
		q3.addBindValue("Not Ready");
		q3.addBindValue(name.toUtf8().data());
		if (!q3.exec())
		{
			qDebug() << db.lastError();
			qDebug() << "Error: Unable to create table due to above error";
		}
	}
}

void SQLDatabase::IncrementScore(QString name)
{
	QSqlQuery q;
	q.prepare("UPDATE results SET score = score + 1 WHERE name = ?");
	q.addBindValue(name.toUtf8().data());
	if (!q.exec())
	{
		qDebug() << db.lastError();
		qDebug() << "Error: Unable to create table due to above error";
	}
}

void SQLDatabase::RemovePlayer(QString name)

{
	QSqlQuery q;
	q.prepare("DELETE FROM results WHERE name = ?");
	q.addBindValue(name.toUtf8().data());
	if (!q.exec())
	{
		qDebug() << db.lastError();
		qDebug() << "Error:  Unable to remove player due to above error";
	}
}

void SQLDatabase::DropTable()
{
	QSqlQuery q;
	if (!q.exec("DROP TABLE results;"))
	{
		qDebug() << db.lastError();
		qDebug() << "Error:  Unable to drop table due to above error";
	}
	else {
		QSqlQuery q;
		if (!q.exec("CREATE TABLE results (id INT, name TEXT, country TEXT, score INT, Ready_Status TEXT)"))
		{
			qDebug() << db.lastError();
			qDebug() << "Error: Unable to create table due to above error";
		}
	}
}
