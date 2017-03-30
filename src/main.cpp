
// OpenGL platform-specific headers
#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <math.h>
// MinVR header
#include <api/MinVR.h>
#include "main/VREventInternal.h"
#include "tinyxml2.h"
#include "main/VRGraphicsStateInternal.h"
#include "VRMenu.h"
#include "VRButton.h"
#include "VRMenuHandler.h"
#include "VRMultiLineTextBox.h"
#include "VRTextBox.h"
#include "VRToggle.h"
#include "VRGraph.h"

#include "XMAObject.h"
#include "glm.h"

using namespace MinVR;

#ifndef _PI
#define _PI 3.141592653
#endif

#ifdef _MSC_VER
#define slash "\\"
#else
#define slash "/"
#endif

struct particle
{
	VRPoint3 position;
	int object;
	std::vector<VRPoint3> positions;
	int color;
};

int current_color = 0;
float color_array[20][3] = { 
	{ 0, 1, 0 },
	{ 0, 0, 1 }, 
	{ 1, 1, 0 }, 
	{ 1, 0, 1 }, 
	{ 0, 1, 1 }, 
	{0.5, 1, 0},
	{0, 1, 0.5 },
	{ 0.5, 0, 1 },
	{ 0, 0.5, 1 },
	{ 1, 1, 0.5 },
	{ 1, 0.5, 1 },
	{ 0.5, 1, 1 },
	{ 0, 0.5, 0 },
	{ 0, 0, 0.5 },
	{ 0.5, 1, 0 },
	{ 1, 0.5, 0 },
	{ 0.5, 0, 1 },
	{ 1, 0, 0.5 },
	{ 0, 0.5, 1 },
	{ 0, 1, 0.5 },
};

// deg2rad * degrees = radians
#define deg2rad (3.14159265/180.0)
// rad2deg * radians = degrees
#define rad2deg (180.0/3.14159265)

bool StartsWith(const std::string& text, const std::string& token)
{
	if (text.length() < token.length())
		return false;
	return (text.compare(0, token.length(), token) == 0);
}

class MyVRApp : public VRApp, VRMenuHandler
{
public:
	MyVRApp(int argc, char** argv, const std::string& configFile) : VRApp(argc, argv, configFile), menuVisible(false), clicked(false), movement_x(0.0), movement_y(0.0), rotateObj(false), current_obj(-1), tool_dist(-0.8), hover_particle(-1), selected_particle(-1), particle_trail(-1), currentMenu(0), objscale(1.0)
	{

		initXROMM(argv[2]);

		if (argc >= 4)
		{
			objscale = std::atof(argv[3]);
		}

		light_pos[0] = 0.0;
		light_pos[1] = 4.0;
		light_pos[2] = 0.0;
		light_pos[3] = 1.0;

		points[0] = -1; 
		points[1] = -1;
		points[2] = -1;
		points[3] = -1;

		rotateObj = 0;
	}

	virtual ~MyVRApp()
	{
		std::cerr << "Delete" << std::endl;
		glDeleteLists(sphere, 1);
	}

	void initXROMM(const string& mySetup){

		frame = 0;
		speed = 1.0;
		scale = 0.0328;
		initialised = false;
		filename = mySetup + slash;
	}

