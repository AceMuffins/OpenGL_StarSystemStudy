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

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;
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

	Shader ourShader(".\\shaders\\shader.vs", ".\\shaders\\shader.fs");
	Shader lightShader(".\\shaders\\light.vs", ".\\shaders\\light.fs");
	char modelPath[] = ".\\models\\maxwell\\maxwell.obj";
	char sunPath[] = ".\\models\\planet\\sun.obj";
	Model ourModel(modelPath);
	Model sunModel(sunPath);

	//arrays for colors, vertices and indices
	float colorBackground[4] = { 0.2f, 0.2f, 0.2f, 1.0f };



	//textures
	unsigned int diffuseMap;
	glGenTextures(1, &diffuseMap);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(".\\textures\\cat1.png", &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	
	unsigned int specularMap;
	glGenTextures(1, &specularMap);
	glBindTexture(GL_TEXTURE_2D, specularMap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);

	data = stbi_load(".\\textures\\cat1_spec.png", &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	


	ourShader.use();
	ourShader.setInt("material.diffuse", 0);
	ourShader.setInt("material.specular", 1);

	bool rotate = false;
	bool warp = false;
	bool flip = false;
	float angle = 0;
	float angular_speed = 0;
	//main render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glfwGetWindowSize(window, &width, &height);

		//clearing screen
		glClearColor(colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);


		ourShader.use();
		ourShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
		ourShader.setFloat("material.shininess", 32.0f);
		ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		ourShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f); // darkened
		ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		ourShader.setFloat("light.constant", 1.0f);
		ourShader.setFloat("light.linear", 0.027f);
		ourShader.setFloat("light.quadratic", 0.0028f);

		// pass projection matrix to shader (note that in this case it could change every frame)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);


		glm::vec3 lightPos(4.0f, 4.0f, 4.0f);


		glm::mat4 model = glm::mat4(1.0f);
		ourShader.use();
		ourShader.setVec3("lightPos", lightPos);
		ourShader.setVec3("viewPos", camera.Position);
		model = glm::mat4(1.0f);
		glm::mat3 normal = glm::mat3(1.0f);
		
		model = glm::mat4(1.0f);
		angular_speed = deltaTime * 60.f;
		if (angle > 15.0f) {
			flip = true;
		}
		if(angle < -15.0f){
			flip = false;
		}
		if (flip) {
			angle = angle - angular_speed;
		} else {
			angle = angle + angular_speed;
		}
		/* easing - kinda jank
		float x = angle / 15.f;
		float easing = -(cos(3.14159f * x) - 1.f) / 2.f;
		float ease_angle = angle * easing;
		*/
		if (angle > 0.0f) {
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
		}
		else {
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		/*
		float angle = 20.0f * currentFrame;
		float speed = 10.0f * currentFrame;
		model = glm::rotate(model, glm::radians(speed), glm::vec3(0.0f, 1.0f, 0.0f)); //orbit
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f)); //orbit distance
		//model = glm::rotate(model, glm::radians(26.73f), glm::vec3(1.0f, 0.0f, 0.0f)); //axial tilt
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); //spin
		*/

		normal = glm::mat3(glm::transpose(glm::inverse(model)));
		ourShader.setMat4("model", model);
		ourShader.setMat3("transNormal", normal);
		ourModel.Draw(ourShader);

		lightShader.use();
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
		lightShader.setMat4("model", model);

		sunModel.Draw(lightShader);

		/*
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug menu");
		//ImGui::Text((std::to_string(1000.0 / deltaTime)).c_str());
		//ImGui::ColorEdit3("BG Color", colorBackground);
		ImGui::Checkbox("Rotation", &rotate);
		ImGui::Checkbox("Warp", &warp);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		*/

		//poll events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//release all GLFW resources and destroy ImGUI menus
	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext();

	glfwTerminate();
	return 0;
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