#include "psrc.h"
#include "main.h"

#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>

#ifndef MAX_PATH
#define MAX_PATH 32768
#endif

psrc_main_struct psrc;

uint64_t psrc_main_utime() {
    struct timeval time1;
    gettimeofday(&time1, NULL);
    return time1.tv_sec * 1000000 + time1.tv_usec;
}

void psrc_main_wait(uint64_t d) {
    #ifndef _WIN32
    struct timespec dts;
    dts.tv_sec = d / 1000000;
    dts.tv_nsec = (d % 1000000) * 1000;
    nanosleep(&dts, NULL);
    #else
    uint64_t t = d + psrc_main_utime();
    while (t > psrc_main_utime()) {}
    #endif
}

float psrc_main_randfloat(float num1, float num2) {
    float range = num2 - num1;
    float div = RAND_MAX / range;
    return num1 + (rand() / div);
}

char* psrc_main_getErrorText_text = NULL;

char* psrc_main_getErrorText(char* func, char* estr) {
    psrc_main_getErrorText_text = realloc(psrc_main_getErrorText_text, strlen(func) + strlen(estr) + 3);
    sprintf(psrc_main_getErrorText_text, "%s: %s", func, estr);
    return psrc_main_getErrorText_text;
}

void psrc_main_displayError(int ecode, char* func, char* estr) {
    switch (ecode) {
        case PSRC_INF:;
            #ifndef _WIN32
            printf("i: %s\n", psrc_main_getErrorText(func, estr));
            #else
            MessageBox(NULL, psrc_main_getErrorText(func, estr), "Info", MB_OK | MB_ICONINFORMATION);
            #endif
            break;
        case PSRC_WRN:;
            #ifndef _WIN32
            printf("\e[33;1m!: %s\e[0m\n", psrc_main_getErrorText(func, estr));
            #else
            MessageBox(NULL, psrc_main_getErrorText(func, estr), "Warning", MB_OK | MB_ICONWARNING);
            #endif
            break;
        default:;
            #ifndef _WIN32
            printf("\e[31;1mX: %s\e[0m\n", psrc_main_getErrorText(func, estr));
            #else
            MessageBox(NULL, psrc_main_getErrorText(func, estr), "Error", MB_OK | MB_ICONERROR);
            #endif
            break;
    }
}

char psrc_main_getCfgValBuf[32768];

void psrc_main_getCfgVar(char* fdata, char* var, char* dval, char* out) {
    int spcoff = 0;
    if (!out) return;
    char* oout = out;
    *out = 0;
    if (!var || !dval) return;
    if (!fdata) goto retdef;
    spcskip:;
    while (*fdata == ' ' || *fdata == '\t' || *fdata == '\n') {++fdata;}
    if (!*fdata) goto retdef;
    if (*fdata == '#') {while (*fdata && *fdata != '\n') {++fdata;} goto spcskip;}
    for (char* tvar = var; *fdata; ++fdata, ++tvar) {
        bool upc = false;
        if (*fdata >= 'A' && *fdata <= 'Z') {upc = true;}
        if (*fdata == ' ' || *fdata == '\t' || *fdata == '=') {
            while (*fdata == ' ' || *fdata == '\t') {++fdata;}
            if (*tvar || *fdata != '=') {while (*fdata && *fdata != '\n') {++fdata;} goto spcskip;}
            else {++fdata; goto getval;}
        } else if (upc && (*fdata) + 32 == *tvar) {
        } else if (upc && *fdata == *tvar) {
        } else if (!upc && *fdata == (*tvar) + 32) {
        } else if (*fdata != *tvar) {
            while (*fdata && *fdata != '\n') {++fdata;}
            goto spcskip;
        }
    }
    getval:;
    while (*fdata == ' ' || *fdata == '\t') {++fdata;}
    if (!*fdata) goto retdef;
    else if (*fdata == '\n') goto spcskip;
    bool inStr = false;
    while (*fdata && *fdata != '\n') {
        if (*fdata == '"') {
            inStr = !inStr;
        } else {
            if (*fdata == '\\') {
                if (inStr) {
                    ++fdata;
                    if (*fdata == '"') *out++ = '"';
                    else if (*fdata == 'n') *out++ = '\n';
                    else if (*fdata == 'r') *out++ = '\r';
                    else if (*fdata == 't') *out++ = '\t';
                    else if (*fdata == 'e') *out++ = '\e';
                    else if (*fdata == '\n') *out++ = '\n';
                    else if (*fdata == '\r' && *(fdata + 1) == '\n') {*out++ = '\n'; ++fdata;}
                    else {
                        *out++ = '\\';
                        *out++ = *fdata;
                    }
                } else {
                    ++fdata;
                    if (*fdata == '"') *out++ = '"';
                    else if (*fdata == '\n') *out++ = '\n';
                    else {
                        *out++ = '\\';
                        *out++ = *fdata;
                    }
                }
            } else {
                *out++ = *fdata;
            }
            if (!inStr && (*fdata == ' ' || *fdata == '\t')) ++spcoff;
            else {spcoff = 0;}
        }
        ++fdata;
    }
    goto retok;
    retdef:;
    strcpy(oout, dval);
    return;
    retok:;
    out -= spcoff;
    *out = 0;
}

bool psrc_main_cfgValBool(char* val) {
    if (!val) return -1;
    char* nval = strdup(val);
    for (char* nvalp = nval; *nvalp; ++nvalp) {
        if (*nvalp >= 'A' && *nvalp <= 'Z') *nvalp += 32;
    }
    bool ret = (!strcmp(nval, "true") || !strcmp(nval, "yes") || atoi(nval) != 0);
    free(nval);
    return ret;
}

