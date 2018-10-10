#pragma once

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "gCamera.h"

struct Light {
	glm::vec3 ambient;
	glm::vec3 specular;
	glm::vec3 diffuse;
	glm::vec3 position;
};

class CMyApp
{
public:
	CMyApp(void);
	~CMyApp(void);

	bool Init();
	void Clean();

	void Update();
	void Render();

	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);
protected:
	// segédfüggvények
	glm::vec3 GetUV_henger(float u, float v);
	glm::vec3 GetUV_torusz(float u, float v);

	// shaderekhez szükséges változók
	GLuint m_programID; // shaderek programja

	// OpenGL-es dolgok
	GLuint m_vaoID; // vertex array object erõforrás azonosító
	GLuint m_vboID; // vertex buffer object erõforrás azonosító
	GLuint m_ibID;  // index buffer object erõforrás azonosító
	GLuint m_1TextureID; // fájlból betöltött textúra azonosítója
	GLuint m_2TextureID; // fájlból betöltött textúra azonosítója

	// transzformációs mátrixok
	int m_fenydb;
	Light m_lights[10];
 	gCamera	m_camera;
	
	GLuint m_loc_fenydb;
	GLuint m_loc_eyePosition;

	// mátrixok helye a shaderekben
	GLuint	m_loc_mvp; // a három mátrixunk szorzatát adjuk át a hatékonyság érdekében
  GLuint  m_loc_world;
	GLuint	m_loc_1texture, m_loc_2texture;

	struct Vertex
	{
		glm::vec3 p;
	};

	// NxM darab négyszöggel közelítjük a parametrikus felületünket => (N+1)x(M+1) pontban kell kiértékelni
	static const int N	= 100;
	static const int M	= 100;
};

