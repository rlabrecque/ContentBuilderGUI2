#include <QtWidgets>
#include <QtCore\qdatetime.h>
#include <QtGui\QIntValidator>

#include "mainwindow.h"
#include "tabpage.h"

MainWindow::MainWindow() {
	QFile file(QString("builder/steamcmd.exe"));
	if (!file.exists()) {
		QMessageBox::warning(this, tr("Error"), tr("Could not find steamcmd.exe\nContent Builder GUI must be launched from \"sdk/tools/ContentBuilder/\""));
		exit(0);
	}

	QWidget *widget = new QWidget;
	setCentralWidget(widget);

	createMenuActions();
	createMenus();

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addWidget(createAccountSettingsGroup());

	QHBoxLayout *middleLayout = new QHBoxLayout();
	middleLayout->addWidget(createAppSettingsGroup());
	middleLayout->addWidget(createDepotSettingsGroup());
	mainLayout->addLayout(middleLayout);

	m_pOutputPlainTextEdit = new QPlainTextEdit();
	m_pOutputPlainTextEdit->setReadOnly(true);
	m_pOutputPlainTextEdit->setMinimumSize(800, 100);
	//m_pOutputPlainTextEdit->setMaximumSize(INT_MAX, 100);
	mainLayout->addWidget(m_pOutputPlainTextEdit);

	m_pRunPushButton = new QPushButton(tr("Run"));
	connect(m_pRunPushButton, SIGNAL(clicked()), this, SLOT(onRunButton()));
	mainLayout->addWidget(m_pRunPushButton);

	createProcess();

	widget->setLayout(mainLayout);
	setWindowTitle(tr("Content Builder GUI"));

	if (!loadConfig()) {
		//Todo: Run First time setup wizard
		//Todo: error handling
		// Defaults:
		TabPage *pDepot = new TabPage(this);
		pDepot->setDefaults(m_pDepotsTabWidget);
	}
}

bool MainWindow::loadConfig() {
	QFile file(QLatin1String("contentbuildergui/save.json"));
	if (!file.exists()) {
		return false;
	}

	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(this, "Error opening config file.", "Could not open the config file.");
		return false;
	}

	QJsonDocument loadDoc(QJsonDocument::fromJson(file.readAll()));
	QJsonObject obj = loadDoc.object();

	QString username = obj["username"].toString();
	if (!username.isEmpty()) {
		m_pUsernameLineEdit->setText(username);
	}

	QString password = obj["password"].toString();
	if (!password.isEmpty()) {
		m_pPasswordLineEdit->setText(username);
		m_pSavePasswordCheckbox->setChecked(true);
	}

	QJsonObject app = obj["app"].toObject();
	m_pAppIdLineEdit->setText(app["appid"].toString());
	m_pDescriptionLineEdit->setText(app["desc"].toString());
	m_pBuildOutputLineEdit->setText(app["buildoutput"].toString());
	m_pContentRootLineEdit->setText(app["contentroot"].toString());
	m_pSetLiveLineEdit->setText(app["setlive"].toString());
	m_pPreviewCheckbox->setChecked(app["preview"].toString() == "1" ? true : false);
	m_pLCSPathLineEdit->setText(app["local"].toString());

	QJsonArray depots = app["depots"].toArray();
	for (auto&& item : depots) {
		const QJsonObject& depot = item.toObject();
		TabPage *w = new TabPage(this);
		w->m_pDepotNameLineEdit->setText(depot["name"].toString());
		w->m_pDepotIdLineEdit->setText(depot["DepotID"].toString());

		{
			QJsonObject filemapping = depot["FileMapping"].toObject();
			w->m_pLocalPathLineEdit->setText(filemapping["LocalPath"].toString());
			w->m_pDepotPathLineEdit->setText(filemapping["DepotPath"].toString());
			w->m_pRecursiveCheckbox->setChecked(filemapping["recursive"].toString() == "1" ? true : false);
		}

		QJsonArray excludedfiles = depot["FileExclusion"].toArray();
		for (auto&& test : excludedfiles) {
			QListWidgetItem *item = new QListWidgetItem(test.toString(), w->m_pExclusionList);
			item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemNeverHasChildren);
		}

		m_pDepotsTabWidget->addTab(w, w->m_pDepotNameLineEdit->text());
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Creation
///////////////////////////////////////////////////////////////////////////////

