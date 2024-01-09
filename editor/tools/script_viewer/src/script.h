#pragma once
#include "palcommon.h"

class PalScriptParser {

public:
    void parse();
    bool parseInstruction(WORD wScriptEntry);
};