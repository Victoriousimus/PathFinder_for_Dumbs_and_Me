#include "main.h"
//#define SCALEF 9.0f
//#define SCALET 0.035f
#define SCALEF 1.0f
struct mainGlobalVaribles{
	i_2b mause_X{};
	i_2b mause_Y{};
	i_1b button{};
	Bool update = false;
} GV;
static void MouseCallback(i_4b button, i_4b action, i_4b x, i_4b y) {
	if (!action) return;
	GV.button = button;
	GV.mause_X = static_cast<i_2b>(x);
	GV.mause_Y = static_cast<i_2b>(y);
	GV.update = true;
}
static void BroadCallback(i_4b key, i_4b scancode, i_4b action, i_4b mods) {

}
static void DrawPath(pthfd::CPathArray* path_ptr, i_4b size) {
	if (!path_ptr) return;
	pthfd::CPathArray& path = *path_ptr;
	if (path.Length() < 2) return;
	SPoint2u p1{};
	SPoint2u p2{};
	f_4b p1_X, p1_Y, p2_X, p2_Y;
	glColor3f(1.0f, 0.2f, 0.2f);
	glLineWidth(2.0f);
	glBegin(GL_LINE_STRIP);
	for (u_4b i = 1; i < path.Length(); ++i) {
		p1.x = path[i - 1].x;
		p1.y = path[i - 1].y;
		p2.x = path[i].x;
		p2.y = path[i].y;
		p1_Y = (static_cast<f_4b>(p1.y << 1) / static_cast<f_4b>(size)) - 1.0f;
		p1_X = (static_cast<f_4b>(p1.x << 1) / static_cast<f_4b>(size)) - 1.0f;
		p2_Y = (static_cast<f_4b>(p2.y << 1) / static_cast<f_4b>(size)) - 1.0f;
		p2_X = (static_cast<f_4b>(p2.x << 1) / static_cast<f_4b>(size)) - 1.0f;
		glVertex2f(p1_X, p1_Y);
		glVertex2f(p2_X, p2_Y);
	}
	glEnd();
}

i_4b main(i_4b argc, c_wrd argv[]) {
	CStbImg Img;
	CWindow Win;
	SMap_8b Map;
	i_2b window_size;
	i_2b size;
	SPoint2u start_end_path[2] = {
		{  27, 47 },
		{ 116, 30 }
	};

	size = Img.load("map_smpl_0.png", SCALEF);
	if (size < 1) std::cout << "ERROR_LOAD_IMG";

	window_size = size * (i_2b)SCALEF;

	if (Win.init(window_size, window_size)) {

		Win.setMouseCallback(MouseCallback);
		Win.setBroadCallback(BroadCallback);
		Map.init(Img, size);
		if(size<128) Map.print();

		pthfd::CDumbPather Pth;
		pthfd::CPathArray path_arr;
		Pth.SetByteMap(Map.cell, Map.size_);
		GV.update = false;

		f_8b accum = 0.0f;

		start_end_path[0] = { 46, 284 };
		start_end_path[1] = { 195, 18 };
		for (i_4b i = 0; i < 128; ++i) {
			f_8b find_time = Win.time();
			Pth.Find(start_end_path, &path_arr);
			find_time = Win.time() - find_time;
			accum += find_time;
		}
		std::cout << "FRONT Mid_TIME[" << static_cast<i_4b>(accum) << "]: " << accum / 128.0f << std::endl;

		start_end_path[0] = { 195, 18 };
		start_end_path[1] = { 46, 284 };
		for (i_4b i = 0; i < 128; ++i) {
			f_8b find_time = Win.time();
			Pth.Find(start_end_path, &path_arr);
			find_time = Win.time() - find_time;
			accum += find_time;
		}
		std::cout << "BACK  Mid_TIME[" << static_cast<i_4b>(accum) << "]: " << accum / 128.0f << std::endl;

		while (!Win.window_close()) {
			if (GV.update) {
				GV.update = false;
				if (GV.mause_X >= static_cast<i_2b>(window_size)) GV.mause_X = static_cast<i_2b>(window_size - 1);
				else if (GV.mause_X < static_cast<i_2b>(0)) GV.mause_X = static_cast<i_2b>(0);
				if (GV.mause_Y >= static_cast<i_2b>(window_size)) GV.mause_Y = static_cast<i_2b>(window_size - 1);
				else if (GV.mause_Y < static_cast<i_2b>(0)) GV.mause_Y = static_cast<i_2b>(0);
				GV.mause_Y = static_cast<i_2b>(window_size) - GV.mause_Y;
				if (GV.button < 2) {
					start_end_path[GV.button] = {
						static_cast<u_2b>(static_cast<f_4b>(GV.mause_X) / SCALEF),
						static_cast<u_2b>(static_cast<f_4b>(GV.mause_Y) / SCALEF)
					};
					f_8b t_arr = Win.time();
					Pth.Find(start_end_path, &path_arr);
					if (path_arr.Length() > 0) {
						t_arr = Win.time() - t_arr;
						std::cout << "PathArray: s-e[" << start_end_path[0].x << ";" << start_end_path[0].y << " > " << start_end_path[1].x << ";" << start_end_path[1].y << "] path" <<
							path_arr.Length() << "  [ " << sizeof(void*) + path_arr.Length() * sizeof(SPoint2u) << " Byte ] Time: " << t_arr << std::endl;
					}
					else {
						std::cout << "PathArrayFinding: FAIL!" << std::endl;
					}
				}
				else {
					SPoint2u point = {
						static_cast<u_2b>(static_cast<f_4b>(GV.mause_X) / SCALEF),
						static_cast<u_2b>(static_cast<f_4b>(GV.mause_Y) / SCALEF)
					};
					std::cout
						<< "Mouse = { "
						<< (point.x > 99 ? "" : (point.x > 9 ? " " : "  ")) << point.x << "; "
						<< (point.y > 99 ? "" : (point.y > 9 ? " " : "  ")) << point.y << " } B["
						<< static_cast<i_4b>(GV.button) << "]; CELL[ " << Map.cell[point.x][point.y] << "]"
						<< std::endl;
					Pth.PrintCacheState();
				}
			}
			glPushMatrix();
			Img.draw(-1.0f, -1.0f);
			if(path_arr.Length()>0) DrawPath( &path_arr, size);
			glPopMatrix();
			Win.swap();
		}
	}
	Win.print_descriprion();
	Map.free();
	Win.free();
	Img.free();
	system("pause");
	return 0;
}