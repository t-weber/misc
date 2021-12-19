/**
 * qt list widget example
 * @author Tobias Weber
 * @date Dec-2021
 * @license see 'LiCENSE.GPL' file
 */

#include "qttst.h"

#include <QApplication>
#include <QGridLayout>
#include <QSplitter>
#include <QPainter>
#include <QPushButton>
#include <QFileDialog>

#include <locale>
#include <iostream>
#include <filesystem>



// ----------------------------------------------------------------------------
void ImageWidget::SetImage(const QString& img)
{
	if(img.isEmpty())
		return;

	if(!m_img.load(img))
		std::cerr << "Could not load image \"" << img.toStdString() << "\"." << std::endl;

	update();
}


void ImageWidget::paintEvent(QPaintEvent *evt)
{
	QWidget::paintEvent(evt);

	if(!m_img.isNull())
	{
		QPainter painter{};
		painter.begin(this);
		painter.drawPixmap(0,0,width(), height(), m_img);
		painter.end();
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
TstDlg::TstDlg(QWidget* pParent) : QDialog{pParent}
{
	// list widget
	m_list = std::make_shared<QListWidget>(this);
	m_list->setSortingEnabled(true);
	m_list->setMouseTracking(true);

	QPushButton *btnBrowse = new QPushButton("Browse...", this);

	// list widget grid
	QWidget *widget_list = new QWidget(this);
	auto grid_list = new QGridLayout(widget_list);
	grid_list->setSpacing(2);
	grid_list->setContentsMargins(4,4,4,4);
	grid_list->addWidget(m_list.get(), 0,0,1,1);
	grid_list->addWidget(btnBrowse, 1,0,1,1);

	// image widget
	m_image = std::make_shared<ImageWidget>(this);

	// signals
	connect(m_list.get(), &QListWidget::currentItemChanged, this, &TstDlg::ListItemChanged);
	connect(btnBrowse, &QAbstractButton::clicked, this, &TstDlg::BrowseDirs);

	// splitter
	QSplitter *split = new QSplitter(Qt::Horizontal, this);
	split->addWidget(/*m_list.get()*/ widget_list);
	split->addWidget(m_image.get());
	split->setStretchFactor(0, 1);
	split->setStretchFactor(1, 4);

	// grid
	auto grid_dlg = new QGridLayout(this);
	grid_dlg->setSpacing(2);
	grid_dlg->setContentsMargins(4,4,4,4);
	grid_dlg->addWidget(split, 0,0,1,1);
}


void TstDlg::BrowseDirs()
{
	QFileDialog dlg(this, "Select Image Directory");
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setOption(QFileDialog::ShowDirsOnly, true);

	if(dlg.exec() && dlg.selectedFiles().size())
		SetCurDir(dlg.selectedFiles()[0]);
}


void TstDlg::SetCurDir(const QString& path)
{
	m_list->clear();

	namespace fs = std::filesystem;
	using t_diriter = fs::directory_iterator;

	t_diriter diriter(path.toStdString());
	for(t_diriter iter=fs::begin(diriter); iter!=fs::end(diriter); ++iter)
	{
		const fs::directory_entry& ent = *iter;
		const fs::path& entpath = ent;

		if(fs::is_regular_file(entpath))
		{
			m_list->addItem(entpath.string().c_str());
		}
	}
}


void TstDlg::ListItemChanged(QListWidgetItem* cur, [[maybe_unused]] QListWidgetItem* prev)
{
	//std::cout << "item selected: " << cur->text().toStdString() << std::endl;
	m_image->SetImage(cur->text());
}

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	auto app = std::make_unique<QApplication>(argc, argv);
	std::ios_base::sync_with_stdio(false);
	::setlocale(LC_ALL, "C");
	std::locale::global(std::locale("C"));
	QLocale::setDefault(QLocale::C);

	auto dlg = std::make_unique<TstDlg>(nullptr);
	dlg->resize(600, 500);
	dlg->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
