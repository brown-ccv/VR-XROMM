#ifndef XMAOBJECT
#define XMAOBJECT

#include <string>
#include <vector>
#include <math/VRMath.h>

class XMAObject                   // begin declaration of the class
{
  public:
	// begin public section
	  XMAObject(std::string obj_file, std::string  transformation_file, float scale = 1.0);     // constructor
    ~XMAObject();                  // destructor
	void render(int frame);
	std::string getName();
	MinVR::VRMatrix4  getTransformation(int frame);
	int getTransformationSize();
	bool isVisible(int frame);
 private:                   // begin private section
    unsigned int displayList;              // member variable
	std::vector<float*> transformation;
	std::vector<bool> visible;
	std::string name;
	
	std::string getFilename(std::string path);
	std::istream& safeGetline(std::istream& is, std::string& t);
	bool StartsWith(const std::string& text, const std::string& token);
	std::istream& comma(std::istream& in);

};

#endif
