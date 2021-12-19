/**
 * qt list widget example
 * @author Tobias Weber
 * @date Dec-2021
 * @license see 'LICENSE.GPL' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__

#include <QDialog>
#include <QListWidget>
#include <QToolButton>
#include <QMenu>

#include <vector>
#include <sstream>
#include <memory>


template<class T = int>
class NumericListWidgetItem : public QListWidgetItem
{
public:
	NumericListWidgetItem(const QString& text, const T& val)
		: QListWidgetItem(text), m_val(val)
		//: QListWidgetItem(std::to_string(std::forward<T>(val)).c_str())
	{
	}

	virtual bool operator<(const QListWidgetItem& _item) const override
	{
		const NumericListWidgetItem *item = dynamic_cast<const NumericListWidgetItem*>(&_item);
		if(!item)
			return true;

		/*std::istringstream istr1{this->text().toStdString()};
		std::istringstream istr2{item.text().toStdString()};

		T val1{}, val2{};
		istr1 >> val1;
		istr2 >> val2;*/

		const T& val1 = this->GetValue();
		const T& val2 = item->GetValue();

		return val1 < val2;
	}

	const T& GetValue() const { return m_val; }


private:
	T m_val{};
};


class TstDlg : public QDialog
{
public:
	TstDlg(QWidget* pParent = nullptr);
	~TstDlg() = default;

protected:
	std::shared_ptr<QWidget> m_listWidget{};
	std::shared_ptr<QListWidget> m_list{};

	std::shared_ptr<QToolButton> m_listBtnAdd{};
	std::shared_ptr<QToolButton> m_listBtnDel{};
	std::shared_ptr<QToolButton> m_listBtnUp{};
	std::shared_ptr<QToolButton> m_listBtnDown{};

	std::shared_ptr<QMenu> m_listContextMenu{};

protected:
	void AddListItem(int row = -1);
	void DelListItem();
	void MoveListItemUp();
	void MoveListItemDown();

	void ListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev);
	void ListItemEntered(QListWidgetItem* item);
	void ShowListContextMenu(const QPoint& pt);

private:
	int m_cursor_row = -1;

private:
	std::vector<int> GetSelectedRows(bool sort_reversed = false) const;
};


#endif
