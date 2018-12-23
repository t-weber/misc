/**
 * qt table widget example
 * @author Tobias Weber
 * @date Dec-2018
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__


#include <QDialog>
#include <QTableWidget>
#include <QToolButton>
#include <QMenu>
#include <vector>


class TstDlg : public QDialog
{
public:
	TstDlg(QWidget* pParent = nullptr);
	~TstDlg() = default;

protected:
	QWidget *m_pTabWidget = nullptr;
	QTableWidget *m_pTab = nullptr;

	QToolButton *m_pTabBtnAdd = nullptr;
	QToolButton *m_pTabBtnDel = nullptr;
	QToolButton *m_pTabBtnUp = nullptr;
	QToolButton *m_pTabBtnDown = nullptr;

	QMenu *m_pTabContextMenu = nullptr;

protected:
	void AddTabItem(int row = -1);
	void DelTabItem();
	void MoveTabItemUp();
	void MoveTabItemDown();

	void TableCellChanged(int rowNew, int colNew, int rowOld, int colOld);
	void TableCellEntered(const QModelIndex& idx);
	void ShowTableContextMenu(const QPoint& pt);

private:
	int m_iCursorRow = -1;

private:
	std::vector<int> GetSelectedRows(bool sort_reversed = false) const;
};


#endif
