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

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
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
#include "config.h"

int BitmapWidgetProc(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2);

static int g_num_engines = 0;                   // The number of engines on the currently loaded aircraft
static bool g_disabled = false;                 // Was the plugin disabled by plugin admin?
static bool g_completed = false;                // Did the plugin finish by displaying the popup?
static XPWidgetID g_popups[2] = {NULL, NULL};   // The popup handles
static XPLMMenuID g_menus[2] = {NULL,NULL};     // The plugin menus
static ini_table_s* g_prefs = NULL;             // The preferences handler
static char g_prefs_path[512];

enum menu_choice
{
    menu_popupLocation,
    menu_main
};

/* Called upon interaction with the popup window */
static int PopupHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, long inParam1, long inParam2)
{
    if(inMessage == xpMessage_CloseButtonPushed) {
        XPHideWidget(inWidget);
        return 1;
    }

    return 0;
}

static int ConfigHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, long inParam1, long inParam2)
{
    if(PopupHandler(inMessage, inWidget, inParam1, inParam2)) {
        return 1;
    }

    if(inMessage == xpMsg_PushButtonPressed) {
        XPWidgetID text_field = XPGetNthChildWidget(inWidget, 1);
        char pic_path[512];
        XPGetWidgetDescriptor(text_field, pic_path, 512);
        ini_table_create_entry(g_prefs, "config", "pic_path", pic_path);
    }
}

/* Create the popup */
static void CreateWidget()
{
    char path[512];
    XPLMGetSystemPath(path);
    strcat(path, "Resources");
    strcat(path, XPLMGetDirectorySeparator());
    strcat(path, "plugins");
    strcat(path, XPLMGetDirectorySeparator());
    strcat(path, "LogInDummy");
    strcat(path, XPLMGetDirectorySeparator());
    const char* pic_path = ini_table_get_entry(g_prefs, "config", "pic_path");
    strcat(path, pic_path);

    // All coordinates are relative to the screen, not to the widget itself
    int x = 100, y = 567, x2 = 402, y2 = 300;
    g_popups[0] = XPCreateWidget(x, y, x2, y2, 1, "Reminder", 1, NULL, xpWidgetClass_MainWindow);
    XPSetWidgetProperty(g_popups[0] , xpProperty_MainWindowHasCloseBoxes, 1);
    if(pic_path) {
        XPCreateCustomWidget(x, y - 40, x2, y2, 1, path, 0, g_popups[0], BitmapWidgetProc);
    }

    XPWidgetID caption = XPCreateWidget(x+50, y-15, x2-50, y-35, 1, "Don't forget to log into your virtual airline", 0, g_popups[0], xpWidgetClass_Caption);
    XPAddWidgetCallback(g_popups[0], PopupHandler);
}

static void CreateConfig()
{
    const char* pic_path = ini_table_get_entry(g_prefs, "config", "pic_path");
    int x = 500, y = 567, x2 = 1102, y2 = 400;
    g_popups[1] = XPCreateWidget(x, y, x2, y2, 1, "Configuration", 1, NULL, xpWidgetClass_MainWindow);
    XPSetWidgetProperty(g_popups[1] , xpProperty_MainWindowHasCloseBoxes, 1);
    XPCreateWidget(x+25, y-25, x2-25, y-45, 1, "Enter the path to your background image", 0, g_popups[1], xpWidgetClass_Caption);
    XPCreateWidget(x+25, y-50, x2-25, y-70, 1, pic_path, 0, g_popups[1], xpWidgetClass_TextField);
    XPWidgetID button = XPCreateWidget(x+25, y-75, x+125, y-105, 1, "Save", 0, g_popups[1], xpWidgetClass_Button);
    XPSetWidgetProperty(button, xpProperty_ButtonType, xpPushButton);
    XPAddWidgetCallback(g_popups[1], ConfigHandler);
}

