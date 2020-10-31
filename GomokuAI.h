/*
    GomokuAI.h
        GomokuAI��ʵ��
            ���� by ʯ���� 2020.07.08
            �㷨�о� by ������
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
    CHESS_WIN = 1,             //��������
    CHESS_LOSS,                //��������(�з�)
    CHESS_FLEX4,               //��4
    CHESS_FLEX4_INV,           //��4(�з�)
    CHESS_BLOCK4,              //��4
    CHESS_BLOCK4_INV,          //��4(�з�)
    CHESS_FLEX3,               //��3
    CHESS_FLEX3_INV,           //��3(�з�)
    CHESS_BLOCK3,              //��3
    CHESS_BLOCK3_INV,          //��3(�з�)
    CHESS_FLEX2,               //��2
    CHESS_FLEX2_INV,           //��2(�з�)
    CHESS_BLOCK2,              //��2
    CHESS_BLOCK2_INV,          //��2(�з�)
    CHESS_FLEX1,               //��1
    CHESS_FLEX1_INV,           //��1(�з�)
    CHESS_WEIGHT_END
};
constexpr int GOMOKUAI_SELF_CHESS = 1;    //��1��ʾ�Լ�������(AI�Լ�������)
constexpr int GOMOKUAI_RIVAL_CHESS = 2;    //��2��ʾ�з�������

static const int initialWeights[CHESS_WEIGHT_END] = { 0, 4000, -4000,  //��������
                                                        2000, -2000,  //��4
                                                        1000, -1000,  //��4
                                                        1000, -1000,  //��3
                                                         400,  -600,  //��3
                                                         400,  -600,  //��2
                                                         100,  -150,  //��2
                                                         100,  -150   //��1
};

class GomokuAI {
    int grid[15][15];
    using GridVector = int[15];

    //��������
    void copy(GridVector* dest, GridVector* src, bool inv = false) {
        int cmap[3];
        if (inv) cmap[0] = 0, cmap[1] = 2, cmap[2] = 1;
        else cmap[0] = 0, cmap[1] = 1, cmap[2] = 2;
        for (int i = 0; i < 15; i++) for (int j = 0; j < 15; j++) {
            dest[i][j] = cmap[src[i][j]];
        }
    }
#define N 3
    int eval[N][N][N][N][N][N];  //�������ɷ�ʽ���������
#undef N
    void initParameters() {
        memset(eval, 0, sizeof(eval));
        //�ҷ����� 
        eval[1][1][1][1][1][1] = CHESS_WIN;
        eval[1][1][1][1][1][0] = CHESS_WIN;
        eval[0][1][1][1][1][1] = CHESS_WIN;
        eval[1][1][1][1][1][2] = CHESS_WIN;
        eval[2][1][1][1][1][1] = CHESS_WIN;
        //�з�����   
        eval[2][2][2][2][2][2] = CHESS_LOSS;
        eval[2][2][2][2][2][0] = CHESS_LOSS;
        eval[0][2][2][2][2][2] = CHESS_LOSS;
        eval[2][2][2][2][2][1] = CHESS_LOSS;
        eval[1][2][2][2][2][2] = CHESS_LOSS;
        //�ҷ���4
        eval[0][1][1][1][1][0] = CHESS_FLEX4;
        //�з���4
        eval[0][2][2][2][2][0] = CHESS_FLEX4_INV;
        //�ҷ���3
        eval[0][1][1][1][0][0] = CHESS_FLEX3;
        eval[0][1][1][0][1][0] = CHESS_FLEX3;
        eval[0][1][0][1][1][0] = CHESS_FLEX3;
        eval[0][0][1][1][1][0] = CHESS_FLEX3;
        //�з���3
        eval[0][2][2][2][0][0] = CHESS_FLEX3_INV;
        eval[0][2][2][0][2][0] = CHESS_FLEX3_INV;
        eval[0][2][0][2][2][0] = CHESS_FLEX3_INV;
        eval[0][0][2][2][2][0] = CHESS_FLEX3_INV;
        //�ҷ���2
        eval[0][1][1][0][0][0] = CHESS_FLEX2;
        eval[0][1][0][1][0][0] = CHESS_FLEX2;
        eval[0][1][0][0][1][0] = CHESS_FLEX2;
        eval[0][0][1][1][0][0] = CHESS_FLEX2;
        eval[0][0][1][0][1][0] = CHESS_FLEX2;
        eval[0][0][0][1][1][0] = CHESS_FLEX2;
        //�з���2
        eval[0][2][2][0][0][0] = CHESS_FLEX2_INV;
        eval[0][2][0][2][0][0] = CHESS_FLEX2_INV;
        eval[0][2][0][0][2][0] = CHESS_FLEX2_INV;
        eval[0][0][2][2][0][0] = CHESS_FLEX2_INV;
        eval[0][0][2][0][2][0] = CHESS_FLEX2_INV;
        eval[0][0][0][2][2][0] = CHESS_FLEX2_INV;
        //�ҷ���1
        for (int i = 1; i <= 4; i++)
            eval[0][i == 1][i == 2][i == 3][i == 4][0] = CHESS_FLEX1;
        //�з���1
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

                                //��4
                                GEN_CODE(4, CHESS_BLOCK4, CHESS_BLOCK4_INV);
                                //��3
                                GEN_CODE(3, CHESS_BLOCK3, CHESS_BLOCK3_INV);
                                //��2
                                GEN_CODE(2, CHESS_BLOCK2, CHESS_BLOCK2_INV);
#undef GEN_CODE            
                            }
    }

    void inspire(GridVector* A, int& score, bool& winloss) {
        score = 0;
        winloss = false;
        int status[4][CHESS_WEIGHT_END] = { 0 };

        //������
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 10; j++) {
                const int s = eval[A[i][j]][A[i][j + 1]][A[i][j + 2]][A[i][j + 3]][A[i][j + 4]][A[i][j + 5]];
                status[0][s]++;
            }
        }

        //�������
        for (int j = 0; j < 15; j++) {
            for (int i = 0; i < 10; i++) {
                const int s = eval[A[i][j]][A[i + 1][j]][A[i + 2][j]][A[i + 3][j]][A[i + 4][j]][A[i + 5][j]];
                status[1][s]++;
            }
        }

        //���ϵ�����
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                const int s = eval[A[i][j]][A[i + 1][j + 1]][A[i + 2][j + 2]][A[i + 3][j + 3]][A[i + 4][j + 4]][A[i + 5][j + 5]];
                status[2][s]++;
            }
        }

        //���ϵ�����
        for (int i = 14; i >= 5; i--) {
            for (int j = 0; j < 10; j++) {
                const int s = eval[A[i][j]][A[i - 1][j + 1]][A[i - 2][j + 2]][A[i - 3][j + 3]][A[i - 4][j + 4]][A[i - 5][j + 5]];
                status[3][s]++;
            }
        }
        int statusCount[CHESS_WEIGHT_END] = { 0 };
        int acc = 0;
        //���ݾ��水Ȩֵ�ۼӵ÷�
        for (int i = 1; i < CHESS_WEIGHT_END; i++) {
            acc += (status[0][i] + status[1][i] + status[2][i] + status[3][i]) * initialWeights[i];
            statusCount[i] = (status[0][i] > 0) + (status[1][i] > 0) + (status[2][i] > 0) + (status[3][i] > 0);
        }

        //���ӷ�
        if (statusCount[CHESS_WIN] > 0) {
            acc += 100000;   //��һ����ʤ
            winloss = true;
        }
        else if (statusCount[CHESS_LOSS] > 0) {
            acc -= 100000;   //��һ���ذ�
            winloss = true;
        }
        else if (statusCount[CHESS_FLEX4_INV] > 0) { //���ֳ��ֻ�4��������ذ�
            acc -= 50000;
        }
        else if (statusCount[CHESS_BLOCK4_INV] > 0) {  //���ֳ��ֳ�4
            acc -= 30000;
        }
        else if (!statusCount[CHESS_FLEX4_INV] && !statusCount[CHESS_BLOCK4_INV]) {  //�����޻�4����4
            int k = 0;
            //���� ���Ļ���
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    if (i != j)
                        k += status[i][CHESS_BLOCK4] * status[j][CHESS_FLEX3];

            //�ҷ���4
            if (statusCount[CHESS_FLEX4]) {
                acc += 20000;
            }
            else if (statusCount[CHESS_BLOCK4] >= 2) { //����˫��4
                acc += 20000;
            }
            else if (k > 0) { //��4��3
                acc += 20000;
            }
            else if (statusCount[CHESS_FLEX3_INV] && !statusCount[CHESS_BLOCK4]) {  //�����л�3���ҷ�û�г�4
                acc -= 20000;
            }
            else if (!statusCount[CHESS_FLEX3_INV] && statusCount[CHESS_FLEX3] >= 2) {  //����û�л�3�����ҷ���˫��3
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
                    if (A[i][j]) {  //���A[i][j]�������ӣ���ô��������Χ3�㷶Χ������
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
            //�����ڿյ㴦���Ӳ����� 
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

            //ɸѡ������ߵ�NEXT_STATE����
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
            if ((level & 1) == 0) { //�ҷ�����
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
            else {  //�Է�����
                Points p;
                GridVector invA[15];
                copy(invA, A, true);
                evaluateNext(invA, p);  //�Եз������������ӵ�
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
