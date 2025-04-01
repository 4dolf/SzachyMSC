#include "board.h"
#include "computer.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>


#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <vector>
#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"

#define PNG_SIZE 800
int C;


VAO genVAOBackground() {
	GLfloat vertices[] =
	{ //        COORDINATES        /      COLORS     /   TEXTURE COORDINATES   //
		 -0.666f, -0.666f, 0.0f, 1.0f, 0.0f,  0.0f,  0.0f, 0.0f, // Lower left corner
		 -0.666f, 0.666f, 0.0f, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f, // Upper left corner
		 0.666f, 0.666f, 0.0f, 0.0f, 0.0f,  1.0f,  1.0f, 1.0f, // Upper right corner
		 0.666f, -0.666f, 0.0f, 1.0f, 1.0f,  1.0f,  1.0f, 0.0f  // Lower right corner
	};
	GLuint indices[] =
	{
		0, 2, 1, // Upper triangle
		0, 3, 2	 // Lower triangle
	};

	// Generates Vertex Array Object and binds it
	VAO VAO2;
	VAO2.Bind();

	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO1(vertices, sizeof(vertices));
	// Generates Element Buffer Object and links it to indices
	EBO EBO1(indices, sizeof(indices));

	// Links VBO attributes such as coordinates and colors to VAO
	VAO2.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
	VAO2.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	VAO2.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	VAO2.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();
	return VAO2;
}

VAO genVAO(int x, int y) {
	x -= 4;
	y -= 4;
	GLfloat vertices[] =
	{ //        COORDINATES        /      COLORS     /   TEXTURE COORDINATES   //
		 x / 6.0f, y / 6.0f , 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Lower left corner
		 x / 6.0f, (y + 1) / 6.0f ,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f, // Upper left corner
		 (x + 1) / 6.0f, (y + 1) / 6.0f,  0.0f, 0.0f, 0.0f,  1.0f,  1.0f, 1.0f, // Upper right corner
		 (x + 1) / 6.0f, y / 6.0f, 0.0f, 1.0f, 1.0f,  1.0f,  1.0f, 0.0f  // Lower right corner
	};


	GLuint indices[] =
	{
		0, 2, 1, // Upper triangle
		0, 3, 2	 // Lower triangle
	};

	// Generates Vertex Array Object and binds it
	VAO VAO1;
	VAO1.Bind();

	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO1(vertices, sizeof(vertices));
	// Generates Element Buffer Object and links it to indices
	EBO EBO1(indices, sizeof(indices));

	// Links VBO attributes such as coordinates and colors to VAO
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	return VAO1;

}


