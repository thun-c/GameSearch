// Copyright [2021] <Copyright Eita Aoki (Thunder) >
#include<string>
#include<vector>
#include<sstream>
#include<utility>
#include<random>
#include<assert.h>
#include<math.h>
#include<chrono>
#include <algorithm>
std::random_device rnd;
std::mt19937 mt(rnd());

using Action = int;
using Actions = std::vector<int>;
using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;
// 時間を管理するクラス
class TimeKeeper {
private:
	std::chrono::high_resolution_clock::time_point start_time_;
	int64_t time_threshold_;

public:

	// 時間制限をミリ秒単位で指定してインスタンスをつくる。
	TimeKeeper(const int64_t& time_threshold)
		:start_time_(std::chrono::high_resolution_clock::now()),
		time_threshold_(time_threshold)
	{

	}

	// インスタンス生成した時から指定した時間制限を超過したか判定する。
	bool isTimeOver() const {
		auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
		return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= time_threshold_;
	}

};

// 交互二人ゲームの例
// ○×ゲーム
class TicTacToeState {
private:
	std::vector<int>pieces_;
	std::vector<int>enemy_pieces_;

	//駒の数を計算する
	int pieceCount(const std::vector<int>& pieces) const {
		int count = 0;
		for (const auto i : pieces) {
			if (i == 1)++count;
		}
		return count;
	}

	//敵が3目並んだか判定する
	bool enemyIsComplete(int x, int y, const int dx, const int dy)const {
		for (int k = 0; k < 3; k++) {
			if (y < 0 || 2 < y || x < 0 || 2 < x ||
				this->enemy_pieces_[x + y * 3] == 0
				)return false;
			x += dx; y += dy;
		}
		return true;
	}

	//現在のプレイヤーが先手であるか判定する
	bool isFirstPlayer()const {
		return this->pieceCount(this->pieces_) == this->pieceCount(this->enemy_pieces_);
	}
public:
	TicTacToeState(
		const std::vector<int>& pieces = std::vector<int>(9),
		const std::vector<int>& enemy_pieces = std::vector<int>(9)
	) :
		pieces_(pieces),
		enemy_pieces_(enemy_pieces)
	{	}

	// [どのゲームでも実装する] : 現在のプレイヤー視点の盤面評価をする
	ScoreType getScore()const {
		if (this->isLose())return -1;
		if (this->isDraw())return 0;

		return 0; // 決着のついてない状態での評価のつけ方が工夫の余地
	}

	// [どのゲームでも実装する] : 現在のプレイヤーが負けたか判定する
	bool isLose()const {
		if (enemyIsComplete(0, 0, 1, 1) || enemyIsComplete(0, 2, 1, -1))return true;
		for (int i = 0; i < 3; i++) {
			if (enemyIsComplete(0, i, 1, 0) || enemyIsComplete(i, 0, 0, 1))
				return true;
		}
		return false;
	}

	// [どのゲームでも実装する] : 引き分けになったか判定する
	bool isDraw()const {
		return this->pieceCount(this->pieces_) + this->pieceCount(this->enemy_pieces_) == 9;
	}

	// [どのゲームでも実装する] : ゲームが終了したか判定する
	bool isDone()const {
		return this->isLose() || this->isDraw();
	}

	// [どのゲームでも実装する] : 指定したactionでゲームを1ターン進め、次のプレイヤー視点の盤面にする
	void advance(const Action action) {
		this->pieces_[action] = 1;
		std::swap(this->pieces_, this->enemy_pieces_);

	}

	// [どのゲームでも実装する] : 現在のプレイヤーが可能な行動を全て取得する
	Actions legalActions()const {
		Actions actions;
		for (Action i = 0; i < 9; i++) {
			if (this->pieces_[i] == 0 && this->enemy_pieces_[i] == 0) {
				actions.emplace_back(i);
			}
		}
		return actions;
	}

	// [実装しなくてもよいが実装すると便利] : 現在のプレイヤーの勝率計算のためのスコアを計算する
	double getFirstPlayerScoreForWinRate() const {
		if (this->isLose()) {
			if (this->isFirstPlayer()) {
				return 0.;
			}
			else {
				return 1.;
			}

		}
		else return 0.5;
	}

	// [実装しなくてもよいが実装すると便利] : 現在のゲーム状況を文字列にする
	std::string toString()const {
		std::stringstream ss;
		std::pair<char, char> ox =
			this->isFirstPlayer() ?
			std::pair<char, char>{ 'x', 'o' } :
			std::pair<char, char>{ 'o', 'x' };
		ss << "player: " << ox.first << std::endl;
		for (int i = 0; i < 9; i++) {
			if (this->pieces_[i] == 1)
				ss << ox.first;
			else if (this->enemy_pieces_[i] == 1)
				ss << ox.second;
			else
				ss << '_';
			if (i % 3 == 2)
				ss << std::endl;
		}

		return ss.str();
	}
};

