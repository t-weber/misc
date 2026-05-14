/**
 * bt test
 * @author Tobias Weber
 * @date 13-May-2026
 * @license see 'LiCENSE.GPL' file
 *
 * References:
 *  * example:     https://doc.qt.io/qt-6/qtbluetooth-lowenergyscanner-example.html
 *  * classic API: https://doc.qt.io/qt-6/qtbluetooth-overview.html
 *  * low E API:   https://doc.qt.io/qt-6/qtbluetooth-le-overview.html
 *  * get bt devices:     https://doc.qt.io/qt-6/qbluetoothdevicediscoveryagent.html
 *  * get bt device info: https://doc.qt.io/qt-6/qbluetoothdeviceinfo.html
 *  * get services:       https://doc.qt.io/qt-6/qbluetoothserviceinfo.html
 *  * get services:       https://doc.qt.io/qt-6/qlowenergycontroller.html
 *  * service:            https://doc.qt.io/qt-6/qlowenergyservice.html
 */

#include "bt.h"

#include <QApplication>
#include <QGridLayout>
#include <QPermissions>
#include <QLoggingCategory>

#include <locale>
#include <iostream>


// ----------------------------------------------------------------------------
BtDlg::BtDlg(QWidget* parent) : QDialog{parent}
{
	setWindowTitle("Bt Test");

	m_treeWidget = std::make_shared<QWidget>(this);

	m_tree = std::make_shared<QTreeWidget>(m_treeWidget.get());
	m_tree->setSortingEnabled(true);
	//m_tree->setMouseTracking(true);
	//m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	m_tree->setColumnCount(1);
	m_tree->setHeaderLabels(QStringList("Name"));

	m_btnScan = std::make_shared<QToolButton>(m_treeWidget.get());
	m_btnScan->setText("Scan");

	m_btnStop = std::make_shared<QToolButton>(m_treeWidget.get());
	m_btnStop->setText("Stop");

	m_checkLowE = std::make_shared<QCheckBox>(m_treeWidget.get());
	m_checkLowE->setText("Low Energy Devices");
	m_checkLowE->setChecked(true);

	m_checkPermissions = std::make_shared<QCheckBox>(m_treeWidget.get());
	m_checkPermissions->setText("Check Permissions");
	m_checkPermissions->setChecked(false);

	m_status = std::make_shared<QLabel>();
	m_status->setFrameStyle(QFrame::Panel);

	// tree widget grid
	auto grid = new QGridLayout(m_treeWidget.get());
	grid->setSpacing(6);
	grid->setContentsMargins(4, 4, 4, 4);
	grid->addWidget(m_tree.get(), 0, 0, 1, 4);
	grid->addWidget(m_btnScan.get(), 1, 0, 1, 1);
	grid->addWidget(m_btnStop.get(), 1, 1, 1, 1);
	grid->addWidget(m_checkLowE.get(), 1, 2, 1, 1);
	grid->addWidget(m_checkPermissions.get(), 1, 3, 1, 1);
	grid->addWidget(m_status.get(), 2, 0, 1, 4);


	// signals
	connect(m_btnScan.get(), &QToolButton::clicked, this, [this]() { this->Scan(); });
	connect(m_btnStop.get(), &QToolButton::clicked, this, [this]() { this->Stop(); });


	// main grid
	auto grid_dlg = new QGridLayout(this);
	grid_dlg->setSpacing(6);
	grid_dlg->setContentsMargins(4, 4, 4, 4);
	grid_dlg->addWidget(m_treeWidget.get(), 0, 0, 1, 1);
}