// NOT main function
int draw_board(std::atomic<char>* fen)
{
	ChessBoard board;
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
	GLFWwindow* window = glfwCreateWindow(PNG_SIZE, PNG_SIZE, "YoutubeOpenGL", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, PNG_SIZE, PNG_SIZE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Generates Shader object using shaders defualt.vert and default.frag
	Shader shaderProgram("default.vert", "default.frag");

	// Gets ID of uniform called "scale"
	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

	//Texture


	// Load texture dimensions and channels
	int widthbB, heightbB, monColChbB;

	// Set the flag to flip the texture vertically when loaded (because OpenGL expects texture data origin at the bottom-left corner)
	stbi_set_flip_vertically_on_load(true);

	// Load the texture "bB.png" into memory
	unsigned char* bytes = stbi_load("bB.png", &widthbB, &heightbB, &monColChbB, 0);

	// Generate a texture ID for the "bB.png" texture
	GLuint texturebB;
	glGenTextures(1, &texturebB);

	// Activate the texture unit 0 (bind it as the current active texture)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturebB);

	// Set texture filtering for scaling up (magnification) and scaling down (minification)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Nearest-neighbor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Nearest-neighbor filtering

	// Set texture wrapping modes (how the texture is applied when UV coordinates are out of bounds)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat horizontally (S-axis)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat vertically (T-axis)

	// Send the texture data to the GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthbB, heightbB, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

	// Generate mipmaps for the texture (used for performance at different zoom levels)
	glGenerateMipmap(GL_TEXTURE_2D);

	// Free the memory for the texture data as it has been sent to the GPU
	stbi_image_free(bytes);

	// Unbind the texture to prevent accidental modifications
	glBindTexture(GL_TEXTURE_2D, 0);

	//all of the next GL texture units work the same but for different textures

	int widthbK, heightbK, monColChbK;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytesbK = stbi_load("bK.png", &widthbK, &heightbK, &monColChbK, 0);

	GLuint texturebK;
	glGenTextures(1, &texturebK);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturebK);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthbK, heightbK, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytesbK);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytesbK);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthbN, heightbN, monColChbN;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytesbN = stbi_load("bN.png", &widthbN, &heightbN, &monColChbN, 0);

	GLuint texturebN;
	glGenTextures(1, &texturebN);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturebN);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthbN, heightbN, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytesbN);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytesbN);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthbp, heightbp, monColChbp;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytesbp = stbi_load("bp.png", &widthbp, &heightbp, &monColChbp, 0);

	GLuint texturebp;
	glGenTextures(1, &texturebp);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturebp);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthbp, heightbp, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytesbp);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytesbp);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthbQ, heightbQ, monColChbQ;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytesbQ = stbi_load("bQ.png", &widthbQ, &heightbQ, &monColChbQ, 0);

	GLuint texturebQ;
	glGenTextures(1, &texturebQ);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturebQ);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthbQ, heightbQ, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytesbQ);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytesbQ);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthbR, heightbR, monColChbR;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytesbR = stbi_load("bR.png", &widthbR, &heightbR, &monColChbR, 0);

	GLuint texturebR;
	glGenTextures(1, &texturebR);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturebR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthbR, heightbR, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytesbR);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytesbR);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthwB, heightwB, monColChwB;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* byteswB = stbi_load("wB.png", &widthwB, &heightwB, &monColChwB, 0);

	GLuint texturewB;
	glGenTextures(1, &texturewB);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturewB);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthwB, heightwB, 0, GL_RGBA, GL_UNSIGNED_BYTE, byteswB);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(byteswB);
	glBindTexture(GL_TEXTURE_2D, 0);

	int widthwK, heightwK, monColChwK;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* byteswK = stbi_load("wK.png", &widthwK, &heightwK, &monColChwK, 0);

	GLuint texturewK;
	glGenTextures(1, &texturewK);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturewK);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthwK, heightwK, 0, GL_RGBA, GL_UNSIGNED_BYTE, byteswK);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(byteswK);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthwN, heightwN, monColChwN;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* byteswN = stbi_load("wN.png", &widthwN, &heightwN, &monColChwN, 0);

	GLuint texturewN;
	glGenTextures(1, &texturewN);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturewN);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthwN, heightwN, 0, GL_RGBA, GL_UNSIGNED_BYTE, byteswN);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(byteswN);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthwp, heightwp, monColChwp;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* byteswp = stbi_load("wp.png", &widthwp, &heightwp, &monColChwp, 0);

	GLuint texturewp;
	glGenTextures(1, &texturewp);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturewp);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthwp, heightwp, 0, GL_RGBA, GL_UNSIGNED_BYTE, byteswp);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(byteswp);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthwQ, heightwQ, monColChwQ;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* byteswQ = stbi_load("wQ.png", &widthwQ, &heightwQ, &monColChwQ, 0);

	GLuint texturewQ;
	glGenTextures(1, &texturewQ);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturewQ);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthwQ, heightwQ, 0, GL_RGBA, GL_UNSIGNED_BYTE, byteswQ);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(byteswQ);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthwR, heightwR, monColChwR;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* byteswR = stbi_load("wR.png", &widthwR, &heightwR, &monColChwR, 0);

	GLuint texturewR;
	glGenTextures(1, &texturewR);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturewR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthwR, heightwR, 0, GL_RGBA, GL_UNSIGNED_BYTE, byteswR);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(byteswR);
	glBindTexture(GL_TEXTURE_2D, 0);



	int widthCB, heightCB, monColChCB;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytesCB = stbi_load("pngwing1.png", &widthCB, &heightCB, &monColChCB, 0);

	GLuint textureCB;
	glGenTextures(1, &textureCB);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureCB);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthCB, heightCB, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytesCB);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytesCB);
	glBindTexture(GL_TEXTURE_2D, 0);




	GLuint tex0Uni = glGetUniformLocation(shaderProgram.ID, "tex0");
	shaderProgram.Activate();
	glUniform1i(tex0Uni, 0);


	// Main while loop
	while (!glfwWindowShouldClose(window)) {

		std::string fen_string = "";
		for (int i = 0; i < 200; i++)
		{
			if (fen[i] == '\n')
				break;

			fen_string.push_back(fen[i]);
		}


		board.from_fen(fen_string);
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shaderProgram.Activate();

		// Assign scale uniform (you can adjust this if needed)
		glUniform1f(uniID, 0.5f);

		// --- Render background (Chess_Board.png) ---
		glBindTexture(GL_TEXTURE_2D, textureCB); // Bind Chess_Board.png texture
		VAO VAO2 = genVAOBackground();
		VAO2.Bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Draw first quad

		// --- Render textures ---
		for (int x = 0; x < 8; x++) { // Loop through each column of the chessboard
			for (int y = 0; y < 8; y++) { // Loop through each row of the chessboard

				// Check the type of piece at the current position
				// If the square is empty, skip rendering
				if (board.get_piece_type(x + (y * 8)) == piece_t::empty)
					continue;

				// Bind the appropriate texture based on the type of piece
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_bishop)
					glBindTexture(GL_TEXTURE_2D, texturebB); // Bind black bishop texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_king)
					glBindTexture(GL_TEXTURE_2D, texturebK); // Bind black king texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_knight)
					glBindTexture(GL_TEXTURE_2D, texturebN); // Bind black knight texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_pawn)
					glBindTexture(GL_TEXTURE_2D, texturebp); // Bind black pawn texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_queen)
					glBindTexture(GL_TEXTURE_2D, texturebQ); // Bind black queen texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_rook)
					glBindTexture(GL_TEXTURE_2D, texturebR); // Bind black rook texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_bishop)
					glBindTexture(GL_TEXTURE_2D, texturewB); // Bind white bishop texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_king)
					glBindTexture(GL_TEXTURE_2D, texturewK); // Bind white king texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_knight)
					glBindTexture(GL_TEXTURE_2D, texturewN); // Bind white knight texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_pawn)
					glBindTexture(GL_TEXTURE_2D, texturewp); // Bind white pawn texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_queen)
					glBindTexture(GL_TEXTURE_2D, texturewQ); // Bind white queen texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_rook)
					glBindTexture(GL_TEXTURE_2D, texturewR); // Bind white rook texture

				// Generate a Vertex Array Object (VAO) for the current piece position
				// This ensures each piece is correctly positioned on the board
				VAO VAO1 = genVAO(x, y);

				// Bind the VAO to prepare for rendering
				VAO1.Bind();

				// Draw the piece as two triangles forming a quad
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Draw the current quad
			}
		}


		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Poll events (handle keyboard, mouse, etc.)
		glfwPollEvents();
	}




	// Delete all the objects we've created
	glDeleteTextures(1, &texturebB);
	shaderProgram.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}



