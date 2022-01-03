/**
 * vk window
 * @author Tobias Weber
 * @date Feb-2021
 * @license see 'LICENSE.GPL' file
 */

#ifndef __QTVKTST_WND_H__
#define __QTVKTST_WND_H__

#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <QVulkanWindow>
#include <QVulkanWindowRenderer>
#include <QVulkanDeviceFunctions>
#include <QTimer>
#include <QMouseEvent>

#include <memory>

#include "../../libs/math_algos.h"
#include "../../libs/math_conts.h"
#include "renderer.h"
#include "cam.h"
#include "obj.h"
#include "pendulum.h"


class VkWnd : public QVulkanWindow
{ Q_OBJECT
protected:
	std::shared_ptr<QVulkanInstance> m_vkinst;
	VkRenderer *m_vkrenderer = nullptr;

	QTimer m_timer;
	std::chrono::milliseconds m_runningtime{0};

	std::size_t m_sphere_index{};
	std::size_t m_cyl_index{};
	Pendulum<t_vec, t_real> m_pendulum{5.5};

public:
	VkWnd(std::shared_ptr<QVulkanInstance>& vk, QWindow* parent=nullptr);
	virtual ~VkWnd();

	virtual QVulkanWindowRenderer* createRenderer() override;

	virtual void mouseMoveEvent(QMouseEvent *pEvt) override;
	virtual void keyPressEvent(QKeyEvent *pEvt) override;
	virtual void keyReleaseEvent(QKeyEvent *pEvt) override;

	void CreateObjects();
	VkRenderer* GetRenderer() { return m_vkrenderer; }

signals:
	void EmitStatusMsg(const QString& msg);
};


class Wnd : public QMainWindow
{
protected:
	VkWnd *m_vkwnd = nullptr;
	QWidget *m_vkwidget = nullptr;

	QStatusBar *m_statusbar = nullptr;
	QLabel *m_statuslabel = nullptr;

protected:
	virtual void resizeEvent(QResizeEvent *evt) override;

public:
	Wnd(VkWnd *vkwnd, QWidget *parent=nullptr);
	virtual ~Wnd();

	VkWnd* GetVkWnd() { return m_vkwnd; }
};

#endif
