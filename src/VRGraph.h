#ifndef VRGRAPH_H
#define VRGRAPH_H

#include "VRMenuElement.h"
#include "VRFontHandler.h"

#define GRAPHUNDEFINEDVALUE -999999999

class VRGraph : public VRMenuElement {
public:
	VRGraph(std::string name, std::vector<double> data);
	virtual ~VRGraph();

	virtual void addToMenu(VRMenu * menu, double x, double y, double width, double height);
	virtual void draw();
	virtual bool checkIntersect(MinVR::VRPoint3 &pt);
	virtual void click(double x, double y, bool isDown);
	virtual void updateMousePosition(double x, double y);

	void setData(std::vector<double> data);
	void setCurrent(int current);
	int getSelection();

private:
	std::vector <double> m_data;
	double m_spacing[2];
	double m_range[2];
	int m_current;
	int m_selection;
	bool m_mouseDown;

	void computeBounds();
};

#endif //VRGRAPH_H