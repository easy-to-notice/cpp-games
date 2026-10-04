#include <graphics.h>
#include <string>
#include <vector>
#include <cmath>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

bool is_game_started = 0;
bool running = 1;

inline void putimage_alpha(int x,int y,IMAGE* img) {//实现透明背景
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

class Atlas {
public:
	Atlas(LPCTSTR path, int num) {
		

		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++) {
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}

	~Atlas() {
		for (int i = 0; i < frame_list.size(); i++) {
			delete frame_list[i];
		}
	}

public:
	std::vector<IMAGE*> frame_list;
};

Atlas* atlas_player_left;
Atlas* atlas_player_right;
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right;

class Animation {
public:
	Animation(Atlas* atlas,int interval) {
		interval_ms = interval;
		anim_atlas = atlas;
	}

	~Animation() = default;

	void play(int x,int y,int delta) {
		timer += delta;
		if (timer > interval_ms) {
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
			timer = 0;
		}
		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
	}
private:
	int timer=0;
	int idx_frame=0;
	int interval_ms = 0;
	Atlas* anim_atlas;
};

class Player {
public:
	const int FRAME_WIDTH = 80;
	const int FRAME_HEIGHT = 80;
public:
	Player() {
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation (atlas_player_left, 45);
		anim_right = new Animation (atlas_player_right, 45);
	}

	~Player() {

	}

	void ProcessEvent(const ExMessage& msg) {
		if (msg.message == WM_KEYDOWN) {
			switch (msg.vkcode) {
			case VK_UP:
				is_move_up = 1;
				break;
			case VK_DOWN:
				is_move_down = 1;
				break;
			case VK_LEFT:
				is_move_left = 1;
				break;
			case VK_RIGHT:
				is_move_right = 1;
				break;
			}
		}
		else if (msg.message == WM_KEYUP) {
			switch (msg.vkcode) {
			case VK_UP:
				is_move_up = 0;
				break;
			case VK_DOWN:
				is_move_down = 0;
				break;
			case VK_LEFT:
				is_move_left = 0;
				break;
			case VK_RIGHT:
				is_move_right = 0;
				break;
			}
		}

	}

	void Move() {

		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir) {
			double normalized_x = (int)dir_x / len_dir;
			double normalized_y = (int)dir_y / len_dir;
			player_pos.x += (int)(SPEED * normalized_x);
			player_pos.y += (int)(SPEED * normalized_y);
		}
		if (player_pos.x < 0)player_pos.x = 0;
		else if (player_pos.x + FRAME_WIDTH > WINDOW_WIDTH)
			player_pos.x = WINDOW_WIDTH - FRAME_WIDTH;
		if (player_pos.y < 0)player_pos.y = 0;
		else if (player_pos.y + FRAME_HEIGHT > WINDOW_HEIGHT)
			player_pos.y = WINDOW_HEIGHT - FRAME_HEIGHT;

	}

	void Draw(int delta) {
		int pos_shadow_x = player_pos.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = player_pos.y + FRAME_HEIGHT - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		static bool face_left = 0;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)face_left = 1;
		else if (dir_x > 0)face_left = 0;

		if (face_left)anim_left->play(player_pos.x, player_pos.y, delta);
		else anim_right->play(player_pos.x, player_pos.y, delta);

	}

	const POINT& GetPosition()const {
		return player_pos;
	}

private:
	const int SPEED = 4;
	const int SHADOW_WIDTH = 32;

	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT player_pos = { 500,500 };
	bool is_move_up = 0;
	bool is_move_down = 0;
	bool is_move_left = 0;
	bool is_move_right = 0;

};

class Bullet {
public:
	POINT position = { 0,0 };
public:
	Bullet() = default;
	~Bullet() = default;

	void Draw()const {
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	}
private:
	const int RADIUS = 10;
};

class Enemy {
public:
	Enemy() {
		loadimage(&img_shadow, _T("img/enemy_shadow.png"));
		anim_left = new Animation(atlas_enemy_left, 45);
		anim_right = new Animation(atlas_enemy_right, 45);

		enum class SpawnEdge {
			Up=0,Down,Left,Right
		};

		//将敌人放置在地图外边界处的随机位置
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge) {
		case SpawnEdge::Up:
			position.x = rand() % WINDOW_WIDTH;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH;
			position.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left:
			position.x = -FRAME_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		case SpawnEdge::Right:
			position.x = WINDOW_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		default:
			break;
		}
	}

	~Enemy() {
		delete anim_left, anim_right;
	}

	bool CheckBulletCollision(const Bullet& bullet) {
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
		return is_overlap_x && is_overlap_y;

	}

	bool CheckPlayerCollision(const Player& player) {
		POINT check_position = { position.x + FRAME_WIDTH / 2, position.y + FRAME_HEIGHT / 2 };
		bool is_overlap_x = check_position.x >= player.GetPosition().x && check_position.x <= player.GetPosition().x + player.FRAME_WIDTH;
		bool is_overlap_y = check_position.y >= player.GetPosition().y && check_position.y <= player.GetPosition().y + player.FRAME_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}
	
	void Move(const Player& player) {
		const POINT& player_pos = player.GetPosition();
		int dir_x = player_pos.x - position.x;
		int dir_y = player_pos.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir) {
			double normalized_x = (int)dir_x / len_dir;
			double normalized_y = (int)dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}
		if (dir_x < 0)facing_left = 1;
		else facing_left = 0;
	}

	void Draw(int delta) {
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + FRAME_HEIGHT - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (facing_left)anim_left->play(position.x, position.y, delta);
		else anim_right->play(position.x, position.y, delta);

	}

	void Hurt() {
		alive = 0;
	}

	bool CheckAlive() {
		return alive;
	}

