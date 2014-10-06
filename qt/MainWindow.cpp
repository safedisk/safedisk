#include "MainWindow.h"
#include "CreateDiskDialog.h"

#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QStandardPaths>

MainWindow::MainWindow()
{
	m_createAction = new QAction("Create SafeDisk", this);
	connect(m_createAction, SIGNAL(triggered()), this, SLOT(createDisk()));

	m_restoreAction = new QAction("Restore SafeDisk", this);
	connect(m_restoreAction, SIGNAL(triggered()), this, SLOT(restoreDisk()));

	m_quitAction = new QAction("&Quit", this);
	connect(m_quitAction, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

	m_trayIconMenu = new QMenu(this);
	m_trayIconMenu->addAction(m_createAction);
	m_trayIconMenu->addAction(m_restoreAction);
	m_trayIconMenu->addSeparator();
	m_disksSeparator = m_trayIconMenu->addSeparator();
	m_trayIconMenu->addAction(m_quitAction);

	m_trayIcon = new QSystemTrayIcon(this);
	m_trayIcon->setContextMenu(m_trayIconMenu);
	m_trayIcon->setIcon(QIcon(":/images/glyphicons_240_rotation_lock.png"));
	m_trayIcon->show();

	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	QDir::root().mkpath(paths[0]);
	QDir dataPath(paths[0]);
	QStringList disks = dataPath.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	for (auto it = disks.cbegin(); it != disks.cend(); it++) {
		Disk* disk = new Disk(*it, this);
		m_trayIconMenu->insertMenu(m_disksSeparator, disk->menu());
	}
}

void MainWindow::createDisk()
{
	CreateDiskDialog dialog;
	int result = dialog.exec();
	if (result == QDialog::Accepted) {
		Disk* disk = new Disk(dialog.volumeName(), this);
		m_trayIconMenu->insertMenu(m_disksSeparator, disk->menu());
	}
}

void MainWindow::restoreDisk()
{
	QMessageBox::information(this, "SafeDisk", "Restore Disk");
}
