/*
    GomokuAI.h
        GomokuAI的实现
            代码 by 石响宇 2020.07.08
            算法研究 by 罗叙卿
*/
#pragma once

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstddef>
#include <algorithm>
#include <vector>

#define GomokuAI_Begin namespace GomokuAI {
#define GomokuAI_End }

GomokuAI_Begin

constexpr int NEXT_STATES = 7;

enum {
    CHESS_WIN = 1,             //五子连珠
    CHESS_LOSS,                //五子连珠(敌方)
    CHESS_FLEX4,               //活4
    CHESS_FLEX4_INV,           //活4(敌方)
    CHESS_BLOCK4,              //冲4
    CHESS_BLOCK4_INV,          //冲4(敌方)
    CHESS_FLEX3,               //活3
    CHESS_FLEX3_INV,           //活3(敌方)
    CHESS_BLOCK3,              //冲3
    CHESS_BLOCK3_INV,          //冲3(敌方)
    CHESS_FLEX2,               //活2
    CHESS_FLEX2_INV,           //活2(敌方)
    CHESS_BLOCK2,              //冲2
    CHESS_BLOCK2_INV,          //冲2(敌方)
    CHESS_FLEX1,               //活1
    CHESS_FLEX1_INV,           //活1(敌方)
    CHESS_WEIGHT_END
};
constexpr int GOMOKUAI_SELF_CHESS = 1;    //用1表示自己的棋子(AI自己的棋子)
constexpr int GOMOKUAI_RIVAL_CHESS = 2;    //用2表示敌方的棋子

static const int initialWeights[CHESS_WEIGHT_END] = { 0, 4000, -4000,  //五子连珠
                                                        2000, -2000,  //活4
                                                        1000, -1000,  //冲4
                                                        1000, -1000,  //活3
                                                         400,  -600,  //冲3
                                                         400,  -600,  //活2
                                                         100,  -150,  //冲2
                                                         100,  -150   //活1
};

class GomokuAI {
    int grid[15][15];
    using GridVector = int[15];