using namespace std;

// Funkcja testowa – do sprawdzenia działania klasy ChessBoard
int test()
{
    // Tworzenie szachownicy
    ChessBoard board;

    // Załadowanie pozycji startowej (przykład z FEN)
    board.from_fen("rnbqkb1r/pp3p1p/3p2p1/2pP3n/7P/2N2P2/PP2P1P1/R1BQKBNR w KQkq - 1 8");
    board.visualise(); // Wyświetlenie szachownicy w terminalu

    auto chess_move = board.encode_move(piece_no_color_t::pawn, 12, 28);
    board.move(chess_move);
    board.visualise();

    chess_move = board.encode_move(piece_no_color_t::rook, 56, 40);
    board.move(chess_move);
    board.visualise();

    chess_move = board.encode_move(piece_no_color_t::pawn, 8, 16);
    for (auto legal_move : board.generate_moves())
    {
        if (chess_move == legal_move)
            board.move(chess_move);
    }
    board.visualise();

    board.undo_move();
    board.visualise();

    cout << "piece type: " << int(board.get_piece_type(4)) << endl;
    /*
    empty = 0,
    white_king = 1,
    white_pawn = 2,
    white_knight = 3,
    white_bishop = 4,
    white_rook = 5,
    white_queen = 6,
    black_king = 9,
    black_pawn = 10,
    black_knight = 11,
    black_bishop = 12,
    black_rook = 13,
    black_queen = 14,
    */
    return 0;
}


