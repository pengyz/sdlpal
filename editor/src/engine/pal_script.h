#pragma once
#include "global.h"
#include "palcommon.h"

void printScript(WORD wScriptEntry, LPSCRIPTENTRY pScript);

class PalScriptParser {

public:
    void parse();
    bool parseInstruction(WORD wScriptEntry);
};