/**
 * GL plotter test
 * @author Tobias Weber
 * @date Nov-2017, Jan-2022
 * @license see 'LICENSE.GPL' file
 */
#include "glplot.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtGui/QSurfaceFormat>

#include <locale>
#include <iostream>



// ----------------------------------------------------------------------------
class PltDlg : public QDialog
{
private:
	std::vector<std::shared_ptr<GlPlot>> m_plots;

public:
	using QDialog::QDialog;

	PltDlg(QWidget* pParent) : QDialog{pParent, Qt::Window},
	m_plots{{ std::make_shared<GlPlot>(this) }}
	{
		setWindowTitle("Gl plotter test");

		auto pGrid = new QGridLayout(this);
		pGrid->setSpacing(2);
		pGrid->setContentsMargins(4,4,4,4);
		pGrid->addWidget(m_plots[0].get(), 0,0, 1,1);

		this->setSizeGripEnabled(true);

		connect(m_plots[0].get(), &GlPlot::AfterGLInitialisation, this, &PltDlg::AfterGLInitialisation);
	}


	// add test object for plot
	void AfterGLInitialisation()
	{
		std::size_t plane_idx = 
			m_plots[0]->AddPlane(2.5, 0.,0.,-4., 0.,0.,1., 0.,0.,0.,1.);
		m_plots[0]->AddSphere(0.2, 0.,0.,2., 0.,0.,1.,1.);
		m_plots[0]->AddCone(1., 1., 0.,0.,0.,  0.,0.5,0.,1.);

		// enlarging mirror
		m_plots[0]->SetObjectPortal(plane_idx, true);
		m_plots[0]->SetObjectPortalMatrix(plane_idx,
			m::hom_scaling<t_mat_gl>(1.5f, 1.5f, -1.5f));
	}
};
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
	set_gl_format(1, _GL_MAJ_VER, _GL_MIN_VER);
	set_locales();
	auto app = std::make_unique<QApplication>(argc, argv);

	auto dlg = std::make_unique<PltDlg>(nullptr);
	dlg->resize(800, 600);
	dlg->show();

	return app->exec();
}
// ----------------------------------------------------------------------------
