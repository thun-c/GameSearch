// Copyright [2021] <Copyright Eita Aoki (Thunder) >
#include<string>
#include<vector>
#include<sstream>
#include<iostream>
#include<utility>
#include<random>
#include<assert.h>
#include<math.h>
#include<chrono>
#include <queue>
#include <algorithm>
std::random_device rnd;
std::mt19937 mt(rnd());

using Action = int;
using Actions = std::vector<int>;
using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;
// ���Ԃ��Ǘ�����N���X
class TimeKeeper {
private:
	std::chrono::high_resolution_clock::time_point start_time_;
	int64_t time_threshold_;

public:

	// ���Ԑ������~���b�P�ʂŎw�肵�ăC���X�^���X������B
	TimeKeeper(const int64_t& time_threshold)
		:start_time_(std::chrono::high_resolution_clock::now()),
		time_threshold_(time_threshold)
	{

	}

	// �C���X�^���X��������������w�肵�����Ԑ����𒴉߂��������f����B
	bool isTimeOver() const {
		auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
		return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= time_threshold_;
	}

};

// ��l�Q�[���̗�
// 1�^�[���ɏ㉺���E�l�����̂����ꂩ�ɕǂ̂Ȃ��ꏊ��1�}�X���i�ށB
// ���ɂ���|�C���g�𓥂ނƎ��g�̃X�R�A�ƂȂ�A���̃|�C���g��������B
// END_TURN�̎��_�̃X�R�A���������邱�Ƃ��ړI
class MazeState {
private:
	static constexpr const int dx[4] = { 1,-1,0,0 };
	static constexpr const int dy[4] = { 0,0,1,-1 };

	// �L�����N�^�[�̍��W��ێ�����B
	struct Character {
		int y_;
		int x_;
		Character(const int y = 0, const int x = 0) :y_(y), x_(x) {}
	};

	int h_; // ���H�̍���
	int w_; // ���H�̕�
	int END_TURN_; // �Q�[���I���^�[��
	std::vector<std::vector<int>>walls_; // �ǂ�����ꏊ��1�ŕ\������
	std::vector<std::vector<int>>points_; // ���̃|�C���g��1~9�ŕ\������
	int turn_; // ���݂̃^�[��
	Character character_;

	// ���W(y,x)�ɃL�����N�^�[�����邩���肷��
	bool isCharacterPosition(const int y, const int x)const {
		return(this->character_.y_ == y && this->character_.x_ == x);
	}

public:
	int game_score_; // �Q�[����Ŏ��ۂɓ����X�R�A
	ScoreType evaluated_score_; // �T����ŕ]�������X�R�A
	Action first_action_;// �T���؂̃��[�g�m�[�h�ōŏ��ɐ��󂵂��s��
	MazeState() {}

	// h*w�̖��H�𐶐�����B
	MazeState(const int h, const int w,const int end_turn, const int seed) :
		h_(h),
		w_(w),
		END_TURN_(end_turn),
		walls_(h, std::vector<int>(w)),
		points_(h, std::vector<int>(w)),
		turn_(0),
		character_(h / 2, w / 2),
		game_score_(0),
		evaluated_score_(0),
		first_action_(-1)
	{
		if (h % 2 == 0 || w % 2 == 0) {
			throw std::string("h and w must be odd number");
		}
		auto mt = std::mt19937(seed);

		auto check = std::vector<std::vector<int>>(h, std::vector<int>(w));
		check[character_.y_][character_.x_] = 1;

		for (int y = 1; y < h; y += 2)
			for (int x = 1; x < w; x += 2) {
				int ty = y;
				int tx = x;
				if (!check[ty][tx]) {
					this->walls_[ty][tx] = 1;
					check[ty][tx] = 1;
				}
				if (mt() % 10 > 50) {
					int direction = mt() % (y == 1 ? 4 : 3);
					ty += dy[direction];
					tx += dx[direction];
					if (!check[ty][tx]) {
						this->walls_[ty][tx] = 1;
						check[ty][tx] = 1;
					}
				}
			}
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++) {
				if (!check[y][x]) {
					this->points_[y][x] = mt() % 10;
					check[y][x] = 1;
				}
			}

	}

	// [�ǂ̃Q�[���ł���������] : �T���p�̔Ֆʕ]��������
	void evaluateScore() {
		this->evaluated_score_ = this->game_score_; // �T���ł̓Q�[���{���̃X�R�A�ɕʂ̕]���l���v���X����Ƃ����T�����ł���̂ŁA�����ɍH�v�̗]�n������B
	}

	// [�ǂ̃Q�[���ł���������] : �Q�[���̏I������
	bool isDone()const {
		return this->turn_ == END_TURN_;
	}

	// [�ǂ̃Q�[���ł���������] : �w�肵��action�ŃQ�[����1�^�[���i�߂�
	void advance(const Action& action) {
		this->character_.x_ += dx[action];
		this->character_.y_ += dy[action];
		auto& point = this->points_[this->character_.y_][this->character_.x_];
		if (point > 0) {
			this->game_score_ += point;
			point = 0;
		}
		this->turn_++;

	}

	// [�ǂ̃Q�[���ł���������] : ���݂̏󋵂Ńv���C���[���\�ȍs����S�Ď擾����
	Actions legalActions()const {
		Actions actions;
		for (Action action = 0; action < 4; action++) {
			int ty = this->character_.y_ + dy[action];
			int tx = this->character_.x_ + dx[action];
			if (ty >= 0 && ty < h_ && tx >= 0 && tx < w_
				&& !this->walls_[ty][tx]) {
				actions.emplace_back(action);
			}
		}
		return actions;
	}

	// [�������Ȃ��Ă��悢����������ƕ֗�] : ���݂̃Q�[���󋵂𕶎���ɂ���
	std::string toString()const {
		std::stringstream ss;
		ss << "turn:\t" << this->turn_ << "\n";
		ss << "score:\t" << this->game_score_ << "\n";
		for (int h = 0; h < this->h_; h++) {
			for (int w = 0; w < this->w_; w++) {
				char c = '.';
				if (this->walls_[h][w]) {
					c = '#';
				}
				if (this->character_.y_ == h && this->character_.x_ == w) {
					c = '@';
				}
				if (this->points_[h][w]) {
					c = '0' + (char)points_[h][w];
				}
				ss << c;
			}
			ss << '\n';
		}

		return ss.str();
	}

};

