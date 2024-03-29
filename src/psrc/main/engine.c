#include "../engine/renderer.h"
#include "../engine/input.h"
#include "../engine/ui.h"
#include "../engine/audio.h"
#include "../common/logging.h"
#include "../common/string.h"
#include "../common/filesystem.h"
#include "../common/config.h"
#include "../common/resource.h"
#include "../common/time.h"
#include "../common/arg.h"

#include "../version.h"
#include "../platform.h"
#include "../debug.h"
#include "../common.h"

#if PLATFORM == PLAT_NXDK
    #include <SDL.h>
#elif defined(PSRC_USESDL1)
    #include <SDL/SDL.h>
#else
    #include <SDL2/SDL.h>
#endif

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#if PLATFORM == PLAT_WIN32
    #include <windows.h>
#elif PLATFORM == PLAT_EMSCR
    #include <emscripten.h>
#elif PLATFORM == PLAT_NXDK
    #include <xboxkrnl/xboxkrnl.h>
    #include <winapi/winnt.h>
    #include <hal/video.h>
    #include <pbkit/pbkit.h>
    #include <pbgl.h>
    #include <GL/gl.h>
#elif PLATFORM == PLAT_DREAMCAST
    #include <kos.h>
    #include <dirent.h>
    KOS_INIT_FLAGS(INIT_IRQ | INIT_THD_PREEMPT | INIT_CONTROLLER | INIT_VMU | INIT_NO_DCLOAD);
#endif

#include "../glue.h"

#if PLATFORM != PLAT_NXDK && PLATFORM != PLAT_EMSCR && PLATFORM != PLAT_DREAMCAST
static void sigh_log(int l, char* name, char* msg) {
    if (msg) {
        plog(l, "Received signal: %s; %s", name, msg);
    } else {
        plog(l, "Received signal: %s", name);
    }
}
static void sigh(int sig) {
    signal(sig, sigh);
    #if defined(_POSIX_VERSION) && _POSIX_VERSION >= 200809L
    char* signame = strsignal(sig);
    #else
    char signame[8];
    snprintf(signame, sizeof(signame), "%d", sig);
    #endif
    switch (sig) {
        case SIGINT:
            if (quitreq > 0) {
                sigh_log(LL_WARN, signame, "Graceful exit already requested; Forcing exit...");
                exit(1);
            } else {
                sigh_log(LL_INFO, signame, "Requesting graceful exit...");
                ++quitreq;
            }
            break;
        case SIGTERM:
        #ifdef SIGQUIT
        case SIGQUIT:
        #endif
            sigh_log(LL_WARN, signame, "Forcing exit...");
            exit(1);
            break;
        default:
            sigh_log(LL_WARN, signame, NULL);
            break;
    }
}
#endif

#if PLATFORM == PLAT_NXDK && !defined(PSRC_NOMT)
static thread_t watchdogthread;
static volatile bool killwatchdog;
static void* watchdog(struct thread_data* td) {
    plog(LL_INFO, "Watchdog armed for %u seconds", (unsigned)td->args);
    uint64_t t = altutime() + (uint64_t)(td->args) * 1000000;
    while (t > altutime()) {
        if (killwatchdog) {
            killwatchdog = false;
            plog(LL_INFO, "Watchdog cancelled");
            return NULL;
        }
        yield();
    }
    HalReturnToFirmware(HalRebootRoutine);
    return NULL;
}
static void armWatchdog(unsigned sec) {
    createThread(&watchdogthread, "watchdog", watchdog, (void*)sec);
}
static void cancelWatchdog(void) {
    killwatchdog = true;
    destroyThread(&watchdogthread, NULL);
}
static void rearmWatchdog(unsigned sec) {
    killwatchdog = true;
    destroyThread(&watchdogthread, NULL);
    createThread(&watchdogthread, "watchdog", watchdog, (void*)sec);
}
#endif

struct rc_script* mainscript;

static struct rc_sound* test;
static uint64_t testsound = -1;
static float lookspeed[2];
static uint64_t toff;
static uint64_t framestamp;