void MainWindow::createMenuActions() {
	saveAct = new QAction(tr("Save"), this);
	saveAct->setShortcuts(QKeySequence::Save);
	saveAct->setStatusTip(tr("Save Username, Password and currently loaded content sheet."));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(onSave()));

	exitAct = new QAction(tr("Exit"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Quit ContentBuilderGUI."));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(onClose()));

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(onMenuHelpAbout()));
}

void MainWindow::createMenus() {
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(saveAct);
	fileMenu->addAction(exitAct);

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
}

void MainWindow::createProcess() {
	m_pSteamcmdProcess = new QProcess(this);
	connect(m_pSteamcmdProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(onStdoutAvailable()));
	connect(m_pSteamcmdProcess, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
}

QGroupBox *MainWindow::createAccountSettingsGroup() {
	QFormLayout *topLayout = new QFormLayout();

	QLabel *m_pUsernameLabel = new QLabel(tr("Username *:"), this);
	m_pUsernameLineEdit = new QLineEdit();
	topLayout->addRow(m_pUsernameLabel, m_pUsernameLineEdit);

	QLabel *m_pPasswordLabel = new QLabel(tr("Password *:"), this);
	m_pPasswordLineEdit = new QLineEdit();
	m_pPasswordLineEdit->setEchoMode(QLineEdit::EchoMode::Password);
	topLayout->addRow(m_pPasswordLabel, m_pPasswordLineEdit);

	QLabel *m_pSavePasswordLabel = new QLabel(tr("Save Password:"), this);
	m_pSavePasswordCheckbox = new QCheckBox(this);
	topLayout->addRow(m_pSavePasswordLabel, m_pSavePasswordCheckbox);

	QGroupBox *groupBox = new QGroupBox(tr("Account Settings"));
	groupBox->setLayout(topLayout);
	return groupBox;
}

QGroupBox *MainWindow::createAppSettingsGroup() {
	QFormLayout *appLayout = new QFormLayout();

	QLabel *pAppIdLabel = new QLabel(tr("AppId *:"), this);
	pAppIdLabel->setToolTip(tr("The AppId for your application can be found by selecting the application on your homepage in Steamworks. (Required)"));
	m_pAppIdLineEdit = new QLineEdit("18300");
	m_pAppIdLineEdit->setValidator(new QIntValidator(0, INT_MAX, m_pAppIdLineEdit));
	appLayout->addRow(pAppIdLabel, m_pAppIdLineEdit);

	QLabel *pDescriptionLabel = new QLabel(tr("Description:"), this);
	m_pDescriptionLineEdit = new QLineEdit();
	appLayout->addRow(pDescriptionLabel, m_pDescriptionLineEdit);

	QLabel *pBuildOutputLabel = new QLabel(tr("Build Output *:"), this);
	m_pBuildOutputLineEdit = new QLineEdit("../output/");
	appLayout->addRow(pBuildOutputLabel, m_pBuildOutputLineEdit);

	QLabel *pContentRootLabel = new QLabel(tr("Content Root *:"), this);
	m_pContentRootLineEdit = new QLineEdit("../content/");
	connect(m_pContentRootLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onContentRootChanged(QString)));
	appLayout->addRow(pContentRootLabel, m_pContentRootLineEdit);

	QLabel *pSetLiveLabel = new QLabel(tr("Set Live:"), this);
	m_pSetLiveLineEdit = new QLineEdit();
	appLayout->addRow(pSetLiveLabel, m_pSetLiveLineEdit);

	QLabel *pLCSPathLabel = new QLabel(tr("LCS Path:"), this);
	m_pLCSPathLineEdit = new QLineEdit();
	appLayout->addRow(pLCSPathLabel, m_pLCSPathLineEdit);

	QLabel *pPreviewLabel = new QLabel(tr("Preview:"), this);
	m_pPreviewCheckbox = new QCheckBox();
	m_pPreviewCheckbox->setChecked(true);
	appLayout->addRow(pPreviewLabel, m_pPreviewCheckbox);

	QFrame* line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	appLayout->addRow(line);

	m_pAbsolutePathLabel = new QLabel();
	m_pFileList = new QTreeWidget();
	
	m_pAbsolutePathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
	onContentRootChanged(m_pContentRootLineEdit->text());

	m_pFileList->setColumnCount(1);
	m_pFileList->setHeaderHidden(true);

	appLayout->addRow(m_pAbsolutePathLabel);
	appLayout->addRow(m_pFileList);

	QGroupBox *groupBox = new QGroupBox(tr("App Settings"));
	groupBox->setLayout(appLayout);
	return groupBox;
}

