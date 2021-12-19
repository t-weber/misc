/**
 * qt list widget example
 * @author Tobias Weber
 * @date Dec-2021
 * @license see 'LiCENSE.GPL' file
 */

#include "qttst.h"

#include <QApplication>
#include <QGridLayout>
#include <QHeaderView>
#include <QSpinBox>

#include <locale>
#include <iostream>



// ----------------------------------------------------------------------------
TstDlg::TstDlg(QWidget* pParent) : QDialog{pParent}
{
	m_listWidget = std::make_shared<QWidget>(this);

	m_list = std::make_shared<QListWidget>(m_listWidget.get());
	m_list->setSortingEnabled(true);
	m_list->setMouseTracking(true);
	m_list->setContextMenuPolicy(Qt::CustomContextMenu);

	m_listBtnAdd = std::make_shared<QToolButton>(m_listWidget.get());
	m_listBtnDel = std::make_shared<QToolButton>(m_listWidget.get());
	m_listBtnUp = std::make_shared<QToolButton>(m_listWidget.get());
	m_listBtnDown = std::make_shared<QToolButton>(m_listWidget.get());

	m_listBtnAdd->setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});
	m_listBtnDel->setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});
	m_listBtnUp->setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});
	m_listBtnDown->setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});

	m_listBtnAdd->setText("\xe2\x8a\x95");
	m_listBtnDel->setText("\xe2\x8a\x96");
	m_listBtnUp->setText("\342\206\221");
	m_listBtnDown->setText("\342\206\223");

	m_listBtnAdd->setToolTip("Add item.");
	m_listBtnDel->setToolTip("Delete selected item(s).");
	m_listBtnUp->setToolTip("Move selected item(s) up.");
	m_listBtnDown->setToolTip("Move selected item(s) down.");

	// list widget grid
	auto grid = new QGridLayout(m_listWidget.get());
	grid->setSpacing(2);
	grid->setContentsMargins(4,4,4,4);
	grid->addWidget(m_list.get(), 0,0,1,5);
	grid->addWidget(m_listBtnAdd.get(), 1,0,1,1);
	grid->addWidget(m_listBtnDel.get(), 1,1,1,1);
	grid->addItem(new QSpacerItem(4, 4, QSizePolicy::Expanding, QSizePolicy::Minimum), 1,2,1,1);
	grid->addWidget(m_listBtnUp.get(), 1,3,1,1);
	grid->addWidget(m_listBtnDown.get(), 1,4,1,1);


	// list widget context menu
	m_listContextMenu = std::make_shared<QMenu>(m_list.get());
	m_listContextMenu->addAction("Add Item Before", this, [this]() { this->AddListItem(-2); });
	m_listContextMenu->addAction("Add Item After", this, [this]() { this->AddListItem(-3); });
	m_listContextMenu->addAction("Delete Item", this, &TstDlg::DelListItem);


	// signals
	connect(m_listBtnAdd.get(), &QToolButton::clicked, this, [this]() { this->AddListItem(-1); });
	connect(m_listBtnDel.get(), &QToolButton::clicked, this, &TstDlg::DelListItem);
	connect(m_listBtnUp.get(), &QToolButton::clicked, this, &TstDlg::MoveListItemUp);
	connect(m_listBtnDown.get(), &QToolButton::clicked, this, &TstDlg::MoveListItemDown);
	connect(m_list.get(), &QListWidget::currentItemChanged, this, &TstDlg::ListItemChanged);
	connect(m_list.get(), &QListWidget::itemEntered, this, &TstDlg::ListItemEntered);
	connect(m_list.get(), &QListWidget::customContextMenuRequested, this, &TstDlg::ShowListContextMenu);


	// main grid
	auto grid_dlg = new QGridLayout(this);
	grid_dlg->setSpacing(2);
	grid_dlg->setContentsMargins(4,4,4,4);
	grid_dlg->addWidget(m_listWidget.get(), 0,0,1,1);
}


