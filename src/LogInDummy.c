// LogInDummy.c
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org>

#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <XPLMDataAccess.h>
#include <XPLMPlugin.h>
#include <XPLMProcessing.h>
#include <XPLMUtilities.h>
#include <XPWidgets.h>
#include <XPStandardWidgets.h>
#include <XPLMMenus.h>

static int g_num_engines = 0;           // The number of engines on the currently loaded aircraft
static bool g_disabled = false;         // Was the plugin disabled by plugin admin?
static bool g_completed = false;        // Did the plugin finish by displaying the popup?
static XPWidgetID g_popup = NULL;       // The popup handle

/* Called upon interaction with the popup window */
static int PopupHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, long inParam1, long inParam2)
{
    if(inMessage == xpMessage_CloseButtonPushed) {
        XPHideWidget(g_popup);
        return 1;
    }

    return 0;
}

/* Create the popup */
static void CreateWidget()
{
    // All coordinates are relative to the screen, not to the widget itself
    int x = 100, y = 550, x2 = 450, y2 = 450;
    g_popup = XPCreateWidget(x, y, x2, y2, 1, "Reminder", 1, NULL, xpWidgetClass_MainWindow);
    XPSetWidgetProperty(g_popup, xpProperty_MainWindowHasCloseBoxes, 1);
    XPCreateWidget(x+50, y-25, x2-50, y2+20, 1, "Don't forget to log into your virtual airline", 0, g_popup, xpWidgetClass_Caption);
    XPAddWidgetCallback(g_popup, PopupHandler);
}

/* The main loop that checks the engine status */
static float EngineCheckCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefCon)
{
    if(g_disabled) {
        XPLMUnregisterFlightLoopCallback(EngineCheckCallback, NULL);
        return 0.0f;
    }

    // Gets the dataref for reading the engine status
    XPLMDataRef engines_states_ref = XPLMFindDataRef("sim/flightmodel/engine/ENGN_running");
    if(!engines_states_ref) {
        XPLMDebugString("LogInDummy: Could not read state of engines!!\n");
        XPLMUnregisterFlightLoopCallback(EngineCheckCallback, NULL);
        return 0.0f;
    }

    // Read the data from the dataref
    int engine_states[8];
    XPLMGetDatavi(engines_states_ref, engine_states, 0, 8);
    for(int i = 0; i < g_num_engines; i++) {
        if(engine_states[i] == 0) {
            return 10.0;
        }
    }

    // If we don't have a popup yet, create one
    if(!g_popup) {
        CreateWidget();
    } 

    // Show the popup, if it is not visible already
    if(!XPIsWidgetVisible(g_popup)) {
        XPShowWidget(g_popup);
    }

    g_completed = true;
    XPLMUnregisterFlightLoopCallback(EngineCheckCallback, NULL);
    return 0.0;
}

PLUGIN_API int XPluginEnable(void)
{
    g_disabled = false;
    if(!g_completed) {
        XPLMRegisterFlightLoopCallback(EngineCheckCallback, 10.0, NULL);
    }

    return 1;
}

PLUGIN_API void XPluginDisable(void)
{
    g_disabled = true;
    XPLMUnregisterFlightLoopCallback(EngineCheckCallback, NULL);
}

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
    strcpy(outName, "LogInDummy");
    strcpy(outSig, "com.borrrden.logindummy");
    strcpy(outDesc, "Reminder to log into virtual airline");

    XPLMDebugString("LogInDummy: Started\n");

    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    XPLMDestroyWindow(g_popup);
    g_popup = NULL;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
    if (inFrom == XPLM_PLUGIN_XPLANE)
    {
        switch(inMsg)
        {
            case XPLM_MSG_PLANE_LOADED:
                if((int)inParam == 0) {
                    // The user's aircraft was loaded, so read the number of engines it has
                    XPLMDataRef num_engines_ref = XPLMFindDataRef("sim/aircraft/engine/acf_num_engines");
                    g_num_engines = XPLMGetDatai(num_engines_ref);
                    if(g_num_engines == 0) {
                        XPLMDebugString("LogInDummy: Could not read number of engines!!\n");
                        break;
                    }

                    g_completed = false;
                    XPLMRegisterFlightLoopCallback(EngineCheckCallback, 10.0, NULL);
                }

                break;
        }
    }
}