QGroupBox *MainWindow::createDepotSettingsGroup() {
	QHBoxLayout *depotLayout = new QHBoxLayout();

	m_pDepotsTabWidget = new QTabWidget();
	m_pDepotsTabWidget->setMovable(true);
	m_pDepotsTabWidget->setTabsClosable(true);
	m_pDepotsTabWidget->setUsesScrollButtons(true);
	connect(m_pDepotsTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));
	depotLayout->addWidget(m_pDepotsTabWidget);

	QToolButton *pNewTabToolButton = new QToolButton(m_pDepotsTabWidget);
	connect(pNewTabToolButton, SIGNAL(clicked()), this, SLOT(onNewTab()));
	pNewTabToolButton->setCursor(Qt::ArrowCursor);
	pNewTabToolButton->setAutoRaise(true);
	pNewTabToolButton->setText("+");
	//pNewTabToolButton->setIcon(QIcon(":/images/deviceAdd.png")); // TODO
	m_pDepotsTabWidget->setCornerWidget(pNewTabToolButton, Qt::TopRightCorner);
	
	QGroupBox *groupBox = new QGroupBox(tr("Depot Settings"));
	groupBox->setLayout(depotLayout);
	return groupBox;
}

///////////////////////////////////////////////////////////////////////////////
// Slots
///////////////////////////////////////////////////////////////////////////////

void MainWindow::onSave() {
	QJsonObject obj;
	if (!m_pUsernameLineEdit->text().isEmpty()) {
		obj["username"] = m_pUsernameLineEdit->text();
	}
	if (m_pSavePasswordCheckbox->isChecked() && !m_pPasswordLineEdit->text().isEmpty()) {
		obj["password"] = m_pPasswordLineEdit->text();
	}

	// App Settings
	// - This should move into prefab exporting
	{
		QJsonObject app;
		app["appid"] = m_pAppIdLineEdit->text();
		app["desc"] = m_pDescriptionLineEdit->text();
		app["buildoutput"] = m_pBuildOutputLineEdit->text();
		app["contentroot"] = m_pContentRootLineEdit->text();
		app["setlive"] = m_pSetLiveLineEdit->text();
		app["preview"] = (m_pPreviewCheckbox->isChecked() ? "1" : "0");
		app["local"] = m_pLCSPathLineEdit->text();
		{
			QJsonArray depots;
			for (int i = 0; i < m_pDepotsTabWidget->count(); ++i) {
				TabPage *w = qobject_cast<TabPage*>(m_pDepotsTabWidget->widget(i));
				QJsonObject depot;
				depot["name"] = w->m_pDepotNameLineEdit->text();
				depot["DepotID"] = w->m_pDepotIdLineEdit->text();

				{
					QJsonObject filemapping;
					filemapping["LocalPath"] = w->m_pLocalPathLineEdit->text();
					filemapping["DepotPath"] = w->m_pDepotPathLineEdit->text();
					filemapping["recursive"] = (w->m_pRecursiveCheckbox->isChecked() ? "1" : "0");

					depot["FileMapping"] = filemapping;
				}

				QJsonArray excludedfiles;
				for (int i = w->m_pExclusionList->count(); i--;) {
					QListWidgetItem* item = w->m_pExclusionList->item(i);
					if (item->text().isEmpty()) {
						delete item;
						continue;
					}

					excludedfiles.append(item->text());
				}
				depot["FileExclusion"] = excludedfiles;

				depots.append(depot);
			}
			app["depots"] = depots;
		}
		obj["app"] = app;
	}

	QJsonDocument saveDoc = QJsonDocument(obj);
	QFile saveFile("contentbuildergui/save.json");
	if (!saveFile.open(QIODevice::WriteOnly)) {
		m_pOutputPlainTextEdit->appendPlainText("Couldn't Open Save File.");
		return;
	}
	saveFile.write(saveDoc.toJson());
}

void MainWindow::onClose() {
	onSave();
	close();
}

void MainWindow::onMenuHelpAbout() {
	QMessageBox::about(this, tr("About"), tr("ContentBuilderGUI v0.0.1 created by Riley Labrecque"));
}

