#include "GensokyoGomoku.h"
#include <thread>
#include <QtCore/QThread>
#include "Audio.h"
#include <QtWidgets/QFileDialog>

GensokyoGomoku::GensokyoGomoku(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.menuBar->setDisabled(true);
    setWindowFlags(windowFlags() & (~Qt::WindowMaximizeButtonHint));
    setWindowFlags(windowFlags() | Qt::MSWindowsOwnDC);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFixedSize(size());
    connect(ui.actionExit, &QAction::triggered, [&](int) {
        ExitProcess(0);
    });
    frameBuffer.reset(new QPixmap(SCENE_WIDTH, SCENE_HEIGHT));
    painter.reset(new QPainter(frameBuffer.get()));
    centralHWND = (HWND)centralWidget()->winId();

    connect(ui.actionPVP, &QAction::triggered, [&](int) {
        runLater([&]() {
            onStartGame(GAME_PVP);
        });
    });
    connect(ui.actionPVCFH, &QAction::triggered, [&](int) {
        runLater([&]() {
            onStartGame(GAME_PVCFH);
        });
    });
    connect(ui.actionPVCAH, &QAction::triggered, [&](int) {
        runLater([&]() {
            onStartGame(GAME_PVCAH);
        });
    });

    {
        auto cancelChecks = [&]() {
            ui.actionVeryEasy->setChecked(false);
            ui.actionEasy->setChecked(false);
            ui.actionNormal->setChecked(false);
            ui.actionHard->setChecked(false);
            ui.actionLunatic->setChecked(false);
        };

        connect(ui.actionVeryEasy, &QAction::triggered, [&,c=cancelChecks](int) {
            c();
            gameAILevel = 0;
            ui.actionVeryEasy->setChecked(true);
        });

        connect(ui.actionEasy, &QAction::triggered, [&, c = cancelChecks](int) {
            c();
            gameAILevel = 1;
            ui.actionEasy->setChecked(true);
        });

        connect(ui.actionNormal, &QAction::triggered, [&, c = cancelChecks](int) {
            c();
            gameAILevel = 3;
            ui.actionNormal->setChecked(true);
        });

        connect(ui.actionHard, &QAction::triggered, [&, c = cancelChecks](int) {
            c();
            gameAILevel = 6;
            ui.actionHard->setChecked(true);
        });

        connect(ui.actionLunatic, &QAction::triggered, [&, c=cancelChecks](int) {
            c();
            gameAILevel = 7;
            ui.actionLunatic->setChecked(true);
        });
        gameAILevel = 6;
        ui.actionHard->setChecked(true);
    }
    
    ui.menuTerminate->setDisabled(true);
    
    connectUI.reset(new ConnectPlayUI());

    connect(ui.actionCreateGame, &QAction::triggered, [&](int) {
        connectUI->showAndSetup(1);
        connectUI->onConfirmListener = [&](const QString addr, int port) {
            try {
                networkPlayer.reset(new GomokuNetworkPlayer(GOMOKU_PLAYER_WHITE, addr.toStdString(), port, true));
            }
            catch (std::runtime_error& e) {
                QMessageBox::warning(this, u8"错误", e.what());
                return false;
            }
            while (!networkPlayer->checkConnection())
                QCoreApplication::processEvents();
            runLater([&]() {
                onStartGame(GAME_PVP_ONLINE_FH);
            });
            return true;
        };
    });
    connect(ui.actionJoinGame, &QAction::triggered, [&](int) {
        connectUI->showAndSetup(0);
        connectUI->onConfirmListener = [&](const QString addr, int port) {
            try {
                networkPlayer.reset(new GomokuNetworkPlayer(GOMOKU_PLAYER_BLACK, addr.toStdString(), port, false));
            }
            catch (std::runtime_error& re) {
                QMessageBox::warning(this, u8"错误", re.what());
                return false;
            }
            runLater([&]() {
                onStartGame(GAME_PVP_ONLINE_AH);
            });
            return true;
        };
    });
    ui.menuTerminate->setDisabled(true);

    auto ani = std::make_shared<AnimationProc>([&](AnimationProc* a) {
        
        QFont font("Arial", 18);
        QPixmap background(u8"./Resources/Graphics/背景.png");
        QPixmap logo("./Resources/Graphics/logo.png");
        SoundEffect se(u8"./Resources/Sound/片头音效.mp3");
        Audio::playSE(&se);

        while (a->getAnimatedTime() < 4) {
            painter->save();
            painter->setPen(Qt::blue);
            painter->setFont(font);
            
            painter->drawPixmap(0, 0, background);
            if(a->getAnimatedTime() < 2.0) {
                painter->setOpacity(a->getAnimatedTime() / 2.0);
            }
            else {
                painter->setOpacity((4 - a->getAnimatedTime()) / 2.0);
            }
            const int w = SCENE_HEIGHT - 200;
            
            painter->drawPixmap(QRect(100, 50, w, w), logo);

            painter->drawText(QPoint(40, w + 110), u8"幻想乡老年五子棋俱乐部 制");
            painter->drawText(QPoint(40, w + 150), u8"北京交通大学计算机综合训练课程作业");
            painter->restore();
            Coroutines::yield();
        }
        
     }, 
        [&] {
            ui.menuBar->setDisabled(false);
            setupGame();
    }); 
    addAnimation(ani, 9);

    connect(ui.actionTerminate, &QAction::triggered, [&](int) {
        gamePhase = GAME_STOP;
    });

    connect(ui.actionYield, &QAction::triggered, [&](int) {
        gamePhase = GAME_STOP;
    });

    connect(ui.actionReplay, &QAction::triggered, [&](int) {
        QFileDialog fd;
        fd.setFileMode(QFileDialog::ExistingFile);
        fd.setAcceptMode(QFileDialog::AcceptOpen);
        if (fd.exec()) {
            std::string fname(fd.selectedFiles()[0].toLocal8Bit().data());
            replayFileName = std::move(fname);
            
            runLater([&]() {
                onStartGame(GAME_REPLAY);
            });
        }
        
    });

    connect(ui.actionAbout, &QAction::triggered, [&](int) {
        QMessageBox mbox(u8"关于",u8"北京交通大学计算机与信息技术学院18级\n计算机综合训练2组\n\t石响宇(组长)\n\t包慧敏\n\t罗叙卿\n\t(按小组报名顺序)", 
            QMessageBox::Information,
            QMessageBox::Yes,
            QMessageBox::NoButton,
            QMessageBox::NoButton, this);
        auto btn = mbox.button(QMessageBox::Yes);
        btn->setText(u8"赞！");
        mbox.exec();
    });

    lastTime = GetCurrentTimeStamp();
    
}

