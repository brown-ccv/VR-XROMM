#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "VRTextBox.h"

VRTextBox::VRTextBox(std::string name, std::string text, VRFontHandler::TextAlignment alignment) : VRMenuElement(name,text), m_alignment(alignment)
{

}

VRTextBox::~VRTextBox()
{

}

void VRTextBox::draw()
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

	if (!m_text.empty())
		VRFontHandler::getInstance()->renderTextBox(m_text, m_x, m_y, Z_OFFSET, m_width, m_height, m_alignment);
}

void VRTextBox::setText(std::string text)
{
	m_text = text;
}
