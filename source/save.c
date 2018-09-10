#include "save.h"

#include <stdio.h>
#include "io.h"

#define SEED_LEN 0x10

static FsFileSystem g_activeFs;

Result openSystemSavedata(u64 titleId, u64 saveId)
{
    Result rc;
    
    for (int attempt = 0; attempt < 100; attempt++)
    {
        pmshellTerminateProcessByTitleId(titleId);

        if (R_SUCCEEDED(rc = fsMount_SystemSaveData(&g_activeFs, saveId)))
        {
            break;
        }
    }

    // Took too many attempts
    if (R_FAILED(rc))
    {
        printf("Failed to mount system save data %016lx for tid %016lx. Error code: 0x%08x", saveId, titleId, rc);
        return rc;
    }

    if (fsdevMountDevice("s4ve", g_activeFs) == -1) 
    {
        printf("Failed to mount system save data device.\n");
        return -1;
    }

    return rc;
}

Result backupSystemSavedata(u64 titleId, u64 saveId)
{
    Result rc;

    if (R_FAILED(rc = openSystemSavedata(titleId, saveId)))
    {
        printf("Failed to open save data. Error code: 0x%08x\n", rc);
        return rc;
    }

    // Setup folders to copy save data into
    createDir("/switch/");

    if (R_FAILED(rc = createDir("/switch/compelled_disclosure/")))
    {
        printf("Failed to create compelled disclosure folder\n");
        return rc;
    }

    char outPath[FS_MAX_PATH];
    snprintf(outPath, FS_MAX_PATH, "/switch/compelled_disclosure/%016lx/", saveId);

    if (R_FAILED(rc = removeDir(outPath)))
    {
        printf("Failed to remove compelled disclosure folder\n");
        return rc;
    }

    if (R_FAILED(rc = createDir(outPath)))
    {
        printf("Failed to create out dir path %s\n", outPath);
        return rc;
    }

    // Begin copying files
    if (R_FAILED(rc = copyDir("s4ve:/", outPath)))
    {
        printf("Failed to copy s4ve:/ to out path %s\n", outPath);
        return rc;
    }

    return rc;
}

Result restoreSystemSavedata(u64 titleId, u64 saveId)
{
    Result rc;

    if (R_FAILED(rc = openSystemSavedata(titleId, saveId)))
    {
        printf("Failed to open save data. Error code: 0x%08x\n", rc);
        return rc;
    }

    char outPath[FS_MAX_PATH];
    snprintf(outPath, FS_MAX_PATH, "/switch/compelled_disclosure/%016lx/", titleId);

    //TODO: THIS IS BROKEN
    /*if (R_FAILED(rc = removeDir("save:/")))
    {
        printf("Failed to remove save folder\n");
        return rc;
    }*/

    // Begin copying files
    if (R_FAILED(rc = copyDir(outPath, "save:/")))
    {
        printf("Failed to copy save:/ to out path %s\n", outPath);
        return rc;
    }

    return rc;
}

Result backupSDSeed(u64 saveId)
{
    Result rc;

    FILE *src;
    FILE *dst;

    char srcPath[FS_MAX_PATH];
    snprintf(srcPath, FS_MAX_PATH, "/switch/compelled_disclosure/%016lx/private", saveId);

    char dstPath[FS_MAX_PATH];
    snprintf(dstPath, FS_MAX_PATH, "/switch/compelled_disclosure/sdseed.txt");
    
    if (!(src = fopen(srcPath, "rb")))
    {
        printf("Failed to open file %s\n", srcPath);
        return 1;
    }

    if (!(dst = fopen(dstPath, "w")))
    {
        printf("Failed to open file %s\n", dstPath);
        fclose(src);
        return 1;
    }

    u8 seed[SEED_LEN];

    fseek(src, 0x10, SEEK_SET);
    fread(seed, 1, SEED_LEN, src);

    char hex[3];
    for (int i = 0; i < SEED_LEN; i++)
    {
        sprintf(hex, "%02x", seed[i]);
        fwrite(hex, 2, 1, dst);
    }

    fclose(src);
    fclose(dst);

    return rc;
}