using State = TicTacToeState;

// ランダムに行動を決定する
Action randomAction(const State& state) {
	auto legal_actions = state.legalActions();
	return legal_actions[mt() % (legal_actions.size())];
}

namespace minimax {
	// minimaxのためのスコア計算
	ScoreType miniMaxScore(const State& state, const int depth) {
		if (state.isDone() || depth == 0) {
			return state.getScore();
		}
		auto legal_actions = state.legalActions();
		if (legal_actions.empty()) {
			return state.getScore();
		}
		ScoreType bestScore = -INF;
		for (const auto action : legal_actions) {
			State next_state = state;
			next_state.advance(action);
			ScoreType score = -miniMaxScore(next_state, depth - 1);
			if (score > bestScore) {
				bestScore = score;
			}
		}
		return bestScore;
	}
	// 深さを指定してminimaxで行動を決定する
	Action miniMaxAction(const State& state, const int depth) {
		ScoreType best_action = -1;
		ScoreType best_score = -INF;
		for (const auto action : state.legalActions()) {
			State next_state = state;
			next_state.advance(action);
			ScoreType score = -miniMaxScore(next_state, depth);
			if (score > best_score) {
				best_action = action;
				best_score = score;
			}
		}
		return best_action;
	}
}
using minimax::miniMaxAction;

namespace alphabeta {
	// alphabetaのためのスコア計算
	ScoreType alphaBetaScore(const State& state, ScoreType alpha, const ScoreType beta, const int depth) {
		if (state.isDone() || depth == 0) {
			return state.getScore();
		}
		auto legal_actions = state.legalActions();
		if (legal_actions.empty()) {
			return state.getScore();
		}
		for (const auto action : legal_actions) {
			State next_state = state;
			next_state.advance(action);
			ScoreType score = -alphaBetaScore(next_state, -beta, -alpha, depth - 1);
			if (score > alpha) {
				alpha = score;
			}
			if (alpha >= beta) {
				return alpha;
			}
		}
		return alpha;
	}
	// 深さを指定してalphabetaで行動を決定する
	Action alphaBetaAction(const State& state, const int depth) {
		ScoreType best_action = -1;
		ScoreType alpha = -INF;
		for (const auto action : state.legalActions()) {
			State next_state = state;
			next_state.advance(action);
			ScoreType score = -alphaBetaScore(next_state, -INF, -alpha, depth);
			if (score > alpha) {
				best_action = action;
				alpha = score;
			}
		}
		assert(best_action >= 0);
		return best_action;
	}
}
using alphabeta::alphaBetaAction;


namespace iterativedeepning {
	// 制限時間が切れた際に停止できるalphabetaのためのスコア計算
	ScoreType alphaBetaScore(const State& state, ScoreType alpha, const ScoreType beta, const int depth, const TimeKeeper& time_keeper) {
		if (time_keeper.isTimeOver())return 0;
		if (state.isDone() || depth == 0) {
			return state.getScore();
		}
		auto legal_actions = state.legalActions();
		if (legal_actions.empty()) {
			return state.getScore();
		}
		for (const auto action : legal_actions) {
			State next_state = state;
			next_state.advance(action);
			ScoreType score = -alphaBetaScore(next_state, -beta, -alpha, depth - 1, time_keeper);
			if (time_keeper.isTimeOver())return 0;
			if (score > alpha) {
				alpha = score;
			}
			if (alpha >= beta) {
				return alpha;
			}
		}
		return alpha;
	}
	// 深さと制限時間(ms)を指定してalphabetaで行動を決定する
	Action alphaBetaActionWithTimeThreshold(const State& state, const int depth, const TimeKeeper& time_keeper) {
		ScoreType best_action = -1;
		ScoreType alpha = -INF;
		for (const auto action : state.legalActions()) {
			State next_state = state;
			next_state.advance(action);
			ScoreType score = -alphaBetaScore(next_state, -INF, -alpha, depth, time_keeper);
			if (time_keeper.isTimeOver())return 0;
			if (score > alpha) {
				best_action = action;
				alpha = score;
			}
		}
		return best_action;
	}

	// 制限時間(ms)を指定して反復深化で行動を決定する
	Action iterativeDeepningAction(const State& state, const int64_t time_threshold) {
		auto time_keeper = TimeKeeper(time_threshold);
		Action best_action = -1;
		for (int depth = 1;; depth++) {
			Action action = alphaBetaActionWithTimeThreshold(state, depth, time_keeper);


			if (time_keeper.isTimeOver()) {
				break;
			}
			else {
				best_action = action;
			}
		}
		return best_action;
	}
}
using iterativedeepning::iterativeDeepningAction;

