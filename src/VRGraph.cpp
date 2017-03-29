#ifdef _WIN32
#define NOMINMAX 
#include <windows.h>
#endif
#include <GL/gl.h>
#include <limits>
#include "VRGraph.h"

VRGraph::VRGraph(std::string name, std::vector<double> data) : VRMenuElement(name, ""), m_data(data), m_current(-1), m_selection(-1), m_mouseDown(false)
{
	computeBounds();
}

VRGraph::~VRGraph()
{

}

void VRGraph::addToMenu(VRMenu * menu, double x, double y, double width, double height)
{
	VRMenuElement::addToMenu(menu, x, y, width, height);
	computeBounds();
}

void VRGraph::draw()
{
	// Draw Outline
	glColor3f(0.0f, 0.0, 0.0f);
	glBegin(GL_LINE_STRIP);
	glVertex3f(m_x, m_y + m_height, Z_OFFSET);					// Top Left
	glVertex3f(m_x + m_width, m_y + m_height, Z_OFFSET);		// Top Right
	glVertex3f(m_x + m_width, m_y, Z_OFFSET);					// Bottom Right
	glVertex3f(m_x, m_y, Z_OFFSET);							// Bottom Left
	glVertex3f(m_x, m_y + m_height, Z_OFFSET);					// Top Left
	glEnd();

	glColor3f(0.0f, 0.0, 0.0f);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < m_data.size(); i++){
		if (m_data[i] != GRAPHUNDEFINEDVALUE)
			glVertex3f(m_x + m_spacing[0] * i, m_y + (m_data[i] - m_range[0]) * m_spacing[1], Z_OFFSET);
	}
	glEnd();

	if (m_current >= 0 && m_current < m_data.size())
	{
		glColor3f(0.9f, 0.0, 0.0f);
		glBegin(GL_LINE_STRIP);
		glVertex3f(m_x + m_spacing[0] * m_current, m_y , Z_OFFSET);
		glVertex3f(m_x + m_spacing[0] * m_current, m_y + (m_range[1] - m_range[0]) * m_spacing[1], Z_OFFSET);
		glEnd();
	}

	if (m_selection >= 0 && m_selection < m_data.size())
	{
		glColor3f(0.0f, 0.9, 0.0f);
		glBegin(GL_LINE_STRIP);
		glVertex3f(m_x + m_spacing[0] * m_selection, m_y, Z_OFFSET);
		glVertex3f(m_x + m_spacing[0] * m_selection, m_y + (m_range[1] - m_range[0]) * m_spacing[1], Z_OFFSET);
		glEnd();
	}
}

bool VRGraph::checkIntersect(MinVR::VRPoint3& pt)
{
	if (VRMenuElement::checkIntersect(pt))
	{
		m_selection = (pt.x - m_x) / m_spacing[0] + 0.5;
		if(m_selection < 0) m_selection = 0;
		if (m_selection >= m_data.size()) m_selection = m_data.size() - 1;
		return true;
	}
	m_selection = -1;
	return false;
}

void VRGraph::click(double x, double y, bool isDown)
{
	if (isDown)
	{
		m_mouseDown = true;
		m_menu->sendEvent(this);
	}
	else
	{
		m_mouseDown = false;
	}
}

void VRGraph::updateMousePosition(double x, double y)
{
	if (m_mouseDown)
	{
		m_selection = (x - m_x) / m_spacing[0] + 0.5;
		if (m_selection < 0) m_selection = 0;
		if (m_selection >= m_data.size()) m_selection = m_data.size() - 1;
		m_menu->sendEvent(this);
	}
}

void VRGraph::setData(std::vector<double> data)
{
	m_data = data;
	computeBounds();
}

void VRGraph::setCurrent(int current)
{
	m_current = current;
}

int VRGraph::getSelection()
{
	return m_selection;
}

void VRGraph::computeBounds()
{
	double min = std::numeric_limits<double>::max();
	double max = std::numeric_limits<double>::min();

	if (m_data.size() <= 0)
		return;

	for (std::vector<double>::const_iterator it = m_data.begin(); it != m_data.end(); ++it)
	{
		if (*it != GRAPHUNDEFINEDVALUE){
			if (min > *it) min = *it;
			if (max < *it) max = *it;
		}
	}
	m_range[0] = min;
	m_range[1] = max;

	m_spacing[0] = m_width / m_data.size();
	m_spacing[1] = m_height / (m_range[1] - m_range[0]);
}
