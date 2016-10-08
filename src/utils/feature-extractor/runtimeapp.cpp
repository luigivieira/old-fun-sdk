/*
 * Copyright (C) 2016-2017 Luiz Carlos Vieira (http://www.luiz.vieira.nom.br)
 *
 * This file is part of Fun SDK (FSDK).
 *
 * FSDK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FSDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "runtimeapp.h"
#include "version.h"
#include "featureextractor.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QThreadPool>

 // +-----------------------------------------------------------
fsdk::RuntimeApp::RuntimeApp(int &argc, char **argv, const QString &sOrgName, const QString &sOrgDomain, const QString &sAppName, const QString &sAppVersion, const bool bUseSettings):
	Application(argc, argv, sOrgName, sOrgDomain, sAppName, sAppVersion, bUseSettings)
{
	// Replace the original message pattern from the parent class Application.
	// i.e.: - remove the source and line number from trace even in debug;
	//       - add the progress level instead of the log type;
	qSetMessagePattern("%{time yyyy.MM.dd h:mm:ss.zzz} [%{if-critical}p1%{endif}%{if-warning}p2%{endif}%{if-info}p3%{endif}%{if-debug}p4%{endif}]: %{message}");
}

// +-----------------------------------------------------------
fsdk::RuntimeApp::CommandLineParseResult fsdk::RuntimeApp::parseCommandLine()
{
	// Setup the command line parser
	QCommandLineParser oParser;
	
	oParser.setApplicationDescription(tr("Extracts facial features from video files into CSV files."));
	oParser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

	QCommandLineOption oHelpOpt = oParser.addHelpOption();
	QCommandLineOption oVersionOpt = oParser.addVersionOption();

	oParser.addPositionalArgument("input", tr("File name or wildcard pattern with the input video file(s) from where to perform the face tracking"), "<input file name or mask>");
	oParser.addPositionalArgument("output", tr("Directory where the CSV files will be created"), "<output directory>");

	QCommandLineOption oShowProgressOpt(QStringList({ "p", "progress" }),
		tr("Sets the level of progress messages that shall be displayed."
		   "The possible values are in range [1, 4], meaning: \n"
		   "1: display only critical error messages (default)\n"
		   "2: display critical and warning messages\n"
		   "3: display critical, warning and informational messages\n"
		   "4: display critical, warning, informational and debug messages\n"
		), "level", "1");
	oParser.addOption(oShowProgressOpt);

	// Parse the arguments
	if(!oParser.parse(arguments()))
	{
		QString s = oParser.errorText();
		qCritical() << oParser.errorText() << endl;
		oParser.showHelp();
		return CommandLineError;
	}

	// Check received arguments
	if(oParser.isSet(oHelpOpt))
	{
		oParser.showHelp();
		return CommandLineHelpRequested;
	}

	if(oParser.isSet(oVersionOpt))
	{
		oParser.showVersion();
		return CommandLineVersionRequested;
	}

	int iLogLevel = oParser.value(oShowProgressOpt).toInt();
	if(iLogLevel < Critical || iLogLevel > Debug)
	{
		qCritical().noquote() << tr("invalid level of progress messages %1").arg(iLogLevel) << endl;
		oParser.showHelp();
		return CommandLineError;
	}
	setLogLevel(static_cast<LogLevel>(iLogLevel));

	switch(oParser.positionalArguments().count())
	{
		case 0:
			qCritical().noquote() << tr("arguments %1 and %2 are required").arg("<input>").arg("<output>") << endl;
			oParser.showHelp();
			return CommandLineError;

		case 1:
			qCritical().noquote() << tr("argument %1 is required").arg("<output>") << endl;
			oParser.showHelp();
			return CommandLineError;

		case 2:
			break;

		default:
			qCritical().noquote() << tr("unknown arguments (only two required): %1").arg(oParser.positionalArguments().join(' ')) << endl;
			oParser.showHelp();
			return CommandLineError;
	}

	QString sInput = oParser.positionalArguments()[0];
	QString sOutput = oParser.positionalArguments()[1];

	QFileInfo oInput(sInput);
	QDir oDir = oInput.dir();
	oDir.setNameFilters(QStringList(oInput.fileName()));

	QStringList lInputFiles;
	foreach(QFileInfo oFile, oDir.entryInfoList())
		lInputFiles.append(oFile.absoluteFilePath());

	if(lInputFiles.count() == 0)
	{
		qCritical().noquote() << tr("invalid input file name or wildcard mask: %1").arg(sInput) << endl;
		oParser.showHelp();
		return CommandLineError;
	}

	QDir oOutput(sOutput);
	if(!oOutput.exists())
	{
		qCritical().noquote() << tr("invalid output directory: %1").arg(sInput) << endl;
		oParser.showHelp();
		return CommandLineError;
	}

	m_lInputFiles = lInputFiles;
	m_sOutputDir = sOutput;
	return CommandLineOk;
}

// +-----------------------------------------------------------
void fsdk::RuntimeApp::run()
{
	// Run an extraction task for each input file received from command line
	foreach(QString sFile, m_lInputFiles)
	{
		QFileInfo oFile(sFile);
		QString sLandmarks = QString("%1/%2-landmarks.csv").arg(m_sOutputDir, oFile.baseName());
		FeatureExtractor *pTask = new FeatureExtractor(sFile, sLandmarks);
		pTask->setAutoDelete(false);
		m_lTasks.append(pTask);

		connect(pTask, &FeatureExtractor::error, this, &RuntimeApp::onTaskError);
		connect(pTask, &FeatureExtractor::progress, this, &RuntimeApp::onTaskProgress);
		connect(pTask, &FeatureExtractor::finished, this, &RuntimeApp::onTaskFinished);
		
		QThreadPool::globalInstance()->start(pTask);
	}
}

// +-----------------------------------------------------------
void fsdk::RuntimeApp::onTaskError(QString sVideoFile, FeatureExtractor::ExtractionError eError)
{
	QFileInfo oFile;
	QString sLandmarks;

	switch(eError)
	{
		case FeatureExtractor::InvalidVideoFile:
			qCritical().noquote() << tr("error reading input video file %1").arg(sVideoFile);
			break;

		case FeatureExtractor::ErrorSavingLandmarks:
			oFile.setFile(sVideoFile);
			sLandmarks = QString("%1/%2-landmarks.csv").arg(m_sOutputDir, oFile.baseName());
			qCritical().noquote() << tr("error saving CSV data to output file %1").arg(sLandmarks);
			break;

		case FeatureExtractor::UnknownError:
			qCritical().noquote() << tr("unknown error while processing input video file %1").arg(sVideoFile);
			break;

		case FeatureExtractor::CancelRequested:
			qDebug().noquote() << tr("(cancellation) task for file %1 stoped.").arg(sVideoFile);
			return; // The thread that requested the cancellation is handling the conclusion
	}

	// Delete the pointer to the task that generated error
	FeatureExtractor *pErrorTask = static_cast<FeatureExtractor*>(sender());
	m_lTasks.removeOne(pErrorTask);
	delete pErrorTask;

	// Signal the cancellation to all remaining tasks and wait for them to stop
	foreach(FeatureExtractor *pTask, m_lTasks)
		pTask->cancel();
	QThreadPool::globalInstance()->waitForDone();

	// Delete the remaining pointers
	foreach(FeatureExtractor *pTask, m_lTasks)
		delete pTask;
	m_lTasks.clear();
	
	// Signal the termination with the error code as the index of the task
	int iError = m_lInputFiles.indexOf(sVideoFile) + 1;
	m_lInputFiles.clear();
	exit(iError);
}

// +-----------------------------------------------------------
void fsdk::RuntimeApp::onTaskProgress(QString sVideoFile, int iProgress)
{
	qDebug().noquote() << tr("file %1 progress: %2%").arg(sVideoFile).arg(iProgress);
}

// +-----------------------------------------------------------
void fsdk::RuntimeApp::onTaskFinished(QString sVideoFile)
{
	qInfo().noquote() << tr("file %1 done.").arg(sVideoFile);

	// Delete the pointer to the task that concluded
	FeatureExtractor *pTask = static_cast<FeatureExtractor*>(sender());
	m_lTasks.removeOne(pTask);
	delete pTask;

	// If all tasks concluded, indicate successful termination
	m_lInputFiles.removeOne(sVideoFile);
	if(m_lInputFiles.count() == 0)
		exit(0);
}