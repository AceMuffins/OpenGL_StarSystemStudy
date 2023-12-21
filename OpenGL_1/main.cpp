#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "stb_image.h"
#include "camera.h"
#include "model.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
unsigned int loadCubemap(vector<std::string> faces);
void drawStar(float currentFrame, Shader& objShader, Model& objModel, glm::vec3& pos, float scale, float max_scale, bool outline);
void drawDevourer(float currentFrame, Shader &objShader, Model &objModel, bool spin, bool outline);
void movePlanet(glm::mat4 &model, float currentFrame, float orbitSpeed, float orbitDistance, float spinSpeed);
void moveMoon(glm::mat4& model, float currentFrame, float orbitSpeed, float orbitDistance, float spinSpeed);
void moveShip(glm::mat4& model, float currentFrame, float orbitSpeed, float orbitDistance, float tilt);
void drawModel(glm::mat4& model, Shader& objShader, Model& objModel, bool outline);

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;
const float PI = 3.1415926f;
int width, height;

//Camera stuff
Camera camera(glm::vec3(0.0f, 3.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

bool shift = false;
bool press, prev_press;
bool toggle = false;
bool flip = false;
bool devourer = false;
bool lightscreen = false;
bool spin = false;
bool global_outline = false;
float angle = 0;
float angular_speed = 0;
int cat_cnt = 1;

float earthDistance = -3.0f;
float earthSpeed = -30.0f;
float earthSpin = -190.0f;
float moonDistance = -0.5f;
float moonSpeed = -90.0f;
float shipDistance = 0.1f;
float shipSpeed = 50.0f;
float shipTilt = -30.0f;
float saturnDistance = 4.5f;
float saturnSpeed = -30.0f;
float saturnSpin = -190.0f;
float saturnTilt = -10.f;
bool stopEarth = false;
bool stopMoon = false;
bool stopShip = false;
bool multiTrackDrifting = false;


int main()
{
	//init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create a window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL :3", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//load GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//load ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//Start skybox
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	vector<std::string> faces
	{
		".\\models\\cubemap\\right.png",
		".\\models\\cubemap\\left.png",
		".\\models\\cubemap\\top.png",
		".\\models\\cubemap\\bottom.png",
		".\\models\\cubemap\\front.png",
		".\\models\\cubemap\\back.png"
	};
	unsigned int cubemapTexture = loadCubemap(faces);
	//End skybox


	Shader objShader(".\\shaders\\shader.vs", ".\\shaders\\shader.fs");
	Shader lightShader(".\\shaders\\light.vs", ".\\shaders\\light.fs");
	Shader skyboxShader(".\\shaders\\skybox.vs", ".\\shaders\\skybox.fs");
	Shader outlineShader(".\\shaders\\shader.vs", ".\\shaders\\outline.fs");

	char catPath[] = ".\\models\\maxwell\\maxwell.obj";
	char starBluePath[] = ".\\models\\star\\star_blue.obj";
	char starOrangePath[] = ".\\models\\star\\star_orange.obj";
	char earthPath[] = ".\\models\\earth\\earth.obj";
	char moonPath[] = ".\\models\\moon\\moon.obj";
	char cubePath[] = ".\\models\\cube\\cube.obj";
	char shipPath[] = ".\\models\\enterprise\\enterprise.obj";
	char saturnPath[] = ".\\models\\saturn\\saturn.obj";
	char ringPath[] = ".\\models\\saturn\\rings.obj";
	char ringsOutPath[] = ".\\models\\saturn\\rings_outline.obj";

	Model catModel(catPath);
	Model starBlueModel(starBluePath);
	Model starOrangeModel(starOrangePath);
	Model earthModel(earthPath);
	Model moonModel(moonPath); //to Elsweyr
	Model cubeModel(cubePath);
	Model shipModel(shipPath); //engage
	Model saturnModel(saturnPath);
	Model ringsModel(ringPath);
	Model ringsOutModel(ringsOutPath);

	//arrays for colors, vertices and indices
	float colorBackground[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

	glm::vec3 pointLightPositions[] = {
		glm::vec3(-0.3f, 0.0f, 0.0f), //change to 10 :3
		glm::vec3(0.7f, 0.0f, 0.0f)
	};
	glm::vec3 pointLightColors[] = {
		glm::vec3(92.f / 255.f, 180.f / 255.f, 255.f / 255.f),
		glm::vec3(255.f / 255.f, 172.f / 255.f, 61.f / 255.f)
	};

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	//main render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		glfwGetWindowSize(window, &width, &height);

		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		//clearing screen
		//glClearColor(colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


		// pass projection matrix to shader (note that in this case it could change every frame)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightShader.use();
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);

		outlineShader.use();
		outlineShader.setMat4("projection", projection);
		outlineShader.setMat4("view", view);

		objShader.use();
		objShader.setMat4("projection", projection);
		objShader.setMat4("view", view);
		objShader.setVec3("viewPos", camera.Position);
		objShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
		objShader.setFloat("material.shininess", 32.0f);
		for (int i = 0; i < 2; i++) {
			objShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions[i]);
			objShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", pointLightColors[i] * 0.05f);
			objShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", pointLightColors[i]);
			objShader.setVec3("pointLights[" + std::to_string(i) + "].specular", pointLightColors[i]);
			objShader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
			objShader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.0014f);
			objShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.000007f);
		}

		glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments pass the stencil test
		glStencilMask(0xFF); // enable writing to the stencil buffer

		//Drawing scene

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat3 normal = glm::mat3(1.0f);
		glm::mat4 rotate = glm::mat4(1.0f);
		rotate = glm::rotate(rotate, glm::radians(deltaTime * (10.f + (multiTrackDrifting * 600.f))), glm::vec3(0, 1, 0));
		pointLightPositions[0] = pointLightPositions[0] * glm::mat3(rotate);
		pointLightPositions[1] = pointLightPositions[1] * glm::mat3(rotate);

		drawStar(currentFrame, lightShader, starBlueModel, pointLightPositions[0], 0.5f, 0.5f, false);
		drawStar(currentFrame, lightShader, starOrangeModel, pointLightPositions[1], 0.2f, 0.5f, false);

		if (lightscreen) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.f, 0.f, -2.f));
			model = glm::scale(model, glm::vec3(1.f, 0.2f, 0.2f));
			drawModel(model, objShader, cubeModel, false);
		}

		if (devourer) {
			drawDevourer(currentFrame, objShader, catModel, spin, false);
		}

		earthSpeed = -30.0f;
		moonSpeed = -90.0f;
		shipSpeed = 50.0f;
		if (stopEarth) {
			earthSpeed = 0.0f;
		}
		if (stopMoon) {
			moonSpeed = 0.0f;
		}
		if (stopShip) {
			shipSpeed = 0.0f;
		}

		model = glm::mat4(1.0f);
		movePlanet(model, currentFrame, earthSpeed, earthDistance, earthSpin);
		model = glm::scale(model, glm::vec3(0.2f));
		drawModel(model, objShader, earthModel, false);

		model = glm::mat4(1.0f);
		movePlanet(model, currentFrame, earthSpeed, earthDistance, earthSpin);
		moveMoon(model, currentFrame, moonSpeed, moonDistance, earthSpin);
		model = glm::scale(model, glm::vec3(0.05));
		drawModel(model, objShader, moonModel, false);

		model = glm::mat4(1.0f);
		movePlanet(model, currentFrame, earthSpeed, earthDistance, earthSpin);
		moveMoon(model, currentFrame, moonSpeed, moonDistance, earthSpin);
		moveShip(model, currentFrame, shipSpeed, shipDistance, shipTilt);
		model = glm::scale(model, glm::vec3(0.02));
		drawModel(model, objShader, shipModel, false);

		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(currentFrame * saturnSpeed), glm::vec3(0, 1.f, 0));
		model = glm::translate(model, glm::vec3(0.f, 0.f, saturnDistance));
		model = glm::rotate(model, glm::radians(saturnTilt), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(currentFrame * saturnSpin), glm::vec3(0, 1.f, 0));
		model = glm::scale(model, glm::vec3(0.3f));
		drawModel(model, objShader, saturnModel, false);
		drawModel(model, objShader, ringsModel, false);

		// Drawing Skybox
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		glDisable(GL_STENCIL_TEST);
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default
		glEnable(GL_STENCIL_TEST);
		
		if (global_outline) {
			//draw outline
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);
			glDisable(GL_DEPTH_TEST);
			drawStar(currentFrame, outlineShader, starBlueModel, pointLightPositions[0], 0.5f, 0.5f, true);
			drawStar(currentFrame, outlineShader, starOrangeModel, pointLightPositions[1], 0.2f, 0.5f, true);

			model = glm::mat4(1.0f);
			movePlanet(model, currentFrame, earthSpeed, earthDistance, earthSpin);
			model = glm::scale(model, glm::vec3(0.2f));
			drawModel(model, outlineShader, earthModel, true);

			model = glm::mat4(1.0f);
			movePlanet(model, currentFrame, earthSpeed, earthDistance, earthSpin);
			moveMoon(model, currentFrame, moonSpeed, moonDistance, earthSpin);
			model = glm::scale(model, glm::vec3(0.05));
			drawModel(model, outlineShader, moonModel, true);

			model = glm::mat4(1.0f);
			movePlanet(model, currentFrame, earthSpeed, earthDistance, earthSpin);
			moveMoon(model, currentFrame, moonSpeed, moonDistance, earthSpin);
			moveShip(model, currentFrame, shipSpeed, shipDistance, shipTilt);
			model = glm::scale(model, glm::vec3(0.02));
			drawModel(model, outlineShader, shipModel, true);

			objShader.use();
			model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(currentFrame * saturnSpeed), glm::vec3(0, 1.f, 0));
			model = glm::translate(model, glm::vec3(0.f, 0.f, saturnDistance));
			model = glm::rotate(model, glm::radians(saturnTilt), glm::vec3(1, 0, 0));
			model = glm::rotate(model, glm::radians(currentFrame * saturnSpin), glm::vec3(0, 1.f, 0));
			model = glm::scale(model, glm::vec3(0.3f));
			model = glm::scale(model, glm::vec3(1.1f));
			drawModel(model, outlineShader, saturnModel, true);
			model = glm::scale(model, glm::vec3(0.9f));
			drawModel(model, outlineShader, ringsOutModel, false);

			if (devourer) {
				drawDevourer(currentFrame, outlineShader, catModel, spin, true);
			}
			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glEnable(GL_DEPTH_TEST);
		}
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Universe control panel");
		//ImGui::Text((std::to_string(1000.0 / deltaTime)).c_str());
		//ImGui::ColorEdit3("BG Color", colorBackground);
		ImGui::Checkbox("Outline", &global_outline);
		ImGui::Checkbox("Devourer", &devourer);
		if (devourer) {
			ImGui::SliderInt("Devourers", &cat_cnt, 1, 10);
			ImGui::Checkbox("Spin", &spin);
		}
		ImGui::Checkbox("Lightscreen", &lightscreen);
		ImGui::Checkbox("Star merger mode", &multiTrackDrifting);
		ImGui::Checkbox("Stop Earth", &stopEarth);
		ImGui::Checkbox("Stop Moon", &stopMoon);
		ImGui::Checkbox("Stop Ship", &stopShip);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		

		//poll events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//release all GLFW resources and destroy ImGUI menus
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glfwTerminate();
	return 0;
}

