#include "pal_global.h"
#include "palcfg.h"
#include "script.h"
#include "text.h"
#include "util.h"

namespace engine {

PalGlobals::PalGlobals()
    : _globals(new GLOBALVARS())
{
}

PalGlobals::~PalGlobals()
{
    deinit();
    if (_globals)
        delete _globals;
    _globals = nullptr;
}

INT PalGlobals::init()
{
    _globals->f.fpFBP = UTIL_OpenRequiredFile("fbp.mkf");
    _globals->f.fpMGO = UTIL_OpenRequiredFile("mgo.mkf");
    _globals->f.fpBALL = UTIL_OpenRequiredFile("ball.mkf");
    _globals->f.fpDATA = UTIL_OpenRequiredFile("data.mkf");
    _globals->f.fpF = UTIL_OpenRequiredFile("f.mkf");
    _globals->f.fpFIRE = UTIL_OpenRequiredFile("fire.mkf");
    _globals->f.fpRGM = UTIL_OpenRequiredFile("rgm.mkf");
    _globals->f.fpSSS = UTIL_OpenRequiredFile("sss.mkf");

    //
    // Retrieve game resource version
    //
    if (!isWINVersion(&isWIN95))
        return -1;

    // NOTE: remove it later
    gConfig.fIsWIN95 = isWIN95;

    //
    // Enable AVI playing only when the resource is WIN95
    //
    // gConfig.fEnableAviPlay = gConfig.fEnableAviPlay && gConfig.fIsWIN95;

    //
    // Detect game language only when no message file specified
    //
    // if (!gConfig.pszMsgFile)
    PAL_SetCodePage(detectCodePage("word.dat"));

    //
    // Set decompress function
    //
    Decompress = isWIN95 ? YJ2_Decompress : YJ1_Decompress;

    _globals->lpObjectDesc = isWIN95 ? NULL : PAL_LoadObjectDesc("desc.dat");
    _globals->bCurrentSaveSlot = 1;

    return 0;
}

void PalGlobals::deinit()
{
    //
    // Close all opened files
    //
    UTIL_CloseFile(_globals->f.fpFBP);
    UTIL_CloseFile(_globals->f.fpMGO);
    UTIL_CloseFile(_globals->f.fpBALL);
    UTIL_CloseFile(_globals->f.fpDATA);
    UTIL_CloseFile(_globals->f.fpF);
    UTIL_CloseFile(_globals->f.fpFIRE);
    UTIL_CloseFile(_globals->f.fpRGM);
    UTIL_CloseFile(_globals->f.fpSSS);

    //
    // Free the game data
    //
    free(_globals->g.lprgEventObject);
    free(_globals->g.lprgScriptEntry);
    free(_globals->g.lprgStore);
    free(_globals->g.lprgEnemy);
    free(_globals->g.lprgEnemyTeam);
    free(_globals->g.lprgMagic);
    free(_globals->g.lprgBattleField);
    free(_globals->g.lprgLevelUpMagic);

    //
    // Free the object description data
    //
    if (!gConfig.fIsWIN95)
        PAL_FreeObjectDesc(_globals->lpObjectDesc);

    //
    // Clear the instance
    //
    memset(_globals, 0, sizeof(GLOBALVARS));

    // PAL_FreeConfig();
}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define DO_BYTESWAP(buf, size)
#else
#define DO_BYTESWAP(buf, size)                                     \
    do {                                                           \
        int i;                                                     \
        for (i = 0; i < (size) / 2; i++) {                         \
            ((LPWORD)(buf))[i] = SDL_SwapLE16(((LPWORD)(buf))[i]); \
        }                                                          \
    } while (0)
#endif

#define LOAD_DATA(buf, size, chunknum, fp)                         \
    do {                                                           \
        PAL_MKFReadChunk((LPBYTE)(buf), (size), (chunknum), (fp)); \
        DO_BYTESWAP(buf, size);                                    \
    } while (0)

void PalGlobals::initGameData(INT iSaveSlot)
{
    initGlobalGameData();

    _globals->bCurrentSaveSlot = (BYTE)iSaveSlot;

    //
    // try loading from the saved game file.
    //
    if (iSaveSlot == 0 || loadGame(iSaveSlot) != 0) {
        //
        // Cannot load the saved game file. Load the defaults.
        //
        loadDefaultGame();
    }

    _globals->iCurInvMenuItem = 0;
    _globals->fInBattle = FALSE;

    memset(_globals->rgPlayerStatus, 0, sizeof(_globals->rgPlayerStatus));

    updateEquipments();
}

void PalGlobals::initGlobalGameData()
{
    int len;

#define PAL_DOALLOCATE(fp, num, type, lptype, ptr, n)                               \
    {                                                                               \
        len = PAL_MKFGetChunkSize(num, fp);                                         \
        ptr = (lptype)malloc(len);                                                  \
        n = len / sizeof(type);                                                     \
        if (ptr == NULL) {                                                          \
            TerminateOnError("PAL_InitGlobalGameData(): Memory allocation error!"); \
        }                                                                           \
    }

    //
    // If the memory has not been allocated, allocate first.
    //
    if (_globals->g.lprgEventObject == NULL) {
        PAL_DOALLOCATE(_globals->f.fpSSS, 0, EVENTOBJECT, LPEVENTOBJECT,
            _globals->g.lprgEventObject, _globals->g.nEventObject);

        PAL_DOALLOCATE(_globals->f.fpSSS, 4, SCRIPTENTRY, LPSCRIPTENTRY,
            _globals->g.lprgScriptEntry, _globals->g.nScriptEntry);

        PAL_DOALLOCATE(_globals->f.fpDATA, 0, STORE, LPSTORE,
            _globals->g.lprgStore, _globals->g.nStore);

        PAL_DOALLOCATE(_globals->f.fpDATA, 1, ENEMY, LPENEMY,
            _globals->g.lprgEnemy, _globals->g.nEnemy);

        PAL_DOALLOCATE(_globals->f.fpDATA, 2, ENEMYTEAM, LPENEMYTEAM,
            _globals->g.lprgEnemyTeam, _globals->g.nEnemyTeam);

        PAL_DOALLOCATE(_globals->f.fpDATA, 4, MAGIC, LPMAGIC,
            _globals->g.lprgMagic, _globals->g.nMagic);

        PAL_DOALLOCATE(_globals->f.fpDATA, 5, BATTLEFIELD, LPBATTLEFIELD,
            _globals->g.lprgBattleField, _globals->g.nBattleField);

        PAL_DOALLOCATE(_globals->f.fpDATA, 6, LEVELUPMAGIC_ALL, LPLEVELUPMAGIC_ALL,
            _globals->g.lprgLevelUpMagic, _globals->g.nLevelUpMagic);

        readGlobalGameData();
    }
#undef PAL_DOALLOCATE
}

void PalGlobals::readGlobalGameData()
{
    const GAMEDATA* p = &_globals->g;

    LOAD_DATA(p->lprgScriptEntry, p->nScriptEntry * sizeof(SCRIPTENTRY),
        4, _globals->f.fpSSS);

    LOAD_DATA(p->lprgStore, p->nStore * sizeof(STORE), 0, _globals->f.fpDATA);
    LOAD_DATA(p->lprgEnemy, p->nEnemy * sizeof(ENEMY), 1, _globals->f.fpDATA);
    LOAD_DATA(p->lprgEnemyTeam, p->nEnemyTeam * sizeof(ENEMYTEAM),
        2, _globals->f.fpDATA);
    LOAD_DATA(p->lprgMagic, p->nMagic * sizeof(MAGIC), 4, _globals->f.fpDATA);
    LOAD_DATA(p->lprgBattleField, p->nBattleField * sizeof(BATTLEFIELD),
        5, _globals->f.fpDATA);
    LOAD_DATA(p->lprgLevelUpMagic, p->nLevelUpMagic * sizeof(LEVELUPMAGIC_ALL),
        6, _globals->f.fpDATA);
    LOAD_DATA(p->rgwBattleEffectIndex, sizeof(p->rgwBattleEffectIndex),
        11, _globals->f.fpDATA);
    PAL_MKFReadChunk((LPBYTE) & (p->EnemyPos), sizeof(p->EnemyPos),
        13, _globals->f.fpDATA);
    DO_BYTESWAP(&(p->EnemyPos), sizeof(p->EnemyPos));
    PAL_MKFReadChunk((LPBYTE)(p->rgLevelUpExp), sizeof(p->rgLevelUpExp),
        14, _globals->f.fpDATA);
    DO_BYTESWAP(p->rgLevelUpExp, sizeof(p->rgLevelUpExp));
}

INT PalGlobals::loadGame(int iSaveSlot)
{
    return isWIN95 ? loadGame_WIN(iSaveSlot) : loadGame_DOS(iSaveSlot);
}

INT PalGlobals::loadGame_WIN(int iSaveSlot)
{
    SAVEDGAME_WIN* s = (SAVEDGAME_WIN*)malloc(sizeof(SAVEDGAME_WIN));

    //
    // Get all the data from the saved game struct.
    //
    if (!loadGame_Common(iSaveSlot, (LPSAVEDGAME_COMMON)s, sizeof(SAVEDGAME_WIN)))
        return -1;

    memcpy(_globals->g.rgObject, s->rgObject, sizeof(_globals->g.rgObject));
    memcpy(_globals->g.lprgEventObject, s->rgEventObject, sizeof(EVENTOBJECT) * _globals->g.nEventObject);

    free(s);

    //
    // Success
    //
    return 0;
}

INT PalGlobals::loadGame_DOS(int iSaveSlot)
{
    SAVEDGAME_DOS* s = (SAVEDGAME_DOS*)malloc(sizeof(SAVEDGAME_DOS));
    int i;

    //
    // Get all the data from the saved game struct.
    //
    if (!loadGame_Common(iSaveSlot, (LPSAVEDGAME_COMMON)s, sizeof(SAVEDGAME_DOS)))
        return -1;

    //
    // Convert the DOS-style data structure to WIN-style data structure
    //
    for (i = 0; i < MAX_OBJECTS; i++) {
        memcpy(&_globals->g.rgObject[i], &s->rgObject[i], sizeof(OBJECT_DOS));
        _globals->g.rgObject[i].rgwData[6] = s->rgObject[i].rgwData[5]; // wFlags
        _globals->g.rgObject[i].rgwData[5] = 0; // wScriptDesc or wReserved2
    }
    memcpy(_globals->g.lprgEventObject, s->rgEventObject, sizeof(EVENTOBJECT) * _globals->g.nEventObject);

    free(s);

    //
    // Success
    //
    return 0;
}

BOOL PalGlobals::loadGame_Common(int iSaveSlot, LPSAVEDGAME_COMMON s, size_t size)
{
    //
    // Try to open the specified file
    //
    FILE* fp = UTIL_OpenFileAtPath(pszSavePath, PAL_va(1, "%d.rpg", iSaveSlot));
    //
    // Read all data from the file and close.
    //
    size_t n = fp ? fread(s, 1, size, fp) : 0;

    if (fp != NULL) {
        fclose(fp);
    }

    if (n < size - sizeof(EVENTOBJECT) * MAX_EVENT_OBJECTS) {
        return FALSE;
    }

    //
    // Adjust endianness
    //
    DO_BYTESWAP(s, size);

    //
    // Cash amount is in DWORD, so do a wordswap in Big-Endian.
    //
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    s->dwCash = ((s->dwCash >> 16) | (s->dwCash << 16));
#endif

    //
    // Get common data from the saved game struct.
    //
    _globals->viewport = PAL_XY(s->wViewportX, s->wViewportY);
    _globals->wMaxPartyMemberIndex = s->nPartyMember;
    _globals->wNumScene = s->wNumScene;
    _globals->fNightPalette = (s->wPaletteOffset != 0);
    _globals->wPartyDirection = s->wPartyDirection;
    _globals->wNumMusic = s->wNumMusic;
    _globals->wNumBattleMusic = s->wNumBattleMusic;
    _globals->wNumBattleField = s->wNumBattleField;
    _globals->wScreenWave = s->wScreenWave;
    _globals->sWaveProgression = 0;
    _globals->wCollectValue = s->wCollectValue;
    _globals->wLayer = s->wLayer;
    _globals->wChaseRange = s->wChaseRange;
    _globals->wChasespeedChangeCycles = s->wChasespeedChangeCycles;
    _globals->nFollower = s->nFollower;
    _globals->dwCash = s->dwCash;
#ifndef PAL_CLASSIC
    _globals->bBattleSpeed = s->wBattleSpeed;
    if (_globals->bBattleSpeed > 5 || _globals->bBattleSpeed == 0) {
        _globals->bBattleSpeed = 2;
    }
#endif

    memcpy(_globals->rgParty, s->rgParty, sizeof(_globals->rgParty));
    memcpy(_globals->rgTrail, s->rgTrail, sizeof(_globals->rgTrail));
    _globals->Exp = s->Exp;
    _globals->g.PlayerRoles = s->PlayerRoles;
    memset(_globals->rgPoisonStatus, 0, sizeof(_globals->rgPoisonStatus));
    memcpy(_globals->rgInventory, s->rgInventory, sizeof(_globals->rgInventory));
    memcpy(_globals->g.rgScene, s->rgScene, sizeof(_globals->g.rgScene));

    _globals->fEnteringScene = FALSE;

    compressInventory();

    return TRUE;
}

void PalGlobals::compressInventory()
{
    int i, j;

    j = 0;

    for (i = 0; i < MAX_INVENTORY; i++) {
        // removed detect zero then break code, due to incompatible with save file hacked by palmod

        if (_globals->rgInventory[i].nAmount > 0) {
            _globals->rgInventory[j] = _globals->rgInventory[i];
            j++;
        }
    }

    for (; j < MAX_INVENTORY; j++) {
        _globals->rgInventory[j].nAmount = 0;
        _globals->rgInventory[j].nAmountInUse = 0;
        _globals->rgInventory[j].wItem = 0;
    }
}

void PalGlobals::loadDefaultGame()
{
    GAMEDATA* p = &_globals->g;
    UINT32 i;

    //
    // Load the default data from the game data files.
    //
    LOAD_DATA(p->lprgEventObject, p->nEventObject * sizeof(EVENTOBJECT),
        0, _globals->f.fpSSS);
    PAL_MKFReadChunk((LPBYTE)(p->rgScene), sizeof(p->rgScene), 1, _globals->f.fpSSS);
    DO_BYTESWAP(p->rgScene, sizeof(p->rgScene));
    if (isWIN95) {
        PAL_MKFReadChunk((LPBYTE)(p->rgObject), sizeof(p->rgObject), 2, _globals->f.fpSSS);
        DO_BYTESWAP(p->rgObject, sizeof(p->rgObject));
    } else {
        OBJECT_DOS objects[MAX_OBJECTS];
        PAL_MKFReadChunk((LPBYTE)(objects), sizeof(objects), 2, _globals->f.fpSSS);
        DO_BYTESWAP(objects, sizeof(objects));
        //
        // Convert the DOS-style data structure to WIN-style data structure
        //
        for (i = 0; i < MAX_OBJECTS; i++) {
            memcpy(&p->rgObject[i], &objects[i], sizeof(OBJECT_DOS));
            p->rgObject[i].rgwData[6] = objects[i].rgwData[5]; // wFlags
            p->rgObject[i].rgwData[5] = 0; // wScriptDesc or wReserved2
        }
    }

    PAL_MKFReadChunk((LPBYTE)(&(p->PlayerRoles)), sizeof(PLAYERROLES),
        3, _globals->f.fpDATA);
    DO_BYTESWAP(&(p->PlayerRoles), sizeof(PLAYERROLES));

    //
    // Set some other default data.
    //
    _globals->dwCash = 0;
    _globals->wNumMusic = 0;
    _globals->wNumPalette = 0;
    _globals->wNumScene = 1;
    _globals->wCollectValue = 0;
    _globals->fNightPalette = FALSE;
    _globals->wMaxPartyMemberIndex = 0;
    _globals->viewport = PAL_XY(0, 0);
    _globals->wLayer = 0;
    _globals->nFollower = 0;
    _globals->wChaseRange = 1;
#ifndef PAL_CLASSIC
    _globals->bBattleSpeed = 2;
#endif

    memset(_globals->rgInventory, 0, sizeof(_globals->rgInventory));
    memset(_globals->rgPoisonStatus, 0, sizeof(_globals->rgPoisonStatus));
    memset(_globals->rgParty, 0, sizeof(_globals->rgParty));
    memset(_globals->rgTrail, 0, sizeof(_globals->rgTrail));
    memset(&(_globals->Exp), 0, sizeof(_globals->Exp));

    for (i = 0; i < MAX_PLAYER_ROLES; i++) {
        _globals->Exp.rgPrimaryExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
        _globals->Exp.rgHealthExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
        _globals->Exp.rgMagicExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
        _globals->Exp.rgAttackExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
        _globals->Exp.rgMagicPowerExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
        _globals->Exp.rgDefenseExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
        _globals->Exp.rgDexterityExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
        _globals->Exp.rgFleeExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
    }

    _globals->fEnteringScene = TRUE;
}

void PalGlobals::updateEquipments()
{
    int i, j;
    WORD w;

    memset(&(_globals->rgEquipmentEffect), 0, sizeof(_globals->rgEquipmentEffect));

    for (i = 0; i < MAX_PLAYER_ROLES; i++) {
        for (j = 0; j < MAX_PLAYER_EQUIPMENTS; j++) {
            w = _globals->g.PlayerRoles.rgwEquipment[j][i];

            if (w != 0) {
                _globals->g.rgObject[w].item.wScriptOnEquip = PAL_RunTriggerScript(_globals->g.rgObject[w].item.wScriptOnEquip, (WORD)i);
            }
        }
    }
}

BOOL PalGlobals::isWINVersion(BOOL* pfIsWIN95)
{
    FILE* fps[] = { UTIL_OpenRequiredFile("abc.mkf"), UTIL_OpenRequiredFile("map.mkf"), _globals->f.fpF, _globals->f.fpFBP, _globals->f.fpFIRE, _globals->f.fpMGO };
    uint8_t* data = NULL;
    int data_size = 0, dos_score = 0, win_score = 0;
    BOOL result = FALSE;

    for (int i = 0; i < sizeof(fps) / sizeof(FILE*); i++) {
        //
        // Find the first non-empty sub-file
        //
        int count = PAL_MKFGetChunkCount(fps[i]), j = 0, size;
        while (j < count && (size = PAL_MKFGetChunkSize(j, fps[i])) < 4)
            j++;
        if (j >= count)
            goto PAL_IsWINVersion_Exit;

        //
        // Read the content and check the compression signature
        // Note that this check is not 100% correct, however in incorrect situations,
        // the sub-file will be over 784MB if uncompressed, which is highly unlikely.
        //
        if (data_size < size)
            data = (uint8_t*)realloc(data, data_size = size);
        PAL_MKFReadChunk(data, data_size, j, fps[i]);
        if (data[0] == 'Y' && data[1] == 'J' && data[2] == '_' && data[3] == '1') {
            if (win_score > 0)
                goto PAL_IsWINVersion_Exit;
            else
                dos_score++;
        } else {
            if (dos_score > 0)
                goto PAL_IsWINVersion_Exit;
            else
                win_score++;
        }
    }

    //
    // Finally check the size of object definition
    //
    data_size = PAL_MKFGetChunkSize(2, _globals->f.fpSSS);
    if (data_size % sizeof(OBJECT) == 0 && data_size % sizeof(OBJECT_DOS) != 0 && dos_score > 0)
        goto PAL_IsWINVersion_Exit;
    if (data_size % sizeof(OBJECT_DOS) == 0 && data_size % sizeof(OBJECT) != 0 && win_score > 0)
        goto PAL_IsWINVersion_Exit;

    if (pfIsWIN95)
        *pfIsWIN95 = (win_score == sizeof(fps) / sizeof(FILE*)) ? TRUE : FALSE;

    result = TRUE;

PAL_IsWINVersion_Exit:
    free(data);
    fclose(fps[1]);
    fclose(fps[0]);

    return result;
}

CODEPAGE PalGlobals::detectCodePage(const char* filename)
{
    FILE* fp;
    char* word_buf = NULL;
    size_t word_len;
    CODEPAGE cp = CP_BIG5;

    if (NULL != (fp = UTIL_OpenFile(filename))) {
        fseek(fp, 0, SEEK_END);
        word_len = ftell(fp);
        word_buf = (char*)malloc(word_len);
        fseek(fp, 0, SEEK_SET);
        word_len = fread(word_buf, 1, word_len, fp);
        UTIL_CloseFile(fp);
        // Eliminates null characters so that PAL_MultiByteToWideCharCP works properly
        for (char* ptr = word_buf; ptr < word_buf + word_len; ptr++) {
            if (!*ptr)
                *ptr = ' ';
        }
    }

    if (word_buf) {
        int probability;
        cp = PAL_DetectCodePageForString(word_buf, (int)word_len, cp, &probability);

        free(word_buf);

        if (probability == 100)
            UTIL_LogOutput(LOGLEVEL_INFO, "detectCodePage detected code page '%s' for %s\n", cp ? "GBK" : "BIG5", filename);
        else
            UTIL_LogOutput(LOGLEVEL_WARNING, "detectCodePage detected the most possible (%d) code page '%s' for %s\n", probability, cp ? "GBK" : "BIG5", filename);
    }

    return cp;
}

}
