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
	void onAbout();
	void onCreate();
	void onAttach();
	void refresh();

private:
	QAction m_aboutAction;
	QAction m_aboutQtAction;
	QAction m_createAction;
	QAction m_attachAction;
	QAction m_quitAction;

	QMenu m_trayIconMenu;
	QMenu m_disksSubMenu;
	QSystemTrayIcon m_trayIcon;

	QAction* m_disksSeparator = nullptr;
	QList<DiskWidget*> m_disks;
};
