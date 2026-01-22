#include <ftxui/dom/canvas.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>

#include <chrono>
#include <vector>
#include <utility>
#include <cmath>

using namespace std;
using namespace ftxui;
using namespace chrono;

float screenX = 200.0f, screenY = 200.0f;

pair<int, int> Rotate(float x, float y, float cx, float cy, float theta){
	float s = sin(theta);
	float c = cos(theta);

	x -= cx;
	y -= cy;

	float nx = x * c - y * s;
	float ny = x * s + y * c;

	return {nx + cx, ny + cy};
}

struct Point{
	float x, y;
};

vector<Point> MakeRectangle(float cx, float cy, float w, float h){
	return {
		{cx - w / 2, cy - h / 2},
		{cx + w / 2, cy - h / 2},
		{cx + w / 2, cy + h / 2},
		{cx - w / 2, cy + h /2 }
	};
}

Element RotateRectangle(float angle){
	Canvas c(screenX, screenY);

	float cx = screenX / 2.0f;
	float cy = screenY / 2.0f;

	auto rectangle = MakeRectangle(cx, cy, 80, 80);

	vector<Point> r;
	for(auto p: rectangle){
		auto [x, y] = Rotate(p.x, p.y, cx, cy, angle);
		r.push_back({(float)x, (float)y});
	}

	c.DrawPointLine(r[0].x, r[0].y, r[1].x, r[1].y);
    c.DrawPointLine(r[1].x, r[1].y, r[2].x, r[2].y);
    c.DrawPointLine(r[2].x, r[2].y, r[3].x, r[3].y);
    c.DrawPointLine(r[3].x, r[3].y, r[0].x, r[0].y);

	return canvas(std::move(c)) | center;
}

int main(){
	ScreenInteractive screen = ScreenInteractive::Fullscreen();

	float angle = 0.0f; 
	float omega = 0.0f;       
	float alpha = 0.0f; 
	const float accel = 6.0f;  
	const float damping = 0.98f;
	const float dt = 1.0f / 30.0f;
	
	auto renderer = Renderer([&] {
		return vbox({
			text("ctrl c/esc to quit\n"),
			text("accel with arrow left/right and break with arrowdown"),
			RotateRectangle(angle)});
	});

	auto app = CatchEvent(renderer, [&](Event e) {
		if (e == Event::ArrowRight){
			alpha = +accel;
			return true;
		}

		if (e == Event::ArrowLeft){
			alpha = -accel;
			return true;
		}

		if(e == Event::ArrowDown){
			omega *= 0.8f;
			return true;
		}

		if(e == Event::Escape){
			screen.Exit();
			return true;
		}

		return false;
	});
	
	atomic<bool> running = true;

	thread ticker([&] {
		while (running) {
			this_thread::sleep_for(milliseconds(33)); 

			omega += alpha * dt;
			angle += omega * dt;
			omega *= damping;
			alpha = 0.0f;
			screen.PostEvent(Event::Custom);
		}
	});

	screen.Loop(app);

	running = false;
	ticker.join();
	
	return 0;
}