namespace montecarlo {
	// 配列の最大値のインデックスを返す
	int argMax(const std::vector<double>& x) {
		return std::distance(x.begin(), std::max_element(x.begin(), x.end()));
	}
	// ランダムプレイアウトをして勝敗スコアを計算する
	double playout(State* state) { // const&にすると再帰中にディープコピーが必要になるため、高速化のためポインタにする。(constでない参照でも可)
		if (state->isLose())
			return 0;
		if (state->isDraw())
			return 0.5;
		state->advance(randomAction(*state));
		return 1. - playout(state);
	}
	// プレイアウト回数を指定して原始モンテカルロ法で行動を決定する
	Action primitiveMontecarloAction(const State& state, int playout_number) {
		auto legal_actions = state.legalActions();
		double best_value = -INF;
		int best_i = -1;
		for (int i = 0; i < legal_actions.size(); i++) {
			double value = 0;
			for (int j = 0; j < playout_number; j++) {
				State next_state = state;
				next_state.advance(legal_actions[i]);
				value += 1. - playout(&next_state);
			}
			if (value > best_value) {
				best_i = i;
				best_value = value;
			}
		}
		return legal_actions[best_i];

	}
	// 制限時間(ms)を指定して原始モンテカルロ法で行動を決定する
	Action primitiveMontecarloActionWithTimeThreshold(const State& state, const int64_t time_threshold) {
		auto legal_actions = state.legalActions();
		double best_value = -INF;
		int best_i = -1;
		auto time_keeper = TimeKeeper(time_threshold);
		auto values = std::vector<double>(legal_actions.size());
		while (true) {

			for (int i = 0; i < legal_actions.size(); i++) {
				State next_state = state;
				next_state.advance(legal_actions[i]);
				values[i] += 1. - playout(&next_state);
			}
			if (time_keeper.isTimeOver()) {
				break;
			}

		}
		return legal_actions[argMax(values)];

	}

	constexpr const double C = 1.; //UCB1の計算に使う定数
	constexpr const int EXPAND_THRESHOLD = 10; // ノードを展開する閾値

	// MCTSの計算に使うノード
	class Node {
	private:
		State state_;
		double w_;
	public:
		std::vector<Node>child_nodes;
		double n_;

		// ノードの評価を行う
		double evaluate() {
			if (this->state_.isDone()) {
				double value = this->state_.isLose() ? 0 : 0.5;
				this->w_ += value;
				++this->n_;
				return value;
			}
			if (this->child_nodes.empty()) {
				State state_copy = this->state_;
				double value = playout(&state_copy);
				this->w_ += value;
				++this->n_;

				if (this->n_ == EXPAND_THRESHOLD)
					this->expand();

				return value;
			}
			else {
				double value = 1. - this->nextChiledNode().evaluate();
				this->w_ += value;
				++this->n_;
				return value;
			}
		}

		// ノードを展開する
		void expand() {
			auto legal_actions = this->state_.legalActions();
			this->child_nodes.clear();
			for (const auto action : legal_actions) {
				this->child_nodes.emplace_back(this->state_);
				this->child_nodes.back().state_.advance(action);
			}
		}

		// どのノードを評価するか選択する
		Node& nextChiledNode() {
			for (auto& child_node : this->child_nodes) {
				if (child_node.n_ == 0)
					return child_node;
			}
			double t = 0;
			for (const auto& child_node : this->child_nodes) {
				t += child_node.n_;
			}
			double best_value = -INF;
			int best_i = -1;
			for (int i = 0; i < this->child_nodes.size(); i++) {
				const auto& child_node = this->child_nodes[i];
				double wr = 1. - child_node.w_ / child_node.n_;
				double bias = std::sqrt(2. * std::log(t) / child_node.n_);

				double ucb1_value = 1. - child_node.w_ / child_node.n_ + (double)C * std::sqrt(2. * std::log(t) / child_node.n_);
				if (ucb1_value > best_value) {
					best_i = i;
					best_value = ucb1_value;
				}
			}
			return this->child_nodes[best_i];
		}

		Node(const State& state) :state_(state), w_(0), n_(0) {}

	};

	// プレイアウト数を指定してMCTSで行動を決定する
	Action mctsAction(const State& state, const int playout_number) {
		Node root_node = Node(state);
		root_node.expand();
		for (int i = 0; i < playout_number; i++) {
			root_node.evaluate();
		}
		auto legal_actions = state.legalActions();

		int best_n = -1;
		int best_i = -1;
		assert(legal_actions.size() == root_node.child_nodes.size());
		for (int i = 0; i < legal_actions.size(); i++) {
			int n = root_node.child_nodes[i].n_;
			if (n > best_n) {
				best_i = i;
				best_n = n;
			}
		}
		return legal_actions[best_i];
	}