/* The main loop that checks the engine status */
static float EngineCheckCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefCon)
{
    if(g_disabled || g_completed) {
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
    if(!g_popups[0]) {
        CreateWidget();
    } 

    // Show the popup, if it is not visible already
    if(!XPIsWidgetVisible(g_popups[0])) {
        XPShowWidget(g_popups[0]);
    }

    g_completed = true;
    XPLMUnregisterFlightLoopCallback(EngineCheckCallback, NULL);
    return 0.0;
}

static void menu_handler(void* in_menu_ref, void* in_item_ref)
{
    if((int)in_menu_ref == menu_main) {
        if((int)in_item_ref == 1) {
            CreateConfig();
        }

        return;
    }

    size_t choice = (size_t)in_item_ref;
    char buffer[2];
    itoa(choice, buffer, 10);
    ini_table_create_entry(g_prefs, "config", "popup_location", buffer);

    for(int i = 0; i < 2; i++) {
        XPLMCheckMenuItem(g_menus[1], i, choice == i ? xplm_Menu_Checked : xplm_Menu_Unchecked);
    }
}

PLUGIN_API int XPluginEnable(void)
{
    g_disabled = false;
    if(g_num_engines == 0) {
        XPLMDataRef num_engines_ref = XPLMFindDataRef("sim/aircraft/engine/acf_num_engines");
        g_num_engines = XPLMGetDatai(num_engines_ref);
        if(g_num_engines == 0) {
            XPLMDebugString("LogInDummy: Could not read number of engines!!\n");
            return 0;
        }
    }

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

    const int menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "LogInDummy", 0, 0);
    g_menus[0] = XPLMCreateMenu("LogInDummy", XPLMFindPluginsMenu(), menu_container_idx, menu_handler, (void *)menu_main);
    XPLMAppendMenuItem(g_menus[0], "Popup Timing", NULL, 0);
    g_menus[1] = XPLMCreateMenu("Popup Timing", g_menus[0], 0, menu_handler, (void *)menu_popupLocation);
    XPLMAppendMenuItem(g_menus[1], "Show at Aircraft Load", (void *)0, 0);
    XPLMAppendMenuItem(g_menus[1], "Show after Engine Start", (void *)1, 0);

    XPLMGetSystemPath(g_prefs_path);
    strcat(g_prefs_path, "Resources");
    strcat(g_prefs_path, XPLMGetDirectorySeparator());
    strcat(g_prefs_path, "plugins");
    strcat(g_prefs_path, XPLMGetDirectorySeparator());
    strcat(g_prefs_path, "LogInDummy");
    strcat(g_prefs_path, XPLMGetDirectorySeparator());
    strcat(g_prefs_path, "LogInDummy.ini");

    g_prefs = ini_table_create();
    ini_table_read_from_file(g_prefs, g_prefs_path);
    const char* location_val = ini_table_get_entry(g_prefs, "config", "popup_location");
    if(!location_val || strcmp(location_val, "0") == 0) {
        XPLMCheckMenuItem(g_menus[1], 0, xplm_Menu_Checked);
        XPLMCheckMenuItem(g_menus[1], 1, xplm_Menu_Unchecked);
    } else {
        XPLMCheckMenuItem(g_menus[1], 0, xplm_Menu_Unchecked);
        XPLMCheckMenuItem(g_menus[1], 1, xplm_Menu_Checked);
    }

    XPLMAppendMenuItem(g_menus[0], "Configure", (void *)1, 0);
    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    XPDestroyWidget(g_popups[0], 1);
    XPDestroyWidget(g_popups[1], 1);
    for(int i = 0; i < 2; i++) {
        XPLMDestroyMenu(g_menus[i]);
        g_menus[i] = NULL;
    }

    for(int i = 0; i < 2; i++) {
        XPDestroyWidget(g_popups[i], 1);
        g_popups[i] = NULL;
    }

    ini_table_write_to_file(g_prefs, g_prefs_path);
    ini_table_destroy(g_prefs);
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

                    XPLMMenuCheck checked;
                    XPLMCheckMenuItemState(g_menus[1], 0, &checked);
                    if(checked == xplm_Menu_Checked) {
                        CreateWidget();
                        g_completed = true;
                    } else {
                        g_completed = false;
                        XPLMRegisterFlightLoopCallback(EngineCheckCallback, 10.0, NULL);
                    }
                }

                break;
        }
    }
}
