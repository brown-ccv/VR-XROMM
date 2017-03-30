#include "XMAObject.h"
#include "glm.h"
#include <iostream>
#include <fstream>
#include <math/VRMath.h>

std::istream& XMAObject::safeGetline(std::istream& is, std::string& t)
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for (;;)
	{
		int c = sb->sbumpc();
		switch (c)
		{
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (t.empty())
			{
				is.setstate(std::ios::eofbit);
			}
			return is;
		default:
			t += (char)c;
		}
	}
}


bool XMAObject::StartsWith(const std::string& text, const std::string& token)
{
	if (text.length() < token.length())
		return false;
	return (text.compare(0, token.length(), token) == 0);
}


std::istream& XMAObject::comma(std::istream& in)
{
	if ((in >> std::ws).peek() != std::char_traits<char>::to_int_type(','))
	{
		in.setstate(std::ios_base::failbit);
	}
	return in.ignore();
}

XMAObject::XMAObject(std::string obj_file, std::string  transformation_file, float scale){
	
	GLMmodel* pmodel = glmReadOBJ((char*) obj_file.c_str());

	name = getFilename(obj_file);
	std::cerr << "Load " << name << std::endl;

	//glmUnitize(pmodel);
	glmFacetNormals(pmodel);
	glmVertexNormals(pmodel, 90.0);
	glmScale(pmodel, scale);
	displayList = glmList(pmodel, GLM_SMOOTH  );
	glmDelete(pmodel);


	std::ifstream fin(transformation_file.c_str());
	std::istringstream in;
	std::string line;
	safeGetline(fin, line);
	while (!safeGetline(fin, line).eof())
	{
		in.clear();
		in.str(line);
		std::vector<double> tmp;
		if (StartsWith(line, "NaN"))
		{
			visible.push_back(false);

			float* trans = new float[16];
			for (int x = 0; x < 16; x++)
				trans[x] = 0;
			trans[0] = 1;
			trans[5] = 1;
			trans[10] = 1;
			trans[15] = 1;
			transformation.push_back(trans);
		}
		else{
			for (double value; in >> value; comma(in))
			{
				tmp.push_back(value);
			}

			if (tmp.size() == 16)
			{
				visible.push_back(true);
				float* trans = new float[16];

				for (int x = 0; x < 16; x++)trans[x] = tmp[x];
				transformation.push_back(trans);
			}
			else
			{
				visible.push_back(false);

				float* trans = new float[16];
				for (int x = 0; x < 16; x++)
					trans[x] = 0;
				trans[0] = 1;
				trans[5] = 1;
				trans[10] = 1;
				trans[15] = 1;
				transformation.push_back(trans);
			}
		}
		line.clear();
		tmp.clear();
	}
	std::cerr << transformation.size() << std::endl;
	fin.close();
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
	if (visible[frame]){
		glPushMatrix();
		glMultMatrixf(transformation[frame]);
		glCallList(displayList);

		glPopMatrix();
	}
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

bool XMAObject::isVisible(int frame)
{
	return visible[frame];
}