// [�ǂ̃Q�[���ł���������] : �T�����̃\�[�g�p�ɕ]�����r����
bool operator<(const MazeState& maze_1, const MazeState& maze_2) {
	return maze_1.evaluated_score_ < maze_2.evaluated_score_;
}
using State = MazeState;

// �����_���ɍs�������肷��
Action randomAction(const State& state) {
	auto legal_actions = state.legalActions();
	return legal_actions[mt() % (legal_actions.size())];
}

// �×~�@�ōs�������肷��
Action greedyAction(const State& state) {
	auto legal_actions = state.legalActions();
	ScoreType best_score = -INF;
	Action best_action = -1;
	for (const auto action : legal_actions) {
		State now_state = state;
		now_state.advance(action);
		now_state.evaluateScore();
		if (now_state.evaluated_score_ > best_score) {
			best_score = now_state.evaluated_score_;
			best_action = action;
		}
	}
	return best_action;
}

// �r�[�����Ɛ[�����w�肵�ăr�[���T�[�`�ōs�������肷��
Action beamSearchAction(const State& state, const int beam_width, const int beam_depth) {
	auto legal_actions = state.legalActions();
	std::priority_queue<State> now_beam;
	State best_state;

	now_beam.push(state);
	for (int t = 0; t < beam_depth; t++) {
		std::priority_queue<State> next_beam;
		for (int i = 0; i < beam_width; i++) {
			if (now_beam.empty())break;
			State now_state = now_beam.top(); now_beam.pop();
			auto legal_actions = now_state.legalActions();
			for (const auto& action : legal_actions) {
				State next_state = now_state;
				next_state.advance(action);
				next_state.evaluateScore();
				if (t == 0)next_state.first_action_ = action;
				next_beam.push(next_state);
			}
		}

		now_beam = next_beam;
		best_state = now_beam.top();


		if (best_state.isDone())
		{
			break;
		}
	}
	return best_state.first_action_;
}

// �r�[�����Ɛ�������(ms)���w�肵�ăr�[���T�[�`�ōs�������肷��
Action beamSearchActionWithTimeThreshold(const State& state, const int beam_width, const int64_t time_threshold) {
	auto time_keeper = TimeKeeper(time_threshold);
	auto legal_actions = state.legalActions();
	std::priority_queue<State> now_beam;
	State best_state;

	now_beam.push(state);
	for (int t = 0; ; t++) {
		std::priority_queue<State> next_beam;
		for (int i = 0; i < beam_width; i++) {
			if (time_keeper.isTimeOver()) {
				return best_state.first_action_;
			}
			if (now_beam.empty())break;
			State now_state = now_beam.top(); now_beam.pop();
			auto legal_actions = now_state.legalActions();
			for (const auto& action : legal_actions) {
				State next_state = now_state;
				next_state.advance(action);
				next_state.evaluateScore();
				if (t == 0)next_state.first_action_ = action;
				next_beam.push(next_state);
			}
		}

		now_beam = next_beam;
		best_state = now_beam.top();


		if (best_state.isDone())
		{
			break;
		}
	}
	return best_state.first_action_;
}