	void loadData(std::string directory, float scale) {
		initialised = true;
		FILE *file;
		std::string filename = directory;
		file = std::fopen(filename.append("Data.csv").c_str(), "r"); // open a file

		char buf[256];

		while (std::fscanf(file, "%s", buf) != EOF)
		{
			// read an entire line into memory
			char *obj_file = std::strtok(buf, ",");
			char * trans_file = std::strtok(NULL, "\n");
			std::string obj_filename = directory;
			std::string trans_filename = directory;
			XMAObject * obj = new XMAObject(obj_filename.append(obj_file), trans_filename.append(trans_file),objscale);
			objects.push_back(obj);
		}

		std::fclose(file);

		GLMmodel* pmodel = glmReadOBJ("sphere.obj");

		glmUnitize(pmodel);
		glmScale(pmodel, 0.1);
		glmFacetNormals(pmodel);
		glmVertexNormals(pmodel, 90.0);
		sphere = glmList(pmodel, GLM_SMOOTH);
		glmDelete(pmodel);

		max_Frame = 1000000000;

		for (std::vector<XMAObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
		{
			max_Frame = (max_Frame > (*it)->getTransformationSize()) ? (*it)->getTransformationSize() : max_Frame;
		}

		createMenu();
	}

	void updateParticles()
	{
		for (int j = 0; j < particles.size(); j++)
		{
			particles[j].positions.clear();
			for (int i = 0; i < max_Frame; i++)
			{
				VRPoint3 p_frame = particles[j].position;
				if (particles[j].object != -1)
					p_frame = objects[particles[j].object]->getTransformation(i) * p_frame;
				if (toggle_fix_current_Object->isToggled())
				{
					p_frame = object_fixpose * objects[fixed_obj]->getTransformation(i).inverse() * p_frame;
				}
				particles[j].positions.push_back(p_frame);
			}
		}
	}

	void setDistanceData()
	{
		std::vector <double> data;		
		for (int i = 0; i < max_Frame; i++)
			data.push_back((particles[points[0]].positions[i] - particles[points[1]].positions[i]).length());
		
		graph_distance->setData(data);
	}

	void setAngleData()
	{
		if (points[0] == -1 || points[1] == -1 || points[2] == -1 || points[3] == -1)
			return;

		std::vector <double> data;
		for (int i = 0; i < max_Frame; i++)
			data.push_back(
			(particles[points[0]].positions[i] - particles[points[1]].positions[i]).dot(
			(particles[points[2]].positions[i] - particles[points[3]].positions[i]))
				);

		graph_angle->setData(data);
	}

	virtual void onVREvent(const VREvent &event) {
		//event.print();
		if(!initialised)
			return;
		if (event.getName() == "HTC_HMD_1")
		{
			headpose = event.getDataAsFloatArray("Pose");
		}
		else if (event.getName() == "Head_Move"){
			headpose = event.getDataAsFloatArray("Transform");
		}
		else if (event.getName() == "HTC_Controller_1_Axis1Button_Pressed" ||
			event.getName() == "Wand_Right_Btn_Down"){
			if (!clickMenus(true)){
				clicked = true;

				if (currentMenu == 1)
				{
					if (hover_particle != -1){
						points[0] = hover_particle;
						points[1] = -1;

						std::vector <double> data;
						for (int i = 0; i < max_Frame; i++)
							data.push_back(0);
						graph_distance->setData(data);
					}
				}
				else if (currentMenu == 2)
				{
					if (hover_particle != -1){
						if (toggle_angle_light1->isToggled()){
							points[0] = hover_particle;
							points[1] = -1;
							std::vector <double> data;
							for (int i = 0; i < max_Frame; i++)
								data.push_back(0);
							graph_angle->setData(data);
						}
						else if (toggle_angle_light2->isToggled()){
							points[2] = hover_particle;
							points[3] = -1;
							std::vector <double> data;
							for (int i = 0; i < max_Frame; i++)
								data.push_back(0);
							graph_angle->setData(data);
						}
					}
				}
				else if (!toggle_move_light->isToggled() && !toggle_add_Particle->isToggled()
					&& !toggle_move_Particle->isToggled() && !toggle_delete_Particle->isToggled() && !toggle_project_Particle->isToggled())
				{
					obj_rotation = controllerpose;
					rotateObj = true;
				}
				else if (toggle_add_Particle->isToggled())
				{
					
					particle p;
					p.object = current_obj;

					VRPoint3 p_tmp = roompose.inverse() * controllerpose * VRPoint3(0, 0, tool_dist);

					p_tmp.x = p_tmp.x / scale;
					p_tmp.y = p_tmp.y / scale;
					p_tmp.z = p_tmp.z / scale;

					if (toggle_fix_current_Object->isToggled())
					{
						p_tmp = objects[fixed_obj]->getTransformation(frame) * object_fixpose.inverse() * p_tmp;
					}
					if (current_obj != -1)
						p_tmp = objects[current_obj]->getTransformation(frame).inverse() * p_tmp;
					p.position = p_tmp;

					for (int i = 0; i < max_Frame; i++)
					{
						VRPoint3 p_frame = p.position;
						if (current_obj != -1)
							p_frame = objects[current_obj]->getTransformation(i) * p_frame;
						if (toggle_fix_current_Object->isToggled())
						{
							p_frame = object_fixpose * objects[fixed_obj]->getTransformation(i).inverse() * p_frame;
						}
						p.positions.push_back(p_frame);
					}

					p.color = current_color;
					current_color++;
					current_color = current_color % 20;
					particles.push_back(p);
					
				}
				else if (toggle_move_Particle->isToggled())
				{
					if (hover_particle != -1)
					{
						selected_particle = hover_particle;
					}
				}
				else if (toggle_delete_Particle->isToggled())
				{
					
					if (hover_particle != -1)
					{
						if (hover_particle <= points[0] || hover_particle <= points[1])
						{
							points[0] = -1;
							points[1] = -1;
						}
						if (hover_particle <= points[2] || hover_particle <= points[3])
						{
							points[2] = -1;
							points[3] = -1;
						}
						particles.erase(particles.begin() + hover_particle);
					}

				}
			}
		}
		else if (event.getName() == "HTC_Controller_1_Axis1Button_Released" ||
			event.getName() == "Wand_Right_Btn_Up"){
			clickMenus(false);
			clicked = false;
			rotateObj = false;

			if (currentMenu == 1)
			{
				if (hover_particle != -1){
					points[1] = hover_particle;
					setDistanceData();
				}
			}
			else if (currentMenu == 2){
				if (hover_particle != -1){
					if (toggle_angle_light1->isToggled()){
						points[1] = hover_particle;
					}
					else if (toggle_angle_light2->isToggled()){
						points[3] = hover_particle;
					}

					if (points[0] == -1 || points[1] == -1)
					{
						toggle_angle_light1->setToggled(true);
						toggle_angle_light2->setToggled(false);
					}
					else if (points[2] == -1 || points[3] == -1)
					{
						toggle_angle_light1->setToggled(false);
						toggle_angle_light2->setToggled(true);
					}
					else
					{
						toggle_angle_light1->setToggled(false);
						toggle_angle_light2->setToggled(false);		
					}	
					setAngleData();
				}
			}
			else if (selected_particle != -1)
			{
				VRPoint3 p_tmp = roompose.inverse() * controllerpose * VRPoint3(0, 0, tool_dist);

				p_tmp.x = p_tmp.x / scale;
				p_tmp.y = p_tmp.y / scale;
				p_tmp.z = p_tmp.z / scale;

				if (toggle_fix_current_Object->isToggled())
				{
					p_tmp = objects[fixed_obj]->getTransformation(frame) * object_fixpose.inverse() * p_tmp;
				}

				if (particles[selected_particle].object != -1)
					p_tmp = objects[particles[selected_particle].object]->getTransformation(frame).inverse() * p_tmp;

				particles[selected_particle].position = p_tmp;

				particles[selected_particle].positions.clear();
				for (int i = 0; i < max_Frame; i++)
				{
					p_tmp = particles[selected_particle].position;
					if (particles[selected_particle].object != -1)
						p_tmp = objects[particles[selected_particle].object]->getTransformation(i) * p_tmp;
					VRPoint3 p_frame = p_tmp;

					if (toggle_fix_current_Object->isToggled())
					{
						p_frame = object_fixpose * objects[fixed_obj]->getTransformation(i).inverse() * p_frame;
					}
					particles[selected_particle].positions.push_back(p_frame);
				}

				selected_particle = -1;
			}
		}
		else if (event.getName() == "HTC_Controller_2_Axis0Button_Pressed"){
			if (menuVisible){
				double val = event.getInternal()->getDataIndex()->getValue("/HTC_Controller_2/State/Axis0/XPos");
				if (val > 0){
					currentMenu++;
				}
				else
				{
					currentMenu--;
				}
				currentMenu = currentMenu % menus.size();
				displayMenu(currentMenu);
				if (currentMenu == 2)
				{
					if (points[0] == -1 || points[1] == -1)
					{
						toggle_angle_light1->setToggled(true);
						toggle_angle_light2->setToggled(false);
					}
					else if (points[2] == -1 || points[3] == -1)
					{
						toggle_angle_light1->setToggled(false);
						toggle_angle_light2->setToggled(true);
					}
					else
					{
						toggle_angle_light1->setToggled(false);
						toggle_angle_light2->setToggled(false);
					}
				}
			}
		}
		else if (event.getName() == "B11_Down"){
			if (menuVisible){			
				currentMenu--;
				
				currentMenu = currentMenu % menus.size();
				displayMenu(currentMenu);
				if (currentMenu == 2)
				{
					if (points[0] == -1 || points[1] == -1)
					{
						toggle_angle_light1->setToggled(true);
						toggle_angle_light2->setToggled(false);
					}
					else if (points[2] == -1 || points[3] == -1)
					{
						toggle_angle_light1->setToggled(false);
						toggle_angle_light2->setToggled(true);
					}
					else
					{
						toggle_angle_light1->setToggled(false);
						toggle_angle_light2->setToggled(false);
					}
				}
			}
		}
		else if (event.getName() == "B12_Down"){
			if (menuVisible){			
				currentMenu++;
				
				currentMenu = currentMenu % menus.size();
				displayMenu(currentMenu);
				if (currentMenu == 2)
				{
					if (points[0] == -1 || points[1] == -1)
					{
						toggle_angle_light1->setToggled(true);
						toggle_angle_light2->setToggled(false);
					}
					else if (points[2] == -1 || points[3] == -1)
					{
						toggle_angle_light1->setToggled(false);
						toggle_angle_light2->setToggled(true);
					}
					else
					{
						toggle_angle_light1->setToggled(false);
						toggle_angle_light2->setToggled(false);
					}
				}
			}
		}
		else if (event.getName() == "HTC_Controller_2_Axis1Button_Pressed" ||
			event.getName() == "B13_Down")
{
			displayMenu(currentMenu);
		}
		else if (event.getName() == "HTC_Controller_2_Axis1Button_Released" ||
			event.getName() == "B13_Up"){
			displayMenu(-1);
		}
		else if (event.getName() == "Wand1_Move"){
			menupose = event.getDataAsFloatArray("Transform");
			menupose = menupose * VRMatrix4::rotationY(deg2rad * -180)  * VRMatrix4::translation(VRVector3(0, 0.6,0));		
			updateMenus();
		}
		else if (event.getName() == "HTC_Controller_2")
		{
			if (event.getInternal()->getDataIndex()->exists("/HTC_Controller_2/Pose")){
				menupose = event.getDataAsFloatArray("Pose");
				menupose = menupose * VRMatrix4::rotationX(deg2rad * -90) * VRMatrix4::translation(VRVector3(0, -0.2, 0));
				updateMenus();
			}
		}
		else if (event.getName() == "Wand0_Move"){
			controllerpose = event.getDataAsFloatArray("Transform");
			updateMenus();
		}
		else if(event.getName() == "Wand_Joystick_Y_Change") {
			movement_y = event.getDataAsFloat("AnalogValue");
		}
		else if(event.getName() == "Wand_Joystick_X_Change") {
			movement_x = event.getDataAsFloat("AnalogValue");
		}
		else if (event.getName() == "HTC_Controller_1")
		{
			if (event.getInternal()->getDataIndex()->exists("/HTC_Controller_1/Pose")){
				controllerpose = event.getDataAsFloatArray("Pose");
				updateMenus();
			}
			if (event.getInternal()->getDataIndex()->exists("/HTC_Controller_1/State/Axis0Button_Pressed") &&
				(int)event.getInternal()->getDataIndex()->getValue("/HTC_Controller_1/State/Axis0Button_Pressed")){
				movement_x = event.getInternal()->getDataIndex()->getValue("/HTC_Controller_1/State/Axis0/XPos");
				movement_y = event.getInternal()->getDataIndex()->getValue("/HTC_Controller_1/State/Axis0/YPos");
			}
			else
			{
				movement_y = 0;
				movement_x = 0;
			}
		}

		if (event.getName() == "KbdEsc_Down") {
			//shutdown();
			return;
		}
	}

	virtual void onVRRenderGraphicsContext(const VRGraphicsState& state)
	{
		if (!initialised) {
			loadData(filename, objscale);
			glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, toggle_display_transparent->isToggled());
		}

		graph_distance->setCurrent(frame);
		graph_angle->setCurrent(frame);

		if (toggle_play->isToggled()){
			frame += speed;
			if (frame >= max_Frame)frame = frame - max_Frame;
			textbox_current_frame->setText("Frame: " + std::to_string((long long)frame + 1));
			
		}

		for (std::vector<VRMenu*>::const_iterator it = menus.begin(); it != menus.end(); ++it){
			(*it)->updateIteration();
		}

		if (clicked)
		{
			if (toggle_move_light->isToggled())
			{
				toggle_fix_light_to_head->setToggled(false);
				VRPoint3 pos = controllerpose * VRPoint3(0, 0, tool_dist);
				light_pos[0] = pos.x;
				light_pos[1] = pos.y;;
				light_pos[2] = pos.z;
				glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
			}
		}

		if (toggle_fix_light_to_head->isToggled())
		{
			light_pos[0] = headpose.getColumn(3)[0];
			light_pos[1] = headpose.getColumn(3)[1];
			light_pos[2] = headpose.getColumn(3)[2];
			glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		}

		if (fabs(movement_x) > 0.0 || fabs(movement_y)) {
			if (toggle_add_Particle->isToggled() || toggle_move_Particle->isToggled() ||
				toggle_delete_Particle->isToggled() || toggle_move_light->isToggled()){
				tool_dist -= 0.01 * movement_y;
			}
			else{
				VRVector3 trans1 = -movement_x * scale * 0.25 * controllerpose * VRVector3(1, 0, 0);
				VRVector3 trans2 = movement_y * scale * 0.25 * controllerpose * VRVector3(0, 0, 1);
				roompose = VRMatrix4::translation(trans1 + trans2) * roompose;
			}
		}

		if (rotateObj){
			roompose = controllerpose * obj_rotation.inverse() * roompose;
			obj_rotation = controllerpose;
		}

		if (toggle_delete_Particle->isToggled() || toggle_move_Particle->isToggled() || currentMenu == 1 || currentMenu == 2)
		{
			VRPoint3 pos = controllerpose * VRPoint3(0, 0, tool_dist);
			hover_particle = -1;
			double d = 0.15;
			for (int i = 0; i < particles.size(); i++)
			{
				VRPoint3 p_particle = particles[i].position;
				if (particles[i].object != -1)
					p_particle = objects[particles[i].object]->getTransformation(frame) * p_particle;

				if (toggle_fix_current_Object->isToggled())
				{
					p_particle = object_fixpose * objects[fixed_obj]->getTransformation(frame).inverse() * p_particle;
				}
				p_particle.x = p_particle.x * scale;
				p_particle.y = p_particle.y * scale;
				p_particle.z = p_particle.z * scale;
				p_particle = roompose.getArray() * p_particle;

				if ((p_particle - pos).length() < d)
				{
					d = (p_particle - pos).length();
					hover_particle = i;
				}

			}
		}
	}

