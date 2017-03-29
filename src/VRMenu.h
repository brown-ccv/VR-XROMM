#ifndef VRMENU_H
#define VRMENU_H

#include <math/VRMath.h>

class VRMenuElement;
class VRMenuHandler;

class VRMenu {
public:
	VRMenu(double width, double height, int col, int row, std::string title, double titleHeight = 0.05);
	~VRMenu();

	void draw();
	VRMenuElement * intersect(MinVR::VRPoint3& position, MinVR::VRVector3& direction, double &distance);
	bool click(bool isDown);
	void updateIteration();

	void addElement(VRMenuElement * element, int col, int row, int width, int height);
	void setTransformation(MinVR::VRMatrix4& transformation);
	void setVisible(bool visible);
	void sendEvent(VRMenuElement * element);
	void addMenuHandler(VRMenuHandler * handler);

private:
	int m_col;
	int m_row;
	double m_width, m_height;
	double m_col_width;
	double m_row_height;
	
	std::string m_title;
	double m_titleHeight;

	bool m_visible;
	bool m_hover;
	MinVR::VRMatrix4 m_transformation;

	std::vector<VRMenuElement *> m_elements;
	std::vector<VRMenuHandler *> m_handlers;

	VRMenuElement * m_activeElement;
	MinVR::VRPoint3 m_interactionPoint;
	bool m_isMouseDown;
};

#endif //VRMENU_H
