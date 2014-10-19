#pragma once

#include "MainWindow.h"

#include <QApplication>

class App : public QApplication
{
	Q_OBJECT
public:
	explicit App(int& argc, char** argv);
	int run();

protected:
	bool event(QEvent* evt);

private:
	MainWindow m_mainWindow;
};
