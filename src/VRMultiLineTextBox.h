#ifndef VRMULTILINETEXTBOX_H
#define VRMULTILINETEXTBOX_H

#include "VRMenuElement.h"
#include "VRFontHandler.h"

class VRMultiLineTextBox : public VRMenuElement {
public:
	VRMultiLineTextBox(std::string name, std::vector<std::string> text, VRFontHandler::TextAlignment alignment = VRFontHandler::CENTER);
	virtual ~VRMultiLineTextBox();

	virtual void draw();

	void setText(std::vector<std::string> text);

private:
	VRFontHandler::TextAlignment m_alignment;
	std::vector<std::string> m_multiLineText;

	bool m_drawOutline;
};

#endif //VRMULTILINETEXTBOX_H