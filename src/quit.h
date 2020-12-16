#ifndef QUIT_H
#define QUIT_H

#include <QDialog>
#include <QDebug>

namespace Ui {
	class quit;
}

class quit : public QDialog
{
	Q_OBJECT

public:
	explicit quit(QWidget* parent = nullptr);
	~quit();

private:
	Ui::quit* ui;

private slots:
	void QuitWindow();
	void QuitGame();
};

#endif // QUIT_H
