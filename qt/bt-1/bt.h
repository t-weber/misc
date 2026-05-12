/**
 * bt test
 * @author Tobias Weber
 * @date May-2026
 * @license see 'LICENSE.GPL' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__

#include <QDialog>
#include <QListWidget>
#include <QToolButton>
#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QBluetoothDeviceDiscoveryAgent>

#include <vector>
#include <sstream>
#include <memory>


class BtDlg : public QDialog
{
public:
	BtDlg(QWidget* parent = nullptr);
	~BtDlg() = default;

protected:
	std::shared_ptr<QWidget> m_listWidget{};
	std::shared_ptr<QLabel> m_status{};
	std::shared_ptr<QListWidget> m_list{};
	std::shared_ptr<QToolButton> m_btnScan{};
	std::shared_ptr<QToolButton> m_btnStop{};
	std::shared_ptr<QCheckBox> m_checkLowPower{};
	std::shared_ptr<QCheckBox> m_checkPermissions{};
	std::shared_ptr<QMenu> m_listContextMenu{};

	std::shared_ptr<QBluetoothDeviceDiscoveryAgent> m_btagent{};

protected:
	void Scan();
	void Stop();

	void DelListItem();
	void ShowListContextMenu(const QPoint& pt);

private:
	std::vector<int> GetSelectedRows(bool sort_reversed = false) const;
};


#endif
