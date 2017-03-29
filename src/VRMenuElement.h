#ifndef VRMENUELEMENT_H
#define VRMENUELEMENT_H

#define Z_OFFSET 0.0001

#include <math/VRMath.h>

#include "VRMenu.h"

	class VRMenuElement {
	public:
		VRMenuElement(std::string name, std::string text = "");
		virtual ~VRMenuElement();

		virtual void draw() = 0;
		virtual void addToMenu(VRMenu * menu, double x, double y, double width, double height);
		virtual void resetHover();
		virtual void click(double x, double y, bool isDown){};
		virtual void updateIteration(){};
		virtual void updateMousePosition(double x, double y){};
		virtual bool checkIntersect(MinVR::VRPoint3 &pt);
		std::string getName();

	protected:
		bool m_hover;
		double m_x, m_y, m_width, m_height;
		std::string m_text;
		std::string m_name;

		VRMenu * m_menu;
	};

#endif //VRMENUELEMENT_H