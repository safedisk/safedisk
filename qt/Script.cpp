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
