// Copyright [2021] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <random>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <queue>
#include <algorithm>
std::random_device rnd;
std::mt19937 mt(rnd());

using Action = int;
using Actions = std::vector<int>;
using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;
// 時間を管理するクラス
class TimeKeeper
{
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    int64_t time_threshold_;

public:
    // 時間制限をミリ秒単位で指定してインスタンスをつくる。
    TimeKeeper(const int64_t &time_threshold)
        : start_time_(std::chrono::high_resolution_clock::now()),
          time_threshold_(time_threshold)
    {
    }

    // インスタンス生成した時から指定した時間制限を超過したか判定する。
    bool isTimeOver() const
    {
        auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
        return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= time_threshold_;
    }
};
static const std::string dstr[4] = {"RIGHT", "LEFT", "DOWN", "UP"};

// 同時二人ゲームの例
// 1ターンに上下左右四方向のいずれかに壁のない場所に1マスずつ進む。
// 床にあるポイントを踏むと自身のスコアとなり、床のポイントが消える。
// 勝利条件はEND_TURNの時点のスコアが敵より高いこと
class SimultaneousMazeState
{
private:
    static constexpr const int END_TURN = 20;
    static constexpr const int dx[4] = {1, -1, 0, 0};
    static constexpr const int dy[4] = {0, 0, 1, -1};
    struct Character
    {
        int y_;
        int x_;
        int game_score_;
        Character(const int y = 0, const int x = 0) : y_(y), x_(x), game_score_(0) {}
    };
    int h_;
    int w_;
    std::vector<std::vector<int>> walls_;
    std::vector<std::vector<int>> points_;
    int turn_;
    std::vector<Character> characters_;

public:
    Action first_action_;
    SimultaneousMazeState(const int h, const int w, const int seed) : h_(h),
                                                                      w_(w),
                                                                      walls_(h, std::vector<int>(w)),
                                                                      points_(h, std::vector<int>(w)),
                                                                      turn_(0),
                                                                      characters_({Character(h / 2, (w / 2) - 1), Character(h / 2, (w / 2) + 1)}),
                                                                      first_action_(-1)
    {
        if (h % 2 == 0 || w % 2 == 0)
        {
            throw std::string("h and w must be odd number");
        }
        auto mt = std::mt19937(seed);

        auto check = std::vector<std::vector<int>>(h, std::vector<int>(w));
        for (auto &character : this->characters_)
        {
            check[character.y_][character.x_] = 1;
        }

        for (int y = 1; y < h; y += 2)
            for (int x = 1; x < w; x += 2)
            {
                int ty = y;
                int tx = x;
                if (!check[ty][tx])
                {
                    this->walls_[ty][tx] = 1;
                    check[ty][tx] = 1;
                }
                if (mt() % 10 > 50)
                {
                    int direction = mt() % (y == 1 ? 4 : 3);
                    ty += dy[direction];
                    tx += dx[direction];
                    if (!check[ty][tx])
                    {
                        this->walls_[ty][tx] = 1;
                        check[ty][tx] = 1;
                    }
                }
            }
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w / 2 + 1; x++)
            {
                int ty = y;
                int tx = x;
                int point = mt() % 10;
                if (!check[ty][tx])
                {
                    this->points_[ty][tx] = point;
                    check[ty][tx] = 1;
                }
                tx = w - 1 - x;
                if (!check[ty][tx])
                {
                    this->points_[ty][tx] = point;
                    check[ty][tx] = 1;
                }
            }
    }
    // [どのゲームでも実装する] : プレイヤー0が勝ったか判定する
    bool isWin() const
    {
        return isDone() && (characters_[0].game_score_ > characters_[1].game_score_);
    }
    // [どのゲームでも実装する] : プレイヤー0が負けたか判定する
    bool isLose() const
    {
        return isDone() && (characters_[0].game_score_ < characters_[1].game_score_);
    }
    // [どのゲームでも実装する] : 引き分けになったか判定する
    bool isDraw() const
    {
        return isDone() && (characters_[0].game_score_ == characters_[1].game_score_);
    }
    // [どのゲームでも実装する] : ゲームが終了したか判定する
    bool isDone() const
    {
        return this->turn_ == END_TURN;
    }
    // [どのゲームでも実装する] : 指定したactionでゲームを1ターン進める
    void advance(const Action action0, const Action action1)
    {
        {
            auto &character = this->characters_[0];
            const auto &action = action0;
            character.x_ += dx[action];
            character.y_ += dy[action];
            const auto point = this->points_[character.y_][character.x_];
            if (point > 0)
            {
                character.game_score_ += point;
            }
        }
        {
            auto &character = this->characters_[1];
            const auto &action = action1;
            character.x_ += dx[action];
            character.y_ += dy[action];
            const auto point = this->points_[character.y_][character.x_];
            if (point > 0)
            {
                character.game_score_ += point;
            }
        }

        for (const auto &character : this->characters_)
        {
            this->points_[character.y_][character.x_] = 0;
        }
        this->turn_++;
    }

    // [どのゲームでも実装する] : 指定したプレイヤーが可能な行動を全て取得する
    Actions legalActions(const int player_id) const
    {
        Actions actions;
        const auto &character = this->characters_[player_id];
        for (Action action = 0; action < 4; action++)
        {
            int ty = character.y_ + dy[action];
            int tx = character.x_ + dx[action];
            if (ty >= 0 && ty < h_ && tx >= 0 && tx < w_ && !this->walls_[ty][tx])
            {
                actions.emplace_back(action);
            }
        }
        return actions;
    }

    // [実装しなくてもよいが実装すると便利] : プレイヤー0の勝率計算のためのスコアを計算する
    double getFirstPlayerScoreForWinRate() const
    {
        if (this->isWin())
        {
            return 1.;
        }
        else if (this->isLose())
        {
            return 0.;
        }
        else
            return 0.5;
    }

    // [実装しなくてもよいが実装すると便利] : 現在のゲーム状況を文字列にする
    std::string toString() const
    {
        std::stringstream ss("");
        ss << "turn:\t" << this->turn_ << "\n";
        for (int player_id = 0; player_id < this->characters_.size(); player_id++)
        {
            ss << "score(" << player_id << "):\t" << this->characters_[player_id].game_score_ << "\n";
        }
        for (int h = 0; h < this->h_; h++)
        {
            for (int w = 0; w < this->w_; w++)
            {
                char c = '.';
                if (this->walls_[h][w])
                {
                    c = '#';
                }
                for (int player_id = 0; player_id < this->characters_.size(); player_id++)
                {
                    const auto &character = this->characters_[player_id];
                    if (character.y_ == h && character.x_ == w)
                    {
                        c = 'A' + (player_id);
                    }
                }
                if (this->points_[h][w])
                {
                    c = '0' + (char)points_[h][w];
                }
                ss << c;
            }
            ss << '\n';
        }

        return ss.str();
    }
};
using State = SimultaneousMazeState;

