#include "script.h"
#include "global.h"
#include "palcfg.h"
#include "text.h"
#include <codecvt>
#include <locale>
#include <string>

PAL_FORCE_INLINE
INT MESSAGE_GetSpan(
    WORD* pwScriptEntry)
/*++
 Purpose:

 Get the final span of a message block which started from message index of wScriptEntry

 Parameters:

 [IN]  pwScriptEntry - The pointer of script entry which starts the message block, must be a 0xffff command.

 Return value:

 The final span of the message block.

 --*/
{
    int currentScriptEntry = *pwScriptEntry;
    int result = 0;
    int beginning = 1;
    int firstMsgIndex, lastMsgIndex;

    // ensure the command is 0xFFFF
    assert(gpGlobals->g.lprgScriptEntry[currentScriptEntry].wOperation == 0xFFFF);

    firstMsgIndex = lastMsgIndex = gpGlobals->g.lprgScriptEntry[currentScriptEntry].rgwOperand[0];

    //
    // If the NEXT command is 0xFFFF, but the message index is not continuous or not incremental,
    // this MESSAGE block shoud end at THIS command.
    //
    if (gpGlobals->g.lprgScriptEntry[currentScriptEntry + 1].wOperation == 0xFFFF && gpGlobals->g.lprgScriptEntry[currentScriptEntry + 1].rgwOperand[0] != lastMsgIndex + 1)
        currentScriptEntry++;
    else
        while ((gpGlobals->g.lprgScriptEntry[currentScriptEntry].wOperation == 0xFFFF && (!beginning ? gpGlobals->g.lprgScriptEntry[currentScriptEntry].rgwOperand[0] == lastMsgIndex + 1 : 1))
            || gpGlobals->g.lprgScriptEntry[currentScriptEntry].wOperation == 0x008E) {
            if (gpGlobals->g.lprgScriptEntry[currentScriptEntry].wOperation == 0xFFFF)
                lastMsgIndex = gpGlobals->g.lprgScriptEntry[currentScriptEntry].rgwOperand[0];
            currentScriptEntry++;
            beginning = 0;
        }

    result = lastMsgIndex - firstMsgIndex;
    assert(result >= 0);
    *pwScriptEntry = currentScriptEntry;
    return result;
}

enum class ArgFmt {
    INVALID = 0,
    INT,
    ENTRY,
    BOOL,
};

struct ParamFormat {
    const char* paramName;
    ArgFmt format;
};

struct ScriptParserItem {
    const char* opCode;
    ParamFormat args[3];
};

static ScriptParserItem g_parse_table[] = {
// clang-format off
#define X(name, ...) { #name, { __VA_ARGS__ } }
#define U() { nullptr, { N, N, N } }


#define N { nullptr, ArgFmt::INVALID }
#define I(name) { #name, ArgFmt::INT }
#define B(name) { #name, ArgFmt::BOOL }
#define E(name) { #name, ArgFmt::ENTRY }
#include "script_opcode.h"
#undef X
#undef U
#undef N
#undef I
#undef B
#undef E
    // clang-format on
};

char* formatParam(LPSCRIPTENTRY pScript, ScriptParserItem* item)
{
    static char args_buf[128 * 3];
    for (int i = 0; i < 3; i++) {
        char* buf = &args_buf[i * 128];
        ParamFormat* fmt = &item->args[i];
        switch (fmt->format) {
        case ArgFmt::INVALID: {
            strcpy(buf, "nil");
        } break;
        case ArgFmt::INT: {
            sprintf(buf, "%s: %d", fmt->paramName, pScript->rgwOperand[i]);
        } break;
        case ArgFmt::ENTRY: {
            sprintf(buf, "%s: %.4x", fmt->paramName, pScript->rgwOperand[i]);
        };
        case ArgFmt::BOOL: {
            sprintf(buf, "%s: %s", fmt->paramName, pScript->rgwOperand[i] ? "true" : "false");
        } break;
        default:
            assert(0);
        }
    }
    return args_buf;
}

void PalScriptParser::parse()
{
    auto g = &gpGlobals->g;
    for (WORD wScriptEntry = 0; wScriptEntry < g->nScriptEntry; wScriptEntry++) {
        LPSCRIPTENTRY pScript = &g->lprgScriptEntry[wScriptEntry];
        ScriptParserItem* item = &g_parse_table[pScript->wOperation == 0xFFFF ? 0xA8 : pScript->wOperation];
        if (!item || !item->opCode) {
            printf("%.4x: %.4x %.4x %.4x %.4x ====\n", wScriptEntry,
                pScript->wOperation, pScript->rgwOperand[0],
                pScript->rgwOperand[1], pScript->rgwOperand[2]);
            continue;
        }
        char* bufs = formatParam(pScript, item);
        if (!strcmp(&bufs[0 * 128], "nil")) {
            printf("%.4x: %s()", wScriptEntry, item->opCode);
        } else if (!strcmp(&bufs[1 * 128], "nil")) {
            printf("%.4x: %s(%s)", wScriptEntry, item->opCode, &bufs[0 * 128]);
        } else if (!strcmp(&bufs[2 * 128], "nil")) {
            printf("%.4x: %s(%s, %s)", wScriptEntry, item->opCode, &bufs[0 * 128], &bufs[1 * 128]);
        } else {
            printf("%.4x: %s(%s, %s, %s)", wScriptEntry, item->opCode, &bufs[0 * 128], &bufs[1 * 128], &bufs[2 * 128]);
        }
        if (pScript->wOperation == 0xFFFF) {
            // handle text logic
            std::wstring wmsg;
            if (gConfig.pszMsgFile) {
                int msgSpan = MESSAGE_GetSpan(&wScriptEntry);
                int idx = 0, iMsg;
                while ((iMsg = PAL_GetMsgNum(pScript->rgwOperand[0], msgSpan, idx++)) >= 0) {
                    if (iMsg == 0) {
                        //
                        // Restore the screen
                        //
                    } else {
                        wmsg.append(PAL_GetMsg(iMsg)).append(L"\n");
                    }
                }
            } else {
                wmsg = PAL_GetMsg(pScript->rgwOperand[0]);
            }
            std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
            std::string msg = utf8_conv.to_bytes(wmsg);
            printf(" text: %s", msg.c_str());
        }
        printf("\n");
    }
}