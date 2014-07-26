#pragma once
#include <QMainWindow>

class QMenuBar;
class QGroupBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QProcess;
class QTabWidget;
class QListWidget;
class TabPage;
class QTreeWidget;

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow();

private slots:
	void onSave();
	void onClose();
	void onMenuHelpAbout();

	void onRunButton();
	void onStdoutAvailable();
	void onFinished(int exitCode);
	void onContentRootChanged(const QString &text);
	void onTabCloseRequested(int index);
	void onDepotNameChanged(const QString &text);
	void onNewTab();

private:
	bool loadConfig();
	// Creation
	void createMenuActions();
	void createMenus();
	void createProcess();
	//QWidget *createDepot();

	QGroupBox *createAccountSettingsGroup();
	QGroupBox *createAppSettingsGroup();
	QGroupBox *createDepotSettingsGroup();

	// Menu & Actions
	QMenu *fileMenu;
	QAction *saveAct;
	QAction *exitAct;

	QMenu *helpMenu;
	QAction *aboutAct;

	// Top Area
	QLineEdit *m_pUsernameLineEdit;
	QLineEdit *m_pPasswordLineEdit;
	QCheckBox *m_pSavePasswordCheckbox;
		
	// App Area
	QLineEdit *m_pAppIdLineEdit;
	QLineEdit *m_pDescriptionLineEdit;
	QLineEdit *m_pBuildOutputLineEdit;
	QLineEdit *m_pContentRootLineEdit;
	QLineEdit *m_pSetLiveLineEdit;
	QLineEdit *m_pLCSPathLineEdit;
	QCheckBox *m_pPreviewCheckbox;

	QLabel *m_pAbsolutePathLabel;
	QTreeWidget *m_pFileList;

	// Depot Area
	QTabWidget *m_pDepotsTabWidget;

	// Bottom Area
	QPlainTextEdit *m_pOutputPlainTextEdit;
	QPushButton *m_pRunPushButton;

	QProcess* m_pSteamcmdProcess;
};
