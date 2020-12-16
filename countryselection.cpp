#include "countryselection.h"
#include "ui_countryselection.h"

CountrySelection::CountrySelection(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::CountrySelection)
{
	ui->setupUi(this);
	connect(ui->pushButton_Atlantis, SIGNAL(clicked()), this, SLOT(PickAtlantis()));
	connect(ui->pushButton_CI, SIGNAL(clicked()), this, SLOT(PickCI()));
	connect(ui->pushButton_RoT, SIGNAL(clicked()), this, SLOT(PickRoT()));
	connect(ui->pushButton_WR, SIGNAL(clicked()), this, SLOT(PickWR()));
	connect(ui->pushButton_Zathura, SIGNAL(clicked()), this, SLOT(PickZathura()));
}

CountrySelection::~CountrySelection()
{
	delete ui;
}

void CountrySelection::PickAtlantis()
{
	this->accept();
	target = "Atlantis";
}

void CountrySelection::PickCI()
{
	this->accept();
	target = "Cobra";
}

void CountrySelection::PickRoT()
{
	this->accept();
	target = "Republic";
}

void CountrySelection::PickWR()
{
	this->accept();
	target = "West";
}

void CountrySelection::PickZathura()
{
	this->accept();
	target = "Zathura";
}
