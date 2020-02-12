
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>
# include "sn_mynode.h"
# include "csg_prim.h"
# include <fstream> 
# include <sigogl/ws_run.h>
# include <sigogl/gl_tools.h>
# include <iostream> 
# include <cstdlib> 
# include <cstdio> 
# include <cmath> 
# include <cassert> 
# include <iostream>
# include <string>

#define PI 3.141592653589793

void dbg_csgprim() {
	CSGPrimitive* prim = (CSGPrimitive*)new GsPrimitive();
	prim->sphere(5); //make sphere prim with radius = 5
}

SnModel* add_csgprim() {

	//create a new empty GsModel
	GsModel* G = new GsModel();

	//create a CSGPrimitive with ra (radius) = 5; centered at (0,0,0).
	CSGPrimitive prim; prim.ra = 5; prim.center = GsVec(0, 0, 0);

	//make the GsModel represent the CSGPrimitive
	G->make_primitive(prim);

	//make the SnModel represent the GsModel
	SnModel* M = new SnModel(G);
	return M;
}

MyViewer::MyViewer(int x, int y, int w, int h, const char* l) : WsViewer(x, y, w, h, l)
{
	add_ui();
	//add_mynode ();
	build_scene();
	//trace(WsViewer::w, WsViewer::h, 2);
	camera().eye = GsVec(0, 0, 1);
	camera().center = GsVec(0, 0, 0);

	render_pixel(640, 480, 2);	//640x480 res and 2 bounces max
}

void MyViewer::add_ui()
{
	UiPanel* p;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel("", UiPanel::HorizLeft);
	//p->add ( new UiButton ( "Add", EvAdd ) );
	p->add(new UiButton("OpenGL", EvInfo));
	p->add(new UiButton("Exit", EvExit));
}

void MyViewer::add_mynode(int n)
{
	SnMyNode* c;

	float r = 0.15f; // position range
	while (n-- > 0)
	{
		c = new SnMyNode;
		if (mcolorbut->value())
			c->multicolor = true;
		else
			c->color(GsColor::random());

		c->init.set(gs_random(-r, r), gs_random(-r, r), gs_random(-r, r));
		c->width = gs_random(0.001f, r);
		c->height = gs_random(0.001f, r * 2);

		// Example how to print/debug your generated data:
		// gsout<<n<<": "<<c->color()<<gsnl;

		rootg()->add(c);
	}
}

void MyViewer::add_model(SnGroup* parentg, SnShape* s, GsVec p)
{
	SnGroup* g = new SnGroup; // group will have: transf, shape, lines
	SnTransform* t = new SnTransform;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(t);
	g->add(s);
	g->add(l);
	g->separator(true);
	parentg->add(g);
}

GsVec get_rgb(GsMaterial mat) {
	float r, g, b;
	r = mat.ambient.r;
	g = mat.ambient.g;
	b = mat.ambient.b;
	return GsVec(r, g, b);
}

GsVec getnormal(GsVec orig, GsVec dir, GsPnt incident, GsPnt center) {
	GsVec ray = dir - orig;
	ray.normalize();

	GsVec normal = incident - center;
	normal.normalize();
	return normal;
}

float mix(const float& a, const float& b, const float& mix)
{
	return b * mix + a * (1 - mix);
}