void drawModel(glm::mat4& model, Shader& objShader, Model& objModel, bool outline) {
	objShader.use();
	if (outline) {
		model = glm::scale(model, glm::vec3(1.1f));
	}
	glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(model)));
	objShader.setMat4("model", model);
	objShader.setMat3("transNormal", normal);
	objModel.Draw(objShader);
}

void drawStar(float currentFrame, Shader& objShader, Model& objModel, glm::vec3& pos, float scale, float max_scale, bool outline) {
	objShader.use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::rotate(model, glm::radians(currentFrame * -10.f), glm::vec3(0, 1.f, 0));
	model = glm::scale(model, glm::vec3(scale));
	if (outline) {
		if (max_scale != scale) {
			//calculating the distance of the offset
			//see whiteboard
			float x = (scale + (max_scale * 1.1f - max_scale)) / scale;
			model = glm::scale(model, glm::vec3(x));

		} else {
			model = glm::scale(model, glm::vec3(1.1f));
		}
	}
	objShader.setMat4("model", model);
	objModel.Draw(objShader);
}

void movePlanet(glm::mat4& model, float currentFrame, float orbitSpeed, float orbitDistance, float spinSpeed) {
	model = glm::rotate(model, glm::radians(currentFrame * orbitSpeed), glm::vec3(0, 1.f, 0));
	model = glm::translate(model, glm::vec3(0.f, 0.f, orbitDistance));
	model = glm::rotate(model, glm::radians(currentFrame * spinSpeed), glm::vec3(0, 1.f, 0));
}

