/**
 * minimal qt example
 * @author Tobias Weber
 * @date Nov-2017
 */

#include "qttst.h"
#include <QApplication>

#include <locale>
#include <memory>


// ----------------------------------------------------------------------------


TstDlg::TstDlg(QWidget* pParent) : QDialog{pParent}
{}


TstWnd::TstWnd(QWidget* pParent) : QMainWindow{pParent}
{}


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

	auto wnd = std::make_unique<TstWnd>();
	wnd->show();
	auto dlg = std::make_unique<TstDlg>();
	dlg->show();

	return app->exec();
}

// ----------------------------------------------------------------------------