char* psrc_main_getCfgVarStatic(char* fdata, char* var, char* dval) {
    psrc_main_getCfgVar(fdata, var, dval, psrc_main_getCfgValBuf);
    return psrc_main_getCfgValBuf;
}

char* psrc_main_getCfgVarAlloc(char* fdata, char* var, char* dval) {
    char* ret = malloc(32768);
    psrc_main_getCfgVar(fdata, var, dval, psrc_main_getCfgValBuf);
    ret = realloc(ret, strlen(ret) + 1);
    return ret;
}

char* psrc_main_getTextFile(char* name) {
    struct stat fnst;
    memset(&fnst, 0, sizeof(struct stat));
    if (stat(name, &fnst)) {
        psrc_main_displayError(PSRC_ERR, "psrc_main_getTextFile: File not found", name);
        return NULL;
    }
    if (!S_ISREG(fnst.st_mode)) {
        psrc_main_displayError(PSRC_ERR, "psrc_main_getTextFile: Not a file", name);
        return NULL;
    }
    FILE* file = fopen(name, "r");
    if (!file) {
        psrc_main_displayError(PSRC_ERR, "psrc_main_getTextFile: File not found", name);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    char* data = malloc(size + 1);
    fseek(file, 0, SEEK_SET);
    long int i = 0;
    while (i < size && !feof(file)) {
        int tmpc = fgetc(file);
        if (tmpc < 0) tmpc = 0;
        data[i++] = (char)tmpc;
    }
    data[i] = 0;
    return data;
}

char* psrc_main_getTextFileSilent(char* name) {
    struct stat fnst;
    memset(&fnst, 0, sizeof(struct stat));
    if (stat(name, &fnst)) {
        return NULL;
    }
    if (!S_ISREG(fnst.st_mode)) {
        return NULL;
    }
    FILE* file = fopen(name, "r");
    if (!file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    char* data = malloc(size + 1);
    fseek(file, 0, SEEK_SET);
    long int i = 0;
    while (i < size && !feof(file)) {
        int tmpc = fgetc(file);
        if (tmpc < 0) tmpc = 0;
        data[i++] = (char)tmpc;
    }
    data[i] = 0;
    return data;
}

char* psrc_main_bfnbuf = NULL;

char* psrc_main_basefilename(char* fn) {
    int32_t fnlen = strlen(fn);
    int32_t i;
    for (i = fnlen; i > -1; --i) {
        if (fn[i] == '/') break;
        #ifdef _WIN32
        if (fn[i] == '\\') break;
        #endif
    }
    psrc_main_bfnbuf = realloc(psrc_main_bfnbuf, fnlen - i);
    strcpy(psrc_main_bfnbuf, fn + i + 1);
    return psrc_main_bfnbuf;
}

char* psrc_main_pathfilename(char* fn) {
    int32_t fnlen = strlen(fn);
    int32_t i;
    for (i = fnlen; i > -1; --i) {
        if (fn[i] == '/') break;
        #ifdef _WIN32
        if (fn[i] == '\\') break;
        #endif
    }
    psrc_main_bfnbuf = realloc(psrc_main_bfnbuf, i + 2);
    char tmp = fn[i + 1];
    fn[i + 1] = 0;
    strcpy(psrc_main_bfnbuf, fn);
    fn[i + 1] = tmp;
    return psrc_main_bfnbuf;
}

void psrc_main_cleanExit(int code) {
    free(psrc_main_getErrorText_text);
    if (psrc.ui && psrc.ui->deinit) psrc.ui->deinit();
    if (psrc.gfx && psrc.gfx->deinit) psrc.gfx->deinit();
    if (psrc.sound && psrc.sound->deinit) psrc.sound->deinit();
    SDL_Quit();
    exit(code);
}

void psrc_main_cleanExitSig(int sig) {
    (void)sig;
    psrc.quitRequested = true;
}

float psrc_main_test_posmult = 0.25;
float psrc_main_test_rotmult = 3;
float psrc_main_test_fpsmult = 0;
bool psrc_main_test_ps1gfx = false;
bool psrc_main_test_fpsct = false;
float psrc_main_test_mousesns = 0.125;

static inline void psrc_main_test_input() {
    float pmult = psrc_main_test_posmult;
    float rmult = psrc_main_test_rotmult;
    static bool setui = false;
    if (psrc.gfx->chkKey(GLFW_KEY_ESCAPE)) {
        if (!setui) {
            glfwSetInputMode(psrc.gfx->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            if (!psrc.ui->shown) {
                psrc.ui->showUI();
            } else {
                psrc.ui->hideUI();
            }
            setui = true;
        }
    } else {
        setui = false;
    }
    static int fullscreen = -1;
    if (fullscreen == -1) fullscreen = psrc.gfx->fullscr;
    static bool setfullscr = false;
    if (psrc.gfx->chkKey(GLFW_KEY_LEFT_ALT) == GLFW_PRESS || psrc.gfx->chkKey(GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
        psrc.gfx->chkKey(GLFW_KEY_ENTER);
        if (psrc.gfx->chkKey(GLFW_KEY_ENTER) == GLFW_PRESS) {
            if (!setfullscr) {
                setfullscr = true;
                fullscreen = !fullscreen;
                psrc.gfx->setFullscreen(fullscreen);
            }
        }
    } else {
        setfullscr = false;
    }
    if (!psrc.ui->shown) {
        if (psrc.gfx->chkKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || psrc.gfx->chkKey(GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
            psrc.gfx->campos.y -= psrc_main_test_fpsmult / M_PI * 0.75;
            if (psrc.gfx->campos.y < 1) psrc.gfx->campos.y = 1;
            pmult /= 2;
        } else {
            if (psrc.gfx->campos.y < 2) psrc.gfx->campos.y = 2;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_Z) == GLFW_PRESS) {
            psrc.gfx->camfov = 10;
            pmult /= 2;
            rmult /= 4;
        } else {
            psrc.gfx->camfov = 50;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || psrc.gfx->chkKey(GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
            pmult *= 2;
        }
        static bool mrelease = false;
        static bool mposset = false;
        static double mxpos, mypos;
        if ((glfwGetWindowAttrib(psrc.gfx->window, GLFW_HOVERED) || mrelease) && glfwGetMouseButton(psrc.gfx->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            mrelease = true;
            if (!mposset) {glfwGetCursorPos(psrc.gfx->window, &mxpos, &mypos); mposset = true;}
            static double nmxpos, nmypos;
            glfwGetCursorPos(psrc.gfx->window, &nmxpos, &nmypos);
            psrc.gfx->camrot.x += (mypos - nmypos) * psrc_main_test_mousesns * rmult / psrc_main_test_rotmult;
            psrc.gfx->camrot.y -= (mxpos - nmxpos) * psrc_main_test_mousesns * rmult / psrc_main_test_rotmult;
            if (psrc.gfx->camrot.y < -360) psrc.gfx->camrot.y += 360;
            else if (psrc.gfx->camrot.y > 360) psrc.gfx->camrot.y -= 360;
            if (psrc.gfx->camrot.x > 89.99) psrc.gfx->camrot.x = 89.99;
            if (psrc.gfx->camrot.x < -89.99) psrc.gfx->camrot.x = -89.99;
            mxpos = nmxpos;
            mypos = nmypos;
            glfwSetInputMode(psrc.gfx->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            mrelease = false;
            mposset = false;
            glfwSetInputMode(psrc.gfx->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwGetCursorPos(psrc.gfx->window, &mxpos, &mypos);
        }
        if (psrc.gfx->chkKey(GLFW_KEY_UP) == GLFW_PRESS) {
            psrc.gfx->camrot.x += rmult;
            if (psrc.gfx->camrot.x > 89.99) psrc.gfx->camrot.x = 89.99;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_DOWN) == GLFW_PRESS) {
            psrc.gfx->camrot.x -= rmult;
            if (psrc.gfx->camrot.x < -89.99) psrc.gfx->camrot.x = -89.99;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_LEFT) == GLFW_PRESS) {
            psrc.gfx->camrot.y -= rmult;
            if (psrc.gfx->camrot.y < -360) psrc.gfx->camrot.y += 360;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_RIGHT) == GLFW_PRESS) {
            psrc.gfx->camrot.y += rmult;
            if (psrc.gfx->camrot.y > 360) psrc.gfx->camrot.y -= 360;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_SPACE) == GLFW_PRESS) {
            psrc.gfx->campos.y += psrc_main_test_fpsmult / M_PI * 0.75;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_W) == GLFW_PRESS) {
            float yrotrad = (psrc.gfx->camrot.y / 180 * M_PI);
            psrc.gfx->campos.x += sinf(yrotrad) * pmult;
            psrc.gfx->campos.z -= cosf(yrotrad) * pmult;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_S) == GLFW_PRESS) {
            float yrotrad = (psrc.gfx->camrot.y / 180 * M_PI);
            psrc.gfx->campos.x -= sinf(yrotrad) * pmult;
            psrc.gfx->campos.z += cosf(yrotrad) * pmult;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_A) == GLFW_PRESS) {
            float yrotrad;
            yrotrad = (psrc.gfx->camrot.y / 180 * M_PI);
            psrc.gfx->campos.x -= cosf(yrotrad) * pmult;
            psrc.gfx->campos.z -= sinf(yrotrad) * pmult;
        }
        if (psrc.gfx->chkKey(GLFW_KEY_D) == GLFW_PRESS) {
            float yrotrad;
            yrotrad = (psrc.gfx->camrot.y / 180 * M_PI);
            psrc.gfx->campos.x += cosf(yrotrad) * pmult;
            psrc.gfx->campos.z += sinf(yrotrad) * pmult;
        }
        psrc.gfx->updateCam();
    }
}

static inline void psrc_main_test_renderObjAtFloors(psrc_gfx_obj* obj) {
    obj->pos.x -= 20;
    obj->pos.z -= 20;
    psrc.gfx->pushObj(obj);
    obj->pos.x += 40;
    psrc.gfx->pushObj(obj);
    obj->pos.x -= 40;
    obj->pos.z += 40;
    psrc.gfx->pushObj(obj);
    obj->pos.x += 40;
    psrc.gfx->pushObj(obj);
    obj->pos.x -= 40;
    obj->pos.z -= 40;
    obj->pos.x += 20;
    obj->pos.z += 20;
}

bool psrc_main_test_eventVerbose = false;

void psrc_main_test_testboxCallback(psrc_ui_dialog* box, psrc_ui_elem* elem, psrc_ui_event event) {
    if (psrc_main_test_eventVerbose) printf("event %d on box %d at (%d, %d)", event.event, box->id, event.pos.x, event.pos.y);
    if (elem) {
        if (psrc_main_test_eventVerbose) printf(" on element \"%s\" of type %d", elem->id, elem->type);
        if (elem->type == PSRC_UI_ELEM_BTN) {
            if (event.event == PSRC_UI_EVENT_CLICK) elem->border = PSRC_UI_BDR_CONCAVE;
            else if (event.event == PSRC_UI_EVENT_RELEASE) elem->border = PSRC_UI_BDR_CONVEX;
        } else if (elem->type == PSRC_UI_ELEM_SLIDER) {
            if (event.event == PSRC_UI_EVENT_CLICK || event.event == PSRC_UI_EVENT_HOLD) {
                elem->border = PSRC_UI_BDR_CONCAVE;
                int expos = elem->pos.x;
                int exsize = elem->size.x;
                if (expos < 0) expos = box->size.x - exsize + expos;
                if (exsize < 0) exsize = box->size.x - elem->pos.x + exsize;
                expos += 5;
                exsize -= 14;
                int mpos = event.pos.x - expos;
                if (mpos < 0) mpos = 0;
                if (mpos > exsize - 1) mpos = exsize - 1;
                int prcnt = (float)mpos * 100 / (float)(exsize - 1);
                elem->data = (void*)(uintptr_t)prcnt;
            } else if (event.event == PSRC_UI_EVENT_RELEASE) {
                elem->border = PSRC_UI_BDR_CONVEX;
            }
        } else if (elem->type == PSRC_UI_ELEM_CBOX) {
            if (event.event == PSRC_UI_EVENT_CLICK) elem->data = (void*)(uintptr_t)!(uintptr_t)elem->data;
        } else if (elem->type == PSRC_UI_ELEM_RBTN) {
            if (event.event == PSRC_UI_EVENT_CLICK) {
                for (int i = 0; i < box->elemct; ++i) {
                    psrc_ui_elem* lelem = &box->elems[i];
                    if (lelem->type == PSRC_UI_ELEM_RBTN) lelem->data = (void*)(uintptr_t)0;
                }
                elem->data = (void*)(uintptr_t)1;
            }
        } else if (elem->type == PSRC_UI_ELEM_PBAR) {
            if (event.event == PSRC_UI_EVENT_CLICK) elem->data = (void*)(uintptr_t)psrc.randfloat(0, 100);
        }
    }
    if (psrc_main_test_eventVerbose) putchar('\n');
}

float psrc_main_test_p1l = 1.0f;
float psrc_main_test_p2l = 1.0f;

psrc_gfx_light* psrc_main_test_p1light;
psrc_gfx_light* psrc_main_test_p2light;

void psrc_main_test_lightingBoxCallback(psrc_ui_dialog* box, psrc_ui_elem* elem, psrc_ui_event event) {
    if (psrc_main_test_eventVerbose) printf("event %d on box %d at (%d, %d)", event.event, box->id, event.pos.x, event.pos.y);
    if (elem) {
        if (psrc_main_test_eventVerbose) printf(" on element \"%s\" of type %d", elem->id, elem->type);
        if (elem->type == PSRC_UI_ELEM_SLIDER) {
            if (event.event == PSRC_UI_EVENT_CLICK || event.event == PSRC_UI_EVENT_HOLD) {
                elem->border = PSRC_UI_BDR_CONCAVE;
                int expos = elem->pos.x;
                int exsize = elem->size.x;
                if (expos < 0) expos = box->size.x - exsize + expos;
                if (exsize < 0) exsize = box->size.x - elem->pos.x + exsize;
                expos += 5;
                exsize -= 14;
                int mpos = event.pos.x - expos;
                if (mpos < 0) mpos = 0;
                if (mpos > exsize - 1) mpos = exsize - 1;
                int prcnt = 0;
                if (elem->id[6] == '1') {
                    psrc_main_test_p1l = (float)mpos * 100 / (float)(exsize - 1);
                    prcnt = psrc_main_test_p1l;
                    psrc_main_test_p1l /= 100;
                    psrc_main_test_p1light->ambient = (psrc_color){0.55 * psrc_main_test_p1l, 0.5 * psrc_main_test_p1l, 0.4 * psrc_main_test_p1l};
                    psrc_main_test_p1light->diffuse = (psrc_color){0.65 * psrc_main_test_p1l, 0.6 * psrc_main_test_p1l, 0.6 * psrc_main_test_p1l};
                    psrc.gfx->updateLight(psrc_main_test_p1light->id);
                } else {
                    psrc_main_test_p2l = (float)mpos * 100 / (float)(exsize - 1);
                    prcnt = psrc_main_test_p2l;
                    psrc_main_test_p2l /= 100;
                    psrc_main_test_p2light->ambient = (psrc_color){0.25 * psrc_main_test_p2l, 0.2 * psrc_main_test_p2l, 0.1 * psrc_main_test_p2l};
                    psrc_main_test_p2light->diffuse = (psrc_color){0.9 * psrc_main_test_p2l, 0.7 * psrc_main_test_p2l, 0.6 * psrc_main_test_p2l};
                    psrc.gfx->updateLight(psrc_main_test_p2light->id);
                }
                elem->data = (void*)(uintptr_t)prcnt;
            } else if (event.event == PSRC_UI_EVENT_RELEASE) {
                elem->border = PSRC_UI_BDR_CONVEX;
            }
        }
    }
    if (psrc_main_test_eventVerbose) putchar('\n');
}

psrc_gfx_obj* psrc_main_splash;

static inline void psrc_main_test() {
    uint16_t lbox = psrc.ui->newDialog(
        -2, -2, 200, 48, false, "Loading...",
        false, 0, NULL, 1,
        PSRC_UI_ELEM_PBAR, "lbar", 1, 1, 198, 46, PSRC_UI_BDR_NONE, 0
    );
    psrc_ui_elem* lbar = psrc.ui->getElem(psrc.ui->getDialog(lbox), "lbar");
    psrc.ui->shown = true;
    psrc.gfx->render();
    {
        char* cfg = psrc_main_getTextFileSilent("config/base/test.cfg");
        psrc_main_test_posmult = atof(psrc_main_getCfgVarStatic(cfg, "posmult", "0.25"));
        psrc_main_test_rotmult = atof(psrc_main_getCfgVarStatic(cfg, "rotmult", "3"));
        psrc_main_test_mousesns = atof(psrc_main_getCfgVarStatic(cfg, "mousemult", "0.125"));
        psrc_main_test_ps1gfx = psrc_main_cfgValBool(psrc_main_getCfgVarStatic(cfg, "ps1", "false"));
        psrc_main_test_fpsct = psrc_main_cfgValBool(psrc_main_getCfgVarStatic(cfg, "fps", "false"));
        psrc_main_test_eventVerbose = psrc_main_cfgValBool(psrc_main_getCfgVarStatic(cfg, "events", "false"));
        free(cfg);
    }
    lbar->data = (void*)(uintptr_t)25;
    psrc.gfx->render();
    psrc.gfx->campos = (psrc_coord_3d){0, 1, 0};
    psrc.gfx->camrot = (psrc_coord_3d){0, 270, 0};
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f
    };
    unsigned int indices[] = {
         0,  1,  2,
         3,  4,  5,
         6,  7,  8,
         9, 10, 11,
        12, 13, 14,
        15, 16, 17,
        18, 19, 20,
        21, 22, 23,
        24, 25, 26,
        27, 28, 29,
        30, 31, 32,
        33, 34, 35
    };
    /*
    float vertices2[] = {
         0.8f,  0.8f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f, -0.8f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f,  0.8f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,  0.0f,  0.0f, -1.0f
    };
    */
    float vertices3[] = {
         0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   12.0f, 12.0f,  0.0f,  0.0f, -1.0f,
         0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   12.0f,  0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,    0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,    0.0f, 12.0f,  0.0f,  0.0f, -1.0f
    };
    float vertices4[] = {
         0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   4.0f, 4.0f,  0.0f,  0.0f, -1.0f,
         0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   4.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 4.0f,  0.0f,  0.0f, -1.0f
    };
    float vertices5[] = {
         0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   7.0f, 7.0f,  0.0f,  0.0f, -1.0f,
         0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   7.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 7.0f,  0.0f,  0.0f, -1.0f
    };
    unsigned int indices2[] = {
        0, 1, 3,
        1, 2, 3
    };
    if (psrc_main_test_ps1gfx) {
        psrc.gfx->changeShader(&psrc.gfx->objsprog, "resources/common/shaders/ps1_vertex.glsl", "resources/common/shaders/ps1_fragment.glsl");
        psrc.gfx->texNearFilter = GL_NEAREST;
        psrc.gfx->texFarFilter = GL_NEAREST;
    }
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(psrc.gfx->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    lbar->data = (void*)(uintptr_t)50;
    psrc.gfx->render();
    psrc_gfx_obj* testobj1 = psrc.gfx->newObj((psrc_coord_3d){7, 1.75, 7}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){1, 1, 1},
        vertices, sizeof(vertices), indices, sizeof(indices), "resources/common/textures/kekw.jpg", 256, 0, true);
    psrc_gfx_obj* testobj3 = psrc.gfx->newObj((psrc_coord_3d){1, 0.5, 1}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){1, 1, 1},
        vertices, sizeof(vertices), indices, sizeof(indices), "resources/common/textures/crate.jpg", 16, 0, true);
    psrc_gfx_obj* testobj4 = psrc.gfx->newObj((psrc_coord_3d){-7, 1.75, 7}, (psrc_coord_3d){-45, 0, 0}, (psrc_coord_3d){1, 1, 1},
        vertices, sizeof(vertices), indices, sizeof(indices), NULL, 256, 0, true);
    psrc_gfx_obj* testobj5 = psrc.gfx->newObj((psrc_coord_3d){0, 10, 0}, (psrc_coord_3d){0, 0, 0}, (psrc_coord_3d){8, 8, 8},
        vertices, sizeof(vertices), indices, sizeof(indices), "resources/common/textures/crate.jpg", 16, 0, true);
    psrc_gfx_obj* modelp1 = psrc.gfx->loadObj("resources/common/models/barney/barney.obj", 0, "resources/common/models/barney/DM_Base.bmp", 32, 0, true);
    psrc_gfx_obj* modelp2 = psrc.gfx->loadObj("resources/common/models/barney/barney.obj", 1, "resources/common/models/barney/DM_Face.bmp", 16, 0, true);
    psrc_gfx_obj* modelp3 = psrc.gfx->loadObj("resources/common/models/barney/barney.obj", 2, "resources/common/models/barney/DM_Chrome.bmp", 512, 0, true);
    modelp1->scale = (psrc_coord_3d){0.03, 0.03, 0.03};
    modelp2->scale = (psrc_coord_3d){0.03, 0.03, 0.03};
    modelp3->scale = (psrc_coord_3d){0.03, 0.03, 0.03};
    psrc_gfx_obj* floor1 = psrc.gfx->newObj((psrc_coord_3d){-20, 0, -20}, (psrc_coord_3d){90, 0, 0}, (psrc_coord_3d){25, 25, 25},
        vertices4, sizeof(vertices4), indices2, sizeof(indices2), "resources/common/textures/toxic.jpg", 0, 1, true);
    psrc_gfx_obj* floor2 = psrc.gfx->newObj((psrc_coord_3d){20, 0, -20}, (psrc_coord_3d){90, 0, 0}, (psrc_coord_3d){25, 25, 25},
        vertices5, sizeof(vertices5), indices2, sizeof(indices2), "resources/common/textures/woodenfloor.jpg", 128, 0, true);
    psrc_gfx_obj* floor3 = psrc.gfx->newObj((psrc_coord_3d){-20, 0, 20}, (psrc_coord_3d){90, 0, 0}, (psrc_coord_3d){25, 25, 25},
        vertices3, sizeof(vertices3), indices2, sizeof(indices2), "resources/common/textures/brickwall.jpg", 0, 0, true);
    psrc_gfx_obj* floor4 = psrc.gfx->newObj((psrc_coord_3d){20, 0, 20}, (psrc_coord_3d){90, 0, 0}, (psrc_coord_3d){25, 25, 25},
        vertices4, sizeof(vertices4), indices2, sizeof(indices2), "resources/common/textures/water.jpg", 0.5, 1, true);
    psrc_gfx_light* camlight = psrc.gfx->getNextLight();
    psrc.gfx->setSkybox(psrc.gfx->newSkybox("resources/common/textures/skybox1/", ".bmp"));
    lbar->data = (void*)(uintptr_t)75;
    psrc.gfx->render();
    camlight->type = 1;
    camlight->range = 0.8;
    camlight->diffuse = (psrc_color){0.75, 0.8, 0.75};
    camlight->specular = (psrc_color){0.7, 0.8, 0.7};
    psrc_gfx_light* ambientlight = psrc.gfx->getNextLight();
    ambientlight->type = 2;
    ambientlight->range = 1;
    ambientlight->pos = (psrc_coord_3d){0, 0, 0};
    ambientlight->ambient = (psrc_color){0.2, 0.4, 0.2};
    ambientlight->diffuse = (psrc_color){0.6, 0.9, 0.5};
    ambientlight->direction = (psrc_coord_3d){0, 180, 0};
    ambientlight->topCorner = (psrc_coord_3d){0, 20, 0};
    ambientlight->bottomCorner = (psrc_coord_3d){-40, -0.5, -40};
    psrc.gfx->updateLight(ambientlight->id);
    ambientlight = psrc.gfx->getNextLight();
    psrc_main_test_p1light = ambientlight;
    ambientlight->type = 2;
    ambientlight->range = 1;
    ambientlight->pos = (psrc_coord_3d){40, 20, 0};
    ambientlight->ambient = (psrc_color){0.55, 0.5, 0.4};
    ambientlight->diffuse = (psrc_color){0.65, 0.6, 0.6};
    ambientlight->direction = (psrc_coord_3d){0, -180, 0};
    ambientlight->topCorner = (psrc_coord_3d){0, 20, 0};
    ambientlight->bottomCorner = (psrc_coord_3d){40, -0.5, -40};
    psrc.gfx->updateLight(ambientlight->id);
    ambientlight = psrc.gfx->getNextLight();
    psrc_main_test_p2light = ambientlight;
    ambientlight->type = 2;
    ambientlight->range = 1;
    ambientlight->pos = (psrc_coord_3d){0, 0, 40};
    ambientlight->ambient = (psrc_color){0.25, 0.2, 0.1};
    ambientlight->diffuse = (psrc_color){0.9, 0.7, 0.6};
    ambientlight->direction = (psrc_coord_3d){90, -180, 90};
    ambientlight->topCorner = (psrc_coord_3d){0, 20, 0};
    ambientlight->bottomCorner = (psrc_coord_3d){-40, -0.5, 40};
    psrc.gfx->updateLight(ambientlight->id);
    ambientlight = psrc.gfx->getNextLight();
    ambientlight->type = 2;
    ambientlight->range = 1;
    ambientlight->pos = (psrc_coord_3d){0, 20, 40};
    ambientlight->ambient = (psrc_color){0.4, 0.4, 0.5};
    ambientlight->diffuse = (psrc_color){0.5, 0.5, 0.3};
    ambientlight->direction = (psrc_coord_3d){90, -180, 45};
    ambientlight->topCorner = (psrc_coord_3d){0, 20, 0};
    ambientlight->bottomCorner = (psrc_coord_3d){40, -0.5, 40};
    psrc.gfx->updateLight(ambientlight->id);
    psrc.gfx->setMaxLight(5);
    float opm = psrc_main_test_posmult;
    float orm = psrc_main_test_rotmult;
    camlight->type = 0;
    float fpstimeval = -1;
    int fpsct = 0;
    int fpsbuf[5] = {0, 0, 0, 0, 0};
    int fpsbufp = 0;
    int fpsbufm = 0;
    uint64_t rendtime = 0;
    lbar->data = (void*)(uintptr_t)100;
    psrc.gfx->render();
    psrc.ui->shown = false;
    psrc.ui->closeDialog(lbox);
    psrc.ui->newDialog(-1, -1, 400, 300, true, "Test Window",
        true, PSRC_UI_BTN_CLOSE | PSRC_UI_BTN_RESIZE | PSRC_UI_BTN_HELP, psrc_main_test_testboxCallback, 9,
        PSRC_UI_ELEM_BTN, "button", 10, 10, 128, 24, PSRC_UI_BDR_CONVEX, "Button",
        PSRC_UI_ELEM_TBOX, "textbox", 10, 44, -10, 24, PSRC_UI_BDR_CONCAVE, "Textbox",
        PSRC_UI_ELEM_PBAR, "progressbar", 10, 78, -10, 24, PSRC_UI_BDR_SOLID, 25,
        PSRC_UI_ELEM_SLIDER, "slider", 10, 112, -10, 24, PSRC_UI_BDR_CONVEX, 75,
        PSRC_UI_ELEM_CBOX, "checkbox 1", 10, 146, 24, 24, PSRC_UI_BDR_CONCAVE, 0,
        PSRC_UI_ELEM_CBOX, "checkbox 2", 44, 146, 24, 24, PSRC_UI_BDR_CONCAVE, 1,
        PSRC_UI_ELEM_RBTN, "radio 1", 78, 146, 24, 24, PSRC_UI_BDR_CONCAVE, 0,
        PSRC_UI_ELEM_RBTN, "radio 2", 112, 146, 24, 24, PSRC_UI_BDR_CONCAVE, 1,
        PSRC_UI_ELEM_LIST, "list", 10, 180, -10, -10, PSRC_UI_BDR_SOLID, &(psrc_ui_elem_list){false, 3, false, 0, false, (char*[]){"list", "of", "text"}}
    );
    psrc.ui->newDialog(-1, -1, 480, 122, true, "Lighting",
        true, 0, psrc_main_test_lightingBoxCallback, 4,
        PSRC_UI_ELEM_LABEL, "label1", 10, 10, 128, 12, PSRC_UI_BDR_NONE, "Platform 1",
        PSRC_UI_ELEM_SLIDER, "slider1", 10, 32, -10, 24, PSRC_UI_BDR_CONVEX, 100,
        PSRC_UI_ELEM_LABEL, "label2", 10, 66, 128, 12, PSRC_UI_BDR_NONE, "Platform 2",
        PSRC_UI_ELEM_SLIDER, "slider2", 10, 88, -10, 24, PSRC_UI_BDR_CONVEX, 100
    );
    #if 0
    psrc.ui->newDialog(-1, -1, 160, 120, true, "test2",
        true, PSRC_UI_BTN_HELP, psrc_main_test_testboxCallback, 3,
        PSRC_UI_ELEM_BTN, "b0", 10, 10, 56, 24, 1, "Test1",
        PSRC_UI_ELEM_BTN, "b1", -10, 44, 56, 24, 1, "Test2",
        PSRC_UI_ELEM_TBOX, "b2", 10, 78, 56, 24, 2, "Test3"
    );
    #endif
    //psrc.sound->playMusic("resources/common/music/T2.MOD");
    while (!psrc.gfx->winQuit()) {
        uint64_t starttime = psrc.utime();
        float timeval = glfwGetTime();
        camlight->pos = psrc.gfx->campos;
        psrc.gfx->updateLight(camlight->id);
        if (psrc_main_test_fpsct) {
            if (fpstimeval < 0) fpstimeval = timeval;
            if (timeval - fpstimeval >= 1) {
                fpsbuf[fpsbufp] = fpsct;
                if (fpsbufm < 3) ++fpsbufm;
                int fpstotal = 0;
                for (int i = 0, j = fpsbufp; i < fpsbufm; ++i) {
                    fpstotal += fpsbuf[j];
                    --j; if (j < 0) j = 2;
                }
                fpsbufp = (fpsbufp + 1) % 3;
                uint64_t avgrendtime = rendtime / fpsct;
                printf("%d frames in %f seconds, %d - %d FPS average, %.*f millisecond average render time, vsync %s\n",
                    fpsct, timeval - fpstimeval, (fpstotal / fpsbufm - 4) / 5 * 5, (fpstotal / fpsbufm + 6) / 5 * 5, 3, (float)avgrendtime / 1000,
                    (psrc.gfx->vsync) ? "enabled" : "disabled");
                fpsct = 0;
                rendtime = 0;
                fpstimeval = timeval;
            }
        }
        testobj1->rot.x = timeval * 90;
        testobj1->rot.y = timeval * 90;
        testobj1->rot.z = timeval * 90;
        testobj4->rot.x = timeval * 90;
        testobj4->rot.z = -timeval * 90;
        modelp1->rot.y = -timeval * 22.5;
        modelp2->rot.y = -timeval * 22.5;
        modelp3->rot.y = -timeval * 22.5;
        testobj5->pos.y = 10 + sin(timeval * 1.5) * 1.5;
        testobj5->rot.y = timeval * 45;
        psrc.gfx->pushObj(floor1);
        psrc.gfx->pushObj(floor2);
        psrc.gfx->pushObj(floor3);
        psrc.gfx->pushObj(floor4);
        psrc.gfx->pushObj(testobj5);
        psrc_main_test_renderObjAtFloors(testobj1);
        psrc_main_test_renderObjAtFloors(testobj3);
        psrc_main_test_renderObjAtFloors(testobj4);
        psrc_main_test_renderObjAtFloors(modelp1);
        psrc_main_test_renderObjAtFloors(modelp2);
        psrc_main_test_renderObjAtFloors(modelp3);
        glfwPollEvents();
        psrc_main_test_input();
        if (psrc.ui->shown) psrc.ui->pollUI();
        psrc.gfx->render();
        if (!glfwGetWindowAttrib(psrc.gfx->window, GLFW_FOCUSED)) psrc.wait(1000000 / 15);
        uint64_t delayoffset = psrc.utime() - starttime;
        if (psrc_main_test_fpsct) {
            ++fpsct;
            rendtime += delayoffset;
        }
        if (psrc.gfx->cur_fps) {
            uint64_t delaytime = 1000000 / psrc.gfx->cur_fps - delayoffset;
            if (delaytime < 1000000 / psrc.gfx->cur_fps) psrc.wait(delaytime);
        }
        float initv = vertices4[7];
        float waterspeed = 0.0025 * psrc_main_test_fpsmult;
        vertices4[7] += waterspeed;
        vertices4[8] += waterspeed;
        vertices4[18] += waterspeed;
        vertices4[19] += waterspeed;
        vertices4[29] += waterspeed;
        vertices4[30] += waterspeed;
        vertices4[40] += waterspeed;
        vertices4[41] += waterspeed;
        if (vertices4[7] >= initv * 2) {
            vertices4[7] -= initv;
            vertices4[8] -= initv;
            vertices4[18] -= initv;
            vertices4[19] -= initv;
            vertices4[29] -= initv;
            vertices4[30] -= initv;
            vertices4[40] -= initv;
            vertices4[41] -= initv;
        }
        psrc_main_test_fpsmult = (float)(psrc.utime() - starttime) / (1000000.0f / 60.0f);
        psrc_main_test_posmult = opm * psrc_main_test_fpsmult;
        psrc_main_test_rotmult = orm * psrc_main_test_fpsmult;
    }
}