    //拷贝棋盘
    void copy(GridVector* dest, GridVector* src, bool inv = false) {
        int cmap[3];
        if (inv) cmap[0] = 0, cmap[1] = 2, cmap[2] = 1;
        else cmap[0] = 0, cmap[1] = 1, cmap[2] = 2;
        for (int i = 0; i < 15; i++) for (int j = 0; j < 15; j++) {
            dest[i][j] = cmap[src[i][j]];
        }
    }
#define N 3
    int eval[N][N][N][N][N][N];  //棋子连成方式检测用数组
#undef N
    void initParameters() {
        memset(eval, 0, sizeof(eval));
        //我方五连 
        eval[1][1][1][1][1][1] = CHESS_WIN;
        eval[1][1][1][1][1][0] = CHESS_WIN;
        eval[0][1][1][1][1][1] = CHESS_WIN;
        eval[1][1][1][1][1][2] = CHESS_WIN;
        eval[2][1][1][1][1][1] = CHESS_WIN;
        //敌方五连   
        eval[2][2][2][2][2][2] = CHESS_LOSS;
        eval[2][2][2][2][2][0] = CHESS_LOSS;
        eval[0][2][2][2][2][2] = CHESS_LOSS;
        eval[2][2][2][2][2][1] = CHESS_LOSS;
        eval[1][2][2][2][2][2] = CHESS_LOSS;
        //我方活4
        eval[0][1][1][1][1][0] = CHESS_FLEX4;
        //敌方活4
        eval[0][2][2][2][2][0] = CHESS_FLEX4_INV;
        //我方活3
        eval[0][1][1][1][0][0] = CHESS_FLEX3;
        eval[0][1][1][0][1][0] = CHESS_FLEX3;
        eval[0][1][0][1][1][0] = CHESS_FLEX3;
        eval[0][0][1][1][1][0] = CHESS_FLEX3;
        //敌方活3
        eval[0][2][2][2][0][0] = CHESS_FLEX3_INV;
        eval[0][2][2][0][2][0] = CHESS_FLEX3_INV;
        eval[0][2][0][2][2][0] = CHESS_FLEX3_INV;
        eval[0][0][2][2][2][0] = CHESS_FLEX3_INV;
        //我方活2
        eval[0][1][1][0][0][0] = CHESS_FLEX2;
        eval[0][1][0][1][0][0] = CHESS_FLEX2;
        eval[0][1][0][0][1][0] = CHESS_FLEX2;
        eval[0][0][1][1][0][0] = CHESS_FLEX2;
        eval[0][0][1][0][1][0] = CHESS_FLEX2;
        eval[0][0][0][1][1][0] = CHESS_FLEX2;
        //敌方活2
        eval[0][2][2][0][0][0] = CHESS_FLEX2_INV;
        eval[0][2][0][2][0][0] = CHESS_FLEX2_INV;
        eval[0][2][0][0][2][0] = CHESS_FLEX2_INV;
        eval[0][0][2][2][0][0] = CHESS_FLEX2_INV;
        eval[0][0][2][0][2][0] = CHESS_FLEX2_INV;
        eval[0][0][0][2][2][0] = CHESS_FLEX2_INV;
        //我方活1
        for (int i = 1; i <= 4; i++)
            eval[0][i == 1][i == 2][i == 3][i == 4][0] = CHESS_FLEX1;
        //敌方活1
        for (int i = 1; i <= 4; i++) {
            eval[0][i == 1 ? 2 : 0][i == 2 ? 2 : 0][i == 3 ? 2 : 0][i == 4 ? 2 : 0][0] = CHESS_FLEX1_INV;
        }
        int pos[7] = { 0 };
        for (pos[1] = 0; pos[1] <= 2; pos[1]++)
            for (pos[2] = 0; pos[2] <= 2; pos[2]++)
                for (pos[3] = 0; pos[3] <= 2; pos[3]++)
                    for (pos[4] = 0; pos[4] <= 2; pos[4]++)
                        for (pos[5] = 0; pos[5] <= 2; pos[5]++)
                            for (pos[6] = 0; pos[6] <= 2; pos[6]++) {
                                int sx = 0, sy = 0, rx = 0, ry = 0;

                                if (pos[1] == 1) sx++;
                                else if (pos[1] == 2) sy++;
                                for (int i = 2; i <= 5; i++) {
                                    if (pos[i] == 1) sx++, rx++;
                                    else if (pos[i] == 2) sy++, ry++;
                                }
                                if (pos[6] == 1) rx++;
                                else if (pos[6] == 2) ry++;

#define GEN_CODE(x, a, b) if(((sx == x && sy == 0) || (rx == x && ry == 0)) && !eval[pos[1]][pos[2]][pos[3]][pos[4]][pos[5]][pos[6]]) { \
                eval[pos[1]][pos[2]][pos[3]][pos[4]][pos[5]][pos[6]] = a;   \
            } \
            if(((sx == 0 && sy == x) || (rx == 0 && ry == x)) && !eval[pos[1]][pos[2]][pos[3]][pos[4]][pos[5]][pos[6]]) { \
                eval[pos[1]][pos[2]][pos[3]][pos[4]][pos[5]][pos[6]] = b;   \
            }

                                //冲4
                                GEN_CODE(4, CHESS_BLOCK4, CHESS_BLOCK4_INV);
                                //冲3
                                GEN_CODE(3, CHESS_BLOCK3, CHESS_BLOCK3_INV);
                                //冲2
                                GEN_CODE(2, CHESS_BLOCK2, CHESS_BLOCK2_INV);
#undef GEN_CODE            
                            }
    }

    void inspire(GridVector* A, int& score, bool& winloss) {
        score = 0;
        winloss = false;
        int status[4][CHESS_WEIGHT_END] = { 0 };

        //检查横向
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 10; j++) {
                const int s = eval[A[i][j]][A[i][j + 1]][A[i][j + 2]][A[i][j + 3]][A[i][j + 4]][A[i][j + 5]];
                status[0][s]++;
            }
        }

        //检查竖向
        for (int j = 0; j < 15; j++) {
            for (int i = 0; i < 10; i++) {
                const int s = eval[A[i][j]][A[i + 1][j]][A[i + 2][j]][A[i + 3][j]][A[i + 4][j]][A[i + 5][j]];
                status[1][s]++;
            }
        }

        //左上到右下
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                const int s = eval[A[i][j]][A[i + 1][j + 1]][A[i + 2][j + 2]][A[i + 3][j + 3]][A[i + 4][j + 4]][A[i + 5][j + 5]];
                status[2][s]++;
            }
        }

        //右上到左下
        for (int i = 14; i >= 5; i--) {
            for (int j = 0; j < 10; j++) {
                const int s = eval[A[i][j]][A[i - 1][j + 1]][A[i - 2][j + 2]][A[i - 3][j + 3]][A[i - 4][j + 4]][A[i - 5][j + 5]];
                status[3][s]++;
            }
        }
        int statusCount[CHESS_WEIGHT_END] = { 0 };
        int acc = 0;
        //根据局面按权值累加得分
        for (int i = 1; i < CHESS_WEIGHT_END; i++) {
            acc += (status[0][i] + status[1][i] + status[2][i] + status[3][i]) * initialWeights[i];
            statusCount[i] = (status[0][i] > 0) + (status[1][i] > 0) + (status[2][i] > 0) + (status[3][i] > 0);
        }

        //附加分
        if (statusCount[CHESS_WIN] > 0) {
            acc += 100000;   //下一步必胜
            winloss = true;
        }
        else if (statusCount[CHESS_LOSS] > 0) {
            acc -= 100000;   //下一步必败
            winloss = true;
        }
        else if (statusCount[CHESS_FLEX4_INV] > 0) { //对手出现活4，两步后必败
            acc -= 50000;
        }
        else if (statusCount[CHESS_BLOCK4_INV] > 0) {  //对手出现冲4
            acc -= 30000;
        }
        else if (!statusCount[CHESS_FLEX4_INV] && !statusCount[CHESS_BLOCK4_INV]) {  //对手无活4，冲4
            int k = 0;
            //检验 冲四活三
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    if (i != j)
                        k += status[i][CHESS_BLOCK4] * status[j][CHESS_FLEX3];

            //我方活4
            if (statusCount[CHESS_FLEX4]) {
                acc += 20000;
            }
            else if (statusCount[CHESS_BLOCK4] >= 2) { //至少双冲4
                acc += 20000;
            }
            else if (k > 0) { //冲4活3
                acc += 20000;
            }
            else if (statusCount[CHESS_FLEX3_INV] && !statusCount[CHESS_BLOCK4]) {  //对手有活3而我方没有冲4
                acc -= 20000;
            }
            else if (!statusCount[CHESS_FLEX3_INV] && statusCount[CHESS_FLEX3] >= 2) {  //对手没有活3，而我方有双活3
                acc += 10000;
            }
        }
        score = acc;
    }
    struct Points {
        int x[NEXT_STATES], y[NEXT_STATES], score[NEXT_STATES], exist[NEXT_STATES];
    };
    void evaluateNext(GridVector* A, Points& ps) {
        for (int i = 0; i < NEXT_STATES; i++)
            ps.exist[i] = 0;
        int score;
        bool winloss;
        inspire(A, score, winloss);
        GridVector B[15] = { 0 }, value[15] = { 0 };

        if (winloss) {
            ps.exist[0] = 1;
            ps.score[0] = score;
            for (int i = 0; i < 15; i++)
                for (int j = 0; j < 15; j++)
                    if (A[i][j] == 1) {
                        ps.x[0] = i;
                        ps.y[0] = j;
                        goto evaluate_end;
                    }
        }
        else {
            for (int i = 0; i < 15; i++) {
                for (int j = 0; j < 15; j++) {
                    if (A[i][j]) {  //如果A[i][j]处有棋子，那么考虑它周围3层范围内落子
                        for (int k = -3; k <= 3; k++) {
                            if (i + k >= 0 && i + k < 15) {
                                B[i + k][j] = 1;
                                if (j + k >= 0 && j + k < 15) {
                                    B[i + k][j + k] = 1;
                                }
                                if (j - k >= 0 && j - k < 15) {
                                    B[i + k][j - k] = 1;
                                }
                            }
                            if (j + k >= 0 && j + k < 15) {
                                B[i][j + k] = 1;
                            }
                        }
                    }
                }
            }
            //考虑在空点处落子并评估 
            for (int i = 0; i < 15; i++) for (int j = 0; j < 15; j++) {
                value[i][j] = -1000000;
                if (A[i][j] == 0 && B[i][j] == 1) {
                    A[i][j] = 1;
                    int v;
                    bool w;
                    inspire(A, v, w);
                    value[i][j] = v;
                    A[i][j] = 0;
                }
            }

            //筛选估分最高的NEXT_STATE个点
            int v = -1000000;
            for (int k = 0; k < NEXT_STATES; k++) {
                v = -1000000;
                for (int i = 0; i < 15; i++) for (int j = 0; j < 15; j++) {
                    if (value[i][j] > v) {
                        v = value[i][j];
                        ps.x[k] = i;
                        ps.y[k] = j;
                    }
                }
                if (k > 0 && (ps.score[0] - v > 3000))
                    break;
                ps.score[k] = v;
                ps.exist[k] = 1;
                value[ps.x[k]][ps.y[k]] = -1000000;
            }
        }

    evaluate_end:
        return;
    }

    int searchLevelLimits = 6;