enum __attribute__((packed)) action {
    ACTION_NONE,
    ACTION_MENU,
    ACTION_FULLSCREEN,
    ACTION_SCREENSHOT,
    ACTION_MOVE_FORWARDS,
    ACTION_MOVE_BACKWARDS,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_RIGHT,
    ACTION_LOOK_UP,
    ACTION_LOOK_DOWN,
    ACTION_LOOK_LEFT,
    ACTION_LOOK_RIGHT,
    ACTION_WALK,
    ACTION_JUMP,
    ACTION_CROUCH,
};

int initLoop(void) {
    plog(LL_INFO, "Initializing renderer...");
    if (!initRenderer()) {
        plog(LL_CRIT | LF_MSGBOX | LF_FUNCLN, "Failed to init renderer");
        return 1;
    }
    plog(LL_INFO, "Initializing input manager...");
    if (!initInput()) {
        plog(LL_CRIT | LF_MSGBOX | LF_FUNCLN, "Failed to init input manager");
        return 1;
    }
    plog(LL_INFO, "Initializing UI manager...");
    if (!initUI()) {
        plog(LL_CRIT | LF_MSGBOX | LF_FUNCLN, "Failed to init UI manager");
        return 1;
    }
    plog(LL_INFO, "Initializing audio manager...");
    if (!initAudio()) {
        plog(LL_CRIT | LF_MSGBOX | LF_FUNCLN, "Failed to init audio manager");
        return 1;
    }

    char* tmp;
    if (options.icon) {
        rendstate.icon = mkpath(NULL, options.icon, NULL);
    } else {
        tmp = cfg_getvar(gameconfig, NULL, "icon");
        if (tmp) {
            rendstate.icon = getRcPath(tmp, RC_TEXTURE, NULL);
            free(tmp);
        } else {
            rendstate.icon = mkpath(maindir, "icons", "engine.png", NULL);
        }
    }
    plog(LL_INFO, "Starting renderer...");
    if (!startRenderer()) {
        plog(LL_CRIT | LF_MSGBOX | LF_FUNCLN, "Failed to start renderer");
        return 1;
    }
    plog(LL_INFO, "Starting audio manager...");
    if (!startAudio()) {
        plog(LL_CRIT | LF_MSGBOX | LF_FUNCLN, "Failed to start audio manager");
        return 1;
    }

    plog(LL_INFO, "Almost there...");

    mainscript = loadResource(RC_SCRIPT, "main", NULL);
    if (!mainscript) {
        plog(LL_CRIT | LF_MSGBOX, "Could not start main script");
        //return 1;
    }

    test = loadResource(RC_SOUND, "sounds/ambient/wind1", &audiostate.soundrcopt);
    if (test) playSound(false, test, SOUNDFLAG_LOOP | SOUNDFLAG_WRAP, SOUNDFX_VOL, 0.5, 0.5, SOUNDFX_END);
    freeResource(test);
    test = loadResource(RC_SOUND, "sounds/ac1", &audiostate.soundrcopt);
    if (test) playSound(
        false, test,
        SOUNDFLAG_POSEFFECT | SOUNDFLAG_FORCEMONO | SOUNDFLAG_LOOP,
        SOUNDFX_POS, 0.0, 0.0, 2.0, SOUNDFX_END
    );
    freeResource(test);
    test = loadResource(RC_SOUND, "sounds/health", &audiostate.soundrcopt);
    if (test) testsound = playSound(
        false, test,
        SOUNDFLAG_POSEFFECT | SOUNDFLAG_FORCEMONO | SOUNDFLAG_LOOP,
        SOUNDFX_POS, 0.0, 0.0, 4.0, SOUNDFX_END
    );
    freeResource(test);

    // TODO: cleanup
    setInputMode(INPUTMODE_INGAME);
    struct inputkey* k;
    #if PLATFORM != PLAT_EMSCR
    k = inputKeysFromStr("k,esc;g,b,start");
    #else
    k = inputKeysFromStr("k,backspace;g,b,start");
    #endif
    newInputAction(INPUTACTIONTYPE_ONCE, "menu", k, (void*)ACTION_MENU);
    free(k);
    k = inputKeysFromStr("k,f11");
    newInputAction(INPUTACTIONTYPE_ONCE, "fullscreen", k, (void*)ACTION_FULLSCREEN);
    free(k);
    k = inputKeysFromStr("k,f2");
    newInputAction(INPUTACTIONTYPE_ONCE, "screenshot", k, (void*)ACTION_SCREENSHOT);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("k,w;g,a,-lefty");
    #else
    k = inputKeysFromStr("k,w;g,b,y");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "move_forwards", k, (void*)ACTION_MOVE_FORWARDS);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("k,a;g,a,-leftx");
    #else
    k = inputKeysFromStr("k,a;g,b,x");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "move_left", k, (void*)ACTION_MOVE_LEFT);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("k,s;g,a,+lefty");
    #else
    k = inputKeysFromStr("k,s;g,b,a");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "move_backwards", k, (void*)ACTION_MOVE_BACKWARDS);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("k,d;g,a,+leftx");
    #else
    k = inputKeysFromStr("k,d;g,b,b");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "move_right", k, (void*)ACTION_MOVE_RIGHT);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("m,m,+y;k,up;g,a,-righty");
    #else
    k = inputKeysFromStr("m,m,+y;k,up;g,a,-lefty");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "look_up", k, (void*)ACTION_LOOK_UP);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("m,m,-x;k,left;g,a,-rightx");
    #else
    k = inputKeysFromStr("m,m,-x;k,left;g,a,-leftx");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "look_left", k, (void*)ACTION_LOOK_LEFT);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("m,m,-y;k,down;g,a,+righty");
    #else
    k = inputKeysFromStr("m,m,-y;k,down;g,a,+lefty");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "look_down", k, (void*)ACTION_LOOK_DOWN);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("m,m,+x;k,right;g,a,+rightx");
    #else
    k = inputKeysFromStr("m,m,+x;k,right;g,a,+leftx");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "look_right", k, (void*)ACTION_LOOK_RIGHT);
    free(k);
    #if PLATFORM != PLAT_EMSCR
    k = inputKeysFromStr("k,lctrl;g,b,leftstick");
    #else
    k = inputKeysFromStr("k,c;g,b,leftstick");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "walk", k, (void*)ACTION_WALK);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("k,space;g,b,a");
    #else
    k = inputKeysFromStr("k,space;g,a,+lefttrigger");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "jump", k, (void*)ACTION_JUMP);
    free(k);
    #if PLATFORM != PLAT_DREAMCAST
    k = inputKeysFromStr("k,lshift;g,a,+lefttrigger");
    #else
    k = inputKeysFromStr("k,lshift;g,b,dpdown");
    #endif
    newInputAction(INPUTACTIONTYPE_MULTI, "crouch", k, (void*)ACTION_CROUCH);
    free(k);

    tmp = cfg_getvar(config, "Input", "lookspeed");
    if (tmp) {
        sscanf(tmp, "%f,%f", &lookspeed[0], &lookspeed[1]);
        free(tmp);
    } else {
        lookspeed[0] = 2.0;
        lookspeed[1] = 2.0;
    }

    plog(LL_INFO, "All systems go!");
    toff = SDL_GetTicks();
    framestamp = altutime();

    return 0;
}

