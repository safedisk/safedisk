/*  osx-nbd
 *  Copyright (C) 2014 Frank Laub
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MainWindow.h"
#include "CreateDiskDialog.h"

#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow()
{
	setWindowFlags(windowType() | Qt::FramelessWindowHint);
	resize(0, 0);

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

	for (auto disk : Disk::listDisks(this)) {
		m_trayIconMenu->insertMenu(m_disksSeparator, disk->menu());
	}
}

void MainWindow::createDisk()
{
	raise();

	CreateDiskDialog dialog;
	int result = dialog.exec();
	if (result == QDialog::Accepted) {
		Disk* disk = new Disk(dialog.volumeName(), this);
		m_trayIconMenu->insertMenu(m_disksSeparator, disk->menu());
	}
}

void MainWindow::restoreDisk()
{
	raise();

	QMessageBox::information(this, "SafeDisk", "Restore Disk");
}
