#include <QtWidgets>

#include "tabpage.h"


TabPage::TabPage(QWidget *parent, Qt::WindowFlags f) {
	QFormLayout *pLayout = new QFormLayout();
	setLayout(pLayout);

	{
		QLabel *pDepotNameLabel = new QLabel(tr("Depot Name *:"), this);
		m_pDepotNameLineEdit = new QLineEdit();
		connect(m_pDepotNameLineEdit, SIGNAL(textChanged(QString)), parent, SLOT(onDepotNameChanged(QString)));
		pLayout->addRow(pDepotNameLabel, m_pDepotNameLineEdit);
	}

	{
		QFrame* line = new QFrame();
		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		pLayout->addRow(line);
	}

	{
		QLabel *pDepotIdLabel = new QLabel(tr("DepotId *:"), this);
		m_pDepotIdLineEdit = new QLineEdit();
		m_pDepotIdLineEdit->setValidator(new QIntValidator(0, INT_MAX, m_pDepotIdLineEdit));
		pLayout->addRow(pDepotIdLabel, m_pDepotIdLineEdit);
	}

	{
		QGroupBox *pFileMappingGroupBox = new QGroupBox(tr("FileMapping"));
		QFormLayout *pFileMappingLayout = new QFormLayout();
		pFileMappingGroupBox->setLayout(pFileMappingLayout);
		pLayout->addRow(pFileMappingGroupBox);
		{
			QLabel *pLocalPathLabel = new QLabel(tr("LocalPath:"), this);
			m_pLocalPathLineEdit = new QLineEdit();
			pFileMappingLayout->addRow(pLocalPathLabel, m_pLocalPathLineEdit);
		}

		{
			QLabel *pDepotPathLabel = new QLabel(tr("DepotPath:"), this);
			m_pDepotPathLineEdit = new QLineEdit();
			pFileMappingLayout->addRow(pDepotPathLabel, m_pDepotPathLineEdit);
		}

		{
			QLabel *pRecursiveLabel = new QLabel(tr("Recursive:"), this);
			m_pRecursiveCheckbox = new QCheckBox();
			pFileMappingLayout->addRow(pRecursiveLabel, m_pRecursiveCheckbox);
		}
	}

	QGroupBox *pExclusionGroupBox = new QGroupBox(tr("Exclude Files"));
	QVBoxLayout *pFileMappingLayout = new QVBoxLayout();
	pExclusionGroupBox->setLayout(pFileMappingLayout);
	pLayout->addRow(pExclusionGroupBox);
	{
		m_pExclusionList = new QListWidget();
		m_pExclusionList->setSortingEnabled(true);
		m_pExclusionList->setFixedHeight(110); // Shows 6 without scrolling
		m_pExclusionList->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
		m_pExclusionList->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
		m_pExclusionList->setEditTriggers(QAbstractItemView::EditTrigger::DoubleClicked);
		pFileMappingLayout->addWidget(m_pExclusionList);
	}

	{
		QHBoxLayout *pButtonLayout = new QHBoxLayout();
		m_pAddExcludeItemButton = new QPushButton(tr("Add"));
		connect(m_pAddExcludeItemButton, SIGNAL(clicked()), this, SLOT(onAddExcludeButton()));
		pButtonLayout->addWidget(m_pAddExcludeItemButton);

		m_pRemoveExcludeItemButton = new QPushButton(tr("Remove"));
		connect(m_pRemoveExcludeItemButton, SIGNAL(clicked()), this, SLOT(onRemoveExcludeButton()));
		pButtonLayout->addWidget(m_pRemoveExcludeItemButton);
		pFileMappingLayout->addLayout(pButtonLayout);
	}
	
	// TODO: Advanced File mapping
	// https://partner.steamgames.com/documentation/steampipe#Advanced_File_Mapping
}

void TabPage::setDefaults(QTabWidget *pTabWidget) {
	m_pDepotNameLineEdit->setText("Default");

	m_pDepotIdLineEdit->setText("1001");

	m_pLocalPathLineEdit->setText("*");
	m_pDepotPathLineEdit->setText(".");
	m_pRecursiveCheckbox->setChecked(true);

	for (auto&& test : { "*.pdb", "steam_appid.txt", ".DS_Store" }) {
		QListWidgetItem *item = new QListWidgetItem(test, m_pExclusionList);
		item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemNeverHasChildren);
	}
	pTabWidget->addTab(this, m_pDepotNameLineEdit->text());
}

void TabPage::onAddExcludeButton() {
	m_pExclusionList->clearSelection();

	QListWidgetItem *item = new QListWidgetItem("", m_pExclusionList);
	item->setFlags(item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemNeverHasChildren);
	item->setSelected(true);
	m_pExclusionList->editItem(item);
}

void TabPage::onRemoveExcludeButton() {
	qDeleteAll(m_pExclusionList->selectedItems());
}

