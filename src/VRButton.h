#ifndef VRBUTTON_H
#define VRBUTTON_H

#include "VRMenuElement.h"

class VRButton : public VRMenuElement {
	public:
		VRButton(std::string name, std::string text = "", bool isContinous = false);
		virtual ~VRButton();

		virtual void draw();
		virtual void click(double x, double y, bool isDown);
		virtual void updateIteration();

	private:
		bool m_isContinous;
		bool m_clicked;
	};

#endif //VRVRBUTTON_H