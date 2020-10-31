#include "GensokyoGomoku.h"
#include <QtWidgets/QApplication>
#include "Audio.h"
#include <QtCore/QAnimationGroup>
#include <QtGui/QPainter>
#include <SDL2/SDL_net.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2_mixer.lib")
#pragma comment(lib, "SDL2_net.lib")

#ifdef main 
#undef main     //Cancel #define main SDL_main 
#endif

int main(int argc, char *argv[])
{
    Audio::init();

    SDLNet_Init();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication a(argc, argv);
    GensokyoGomoku w;

    w.show();

    w.Mainloop();
    return 0;
}
