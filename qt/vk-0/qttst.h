/**
 * minimal qt vk example
 * @author Tobias Weber
 * @date Nov-2017
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __QTTST_H__
#define __QTTST_H__

#include <QVulkanWindow>
#include <memory>


class VkWnd : public QVulkanWindow
{
public:
	VkWnd(QWindow* parent=nullptr);
	virtual ~VkWnd();
};


#endif
