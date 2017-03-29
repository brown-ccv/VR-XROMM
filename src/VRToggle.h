#ifndef VRTOGGLE_H
#define VRTOGGLE_H

#include "VRMenuElement.h"

class VRToggle : public VRMenuElement {
	public:
		VRToggle(std::string name, std::string text = "");
		virtual ~VRToggle();

		virtual void draw();
		virtual void click(double x, double y, bool isDown);

		void setToggled(bool isToggled);
		bool isToggled();
	private:
		bool m_isToggled;
	};

#endif //VRTOGGLE_H