/**
 * bt test
 * @author Tobias Weber
 * @date May-2026
 * @license see 'LiCENSE.GPL' file
 *
 * References:
 *  * https://doc.qt.io/qt-6/qbluetoothdevicediscoveryagent.html
 *  * https://doc.qt.io/qt-6/qtbluetooth-lowenergyscanner-example.html
 *  * https://doc.qt.io/qt-6/qbluetoothdeviceinfo.html
 */

#include "bt.h"

#include <QApplication>
#include <QGridLayout>
#include <QPermissions>

#include <locale>
#include <iostream>


// ----------------------------------------------------------------------------
BtDlg::BtDlg(QWidget* parent) : QDialog{parent}
{
	m_listWidget = std::make_shared<QWidget>(this);

	m_list = std::make_shared<QListWidget>(m_listWidget.get());
	m_list->setSortingEnabled(true);
	m_list->setMouseTracking(true);
	m_list->setContextMenuPolicy(Qt::CustomContextMenu);

	m_btnScan = std::make_shared<QToolButton>(m_listWidget.get());
	m_btnScan->setText("Scan");

	m_btnStop = std::make_shared<QToolButton>(m_listWidget.get());
	m_btnStop->setText("Stop");

	m_checkLowPower = std::make_shared<QCheckBox>(m_listWidget.get());
	m_checkLowPower->setText("Low Power Devices");
	m_checkLowPower->setChecked(true);

	m_checkPermissions = std::make_shared<QCheckBox>(m_listWidget.get());
	m_checkPermissions->setText("Check Permissions");
	m_checkPermissions->setChecked(false);

	m_status = std::make_shared<QLabel>();
	m_status->setFrameStyle(QFrame::Panel);

	// list widget grid
	auto grid = new QGridLayout(m_listWidget.get());
	grid->setSpacing(6);
	grid->setContentsMargins(4, 4, 4, 4);
	grid->addWidget(m_list.get(), 0, 0, 1, 4);
	grid->addWidget(m_btnScan.get(), 1, 0, 1, 1);
	grid->addWidget(m_btnStop.get(), 1, 1, 1, 1);
	grid->addWidget(m_checkLowPower.get(), 1, 2, 1, 1);
	grid->addWidget(m_checkPermissions.get(), 1, 3, 1, 1);
	grid->addWidget(m_status.get(), 2, 0, 1, 4);


	// list widget context menu
	m_listContextMenu = std::make_shared<QMenu>(m_list.get());
	m_listContextMenu->addAction("Delete", this, &BtDlg::DelListItem);


	// signals
	connect(m_btnScan.get(), &QToolButton::clicked, this, [this]() { this->Scan(); });
	connect(m_btnStop.get(), &QToolButton::clicked, this, [this]() { this->Stop(); });
	connect(m_list.get(), &QListWidget::customContextMenuRequested, this, &BtDlg::ShowListContextMenu);


	// main grid
	auto grid_dlg = new QGridLayout(this);
	grid_dlg->setSpacing(6);
	grid_dlg->setContentsMargins(4, 4, 4, 4);
	grid_dlg->addWidget(m_listWidget.get(), 0, 0, 1, 1);
}


void BtDlg::Scan()
{
	Stop();
	m_list->clear();
	
	// bt discovery
	if(m_checkPermissions->isChecked())
	{
		QBluetoothPermission bt_perm;
		bt_perm.setCommunicationModes(QBluetoothPermission::Access);

		bool bt_granted = false;
		bt_granted = (qApp->checkPermission(bt_perm) == Qt::PermissionStatus::Granted);

		if(!bt_granted)
		{
			qApp->requestPermission(bt_perm, [&bt_granted](const QPermission& perm)
			{
				if(perm.status() == Qt::PermissionStatus::Granted)
					bt_granted = true;
			});
		}

		if(!bt_granted)
		{
			m_status->setText("No permissions granted.");
			return;
		}
	}

	m_btagent = std::make_shared<QBluetoothDeviceDiscoveryAgent>();

	connect(m_btagent.get(), &QBluetoothDeviceDiscoveryAgent::canceled, [this]()
	{
		m_status->setText("Bt discovery cancelled.");
	});

	connect(m_btagent.get(), &QBluetoothDeviceDiscoveryAgent::errorOccurred,
		[this](QBluetoothDeviceDiscoveryAgent::Error err)
	{
		m_status->setText(QString("Bt discovery error %1: %2.").
		  arg(int(err)).arg(m_btagent->errorString()));
	});

	connect(m_btagent.get(), &QBluetoothDeviceDiscoveryAgent::finished, [this]()
	{
		m_status->setText("Bt discovery finished.");
	});

	connect(m_btagent.get(), &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
		[this](const QBluetoothDeviceInfo& inf)
	{
		// see if the device is already in the list
		QString itemtext = QString("%1").arg(inf.name());
		if(m_list->findItems(itemtext, Qt::MatchExactly).size())
		{
			m_status->setText("Bt device already discovered.");
			return;
		}

		// add new device
		m_list->setSortingEnabled(false);
		QListWidgetItem *item = new QListWidgetItem(itemtext);
		m_list->insertItem(m_list->count(), item);
		m_list->scrollToItem(item);
		m_list->setCurrentItem(item);
		m_list->setSortingEnabled(true);

		m_status->setText("Bt device discovered.");
	});

	connect(m_btagent.get(), &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
		[this]([[__maybe_unused__]] const QBluetoothDeviceInfo& inf,
			[[__maybe_unused__]] QBluetoothDeviceInfo::Fields upd)
	{
		m_status->setText("Bt device updated.");
	});

	if(m_checkLowPower->isChecked())
	{
		m_btagent->setLowEnergyDiscoveryTimeout(0);
		m_btagent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
	}
	else
	{
		m_btagent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
	}

	m_status->setText("Bt discovery started.");
}



void BtDlg::Stop()
{
	if(m_btagent)
		m_btagent->stop();
}



void BtDlg::DelListItem()
{
	// if nothing is selected, clear all items
	if(m_list->selectedItems().count() == 0)
		m_list->clear();

	for(int row : GetSelectedRows(true))
	{
		if(QListWidgetItem* item = m_list->takeItem(row); item)
			delete item;
	}
}


std::vector<int> BtDlg::GetSelectedRows(bool sort_reversed) const
{
	std::vector<int> vec;
	vec.reserve(m_list->selectedItems().size());

	for(int row = 0; row < m_list->count(); ++row)
	{
		if(auto *item = m_list->item(row); item && item->isSelected())
			vec.push_back(row);
	}

	if(sort_reversed)
	{
		std::stable_sort(vec.begin(), vec.end(), [](int row1, int row2)
		{
			return row1 > row2;
		});
	}

	return vec;
}


void BtDlg::ShowListContextMenu(const QPoint& pt)
{
	const auto* item = m_list->itemAt(pt);
	if(!item)
		return;

	auto ptGlob = m_list->mapToGlobal(pt);
	ptGlob.setY(ptGlob.y() + m_listContextMenu->sizeHint().height()/2);
	m_listContextMenu->popup(ptGlob);
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
static inline void set_locales()
{
	std::ios_base::sync_with_stdio(false);

	::setlocale(LC_ALL, "C");
	std::locale::global(std::locale("C"));
	QLocale::setDefault(QLocale::C);
}


int main(int argc, char** argv)
{
	auto app = std::make_unique<QApplication>(argc, argv);
	set_locales();

	auto dlg = std::make_unique<BtDlg>(nullptr);
	dlg->resize(600, 500);
	dlg->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
