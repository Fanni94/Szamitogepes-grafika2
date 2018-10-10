#include "MyApp.h"
#include "GLUtils.hpp"

#include <math.h>
#include <sstream>      // std::stringstream

#include "imgui/imgui.h"



CMyApp::CMyApp(void)
{
	m_vaoID = 0;
	m_vboID = 0;
	m_ibID = 0;
	m_fenydb = 1;

	m_programID = 0;
}


CMyApp::~CMyApp(void)
{
}

bool CMyApp::Init()
{
	// t�rl�si sz�n legyen k�kes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST); // m�lys�gi teszt bekapcsol�sa (takar�s)
	glCullFace(GL_BACK); // GL_BACK: a kamer�t�l "elfel�" n�z� lapok, GL_FRONT: a kamera fel� n�z� lapok

	//
	// geometria letrehozasa
	//

	// NxM darab n�gysz�ggel k�zel�tj�k a parametrikus fel�let�nket => (N+1)x(M+1) pontban kell ki�rt�kelni
	Vertex vert[(N + 1)*(M + 1)];
	float d[(N + 1)*(M + 1)];
	for (int i = 0; i <= N; ++i)
		for (int j = 0; j <= M; ++j)
		{
			float u = i / (float)N;
			float v = j / (float)M;

			//https://stackoverflow.com/questions/686353/c-random-float-number-generation
			float r3 = -0.05f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.05f + 0.05f)));
			vert[i + j*(N + 1)].p = glm::vec3(u, v, r3);
			//vert[i + j*(N+1)].c = glm::normalize( vert[i + j*(N+1)].p );
		}

	// indexpuffer adatai: NxM n�gysz�g = 2xNxM h�romsz�g = h�romsz�glista eset�n 3x2xNxM index
	GLushort indices[3 * 2 * (N)*(M)];
	for (int i = 0; i < N; ++i)
		for (int j = 0; j < M; ++j)
		{
			// minden n�gysz�gre csin�ljunk kett� h�romsz�get, amelyek a k�vetkez� 
			// (i,j) indexekn�l sz�letett (u_i, v_i) param�ter�rt�kekhez tartoz�
			// pontokat k�tik �ssze:
			//
			//		(i,j+1)
			//		  o-----o(i+1,j+1)
			//		  |\    |			a = p(u_i, v_i)
			//		  | \   |			b = p(u_{i+1}, v_i)
			//		  |  \  |			c = p(u_i, v_{i+1})
			//		  |   \ |			d = p(u_{i+1}, v_{i+1})
			//		  |	   \|
			//	(i,j) o-----o(i+1, j)
			//
			// - az (i,j)-hez tart�z� 1D-s index a VBO-ban: i+j*(N+1)
			// - az (i,j)-hez tart�z� 1D-s index az IB-ben: i*6+j*6*(N+1) 
			//		(mert minden n�gysz�gh�z 2db h�romsz�g = 6 index tartozik)
			//
			indices[6 * i + j * 3 * 2 * (N)+0] = (i)+(j)*	(N + 1);
			indices[6 * i + j * 3 * 2 * (N)+1] = (i + 1) + (j)*	(N + 1);
			indices[6 * i + j * 3 * 2 * (N)+2] = (i)+(j + 1)*(N + 1);
			indices[6 * i + j * 3 * 2 * (N)+3] = (i + 1) + (j)*	(N + 1);
			indices[6 * i + j * 3 * 2 * (N)+4] = (i + 1) + (j + 1)*(N + 1);
			indices[6 * i + j * 3 * 2 * (N)+5] = (i)+(j + 1)*(N + 1);
		}


	// 1 db VAO foglalasa
	glGenVertexArrays(1, &m_vaoID);
	// a frissen gener�lt VAO beallitasa akt�vnak
	glBindVertexArray(m_vaoID);

	// hozzunk l�tre egy �j VBO er�forr�s nevet
	glGenBuffers(1, &m_vboID);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID); // tegy�k "akt�vv�" a l�trehozott VBO-t
	// t�lts�k fel adatokkal az akt�v VBO-t
	glBufferData(GL_ARRAY_BUFFER,	// az akt�v VBO-ba t�lts�nk adatokat
		sizeof(vert),		// ennyi b�jt nagys�gban
		vert,	// err�l a rendszermem�riabeli c�mr�l olvasva
		GL_STATIC_DRAW);	// �gy, hogy a VBO-nkba nem tervez�nk ezut�n �rni �s minden kirajzol�skor felhasnz�ljuk a benne l�v� adatokat