// --- Funkcja gry z botem ---
void playBot(atomic<char>* fen) {
	// Initialize the chessboard with the default starting position using FEN notation
	ChessBoard board;
	board.from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

	// Initialize variables for moves and promotion handling
	move_t move_; // Represents the move being played
	string move_string; // Stores the move entered by the player
	string promotion_board = " KPNBRQ  kpnbrq"; // Defines promotion options for pawns

	// Create a bot (computer player) and set the board state
	Computer computer;
	computer.set_board(board);

	// Promotion flag (used for pawn promotion)
	int promotion = 0;

	// Visualize the initial chessboard state
	board.visualise();

	// Convert the board state to FEN notation for display purposes
	string fen_string = board.get_fen();
	for (int i = 0; i < fen_string.size(); i++)
		fen[i] = fen_string[i];

	// Append the newline character to the FEN string
	fen[fen_string.size()] = '\n';

	// --- Main Game Loop ---
	while (true)
	{
		// Player enters a move string (e.g., e2e4, e7e8q for promotion)
		cin >> move_string;

		// Check if the player entered a promotion move (5 characters, e.g., "e7e8q")
		if (move_string.size() == 5)
			promotion = (int)promotion_board.find(move_string[4]); // Find promotion piece type

		// Get the type of piece from the starting square
		auto piece = board.get_piece_type(((int)move_string[1] - '1') * 8 + ((int)move_string[0] - 'a'));

		// Encode the move (convert it to the board's internal representation)
		move_ = board.encode_move(
			(piece_no_color_t)(((int)piece) % 8), // Remove color information from the piece type
			((int)move_string[1] - '1') * 8 + ((int)move_string[0] - 'a'), // Starting square
			((int)move_string[3] - '1') * 8 + ((int)move_string[2] - 'a'), // Target square
			(piece_t)promotion // Promotion type (if any)
		);

		// Generate all legal moves for the player
		vector<move_t> moves = board.generate_moves();

		// Check if the entered move is legal
		if (find(moves.begin(), moves.end(), move_) != moves.end()) {
			// If the move is valid, apply it to the board
			board.move(move_);
			board.visualise();

			// Update the FEN string for the new board state
			fen_string = board.get_fen();
			for (int i = 0; i < fen_string.size(); i++)
				fen[i] = fen_string[i];

			fen[fen_string.size()] = '\n';

			// Check if the bot has no legal moves (win condition for the player)
			if (board.generate_moves().empty())
			{
				cout << "You win!" << endl;
				return; // Exit the game
			}

			// Bot plays the player's move on its internal representation of the board
			computer.play_move_on_board(move_);

			// Perform bot search for the best move using deepening search with a time constraint
			pair<move_t, int> output = computer.deapening_search(chrono::milliseconds(C)); // C is the difficulty level/time limit

			// Bot applies the chosen move to its internal board representation
			computer.play_move_on_board(output.first);

			// Display evaluation score of the bot's chosen move
			cout << "eval: " << output.second << endl;

			// Apply the bot's move to the actual chessboard
			board.move(output.first);
			board.visualise();

			// Update the FEN string again for the updated state after the bot's move
			fen_string = board.get_fen();
			for (int i = 0; i < fen_string.size(); i++)
				fen[i] = fen_string[i];

			fen[fen_string.size()] = '\n';

			// Check if the player has no legal moves (loss condition for the player)
			if (board.generate_moves().empty())
			{
				cout << "You lose! hahahhahaha" << endl;
				return; // Exit the game
			}
		}
		else {
			// If the move is illegal, display an error message
			cout << "illigal move" << endl;
		}
	}
}

