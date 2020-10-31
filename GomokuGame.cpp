#include "GomokuGame.h"
#include "GomokuAI.h"
#include <sstream>
#include <string>

#include "Coroutine.h"

bool GomokuGame::put(int player, int x, int y) {
    if (grid[x][y] == -1) {
        auto s = &steps[stepCounter++];
        s->player = player;
        s->x = x;
        s->y = y;
        grid[x][y] = player;

        if (stepCounter >= GOMOKU_GRID_SIZE * GOMOKU_GRID_SIZE)
            gameEnd = true;

        return true;
    }
    else {
        return false;
    }
}

void GomokuGame::checkWinLoss(int player, WinLossInfo *wl) {

    auto flag = false;
    //check horizon
    for (auto i = 0; i < 15; i++) {
        for (auto j = 0; j <= 10; j++) {
            auto t = true;
            for (auto k = 0; k < 5; k++) {
                if (grid[i][j + k] != player) {
                    t = false;
                    break;
                }
            }
            if (t) {
                wl->win = true;
                wl->x = j;
                wl->y = i;
                wl->dir = WinLossInfo::LEFT2RIGHT;
                flag = true;
                break;
            }
        }
        if (flag) break;
    }

    //check vertical
    if (!flag) {
        for (auto j = 0; j < 15; j++) {
            for (auto i = 0; i <= 10; i++) {
                auto t = true;
                for (auto k = 0; k < 5; k++) {
                    if (grid[i + k][j] != player) {
                        t = false;
                        break;
                    }
                }
                if (t) {
                    wl->win = true;
                    wl->x = j;
                    wl->y = i;
                    wl->dir = WinLossInfo::TOP2DOWN;
                    flag = true;
                    break;
                }
            }
            if (flag) break;
        }
    }

    //check TOPRIGHT2DOWNLEFT
    if (!flag) {
        for (auto i = 0; i <= 10; i++) {
            for (auto j = 4; j <= 14; j++) {
                auto t = true;
                for (auto k = 0; k < 5; k++) {
                    if (grid[i + k][j - k] != player) {
                        t = false;
                        break;
                    }
                }
                if (t) {
                    wl->win = true;
                    wl->x = j;
                    wl->y = i;
                    wl->dir = WinLossInfo::TOPRIGHT2DOWNLEFT;
                    flag = true;
                    break;
                }
            }
            if (flag) break;
        }
    }

    if (!flag) {
        for (auto i = 0; i <= 10; i++) {
            for (auto j = 0; j <= 10; j++) {
                auto t = true;
                for (auto k = 0; k < 5; k++) {
                    if (grid[i + k][j + k] != player) {
                        t = false;
                        break;
                    }
                }
                if (t) {
                    wl->win = true;
                    wl->x = j;
                    wl->y = i;
                    wl->dir = WinLossInfo::TOPLEFT2DOWNRIGHT;
                    flag = true;
                    break;
                }
            }
            if (flag) break;
        }
    }
    if (!flag) {
        wl->win = false;
    }
    else {
        gameEnd = true;
    }
}

void GomokuRandomPlayer::makeOutNext(GridVector* chessGrid, int, int, int& x, int& y) {
    int pos = 0;
    x = y = 0;
    for (int i = 0; i < GOMOKU_GRID_SIZE; i++)for (int j = 0; j < GOMOKU_GRID_SIZE; j++) {
        if (chessGrid[i][j] < 0) {
            ipos[pos] = i;
            jpos[pos] = j;
            pos++;
        }
    }
    std::uniform_int_distribution<int> u(0, pos - 1);
    int p = u(re);
    x = ipos[p];
    y = jpos[p];
    Coroutines::yield();
}

void GomokuHumanPlayer::makeOutNext(GridVector* chessGrid, int, int, int& x, int& y) {
    while (true) {        
        if (mouseClickDetector && mouseClickDetector(x, y)) {
            if (chessGrid[x][y] == -1)
                break;
        }
        Coroutines::yield();
    }
    Coroutines::yield();
}