	virtual void onVRRenderGraphics(const VRGraphicsState &state) {
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_NORMALIZE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_COLOR_MATERIAL);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(state.getProjectionMatrix());

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(state.getViewMatrix());

		if (toggle_display_transparent->isToggled()){
			//transparent
			glDepthFunc(GL_ALWAYS); // The Type Of Depth Testing To Do
			glEnable(GL_BLEND);
			glBlendFunc(GL_ZERO, GL_SRC_COLOR);
			glDisable(GL_CULL_FACE);
		}
		//Do Rendereing
		glPushMatrix();
		glMultMatrixf(roompose.getArray());
		glScaled(scale, scale, scale);
		if (toggle_fix_current_Object->isToggled())
		{
			glMultMatrixf(object_fixpose.getArray());
			glMultMatrixf(objects[fixed_obj]->getTransformation(frame).inverse().getArray());
		}
		for (int i = 0; i < objects.size(); i++)
		{
			if (i == current_obj) {
				glColor3f(1.0, 1.0, 0.0);
			}
			else
			{
				glColor3f(1.0, 1.0, 1.0);
			}
			objects[i]->render((int)frame);
		}
		glPopMatrix();

		
		if (toggle_display_transparent->isToggled()){
			//transparent
			glDisable(GL_BLEND);
			glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
		}

