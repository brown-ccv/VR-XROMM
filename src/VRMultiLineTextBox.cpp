#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "VRMultiLineTextBox.h"

VRMultiLineTextBox::VRMultiLineTextBox(std::string name, std::vector<std::string> text, VRFontHandler::TextAlignment alignment) : VRMenuElement(name, ""), m_alignment(alignment), m_multiLineText(text), m_drawOutline(false)
{

}

VRMultiLineTextBox::~VRMultiLineTextBox()
{

}

void VRMultiLineTextBox::draw()
{
	if (m_drawOutline){
		// Draw Outline
		glColor3f(0.0f, 0.0, 0.0f);
		glBegin(GL_LINE_STRIP);
		glVertex3f(m_x, m_y + m_height, Z_OFFSET);					// Top Left
		glVertex3f(m_x + m_width, m_y + m_height, Z_OFFSET);		// Top Right
		glVertex3f(m_x + m_width, m_y, Z_OFFSET);					// Bottom Right
		glVertex3f(m_x, m_y, Z_OFFSET);							// Bottom Left
		glVertex3f(m_x, m_y + m_height, Z_OFFSET);					// Top Left
		glEnd();
	}

	VRFontHandler::getInstance()->renderMultiLineTextBox(m_multiLineText, m_x, m_y, Z_OFFSET, m_width, m_height, m_alignment);
}

void VRMultiLineTextBox::setText(std::vector<std::string> text)
{
	m_multiLineText = text;
}
