#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QProcess;

class MainWindow : public QWidget {
	Q_OBJECT

public:
	MainWindow();

private:
	void createBatchSettingsGroupBox();
	void createAppSettingsGroupBox();
	void createDepotSettingsGroupBox();
	void createOutputGroupBox();

	QLabel *queryLabel;
	QLineEdit *queryEdit;
	QPushButton *startButton;
	QPlainTextEdit *resultView;

	QProcess* process;
};
