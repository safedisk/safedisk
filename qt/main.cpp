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

#include "MainWindow.h"

#include <QApplication>
#include <QMessageBox>

// TODO:
// http://doc.qt.digia.com/qq/qq12-mac-events.html
// Add attribution for glyphicons or replace icons with better ones

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(nullptr, "SafeDisk", "Could not detect the system tray on this OS.");
		return 1;
	}

	app.setQuitOnLastWindowClosed(false);

	MainWindow window;
	window.show();
	return app.exec();
}