void moveMoon(glm::mat4& model, float currentFrame, float orbitSpeed, float orbitDistance, float spinSpeed) {
	model = glm::rotate(model, glm::radians(currentFrame * -spinSpeed), glm::vec3(0, 1.f, 0));
	model = glm::rotate(model, glm::radians(currentFrame * orbitSpeed), glm::vec3(0, 1.f, 0));
	model = glm::translate(model, glm::vec3(0.f, 0.f, -.5f));
	model = glm::rotate(model, glm::radians(-90.f), glm::vec3(0, 1.f, 0));
}

void moveShip(glm::mat4& model, float currentFrame, float orbitSpeed, float orbitDistance, float tilt) {
	model = glm::rotate(model, glm::radians(tilt), glm::vec3(0.f, 0.f, 1.f));
	model = glm::rotate(model, glm::radians(currentFrame * orbitSpeed), glm::vec3(1.f, 0.f, 0.f));
	model = glm::translate(model, glm::vec3(0.f, orbitDistance, 0.f));
}

void drawDevourer(float currentFrame, Shader &objShader, Model &objModel, bool spin, bool outline) {
	objShader.use();
	if (!spin) {
		angular_speed = deltaTime * 60.f;
		if (angle > 15.f) {
			flip = true;
		}
		if (angle < -15.f) {
			flip = false;
		}
		if (!outline) {
			if (flip) {
				angle = angle - angular_speed;
			}
			else {
				angle = angle + angular_speed;
			}
		}
		for (int i = 0; i < cat_cnt; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(currentFrame * -20.f) + 2 * PI * i / cat_cnt, glm::vec3(0, 1.f, 0));
			model = glm::translate(model, glm::vec3(0.f, -0.2f, -2.0f));
			model = glm::rotate(model, glm::radians(-20.f), glm::vec3(0, 1.f, 0));
			model = glm::scale(model, glm::vec3(0.2));
			if (outline) {
				model = glm::translate(model, glm::vec3(0.f, -0.1f, 0.0f));
				model = glm::scale(model, glm::vec3(1.1f));
			}

			if (angle > 0.0f) {
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, 0.f, 0.f));
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
			}
			else {
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
				model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, 0.f, 0.f));
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
			}
			glm::mat3 normal = glm::mat3(1.0f);
			normal = glm::mat3(glm::transpose(glm::inverse(model)));
			objShader.setMat4("model", model);
			objShader.setMat3("transNormal", normal);
			objModel.Draw(objShader);
		}
	} else {
		for (int i = 0; i < cat_cnt; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(currentFrame * -20.f) + 2 * PI * i / cat_cnt, glm::vec3(0, 1.f, 0));
			model = glm::translate(model, glm::vec3(0.f, -0.2f, -2.0f));
			model = glm::rotate(model, glm::radians(currentFrame * -800.f), glm::vec3(0, 1.f, 0));
			model = glm::translate(model, glm::vec3(0.f, glm::sin(currentFrame*10.f)*0.2f, 0.f));
			model = glm::scale(model, glm::vec3(0.2));
			if (outline) {
				model = glm::translate(model, glm::vec3(0.f, -0.1f, 0.0f));
				model = glm::scale(model, glm::vec3(1.1f));
			}
			
			glm::mat3 normal = glm::mat3(1.0f);
			normal = glm::mat3(glm::transpose(glm::inverse(model)));
			objShader.setMat4("model", model);
			objShader.setMat3("transNormal", normal);
			objModel.Draw(objShader);
		}
	}
	
}

//provess user input
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	prev_press = press;
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		press = true; 

	if (prev_press != press) {
		toggle = !toggle;
	}

	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
		press = false;
	
	if (toggle && camera.Active == true) {
		camera.Active = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	} else if (!toggle && camera.Active == false) {
		camera.Active = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	shift = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		shift = true;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime, shift);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime, shift);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime, shift);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime, shift);
}

//callback function for when the size of the window is changed
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap failed to load at path: " << faces[i]
				<< std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}