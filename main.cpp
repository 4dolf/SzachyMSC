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

	int widthbB, heightbB, monColChbB;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytes = stbi_load("bB.png", &widthbB, &heightbB, &monColChbB, 0);

	GLuint texturebB;
	glGenTextures(1, &texturebB);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturebB);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthbB, heightbB, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytes);
	glBindTexture(GL_TEXTURE_2D, 0);



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

		// --- Render first texture (bB.png) ---
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				if (board.get_piece_type(x + (y * 8)) == piece_t::empty)
					continue;
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_bishop)
					glBindTexture(GL_TEXTURE_2D, texturebB); // Bind bB.png texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_king)
					glBindTexture(GL_TEXTURE_2D, texturebK); // Bind bK.png texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_knight)
					glBindTexture(GL_TEXTURE_2D, texturebN); // Bind bN.png texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_pawn)
					glBindTexture(GL_TEXTURE_2D, texturebp); // Bind bp.png texture
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_queen)
					glBindTexture(GL_TEXTURE_2D, texturebQ);
				else if (board.get_piece_type(x + (y * 8)) == piece_t::black_rook)
					glBindTexture(GL_TEXTURE_2D, texturebR);
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_bishop)
					glBindTexture(GL_TEXTURE_2D, texturewB);
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_king)
					glBindTexture(GL_TEXTURE_2D, texturewK);
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_knight)
					glBindTexture(GL_TEXTURE_2D, texturewN);
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_pawn)
					glBindTexture(GL_TEXTURE_2D, texturewp);
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_queen)
					glBindTexture(GL_TEXTURE_2D, texturewQ);
				else if (board.get_piece_type(x + (y * 8)) == piece_t::white_rook)
					glBindTexture(GL_TEXTURE_2D, texturewR);

				VAO VAO1 = genVAO(x, y);
				VAO1.Bind();
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Draw first quad
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

// Funkcja testowa – do sprawdzenia dzia³ania klasy ChessBoard
int test()
{
    // Tworzenie szachownicy
    ChessBoard board;

    // Za³adowanie pozycji startowej (przyk³ad z FEN)
    board.from_fen("rnbqkb1r/pp3p1p/3p2p1/2pP3n/7P/2N2P2/PP2P1P1/R1BQKBNR w KQkq - 1 8");
    board.visualise(); // Wyœwietlenie szachownicy w terminalu

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


// Funkcja gry z botem
void playBot(atomic<char>* fen) {
    ChessBoard board;
    board.from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    move_t move_;
    string move_string;
    string promotion_board = " KPNBRQ  kpnbrq";

    Computer computer;
    computer.set_board(board);

    int promotion = 0;
    board.visualise();

    string fen_string = board.get_fen();
    for (int i = 0; i < fen_string.size(); i++)
        fen[i] = fen_string[i];

    fen[fen_string.size()] = '\n';

    while (true)
    {
        cin >> move_string;
        if (move_string.size() == 5)
            promotion = (int)promotion_board.find(move_string[4]);

        auto piece = board.get_piece_type(((int)move_string[1] - '1') * 8 + ((int)move_string[0] - 'a'));

        move_ = board.encode_move((piece_no_color_t)(((int)piece) % 8),
            ((int)move_string[1] - '1') * 8 + ((int)move_string[0] - 'a'),
            ((int)move_string[3] - '1') * 8 + ((int)move_string[2] - 'a'),
            (piece_t)promotion);

        vector<move_t> moves = board.generate_moves();
        if (find(moves.begin(), moves.end(), move_) != moves.end()) {
            board.move(move_);
            board.visualise();

            fen_string = board.get_fen();
            for (int i = 0; i < fen_string.size(); i++)
                fen[i] = fen_string[i];

            fen[fen_string.size()] = '\n';

            if (board.generate_moves().empty())
            {
                cout << "You win!" << endl;
                return;
            }

            computer.play_move_on_board(move_);

            pair<move_t, int> output = computer.deapening_search(chrono::milliseconds(5000));

            computer.play_move_on_board(output.first);

            cout << "eval: " << output.second << endl;

            board.move(output.first);
            board.visualise();

            fen_string = board.get_fen();
            for (int i = 0; i < fen_string.size(); i++)
                fen[i] = fen_string[i];

            fen[fen_string.size()] = '\n';

            if (board.generate_moves().empty())
            {
                cout << "You lose! hahahhahaha" << endl;
                return;
            }
        }
        else {
            cout << "illigal move" << endl;
        }
    }
}

// Deklaracje nowych funkcji rozgrywki LAN dla trybu turowego
void playTurnBasedLanServer();
void playTurnBasedLanClient();

int main() {
    atomic<char> fen[200];
	string fen_string = "rnbqkb1r/pp3p1p/3p2p1/2pP3n/7P/2N2P2/PP2P1P1/R1BQKBNR w KQkq - 1 8";
	for (int i = 0; i < fen_string.size(); i++)
		fen[i] = fen_string[i];

	fen[fen_string.size()] = '\n';
    thread t1([&]()
        {
            draw_board(fen);
        });

    t1.detach();

    string gameMode;
    cout << "Wybierz tryb gry (bot/lan): ";
    cin >> gameMode;

    if (gameMode == "bot") {
        cout << "Wybrano tryb gry z botem." << endl;
        playBot(fen);
    }
    else if (gameMode == "lan") {
        string netMode;
        cout << "Wybierz tryb polaczenia (server/client): ";
        cin >> netMode;

        if (netMode == "server") {
            cout << "Uruchamiam serwer LAN (tryb turowy)..." << endl;
            playTurnBasedLanServer();
        }
        else if (netMode == "client") {
            cout << "Uruchamiam klienta LAN (tryb turowy)..." << endl;
            playTurnBasedLanClient();
        }
        else {
            cout << "Wybrano niepoprawny tryb po³¹czenia." << endl;
        }
    }
    else {
        cout << "Nieznany tryb gry. Koñczê dzia³anie." << endl;
    }

    return 0;
}
