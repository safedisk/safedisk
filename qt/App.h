#pragma once

#include <QApplication>

class App : public QApplication
{
	Q_OBJECT
public:
	explicit App(int& argc, char** argv);

protected:
	bool event(QEvent* evt);

private:
	void openDisk(const QString& filename);
};
