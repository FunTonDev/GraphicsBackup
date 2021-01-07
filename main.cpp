#define cubemapSpace 2
#include "Utility.h"	//INCLUDES GLEW, GLFW3, SOIL2, FSTREAM, IOSTREAM, STRING
#include "Camera.h" //INCLUDES GLM, GLFW3
#include "Light.h"	//INCLUDES GLM
#include "Entity.h"	//INCLUDES GLM, MATERIAL AND IMPORTEDMODEL

#include <glm/gtc/type_ptr.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

//==========================================================================================================================================
//|                                                                                                                                        |
//|																VARIABLES				                                                   |
//|                                                                                                                                        |
//==========================================================================================================================================
//Display vars
GLFWwindow* window;
GLuint mainProgram, shadowProgram, cubemapProgram;
GLuint vao[30];	//INT_MAX = 2147483647
GLuint vbo[1000];	//INT_MAX = 2147483647
int windowWidth = 1600, windowHeight = 900;
float aspect;

//Materials
Material goldMat(MaterialType::Gold), silverMat(MaterialType::Silver), obsidianMat(MaterialType::Obsidian), plasticMat(MaterialType::Plastic), 
bronzeMat(MaterialType::Bronze), rubberMat(MaterialType::Rubber), tinMat(MaterialType::Tin), stoneMat(MaterialType::Stone);

//Uniform vars
GLuint projLoc, viewLoc, viewPosLoc, posLoc, rotLoc, scaLoc; //MODEL
GLuint globalAmbLoc, lightAmbLoc, lightDiffLoc, lightSpecLoc, lightPosLoc, //LIGHT
lightDirLoc, lightCutoffLoc, lightExponentLoc, lightIntensityLoc,
posLightCountLoc, spotLightCountLoc;
GLuint matShiLoc, matAmbLoc, matDiffLoc, matSpecLoc;		//MATERIAL
GLuint lightPMatLoc, lightVMatLoc; //SHADOW

//Render vars
glm::mat4 pMat, vMat, lightVMat, lightPMat;

//Entities
Camera userCam(glm::vec3(0.0f, 0.2f, 6.0f));//(12, -3.0f, -20)
vector<Entity*> spawnedEntities;
vector<Light*> spawnedLights;

//Textures
GLuint grassTileTex, asphaultTileTex, brickTileTex, waterTileTex, stoneTileTex;
GLuint blankTex, plasticTex, goldTex, steelTex;
GLuint skybox;

//Shadows
GLuint shadowTex, shadowBuffer;

//==========================================================================================================================================
//|                                                                                                                                        |
//|															PROTOTYPE FUNCTIONS			                                                   |
//|                                                                                                                                        |
//==========================================================================================================================================
void Init();

void Display(double currentTime);
	void ShadowPass1();
	void RenderCubemap();
	void RenderScene();
		void InstallLight(Material entityMat);
	void ImGuiUpdate();

void WindowSizeCallback(GLFWwindow* window, int newW, int newH);
void MiscKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

