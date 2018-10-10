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
	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)
	glCullFace(GL_BACK); // GL_BACK: a kamerától "elfelé" nézõ lapok, GL_FRONT: a kamera felé nézõ lapok

	//
	// geometria letrehozasa
	//

	// NxM darab négyszöggel közelítjük a parametrikus felületünket => (N+1)x(M+1) pontban kell kiértékelni
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

	// indexpuffer adatai: NxM négyszög = 2xNxM háromszög = háromszöglista esetén 3x2xNxM index
	GLushort indices[3 * 2 * (N)*(M)];
	for (int i = 0; i < N; ++i)
		for (int j = 0; j < M; ++j)
		{
			// minden négyszögre csináljunk kettõ háromszöget, amelyek a következõ 
			// (i,j) indexeknél született (u_i, v_i) paraméterértékekhez tartozó
			// pontokat kötik össze:
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
			// - az (i,j)-hez tartózó 1D-s index a VBO-ban: i+j*(N+1)
			// - az (i,j)-hez tartózó 1D-s index az IB-ben: i*6+j*6*(N+1) 
			//		(mert minden négyszöghöz 2db háromszög = 6 index tartozik)
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
	// a frissen generált VAO beallitasa aktívnak
	glBindVertexArray(m_vaoID);

	// hozzunk létre egy új VBO erõforrás nevet
	glGenBuffers(1, &m_vboID);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID); // tegyük "aktívvá" a létrehozott VBO-t
	// töltsük fel adatokkal az aktív VBO-t
	glBufferData(GL_ARRAY_BUFFER,	// az aktív VBO-ba töltsünk adatokat
		sizeof(vert),		// ennyi bájt nagyságban
		vert,	// errõl a rendszermemóriabeli címrõl olvasva
		GL_STATIC_DRAW);	// úgy, hogy a VBO-nkba nem tervezünk ezután írni és minden kirajzoláskor felhasnzáljuk a benne lévõ adatokat


// VAO-ban jegyezzük fel, hogy a VBO-ban az elsõ 3 float sizeof(Vertex)-enként lesz az elsõ attribútum (pozíció)
	glEnableVertexAttribArray(0); // ez lesz majd a pozíció
	glVertexAttribPointer(
		0,				// a VB-ben található adatok közül a 0. "indexû" attribútumait állítjuk be
		3,				// komponens szam
		GL_FLOAT,		// adatok tipusa
		GL_FALSE,		// normalizalt legyen-e
		sizeof(Vertex),	// stride (0=egymas utan)
		0				// a 0. indexû attribútum hol kezdõdik a sizeof(Vertex)-nyi területen belül
	);

	// textúrakoordináták bekapcsolása a 2-es azonosítójú attribútom csatornán
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)(sizeof(glm::vec3)));

	// index puffer létrehozása
	glGenBuffers(1, &m_ibID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0); // feltöltüttük a VAO-t, kapcsoljuk le
	glBindBuffer(GL_ARRAY_BUFFER, 0); // feltöltöttük a VBO-t is, ezt is vegyük le
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // feltöltöttük a VBO-t is, ezt is vegyük le

	//
	// shaderek betöltése
	//
	GLuint vs_ID = loadShader(GL_VERTEX_SHADER, "myVert.vert");
	GLuint fs_ID = loadShader(GL_FRAGMENT_SHADER, "myFrag.frag");

	// a shadereket tároló program létrehozása
	m_programID = glCreateProgram();

	// adjuk hozzá a programhoz a shadereket
	glAttachShader(m_programID, vs_ID);
	glAttachShader(m_programID, fs_ID);

	// VAO-beli attribútumok hozzárendelése a shader változókhoz
	// FONTOS: linkelés elõtt kell ezt megtenni!
	glBindAttribLocation(m_programID,	// shader azonosítója, amibõl egy változóhoz szeretnénk hozzárendelést csinálni
		0,				// a VAO-beli azonosító index
		"vs_in_pos");	// a shader-beli változónév
	glBindAttribLocation(m_programID, 1, "vs_in_tex0");

	// illesszük össze a shadereket (kimenõ-bemenõ változók összerendelése stb.)
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

		std::cout << "[app.Init()] Sáder Huba panasza: " << aSzoveg << std::endl;

		delete aSzoveg;
	}

	// mar nincs ezekre szukseg
	glDeleteShader(vs_ID);
	glDeleteShader(fs_ID);


	//
	// egyéb inicializálás
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

	// shader-beli transzformációs mátrixok címének lekérdezése
	m_loc_mvp = glGetUniformLocation(m_programID, "MVP");
	m_loc_world = glGetUniformLocation(m_programID, "world");
	m_loc_1texture = glGetUniformLocation(m_programID, "texImage1");
	m_loc_2texture = glGetUniformLocation(m_programID, "texImage2");
	m_loc_fenydb = glGetUniformLocation(m_programID, "fenydb");
	m_loc_eyePosition = glGetUniformLocation(m_programID, "eyePosition");

	// textúra betöltése
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
	// A következ¿ parancs megnyit egy ImGui tesztablakot és így látszik mit tud az ImGui.
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

	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT) és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// shader bekapcsolasa
	glUseProgram(m_programID);

	// shader parameterek beállítása
	/*

	GLM transzformációs mátrixokra példák:
		glm::rotate<float>( szög, glm::vec3(tengely_x, tengely_y, tengely_z) ) <- tengely_{xyz} körüli elforgatás
		glm::translate<float>( glm::vec3(eltol_x, eltol_y, eltol_z) ) <- eltolás
		glm::scale<float>( glm::vec3(s_x, s_y, s_z) ) <- léptékezés

	*/
	glm::mat4 matWorld = glm::mat4(1.0);
	glm::mat4 mvp = matWorld*m_camera.GetViewProj();

	// majd küldjük át a megfelelõ mátrixot!
	glUniformMatrix4fv(m_loc_mvp,// erre a helyre töltsünk át adatot
		1,			// egy darab mátrixot
		GL_FALSE,	// NEM transzponálva
		&(mvp[0][0])); // innen olvasva a 16 x sizeof(float)-nyi adatot

// majd küldjük át a megfelelõ mátrixot!
	glUniformMatrix4fv(m_loc_world,// erre a helyre töltsünk át adatot
		1,			// egy darab mátrixot
		GL_FALSE,	// NEM transzponálva
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


	// aktiváljuk a 0-és textúra mintavételezõ egységet
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_1TextureID);

	// textúra mintavételezõ és shader-beli sampler2D összerendelése
	glUniform1i(m_loc_1texture,	// ezen azonosítójú sampler 2D
		0);				// olvassa az ezen indexû mintavételezõt

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_2TextureID);
	glUniform1i(m_loc_2texture, 1);

	glUniform1f(glGetUniformLocation(m_programID, "t"), sin(SDL_GetTicks() / 2000.0*M_PI) / 2.0 + 0.5);


	// kapcsoljuk be a VAO-t (a VBO jön vele együtt)
	glBindVertexArray(m_vaoID);

	// kirajzolás
	glDrawElements(GL_TRIANGLES,		// primitív típus
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

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.Resize(_w, _h);
}