// VAO-ban jegyezz�k fel, hogy a VBO-ban az els� 3 float sizeof(Vertex)-enk�nt lesz az els� attrib�tum (poz�ci�)
	glEnableVertexAttribArray(0); // ez lesz majd a poz�ci�
	glVertexAttribPointer(
		0,				// a VB-ben tal�lhat� adatok k�z�l a 0. "index�" attrib�tumait �ll�tjuk be
		3,				// komponens szam
		GL_FLOAT,		// adatok tipusa
		GL_FALSE,		// normalizalt legyen-e
		sizeof(Vertex),	// stride (0=egymas utan)
		0				// a 0. index� attrib�tum hol kezd�dik a sizeof(Vertex)-nyi ter�leten bel�l
	);

	// text�rakoordin�t�k bekapcsol�sa a 2-es azonos�t�j� attrib�tom csatorn�n
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)(sizeof(glm::vec3)));

	// index puffer l�trehoz�sa
	glGenBuffers(1, &m_ibID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0); // felt�lt�tt�k a VAO-t, kapcsoljuk le
	glBindBuffer(GL_ARRAY_BUFFER, 0); // felt�lt�tt�k a VBO-t is, ezt is vegy�k le
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // felt�lt�tt�k a VBO-t is, ezt is vegy�k le

	//
	// shaderek bet�lt�se
	//
	GLuint vs_ID = loadShader(GL_VERTEX_SHADER, "myVert.vert");
	GLuint fs_ID = loadShader(GL_FRAGMENT_SHADER, "myFrag.frag");

	// a shadereket t�rol� program l�trehoz�sa
	m_programID = glCreateProgram();

	// adjuk hozz� a programhoz a shadereket
	glAttachShader(m_programID, vs_ID);
	glAttachShader(m_programID, fs_ID);

	// VAO-beli attrib�tumok hozz�rendel�se a shader v�ltoz�khoz
	// FONTOS: linkel�s el�tt kell ezt megtenni!
	glBindAttribLocation(m_programID,	// shader azonos�t�ja, amib�l egy v�ltoz�hoz szeretn�nk hozz�rendel�st csin�lni
		0,				// a VAO-beli azonos�t� index
		"vs_in_pos");	// a shader-beli v�ltoz�n�v
	glBindAttribLocation(m_programID, 1, "vs_in_tex0");

	// illessz�k �ssze a shadereket (kimen�-bemen� v�ltoz�k �sszerendel�se stb.)
	glLinkProgram(m_programID);

	// linkeles ellenorzese
	GLint infoLogLength = 0, result = 0;

	glGetProgramiv(m_programID, GL_LINK_STATUS, &result);
	glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (GL_FALSE == result)
	{
		std::vector<char> ProgramErrorMessage(infoLogLength);
		glGetProgramInfoLog(m_programID, infoLogLength, NULL, &ProgramErrorMessage[0]);
		fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

		char* aSzoveg = new char[ProgramErrorMessage.size()];
		memcpy(aSzoveg, &ProgramErrorMessage[0], ProgramErrorMessage.size());

		std::cout << "[app.Init()] S�der Huba panasza: " << aSzoveg << std::endl;

		delete aSzoveg;
	}

	// mar nincs ezekre szukseg
	glDeleteShader(vs_ID);
	glDeleteShader(fs_ID);


	//
	// egy�b inicializ�l�s
	static float pos[3] = { 5.0f, 5.0f, 5.0f };
	static float col1[3] = { 0.50f, 0.50f, 0.50f };
	static float col2[3] = { 1.00f, 0.80f, 0.80f };
	static float col3[3] = { 1.00f, 1.00f, 0.70f };
	m_lights[0].position = glm::vec3(pos[0], pos[1], pos[2]);
	m_lights[0].ambient = glm::vec3(col1[0], col1[1], col1[2]);
	m_lights[0].diffuse = glm::vec3(col2[0], col2[1], col2[2]);
	m_lights[0].specular = glm::vec3(col3[0], col3[1], col3[2]);

	// kamera
	m_camera.SetProj(45.0f, 640.0f / 480.0f, 0.01f, 1000.0f);

	// shader-beli transzform�ci�s m�trixok c�m�nek lek�rdez�se
	m_loc_mvp = glGetUniformLocation(m_programID, "MVP");
	m_loc_world = glGetUniformLocation(m_programID, "world");
	m_loc_1texture = glGetUniformLocation(m_programID, "texImage1");
	m_loc_2texture = glGetUniformLocation(m_programID, "texImage2");
	m_loc_fenydb = glGetUniformLocation(m_programID, "fenydb");
	m_loc_eyePosition = glGetUniformLocation(m_programID, "eyePosition");

	// text�ra bet�lt�se
	m_1TextureID = TextureFromFile("bead1.bmp");
	m_2TextureID = TextureFromFile("bead2.bmp");
	return true;
}