	// 制限時間(ms)を指定してMCTSで行動を決定する
	Action mctsActionWithTimeThreshold(const State& state, const int64_t time_threshold) {
		Node root_node = Node(state);
		root_node.expand();
		auto time_keeper = TimeKeeper(time_threshold);
		for (int cnt = 0;; cnt++) {
			if (time_keeper.isTimeOver()) {
				break;
			}
			root_node.evaluate();
		}
		auto legal_actions = state.legalActions();

		int best_n = -1;
		int best_i = -1;
		assert(legal_actions.size() == root_node.child_nodes.size());
		for (int i = 0; i < legal_actions.size(); i++) {
			int n = root_node.child_nodes[i].n_;
			if (n > best_n) {
				best_i = i;
				best_n = n;
			}
		}
		return legal_actions[best_i];
	}
}
using montecarlo::primitiveMontecarloAction;
using montecarlo::mctsAction;
using montecarlo::mctsActionWithTimeThreshold;
using montecarlo::primitiveMontecarloActionWithTimeThreshold;


#include<iostream>
#include<functional>


using AIFunction = std::function<Action(const State&)>;
using StringAIPair = std::pair<std::string, AIFunction>;

// ゲームを1回プレイしてゲーム状況を表示する
void playGame(const std::vector<StringAIPair>& ais) {
	using std::cout; using std::endl;
	auto state = State();
	while (!state.isDone()) {
		// 1p
		{
			cout << "1p " << ais[0].first << "------------------------------------" << endl;
			Action action = ais[0].second(state);
			cout << "action " << action << endl;
			state.advance(action);
			cout << state.toString() << endl;
			if (state.isDone())break;
		}
		// 2p
		{
			cout << "2p " << ais[1].first << "------------------------------------" << endl;
			Action action = ais[1].second(state);
			cout << "action " << action << endl;
			state.advance(action);
			cout << state.toString() << endl;
			if (state.isDone())break;
		}
	}
}

// ゲームをgame_number×2(先手後手を交代)回プレイしてaisの0番目のAIの勝率を表示する。
void testFirstPlayerWinRate(const std::vector<StringAIPair>& ais, const int game_number) {
	using std::cout; using  std::endl;

	double first_player_win_rate = 0;
	for (int i = 0; i < game_number; i++) {
		for (int j = 0; j < 2; j++) {//先手後手平等に行う
			auto state = State();
			auto& first_ai = ais[j];
			auto& second_ai = ais[(j + 1) % 2];
			while (true) {
				state.advance(first_ai.second(state));
				if (state.isDone())break;
				state.advance(second_ai.second(state));
				if (state.isDone())break;
			}
			double win_rate_point = state.getFirstPlayerScoreForWinRate();
			if (j == 1)win_rate_point = 1 - win_rate_point;
			if (win_rate_point >= 0) {
				state.toString();
			}
			first_player_win_rate += win_rate_point;


		}
		cout << "i " << i << " w " << first_player_win_rate / ((i + 1) * 2) << endl;

	}
	first_player_win_rate /= (double)(game_number * 2);
	cout << "Winning rate of " << ais[0].first << " to " << ais[1].first << ":\t" << first_player_win_rate << endl;
}
int main() {
	using std::cout; using  std::endl;

	std::vector<StringAIPair> ais = {
		//StringAIPair("miniMaxAction",[](const State& state) {return miniMaxAction(state,3); }),
		//StringAIPair("randomAction",[](const State& state) {return randomAction(state); }),
		StringAIPair("mctsAction",[](const State& state) {return mctsAction(state, 1000); }),
		StringAIPair("primitiveMontecarloAction",[](const State& state) {return primitiveMontecarloAction(state,1000); }),
		//StringAIPair("mctsActionWithTimeThreshold",[](const State& state) {return mctsActionWithTimeThreshold(state,10); }),
		//StringAIPair("mctsActionWithTimeThreshold10",[](const State& state) {return mctsActionWithTimeThreshold(state,100); }),
		//StringAIPair("primitiveMontecarloActionWithTimeThreshold",[](const State& state) {return primitiveMontecarloActionWithTimeThreshold(state,1); }),
		//StringAIPair("alphaBetaAction",[](const State& state) {return alphaBetaAction(state,-1); }),
		//StringAIPair("iterativeDeepningAction",[](const State& state) {return iterativeDeepningAction(state,10); }),
	};
	playGame(ais);
	//testFirstPlayerWinRate(ais,10);
	return 0;
}