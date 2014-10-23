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

#include "Script.h"

#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QApplication>

Script::Script(const QString& program, const QStringList& args, const QString& input)
	: m_args(args)
	, m_input(input)
{
	QDir appDir(QApplication::applicationDirPath());
	m_program = appDir.filePath(program);
	m_process.setWorkingDirectory(appDir.absolutePath());
	m_process.setProcessChannelMode(QProcess::ForwardedChannels);
	connect(&m_process, SIGNAL(started()), this, SLOT(onStarted()));
	connect(&m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));
}

void Script::start()
{
	m_process.start(m_program, m_args);
}

void Script::terminate()
{
	m_process.terminate();
}

void Script::onStarted()
{
	if (!m_input.isEmpty()) {
		QTextStream stream(&m_process);
		stream << m_input << "\n";
		stream.flush();
	}
}

void Script::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	qDebug() << m_program << "exitStatus:" << exitStatus << "exitCode:" << exitCode;
	emit finished(exitCode, exitStatus);
}