// 指定したプレイヤーの行動をランダムに決定する
Action randomAction(const State &state, const int player_id)
{
    auto legal_actions = state.legalActions(player_id);
    return legal_actions[mt() % (legal_actions.size())];
}
namespace montecarlo
{
    // 配列の最大値のインデックスを返す
    int argMax(const std::vector<double> &x)
    {
        return std::distance(x.begin(), std::max_element(x.begin(), x.end()));
    }
    //プレイヤー0視点での評価
    double playout(State *state)
    { // const&にすると再帰中にディープコピーが必要になるため、高速化のためポインタにする。(constでない参照でも可)
        if (state->isWin())
            return 1;
        if (state->isLose())
            return 0;
        if (state->isDraw())
            return 0.5;
        state->advance(randomAction(*state, 0), randomAction(*state, 1));
        return playout(state);
    }
    // 制限時間(ms)を指定して原始モンテカルロ法で指定したプレイヤーの行動を決定する
    Action primitiveMontecarloAction(const State &state, const int player_id, const int playout_number)
    {
        auto my_legal_actions = state.legalActions(player_id);
        auto opp_legal_actions = state.legalActions((player_id + 1) % 2);
        double best_value = -INF;
        int best_i = -1;
        for (int i = 0; i < my_legal_actions.size(); i++)
        {
            double value = 0;
            for (int j = 0; j < playout_number; j++)
            {
                State next_state = state;
                if (player_id == 0)
                {
                    next_state.advance(my_legal_actions[i], opp_legal_actions[mt() % opp_legal_actions.size()]);
                }
                else
                {
                    next_state.advance(opp_legal_actions[mt() % opp_legal_actions.size()], my_legal_actions[i]);
                }
                double player0_win_rate = playout(&next_state);
                double win_rate = (player_id == 0 ? player0_win_rate : 1. - player0_win_rate);
                value += win_rate;
            }
            if (value > best_value)
            {
                best_i = i;
                best_value = value;
            }
        }
        return my_legal_actions[best_i];
    }
    constexpr const double C = 1.;            // UCB1の計算に使う定数
    constexpr const int EXPAND_THRESHOLD = 5; // ノードを展開する閾値

    // DUCTの計算に使うノード
    class Node
    {
    private:
        State state_;
        double w_;

