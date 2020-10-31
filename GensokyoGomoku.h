/*
    GensokyoGomoku.h
        五子棋界面部分主要实现
                by 石响宇 2020.07.06
*/


#pragma once

#include <QtWidgets/QMainWindow>
#include <QtGui/QCloseEvent>
#include "ui_GensokyoGomoku.h"
#include <Windows.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtOpenGL/QGLWidget>
#include "AnimationProc.h"
#include <memory>
#include "GomokuGame.h"
#include "Audio.h"
#include "ConnectPlayUI.h"

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0) 
#define KEYUP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define SCENE_WIDTH 800
#define SCENE_HEIGHT 800 

class GameSE {
    std::unique_ptr<SoundEffect> sePutChess, seWin, seLoss;
public:
    GameSE() {
        sePutChess.reset(new SoundEffect(u8"./Resources/Sound/落子音效.mp3"));
        seWin.reset(new SoundEffect(u8"./Resources/Sound/胜利音效.mp3"));
        seLoss.reset(new SoundEffect(u8"./Resources/Sound/失败音效.mp3"));
    }

    void playPutChess() const {
        Audio::playSE(sePutChess.get());
    }
    void playWin() const {
        Audio::playSE(seWin.get());
    }
    void playLoss() const {
        Audio::playSE(seLoss.get());
    }
};

class GensokyoGomoku : public QMainWindow
{
    //Q_OBJECT
    HWND centralHWND;

public:

    GensokyoGomoku(QWidget *parent = Q_NULLPTR);

    void Mainloop();


    void OnExit() {
        ExitProcess(0);
    }

    void runLater(const std::function<void(void)>& f) {
        todoList.push_back(f);
    }

    void addAnimation(const std::shared_ptr<AnimationProc>& ani, int layer) {
        assert(layer >= 0 && layer <= 9);
        animations[layer].push_back(ani);
    }

    void setupGame();

private:
    Ui::GensokyoGomokuClass ui;
    std::vector<std::function<void(void)>> todoList;
    std::vector< std::function<void(int, int)>> mouseEventListenser;
    std::vector<std::shared_ptr<AnimationProc>> animations[10];
    std::unique_ptr<QPainter> painter;
    std::unique_ptr<QPixmap> frameBuffer;
    GameSE gameSE;

    QThread *gameDisplayThread;

    double lastTime;
    int frameCount = 0;

    struct {
        std::shared_ptr<AnimationProc> aniDrawBoard;
        std::shared_ptr<AnimationProc> aniDrawChess;
        std::shared_ptr<AnimationProc> aniDrawState;
    }drawAnimations;
    std::shared_ptr<AnimationProc> aniGameProc;

    struct { 
        std::unique_ptr<GomokuPlayer> white, black;
    }players;

protected:
    virtual void closeEvent(QCloseEvent* event) override {
        OnExit();
        event->accept();
    }
    virtual void paintEvent(QPaintEvent* event) override;
public:
    GomokuGame game;
    enum { 
        GAME_READY,
        GAME_PVCFH,   //本地人机对战，人先手
        GAME_PVCAH,   //本地人机对战，人后手
        GAME_PVP,     //本地人人对战
        GAME_PVP_ONLINE_FH,   //联机人人对战 - 先手
        GAME_PVP_ONLINE_AH,   //联机人人对战 - 后手
        GAME_REPLAY,  //回放

        GAME_STOP
    };
    
    int gamePhase = GAME_READY;
    int gameSelfPlayer = -1;
    int gameAILevel = 0;
    std::string replayFileName;

    struct {
        const int x = 100, y = 150, w = 600, h = 600;
    }chessClient;
    struct {
        int x = -1, y = -1;
    }currentChessPosition;

    bool isGameEnd() {
        return game.isGameEnd() || gamePhase == GAME_READY;
    }

private:
    Coroutine* coroutineMouseInput;
    void mouseToChess(int mx, int my, int& x, int& y, bool& valid);

    void onPutChess(int player, int cx, int cy);

    void onStartGame(int gameMode);
    void onTerminateGame();

protected:
    void getLocalMousePos(int &mx, int &my, bool &valid);

private:
    std::unique_ptr<ConnectPlayUI> connectUI;
    std::unique_ptr<GomokuNetworkPlayer> networkPlayer;
};