private:
	const int SPEED = 3;
	const int FRAME_WIDTH = 80;
	const int FRAME_HEIGHT = 80;
	const int SHADOW_WIDTH = 48;

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };
	bool facing_left = 0;
	bool alive = 1;
};

class Button {
public:
	Button(RECT rect,LPCTSTR path_img_idle,LPCTSTR path_img_hovered,LPCTSTR path_img_pushed) {
		region = rect;

		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}
	
	void ProcessEvent(const ExMessage& msg) {
		switch (msg.message) {
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
				status = Status::Hovered;
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
				status = Status::Idle;
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
				status = Status::Pushed;
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
				OnClick();
			break;
		}
	}

	~Button() = default;

	void Draw() {
		switch (status) {
		case Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}

protected:
	virtual void OnClick() = 0;

private:
	enum class Status {
		Idle = 0, Hovered, Pushed
	};

private:
	RECT region;
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;

private:
	bool CheckCursorHit(int x, int y) {
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};

class StartGameButton :public Button {
public:
	StartGameButton(RECT rect,LPCTSTR path_img_idle,LPCTSTR path_img_hovered,LPCTSTR path_img_pushed):
		Button(rect, path_img_idle, path_img_hovered, path_img_pushed){ }
	~StartGameButton() = default;

protected:
	void OnClick() {
		is_game_started = 1;
		mciSendString(_T("play bgm repeat from 0"), 0, 0, 0);
	}
};

class QuitGameButton :public Button {
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed) :
		Button(rect, path_img_idle, path_img_hovered, path_img_pushed){ }
	~QuitGameButton() = default;

protected:
	void OnClick() {
		running = 0;
	}
};

void TryGenerateEnemy(std::vector<Enemy*>& enemy_list) {
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
		enemy_list.push_back(new Enemy());
}

void UpdateBullets(std::vector<Bullet>& bullet_list, const Player player) {
	const double RADIAL_SPEED = 0.0045;//径向速度
	const double TANGENT_SPEED = 0.0045;//切向速度
	double radian_interval = 2 * 3.14159 / bullet_list.size();//弧度间隔
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++) {
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;// 当前子弹所在弧度值
		bullet_list[i].position.x = player_position.x + player.FRAME_WIDTH / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.FRAME_HEIGHT / 2 + (int)(radius * cos(radian));
		//将 sin 与 cos 交换 -> 逆时针旋转，否则顺时针
	}
}

void DrawPlayerScore(int score) {
	static TCHAR text[64];
	_stprintf_s(text,_T("当前玩家得分：%d"),score);
	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(10, 10, text);
}


//-----------------------------------------------------------
int main() {
	initgraph(1280, 720);

flg_game:

	atlas_player_left = new Atlas(_T("img/player_left_%d.png"), 6);
	atlas_player_right = new Atlas(_T("img/player_right_%d.png"), 6);
	atlas_enemy_left = new Atlas(_T("img/enemy_left_%d.png"), 6);
	atlas_enemy_right = new Atlas(_T("img/enemy_right_%d.png"), 6);

	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open mus/hit.wav alias hit"), 0, 0, 0);


	int ms = 1000 / 144;
	int score = 0;
	ExMessage msg;
	IMAGE img_menu;
	IMAGE img_background;
	Player player;
	std::vector<Enemy*> enemy_list;
	std::vector<Bullet> bullet_list(3);
	
	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

	StartGameButton btn_start_game = StartGameButton(region_btn_start_game, _T("img/ui_start_idle.png"),
		_T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game, _T("img/ui_quit_idle.png"),
		_T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

	loadimage(&img_menu, _T("img/menu.png"));
	loadimage(&img_background, _T("img/background.png"));
	
	BeginBatchDraw();

	while (running) {
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg)) {
			if (is_game_started)
				player.ProcessEvent(msg);
			else {
				btn_start_game.ProcessEvent(msg);
				btn_quit_game.ProcessEvent(msg);
			}
		}

		if (is_game_started) {
			player.Move();
			TryGenerateEnemy(enemy_list);
			UpdateBullets(bullet_list, player);
			for (int i = 0; i < enemy_list.size(); i++)
				enemy_list[i]->Move(player);
			//检测敌人和玩家的碰撞
			for (Enemy* enemy : enemy_list) {
				if (enemy->CheckPlayerCollision(player)) {
					static TCHAR text[128];
					_stprintf_s(text, _T("最终得分：%d！"), score);
					MessageBox(GetHWnd(), text, _T("游戏结束"), MB_OK);
					running = 0;
					break;
				}
			}
			//检测敌人和子弹的碰撞
			for (Enemy* enemy : enemy_list) {
				for (const Bullet& bullet : bullet_list) {
					if (enemy->CheckBulletCollision(bullet)) {
						mciSendString(_T("play hit from 0"), 0, 0, 0);
						enemy->Hurt();
						score++;
					}
				}
			}
			//移除生命值归零的敌人
			for (int i = 0; i < enemy_list.size(); i++) {
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive()) {
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;

				}
			}
		}

		cleardevice();

		if (is_game_started) {
			putimage(0, 0, &img_background);
			player.Draw(ms);
			for (int i = 0; i < enemy_list.size(); i++)
				enemy_list[i]->Draw(ms);
			for (const Bullet& bullet : bullet_list)
				bullet.Draw();
			DrawPlayerScore(score);
		}
		else {
			putimage(0, 0, &img_menu);
			btn_start_game.Draw();
			btn_quit_game.Draw();
		}

		FlushBatchDraw();

		//主循环内应该避免阻塞式行为
		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
		if (delta_time < ms) {
			Sleep(ms - delta_time);
		}

	}

	EndBatchDraw();

	delete atlas_player_left;
	delete atlas_player_right;
	delete atlas_enemy_left;
	delete atlas_player_right;

	return 0;
}