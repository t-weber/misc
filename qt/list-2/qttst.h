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
#include <QPixmap>

#include <vector>
#include <memory>


class ImageWidget : public QWidget
{
public:
	ImageWidget(QWidget* parent) : QWidget(parent)
	{};

	virtual ~ImageWidget() = default;

	void SetImage(const QString& img);

protected:
	virtual void paintEvent(QPaintEvent *evt) override;

private:
	QPixmap m_img{};
};


class TstDlg : public QDialog
{
public:
	TstDlg(QWidget* pParent = nullptr);
	virtual ~TstDlg() = default;

protected:
	std::shared_ptr<QListWidget> m_list{};
	std::shared_ptr<ImageWidget> m_image{};

protected:
	void ListItemChanged(QListWidgetItem* cur, QListWidgetItem* prev);

	void BrowseDirs();
	void SetCurDir(const QString& dir);

private:
	int m_cursor_row = -1;

private:
	std::vector<int> GetSelectedRows(bool sort_reversed = false) const;
};


#endif
