#ifndef VRTEXTBOX_H
#define VRTEXTBOX_H

#include "VRMenuElement.h"
#include "VRFontHandler.h"

class VRTextBox : public VRMenuElement {
	public:
		VRTextBox(std::string name, std::string text = "", VRFontHandler::TextAlignment alignment = VRFontHandler::CENTER);
		virtual ~VRTextBox();

		virtual void draw();

		void setText(std::string text);

	private:
		VRFontHandler::TextAlignment m_alignment;
	};

#endif //VRVRBUTTONELEMENT_H