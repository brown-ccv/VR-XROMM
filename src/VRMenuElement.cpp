#include "VRMenuElement.h"



VRMenuElement::VRMenuElement(std::string name, std::string text) : m_hover(false), m_x(0), m_y(0), m_width(0), m_height(0), m_name(name), m_text(text){

}

VRMenuElement::~VRMenuElement()
{

}

void VRMenuElement::addToMenu(VRMenu * menu, double x, double y, double width, double height)
{
	m_menu = menu;
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
}

void VRMenuElement::resetHover()
{
	m_hover = false;
}

bool VRMenuElement::checkIntersect(MinVR::VRPoint3 &pt)
{
	if (pt.x >= m_x && pt.y >= m_y && pt.x <= m_x + m_width && pt.y <= m_y + m_height) {
		m_hover = true;
		return true;
	}
	return false;
}

std::string VRMenuElement::getName()
{
	return m_name;
}
