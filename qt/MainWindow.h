#pragma once

#include "Disk.h"

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QList>

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow();

signals:

private slots:
	void createDisk();
	void restoreDisk();

private:
	QAction* m_createAction = nullptr;
	QAction* m_restoreAction = nullptr;
	QAction* m_quitAction = nullptr;

	QSystemTrayIcon* m_trayIcon = nullptr;
	QMenu* m_disksSubMenu = nullptr;
	QMenu* m_trayIconMenu = nullptr;

	QAction* m_disksSeparator = nullptr;
	QList<Disk*> m_disks;
};
