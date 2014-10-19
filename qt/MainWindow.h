/*  SafeDisk
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

#pragma once

#include "DiskWidget.h"

#include <QMenu>
#include <QList>
#include <QAction>
#include <QMainWindow>
#include <QSystemTrayIcon>

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow();

	void openDisk(const QString& dirName);

private slots:
	void createDisk();
	void attachDisk();
	void refresh();

private:
	QAction* m_createAction = nullptr;
	QAction* m_attachAction = nullptr;
	QAction* m_quitAction = nullptr;

	QSystemTrayIcon* m_trayIcon = nullptr;
	QMenu* m_disksSubMenu = nullptr;
	QMenu* m_trayIconMenu = nullptr;

	QAction* m_disksSeparator = nullptr;
	QList<DiskWidget*> m_disks;
};
