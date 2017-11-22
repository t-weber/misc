/**
 * minimal qt gl example
 * @author Tobias Weber
 * @date Nov-2017
 */

#ifndef __QTTST_H__
#define __QTTST_H__


#include <QDialog>
#include <QMainWindow>


class TstDlg : public QDialog
{
public:
	using QDialog::QDialog;
	TstDlg(QWidget* pParent = nullptr);
	~TstDlg() = default;

private:
};



class TstWnd : public QMainWindow
{
public:
	using QMainWindow::QMainWindow;
	TstWnd(QWidget* pParent = nullptr);
	~TstWnd() = default;

private:
};


#endif

