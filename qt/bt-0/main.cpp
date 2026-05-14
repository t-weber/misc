/**
 * bt test
 * @author Tobias Weber
 * @date may-2026
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *  * https://doc.qt.io/qt-6/qbluetoothdevicediscoveryagent.html
 *  * https://doc.qt.io/qt-6/qtbluetooth-lowenergyscanner-example.html
 */

#include <thread>
#include <future>
#include <memory>
#include <locale>
#include <iostream>

#include <QLocale>
#include <QLoggingCategory>
#include <QCoreApplication>
#include <QPermissions>
#include <QBluetoothDeviceDiscoveryAgent>


static inline void set_locales()
{
	std::ios_base::sync_with_stdio(false);

	::setlocale(LC_ALL, "C");
	std::locale::global(std::locale("C"));
	QLocale::setDefault(QLocale::C);
}


int main(int argc, char** argv)
{
	try
	{
		// ------------------------------------------------------------------------
		// misc initialisation
		// ------------------------------------------------------------------------
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

		auto app = std::make_unique<QCoreApplication>(argc, argv);
		set_locales();

		// this does not seem to work with the message handling loop in another thread...
		auto retval = std::async(std::launch::async, [&app]() -> int
		{			
			return app->exec();
		});
		// ------------------------------------------------------------------------


		// ------------------------------------------------------------------------
		// bt discovery
		// ------------------------------------------------------------------------
		QBluetoothPermission bt_perm;
		bt_perm.setCommunicationModes(QBluetoothPermission::Access);

		std::atomic<bool> bt_granted = false;
		bt_granted = (app->checkPermission(bt_perm) == Qt::PermissionStatus::Granted);

		if(!bt_granted)
		{
			app->requestPermission(bt_perm, [&bt_granted](const QPermission& perm)
			{
				//std::cout << (int)perm.status() << std::endl;
				if(perm.status() == Qt::PermissionStatus::Granted)
					bt_granted = true;
			});
		}

		int max_secs = 10;
		for(int i = 0; i < max_secs; ++i)
		{
			if(bt_granted)
				break;

			std::cout << "\rWaiting for permission " << i + 1 << " / " << max_secs << "...";
			std::cout.flush();
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		std::cout << std::endl;


		auto btagent = std::make_shared<QBluetoothDeviceDiscoveryAgent>();

		QObject::connect(btagent.get(), &QBluetoothDeviceDiscoveryAgent::canceled, []()
		{
			std::cerr << "Bt discovery canceled." << std::endl;
		});

		QObject::connect(btagent.get(), &QBluetoothDeviceDiscoveryAgent::errorOccurred,
			[](QBluetoothDeviceDiscoveryAgent::Error err)
		{
			std::cerr << "Bt discovery error " << err << "." << std::endl;
		});

		QObject::connect(btagent.get(), &QBluetoothDeviceDiscoveryAgent::finished, []()
		{
			std::cout << "Bt discovery finished." << std::endl;
		});

		QObject::connect(btagent.get(), &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
			[](const QBluetoothDeviceInfo& inf)
		{
			std::cout << "Bt device discovered: \""
				<< inf.name().toStdString() << "\"." << std::endl;
		});

		QObject::connect(btagent.get(), &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
			[](const QBluetoothDeviceInfo& inf, QBluetoothDeviceInfo::Fields upd)
		{
			std::cout << "Bt device updated." << std::endl;
		});

		btagent->setLowEnergyDiscoveryTimeout(/*60000*/ 0);
		//btagent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
		btagent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

		max_secs = 60;
		for(int i = 0; i < max_secs; ++i)
		{
			std::cout << "\rScanning " << i + 1 << " / " << max_secs << "...";
			std::cout.flush();
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		std::cout << std::endl;

		btagent->stop();

		QList<QBluetoothDeviceInfo> devs = btagent->discoveredDevices();
		std::cout << devs.size() << " devices discovered." << std::endl;
		// ------------------------------------------------------------------------


		return retval.get();
		//return app->exec();
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}
}
