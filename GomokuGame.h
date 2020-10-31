/*
    GomokuGame.h
        ��������֣��Լ����̶��Ĳ���
                ���԰���������ӣ�������ҿ������ӣ��ط����ӣ�AI���ӣ�����������ӡ�
                                    by ʯ���� 2020.07.07
                ���粿�� by ������
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
            LEFT2RIGHT,      //������
            TOP2DOWN,        //���ϵ���
            TOPLEFT2DOWNRIGHT,  //�����ϵ�����
            TOPRIGHT2DOWNLEFT   //�����ϵ�����
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

//������ң����ԣ��������(�ڲ�����ʹ�ã���Ϸ�в����Ŵ��淨)
class GomokuRandomPlayer : public GomokuPlayer {
    std::default_random_engine re;
    int ipos[225], jpos[225];
public:
    GomokuRandomPlayer(int i) : GomokuPlayer(i) {
        re.seed(timeGetTime());
    }

    //���������������
    virtual void makeOutNext(GridVector* chessGrid, int, int, int& x, int& y);
};


//������ң�ͨ������������
class GomokuHumanPlayer : public GomokuPlayer {
public:
    GomokuHumanPlayer(int i, const std::function<bool(int &, int &)>& mouseFunc) : GomokuPlayer(i) {
        mouseClickDetector = mouseFunc;
    }

    virtual void makeOutNext(GridVector* chessGrid, int, int, int& x, int& y);
    std::function<bool(int&, int&)> mouseClickDetector;
};

//������ң�ͨ���طŶԾּ�¼����
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
            //ʵ����ִ�в�������
        }
    }
};


//������ң��˹������㷨����
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