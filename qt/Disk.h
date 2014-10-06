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

#include <QObject>
#include <QMenu>

class Disk : public QObject
{
	Q_OBJECT
public:
	explicit Disk(const QString& name, QWidget* parent);

	QMenu* menu() const;

	static
	QList<Disk*> listDisks(QWidget* parent);

	static
	bool exists(const QString& name);

signals:

private slots:
	void toggleMount();
	void revealFolder();
	void displaySettings();

private:
	void createMenu();
	void updateState();

private:
	QString m_name;
	QWidget* m_parent;
	QMenu* m_menu = nullptr;
	QAction* m_toggleAction = nullptr;
	QAction* m_revealAction = nullptr;
	bool m_isLocked = true;
	QString m_rootPath;
};