public:
    struct NextState {
        int x, y, score;
    };
private:
    NextState tmpState;
public:
    GomokuAI(int level = 6) {
        clearGrid();
        initParameters();
        if (level > 0)
            searchLevelLimits = level;
    }
    void clearGrid() {
        memset(grid, 0, sizeof(grid));
    }
    void placeAt(int x, int y, int c) {
        if (x < 0 || y < 0 || y >= 15 || y >= 15) {
            //throw std::runtime_error("Invalid placeAt position");
            return;
        }
        grid[x][y] = c;
    }

    int alphaBeta(GridVector* A, int level, int alpha, int beta) {
        if (level == searchLevelLimits) {
            Points p;
            evaluateNext(A, p);
            return p.score[0];
        }
        else {
            if ((level & 1) == 0) { //我方决策
                Points p;
                evaluateNext(A, p);
                GridVector tmp[15];

                for (int i = 0; i < NEXT_STATES && p.exist[i] && beta > alpha; i++) {
                    copy(tmp, A);
                    tmp[p.x[i]][p.y[i]] = 1;
                    int score = alphaBeta(tmp, level + 1, alpha, beta);
                    if (score > alpha) {
                        alpha = score;
                        if (level == 0) {
                            tmpState.x = p.x[i];
                            tmpState.y = p.y[i];
                            tmpState.score = score;
                        }
                    }
                }
                return alpha;
            }
            else {  //对方决策
                Points p;
                GridVector invA[15];
                copy(invA, A, true);
                evaluateNext(invA, p);  //对敌方最有利的落子点
                GridVector tmp[15];

                for (int i = 0; i < NEXT_STATES && p.exist[i] && beta > alpha; i++) {
                    copy(tmp, A);
                    tmp[p.x[i]][p.y[i]] = 2;
                    int score = alphaBeta(tmp, level + 1, alpha, beta);
                    if (score < beta)
                        beta = score;
                }
                return beta;
            }
        }
    }

    NextState makeOutNext() {
        bool flag = false;
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                if (grid[i][j]) {
                    flag = true;
                    break;
                }
            }
        }
        if (flag) {
            alphaBeta(grid, 0, -1000000, 1000000);
        }
        else {
            tmpState.x = 7;
            tmpState.y = 7;
            tmpState.score = 0;
        }
        return tmpState;
    }
};

GomokuAI_End