// Deklaracje nowych funkcji rozgrywki LAN dla trybu turowego
void playTurnBasedLanServer(atomic<char>*fen);
void playTurnBasedLanClient(atomic<char>*fen);

int main() {
	// Create an atomic array to store the FEN string representing the current chessboard state
	atomic<char> fen[200];

	// Define the initial FEN string for the standard chess starting position
	string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	// Copy the FEN string into the atomic array `fen`
	for (int i = 0; i < fen_string.size(); i++)
		fen[i] = fen_string[i];

	// Add a newline character to mark the end of the FEN string
	fen[fen_string.size()] = '\n';

	// Create a thread to render the chessboard based on the FEN string
	thread t1([&]()
		{
			draw_board(fen); // Pass the FEN array to the draw_board function
		});

	// Detach the thread so it runs independently of the main program
	t1.detach();

	// Prompt the user to choose a game mode (against a bot or in LAN mode)
	string gameMode;
	cout << "Wybierz tryb gry (bot/lan): ";
	cin >> gameMode;

	// Check if the user selected the "bot" game mode
	if (gameMode == "bot") {
		// Inform the user that bot mode was selected
		cout << "Wybrano tryb gry z botem." << endl;

		// Prompt the user to select the bot's difficulty level
		cout << "Wybierz poziom trudności bota." << endl;
		cout << "Poziomy trudności. 1440, 1620, 1780, 1930, 2060" << endl;

		int liczba44; // Variable to store the selected difficulty level
		cin >> liczba44;

		// Check the selected difficulty level and set the search depth (C) accordingly
		if (liczba44 == 1440) {
			C = 1; // Easy difficulty
			playBot(fen); // Start the game against the bot
		}
		else if (liczba44 == 1620) {
			C = 10; // Medium difficulty
			playBot(fen);
		}
		else if (liczba44 == 1780) {
			C = 100; // Hard difficulty
			playBot(fen);
		}
		else if (liczba44 == 1930) {
			C = 1000; // Expert difficulty
			playBot(fen);
		}
		else if (liczba44 == 2060) {
			C = 5000; // Master difficulty
			playBot(fen);
		}
		else {
			// Handle invalid difficulty levels
			cout << "Nie ma takiego poziomu trudności" << endl;
		}
	}
	// Check if the user selected the LAN game mode
	else if (gameMode == "lan") {
		// Prompt the user to choose server or client mode
		string netMode;
		cout << "Wybierz tryb polaczenia (server/client): ";
		cin >> netMode;

		// Start the server mode for turn-based LAN gameplay
		if (netMode == "server") {
			cout << "Uruchamiam serwer LAN (tryb turowy)..." << endl;
			playTurnBasedLanServer(fen); // Call the server function
		}
		// Start the client mode for turn-based LAN gameplay
		else if (netMode == "client") {
			cout << "Uruchamiam klienta LAN (tryb turowy)..." << endl;
			playTurnBasedLanClient(fen); // Call the client function
		}
		else {
			// Handle invalid connection modes
			cout << "Wybrano niepoprawny tryb połączenia." << endl;
		}
	}
	else {
		// Handle unknown game modes
		cout << "Nieznany tryb gry. Kończę działanie." << endl;
	}

	return 0;
}

