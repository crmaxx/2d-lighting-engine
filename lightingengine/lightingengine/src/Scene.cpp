#include "Scene.h"
#include "Game.h"

Scene::Scene() {
	m_Polygon *poly = new m_Polygon(5);
	poly->setVertex(0, vector2f(100, 100));
	poly->setVertex(1, vector2f(100, 200));
	poly->setVertex(2, vector2f(200, 200));
	poly->setVertex(3, vector2f(250, 150));
	poly->setVertex(4, vector2f(200, 100));
	polygons.push_back(poly);
	poly = new m_Polygon(4);
	poly->setVertex(0, vector2f(300, 300));
	poly->setVertex(1, vector2f(300, 400));
	poly->setVertex(2, vector2f(400, 400));
	poly->setVertex(3, vector2f(450, 350));
	polygons.push_back(poly);

	Light *l = new Light(vector2f(300, 300), 400);
	lights.push_back(l);

	//Create the lighting alpha texture
	//http://www.opengl.org/wiki/Framebuffer_Object_Examples

	fbo = new FrameBufferObject(800, 600);
	individualLightFbo = new FrameBufferObject(800, 600);
}

Scene::~Scene() {
	delete fbo;
	delete individualLightFbo;
}

void Scene::tick() {
	int mx, my;
	Uint32 mouse = SDL_GetMouseState(&mx, &my);
	if((mouse & SDL_BUTTON_LEFT) && !(lastMouseState & SDL_BUTTON_LEFT)) {
		lights.push_back(new Light(vector2f(mx, my), 400));
	}
	lights[lights.size()-1]->pos = vector2f(mx, my);
	for(int i=0; i < lights.size(); i++) {
		lights[i]->tick();
	}
}

void Scene::drawLighting() {
	//Set up the main fbo
	fbo->bindFrameBuffer(GL_FRAMEBUFFER_EXT);
	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
	
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glColorMask(false, false, false, true);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glViewport(0, 0, fbo->width, fbo->height);
	fbo->unbindFrameBuffer(GL_FRAMEBUFFER_EXT);
	//
	
	//Draw each light to a seperate frame buffer, then draw that frame buffer to the main one
	for(int i=0; i < lights.size(); i++) {
		individualLightFbo->bindFrameBuffer(GL_FRAMEBUFFER_EXT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
		lights[i]->drawAlpha(this);
		individualLightFbo->unbindFrameBuffer(GL_FRAMEBUFFER_EXT);
		
		//Draw that frame buffer to the main one
		fbo->bindFrameBuffer(GL_FRAMEBUFFER_EXT);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		individualLightFbo->draw();
		fbo->unbindFrameBuffer(GL_FRAMEBUFFER_EXT);
	}
	glPopAttrib();

	//Draw geometry
	fbo->bindFrameBuffer(GL_FRAMEBUFFER_EXT);
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glColorMask(true, true, true, false);
	//Draw the lights (colored and everything)
	//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
	for(int i=0; i < lights.size(); i++) {
		lights[i]->draw(this);
	}

	//Draw the scene objects
	//glBlendFunc(GL_DST_COLOR, GL_ONE);
	//glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR);
	//glBlendFunc(GL_SRC_COLOR, GL_ONE);

	glBlendFunc(GL_DST_ALPHA, GL_DST_ALPHA); //THIS ONE
	for(int i=0; i < polygons.size(); i++) {
		float cols[] = { 1.0f, 0.0f, .5f, 1.0f,
						 1.0f, 1.0f, .5f, 1.0f,
						 0.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 0.0f, .5f, 1.0f,
						 1.0f, 0.0f, .5f, 1.0f };
		polygons[i]->draw(cols);
	}	

	glPopAttrib();
	fbo->unbindFrameBuffer(GL_FRAMEBUFFER_EXT);
}

void Scene::draw() {
	glDisable(GL_DEPTH_TEST);
	drawLighting();
	fbo->draw();
	glEnable(GL_DEPTH_TEST);
}