/*
    GomokuGame.h
        定义了棋局，以及棋盘对弈策略
                策略包括随机下子，人类玩家控制下子，回放下子，AI下子，网络玩家下子。
                                    by 石响宇 2020.07.07
                网络部分 by 包慧敏
*/


#pragma once


#include <Windows.h>
#include <mmsystem.h>
#include <algorithm>
#include <random>
#include <functional>
#include <thread>
#include <future>
#include "GomokuAI.h"
#include <SDL2/SDL_net.h>

#define GOMOKU_GRID_SIZE 15
#define GOMOKU_PLAYER_WHITE 0
#define GOMOKU_PLAYER_BLACK 1

class GomokuGame {
    bool gameEnd = false;
public:
    int grid[GOMOKU_GRID_SIZE][GOMOKU_GRID_SIZE];
    
    struct Step {
        int player;
        int x, y;
    };
    Step steps[226];
    int stepCounter = 0;
    int currentPlayer = 1;

    //int consideringX, consideringY;

    GomokuGame() {
        clear();
    }

    void clear() {
        for (int i = 0; i < GOMOKU_GRID_SIZE; i++)
            std::fill(grid[i], grid[i] + GOMOKU_GRID_SIZE, -1);
        stepCounter = 0;
      
        currentPlayer = 1;
        gameEnd = false;
    }
    void switchPlayer(int p = -1) {
        if (p >= 0)
            currentPlayer = p & 1;
        else currentPlayer ^= 1;
    }

    void save(const std::string& filename) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            fwrite(&stepCounter, 4, 1, fp);
            fwrite(steps, sizeof(Step), 226, fp);
            fclose(fp);
        }
    }

    void load(const std::string& filename) {
        FILE* fp = fopen(filename.c_str(), "rb"); 
        if (fp) {
            fread(&stepCounter, 4, 1, fp);
            fread(steps, sizeof(Step), 226, fp);
            fclose(fp);
        }
    }

    bool put(int player, int x, int y);

    bool isGameEnd() const{
        return gameEnd;
    }


    struct WinLossInfo{
        bool win = false;
        int x, y;
        int dir;
        enum {
            LEFT2RIGHT,      //从左到右
            TOP2DOWN,        //从上到下
            TOPLEFT2DOWNRIGHT,  //从左上到右下
            TOPRIGHT2DOWNLEFT   //从右上到左下
        };
    };
    void checkWinLoss(int player, WinLossInfo *wl);
};

class GomokuPlayer {
    int playerid;
public:
    GomokuPlayer(int i){
        playerid = i;
    }
    int getId() const {
        return playerid;
    }
    typedef int GridVector[15];
    
    virtual void makeOutNext(GridVector* chessGrid, int lastRivalX, int lastRivalY, int& x, int& y) = 0;
    virtual void onPutChess(int lastRivalX, int lastRivalY) {};
};

//机器玩家，策略：随机下子(内部测试使用，游戏中不开放此玩法)
class GomokuRandomPlayer : public GomokuPlayer {
    std::default_random_engine re;
    int ipos[225], jpos[225];
public:
    GomokuRandomPlayer(int i) : GomokuPlayer(i) {
        re.seed(timeGetTime());
    }

    //在棋盘上随机下子
    virtual void makeOutNext(GridVector* chessGrid, int, int, int& x, int& y);
};


//人类玩家，通过鼠标控制下子
class GomokuHumanPlayer : public GomokuPlayer {
public:
    GomokuHumanPlayer(int i, const std::function<bool(int &, int &)>& mouseFunc) : GomokuPlayer(i) {
        mouseClickDetector = mouseFunc;
    }

    virtual void makeOutNext(GridVector* chessGrid, int, int, int& x, int& y);
    std::function<bool(int&, int&)> mouseClickDetector;
};

//机器玩家，通过回放对局记录下子
class GomokuReplayer : public GomokuPlayer{
    GomokuGame game;
    int repId;
public:
    GomokuReplayer(int i, const std::string &filename) : GomokuPlayer(i) {
        game.load(filename);
        repId = (i & 1) ? 0 : 1;
    }
    virtual void makeOutNext(GridVector*, int, int, int& x, int& y) {
      
        if(repId < game.stepCounter) {
            x = game.steps[repId].x;
            y = game.steps[repId].y;
            repId += 2;
        } else {
            //实际上执行不到这里
        }
    }
};


//机器玩家，人工智能算法下子
class GomokuAIPlayer : public GomokuPlayer {
    
    std::unique_ptr<GomokuAI::GomokuAI> ai;
    int nextX, nextY;
public:
    GomokuAIPlayer(int i, int level = 6) : GomokuPlayer(i) {
        ai.reset(new GomokuAI::GomokuAI(level));
    }
    virtual void makeOutNext(GridVector* chessGrid, int, int, int& x, int& y);
};

class GomokuNetworkPlayer : public GomokuPlayer {
    bool isServer;
    std::string serverAddr;
    int serverPort;
    

    std::atomic<bool> serverWorking;
    std::atomic<bool> gameWorking;
    TCPsocket serverSocket0 = nullptr, serverSocket1 = nullptr;
    TCPsocket remoteSocket0 = nullptr, remoteSocket1 = nullptr;

    struct TransmitData {
        int x, y;
    };

public:
    int checkConnection() const {
        return remoteSocket0 != nullptr && remoteSocket1 != nullptr;
    }
    GomokuNetworkPlayer(int playerid, const std::string& addr, int port, bool server);
    virtual void makeOutNext(GridVector* chessGrid, int, int, int& x, int& y);
    ~GomokuNetworkPlayer();
    virtual void onPutChess(int lastRivalX, int lastRivalY);
};