static float fwrap(float n, float d) {
    float tmp = n - (int)(n / d) * d;
    if (tmp < 0.0) tmp += d;
    return tmp;
}
void doLoop(void) {
    static float runspeed = 4.0;
    static float walkspeed = 2.0;
    static double framemult = 0.0;

    pollInput();
    if (quitreq) return;

    // TODO: run scripts here

    long lt = SDL_GetTicks() - toff;
    double dt = (double)(lt % 1000) / 1000.0;
    double t = (double)(lt / 1000) + dt;
    changeSoundFX(testsound, false, SOUNDFX_POS, sin(t * 2.5) * 4.0, 0.0, cos(t * 2.5) * 4.0, SOUNDFX_END);

    static bool screenshot = false;

    bool walk = false;
    float movex = 0.0, movez = 0.0, movey = 0.0;
    float lookx = 0.0, looky = 0.0;
    struct inputaction a;
    while (getNextInputAction(&a)) {
        //printf("action!: %s: %f\n", a.data->name, (float)a.amount / 32767.0);
        switch ((enum action)a.userdata) {
            case ACTION_MENU: ++quitreq; break;
            case ACTION_FULLSCREEN: updateRendererConfig(RENDOPT_FULLSCREEN, -1, RENDOPT_END); break;
            case ACTION_SCREENSHOT: screenshot = true; break;
            case ACTION_MOVE_FORWARDS: movez += (float)a.amount / 32767.0; break;
            case ACTION_MOVE_BACKWARDS: movez -= (float)a.amount / 32767.0; break;
            case ACTION_MOVE_LEFT: movex -= (float)a.amount / 32767.0; break;
            case ACTION_MOVE_RIGHT: movex += (float)a.amount / 32767.0; break;
            case ACTION_LOOK_UP: lookx += (float)a.amount * ((a.constant) ? framemult * 50.0 : 1.0) / 32767.0; break;
            case ACTION_LOOK_DOWN: lookx -= (float)a.amount * ((a.constant) ? framemult * 50.0 : 1.0) / 32767.0; break;
            case ACTION_LOOK_LEFT: looky -= (float)a.amount * ((a.constant) ? framemult * 50.0 : 1.0) / 32767.0; break;
            case ACTION_LOOK_RIGHT: looky += (float)a.amount * ((a.constant) ? framemult * 50.0 : 1.0) / 32767.0; break;
            case ACTION_WALK: walk = true; break;
            case ACTION_JUMP: movey += (float)a.amount / 32767.0; break;
            case ACTION_CROUCH: movey -= (float)a.amount / 32767.0; break;
            default: break;
        }
    }
    float speed = (walk) ? walkspeed : runspeed;
    float jumpspeed = (walk) ? 1.0 : 2.5;

    char* tmp = cfg_getvar(config, "Debug", "printfps");
    bool printfps = strbool(tmp, false);
    free(tmp);

    {
        // this can probably be optimized
        float mul = atan2(fabs(movex), fabs(movez));
        mul = fabs(1 / (cos(mul) + sin(mul)));
        movex *= mul;
        movez *= mul;
        float yrotrad = rendstate.camrot[1] * M_PI / 180.0;
        float tmp[4] = {
            movez * sinf(yrotrad),
            movex * cosf(yrotrad),
            movez * cosf(yrotrad),
            movex * -sinf(yrotrad),
        };
        movex = tmp[0] + tmp[1];
        movez = tmp[2] + tmp[3];
    }
    audiostate.campos[0] = (rendstate.campos[0] += movex * speed * framemult);
    audiostate.campos[2] = (rendstate.campos[2] += movez * speed * framemult);
    audiostate.campos[1] = (rendstate.campos[1] += movey * jumpspeed * framemult);
    rendstate.camrot[0] += lookx * lookspeed[1];
    rendstate.camrot[1] += looky * lookspeed[0];
    if (rendstate.camrot[0] > 90.0) rendstate.camrot[0] = 90.0;
    else if (rendstate.camrot[0] < -90.0) rendstate.camrot[0] = -90.0;
    rendstate.camrot[1] = fwrap(rendstate.camrot[1], 360.0);
    audiostate.camrot[0] = rendstate.camrot[0];
    audiostate.camrot[1] = rendstate.camrot[1];
    audiostate.camrot[2] = rendstate.camrot[2];

    updateSounds();
    render();
    display();

    if (screenshot) {
        int sz;
        void* d = takeScreenshot(NULL, NULL, &sz);
        FILE* f = fopen("screenshot.data", "wb");
        fwrite(d, 1, sz, f);
        fclose(f);
        free(d);
        screenshot = false;
    }

    uint64_t tmputime = altutime();
    uint64_t frametime = tmputime - framestamp;
    framestamp = tmputime;
    framemult = frametime / 1000000.0;

    if (printfps) {
        static uint64_t fpstime = 0;
        static uint64_t fpsframetime = 0;
        static int fpsframecount = 0;
        fpsframetime += frametime;
        ++fpsframecount;
        if (tmputime >= fpstime) {
            fpstime += 200000;
            if (tmputime >= fpstime) fpstime = tmputime + 200000;
            uint64_t avgframetime = fpsframetime / fpsframecount;
            float avgfps = 1000000.0 / (uint64_t)avgframetime;
            printf("FPS: %.03f (%"PRId64"us)\n", avgfps, avgframetime);
            fpsframetime = 0;
            fpsframecount = 0;
        }
    }
}