//==========================================================================================================================================
//|                                                                                                                                        |
//|															IMPLEMENTED FUNCTIONS			                                               |
//|                                                                                                                                        |
//==========================================================================================================================================
int main()
{
	//Basic window/context setup
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	window = glfwCreateWindow(windowWidth, windowHeight, "Project 3", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	//Callback setup
	glfwSetWindowSizeCallback(window, WindowSizeCallback);
	glfwSetKeyCallback(window, MiscKeyCallback);

	//ImGui setup
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	Init();

	while (!glfwWindowShouldClose(window))
	{
		Display(glfwGetTime());
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	//Clean up/termination
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void Init()
{
	WindowSizeCallback(window, windowWidth, windowHeight);

	mainProgram = Utility::CreateShaderProgram("glsl/mainVertShader.glsl", "glsl/mainFragShader.glsl");
	shadowProgram = Utility::CreateShaderProgram("glsl/sVertShader.glsl", "glsl/sFragShader.glsl");
	cubemapProgram = Utility::CreateShaderProgram("glsl/cmVertShader.glsl", "glsl/cmFragShader.glsl");

	//SCENE SETUP
	//------------------------------------------------------------
	skybox = Utility::LoadCubeMap("cubemaps/cubemap3");				//CUBEMAP

	grassTileTex = Utility::LoadTileTexture("textures/grass.jpg");	//TILETEXTURES
	asphaultTileTex = Utility::LoadTileTexture("textures/asphault.jpg");
	brickTileTex = Utility::LoadTileTexture("textures/brick.jpg");
	waterTileTex = Utility::LoadTileTexture("textures/water.jpg");
	stoneTileTex = Utility::LoadTileTexture("textures/stone.jpg");

	blankTex = Utility::LoadTexture("textures/blank.jpg");
	plasticTex = Utility::LoadTexture("textures/plastic.jpg");		//TEXTURES
	goldTex = Utility::LoadTexture("textures/gold.jpg");
	steelTex = Utility::LoadTexture("textures/steel.jpg");

	/*spawnedEntities.push_back(new Entity("models/TileCube.obj"));	//MODELS
	Entity* street1 = spawnedEntities.back();
	street1->SetTextureID(asphaultTileTex);
	street1->SetMaterial(stoneMat);
	street1->SetPosition(glm::vec3(0.0f, -6.0f, 0.0f));
	street1->SetScale(glm::vec3(12.0f, 1.0f, 90.0f));

	vector<Entity*> side;
	spawnedEntities.push_back(new Entity("models/TileCube.obj"));
	side.push_back(spawnedEntities.back());
	spawnedEntities.push_back(new Entity("models/TileCube.obj"));
	side.push_back(spawnedEntities.back());
	for (Entity* s : side) { s->SetTextureID(stoneTileTex); s->SetMaterial(stoneMat); }
	side[0]->SetPosition(glm::vec3(-40.0f, -5.0f, 0.0f));	side[0]->SetScale(glm::vec3(30.0f, 0.4f, 90.0f));
	side[1]->SetPosition(glm::vec3(30.0f, -5.0f, 0.0f));	side[1]->SetScale(glm::vec3(20.0f, 0.4f, 90.0f));

	vector<Entity*> streetlights;
	spawnedEntities.push_back(new Entity("models/StreetLight.obj"));
	streetlights.push_back(spawnedEntities.back());
	spawnedEntities.push_back(new Entity("models/StreetLight.obj"));
	streetlights.push_back(spawnedEntities.back());
	spawnedEntities.push_back(new Entity("models/StreetLight.obj"));
	streetlights.push_back(spawnedEntities.back());
	spawnedEntities.push_back(new Entity("models/StreetLight.obj"));
	streetlights.push_back(spawnedEntities.back());
	spawnedEntities.push_back(new Entity("models/StreetLight.obj"));
	streetlights.push_back(spawnedEntities.back());
	spawnedEntities.push_back(new Entity("models/StreetLight.obj"));
	streetlights.push_back(spawnedEntities.back());

	for (int i = 0; i < streetlights.size(); i++){
		float sign2 = (i % 2);	//0 or 1
		float sign1 = sign2 * 2 - 1;

		streetlights[i]->SetTextureID(blankTex);
		streetlights[i]->SetMaterial(tinMat);
		streetlights[i]->SetPosition(glm::vec3(sign1 * 12.0f, 0.0f, 70.0f - i * 30.0f));
		streetlights[i]->SetRotation(glm::vec3(0.0f, sign2 * 180.0f, 0.0f));
		streetlights[i]->SetScale(glm::vec3(0.1f));
	}

	vector<Entity*> building;
	spawnedEntities.push_back(new Entity("models/Building1.obj"));
	building.push_back(spawnedEntities.back());	//1a
	spawnedEntities.push_back(new Entity("models/Building1.obj"));
	building.push_back(spawnedEntities.back()); //1b
	spawnedEntities.push_back(new Entity("models/Building2.obj"));
	building.push_back(spawnedEntities.back()); //2a
	spawnedEntities.push_back(new Entity("models/Building2.obj"));
	building.push_back(spawnedEntities.back()); //2b
	spawnedEntities.push_back(new Entity("models/Building3.obj"));
	building.push_back(spawnedEntities.back()); //3a
	spawnedEntities.push_back(new Entity("models/Building3.obj"));
	building.push_back(spawnedEntities.back()); //3b

	for (Entity* b : building) { b->SetScale(glm::vec3(2)); }

	building[0]->SetTextureID(brickTileTex);	building[0]->SetMaterial(stoneMat);
	building[0]->SetPosition(glm::vec3(-40.0f, -4.5f, -40.0f));		building[0]->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));

	building[1]->SetTextureID(brickTileTex);	building[1]->SetMaterial(stoneMat);
	building[1]->SetPosition(glm::vec3(30.0f, -4.5f, 18.0f));		building[1]->SetRotation(glm::vec3(0.0f, 180.0f, 0.0f));

	building[2]->SetTextureID(stoneTileTex);	building[2]->SetMaterial(stoneMat);
	building[2]->SetPosition(glm::vec3(33.0f, -4.5f, -30.0f));		building[2]->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

	building[3]->SetTextureID(stoneTileTex);	building[3]->SetMaterial(stoneMat);
	building[3]->SetPosition(glm::vec3(-40.0f, -4.5f, 40.0f));		building[3]->SetRotation(glm::vec3(0.0f, 90.0f, 0.0f));

	building[4]->SetTextureID(steelTex);	building[4]->SetMaterial(obsidianMat);
	building[4]->SetPosition(glm::vec3(-70.0f, -4.5f, 0.0f));		building[4]->SetRotation(glm::vec3(0.0f, 180.0f, 0.0f));

	building[5]->SetTextureID(steelTex);	building[5]->SetMaterial(obsidianMat);
	building[5]->SetPosition(glm::vec3(30.0f, -4.5f, 70.0f));		building[5]->SetRotation(glm::vec3(0.0f, 180.0f, 0.0f));

	spawnedEntities.push_back(new Entity("models/Statue.obj"));
	Entity* statue = spawnedEntities.back();
	statue->SetTextureID(goldTex);		statue->SetMaterial(goldMat);
	statue->SetPosition(glm::vec3(-30.0f, 4.5f, 0.0f));		statue->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));		statue->SetScale(glm::vec3(2));

	vector<Entity*> pedestal;
	spawnedEntities.push_back(new Entity("models/Pedestal.obj"));
	pedestal.push_back(spawnedEntities.back());
	spawnedEntities.push_back(new Entity("models/Pedestal.obj"));
	pedestal.push_back(spawnedEntities.back());
	for (Entity* p : pedestal) { p->SetTextureID(steelTex); p->SetMaterial(silverMat); }
	pedestal[0]->SetPosition(glm::vec3(-16.0f, -3.0f, 13.0f));	pedestal[0]->SetRotation(glm::vec3(0.0f, -45.0f, 0.0f));
	pedestal[1]->SetPosition(glm::vec3(-16.0f, -3.0f, -13.0f));	pedestal[1]->SetRotation(glm::vec3(0.0f, 45.0f, 0.0f));*/
	
	spawnedEntities.push_back(new Entity("models/Plane.obj"));
	spawnedEntities.back()->SetMaterial(goldMat);
	spawnedEntities.back()->SetPosition(glm::vec3(0.0f, -2.0f, 0.0f));
	spawnedEntities.back()->SetScale(glm::vec3(3.0f));
	spawnedEntities.push_back(new Entity("models/Pyramid.obj"));
	spawnedEntities.back()->SetMaterial(goldMat);
	spawnedEntities.back()->SetPosition(glm::vec3(-1.0f, 0.1f, 0.3f));
	spawnedEntities.push_back(new Entity("models/Torus.obj"));
	spawnedEntities.back()->SetMaterial(bronzeMat);
	spawnedEntities.back()->SetPosition(glm::vec3(1.6f, 0.0f, -0.3f));
	spawnedEntities.back()->SetRotation(glm::vec3(-25.0f, 0.0f, 0.0f));

	spawnedLights.push_back(new Light(LightType::Positional));			//LIGHTS
	Light* light1 = spawnedLights.back();
	light1->SetPosition(glm::vec3(-3.8f, 2.2f, 1.1f));//0.0f, 5.0f, 10.0f
	
	//spawnedEntities.push_back(new Entity("models/Sphere.obj"));
	//spawnedEntities.back()->SetPosition(light1->GetPosition());
	//lightEnt[0]->SetScale(glm::vec3(0.25f));

	//VAO/VBO SETUP
	//------------------------------------------------------------
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(cubemapSpace + spawnedEntities.size() * bufferSpacing, vbo);

	Entity cube("models/Cube.obj");		//CUBEMAP
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, cube.GetVertexPositions().size() * 4, &cube.GetVertexPositions()[0], GL_STATIC_DRAW);//POSITIONS
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, cube.GetVertexTexCoords().size() * 4, &cube.GetVertexTexCoords()[0], GL_STATIC_DRAW);//TEXCOORDS

	for (Entity* e : spawnedEntities){	//ENTITIES
		glBindBuffer(GL_ARRAY_BUFFER, vbo[cubemapSpace + e->GetBufferIndex()]);
		glBufferData(GL_ARRAY_BUFFER, e->GetVertexPositions().size() * 4, &e->GetVertexPositions()[0], GL_STATIC_DRAW);	//POSITIONS
		glBindBuffer(GL_ARRAY_BUFFER, vbo[cubemapSpace + e->GetBufferIndex() + 1]);
		glBufferData(GL_ARRAY_BUFFER, e->GetVertexTexCoords().size() * 4, &e->GetVertexTexCoords()[0], GL_STATIC_DRAW);	//TEXCOORDS
		glBindBuffer(GL_ARRAY_BUFFER, vbo[cubemapSpace + e->GetBufferIndex() + 2]);
		glBufferData(GL_ARRAY_BUFFER, e->GetVertexNormals().size() * 4, &e->GetVertexNormals()[0], GL_STATIC_DRAW);		//NORMALS
	}

	//SHADOW BUFFER SETUP
	//------------------------------------------------------------
	glGenFramebuffers(1, &shadowBuffer);

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Display(double currentTime)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	userCam.Update(window);

	pMat = userCam.GetCameraProj();
	vMat = userCam.GetCameraView();
	
	lightVMat = glm::lookAt(spawnedLights[0]->GetPosition(), glm::vec3(0), glm::vec3(0, 1, 0));	// vector from light to origin
	lightPMat = glm::perspective(degToRad * 60.0f, aspect, 0.1f, 1000.0f);

	ShadowPass1();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);//RESTORE DEFAULT FRAMEBUFFER, RE-ENABLE DRAWING

	glDrawBuffer(GL_FRONT);

	RenderScene();
}

