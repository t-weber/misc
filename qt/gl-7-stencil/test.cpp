/**
 * GL plotter test
 * @author Tobias Weber
 * @date Nov-2017, Jan-2022
 * @license see 'LICENSE.GPL' file
 */
#include "glplot.h"
#include "../../libs/math_conts.h"

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
		t_vec_gl plane_norm = m::create<t_vec_gl>({0., 0., 1.});
		t_vec_gl plane_pos = m::create<t_vec_gl>({0, 0., -2.});

		std::size_t plane_idx = 
			m_plots[0]->AddPlane(
				2.5,
				plane_pos[0], plane_pos[1], plane_pos[2],
				plane_norm[0], plane_norm[1], plane_norm[2],
				0., 0., 0., 1.);
		m_plots[0]->AddSphere(0.2, 0.,0.,2., 0.,0.,1.,1.);
		m_plots[0]->AddCone(1., 1., 0.,0.,0.,  0.,0.5,0.,1.);

		m_plots[0]->SetObjectPortal(plane_idx, true);


		// enlarging mirror without translation
		//t_mat_gl plane_mat = m::hom_scaling<t_mat_gl>(1.5f, 1.5f, -1.5f);

		// mirror
		t_mat_gl plane_mat = m::hom_mirror<t_mat_gl, t_vec_gl>(plane_norm, plane_pos, true);

		//using namespace m_ops;
		//std::cout << plane_mat << std::endl;

		m_plots[0]->SetObjectPortalMatrix(plane_idx, plane_mat);
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