void quitLoop(void) {
    plog(LL_INFO, "Quit requested");
    #if PLATFORM == PLAT_NXDK
    plog__nodraw = false;
    #endif

    #if PLATFORM == PLAT_NXDK && !defined(PSRC_NOMT)
    armWatchdog(5);
    #endif
    plog(LL_INFO, "Stopping audio manager...");
    stopAudio();
    #if PLATFORM == PLAT_NXDK && !defined(PSRC_NOMT)
    rearmWatchdog(5);
    #endif
    plog(LL_INFO, "Stopping renderer...");
    stopRenderer();
    #if PLATFORM == PLAT_NXDK && !defined(PSRC_NOMT)
    cancelWatchdog();
    #endif

    plog(LL_INFO, "Quitting audio manager...");
    quitAudio();
    plog(LL_INFO, "Quitting UI manager...");
    quitUI();
    plog(LL_INFO, "Quitting input manager...");
    quitInput();
    plog(LL_INFO, "Quitting renderer...");
    quitRenderer();
}

static int bootstrap(void) {
    puts(verstr);
    puts(platstr);
    #if PLATFORM == PLAT_NXDK
    pb_print("%s\n", verstr);
    pb_print("%s\n", platstr);
    pbgl_swap_buffers();
    #endif

    if (!initLogging()) {
        fputs(LP_ERROR "Failed to init logging\n", stderr);
        return 1;
    }
    #if PLATFORM == PLAT_NXDK
    plog_setfile("D:\\log.txt");
    maindir = mkpath("D:\\", NULL);
    #elif PLATFORM == PLAT_DREAMCAST
    {
        DIR* d = opendir("/cd");
        if (d) {
            closedir(d);
            maindir = mkpath("/cd", NULL);
        } else {
            d = opendir("/sd/psrc");
            if (d) {
                closedir(d);
                maindir = mkpath("/sd/psrc", NULL);
            } else {
                d = opendir("/sd/PlatinumSrc");
                if (d) {
                    closedir(d);
                    maindir = mkpath("/sd/PlatinumSrc", NULL);
                } else {
                    maindir = mkpath("/sd", NULL);
                }
            }
        }
    }
    #else
    if (options.maindir) {
        maindir = mkpath(options.maindir, NULL);
        free(options.maindir);
    } else {
        #ifndef PSRC_USESDL1
        maindir = SDL_GetBasePath();
        #else
        // TODO: get the dir where the executable is located
        maindir = NULL;
        #endif
        if (maindir) {
            char* tmp = maindir;
            maindir = mkpath(tmp, NULL);
            SDL_free(tmp);
        } else {
            plog(LL_WARN, "Failed to get main directory: %s", SDL_GetError());
            maindir = strdup(".");
        }
    }
    #endif

    char* tmp;

    if (options.config) {
        tmp = mkpath(options.config, NULL);
        free(options.config);
    } else {
        tmp = mkpath(maindir, "engine/config/config.cfg", NULL);
    }
    config = cfg_open(tmp);
    free(tmp);
    if (!config) {
        plog(LL_WARN, "Failed to load main config");
        config = cfg_open(NULL);
    }
    if (options.set) cfg_mergemem(config, options.set, true);

    tmp = (options.game) ? options.game : cfg_getvar(config, NULL, "defaultgame");
    if (tmp) {
        gamedir = mkpath(NULL, tmp, NULL);
        free(tmp);
    } else {
        static const char* fallback = "default";
        plog(LL_WARN, "No default game specified, falling back to '%s'", fallback);
        gamedir = mkpath(NULL, fallback, NULL);
    }

    tmp = mkpath(maindir, "games", gamedir, NULL);
    if (isFile(tmp)) {
        plog(LL_INFO, "Main directory: %s", maindir);
        plog(LL_CRIT | LF_MSGBOX, "Could not find game directory for '%s'", gamedir);
        free(tmp);
        return 1;
    }
    free(tmp);

    tmp = mkpath(maindir, "games", gamedir, "game.cfg", NULL);
    gameconfig = cfg_open(tmp);
    free(tmp);
    if (!gameconfig) {
        plog(LL_INFO, "Main directory: %s", maindir);
        plog(LL_INFO, "Game directory: %s" PATHSEPSTR "games" PATHSEPSTR "%s", maindir, gamedir);
        plog(LL_CRIT | LF_MSGBOX, "Could not read game.cfg for '%s'", gamedir);
        return 1;
    }

    #if PLATFORM == PLAT_NXDK
    {
        uint32_t titleid = CURRENT_XBE_HEADER->CertificateHeader->TitleID;
        char titleidstr[9];
        sprintf(titleidstr, "%08x", (unsigned)titleid);
        tmp = cfg_getvar(gameconfig, NULL, "userdir");
        if (tmp) {
            char* tmp2 = mkpath(NULL, tmp, NULL);
            free(tmp);
            tmp = tmp2;
        } else {
            tmp = strdup(gamedir);
        }
        userdir = mkpath("E:\\TDATA", titleidstr, "userdata", tmp, NULL);
        free(tmp);
        savedir = mkpath("E:\\UDATA", titleidstr, NULL);
    }
    #elif PLATFORM == PLAT_DREAMCAST
    userdir = mkpath("/rd", NULL);
    #else
    if (options.userdir) {
        userdir = mkpath(options.userdir, NULL);
        free(options.userdir);
    } else {
        tmp = cfg_getvar(gameconfig, NULL, "userdir");
        if (tmp) {
            char* tmp2 = mkpath(NULL, tmp, NULL);
            free(tmp);
            tmp = tmp2;
        } else {
            tmp = strdup(gamedir);
        }
        // TODO: make a function in common/filesystem.c to replace SDL_GetPrefPath
        #ifndef PSRC_USESDL1
        userdir = SDL_GetPrefPath(NULL, tmp);
        #else
        userdir = NULL;
        #endif
        free(tmp);
        if (userdir) {
            char* tmp = userdir;
            userdir = mkpath(tmp, NULL);
            SDL_free(tmp);
        } else {
            plog(LL_WARN, LP_WARN "Failed to get user directory: %s", SDL_GetError());
            userdir = strdup("." PATHSEPSTR "data");
        }
    }
    savedir = mkpath(userdir, "saves", NULL);
    #endif

    tmp = cfg_getvar(gameconfig, NULL, "name");
    if (tmp) {
        free(titlestr);
        titlestr = tmp;
    }

    char* logfile = mkpath(userdir, "log.txt", NULL);
    if (!plog_setfile(logfile)) {
        plog(LL_WARN, "Failed to set log file");
    }
    free(logfile);

    if (!options.nouserconfig) {
        tmp = mkpath(userdir, "config", "config.cfg", NULL);
        if (!cfg_merge(config, tmp, true)) {
            plog(LL_WARN, "Failed to load user config");
        }
        free(tmp);
        if (options.set) cfg_mergemem(config, options.set, true);
    }

    plog(LL_INFO, "Main directory: %s", maindir);
    plog(LL_INFO, "Game directory: %s" PATHSEPSTR "games" PATHSEPSTR "%s", maindir, gamedir);
    plog(LL_INFO, "User directory: %s", userdir);
    plog(LL_INFO, "Save directory: %s", savedir);

    plog(LL_INFO, "Initializing resource manager...");
    if (!initResource()) {
        plog(LL_CRIT | LF_FUNCLN, "Failed to init resource manager");
        return 1;
    }

    return 0;
}
static void unstrap(void) {
    plog(LL_INFO, "Quitting resource manager...");
    quitResource();

    cfg_close(gameconfig);
    char* tmp = mkpath(userdir, "config", "config.cfg", NULL);
    //cfg_write(config, tmp);
    free(tmp);
    cfg_close(config);

    #if PLATFORM == PLAT_NXDK && !defined(PSRC_NOMT)
    armWatchdog(5);
    #endif
    SDL_Quit();
    #if PLATFORM == PLAT_NXDK && !defined(PSRC_NOMT)
    cancelWatchdog();
    #endif

    plog(LL_INFO, "Done");
}