    public:
        std::vector<std::vector<Node>> child_nodeses;
        double n_;

        // ノードの評価を行う
        double evaluate()
        {
            if (this->state_.isDone())
            {
                double value = 0.5;
                if (this->state_.isWin())
                    value = 1.;
                else if (this->state_.isLose())
                    value = 0.;
                this->w_ += value;
                ++this->n_;
                return value;
            }
            if (this->child_nodeses.empty())
            {
                State state_copy = this->state_;
                double value = playout(&state_copy);
                this->w_ += value;
                ++this->n_;

                if (this->n_ == EXPAND_THRESHOLD)
                    this->expand();

                return value;
            }
            else
            {
                double value = this->nextChiledNode().evaluate();
                this->w_ += value;
                ++this->n_;
                return value;
            }
        }
        // ノードを展開する
        void expand()
        {
            auto legal_actions0 = this->state_.legalActions(0);
            auto legal_actions1 = this->state_.legalActions(1);
            this->child_nodeses.clear();
            for (const auto &action0 : legal_actions0)
            {
                this->child_nodeses.emplace_back();
                auto &target_nodes = this->child_nodeses.back();
                for (const auto &action1 : legal_actions1)
                {
                    target_nodes.emplace_back(this->state_);
                    auto &target_node = target_nodes.back();
                    target_node.state_.advance(action0, action1);
                }
            }
        }
        // どのノードを評価するか選択する
        Node &nextChiledNode()
        {
            for (auto &child_nodes : this->child_nodeses)
            {
                for (auto &child_node : child_nodes)
                {
                    if (child_node.n_ == 0)
                        return child_node;
                }
            }
            double t = 0;
            for (auto &child_nodes : this->child_nodeses)
            {
                for (auto &child_node : child_nodes)
                {
                    t += child_node.n_;
                }
            }
            int best_is[] = {-1, -1};

            double best_value = -INF;
            for (int i = 0; i < this->child_nodeses.size(); i++)
            {
                const auto &childe_nodes = this->child_nodeses[i];
                double w = 0;
                double n = 0;
                for (int j = 0; j < childe_nodes.size(); j++)
                {
                    const auto &child_node = childe_nodes[j];
                    w += child_node.w_;
                    n += child_node.n_;
                }
                double bias = std::sqrt(2. * std::log(t) / n);

                double ucb1_value = w / n + (double)C * std::sqrt(2. * std::log(t) / n);
                if (ucb1_value > best_value)
                {
                    best_is[0] = i;
                    best_value = ucb1_value;
                }
            }
            best_value = -INF;
            for (int j = 0; j < this->child_nodeses[0].size(); j++)
            {
                double w = 0;
                double n = 0;
                for (int i = 0; i < this->child_nodeses.size(); i++)
                {
                    const auto &child_node = child_nodeses[i][j];
                    w += child_node.w_;
                    n += child_node.n_;
                }
                double bias = std::sqrt(2. * std::log(t) / n);

                double ucb1_value = (1 - w / n) + (double)C * std::sqrt(2. * std::log(t) / n);
                if (ucb1_value > best_value)
                {
                    best_is[1] = j;
                    best_value = ucb1_value;
                }
            }

            return this->child_nodeses[best_is[0]][best_is[1]];
        }

        Node(const State &state) : state_(state), w_(0), n_(0) {}
    };

    // 制限時間(ms)を指定してDUCTで指定したプレイヤーの行動を決定する
    Action ductAction(const State &state, const int player_id, const int playout_number)
    {
        Node root_node = Node(state);
        root_node.expand();
        for (int i = 0; i < playout_number; i++)
        {
            root_node.evaluate();
        }
        auto legal_actions = state.legalActions(player_id);
        int i_size = root_node.child_nodeses.size();
        int j_size = root_node.child_nodeses[0].size();

        if (player_id == 0)
        {
            int best_n = -1;
            int best_i = -1;
            for (int i = 0; i < i_size; i++)
            {
                int n = 0;
                for (int j = 0; j < j_size; j++)
                {
                    n += root_node.child_nodeses[i][j].n_;
                }
                // std::cout << dstr[legal_actions[i]] << " " << n << std::endl;
                if (n > best_n)
                {
                    best_i = i;
                    best_n = n;
                }
            }
            // std::cout <<"best\t" << dstr[legal_actions[best_i]] << " " << best_n << std::endl;

            return legal_actions[best_i];
        }
        else
        {
            int best_n = -1;
            int best_j = -1;
            for (int j = 0; j < j_size; j++)
            {
                int n = 0;
                for (int i = 0; i < i_size; i++)
                {
                    n += root_node.child_nodeses[i][j].n_;
                }
                if (n > best_n)
                {
                    best_j = j;
                    best_n = n;
                }
            }
            return legal_actions[best_j];
        }
    }

