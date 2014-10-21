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
#include <QList>
#include <QProcess>
#include <QFileInfo>

enum class DiskState
{
	Invalid,
	Missing,
	Locked,
	Unlocked,
};

class Disk
{
private:
	explicit Disk(const QDir& dir);

public:
	Disk() = default;

	static
	Disk create(const QDir& dir,
				const QString& name,
				const QString& password,
				uint64_t size);

	static
	Disk attach(const QDir& dir);

	static
	QList<Disk> fetch();

	static
	bool collision(const QString& dirName, const QString& name);

	QString name() const;
	QString guid() const;
	QString diskPath() const;
	QString volumePath() const;
	DiskState state() const;

	QProcess* lock();
	bool unlock(const QString& password);
	bool link(const QDir& dir);

	void openVolume();
	void revealImage();
	void remove(bool erase);

private:
	static
	QDir systemRoot();

	static
	QDir makeSystemDir(QString guid = "");

	QFileInfo diskLink() const;
	QFileInfo volumeLink() const;
	QDir fuseDir() const;

	static
	bool runScript(const QString& scriptName, const QStringList& args, const QString& input);

	void revealFile(const QString& pathIn);

private:
	QDir m_dir;
};
