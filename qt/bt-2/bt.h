/**
 * bt test
 * @author Tobias Weber
 * @date 13-May-2026
 * @license see 'LICENSE.GPL' file
 */

#ifndef __BT_TST_H__
#define __BT_TST_H__


#include <QDialog>
#include <QTreeWidget>
#include <QToolButton>
#include <QCheckBox>
#include <QLabel>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QLowEnergyController>

#include <memory>


class BtDlg : public QDialog
{
public:
	BtDlg(QWidget* parent = nullptr);
	~BtDlg() = default;

protected:
	std::shared_ptr<QWidget> m_treeWidget{};
	std::shared_ptr<QTreeWidget> m_tree{};
	std::shared_ptr<QLabel> m_status{};
	std::shared_ptr<QToolButton> m_btnScan{}, m_btnStop{};
	std::shared_ptr<QCheckBox> m_checkLowE{}, m_checkPermissions{};

	std::shared_ptr<QBluetoothDeviceDiscoveryAgent> m_btagent{};

protected:
	void Scan();
	void Stop();
};


#endif