#if PLATFORM == PLAT_EMSCR
volatile bool issyncfsok = false;
void __attribute__((used)) syncfsok(void) {issyncfsok = true;}
static void emscrexit(void* arg) {
    (void)arg;
    exit(0);
}
static void emscrmain(void) {
    static bool doloop = false;
    static bool syncmsg = false;
    if (doloop) {
        doLoop();
        if (quitreq) {
            static bool doexit = false;
            if (doexit) {
                if (!syncmsg) {
                    puts("Waiting on RAM to disk FS.syncfs...");
                    syncmsg = true;
                }
                if (issyncfsok) {
                    emscripten_cancel_main_loop();
                    emscripten_async_call(emscrexit, NULL, -1);
                    puts("Done");
                }
                return;
            }
            quitLoop();
            unstrap();
            EM_ASM(
                FS.syncfs(false, function (e) {
                    if (e) console.error("FS.syncfs:", e);
                    ccall("syncfsok");
                });
            );
            doexit = true;
        }
    } else {
        if (!syncmsg) {
            puts("Waiting on disk to RAM FS.syncfs...");
            syncmsg = true;
        }
        if (!issyncfsok) return;
        issyncfsok = false;
        syncmsg = false;
        static int ret;
        ret = bootstrap();
        if (!ret) {
            ret = initLoop();
            if (!ret) {
                doloop = true;
                return;
            }
            unstrap();
        }
    }
}
#endif
int main(int argc, char** argv) {
    makeVerStrs();
    static int ret;
    #if PLATFORM != PLAT_EMSCR
    #if PLATFORM != PLAT_NXDK && PLATFORM != PLAT_DREAMCAST
    ret = -1;
    {
        struct args a;
        args_init(&a, argc, argv);
        struct charbuf opt;
        struct charbuf var;
        struct charbuf val;
        struct charbuf err;
        cb_init(&opt, 32);
        cb_init(&var, 32);
        cb_init(&val, 256);
        cb_init(&err, 64);
        while (1) {
            int e = args_getopt(&a, &opt, &err);
            if (e == -1) {
                fprintf(stderr, "%s\n", cb_peek(&err));
                ret = 1;
                break;
            } else if (e == 0) {
                break;
            }
            cb_nullterm(&opt);
            if (!strcmp(opt.data, "help")) {
                e = args_getoptval(&a, 0, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                printf("USAGE: %s [OPTION]...\n", argv[0]);
                puts("OPTIONS:");
                puts("    -help                       Display this help text.");
                puts("    -version                    Display version information.");
                puts("    -game=NAME                  Set the game to run.");
                puts("    -mods=NAME[,NAME]...        More mods to load.");
                puts("    -icon=PATH                  Override the game's icon.");
                puts("    -maindir=DIR                Set the main directory.");
                puts("    -userdir=DIR                Set user the directory.");
                puts("    -{set|s} [SECT|VAR=VAL]...  Override config values.");
                puts("    -{config|cfg|c}=FILE        Set the config file path.");
                puts("    -{nouserconfig|nousercfg}   Do not load the user config.");
                puts("    -nocontroller               Do not init controllers.");
                ret = 0;
            } else if (!strcmp(opt.data, "version")) {
                e = args_getoptval(&a, 0, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                puts(verstr);
                puts(platstr);
                #if DEBUG(0)
                printf("Debug level: %d\n", PSRC_DBGLVL);
                #endif
                #ifdef PSRC_USEDISCORDGAMESDK
                puts("Discord game SDK enabled");
                #endif
                #ifdef PSRC_NOSIMD
                puts("No SIMD");
                #endif
                #ifdef PSRC_NOMT
                puts("No multithreading");
                #endif
                ret = 0;
            } else if (!strcmp(opt.data, "game")) {
                e = args_getoptval(&a, 1, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                free(options.game);
                options.game = cb_reinit(&val, 256);
            } else if (!strcmp(opt.data, "mods")) {
                e = args_getoptval(&a, 1, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                free(options.mods);
                options.mods = cb_reinit(&val, 256);
            } else if (!strcmp(opt.data, "icon")) {
                e = args_getoptval(&a, 1, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                free(options.icon);
                options.icon = cb_reinit(&val, 256);
            } else if (!strcmp(opt.data, "set") || !strcmp(opt.data, "s")) {
                e = args_getoptval(&a, 0, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                if (!options.set) options.set = cfg_open(NULL);
                char* sect = NULL;
                while (1) {
                    e = args_getvar(&a, &var, &err);
                    if (e == -1) {
                        fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                        ret = 1;
                        free(sect);
                        goto longbreak;
                    } else if (e == 0) {
                        break;
                    }
                    e = args_getvarval(&a, -1, &val, &err);
                    if (e == -1) {
                        fprintf(stderr, "-%s: %s: %s\n", opt.data, var.data, cb_peek(&err));
                        ret = 1;
                        free(sect);
                        goto longbreak;
                    } else if (e == 0) {
                        free(sect);
                        sect = cb_reinit(&var, 32);
                    } else {
                        cfg_setvar(options.set, sect, cb_peek(&var), cb_peek(&val), true);
                        cb_clear(&val);
                    }
                    cb_clear(&var);
                }
                free(sect);
            } else if (!strcmp(opt.data, "maindir")) {
                e = args_getoptval(&a, 1, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                free(options.maindir);
                options.maindir = cb_reinit(&val, 256);
            } else if (!strcmp(opt.data, "userdir")) {
                e = args_getoptval(&a, 1, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                free(options.userdir);
                options.userdir = cb_reinit(&val, 256);
            } else if (!strcmp(opt.data, "config") || !strcmp(opt.data, "cfg") || !strcmp(opt.data, "c")) {
                e = args_getoptval(&a, 1, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                free(options.config);
                options.config = cb_reinit(&val, 256);
            } else if (!strcmp(opt.data, "nouserconfig") || !strcmp(opt.data, "nousercfg")) {
                e = args_getoptval(&a, 0, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                options.nouserconfig = true;
            } else if (!strcmp(opt.data, "nocontroller")) {
                e = args_getoptval(&a, 0, -1, &val, &err);
                if (e == -1) {
                    fprintf(stderr, "-%s: %s\n", opt.data, cb_peek(&err));
                    ret = 1;
                    break;
                }
                options.nocontroller = true;
            } else {
                fprintf(stderr, "Unknown option: -%s\n", opt.data);
                ret = 1;
                break;
            }
            cb_clear(&opt);
        }
        longbreak:;
        cb_dump(&opt);
        cb_dump(&var);
        cb_dump(&val);
        cb_dump(&err);
        if (ret >= 0) return ret;
    }
    signal(SIGINT, sigh);
    signal(SIGTERM, sigh);
    #ifdef SIGQUIT
    signal(SIGQUIT, sigh);
    #endif
    #ifdef SIGUSR1
    signal(SIGUSR1, SIG_IGN);
    #endif
    #ifdef SIGUSR2
    signal(SIGUSR2, SIG_IGN);
    #endif
    #ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    #endif
    #else
    (void)argc; (void)argv;
    #endif
    #if PLATFORM == PLAT_WIN32
    TIMECAPS tc;
    UINT tmrres = 1;
    if (timeGetDevCaps(&tc, sizeof(tc)) != TIMERR_NOERROR) {
        if (tmrres < tc.wPeriodMin) {
            tmrres = tc.wPeriodMin;
        } else if (tmrres > tc.wPeriodMax) {
            tmrres = tc.wPeriodMax;
        }
    }
    timeBeginPeriod(tmrres);
    QueryPerformanceFrequency(&perfctfreq);
    while (!(perfctfreq.QuadPart % 10) && !(perfctmul % 10)) {
        perfctfreq.QuadPart /= 10;
        perfctmul /= 10;
    }
    #elif PLATFORM == PLAT_NXDK
    perfctfreq = KeQueryPerformanceFrequency();
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    pbgl_set_swap_interval(0);
    //pbgl_set_swap_interval(1);
    pbgl_init(true);
    //pbgl_set_swap_interval(1);
    glClearColor(0.0, 0.25, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pbgl_swap_buffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pbgl_swap_buffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pbgl_swap_buffers();
    #endif
    ret = bootstrap();
    if (!ret) {
        ret = initLoop();
        if (!ret) {
            while (!quitreq) {
                doLoop();
            }
            quitLoop();
        }
        unstrap();
    }
    #if PLATFORM == PLAT_WIN32
    timeEndPeriod(tmrres);
    #elif PLATFORM == PLAT_NXDK
    if (ret) Sleep(5000);
    pbgl_shutdown();
    HalReturnToFirmware(HalQuickRebootRoutine);
    #elif PLATFORM == PLAT_DREAMCAST
    arch_menu();
    #endif
    #else
    // TODO: pass in output from SDL_GetPrefPath
    EM_ASM(
        FS.mkdir('/libsdl');
        FS.mount(IDBFS, {}, '/libsdl');
        FS.syncfs(true, function (e) {
            if (e) console.error("FS.syncfs:", e);
            ccall("syncfsok");
        });
    );
    emscripten_set_main_loop(emscrmain, -1, true);
    ret = 0;
    #endif
    return ret;
}