void GomokuAIPlayer::makeOutNext(GridVector* chessGrid, int, int, int& x, int& y) {
    std::atomic<bool> ok = false;
    auto fut = std::async(std::launch::async, [&]() -> int {
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                if(chessGrid[i][j] >= 0) {
                    ai->placeAt(i, j, getId() == chessGrid[i][j] ? 1 : 2);   //
                }
                else {
                    ai->placeAt(i, j, 0);
                }
            }
        }
        auto s = ai->makeOutNext();
        nextX = s.x;
        nextY = s.y;
        ok = true;
        return 0;
    });
    while (true) {
        if (!ok) {
            Coroutines::yield();
        } else {
            x = nextX;
            y = nextY;
            break;
        }
    }
    Coroutines::yield();
}

GomokuNetworkPlayer::GomokuNetworkPlayer(int playerid, const std::string& addr, int port, bool isChessServer) : GomokuPlayer(playerid) {
    isServer = isChessServer;
    IPaddress ip;
    if (isServer) {
        
        if (SDLNet_ResolveHost(&ip, NULL, port) == -1) {
            throw std::runtime_error(u8"创建服务器出错");
        }
        serverSocket0 = SDLNet_TCP_Open(&ip);
        if (!serverSocket0) {
            throw std::runtime_error(u8"创建服务器出错");
        }
        if (SDLNet_ResolveHost(&ip, NULL, port+1) == -1) {
            throw std::runtime_error(u8"创建服务器出错");
        }
        serverSocket1 = SDLNet_TCP_Open(&ip);
        if (!serverSocket1) {
            throw std::runtime_error(u8"创建服务器出错");
        }

        std::thread([&]() {
            TCPsocket tmpSocket = nullptr;
            while(!tmpSocket)
                tmpSocket = SDLNet_TCP_Accept(serverSocket1);
            remoteSocket1 = tmpSocket;           
        }).detach();

        std::thread([&]() {
            TCPsocket tmpSocket = nullptr;
            while (!tmpSocket)
                tmpSocket = SDLNet_TCP_Accept(serverSocket0);
            remoteSocket0 = tmpSocket;
        }).detach();
    }
    else {
        
        if (SDLNet_ResolveHost(&ip, addr.c_str(), port) == -1) {
            throw std::runtime_error(u8"解析连接目标失败");
        }
        remoteSocket0 = SDLNet_TCP_Open(&ip);
        if (!remoteSocket0) {
            throw std::runtime_error(u8"加入游戏失败");
        }
        if (SDLNet_ResolveHost(&ip, addr.c_str(), port+1) == -1) {
            throw std::runtime_error(u8"解析连接目标失败");
        }
        remoteSocket1 = SDLNet_TCP_Open(&ip);
        if (!remoteSocket1) {
            throw std::runtime_error(u8"加入游戏失败");
        }
    }
    serverAddr = addr;
    serverPort = port;
}

void GomokuNetworkPlayer::onPutChess(int lastX, int lastY) {
    std::atomic<bool> okWait;
    okWait = false;
    printf(isServer ? "server send %d %d\n" : "client send %d %d\n", lastX, lastY);
    TCPsocket sendSock = isServer ? remoteSocket0 : remoteSocket1;
    std::thread([lx = lastX, ly = lastY, ss = sendSock, &okWait]() {
        TransmitData td;
        td.x = lx;
        td.y = ly;
        SDLNet_TCP_Send(ss, &td, sizeof(td));
        okWait = true;
    }).detach();
    while (!okWait) {
        Coroutines::yield();
    }
    puts(isServer ? "server send ok" : "client send ok");
}

void GomokuNetworkPlayer::makeOutNext(GridVector* grid, int lastX, int lastY, int& x, int& y) {
    std::atomic<bool> okWait = false;
    puts(isServer ? "server recv" : "client recv");
    TCPsocket recvSock = isServer ? remoteSocket1 : remoteSocket0;

    okWait = false;
    std::thread([rs = recvSock, &okWait, &x, &y]() {
        TransmitData td;
        SDLNet_TCP_Recv(rs, &td, sizeof(td));
        x = td.x;
        y = td.y;
        okWait = true;
    }).detach();
    while (!okWait)
        Coroutines::yield();
    printf(isServer ? "server recv ok %d %d\n" : "client recv ok %d %d\n", x, y);
    
    Coroutines::yield();
}

GomokuNetworkPlayer::~GomokuNetworkPlayer() {
    serverWorking = gameWorking = false;
    SDLNet_TCP_Close(remoteSocket0);
    SDLNet_TCP_Close(serverSocket0);
    SDLNet_TCP_Close(remoteSocket1);
    SDLNet_TCP_Close(serverSocket1);
}