char psrc_main_getFTextBuf[32768];

char* psrc_main_getFText(char* f, ...) {
    va_list a;
    va_start(a, f);
    vsprintf(psrc_main_getFTextBuf, f, a);
    va_end(a);
    return psrc_main_getFTextBuf;
}

char* psrc_startcmd = NULL;

void psrc_main_init() {
    signal(SIGINT, psrc_main_cleanExitSig);
    psrc = (psrc_main_struct){psrc_main_displayError, psrc_main_wait, psrc_main_utime, psrc_main_randfloat,
        psrc_main_getTextFile, psrc_main_getTextFileSilent, psrc_main_getFText,
        psrc_main_getCfgVar, psrc_main_getCfgVarStatic, psrc_main_getCfgVarAlloc, psrc_main_cfgValBool,
        NULL, NULL, NULL, NULL, false};
    if (!(psrc.sound = psrc_sound_init())) psrc_main_cleanExit(1);
    if (!(psrc.gfx = psrc_gfx_init())) psrc_main_cleanExit(1);
    if (!(psrc.gfx2d = psrc_gfx2d_init())) psrc_main_cleanExit(1);
    if (!(psrc.ui = psrc_ui_init())) psrc_main_cleanExit(1);
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    psrc_startcmd = malloc(MAX_PATH + 1);
    #ifndef _WIN32
        #ifndef __APPLE__
            #ifndef __FreeBSD__
                int32_t scsize;
                if ((scsize = readlink("/proc/self/exe", psrc_startcmd, MAX_PATH)) == -1)
                    #ifndef __linux__
                    if ((scsize = readlink("/proc/curproc/file", psrc_startcmd, MAX_PATH)) == -1)
                    #endif
                        goto scargv;
                psrc_startcmd[scsize] = 0;
                psrc_startcmd = realloc(psrc_startcmd, scsize + 1);
            #else
                int mib[4];
                mib[0] = CTL_KERN;
                mib[1] = KERN_PROC;
                mib[2] = KERN_PROC_PATHNAME;
                mib[3] = -1;
                size_t cb = MAX_PATH;
                sysctl(mib, 4, psrc_startcmd, &cb, NULL, 0);
                psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
                char* tmpstartcmd = realpath(psrc_startcmd, NULL);
                swap(psrc_startcmd, tmpstartcmd);
                nfree(tmpstartcmd);
                psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
            #endif
        #else
            uint32_t tmpsize = MAX_PATH;
            if (_NSGetExecutablePath(psrc_startcmd, &tmpsize)) {
                goto scargv;
            }
            char* tmpstartcmd = realpath(psrc_startcmd, NULL);
            swap(psrc_startcmd, tmpstartcmd);
            nfree(tmpstartcmd);
            psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
        #endif
    #else
        if (GetModuleFileName(NULL, psrc_startcmd, MAX_PATH)) {
            psrc_startcmd = realloc(psrc_startcmd, strlen(psrc_startcmd) + 1);
        } else {
            goto scargv;
        }
    #endif
    goto skipscargv;
    scargv:;
    free(psrc_startcmd);
    if (strcmp(argv[0], psrc_main_basefilename(argv[0]))) {
        #ifndef _WIN32
        psrc_startcmd = realpath(argv[0], NULL);
        #else
        psrc_startcmd = _fullpath(NULL, argv[0], MAX_PATH);
        #endif
    } else {
        psrc_startcmd = argv[0];
    }
    skipscargv:;
    if (chdir(psrc_main_pathfilename(psrc_startcmd))) {psrc.displayError(PSRC_ERR, "main", "Could not find resources folder"); psrc_main_cleanExit(1);}
    srand(psrc_main_utime());
    psrc_main_init();
    psrc.gfx->texNearFilter = GL_LINEAR;
    psrc.gfx->texFarFilter = GL_LINEAR;
    bool splash = true;
    float splashtime = 1.0;
    {
        char* cfg = psrc_main_getTextFileSilent("config/base/main.cfg");
        splash = psrc_main_cfgValBool(psrc_main_getCfgVarStatic(cfg, "splash", "true"));
        splashtime = atof(psrc_main_getCfgVarStatic(cfg, "splashtime", "1.0"));
        free(cfg);
    }
    if (splash) {
        psrc_main_splash = psrc.gfx2d->new2DObj("resources/base/images/splash.bmp");
        psrc_main_splash->rot = (psrc_coord_3d){0, 0, 0};
        psrc.gfx->set2D(true);
        glfwSetTime(0);
        while (glfwGetTime() < splashtime) {
            psrc.gfx->clear();
            psrc.gfx2d->renderObj(psrc_main_splash, 0, psrc.gfx->cur_height - 1, psrc.gfx->cur_width, -psrc.gfx->cur_height, 1);
            psrc.gfx->update();
        }
        psrc.gfx->set2D(false);
    }
    glfwSetTime(0);
    psrc_main_test();
    psrc_main_cleanExit(0);
    return 0;
}
