#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <switch.h>

#include "save.h"

void loopInput()
{
    printf("\nPress B to exit.\n");

    while (appletMainLoop())
    {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_B) 
           break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }
}

#define ES_TITLE_ID 0x0100000000000033
#define ES_COMMON_SAVE_ID 0x80000000000000E1
#define ES_PERSONALIZED_SAVE_ID 0x80000000000000E2

int main(int argc, char **argv)
{
    Result rc;

    gfxInitDefault();
    consoleInit(NULL);

    if (R_FAILED(rc = pmshellInitialize()))
    {
        printf("Failed to initialize pm:shell. Error code: 0x%08x\n", rc);
        goto loop_input;
    }

    printf("Backing up es common save data...\n");
    backupSystemSavedata(ES_TITLE_ID, ES_COMMON_SAVE_ID);
    fsdevUnmountDevice("s4ve");
    printf("Backing up es personalized save data...\n");
    backupSystemSavedata(ES_TITLE_ID, ES_PERSONALIZED_SAVE_ID);
    fsdevUnmountDevice("s4ve");

    loop_input:
    loopInput();

    pmshellExit();
    gfxExit();
    return 0;
}

