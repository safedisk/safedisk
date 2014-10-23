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

#include "Script.h"

#include <QDir>
#include <QList>
#include <QObject>
#include <QFileInfo>

enum class DiskState
{
	Invalid,
	Missing,
	Locked,
	Unlocked,
};

class Disk : public QObject
{
	Q_OBJECT
private:
	explicit Disk(const QDir& dir);

public:
	explicit Disk() = default;
	~Disk();

	static
	QList<Disk*> fetch();

	static
	bool collision(const QString& dirName, const QString& name);

	QString name() const;
	QString guid() const;
	QString diskPath() const;
	QString volumePath() const;
	DiskState state() const;

	void create(const QDir& dir, const QString& name, const QString& password, uint64_t size);
	void attach(const QDir& dir);

	bool match(const QDir& dir);

	void lock();
	void unlock(const QString& password);
	bool link(const QDir& dir);

	void openVolume();
	void revealImage();
	void remove(bool erase);

	void cancel();

signals:
	void created();
	void locked();
	void unlocked();
	void error(int exitCode);

private:
	static
	QDir systemRoot();

	QFileInfo diskLink() const;
	QFileInfo volumeLink() const;
	QDir fuseDir() const;

	QDir makeSystemDir(QString guid = "");
	void revealFile(const QString& pathIn);

private:
	QDir* m_dir = nullptr;
	Script* m_pendingScript = nullptr;
};