    // 制限時間(ms)を指定してDUCTで指定したプレイヤーの行動を決定する
    Action ductActionWithTimeThreshold(const State &state, const int player_id, const int64_t time_threshold)
    {
        Node root_node = Node(state);
        root_node.expand();
        auto time_keeper = TimeKeeper(time_threshold);
        for (int cnt = 0;; cnt++)
        {
            if (time_keeper.isTimeOver())
            {
                break;
            }
            root_node.evaluate();
        }
        auto legal_actions = state.legalActions(player_id);
        int i_size = root_node.child_nodeses.size();
        int j_size = root_node.child_nodeses[0].size();

        if (player_id == 0)
        {
            int best_n = -1;
            int best_i = -1;
            for (int i = 0; i < i_size; i++)
            {
                int n = 0;
                for (int j = 0; j < j_size; j++)
                {
                    n += root_node.child_nodeses[i][j].n_;
                }
                // std::cout << dstr[legal_actions[i]] << " " << n << std::endl;
                if (n > best_n)
                {
                    best_i = i;
                    best_n = n;
                }
            }
            // std::cout <<"best\t" << dstr[legal_actions[best_i]] << " " << best_n << std::endl;

            return legal_actions[best_i];
        }
        else
        {
            int best_n = -1;
            int best_j = -1;
            for (int j = 0; j < j_size; j++)
            {
                int n = 0;
                for (int i = 0; i < i_size; i++)
                {
                    n += root_node.child_nodeses[i][j].n_;
                }
                if (n > best_n)
                {
                    best_j = j;
                    best_n = n;
                }
            }
            return legal_actions[best_j];
        }
    }

}
using ::montecarlo::ductAction;
using ::montecarlo::ductActionWithTimeThreshold;
using ::montecarlo::primitiveMontecarloAction;

#include <iostream>
#include <functional>

using AIFunction = std::function<Action(const State &, const int)>;
using StringAIPair = std::pair<std::string, AIFunction>;
// ゲームを1回プレイしてゲーム状況を表示する
void playGame(const std::vector<StringAIPair> &ais)
{
    using std::cout;
    using std::endl;

    auto state = State(5, 5, mt());
    cout << state.toString() << endl;

    while (!state.isDone())
    {
        std::vector<Action> actions = {ais[0].second(state, 0), ais[1].second(state, 1)};
        cout << "actions " << dstr[actions[0]] << " " << dstr[actions[1]] << endl;
        state.advance(actions[0], actions[1]);
        cout << state.toString() << endl;
    }
}
// ゲームをgame_number回プレイしてaisの0番目のAIの勝率を表示する。
void testFirstPlayerWinRate(const std::vector<StringAIPair> &ais)
{
    using std::cout;
    using std::endl;

    using AIFunction = std::function<Action(const SimultaneousMazeState &, const int)>;
    using StringAIPair = std::pair<std::string, AIFunction>;
    const int kyoutu_playout_n = 1000;
    {
        double first_player_win_rate = 0;
        int game_number = 100;
        for (int i = 0; i < game_number; i++)
        {
            auto state = State(5, 5, mt());
            auto &first_ai = ais[0];
            auto &second_ai = ais[1];
            while (true)
            {
                state.advance(first_ai.second(state, 0), second_ai.second(state, 1));
                if (state.isDone())
                    break;
            }
            double win_rate_point = state.getFirstPlayerScoreForWinRate();
            if (win_rate_point >= 0)
            {
                state.toString();
            }
            first_player_win_rate += win_rate_point;

            cout << "i " << i << " w " << first_player_win_rate / (i + 1) << endl;
        }
        first_player_win_rate /= (double)game_number;
        cout << "Winning rate of " << ais[0].first << " to " << ais[1].first << ":\t" << first_player_win_rate << endl;
    }
}

int main()
{
    std::vector<StringAIPair> ais = {
        // StringAIPair("randomAction",[](const State& state,const int player_id) {return randomAction(state,player_id); }),
        StringAIPair("primitiveMontecarloAction", [&](const State &state, const int player_id)
                     { return primitiveMontecarloAction(state, player_id, 1000); }),
        StringAIPair("ductAction", [&](const State &state, const int player_id)
                     { return ductAction(state, player_id, 1000); }),
        // StringAIPair("ductActionWithTimeThreshold",[&](const State& state,const int player_id) {return ductActionWithTimeThreshold(state,player_id,10); }),
    };
    playGame(ais);
    // testFirstPlayerWinRate(ais);
    return 0;
}