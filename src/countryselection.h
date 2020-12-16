#ifndef COUNTRYSELECTION_H
#define COUNTRYSELECTION_H

#include "mainwindow.h"
#include <QDialog>
#include <QString>
#include <QDebug>

namespace Ui
{
	class CountrySelection;
}

class CountrySelection : public QDialog
{
	Q_OBJECT

public:
	explicit CountrySelection(QWidget* parent = nullptr);
	QString target = "NO_COUNTRY";
	~CountrySelection();

private:
	Ui::CountrySelection* ui;

private slots:
	void PickAtlantis();
	void PickCI();
	void PickRoT();
	void PickWR();
	void PickZathura();
};

#endif // COUNTRYSELECTION_H