GsVec MyViewer::trace(GsVec pixeleyeray, GsVec pixeleyeraydirection, GsArray<CSGPrimitive> Shapes, int depth) {
	GsVec color = GsVec(0, 0, 0);	//RGB
	//float illuminationequation;
	bool intersection = false;
	float alpha = 1.0;

	bool illumination = true; //Is a point in shadow?

	GsVec ray;
	GsVec direction;
	GsVec incidentpoint;
	GsVec incidentpointdirection;
	GsVec shapenormal;
	GsVec incidentpoint2;

	ray = pixeleyeray;
	direction = pixeleyeraydirection;

	GsArray<GsVec> LightSources;
	LightSources.push(lightPos);

	float minimumdistance = 99999;
	int k = 0;  //shape index   

	for (int i = 0; i < Shapes.size(); i++) { //Primary ray intersection
		if (Shapes[i].intersects(ray, direction, incidentpoint, incidentpoint2)) {
			if (minimumdistance > dist(ray, incidentpoint)) {
				minimumdistance = dist(ray, incidentpoint);
				k = i;
				intersection = true;
			}
		}
	}

	shapenormal = getnormal(ray, direction, incidentpoint, Shapes[k].center);

	if (intersection) {
		ray = incidentpoint;
		getnormal(ray, direction, incidentpoint, Shapes[k].center);
		intersection = false;
		minimumdistance = 9999;
		GsVec incidentLight = GsVec(0, 0, 0);

		for (int i = 0; i < LightSources.size(); i++) { // Shadow Feelers - not working for multiple light sources
			direction = LightSources[i] - ray;
			direction.normalize();
			for (int j = 0; j < Shapes.size(); j++) {
				if (j == k) {
					continue;
				}
				if ((Shapes[j].intersects(ray, direction, incidentLight, GsVec(0, 0, 0)) == 1) || (Shapes[j].intersects(ray, direction, incidentLight, GsVec(0, 0, 0)) == 2)) { //Last GsVec is used by intersects function to be modified. Passing empty/default GsVec.
					intersection = true;
				}
			}

			if (!intersection) {
				color += alpha * get_rgb(Shapes[k].material); //adds ambient color of intersected object. Alpha is a fraction [0, 1] that grabs a part of the color
			}

			//gsout << pixeleyeraydirection << gsnl;
			//gsout << shapenormal << gsnl;
			
			float facingratio = dot(-pixeleyeraydirection, shapenormal);
			// change the mix value to tweak the effect
			float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
			// compute reflection direction (not need to normalize because all vectors
			// are already normalized)
			GsVec refldir = pixeleyeraydirection - shapenormal * 2 * facingratio * -1;
			refldir.normalize();
			GsVec reflection = trace(incidentpoint + shapenormal * .0001f, refldir, Shapes, depth + 1);
			
			gsout << facingratio << gsnl;
			//gsout << fresneleffect << gsnl;

			color = /*reflection*/ facingratio * get_rgb(Shapes[k].material);

			////Reflection Section
			////calculate reflection ray
			//if (depth != 0) {
			//	GsVec reflectiondirection;

			//	reflectiondirection = direction - shapenormal * 2 * dot(direction, shapenormal);
			//	reflectiondirection.normalize();
			//	color += trace(ray + shapenormal, reflectiondirection, Shapes, --depth);  //compute ray point
			//}

		}
	}

	return color;
}

GsVec get_rgbColor() {
	float r, g, b;
	r = GsColor::random().r;
	g = GsColor::random().g;
	b = GsColor::random().b;
	return GsVec(r, g, b);
}

void MyViewer::render_pixel(int width, int height, int bounces) {

	GsVec* image = new GsVec[width * height];
	float invWidth = 1 / float(width), invHeight = 1 / float(height);
	float fov = 30, aspectratio = width / float(height);
	float angle = tan(PI * 0.5 * fov / 180);
	
	camera().eye = GsVec(0, 0, 100);

	GsVec evalPixel;
	pixels = new GsVec[width * height];
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			float xx = (2 * ((j + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((i + 0.5) * invHeight)) * angle;
			evalPixel.x = xx;
			evalPixel.y = yy;
			GsVec rayDir(xx,yy,-1);
			rayDir.normalize();
			pixels[(i * width) + j] = trace(GsVec(0,0,0), rayDir, prims, bounces);
		}
	}


	std::ofstream ofs("./untitled2.ppm", std::ios::out | std::ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (int i = 0; i < (width * height); i++) {
		ofs << (unsigned char)(std::fmin(float(1), pixels[i].x) * 255) <<
			(unsigned char)(std::fmin(float(1), pixels[i].y) * 255) <<
			(unsigned char)(std::fmin(float(1), pixels[i].z) * 255);
	}
	ofs.close();

	delete[] image;
}

void MyViewer::build_scene() {

	while(!prims.empty()){ prims.pop(); }

	CSGPrimitive sphereCSG;
	sphereCSG.center = GsVec(0, 0, -20);
	sphereCSG.sphere(4);
	sphereCSG.nfaces = 60;
	sphereCSG.material.ambient = GsColor::red;
	sphereCSG.material.diffuse = GsColor::darkred;
	sphereCSG.material.specular = GsColor::white;
	prims.push(sphereCSG);

	CSGPrimitive sphereCSG_;
	sphereCSG_.center = GsVec(0, -10004, -20);
	sphereCSG_.sphere(10000);
	sphereCSG_.nfaces = 60;
	sphereCSG_.material.ambient = GsColor::blue;
	sphereCSG_.material.diffuse = GsColor::blue;
	sphereCSG_.material.specular = GsColor::white;
	//prims.push(sphereCSG_);

}

int MyViewer::handle_keyboard(const GsEvent& e)
{
	int ret = WsViewer::handle_keyboard(e); // 1st let system check events
	if (ret) return ret;

	switch (e.key)
	{
	case GsEvent::KeyEsc: gs_exit(); return 1;
	case GsEvent::KeyLeft: gsout << "Left\n"; return 1;
		// etc
	default: gsout << "Key pressed: " << e.key << gsnl;
	}

	return 0;
}

int MyViewer::uievent(int e)
{
	switch (e)
	{
	case EvAdd: add_mynode(1); return 1;

	case EvInfo:
	{	if (output().len() > 0) { output(""); return 1; }
	output_pos(0, 30);
	activate_ogl_context(); // we need an active context
	GsOutput o; o.init(output()); gl_print_info(&o); // print info to viewer
	return 1;
	}

	case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