void GensokyoGomoku::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    p.drawPixmap(QPoint(0, ui.menuBar->height()), *frameBuffer);


    frameCount++;
    double curTime = GetCurrentTimeStamp();
    if (curTime - lastTime >= 1) {
        double fps = frameCount / (curTime - lastTime);
        setWindowTitle(QString::asprintf("GensokyoGomoku - %.2f fps", fps));
        lastTime = curTime;
        frameCount = 0;
    }
}

void GensokyoGomoku::setupGame() {
    gamePhase = GAME_READY;

    drawAnimations.aniDrawBoard.reset(new AnimationProc([&](AnimationProc* a) {
        /*
        QPen pen(Qt::black, 3);
        const int xstart = chessClient.x, ystart = chessClient.y;
        const int xstep = chessClient.w / GOMOKU_GRID_SIZE, ystep = chessClient.h / GOMOKU_GRID_SIZE;
        */
        QPixmap background(u8"./Resources/Graphics/背景.png");
        QPixmap board(u8"./Resources/Graphics/棋盘.png");
        while (true) {
            painter->save();
            /*
            painter->setPen(pen);
            for (int x = xstart; x <= xstart + chessClient.w; x += xstep) {
                painter->drawLine(x, ystart, x, ystart + chessClient.h);
            }
            for (int y = ystart; y <= ystart + chessClient.h; y += ystep) {
                painter->drawLine(xstart, y, xstart + chessClient.w, y);
            }*/
            painter->drawPixmap(0, 0, background);
            painter->drawPixmap(QRect(75, 125, 650, 650), board);

            painter->restore();
            Coroutines::yield();
        }
    }));
    addAnimation(drawAnimations.aniDrawBoard, 2);

    drawAnimations.aniDrawChess.reset(new AnimationProc([&](AnimationProc* a) {

        //QPen pen(Qt::black);
        //QBrush redBrush(Qt::red);
        //QBrush blueBrush(Qt::blue);
        QPixmap blackChess(u8"./Resources/Graphics/黑子-1.png");
        QPixmap whiteChess(u8"./Resources/Graphics/白子-1.png");

        while (true) {
            painter->save();
            //painter->setPen(pen);
            
            for (int i = 0; i < GOMOKU_GRID_SIZE; i++) {
                for (int j = 0; j < GOMOKU_GRID_SIZE; j++) {
                    int cx = chessClient.x + 40 * j;
                    int cy = chessClient.y + 40 * i;
                    if (game.grid[i][j] == GOMOKU_PLAYER_BLACK) {  
                       //painter->setBrush(redBrush);
                       //painter->drawEllipse(QRect(cx, cy, 40, 40));
                        painter->drawPixmap(QRect(cx, cy, 40, 40), blackChess);
                    }
                    else if (game.grid[i][j] == GOMOKU_PLAYER_WHITE) {
                        //painter->setBrush(blueBrush);
                        //painter->drawEllipse(QRect(cx, cy, 40, 40));
                        painter->drawPixmap(QRect(cx, cy, 40, 40), whiteChess);
                    }
                }
            }
            if(gamePhase == GAME_PVP ||
                ((gamePhase == GAME_PVCAH || gamePhase == GAME_PVCFH || 
                    gamePhase == GAME_PVP_ONLINE_AH || gamePhase == GAME_PVP_ONLINE_FH) 
                    && game.currentPlayer == gameSelfPlayer)) {
                
                if (currentChessPosition.x != -1 && game.grid[currentChessPosition.x][currentChessPosition.y] == -1) {
                    if (game.currentPlayer == GOMOKU_PLAYER_BLACK) {
                        //painter->setBrush(redBrush);
                        painter->setOpacity(0.5);
                        //注意棋盘的坐标(x,y)和屏幕坐标x,y是反过来的
                        painter->drawPixmap(QRect(currentChessPosition.y * 40 + chessClient.x,
                            currentChessPosition.x * 40 + chessClient.y, 40, 40), blackChess);

                    }
                    else if (game.currentPlayer == GOMOKU_PLAYER_WHITE) {
                        //painter->setBrush(blueBrush);
                        painter->setOpacity(0.5);
                        painter->drawPixmap(QRect(currentChessPosition.y * 40 + chessClient.x,
                            currentChessPosition.x * 40 + chessClient.y, 40, 40), whiteChess);

                    }
                }
            }

            painter->restore();
            Coroutines::yield();
        }
    }));
    addAnimation(drawAnimations.aniDrawChess, 1);

    drawAnimations.aniDrawState.reset(new AnimationProc([&](AnimationProc* a) {
        while (true) {

            Coroutines::yield();
        }
        
    }));
    addAnimation(drawAnimations.aniDrawState, 0);    

    //onStartGame(GAME_PVP);
}

