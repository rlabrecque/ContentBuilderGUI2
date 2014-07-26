#pragma once
#include <QtWidgets>

class QLineEdit;
class QCheckBox;
class QPushButton;
class QListWidget;

class TabPage : public QWidget {
	Q_OBJECT

public:
	explicit TabPage(QWidget *parent, Qt::WindowFlags f = 0);
	void setDefaults(QTabWidget *pTabWidget);

private slots:
	void onAddExcludeButton();
	void onRemoveExcludeButton();

public:
	QLineEdit *m_pDepotNameLineEdit;

	QLineEdit *m_pDepotIdLineEdit;

	QLineEdit *m_pLocalPathLineEdit;
	QLineEdit *m_pDepotPathLineEdit;
	QCheckBox *m_pRecursiveCheckbox;

	QListWidget *m_pExclusionList;
	QPushButton *m_pAddExcludeItemButton;
	QPushButton *m_pRemoveExcludeItemButton;
};
