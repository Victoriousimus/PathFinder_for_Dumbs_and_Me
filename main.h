#pragma once
#pragma comment( lib, "opengl32.lib" )  //Добавить в линковкщик либу openGL
#pragma comment( lib, "glfw3.lib" )     //Добавить в линковкщик либу GLfw

#define STBI_ONLY_PNG				// Включить только формат PNG
#define GRINLIZ_NO_STL
#define STB_IMAGE_IMPLEMENTATION	// Подключение библиотеки STB_IMAGE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define DUMBPATHER_TYPES

#include <iostream>
#include <windows.h>
#include <stdint.h>
#include <cstdlib>
#include <cmath>
#include <gl/GL.h>
#include <STB/stb_image.h>
#include <GLFW/glfw3.h>
#include "dumbpather.h"

class CStbImg {
private:
	u_1b* data_;
	i_4b x_;
	i_4b y_;
	f_4b s_;
	Bool empty_;
public:
	CStbImg() : empty_(true), data_(nullptr), x_(0), y_(0), s_(0) {}
	~CStbImg() { free(); }
	void free() {
		if (empty_) return;
		empty_ = true; x_ = 0; y_ = 0;
		stbi_image_free(data_);
	}
	i_4b load(c_wrd fname, f_4b scale) {
		if (!empty_) return -1; i_4b n;
		data_ = stbi_load(fname, &x_, &y_, &n, 3);
		if (!data_) return -1;
		s_ = scale;
		empty_ = false;
		return x_;
	}
	void draw(f_4b posX, f_4b posY) {
		glRasterPos2f(posX, posY);
		glPixelZoom(s_, s_);
		glDrawPixels(x_, y_, GL_RGB, GL_UNSIGNED_BYTE, data_);
		glPixelZoom(1, 1);
	}
	void getPix(i_2b X, i_2b Y, u_4b* color) {
		if (X >= x_ || X < 0 || Y >= y_ || Y < 0) {
			*color = 0;
			return;
		}
		*color =
			((u_4b)(data_[(Y * y_ + X) * 3 + 0]) * 1000000) +
			((u_4b)(data_[(Y * y_ + X) * 3 + 1]) * 1000) +
			((u_4b)(data_[(Y * y_ + X) * 3 + 2]));
	}
	void getPix(i_2b X, i_2b Y, u_1b(*color)[3]) {
		if (X >= x_ || X < 0 || Y >= y_ || Y < 0) {
			(*color)[2] = 0;
			(*color)[1] = 0;
			(*color)[0] = 0;
			return;
		}
		std::memcpy(*color, &(data_[(Y * y_ + X) * 3]), 3);
	}
	i_4b setPix(i_2b X, i_2b Y, u_1b(&color)[3]) {
		if (X >= x_ || X < 0 || Y >= y_ || Y < 0) return -1;
		std::memcpy(&(data_[(Y * y_ + X) * 3]), color, 3);
		return 0;
	}
};

class CWindow {
private:
	GLFWwindow* window_ptr{};
	i_4b error_code{0};
	static void (*user_mouse_callback)(i_4b, i_4b, i_4b, i_4b);  // Статический указатель на пользовательский колбэк
	static void (*user_broad_callback)(i_4b, i_4b, i_4b, i_4b);  // Статический указатель на пользовательский колбэк

	static void internal_mouse_callback(GLFWwindow* window, i_4b button, i_4b action, i_4b mods) {
		if (user_mouse_callback) {
			f_8b x, y;
			glfwGetCursorPos(window, &x, &y);
			user_mouse_callback(button, action, (i_4b)x, (i_4b)y);
		}
	}
	static void internal_broad_callback(GLFWwindow* window, i_4b key, i_4b scancode, i_4b action, i_4b mods) {
		if (user_broad_callback) {
			user_broad_callback(key, scancode, action, mods);
		}
	}

public :
	CWindow() {}
	~CWindow() { if(window_ptr) free(); }
	u_1b init(i_4b width, i_4b height) {
		if (!glfwInit()) { error_code = -1; return 0; }
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);						// Версия OpenGL 
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);						// Версия OpenGL
		glfwWindowHint(GLFW_DECORATED, GL_TRUE);	// Рамка и кнопки окна
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);	// Возможность ресайза окна
		glfwWindowHint(GLFW_FOCUSED, GL_FALSE);	// Фокус на окне
		window_ptr = glfwCreateWindow(width, height, "Window", NULL, NULL);
 		if (!window_ptr) { error_code = -2; return 0; }
		glfwMakeContextCurrent(window_ptr);
		glfwSwapInterval(1); // Вертикальная синхронизация
		return 1;
	}
	void free() {
		glfwDestroyWindow(window_ptr);
		glfwTerminate();
	}
	void swap() {
		glfwSwapBuffers(window_ptr);
		glfwPollEvents();
	}
	f_8b time() {
		return glfwGetTime();
	}
	i_1b window_close() {
		return glfwWindowShouldClose(window_ptr);
	}
	void print_descriprion() {
		if (error_code < 0) {
			c_wrd desc{};
			glfwGetError(&desc);
			if (error_code == -1) std::cout << "FAILED INIT GLFW CONTEXT:" << std::endl;
			if (error_code == -2) std::cout << "ERROR BILD WINDOW OBJECT:" << std::endl;
			std::cout << desc << std::endl;
		} else {
			std::cout << "\nWINDOW CORRECT\n" << std::endl;
		}
	}

	void setMouseCallback(void (*callback_function)(i_4b button, i_4b action, i_4b x, i_4b y)) {
		user_mouse_callback = callback_function;
		glfwSetMouseButtonCallback(window_ptr, internal_mouse_callback);
	}
	void setBroadCallback(void (*callback_function)(i_4b key, i_4b scancode, i_4b action, i_4b mods)) {
		user_broad_callback = callback_function;
		glfwSetKeyCallback(window_ptr, internal_broad_callback);
	}

};

struct SMap_8b {
	i_1b** cell;
	u_2b size_;
	SMap_8b() :size_(0), cell(nullptr) {}
	~SMap_8b() { free(); }
	void init(CStbImg& image, i_4b size) {
		i_2b x = 0;
		i_2b y = 0;
		u_1b color[3];
		size_ = size;
		cell = new i_1b * [size];
		cell[y] = new i_1b[size];
		while (true) {
			if (x >= size) {
				x = 0; ++y;
				if (y >= size) break;
				cell[y] = new i_1b[size];
			}
			image.getPix(x, y, &color);
			std::memset(&cell[y][x], pthfd::TerrainType::WALKABLE, sizeof(cell[y][x]));
			if (color[0] > 8) cell[y][x] = pthfd::TerrainType::BLOKABLE;
			++x;
		}
	}
	void free() {
		if (!size_) return;
		for (i_4b x = 0; x < size_; ++x) { delete[] cell[x]; }
		delete[] cell;
		size_ = 0;
	}
	void print() {
		std::cout << std::endl;
		for (i_4b x, y = size_ - 1; y > -1; --y) {
			for (x = 0; x < size_; ++x) { std::cout << " " << cell[y][x] << " "; }
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
};

//*****************************************************************************************
void (*CWindow::user_mouse_callback)(i_4b, i_4b, i_4b, i_4b) = nullptr;
void (*CWindow::user_broad_callback)(i_4b, i_4b, i_4b, i_4b) = nullptr;
//*****************************************************************************************
