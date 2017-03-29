#ifndef XMAOBJECT
#define XMAOBJECT

#include <string>
#include <vector>
#include <math/VRMath.h>

class XMAObject                   // begin declaration of the class
{
  public:                    // begin public section
    XMAObject(std::string obj_file ,std::string  transformation_file);     // constructor
    ~XMAObject();                  // destructor
	void render(int frame);
	std::string getName();
	MinVR::VRMatrix4  getTransformation(int frame);
	int getTransformationSize();

 private:                   // begin private section
    unsigned int displayList;              // member variable
	std::vector<float*> transformation;
	std::string name;

	std::string getFilename(std::string path);
};

#endif