// �r�[��1�{������̃r�[�����ƃr�[���̖{�����w�肵��chokudai�T�[�`�ōs�������肷��
Action chokudaiSearchAction(const State& state, const int beam_width, const int beam_depth, const int beam_number) {
	auto beam = std::vector<std::priority_queue<State>>(beam_depth + 1);
	for (int t = 0; t < beam_depth + 1; t++) {
		beam[t] = std::priority_queue<State>();
	}
	beam[0].push(state);
	for (int cnt = 0; cnt < beam_number; cnt++) {
		for (int t = 0; t < beam_depth; t++) {
			auto& now_beam = beam[t];
			auto& next_beam = beam[t + 1];
			for (int i = 0; i < beam_width; i++) {
				if (now_beam.empty())break;
				State now_state = now_beam.top();
				if (now_state.isDone()) {
					continue;
				}
				now_beam.pop();
				auto legal_actions = now_state.legalActions();
				for (const auto& action : legal_actions) {
					State next_state = now_state;
					next_state.advance(action);
					next_state.evaluateScore();
					if (t == 0)next_state.first_action_ = action;
					next_beam.push(next_state);
				}
			}
		}
	}
	for (int t = beam_depth; t >= 0; t--) {
		const auto& now_beam = beam[t];
		if (!now_beam.empty()) {
			return now_beam.top().first_action_;
		}
	}

	return -1;
}

// �r�[��1�{������̃r�[�����Ɛ�������(ms)���w�肵��chokudai�T�[�`�ōs�������肷��
Action chokudaiSearchActionWithTimeThreshold(const State& state, const int beam_width, const int beam_depth, const int64_t time_threshold) {
	auto time_keeper = TimeKeeper(time_threshold);
	auto beam = std::vector<std::priority_queue<State>>(beam_depth + 1);
	for (int t = 0; t < beam_depth + 1; t++) {
		beam[t] = std::priority_queue<State>();
	}
	beam[0].push(state);
	for (;;) {
		for (int t = 0; t < beam_depth; t++) {
			auto& now_beam = beam[t];
			auto& next_beam = beam[t + 1];
			for (int i = 0; i < beam_width; i++) {
				if (now_beam.empty())break;
				State now_state = now_beam.top();
				if (now_state.isDone()) {
					continue;
				}
				now_beam.pop();
				auto legal_actions = now_state.legalActions();
				for (const auto& action : legal_actions) {
					State next_state = now_state;
					next_state.advance(action);
					next_state.evaluateScore();
					if (t == 0)next_state.first_action_ = action;
					next_beam.push(next_state);
				}
			}
		}
		if (time_keeper.isTimeOver()) {
			break;
		}
	}
	for (int t = beam_depth; t >= 0; t--) {
		const auto& now_beam = beam[t];
		if (!now_beam.empty()) {
			return now_beam.top().first_action_;
		}
	}

	return -1;
}
#include<iostream>
#include<functional>
using AIFunction = std::function<Action(const State&)>;
using StringAIPair = std::pair<std::string, AIFunction>;

// �Q�[����1��v���C���ăQ�[���󋵂�\������
void playGame(const StringAIPair& ai,const int h,const int w,const int end_turn,const int seed) {
	using std::cout; using std::endl;

	auto state = State(h, w,end_turn, seed);
	state.evaluateScore();
	std::cout << state.toString() << std::endl;
	while (!state.isDone()) {
		state.advance(ai.second(state));
		state.evaluateScore();
		std::cout << state.toString() << std::endl;
	}
}

// �Q�[����game_number��v���C���ăX�R�A���ς�\������
void testAiScore(const StringAIPair& ai, const int game_number, const int h, const int w, const int end_turn) {
	using std::cout; using std::endl;
	std::mt19937 mt_for_construct(0);
	double score_mean = 0;
	for (int i = 0; i < game_number; i++) {
		auto state = State(h, w,end_turn, mt_for_construct());

		while (!state.isDone()) {
			state.advance(ai.second(state));
		}
		auto score = state.game_score_;
		score_mean += score;


		cout << "i " << i << " score " << score_mean / (i + 1) << endl;

	}
	score_mean /= (double)game_number;
	cout << "Score of " << ai.first << ":\t" << score_mean << endl;
}
int main() {
	using std::cout; using  std::endl;

	//const auto& ai = StringAIPair("randomAction", [](const State& state) {return randomAction(state); });
	//const auto& ai = StringAIPair("beamSearchAction", [](const State& state) {return beamSearchAction(state, 2000, 50); });
	//const auto& ai = StringAIPair("beamSearchActionWithTimeThreshold", [](const State& state) {return beamSearchActionWithTimeThreshold(state, 200, 10); });
	//const auto& ai = StringAIPair("chokudaiSearchAction", [](const State& state) {return chokudaiSearchAction(state, 1, 50, 20); });
	//const auto& ai = StringAIPair("chokudaiSearchActionWithTimeThreshold", [](const State& state) {return chokudaiSearchActionWithTimeThreshold(state, 1, 50, 10); });
	const auto& ai = StringAIPair("greedyAction", [](const State& state) {return greedyAction(state); });

	playGame(ai,/*�Ֆʂ̍���*/5,/*�Ֆʂ̕�*/5,/*�Q�[���I���^�[��*/3,/*�Ֆʏ������̃V�[�h*/0);
	//testAiScore(ai,/*�e�X�g�����*/10,/*�Ֆʂ̍���*/31,/*�Ֆʂ̕�*/11,/*�Q�[���I���^�[��*/100);
	return 0;
}