void MainWindow::onRunButton() {
	if (m_pUsernameLineEdit->text().isEmpty() || m_pPasswordLineEdit->text().isEmpty()) {
		m_pOutputPlainTextEdit->setPlainText(tr("You must add your Username and Password!"));
		return;
	}

	if (m_pAppIdLineEdit->text().isEmpty()) {
		m_pOutputPlainTextEdit->setPlainText(tr("You must set your AppId! For information on how to find your AppId view the help page.\n"));
		return;
	}

	if (m_pBuildOutputLineEdit->text().isEmpty()) { // TODO: Ensure that it's a valid folder
		m_pOutputPlainTextEdit->setPlainText(tr("You must have the build output directory set! The default value is \"../output/\", and is typically recommended."));
		return;
	}
	
	if (m_pContentRootLineEdit->text().isEmpty()) { // TODO: Ensure that it's a valid folder
		m_pOutputPlainTextEdit->setPlainText(tr("You must have the content root directory set! The default value is \"../content/\", and is typically recommended."));
		return;
	}

	if (m_pDepotsTabWidget->count() == 0) {
		m_pOutputPlainTextEdit->setPlainText(tr("You must have atleast one depot."));
		return;
	}

	for (int i = 0; i < m_pDepotsTabWidget->count(); ++i) {
		TabPage *w = qobject_cast<TabPage*>(m_pDepotsTabWidget->widget(i));
		if (w->m_pDepotIdLineEdit->text().isEmpty()) {
			m_pOutputPlainTextEdit->setPlainText(tr("You must set your DepotId! For information on how to find your DepotId view the help page."));
			return;
		}

		if (w->m_pLocalPathLineEdit->text().isEmpty()) {
			m_pOutputPlainTextEdit->setPlainText(tr("You must have the Local Path set! The default value is \"*\", and is typically recommended."));
			return;
		}

		if (w->m_pDepotPathLineEdit->text().isEmpty()) {
			m_pOutputPlainTextEdit->setPlainText(tr("You must have the Depot Path set! The default value is \".\", and is typically recommended."));
			return;
		}
	}

	m_pOutputPlainTextEdit->clear();

	{
		QDir dir;
		dir.mkdir("contentbuildergui");
	}

	QString appFilename = "contentbuildergui/app_build_tmp.vdf";
	QFile appFile(appFilename);
	if (!appFile.open(QIODevice::WriteOnly)) {
		m_pOutputPlainTextEdit->appendPlainText(tr("Could not open \"contentbuildergui/app_build_tmp.vdf\" for writing."));
		return;
	}
	
	{
		QTextStream stream(&appFile);
		stream << "\"appbuild\"" << endl;
		stream << "{" << endl;
		stream << "\t\"appid\" \"" << m_pAppIdLineEdit->text() << "\"" << endl;
		stream << "\t\"desc\" \"" << m_pDescriptionLineEdit->text() << "\"" << endl;
		stream << "\t\"buildoutput\" \"" << m_pBuildOutputLineEdit->text() << "\"" << endl;
		stream << "\t\"contentroot\" \"" << m_pContentRootLineEdit->text() << "\"" << endl;
		stream << "\t\"setlive\" \"" << m_pSetLiveLineEdit->text() << "\"" << endl;
		stream << "\t\"preview\" \"" << (m_pPreviewCheckbox->checkState() == Qt::CheckState::Checked ? "1" : "0") << "\"" << endl;
		stream << "\t\"local\" \"" << m_pLCSPathLineEdit->text() << "\"" << endl;
		stream << endl;
		stream << "\t\"depots\"" << endl;
		stream << "\t{" << endl;
		for (int i = 0; i < m_pDepotsTabWidget->count(); ++i) {
			TabPage *w = qobject_cast<TabPage*>(m_pDepotsTabWidget->widget(i));
			stream << "\t\t\"" << w->m_pDepotIdLineEdit->text() << "\" \"" << "depot_build_" << w->m_pDepotNameLineEdit->text() << "_tmp.vdf" << "\"" << endl;
		}
		stream << "\t}" << endl;
		stream << "}" << endl;
	}
	appFile.close();

	for (int i = 0; i < m_pDepotsTabWidget->count(); ++i) {
		TabPage *w = qobject_cast<TabPage*>(m_pDepotsTabWidget->widget(i));

		QString depotFilename;
		QTextStream stream1(&depotFilename);
		stream1 << "contentbuildergui/depot_build_" << w->m_pDepotNameLineEdit->text() << "_tmp.vdf";
		QFile depotFile(depotFilename);
		if (!depotFile.open(QIODevice::WriteOnly)) {
			appFile.close();
			m_pOutputPlainTextEdit->appendPlainText(tr("Could not open %1 for writing.").arg(depotFilename));
			return;
		}

		QTextStream stream(&depotFile);
		stream << "\"DepotBuildConfig\"" << endl;
		stream << "{" << endl;
		stream << "\t" << "\"DepotID\" \"" << w->m_pDepotIdLineEdit->text() << "\"" << endl;
		stream << endl;
		stream << "\t" << "\"FileMapping\"" << endl;
		stream << "\t" << "{" << endl;
		stream << "\t\t" << "\"LocalPath\" \"" << w->m_pLocalPathLineEdit->text() << "\"" << endl;
		stream << "\t\t" << "\"DepotPath\" \"" << w->m_pDepotPathLineEdit->text() << "\"" << endl;
		stream << "\t\t" << "\"recursive\" \"" << (w->m_pRecursiveCheckbox->checkState() == Qt::CheckState::Checked ? "1" : "0") << "\"" << endl;
		stream << "\t" << "}" << endl;
		stream << endl;
		for (int i = w->m_pExclusionList->count(); i--;) {
			QListWidgetItem* item = w->m_pExclusionList->item(i);
			if (item->text().isEmpty()) {
				delete item;
				continue;
			}

			stream << "\t" << "\"FileExclusion\" \"" << item->text() << "\"" << endl;
		}
		stream << "}" << endl;
		depotFile.close();
	}

	m_pSteamcmdProcess->start("builder/steamcmd.exe", QStringList() << "+login" << m_pUsernameLineEdit->text() << m_pPasswordLineEdit->text() << "+run_app_build" << "../contentbuildergui/app_build_tmp.vdf" << "+quit");
}

