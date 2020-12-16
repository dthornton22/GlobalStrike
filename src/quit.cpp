#include "quit.h"
#include "ui_quit.h"

quit::quit(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::quit)
{
	ui->setupUi(this);
	connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(QuitWindow()));
	connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(QuitGame()));
}

quit::~quit()
{
	delete ui;
}

void quit::QuitWindow()
{
	this->accept();
}

void quit::QuitGame()
{
	exit(EXIT_FAILURE);
}