		//particles
		//Do Rendereing
		glPushMatrix();
		glMultMatrixf(roompose.getArray());
		glScaled(scale, scale, scale);
		if (toggle_fix_current_Object->isToggled())
		{
			glMultMatrixf(object_fixpose.getArray());
			glMultMatrixf(objects[fixed_obj]->getTransformation(frame).inverse().getArray());
		}
		for (int i = 0; i < particles.size(); i++)
		{
			if ((i == hover_particle && selected_particle == -1) || 
				selected_particle == i) {
				glColor3f(1.0, 0.0, 0.0);
			}
			else
			{
				glColor3f(color_array[particles[i].color][0], color_array[particles[i].color][1], color_array[particles[i].color][2]);
			}

			glPushMatrix();
			if (particles[i].object != -1)
				glMultMatrixf(objects[particles[i].object]->getTransformation(frame).getArray());
			glTranslatef(particles[i].position.x, particles[i].position.y, particles[i].position.z);
			glScaled(1.0 / scale, 1.0 / scale, 1.0 / scale);
			glCallList(sphere);
			glPopMatrix();
		}
		glPopMatrix();

		if (!toggle_project_Particle->isToggled() || !clicked){
			glPushMatrix();
			glMultMatrixf(controllerpose.getArray());
			glBegin(GL_LINES);                // Begin drawing the color cube with 6 quads
			// Back face (z = -1.0f)
			glColor3f(0.1f, 0.1f, 0.0f);     // Yellow
			glVertex3f(0.0f, 0.0f, -5.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glEnd();  // End of drawing color-cube
			if (toggle_add_Particle->isToggled() || toggle_move_Particle->isToggled() ||
				toggle_delete_Particle->isToggled() || toggle_move_light->isToggled() || currentMenu == 1 || currentMenu == 2){
				glTranslatef(0, 0, tool_dist);
				glScalef(1.0 / scale * 0.03, 1.0 / scale * 0.03, 1.0 / scale * 0.03);
				glCallList(sphere);
			}
			glPopMatrix();
		} 

		if (toggle_project_Particle->isToggled() && clicked)
		{
			VRVector3 controller_n = controllerpose * VRVector3(0, 1, 0);

			for (int i = 0; i < particles.size(); i++)
			{
				if ((i == hover_particle && selected_particle == -1) ||
					selected_particle == i) {
					glColor3f(1.0, 0.0, 0.0);
				}
				else
				{
					glColor3f(color_array[particles[i].color][0], color_array[particles[i].color][1], color_array[particles[i].color][2]);
				}
				 
				VRPoint3 p_particle = particles[i].positions[frame];
				p_particle.x = p_particle.x * scale;
				p_particle.y = p_particle.y * scale;
				p_particle.z = p_particle.z * scale;
				p_particle = roompose * p_particle;
				VRPoint3 p_particle2 = controllerpose.inverse()* p_particle;
				double d = -p_particle2.y;
				glPushMatrix();
				glTranslatef(p_particle.x + d * controller_n.x, p_particle.y + d * controller_n.y, p_particle.z + d * controller_n.z);
				glScalef(1.0 / scale * 0.01, 1.0 / scale * 0.01, 1.0 / scale * 0.01);
				glCallList(sphere);
				glPopMatrix();
			}
		}

		glDisable(GL_LIGHTING);

		if (points[0] != -1)
			{
				VRPoint3 pt1 = particles[points[0]].positions[frame];
				pt1.x = pt1.x *scale;
				pt1.y = pt1.y *scale;
				pt1.z = pt1.z *scale;
				pt1 = roompose * pt1;

				VRPoint3 pt2;

				if (points[1] != -1)
				{
					pt2 = particles[points[1]].positions[frame];
					pt2.x = pt2.x *scale;
					pt2.y = pt2.y *scale;
					pt2.z = pt2.z *scale;
					pt2 = roompose * pt2;
				}
				else
				{
					pt2 = controllerpose * VRPoint3(0, 0, tool_dist);
				}
				glPushMatrix();
				glBegin(GL_LINES);                // Begin drawing the color cube with 6 quads
				// Back face (z = -1.0f)
				glColor3f(1.0f, 0.0f, 0.0f);     // Yellow
				glVertex3f(pt1.x, pt1.y, pt1.z);
				glVertex3f(pt2.x, pt2.y, pt2.z);
				glEnd();  // End of drawing color-cube
				glPopMatrix();
		}

		if (points[2] != -1)
		{
			VRPoint3 pt1 = particles[points[2]].positions[frame];
			pt1.x = pt1.x *scale;
			pt1.y = pt1.y *scale;
			pt1.z = pt1.z *scale;
			pt1 = roompose * pt1;

			VRPoint3 pt2;

			if (points[3] != -1)
			{
				pt2 = particles[points[3]].positions[frame];
				pt2.x = pt2.x *scale;
				pt2.y = pt2.y *scale;
				pt2.z = pt2.z *scale;
				pt2 = roompose * pt2;
			}
			else
			{
				pt2 = controllerpose * VRPoint3(0, 0, tool_dist);
			}
			glPushMatrix();
			glBegin(GL_LINES);                // Begin drawing the color cube with 6 quads
			// Back face (z = -1.0f)
			glColor3f(0.0f, 0.0f, 1.0f);     // Yellow
			glVertex3f(pt1.x, pt1.y, pt1.z);
			glVertex3f(pt2.x, pt2.y, pt2.z);
			glEnd();  // End of drawing color-cube
			glPopMatrix();
		}

		if (toggle_project_Particle->isToggled() && clicked)
		{
			double min_x = 1000000000;
			double min_z = 1000000000;
			double max_x = -1000000000;
			double max_z = -1000000000;
			VRVector3 controller_n = controllerpose * VRVector3(0, 1, 0);

			for (int i = 0; i < particles.size(); i++)
			{
				if ((i == hover_particle && selected_particle == -1) ||
					selected_particle == i) {
					glColor3f(1.0, 0.0, 0.0);
				}
				else
				{
					glColor3f(color_array[particles[i].color][0], color_array[particles[i].color][1], color_array[particles[i].color][2]);
				}

				glBegin(GL_LINE_STRIP);
				int start = 0;
				int end = particles[i].positions.size();

				if (particle_trail != -1)
				{
					start = frame - particle_trail;
					if (start < 0) start = 0;
					end = frame;
				}

				for (int j = start; j < end; j++)
				{
					VRPoint3 p_particle = particles[i].positions[j];
					p_particle.x = p_particle.x * scale;
					p_particle.y = p_particle.y * scale;
					p_particle.z = p_particle.z * scale;
					p_particle = roompose * p_particle;
					VRPoint3 p_particle2 = controllerpose.inverse()* p_particle;
					double d = -p_particle2.y;

					if (p_particle2.x < min_x) min_x = p_particle2.x;
					if (p_particle2.x > max_x) max_x = p_particle2.x;
					if (p_particle2.z < min_z) min_z = p_particle2.z;
					if (p_particle2.z > max_z) max_z = p_particle2.z;
					glVertex3f(p_particle.x + d * controller_n.x, p_particle.y + d * controller_n.y, p_particle.z + d * controller_n.z);
				}
				glEnd();
			}

			glPushMatrix();
			glMultMatrixf(controllerpose.getArray());
			glBegin(GL_LINE_STRIP);                
			glColor3f(0.5f, 0.5f, 0.0f);     // Yellow
			glVertex3f(min_x - 0.1, 0.0f, max_z + 0.1);
			glVertex3f(max_x + 0.1, 0.0f, max_z + 0.1);
			glVertex3f(max_x + 0.1, 0.0f, min_z - 0.1);
			glVertex3f(min_x - 0.1, 0.0f, min_z - 0.1);
			glVertex3f(min_x - 0.1, 0.0f, max_z + 0.1);
			glEnd();  // End of drawing color-cube
			glPopMatrix();
		}
		
		glPushMatrix();
		glMultMatrixf(roompose.getArray());
		glScaled(scale, scale, scale);
		for (int i = 0; i < particles.size(); i++)
		{
			if ((i == hover_particle && selected_particle == -1) ||
				selected_particle == i) {
				glColor3f(1.0, 0.0, 0.0);
			}
			else
			{
				glColor3f(color_array[particles[i].color][0], color_array[particles[i].color][1], color_array[particles[i].color][2]);
			}

			glBegin(GL_LINE_STRIP);
			int start = 0;
			int end = particles[i].positions.size();

			if (particle_trail != -1)
			{
				start = frame - particle_trail;
				if (start < 0) start = 0;
				end = frame;
			}

			for (int j = start; j < end; j++)
			{
				glVertex3f(particles[i].positions[j].x, particles[i].positions[j].y, particles[i].positions[j].z);
			}
			glEnd();
		}
		glPopMatrix();

		drawMenus();
	}