void CMyApp::Clean()
{
	glDeleteBuffers(1, &m_vboID);
	glDeleteBuffers(1, &m_ibID);
	glDeleteVertexArrays(1, &m_vaoID);

	glDeleteProgram(m_programID);
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time) / 1000.0f;

	m_camera.Update(delta_time);

	// jegyezzuk meg az utolso update-et
	last_time = SDL_GetTicks();
}


void CMyApp::Render()
{
	// A k�vetkez� parancs megnyit egy ImGui tesztablakot �s �gy l�tszik mit tud az ImGui.
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	//ImGui::ShowTestWindow();
	if (ImGui::Begin("Fenyek"))
	{

		if (ImGui::Button("Hozzaad"))
		{
			if (m_fenydb < 10)
			{
				static float pos[3] = { 5.0f, 5.0f, 5.0f };
				static float col1[3] = { 0.50f, 0.50f, 0.50f };
				static float col2[3] = { 1.00f, 0.80f, 0.80f };
				static float col3[3] = { 1.00f, 1.00f, 0.70f };
				m_lights[m_fenydb].position = glm::vec3(pos[0], pos[1], pos[2]);
				m_lights[m_fenydb].ambient = glm::vec3(col1[0], col1[1], col1[2]);
				m_lights[m_fenydb].diffuse = glm::vec3(col2[0], col2[1], col2[2]);
				m_lights[m_fenydb].specular = glm::vec3(col3[0], col3[1], col3[2]);

				m_fenydb++;
			}
		}

		for (int i = 0; i < m_fenydb; i++)
		{
			if (ImGui::TreeNode((void*)(intptr_t)i, "Feny %d", i + 1))
			{
				static float pos[3] = { 5.0f, 5.0f, 5.0f };
				ImGui::InputFloat3("Pozicio", pos);

				static float col1[3] = { 0.50f, 0.50f, 0.50f };
				static float col2[3] = { 1.00f, 0.80f, 0.80f };
				static float col3[3] = { 1.00f, 1.00f, 0.70f };
				ImGui::ColorEdit3("Ambiens", col1);
				ImGui::ColorEdit3("Diffuz", col2);
				ImGui::ColorEdit3("Spekularis", col3);

				m_lights[i].position = glm::vec3(pos[0], pos[1], pos[2]);
				m_lights[i].ambient = glm::vec3(col1[0], col1[1], col1[2]);
				m_lights[i].diffuse = glm::vec3(col2[0], col2[1], col2[2]);
				m_lights[i].specular = glm::vec3(col3[0], col3[1], col3[2]);

				ImGui::TreePop();
			}
		}
	}
	ImGui::End();

	// t�r�lj�k a frampuffert (GL_COLOR_BUFFER_BIT) �s a m�lys�gi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// shader bekapcsolasa
	glUseProgram(m_programID);

	// shader parameterek be�ll�t�sa
	/*

	GLM transzform�ci�s m�trixokra p�ld�k:
		glm::rotate<float>( sz�g, glm::vec3(tengely_x, tengely_y, tengely_z) ) <- tengely_{xyz} k�r�li elforgat�s
		glm::translate<float>( glm::vec3(eltol_x, eltol_y, eltol_z) ) <- eltol�s
		glm::scale<float>( glm::vec3(s_x, s_y, s_z) ) <- l�pt�kez�s

	*/
	glm::mat4 matWorld = glm::mat4(1.0);
	glm::mat4 mvp = matWorld*m_camera.GetViewProj();

	// majd k�ldj�k �t a megfelel� m�trixot!
	glUniformMatrix4fv(m_loc_mvp,// erre a helyre t�lts�nk �t adatot
		1,			// egy darab m�trixot
		GL_FALSE,	// NEM transzpon�lva
		&(mvp[0][0])); // innen olvasva a 16 x sizeof(float)-nyi adatot

// majd k�ldj�k �t a megfelel� m�trixot!
	glUniformMatrix4fv(m_loc_world,// erre a helyre t�lts�nk �t adatot
		1,			// egy darab m�trixot
		GL_FALSE,	// NEM transzpon�lva
		&(matWorld[0][0])); // innen olvasva a 16 x sizeof(float)-nyi adatot

	glUniform3f(m_loc_eyePosition, m_camera.GetEye().x, m_camera.GetEye().y, m_camera.GetEye().z);
	glUniform1i(m_loc_fenydb, m_fenydb);

	for (int i = 0; i < m_fenydb; i++)
	{
		std::stringstream current_uniform;

		current_uniform << "lights[" << i << "].Position";
		auto test = glGetUniformLocation(m_programID, current_uniform.str().c_str());
		glUniform3f(test, m_lights[i].position.x, m_lights[i].position.y, m_lights[i].position.z);
		current_uniform.str("");

		current_uniform << "lights[" << i << "].La";
		glUniform3f(glGetUniformLocation(m_programID, current_uniform.str().c_str()), m_lights[i].ambient.x, m_lights[i].ambient.y, m_lights[i].ambient.z);
		current_uniform.str("");

		current_uniform << "lights[" << i << "].Ld";
		glUniform3f(glGetUniformLocation(m_programID, current_uniform.str().c_str()), m_lights[i].diffuse.x, m_lights[i].diffuse.y, m_lights[i].diffuse.z);
		current_uniform.str("");

		current_uniform << "lights[" << i << "].Ls";
		glUniform3f(glGetUniformLocation(m_programID, current_uniform.str().c_str()), m_lights[i].specular.x, m_lights[i].specular.y, m_lights[i].specular.z);
	}


	// aktiv�ljuk a 0-�s text�ra mintav�telez� egys�get
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_1TextureID);

	// text�ra mintav�telez� �s shader-beli sampler2D �sszerendel�se
	glUniform1i(m_loc_1texture,	// ezen azonos�t�j� sampler 2D
		0);				// olvassa az ezen index� mintav�telez�t

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_2TextureID);
	glUniform1i(m_loc_2texture, 1);

	glUniform1f(glGetUniformLocation(m_programID, "t"), sin(SDL_GetTicks() / 2000.0*M_PI) / 2.0 + 0.5);


	// kapcsoljuk be a VAO-t (a VBO j�n vele egy�tt)
	glBindVertexArray(m_vaoID);

	// kirajzol�s
	glDrawElements(GL_TRIANGLES,		// primit�v t�pus
		3 * 2 * (N)*(M),		// hany csucspontot hasznalunk a kirajzolashoz
		GL_UNSIGNED_SHORT,	// indexek tipusa
		0);					// indexek cime

// VAO kikapcsolasa
	glBindVertexArray(0);

	// shader kikapcsolasa
	glUseProgram(0);
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// a k�t param�terbe az �j ablakm�ret sz�less�ge (_w) �s magass�ga (_h) tal�lhat�
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.Resize(_w, _h);
}