void ImGuiUpdate()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Info");
	ImGui::Text("CAM POSITION(%f, %f, %f)", userCam.GetPosition().x, userCam.GetPosition().y, userCam.GetPosition().z);
	ImGui::NewLine();

	ImGui::Text("Entity List(%d)", spawnedEntities.size());
	ImGui::Indent();
	for (Entity* e : spawnedEntities)
	{
		ImGui::Text("%s", e->GetName().c_str());
	}
	ImGui::Unindent();

	ImGui::Text("Light List(%d)", spawnedEntities.size());
	ImGui::Indent();
	for (Light* li : spawnedLights)
	{
		glm::vec3 pos = li->GetPosition();
		switch (li->GetType())
		{
			case (LightType::Directional):
				ImGui::Text("Directional: (%f, %f, %f)", pos.x, pos.y, pos.z);
				break;
			case (LightType::Spotlight):
				ImGui::Text("Spotlight: (%f, %f, %f)", pos.x, pos.y, pos.z);
				break;
			default:
				ImGui::Text("Positional: (%f, %f, %f)", pos.x, pos.y, pos.z);
				break;
		}
	}
	ImGui::Unindent();

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::TextColored(ImVec4(0.9f, 1.0f, 0.0f, 1.0f), "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void RenderCubemap()
{
	//RENDER SKYBOX
	glUseProgram(cubemapProgram);

	projLoc = glGetUniformLocation(cubemapProgram, "u_projMat");
	viewLoc = glGetUniformLocation(cubemapProgram, "u_viewMat");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(vMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);// positions
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

	glDisable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
}

void ShadowPass1()
{
	glUseProgram(shadowProgram);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	
	//CLEARS SHADOW ACNE
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//UNIFORM LOCATIONS
	/*posLoc = glGetUniformLocation(shadowProgram, "u_posVec");
	rotLoc = glGetUniformLocation(shadowProgram, "u_rotVec");
	scaLoc = glGetUniformLocation(shadowProgram, "u_scaVec");
	lightPMatLoc = glGetUniformLocation(shadowProgram, "u_lightPMatrix");
	lightVMatLoc = glGetUniformLocation(shadowProgram, "u_lightVMatrix");*/

	GLuint t_sLoc = glGetUniformLocation(shadowProgram, "shadowMVP1");

	for (Entity* e : spawnedEntities)
	{
		//UNIFORM UPDATE
		/*glUniform3fv(posLoc, 1, glm::value_ptr(e->GetPosition()));		//v
		glUniform3fv(rotLoc, 1, glm::value_ptr(e->GetRotation()));		//TRANSFORM MATS
		glUniform3fv(scaLoc, 1, glm::value_ptr(e->GetScale()));			//^
		glUniformMatrix4fv(lightPMatLoc, 1, GL_FALSE, glm::value_ptr(lightPMat));
		glUniformMatrix4fv(lightVMatLoc, 1, GL_FALSE, glm::value_ptr(lightVMat));*/

		glm::mat4 t_shadowMVP1 = lightPMat * lightVMat * 
						glm::translate(glm::mat4(1), e->GetPosition()) * 
						glm::rotate(glm::mat4(1), e->GetRotation().x, glm::vec3(1, 0, 0)) * 
							glm::rotate(glm::mat4(1), e->GetRotation().y, glm::vec3(0, 1, 0)) * 
								glm::rotate(glm::mat4(1), e->GetRotation().z, glm::vec3(0, 0, 1)) * 
						glm::scale(glm::mat4(1), e->GetScale());
		glUniformMatrix4fv(t_sLoc, 1, GL_FALSE, glm::value_ptr(t_shadowMVP1));

		// VERTEX ATTRIBUTE UPDATE(only need vertices position buffer)
		glBindBuffer(GL_ARRAY_BUFFER, vbo[cubemapSpace + e->GetBufferIndex()]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLES, 0, e->GetVertexCount());
	}
}

void RenderScene()
{
	//NORMALLY RENDER OTHER MODELS
	glUseProgram(mainProgram);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	projLoc = glGetUniformLocation(mainProgram, "u_projMat");
	GLuint t_mvLoc = glGetUniformLocation(mainProgram, "mv_matrix");
	GLuint t_nLoc = glGetUniformLocation(mainProgram, "norm_matrix");
	GLuint t_sLoc = glGetUniformLocation(mainProgram, "shadowMVP");

	/*viewLoc = glGetUniformLocation(mainProgram, "u_viewMat");
	viewPosLoc = glGetUniformLocation(mainProgram, "u_viewPos");

	posLoc = glGetUniformLocation(mainProgram, "u_posVec");
	rotLoc = glGetUniformLocation(mainProgram, "u_rotVec");
	scaLoc = glGetUniformLocation(mainProgram, "u_scaVec");
	lightPMatLoc = glGetUniformLocation(mainProgram, "u_lightPMatrix");
	lightVMatLoc = glGetUniformLocation(mainProgram, "u_lightVMatrix");*/
	//lightVMatLoc = glGetUniformLocation(shadowShader, "u_lightVMatrixB")

	//TEST TEMPS****
	//pMat = lightPMat;
	//vMat = lightVMat;

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); //PROJ MAT
	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(vMat));	//VIEW MAT
	//glUniform3fv(viewPosLoc, 1, glm::value_ptr(userCam.GetPosition()));

	//ATTACH ENVIRONMENT MAP
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

	//ATTACH SHADOW MAP
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (Entity* e : spawnedEntities)
	{
		InstallLight(e->GetMaterial());	//LIGHT AND MATERIAL

		//UNIFORM UPDATE
		glUniform3fv(posLoc, 1, glm::value_ptr(e->GetPosition()));		//v
		glUniform3fv(rotLoc, 1, glm::value_ptr(e->GetRotation()));		//TRANSFORM MATS
		glUniform3fv(scaLoc, 1, glm::value_ptr(e->GetScale()));			//^
		glUniformMatrix4fv(lightPMatLoc, 1, GL_FALSE, glm::value_ptr(lightPMat));
		glUniformMatrix4fv(lightVMatLoc, 1, GL_FALSE, glm::value_ptr(lightVMat));
		//glUniformMatrix4fv(lightVMatLoc, 1, GL_FALSE, glm::value_ptr(lightVMat[1]));

		glm::mat4 b = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.5f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.5f, 0.0f,
								0.5f, 0.5f, 0.5f, 1.0f); //BIAS MATRIX
		glm::mat4 mMat = glm::translate(glm::mat4(1), e->GetPosition()) *
			glm::rotate(glm::mat4(1), e->GetRotation().x, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1), e->GetRotation().y, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1), e->GetRotation().z, glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1), e->GetScale());
		glm::mat4 t_shadowMVP2 = b * lightPMat * lightVMat * mMat;
		glm::mat4 t_mvMat = vMat * mMat;
		glm::mat4 t_invTrMat = glm::transpose(glm::inverse(t_mvMat));

		glUniformMatrix4fv(t_mvLoc, 1, GL_FALSE, glm::value_ptr(t_mvMat));
		glUniformMatrix4fv(t_nLoc, 1, GL_FALSE, glm::value_ptr(t_invTrMat));
		glUniformMatrix4fv(t_sLoc, 1, GL_FALSE, glm::value_ptr(t_shadowMVP2));
		
		//VERTEX ATTRIBUTE UPDATE
		glBindBuffer(GL_ARRAY_BUFFER, vbo[cubemapSpace + e->GetBufferIndex()]);		//POSITIONS
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[cubemapSpace + e->GetBufferIndex() + 1]);	//TEXCOORDS
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[cubemapSpace + e->GetBufferIndex() + 2]);	//NORMALS
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		//ATTACH ALBEDO TEXTURE
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, e->GetTextureID());

		glDrawArrays(GL_TRIANGLES, 0, e->GetVertexCount());
	}
}

