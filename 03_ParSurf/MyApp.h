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
	// seg�df�ggv�nyek
	glm::vec3 GetUV_henger(float u, float v);
	glm::vec3 GetUV_torusz(float u, float v);

	// shaderekhez sz�ks�ges v�ltoz�k
	GLuint m_programID; // shaderek programja

	// OpenGL-es dolgok
	GLuint m_vaoID; // vertex array object er�forr�s azonos�t�
	GLuint m_vboID; // vertex buffer object er�forr�s azonos�t�
	GLuint m_ibID;  // index buffer object er�forr�s azonos�t�
	GLuint m_1TextureID; // f�jlb�l bet�lt�tt text�ra azonos�t�ja
	GLuint m_2TextureID; // f�jlb�l bet�lt�tt text�ra azonos�t�ja

	// transzform�ci�s m�trixok
	int m_fenydb;
	Light m_lights[10];
 	gCamera	m_camera;
	
	GLuint m_loc_fenydb;
	GLuint m_loc_eyePosition;

	// m�trixok helye a shaderekben
	GLuint	m_loc_mvp; // a h�rom m�trixunk szorzat�t adjuk �t a hat�konys�g �rdek�ben
  GLuint  m_loc_world;
	GLuint	m_loc_1texture, m_loc_2texture;

	struct Vertex
	{
		glm::vec3 p;
	};

	// NxM darab n�gysz�ggel k�zel�tj�k a parametrikus fel�let�nket => (N+1)x(M+1) pontban kell ki�rt�kelni
	static const int N	= 100;
	static const int M	= 100;
};