void BtDlg::Scan()
{
	Stop();
	m_tree->clear();

	// bt permission check
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

	// bt device discovery
	m_btagent = std::make_shared<QBluetoothDeviceDiscoveryAgent>();
	bool low_E = m_checkLowE->isChecked();

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

	connect(m_btagent.get(), &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
		[this]([[__maybe_unused__]] const QBluetoothDeviceInfo& devinf,
			[[__maybe_unused__]] QBluetoothDeviceInfo::Fields upd)
	{
		m_status->setText("Bt device \"" + devinf.name() + "\" updated.");
	});

	connect(m_btagent.get(), &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
		[this, low_E](const QBluetoothDeviceInfo& devinf)
	{
		// see if the device is already in the tree
		QString itemtext = QString("%1").arg(devinf.name());
		if(m_tree->findItems(itemtext, Qt::MatchExactly).size())
		{
			m_status->setText("Bt device \"" + itemtext + "\" already discovered.");
			return;
		}

		// add new device
		m_tree->setSortingEnabled(false);
		QTreeWidgetItem *dev = new QTreeWidgetItem(QStringList(itemtext));
		m_tree->addTopLevelItem(dev);
		m_tree->scrollToItem(dev);
		m_tree->setCurrentItem(dev);
		m_tree->setSortingEnabled(true);

		for(const QBluetoothUuid uuid : devinf.serviceUuids())
		{
			QTreeWidgetItem *svc = new QTreeWidgetItem(QStringList(uuid.toString()));
			dev->addChild(svc);
		}

		m_status->setText("Bt device \"" + itemtext + "\" discovered.");

		// service discovery
		if(low_E)
		{
			// look for services from low-energy devices
			QLowEnergyController *ctrl =
				QLowEnergyController::createCentral(devinf, m_btagent.get());

			connect(ctrl, &QLowEnergyController::connected, [ctrl]()
			{
				std::cerr << "LowEnergyController connected." << std::endl;
				//std::cout << ctrl->services().size() << std::endl;
				ctrl->discoverServices();
			});

			connect(ctrl, &QLowEnergyController::disconnected, []()
			{
				std::cerr << "LowEnergyController disconnected." << std::endl;
			});

			connect(ctrl, &QLowEnergyController::errorOccurred,
				[ctrl](QLowEnergyController::Error err)
			{
				ctrl->disconnectFromDevice();
				std::cerr << "LowEnergyController error: " << (int)err << "." << std::endl;
			});

			connect(ctrl, &QLowEnergyController::stateChanged,
				[](QLowEnergyController::ControllerState s)
			{
				std::cerr << "LowEnergyController state changed to " << (int)s << "." << std::endl;
			});

			connect(ctrl, &QLowEnergyController::serviceDiscovered,
				[this, dev, ctrl](const QBluetoothUuid& uuid)
			{
				QLowEnergyService* obj = ctrl->createServiceObject(uuid, m_btagent.get());

				// see if the service is already in the tree
				for(QTreeWidgetItem* item : m_tree->findItems(obj->serviceName(), Qt::MatchExactly))
				{
					if(item->parent() == dev)
						return;
				}

				// add new service name to the tree item
				QTreeWidgetItem *svc = new QTreeWidgetItem(QStringList(obj->serviceName()));
				//QTreeWidgetItem *svc = new QTreeWidgetItem(QStringList(uuid.toString()));
				dev->addChild(svc);
			});

			connect(ctrl, &QLowEnergyController::connectionUpdated,
				[](const QLowEnergyConnectionParameters&)
			{
				std::cerr << "LowEnergyController paramters updated." << std::endl;
			});

			connect(ctrl, &QLowEnergyController::discoveryFinished, [ctrl]()
			{
				ctrl->disconnectFromDevice();
				std::cerr << "LowEnergyController discovery finished." << std::endl;
			});

			connect(ctrl, &QLowEnergyController::mtuChanged, [](int mtu)
			{
				std::cerr << "LowEnergyController MTU changed: " << mtu << "." << std::endl;
			});
	
			connect(ctrl, &QLowEnergyController::rssiRead, [](qint16 rssi)
			{
				std::cerr << "LowEnergyController RSSI read: " << rssi << "." << std::endl;
			});

			ctrl->disconnectFromDevice();
			ctrl->connectToDevice();
		}
		else
		{
			// service discovery for non-low-energy devices
			QBluetoothServiceDiscoveryAgent *ctrl =
				new QBluetoothServiceDiscoveryAgent(devinf.address(), m_btagent.get());

			connect(ctrl, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
				[this, dev](const QBluetoothServiceInfo& svcinf)
			{
				// see if the service is already in the tree
				for(QTreeWidgetItem* item : m_tree->findItems(svcinf.serviceName(), Qt::MatchExactly))
				{
					if(item->parent() == dev)
						return;
				}

				// add new service name to the tree item
				QTreeWidgetItem *svc = new QTreeWidgetItem(QStringList(svcinf.serviceName()));
				dev->addChild(svc);
			});

			connect(ctrl, &QBluetoothServiceDiscoveryAgent::errorOccurred,
				[ctrl](QBluetoothServiceDiscoveryAgent::Error err)
			{
				ctrl->stop();
				std::cerr << "BluetoothServiceDiscoveryAgent error: " << (int)err << "." << std::endl;
			});

			connect(ctrl, &QBluetoothServiceDiscoveryAgent::canceled, []()
			{
				//ctrl->stop();
				std::cerr << "BluetoothServiceDiscoveryAgent cancelled." << std::endl;
			});

			connect(ctrl, &QBluetoothServiceDiscoveryAgent::finished, []()
			{
				//ctrl->stop();
				std::cerr << "BluetoothServiceDiscoveryAgent finished." << std::endl;
			});

			ctrl->stop();
			ctrl->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
		}
	});

	if(low_E)
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
	QLoggingCategory::setFilterRules("*=false\n*.debug=false\n*.bluetooth* = true\n");
	qInstallMessageHandler([](QtMsgType ty, const QMessageLogContext& ctx, const QString& log) -> void
	{
		auto get_msg_type = [](const QtMsgType& _ty) -> std::string
		{
			switch(_ty)
			{
				case QtDebugMsg: return "debug";
				case QtWarningMsg: return "warning";
				case QtCriticalMsg: return "critical";
				case QtFatalMsg: return "fatal";
				case QtInfoMsg: return "info";
				default: return "<unknown>";
			}
		};

		auto get_str = [](const char* pc) -> std::string
		{
			if(!pc)
				return "<unknown>";
			return std::string{"\""} + std::string{pc} + std::string{"\""};
		};

		std::cerr << "qt " << get_msg_type(ty);
		if(ctx.function)
		{
			std::cerr << " in "
				<< "file " << get_str(ctx.file) << ", "
				<< "function " << get_str(ctx.function) << ", "
				<< "line " << ctx.line;
		}
		std::cerr << ": " << log.toStdString() << std::endl;
	});

	auto app = std::make_unique<QApplication>(argc, argv);
	set_locales();

	auto dlg = std::make_unique<BtDlg>(nullptr);
	dlg->resize(600, 500);
	dlg->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