void MainWindow::onStdoutAvailable() {
	m_pOutputPlainTextEdit->appendPlainText(QString(m_pSteamcmdProcess->readAllStandardOutput()));
}

void MainWindow::onFinished(int exitCode) {
	m_pOutputPlainTextEdit->appendPlainText(tr("Executing Steamcmd finished with exit code: %1").arg(exitCode));
	// TODO: clean up
	/*QString filename = "contentbuildergui/app_build_tmp.vdf";
	QFile file(filename);
	if (file.exists()) {
		file.remove();
	}*/
}

void MainWindow::onContentRootChanged(const QString &text) {
	QDir dir("contentbuildergui/" + m_pContentRootLineEdit->text());
	if (dir.isAbsolutePath(m_pContentRootLineEdit->text())) {
		dir.setPath(m_pContentRootLineEdit->text());
	}
	
	if (dir.exists()) {
		m_pAbsolutePathLabel->setText(dir.absolutePath());
		m_pFileList->clear();

		if (m_pDepotsTabWidget == (void*)0xcccccccc) { // TODO: Setup all members before doing anything else.
			return;
		}

		for (int i = 0; i < m_pDepotsTabWidget->count(); ++i) {
			TabPage *w = qobject_cast<TabPage*>(m_pDepotsTabWidget->widget(i));
			QStringList list = QStringList(w->m_pDepotNameLineEdit->text());
			QTreeWidgetItem *test = new QTreeWidgetItem(m_pFileList, list);

			//QTreeWidgetItem *p = test;
			QDirIterator iterator(dir.absolutePath(), QDir::Filter::NoDotAndDotDot | QDir::Filter::AllEntries, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
			while (iterator.hasNext()) {
				iterator.next();
				QString filename = iterator.filePath();
				QTreeWidgetItem *n = new QTreeWidgetItem(test);
				n->setText(0, filename);
				/*if (iterator.fileInfo().isDir()) {
					p = n;
				}
				else {
					// set icon
				}*/
			}

			//QTreeWidgetItem *test2 = new QTreeWidgetItem(test, dirlist);
		}
	}
	else {
		m_pAbsolutePathLabel->setText(tr("Directory does not exist."));
	}
}

void MainWindow::onTabCloseRequested(int index) {
	if (m_pDepotsTabWidget->count() > 1) {
		delete m_pDepotsTabWidget->widget(index);
	}
	else {

	}
}

void MainWindow::onNewTab() {
	TabPage *pDepot = new TabPage(this);
	pDepot->setDefaults(m_pDepotsTabWidget);
	m_pDepotsTabWidget->setCurrentWidget(pDepot);
}

void MainWindow::onDepotNameChanged(const QString &text) {
	m_pDepotsTabWidget->setTabText(m_pDepotsTabWidget->currentIndex(), text);
}