void InstallLight(Material entityMat)
{
	int positionalLightCount = 0, spotLightCount = 0;
	glm::vec3 viewLightPos;
	string indexStr;
	
	//MULTIPLE LIGHTS INSTALLED
	for (Light* li : spawnedLights)
	{
		switch (li->GetType())
		{
			case (LightType::Directional):{
				indexStr = "u_dirLight";

				currLightDirLoc = glGetUniformLocation(mainProgram, (indexStr + ".direction").c_str());	//DIRECTION

				glProgramUniform3fv(mainProgram, currLightDirLoc, 1, glm::value_ptr(li->GetDirection()));
				break;
			}
				
			case (LightType::Spotlight):{
				indexStr = "u_spotLights[" + to_string(spotLightCount) + "]";
				viewLightPos = glm::vec3(vMat * glm::vec4(li->GetPosition(), 1.0));// convert light’s position to view space

				currLightPosLoc = glGetUniformLocation(mainProgram, (indexStr + ".position").c_str());	//POSITION
				currLightDirLoc = glGetUniformLocation(mainProgram, (indexStr + ".direction").c_str());		//DIRECTION
				currLightCutoffLoc = glGetUniformLocation(mainProgram, (indexStr + ".cutoff").c_str());		//CUTOFF
				currLightExponentLoc = glGetUniformLocation(mainProgram, (indexStr + ".exponent").c_str());	//EXPONENT
				currLightIntensityLoc = glGetUniformLocation(mainProgram, (indexStr + ".intensityFactor").c_str());//INTENSITY

				glProgramUniform3fv(mainProgram, currLightPosLoc, 1, glm::value_ptr(viewLightPos));
				glProgramUniform3fv(mainProgram, currLightDirLoc, 1, glm::value_ptr(li->GetDirection()));
				glProgramUniform1f(mainProgram, currLightCutoffLoc, li->GetSLCutoff());
				glProgramUniform1f(mainProgram, currLightExponentLoc, li->GetSLExponent());
				glProgramUniform1f(mainProgram, currLightIntensityLoc, li->GetSLIntensityFactor());

				spotLightCount++;
				break;
			}
				
			default:{
				indexStr = "u_posLights[" + to_string(positionalLightCount) + "]";
				viewLightPos = glm::vec3(vMat * glm::vec4(li->GetPosition(), 1.0));// convert light’s position to view space

				currLightPosLoc = glGetUniformLocation(mainProgram, (indexStr + ".position").c_str()); //POSITION

				glProgramUniform3fv(mainProgram, currLightPosLoc, 1, glm::value_ptr(viewLightPos));

				positionalLightCount++;
				break;
			}
		}
		
		//EACH LIGHT HAS ADS
		currLightAmbLoc = glGetUniformLocation(mainProgram, (indexStr + ".ambient").c_str());
		currLightDiffLoc = glGetUniformLocation(mainProgram, (indexStr + ".diffuse").c_str());
		currLightSpecLoc = glGetUniformLocation(mainProgram, (indexStr + ".specular").c_str());

		glProgramUniform4fv(mainProgram, currLightAmbLoc, 1, glm::value_ptr(li->GetAmbient()));
		glProgramUniform4fv(mainProgram, currLightDiffLoc, 1, glm::value_ptr(li->GetDiffuse()));
		glProgramUniform4fv(mainProgram, currLightSpecLoc, 1, glm::value_ptr(li->GetSpecular()));
	}

	//POSITIONAL AND SPOTLIGHTS COUNTED FOR LOOPING(LATER)
	posLightCountLoc = glGetUniformLocation(mainProgram, "u_posLightCount");
	spotLightCountLoc = glGetUniformLocation(mainProgram, "u_spotLightCount");

	glProgramUniform1i(mainProgram, posLightCountLoc, positionalLightCount);
	glProgramUniform1i(mainProgram, spotLightCountLoc, spotLightCount);


	// SINGLE MATERIAL AND GLOBAL AMBIENT INSTALLED
	globalAmbLoc = glGetUniformLocation(mainProgram, "u_globalAmbient");
	mAmbLoc = glGetUniformLocation(mainProgram, "u_material.ambient");
	mDiffLoc = glGetUniformLocation(mainProgram, "u_material.diffuse");
	mSpecLoc = glGetUniformLocation(mainProgram, "u_material.specular");
	mShiLoc = glGetUniformLocation(mainProgram, "u_material.shininess");

	glProgramUniform4fv(mainProgram, globalAmbLoc, 1, glm::value_ptr(globalAmbient));
	glProgramUniform4fv(mainProgram, mAmbLoc, 1, glm::value_ptr(entityMat.GetAmbient()));
	glProgramUniform4fv(mainProgram, mDiffLoc, 1, glm::value_ptr(entityMat.GetDiffuse()));
	glProgramUniform4fv(mainProgram, mSpecLoc, 1, glm::value_ptr(entityMat.GetSpecular()));
	glProgramUniform1f(mainProgram, mShiLoc, entityMat.GetShininess());
}

//==========================================================================================================================================
//|                                                                                                                                        |
//|															CALLBACK FUNCTIONS			                                                   |
//|                                                                                                                                        |
//==========================================================================================================================================
void WindowSizeCallback(GLFWwindow* window, int newW, int newH)
{
	glViewport(0, 0, newW, newH);
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
	aspect = (float)windowWidth / (float)windowHeight;
}

//Callback for misc key features
void MiscKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		userCam.SwitchNavigation();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		if (userCam.IsCameraMoving())
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) { userCam.Reset(); }
}
