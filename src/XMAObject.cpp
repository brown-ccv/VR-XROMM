#include "XMAObject.h"
#include "glm.h"
#include <iostream>
#include <fstream>
#include <math/VRMath.h>

XMAObject::XMAObject(std::string obj_file ,std::string  transformation_file){
	
	GLMmodel* pmodel = glmReadOBJ((char*) obj_file.c_str());

	name = getFilename(obj_file);
	std::cerr << "Load " << name << std::endl;

	//glmUnitize(pmodel);
	glmFacetNormals(pmodel);
	glmVertexNormals(pmodel, 90.0);
	displayList = glmList(pmodel, GLM_SMOOTH  );
	glmDelete(pmodel);

	FILE * file;
	file = std::fopen(transformation_file.c_str(),"r");

	double tmp[16] ; 
	while(std::fscanf(file, "%lf , %lf , %lf , %lf , %lf , %lf , %lf , %lf , %lf , %lf , %lf , %lf , %lf , %lf , %lf , %lf\n",
		&tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6], &tmp[7], &tmp[8], &tmp[9], &tmp[10], &tmp[11], &tmp[12], &tmp[13], &tmp[14], &tmp[15])==16 )
	{
		float* trans = new float[16];

		for(int x = 0; x < 16; x++)trans[x] = tmp[x];

		transformation.push_back(trans);
	}

	std::fclose(file);
}

std::string XMAObject::getFilename(std::string path)
{
	size_t sep = path.find_last_of("\\/");
	if (sep != std::string::npos)
		path = path.substr(sep + 1, path.size() - sep - 1);

	size_t dot = path.find_last_of(".");
	if (dot != std::string::npos)
		return path.substr(0, dot);
	
	return "";
}

XMAObject::~XMAObject(){
	glDeleteLists(displayList,1);
}

void XMAObject::render(int frame){
	glPushMatrix();
		glMultMatrixf (transformation[frame]);
		glCallList(displayList);

	glPopMatrix();
}

std::string XMAObject::getName()
{
	return name;
}

MinVR::VRMatrix4 XMAObject::getTransformation(int frame)
{
	return MinVR::VRMatrix4(transformation[frame]);
}

int XMAObject::getTransformationSize()
{
	return transformation.size();
}
