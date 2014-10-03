#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

#include <memory>

class MainWindow : public QDialog
{
	Q_OBJECT
public:
	explicit MainWindow();

signals:

public slots:

private:
	std::unique_ptr<QAction> m_minimizeAction;
	std::unique_ptr<QAction> m_maximizeAction;
	std::unique_ptr<QAction> m_restoreAction;
	std::unique_ptr<QAction> m_quitAction;

	std::unique_ptr<QSystemTrayIcon> m_trayIcon;
	std::unique_ptr<QMenu> m_trayIconMenu;
};

#endif // MAINWINDOW_H