	virtual void handleEvent(VRMenuElement * element)
	{
		if (element == toggle_play)
		{
			
		}
		else if (element == graph_distance)
		{		
			frame = graph_distance->getSelection();	
		}
		else if (element == graph_angle)
		{			
			frame = graph_angle->getSelection();		
		}
		else if (element == toggle_display_transparent)
		{
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, toggle_display_transparent->isToggled());
		}
		else if (element == toggle_fix_light_to_head)
		{
			if (toggle_fix_light_to_head->isToggled()){
				toggle_move_light->setToggled(false);
			}
		}
		else if (element == toggle_move_light)
		{
			if (toggle_move_light->isToggled()){
				toggle_add_Particle->setToggled(false);
				toggle_move_Particle->setToggled(false);
				toggle_delete_Particle->setToggled(false);
				toggle_project_Particle->setToggled(false);
			}
		}
		else if (element == toggle_move_Particle)
		{
			if (toggle_move_Particle->isToggled()){
				toggle_move_light->setToggled(false);
				toggle_add_Particle->setToggled(false);
				toggle_delete_Particle->setToggled(false);
				toggle_play->setToggled(false);
				toggle_project_Particle->setToggled(false);
			}
		}
		else if (element == toggle_delete_Particle)
		{
			if (toggle_delete_Particle->isToggled()){
				toggle_move_light->setToggled(false);
				toggle_add_Particle->setToggled(false);
				toggle_move_Particle->setToggled(false);
				toggle_play->setToggled(false);
				toggle_project_Particle->setToggled(false);
			}
		}
		else if (element == toggle_add_Particle)
		{
			if (toggle_add_Particle->isToggled()){
				toggle_move_light->setToggled(false);
				toggle_move_Particle->setToggled(false);
				toggle_delete_Particle->setToggled(false);
				toggle_play->setToggled(false);
				toggle_project_Particle->setToggled(false);
			}
		}
		else if (element == toggle_project_Particle)
		{
			if (toggle_project_Particle->isToggled()){
				toggle_move_light->setToggled(false);
				toggle_move_Particle->setToggled(false);
				toggle_delete_Particle->setToggled(false);
				toggle_play->setToggled(false);
				toggle_add_Particle->setToggled(false);
			}
		}
		else if (element == toggle_fix_current_Object)
		{
			if (toggle_fix_current_Object->isToggled()){
				if (current_obj < 0)
				{
					toggle_fix_current_Object->setToggled(false);
				}
				else{
					object_fixpose = objects[current_obj]->getTransformation(frame);
					fixed_obj = current_obj;
				}
			}
			else
			{
				object_fixpose = VRMatrix4();
			}
			updateParticles();
		}
		else if (element == button_increase_speed)
		{
			speed += 0.1;
			textbox_current_speed->setText("Speed: " + std::to_string((long double)speed));
		}
		else if (element == button_decrease_speed)
		{
			speed -= 0.1;
			if (speed < 0) speed = 0.0;
			textbox_current_speed->setText("Speed: " + std::to_string((long double)speed));

		}
		else if (element == button_next_frame)
		{
			frame++;
			textbox_current_frame->setText("Frame: " + std::to_string((long long)frame + 1));
		}
		else if (element == button_prev_frame)
		{
			frame--;
			if (frame < 0) frame = 0;
			textbox_current_frame->setText("Frame: " + std::to_string((long long)frame + 1));
		}
		else if (element == button_increase_trail)
		{
			particle_trail++;
			if (particle_trail >= max_Frame) particle_trail = max_Frame - 1;
			textbox_current_trail->setText("Particle trail: " + std::to_string((long long)particle_trail));
		}
		else if (element == button_decrease_trail)
		{
			particle_trail--;
			if (particle_trail < -1) particle_trail = -1;
			textbox_current_trail->setText("Particle trail: " + std::to_string((long long)particle_trail));
		}
		else if (element == button_next_object)
		{
			current_obj++;
			if (current_obj >= objects.size()) current_obj = -1;
			if (current_obj == -1){
				textbox_current_object->setText("Object: ");
			}
			else
			{
				textbox_current_object->setText("Object: " + objects[current_obj]->getName());
			}

		}
		else if (element == button_prev_object)
		{
			current_obj--;
			if (current_obj < -1) current_obj = objects.size() - 1;
			if (current_obj == -1){
				textbox_current_object->setText("Object: ");
			}
			else
			{
				textbox_current_object->setText("Object: " + objects[current_obj]->getName());
			}
		}
		else if (element == button_increase_scale)
		{
			scale += 0.01;
			textbox_current_scale->setText("Scale: " + std::to_string((long double)scale));
		}
		else if (element == button_decrease_scale)
		{
			scale -= 0.01;
			if (scale < 0) scale = 0.0;
			textbox_current_scale->setText("Scale: " + std::to_string((long double)scale));
		}
	}

	bool clickMenus(bool isDown)
	{
		bool hit = false;
		for (std::vector<VRMenu*>::const_iterator it = menus.begin(); it != menus.end(); ++it)
			hit = hit || (*it)->click(isDown);
		
		return hit;
	}

	void displayMenu(int menuID)
	{
		if (menuID == -1){
			menuVisible = false;
		}
		else
		{
			menuVisible = true;
		}
		for (int i = 0; i < menus.size(); i++){
			if (i != menuID)
			{
				menus[i]->setVisible(false);
			}
			else
			{
				menus[i]->setVisible(true);
			}
		}
	}

	void createMenu()
	{
		VRMenu *menu1 = new VRMenu(1.0, 1.0, 8, 8, filename);
		toggle_play = new VRToggle("button_play", "Play");
		toggle_play->setToggled(true);
		menu1->addElement(toggle_play, 1, 1, 8, 1);

		button_decrease_speed = new VRButton("button_decrease_speed", "-", true);
		menu1->addElement(button_decrease_speed, 1, 2, 1, 1);
		textbox_current_speed = new VRTextBox("textbox_current_speed", "Speed: " + std::to_string((long double)speed));
		menu1->addElement(textbox_current_speed, 2, 2, 6, 1);
		button_increase_speed = new VRButton("button_increase_speed", "+", true);
		menu1->addElement(button_increase_speed, 8, 2, 1, 1);

		button_prev_frame = new VRButton("button_prev_frame", "-", true);
		menu1->addElement(button_prev_frame, 1, 3, 1, 1);
		textbox_current_frame = new VRTextBox("textbox_current_frame", "Frame: " + std::to_string((long long)frame + 1));
		menu1->addElement(textbox_current_frame, 2, 3, 6, 1);
		button_next_frame = new VRButton("button_next_frame", "+", true);
		menu1->addElement(button_next_frame, 8, 3, 1, 1);

		button_prev_object = new VRButton("button_prev_object", "-");
		menu1->addElement(button_prev_object, 1, 4, 1, 1);
		textbox_current_object = new VRTextBox("textbox_current_object", "Object: ");
		menu1->addElement(textbox_current_object, 2, 4, 6, 1);
		button_next_object = new VRButton("button_next_object", "+");
		menu1->addElement(button_next_object, 8, 4, 1, 1);

		button_decrease_scale = new VRButton("button_decrease_scale", "-", true);
		menu1->addElement(button_decrease_scale, 1, 5, 1, 1);
		textbox_current_scale = new VRTextBox("textbox_current_scale", "Scale: " + std::to_string((long double)scale));
		menu1->addElement(textbox_current_scale, 2, 5, 6, 1);
		button_increase_scale = new VRButton("button_increase_scale", "+", true);
		menu1->addElement(button_increase_scale, 8, 5, 1, 1);

		button_decrease_trail = new VRButton("button_decrease_trail", "-", true);
		menu1->addElement(button_decrease_trail, 1, 6, 1, 1);
		textbox_current_trail = new VRTextBox("textbox_current_trail", "Particle trail: " + std::to_string((long long) particle_trail));
		menu1->addElement(textbox_current_trail, 2, 6, 6, 1);
		button_increase_trail = new VRButton("button_increase_trail", "+", true);
		menu1->addElement(button_increase_trail, 8, 6, 1, 1);


		toggle_move_light = new VRToggle("button_set_light", "Move Light");
		menu1->addElement(toggle_move_light, 1, 7, 2, 1);
		toggle_add_Particle = new VRToggle("button_add_Particle", "Add Particle");
		menu1->addElement(toggle_add_Particle, 3, 7, 2, 1);
		toggle_move_Particle = new VRToggle("toggle_move_Particle", "Move Particle");
		menu1->addElement(toggle_move_Particle, 5, 7, 2, 1);
		toggle_delete_Particle = new VRToggle("toggle_delete_Particle", "Delete Particle");
		menu1->addElement(toggle_delete_Particle, 7, 7, 2, 1);

		toggle_fix_light_to_head = new VRToggle("toggle_fix_light_to_head", "Headlight");
		menu1->addElement(toggle_fix_light_to_head, 1, 8, 2, 1);
		toggle_fix_light_to_head->setToggled(true);
		toggle_fix_current_Object = new VRToggle("toggle_fix_current_Object", "Fix Object");
		menu1->addElement(toggle_fix_current_Object, 3, 8, 2, 1);
		toggle_display_transparent = new VRToggle("toggle_display_transparent", "Transparent");
		menu1->addElement(toggle_display_transparent, 5, 8, 2, 1);
		toggle_project_Particle = new VRToggle("toggle_project_Particle", "Project Particle");
		menu1->addElement(toggle_project_Particle, 7, 8, 2, 1);

		menu1->addMenuHandler(this);

		menus.push_back(menu1);

		VRMenu *menu2 = new VRMenu(1.0,1.0, 8, 8, "Distance");
		std::vector <double> data;
		for (int i = 0; i < max_Frame; i++)
			data.push_back(0);
		graph_distance = new VRGraph("graph_distance", data);
		menu2->addElement(graph_distance, 1, 1, 8, 8);

		menu2->addMenuHandler(this);

		menus.push_back(menu2);

		VRMenu *menu3 = new VRMenu(1.0, 1.0, 8, 8, "Angle");
		toggle_angle_light1 = new VRToggle("toggle_angle_light1", "Set Line 1");
		menu3->addElement(toggle_angle_light1, 1, 1, 8, 1);
		toggle_angle_light2 = new VRToggle("toggle_angle_light2", "Set Line 2");
		menu3->addElement(toggle_angle_light2, 1, 2, 8, 1);

		graph_angle = new VRGraph("graph_angle", data);
		menu3->addElement(graph_angle, 1, 3, 8, 6);

		menu3->addMenuHandler(this);

		menus.push_back(menu3);
		displayMenu(-1);
	}

	void drawMenus()
	{
		for (std::vector<VRMenu*>::const_iterator it = menus.begin(); it != menus.end(); ++it)
			(*it)->draw();
	}

	void updateMenus()
	{
		for (std::vector<VRMenu*>::const_iterator it = menus.begin(); it != menus.end(); ++it)
			(*it)->setTransformation(menupose);


		VRPoint3 pos = controllerpose * VRPoint3(0, 0, 0);
		VRVector3 dir = controllerpose * VRVector3(0, 0, -1);

		double distance;
		for (std::vector<VRMenu*>::const_iterator it = menus.begin(); it != menus.end(); ++it){
			(*it)->intersect(pos, dir, distance);
		}
	}