void GensokyoGomoku::Mainloop() {
   
    /*
    AllocConsole();
    HWND hConsole = GetConsoleWindow();
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    char buf[100];
    */
    auto coroutineMouseInput = Coroutines::exec([&]() {
        POINT pt;
        while (true) {
            do {
                if (!isActiveWindow())
                    break;
                GetCursorPos(&pt);
                ScreenToClient(centralHWND, &pt);
                if (pt.x >= 0 && pt.y >= 0 && pt.x <= centralWidget()->width() && pt.y <= centralWidget()->height()) {
                //sprintf(buf, "%d, %d\n", pt.x, pt.y);
                //WriteConsoleA(hOutput, buf, strlen(buf), nullptr, nullptr);
                    for (auto& f : mouseEventListenser) {
                        f(pt.x, pt.y);
                    }
                }
            } while (0);
            Coroutines::yield();
        }
    });

    while(true) {
        QApplication::processEvents();
        Coroutines::resume(coroutineMouseInput);

        frameBuffer->fill(Qt::white);
        for (auto& f : todoList) {
            f();
        }
        todoList.clear();
        for(int i = 9; i >= 0; i--) {
            for (auto& ani : animations[i]) {
                ani->update();
            }
        }
        update();
        for(int i = 9; i >= 0; i--) {
            if(animations[i].size() > 0)
                animations[i].erase(std::remove_if(animations[i].begin(), animations[i].end(),
                    [](const std::shared_ptr<AnimationProc>& p) {
                    return p->isEnd();
                }), animations[i].end());
        }
       
       //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void GensokyoGomoku::mouseToChess(int mx, int my, int& x, int& y, bool& valid) {
    
    if (mx >= chessClient.x && my >= chessClient.y
        && mx < chessClient.x + chessClient.w &&
        my < chessClient.y + chessClient.h) {
        valid = true;

        x = (mx - chessClient.x) / 40;
        y = (my - chessClient.y) / 40;
    }
    else {
        valid = false;
    }
}

void GensokyoGomoku::getLocalMousePos(int& mx, int& my, bool& valid) {
    if (!isActiveWindow()) {
        valid = false;
        return;
    }
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(centralHWND, &pt);
    if (pt.x >= 0 && pt.y >= 0 && pt.x <= centralWidget()->width() && pt.y <= centralWidget()->height()) {
        valid = true;
        mx = pt.x;
        my = pt.y;
    }
    else {
        valid = false;
    }
}

void GensokyoGomoku::onPutChess(int player, int x, int y) {
    if (x < 0 || y < 0 || x >= GOMOKU_GRID_SIZE || y >= GOMOKU_GRID_SIZE)
        return;
    if (game.put(player, x, y)) {
        gameSE.playPutChess();
        game.switchPlayer();
        Coroutines::sleep(0.5);
        GomokuGame::WinLossInfo win;
        game.checkWinLoss(player, &win);
        if (win.win) {
            if (gameSelfPlayer >= 0) {
                if (gameSelfPlayer == player) {
                    gameSE.playWin();
                }
                else {
                    gameSE.playLoss();
                }
            }else {
                gameSE.playWin();
            }
            Coroutines::sleep(2);
        }
    }
}

void GensokyoGomoku::onStartGame(int gameMode) {
    game.clear();
    
    auto mouseDetector = [&](int& x, int& y) -> bool {
        //x = currentChessPosition.x;
        //y = currentChessPosition.y;
        int mx, my;
        bool v;
        getLocalMousePos(mx, my, v);
        if (!v)
            return false;
        int cx, cy;
        mouseToChess(mx, my, cx, cy, v);
        if (!v)
            return false;
        //注意棋盘的x,y和窗口坐标系的x,y是反过来的
        x = cy;
        y = cx;
        if (KEYDOWN(VK_LBUTTON)) {
            return true;
        }
        else {
            return false;
        }
    };

    ui.menuTerminate->setDisabled(false);

    if (gameMode == GAME_PVCFH) {
        gameSelfPlayer = GOMOKU_PLAYER_BLACK;
        game.switchPlayer(GOMOKU_PLAYER_BLACK);
        //players.white.reset(new GomokuRandomPlayer(0));
        if(gameAILevel > 0) {
            players.white.reset(new GomokuAIPlayer(0, gameAILevel));
        }
        else {
            players.white.reset(new GomokuRandomPlayer(0));
        }
        players.black.reset(new GomokuHumanPlayer(1, mouseDetector));
        /*
        
        */
    }
    else if (gameMode == GAME_PVCAH) {
        gameSelfPlayer = GOMOKU_PLAYER_WHITE;
        game.switchPlayer(GOMOKU_PLAYER_BLACK);
        players.white.reset(new GomokuHumanPlayer(0, mouseDetector));
        if (gameAILevel > 0) {
            players.black.reset(new GomokuAIPlayer(1, gameAILevel));
        }
        else {
            players.black.reset(new GomokuRandomPlayer(1));
        }
    }
    else if (gameMode == GAME_PVP) {
        gameSelfPlayer = -1;
        game.switchPlayer(GOMOKU_PLAYER_BLACK);
        players.black.reset(new GomokuHumanPlayer(1, mouseDetector));
        players.white.reset(new GomokuHumanPlayer(0, mouseDetector));
        
    }
    else if (gameMode == GAME_PVP_ONLINE_AH) {
        gameSelfPlayer = GOMOKU_PLAYER_WHITE;
        game.switchPlayer(GOMOKU_PLAYER_BLACK);
        players.black = std::move(networkPlayer);
        players.white.reset(new GomokuHumanPlayer(0, mouseDetector));

        ui.menuTerminate->setDisabled(true);
    }
    else if (gameMode == GAME_PVP_ONLINE_FH) {
        gameSelfPlayer = GOMOKU_PLAYER_BLACK;
        game.switchPlayer(GOMOKU_PLAYER_BLACK);
        players.black.reset(new GomokuHumanPlayer(1, mouseDetector));
        players.white = std::move(networkPlayer);

        ui.menuTerminate->setDisabled(true);
    }
    else if (gameMode == GAME_REPLAY) {
        gameSelfPlayer = GOMOKU_PLAYER_BLACK;
        game.switchPlayer(GOMOKU_PLAYER_BLACK);
        players.black.reset(new GomokuReplayer(1, replayFileName));
        players.white.reset(new GomokuReplayer(0, replayFileName));
    }

    gamePhase = gameMode;
    auto setMenus = [&](bool disabled) {
        ui.menuSetLevel->setDisabled(disabled);
        ui.menuOfflineGame->setDisabled(disabled);
        ui.menuOnlineGame->setDisabled(disabled);

    };
    setMenus(true);

    //游戏对弈主要逻辑过程
    auto aniGameProc = std::make_shared<AnimationProc>([&,setMenus=setMenus](AnimationProc *ani) {
        int lastX = -1, lastY = -1;
        bool flag = true;
        while (flag) {
            do {
                int x, y;
               
                if (gamePhase == GAME_STOP) {  //游戏终止检查点
                    flag = false;
                    break; 
                }

                int c = game.stepCounter;
                if(gamePhase == GAME_PVP_ONLINE_FH) {  
                    players.black->makeOutNext(game.grid, lastX, lastY, x, y);
                    players.white->onPutChess(x, y);
                }
                /*
                else if(gamePhase == GAME_PVP_ONLINE_AH) {
                    if (lastX == -1)
                        players.black->onGetLast(-1, -1);
                    players.black->makeOutNext(game.grid, lastX, lastY, x, y);
                }*/
                else {
                    players.black->makeOutNext(game.grid, lastX, lastY, x, y);
                }
                lastX = x, lastY = y;

                if (gamePhase == GAME_STOP) {  //游戏终止检查点
                    flag = false;
                    break;
                }
              
                if(x >= 0 && y >= 0) {
                    onPutChess(GOMOKU_PLAYER_BLACK, x, y);
                    if (c + 1 != game.stepCounter) {  //游戏策略有bug，没能生成一个合法的下子位置
                        QMessageBox::warning(this, u8"bug", u8"黑色方游戏策略有bug，请联系作者修复");
                        printf("bug");
                    }
                }

                if (isGameEnd()) {
                    flag = false;
                    break;
                }
                
                c = game.stepCounter;
                
                if (gamePhase == GAME_STOP) {  //游戏终止检查点
                    flag = false;
                    break;
                }
                if(gamePhase == GAME_PVP_ONLINE_AH) {
                    players.white->makeOutNext(game.grid, lastX, lastY, x, y);
                    players.black->onPutChess(x, y);
                }
                else {
                    players.white->makeOutNext(game.grid, lastX, lastY, x, y);
                }
                lastX = x, lastY = y;
               
                if (gamePhase == GAME_STOP) {  //游戏终止检查点
                    flag = false;
                    break;
                }
                if(x >= 0 && y >= 0) {
                    onPutChess(GOMOKU_PLAYER_WHITE, x, y);
                    if (c + 1 != game.stepCounter) {
                        QMessageBox::warning(this, u8"bug", u8"白色方游戏策略有bug，请联系作者修复");
                        printf("bug");
                    }
                }
               
                if (isGameEnd()) {
                    flag = false;
                    break;
                }
            } while (0);
            Coroutines::yield();
        }
       // if (gamePhase != GAME_READY) { //replay模式忽略这个提示

        QMessageBox mbox(u8"提示", gamePhase == GAME_STOP ? u8"结束/认输结束比赛没有保存回放的价值" :
                    u8"游戏已结束，是否保存回放", QMessageBox::NoIcon,
                         QMessageBox::Yes, 
                         QMessageBox::No, 
                         QMessageBox::NoButton, this);
        QAbstractButton* btnOK = mbox.button(QMessageBox::Yes);
        QAbstractButton* btnCancel = mbox.button(QMessageBox::No);
        if (gamePhase == GAME_STOP)
            btnOK->setDisabled(true);
        mbox.show();
        while (true) {
            auto btn = mbox.clickedButton();
            if (btn == btnOK) {
                QFileDialog fd;
                fd.setFileMode(QFileDialog::AnyFile);
                fd.setAcceptMode(QFileDialog::AcceptSave);
                fd.setDefaultSuffix(".replay");
                if (fd.exec()) {
                    std::string fname(fd.selectedFiles()[0].toLocal8Bit().data());
                    game.save(fname);
                }
                break;
            }
            else if (btn == btnCancel) {
                break;
            }
            QCoreApplication::processEvents();
            Coroutines::yield();
        }
        //}
       
        onTerminateGame();
        setMenus(false);
        ui.menuTerminate->setDisabled(true);
        Coroutines::yield();
    });
    addAnimation(aniGameProc, 9);
    
    mouseEventListenser.push_back([&](int mx, int my){
        int cx, cy;
        bool v;
        mouseToChess(mx, my, cx, cy, v);
        if (!v) return;
        currentChessPosition.x = cy;
        currentChessPosition.y = cx;
    });
}

void GensokyoGomoku::onTerminateGame() {
    //game.clear();
    gamePhase = GAME_READY;
    players.black.reset();
    players.white.reset();
}