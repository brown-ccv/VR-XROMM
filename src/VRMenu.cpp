#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <math.h>
#include "VRMenu.h"
#include "VRFontHandler.h"
#include "VRMenuElement.h"
#include "VRMenuHandler.h"

#define BORDER 0.002

VRMenu::VRMenu(double width, double height, int col, int row, std::string title, double titleHeight) :m_width(width), m_height(height), m_col(col), m_row(row),
m_hover(false), m_title(title), m_titleHeight(titleHeight), m_activeElement(NULL), m_isMouseDown(false)
{
	m_col_width = width/col;
	m_row_height = height / row;	

}

VRMenu::~VRMenu()
{
	for (std::vector< VRMenuElement*>::iterator it = m_elements.begin(); it != m_elements.end(); ++it)
	{
		delete (*it);
	}
	m_elements.clear();

	m_handlers.clear();
}

void VRMenu::draw()
{
	if (m_visible)
	{
		GLint depth_funct;
		glGetIntegerv(GL_DEPTH_FUNC, &depth_funct);
		glDepthFunc(GL_LEQUAL);
		glPushMatrix();
		glMultMatrixf(m_transformation.getArray());

		glBegin(GL_QUADS);			
			// Draw A Quad		
			if (m_hover)
			{
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			else
			{
				glColor3f(1.0f, 1.0f, 1.0f);
			}	
			glVertex3f(-m_width*0.5, m_height + m_titleHeight, 0.0f);              // Top Left
			glVertex3f(m_width*0.5, m_height + m_titleHeight, 0.0f);				// Top Right
			glVertex3f(m_width*0.5, 0.0f, 0.0f);					// Bottom Right
			glVertex3f(-m_width*0.5, 0.0f, 0.0f);              // Bottom Left
		glEnd();

		glColor3f(0.0f, 0.0, 0.0f);
		glBegin(GL_LINE_STRIP);
		// Draw A Quad
		glVertex3f(-m_width*0.5, m_height + m_titleHeight, Z_OFFSET);              // Top Left
		glVertex3f(m_width*0.5, m_height + m_titleHeight, Z_OFFSET);				// Top Right
		glVertex3f(m_width*0.5, 0.0f, 0.001f);					// Bottom Right
		glVertex3f(-m_width*0.5, 0.0f, 0.001f);              // Bottom Left
		glVertex3f(-m_width*0.5, m_height + m_titleHeight, Z_OFFSET);              // Top Left
		glEnd();

		glColor3f(0.0f, 0.0, 0.0f);
		glBegin(GL_LINES);
		// Draw A Quad
		glVertex3f(-m_width*0.5, m_height, Z_OFFSET);              // Top Left
		glVertex3f(m_width*0.5, m_height, Z_OFFSET);				// Top Right
		glEnd();

		if (!m_title.empty())
			VRFontHandler::getInstance()->renderTextBox(m_title, -m_width*0.5 + BORDER, m_height + BORDER, Z_OFFSET, m_width - 2.0 * BORDER, m_titleHeight - 2.0 * BORDER);

		for (std::vector<VRMenuElement*>::const_iterator it = m_elements.begin(); it != m_elements.end(); ++it)
		{
			(*it)->draw();
		}
		glPopMatrix();
		glDepthFunc(depth_funct);
	}
}

VRMenuElement* VRMenu::intersect(MinVR::VRPoint3& position, MinVR::VRVector3& direction, double &distance)
{
	if (m_visible)
	{
		m_hover = false;
		for (std::vector<VRMenuElement*>::const_iterator it = m_elements.begin(); it != m_elements.end(); ++it)
		{
			(*it)->resetHover();
		}

		MinVR::VRPoint3 pt = m_transformation.inverse() * position;
		MinVR::VRVector3 dir = m_transformation.inverse() * direction;

		distance = - pt.z / dir.z; 
		m_interactionPoint = pt + dir * distance;
		if (m_isMouseDown && m_activeElement)
		{
			//when mouse down we do not change the active element
			m_activeElement->updateMousePosition(m_interactionPoint.x, m_interactionPoint.y);
			return m_activeElement;
		}
		else{
			if (distance > 0 && distance < 1.0 && fabs(m_interactionPoint.x) <= m_width * 0.5 && m_interactionPoint.y >= 0 && m_interactionPoint.y <= m_height + m_titleHeight){
				for (std::vector<VRMenuElement*>::const_iterator it = m_elements.begin(); it != m_elements.end(); ++it)
				{
					if ((*it)->checkIntersect(m_interactionPoint))
					{
						m_activeElement = *it;
						return (*it);
					}
				}
				m_hover = true;
			}
		}
	}

	m_activeElement = NULL;
	return NULL;
}

bool VRMenu::click(bool isDown)
{
	m_isMouseDown = isDown;
	if (m_activeElement)
	{
		m_activeElement->click(m_interactionPoint.x, m_interactionPoint.y, isDown);
		return true;
	}
	return false;
}

void VRMenu::updateIteration()
{
	if (m_activeElement)
	{
		m_activeElement->updateIteration();
	}
}

void VRMenu::addElement(VRMenuElement* element, int col, int row, int width, int height)
{
	double el_x = -m_width * 0.5 + m_col_width * (col-1);
	double el_y = m_height - m_row_height * (row + height - 1);//
	element->addToMenu(this, el_x + BORDER, el_y + BORDER, m_col_width * width - 2.0f * BORDER, m_row_height * height - 2.0f * BORDER);
	m_elements.push_back(element);
}

void VRMenu::setTransformation(MinVR::VRMatrix4& transformation)
{
	m_transformation = transformation;
}

void VRMenu::setVisible(bool visible)
{
	m_visible = visible;
	if (!visible){
		if (m_isMouseDown){
			//if mouse is down we have to release it
			if(m_activeElement) m_activeElement->click(m_interactionPoint.x, m_interactionPoint.y, false);
			m_isMouseDown = false;
		}
		m_activeElement = NULL;
	}
}

void VRMenu::sendEvent(VRMenuElement* element)
{
	for (std::vector<VRMenuHandler*>::const_iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
		(*it)->handleEvent(element);
}

void VRMenu::addMenuHandler(VRMenuHandler* handler)
{
	m_handlers.push_back(handler);
}
