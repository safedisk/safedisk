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

#include "Disk.h"

#include <QMenu>
#include <QAction>
#include <QObject>

class DiskWidget : public QObject
{
	Q_OBJECT
public:
	explicit DiskWidget(QWidget* parent, const Disk& disk);

	QMenu* menu() const;

	static
	QString prompt(const QString& caption);

	void locate();
	bool unlock();
	void lock();

private slots:
	void onAction();
	void onReveal();

private:
	QWidget* m_parent;
	Disk m_disk;
	QMenu* m_menu;
	QAction* m_toggleAction;
	QAction* m_revealAction;
};
