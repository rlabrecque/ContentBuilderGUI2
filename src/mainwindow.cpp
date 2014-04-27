#include <QtWidgets>

#include "mainwindow.h"

MainWindow::MainWindow() {
	queryLabel = new QLabel(QApplication::translate("contentbuildergui", "Query:"));
	queryEdit = new QLineEdit();
	startButton = new QPushButton(QApplication::translate("contentbuildergui", "Run"));
	resultView = new QPlainTextEdit();
	resultView->setReadOnly(true);
	resultView->setMinimumSize(600, 200);

	QHBoxLayout *queryLayout = new QHBoxLayout();
	queryLayout->addWidget(queryLabel);
	queryLayout->addWidget(queryEdit);
	queryLayout->addWidget(startButton);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addLayout(queryLayout);
	mainLayout->addWidget(resultView);

	setLayout(mainLayout);
	setWindowTitle(QApplication::translate("contentbuildergui", "Content Builder GUI"));

	QProcess process;
	process.start("builder/steamcmd.exe", QStringList() << "+quit", QIODevice::ReadWrite | QIODevice::Text);
	if (!process.waitForFinished()) { // beware the timeout default parameter
		resultView->setPlainText(QString(process.readAllStandardOutput()));
		resultView->setPlainText(QString("Executing Steamcmd failed with exit code: %1").arg(process.exitCode()));
	}
	else {
		resultView->setPlainText(QString(process.readAllStandardOutput()));
	}
}

void MainWindow::createBatchSettingsGroupBox() {
	
}
