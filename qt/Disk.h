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

#include <QDir>
#include <QMenu>
#include <QObject>

class Disk : public QObject
{
	Q_OBJECT

private:
	explicit Disk(const QString& name, QWidget* parent);

public:
	QMenu* menu() const;

	static
	Disk* createDisk(QWidget* parent,
					 const QString& dirName,
					 const QString& name,
					 const QString& password,
					 uint64_t size);

	static
	Disk* attachDisk(QWidget* parent);

	static
	QList<Disk*> listDisks(QWidget* parent);

	static
	QString systemPath(const QString& name);

	static
	bool collision(const QString& dirName, const QString& name);

public slots:
	void revealFolder();

private slots:
	void toggleMount();
	void displaySettings();

private:
	void createMenu();
	void updateState();
	bool mount();
	void unmount();
	void find();

	bool isValid() const;
	bool isMounted() const;
	QString volumePath() const;

	static
	bool runScript(const QString& scriptName, const QStringList& args, const QString& input);

private:
	QString m_name;
	QWidget* m_parent;
	QMenu* m_menu = nullptr;
	QAction* m_toggleAction = nullptr;
	QAction* m_revealAction = nullptr;
	QDir m_bundleDir;
};