protected:
	int current_obj;
	std::vector<XMAObject* > objects;
	std::vector<particle> particles;
	int points[4];
	bool initialised;
	string filename;
	int hover_particle;
	int selected_particle;
	GLfloat light_pos[4];
	double objscale;

	int max_Frame;
	double frame;
	double speed;
	double scale;
	bool clicked;

	unsigned int sphere;
	double tool_dist;

	VRMatrix4 obj_rotation;
	bool rotateObj;

	VRMatrix4 object_fixpose;
	int fixed_obj;

	VRMatrix4 menupose;
	VRMatrix4 controllerpose;
	VRMatrix4 roompose;
	VRMatrix4 headpose;
	double movement_x;
	double movement_y;


	bool menuVisible;
	std::vector<VRMenu*>		menus;
	int currentMenu;

	VRGraph* graph_distance;
	
	VRToggle*	toggle_angle_light1;
	VRToggle*	toggle_angle_light2;
	VRGraph* graph_angle;

	VRToggle*	toggle_move_light;
	VRToggle*	toggle_add_Particle;
	VRToggle*	toggle_move_Particle;
	VRToggle*	toggle_delete_Particle;

	VRToggle*	toggle_fix_light_to_head;
	VRToggle*	toggle_fix_current_Object;
	VRToggle*	toggle_display_transparent;
	VRToggle*	toggle_project_Particle;

	VRToggle*	toggle_play;

	VRTextBox*	textbox_current_speed;
	VRButton*	button_increase_speed;
	VRButton*	button_decrease_speed;

	VRTextBox*	textbox_current_frame;
	VRButton*	button_next_frame;
	VRButton*	button_prev_frame;

	VRTextBox*	textbox_current_object;
	VRButton*	button_next_object;
	VRButton*	button_prev_object;

	VRTextBox*	textbox_current_scale;
	VRButton*	button_increase_scale;
	VRButton*	button_decrease_scale;

	int particle_trail;
	VRTextBox*	textbox_current_trail;
	VRButton*	button_increase_trail;
	VRButton*	button_decrease_trail;
};


int main(int argc, char **argv)
{
	MyVRApp app(argc, argv, argv[1]);
	// This starts the rendering/input processing loop
	app.run();

	return 0;
}


//
////////////////////  end  common/vrg3d/demo/vrg3d_demo.cpp  ///////////////////