void TstDlg::AddListItem(int row)
{
	if(row == -1)	// append to end of list
		row = m_list->count();
	else if(row == -2 && m_cursor_row >= 0)	// use row from member variable
		row = m_cursor_row;
	else if(row == -3 && m_cursor_row >= 0)	// use row from member variable +1
		row = m_cursor_row + 1;

	//bool sorting = m_list->isSortingEnabled();
	m_list->setSortingEnabled(false);

	int item_num = m_list->count()+1;
	//m_list->insertItem(row, new QListWidgetItem(QString("Item %1").arg(item_num)));
	m_list->insertItem(row, new NumericListWidgetItem<int>(QString("Item %1").arg(item_num), item_num));

	QListWidgetItem *item = m_list->item(row);
	m_list->scrollToItem(item);
	m_list->setCurrentItem(item);

	m_list->setSortingEnabled(/*sorting*/ true);
}


void TstDlg::DelListItem()
{
	// if nothing is selected, clear all items
	if(m_list->selectedItems().count() == 0)
		m_list->clear();

	for(int row : GetSelectedRows(true))
	{
		if(QListWidgetItem* item = m_list->item(row); item)
			delete item;
	}
}


void TstDlg::MoveListItemUp()
{
	m_list->setSortingEnabled(false);

	auto selected = GetSelectedRows(false);
	for(int row : selected)
	{
		if(row == 0)
			continue;

		auto *item = m_list->item(row);
		if(!item || !item->isSelected())
			continue;

		m_list->insertItem(row-1, m_list->takeItem(row));
	}

	for(int row=0; row<m_list->count(); ++row)
	{
		if(auto *item = m_list->item(row);
			item && std::find(selected.begin(), selected.end(), row+1) != selected.end())
		{
			m_list->item(row)->setSelected(true);
		}
	}
}


void TstDlg::MoveListItemDown()
{
	m_list->setSortingEnabled(false);

	auto selected = GetSelectedRows(true);
	for(int row : selected)
	{
		if(row == m_list->count()-1)
			continue;

		auto *item = m_list->item(row);
		if(!item || !item->isSelected())
			continue;

		m_list->insertItem(row+1, m_list->takeItem(row));

		//m_list->insertItem(row+2, m_list->item(row)->clone());
		//if(QListWidgetItem* item = m_list->item(row); item)
		//	delete item;
	}

	for(int row=0; row<m_list->count(); ++row)
	{
		if(auto *item = m_list->item(row);
			item && std::find(selected.begin(), selected.end(), row-1) != selected.end())
		{
			m_list->item(row)->setSelected(true);
		}
	}
}


std::vector<int> TstDlg::GetSelectedRows(bool sort_reversed) const
{
	std::vector<int> vec;
	vec.reserve(m_list->selectedItems().size());

	for(int row=0; row<m_list->count(); ++row)
	{
		if(auto *item = m_list->item(row); item && item->isSelected())
			vec.push_back(row);
	}

	if(sort_reversed)
	{
		std::stable_sort(vec.begin(), vec.end(), [](int row1, int row2)
		{ return row1 > row2; });
	}

	return vec;
}


void TstDlg::ListItemChanged(QListWidgetItem* cur, [[maybe_unused]] QListWidgetItem* prev)
{
	std::cout << "item selected: " << cur->text().toStdString() << std::endl;
}


void TstDlg::ListItemEntered(QListWidgetItem* item)
{
	std::cout << "item entered: " << item->text().toStdString() << std::endl;
}


void TstDlg::ShowListContextMenu(const QPoint& pt)
{
	const auto* item = m_list->itemAt(pt);
	if(!item)
		return;

	m_cursor_row = m_list->row(item);
	auto ptGlob = m_list->mapToGlobal(pt);
	ptGlob.setY(ptGlob.y() + m_listContextMenu->sizeHint().height()/2);
	m_listContextMenu->popup(ptGlob);
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	auto app = std::make_unique<QApplication>(argc, argv);
	std::ios_base::sync_with_stdio(false);
	::setlocale(LC_ALL, "C");
	std::locale::global(std::locale("C"));
	QLocale::setDefault(QLocale::C);

	auto dlg = std::make_unique<TstDlg>(nullptr);
	dlg->resize(600, 500);
	dlg->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
