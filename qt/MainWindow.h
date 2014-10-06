#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

#include <memory>

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
	std::unique_ptr<QAction> m_createAction;
	std::unique_ptr<QAction> m_restoreAction;
	std::unique_ptr<QAction> m_quitAction;

	std::unique_ptr<QSystemTrayIcon> m_trayIcon;
	std::unique_ptr<QMenu> m_trayIconMenu;
};
