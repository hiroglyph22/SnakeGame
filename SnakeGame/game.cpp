#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <sstream>
using namespace std;

#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

// TODO
// 1. Make Arena - check
// 2. Collisions - check
// 3. Grid for movement - check
// 4. Food - check
// 5. Increase length - check
// 6. Score - check
// 7. Stopping from collision or hitting itself - check
// 8. fixing the issue of body spawning in the middle

float arena_half_size_x = 46, arena_half_size_y = 46;
float player_half_size_x = 2, player_half_size_y = 2;
float ppos_x, ppos_y;
float speed_x, speed_y;
float elapsed_time;
float speed = 0.2;

int score;
int prev_score;
vector<int> prev_pos_x, prev_pos_y;

bool food_there = true;

ostringstream ss;

static float
give_random_location() {
	return (rand() % ((int)arena_half_size_x - 8) * 2 / 4 - ((int)arena_half_size_x - 4) / 4) * 4;
}

float food_x = give_random_location();
float food_y = give_random_location();

static void
simulate_game(Input* input, float dt) {

	draw_rect(0, 0, 100, 100, 0x000000);
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0x555555);

	if (pressed(BUTTON_UP)) {
		speed_y = 4;
		speed_x = .0f;
	}
	if (pressed(BUTTON_DOWN)) {
		speed_y = -4;
		speed_x = .0f;
	}
	if (pressed(BUTTON_RIGHT)) {
		speed_x = 4;
		speed_y = .0f;
	}
	if (pressed(BUTTON_LEFT)) {
		speed_x = -4;
		speed_y = .0f;
	}

	// Collision
	// Top Wall
	if (ppos_y + player_half_size_y >= arena_half_size_y) {
		ppos_y = arena_half_size_y - player_half_size_y;
		speed_y = 0;
		speed_x = 0;
	}
	// Bottom Wall
	else if (ppos_y - player_half_size_y <= -arena_half_size_y) {
		ppos_y = -arena_half_size_y + player_half_size_y;
		speed_y = 0;
		speed_x = 0;
	}
	// Right Wall
	else if (ppos_x + player_half_size_x >= arena_half_size_x) {
		ppos_x = arena_half_size_x - player_half_size_x;
		speed_x = 0;
		speed_y = 0;
	}
	// Left Wall
	else if (ppos_x - player_half_size_x <= -arena_half_size_x) {
		ppos_x = -arena_half_size_x + player_half_size_x;
		speed_x = 0;
		speed_y = 0;
	}
	// Hitting the food
	else if (ppos_x == food_x && ppos_y == food_y) {
		score++;
		speed *= 0.98;
		prev_pos_x.resize(score);
		prev_pos_y.resize(score);
		prev_pos_x[score - 1] = -60;
		prev_pos_y[score - 1] = -60;
		food_x = give_random_location();
		food_y = give_random_location();
		for (int i = 0; i < score; i++) {
			if (food_x == prev_pos_x[i] && food_y == prev_pos_y[i]) {
				food_x = give_random_location();
				food_y = give_random_location();
			}
		}
	}
	// Hitting itself
	for (int i = 0; i < score; i++) {
		if (ppos_x == prev_pos_x[i] && ppos_y == prev_pos_y[i]) {
			speed_x = 0;
			speed_y = 0;
		}
	}

	// Player head movement
	elapsed_time += dt;
	if (elapsed_time > speed) {
		elapsed_time = 0;
		for (int i = 1; i < score; i++) {
			if (score > 1) {
				prev_pos_x[score - i] = prev_pos_x[(score - i) - 1];
				prev_pos_y[score - i] = prev_pos_y[(score - i) - 1];
			}
		}
		if (score > 0) {
			prev_pos_x[0] = ppos_x;
			prev_pos_y[0] = ppos_y;
		}
		ppos_x += speed_x;
		ppos_y += speed_y;
	}

	ppos_x = clamp(-arena_half_size_x+2.0f, ppos_x, arena_half_size_x-2.0f);
	ppos_y = clamp(-arena_half_size_y+2.0f, ppos_y, arena_half_size_y-2.0f);

	// Draw player
	// The body
	for (int i = 0; i < score; i++) {
		draw_rect(prev_pos_x[i], prev_pos_y[i], player_half_size_x, player_half_size_y, 0x440000);
	}
	// The head
	draw_rect(ppos_x, ppos_y, player_half_size_x, player_half_size_y, 0xff0000);

	// Draw food
	draw_rect(food_x, food_y, player_half_size_x, player_half_size_y, 0x00ff00);

	// Draw score
	draw_number(score, 0, 40, 1.f, 0xffffff);

	// To output to debug console
	// ss << food_x;
	// OutputDebugStringA(ss.str().c_str());
}