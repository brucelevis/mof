//
//  CheckRequest.cpp
//  GameSrv
//
//  Created by prcv on 14-5-8.
//
//

#include "CheckRequest.h"

#include "Utf8Matcher.h"
#include "process.h"
#include "DataCfg.h"

#define THAI_CHAR_COLOR_WHITE  1
#define THAI_CHAR_COLOR_DARKGREEN  2
#define THAI_CHAR_COLOR_GREEN  3
#define THAI_CHAR_COLOR_ORANGE  4
#define THAI_CHAR_COLOR_YELLOW  5
#define THAI_CHAR_COLOR_BLUE  6
#define THAI_CHAR_COLOR_PURPLE  7
#define THAI_CHAR_COLOR_PINK  8
#define THAI_CHAR_COLOR_DARKGREY 9

int getThaiCharColor(int c) {
    if ((c >= 0x30) && (c <= 0x39)) return THAI_CHAR_COLOR_WHITE; // 0-9 (-)
    if ((c >= 0x41) && (c <= 0x5a)) return THAI_CHAR_COLOR_WHITE; // A-Z (-)
    if ((c >= 0x61) && (c <= 0x7a)) return THAI_CHAR_COLOR_WHITE; // a-z (-)
    if ((c >= 0xe0b881) && (c <= 0xe0b8ae)) return THAI_CHAR_COLOR_WHITE; // Thai alphabet (WHITE)
    
    if (c == 0xe0b8af) return THAI_CHAR_COLOR_DARKGREEN; // Thai character Paiyannoi (GREEN)
    if (c == 0xe0b8bf) return THAI_CHAR_COLOR_DARKGREEN; // Thai currency symbol (GREEN)
    
    if ((c >= 0xe0b980) && (c <= 0xe0b984)) return THAI_CHAR_COLOR_GREEN; // Thai start vowel (GREEN)
    
    if ((c >= 0xe0b990) && (c <= 0xe0b999)) return THAI_CHAR_COLOR_ORANGE; // Thai numeral (ORANGE)
    
    if (c == 0xe0b8b0) return THAI_CHAR_COLOR_YELLOW; // Thai end vowel (YELLOW)
    if ((c >= 0xe0b8b2) && (c <= 0xe0b8b3)) return THAI_CHAR_COLOR_YELLOW; // Thai end vowel (YELLOW)
    if ((c >= 0xe0b985) && (c <= 0xe0b986)) return THAI_CHAR_COLOR_YELLOW; // Thai end vowel (YELLOW)
    
    if (c == 0xe0b8b1) return THAI_CHAR_COLOR_BLUE; // Thai vowel (BLUE)
    if ((c >= 0xe0b8b4) && (c <= 0xe0b8b7)) return THAI_CHAR_COLOR_BLUE; // Thai vowel (BLUE)
    
    if ((c >= 0xe0b8b8) && (c <= 0xe0b8b9)) return THAI_CHAR_COLOR_PURPLE; // Thai vowel (PURPLE)
    
    if (c == 0xe0b987) return THAI_CHAR_COLOR_PINK; // Thai vowel (PINK)
    if (c == 0xe0b98c) return THAI_CHAR_COLOR_PINK; // Thai vowel (PINK)
    
    if ((c >= 0xe0b988) && (c <= 0xe0b98b)) return THAI_CHAR_COLOR_DARKGREY; // Thai vowel (DARK GREY)
    
    return 0;
}


int sequence_length(unsigned char lead)
{
    if (lead < 0x80)
        return 1;
    else if ((lead >> 5) == 0x6)
        return 2;
    else if ((lead >> 4) == 0xe)
        return 3;
    else if ((lead >> 3) == 0x1e)
        return 4;
    else
        return 0;
}


bool checkThaiNick(const string& str)
{
    // Check Len
    int prevCharType = 1;
    string::const_iterator pos = str.begin();
    string::const_iterator endPos = str.end();
    
    while (pos != endPos)
    {
        unsigned char lead = *pos;
        int utf8Len = sequence_length(lead);
        if (pos + utf8Len > endPos || utf8Len == 0) {
            return false;
        }
        
        int cp = 0;
        for (int i = 0; i < utf8Len; i++) {
            cp = (cp << 8) + (unsigned char)*pos++;
        }
        
        int color = getThaiCharColor(cp);
        if (color == 0) return false; // Not Valid character
        // Test Starter (First Char)
        if ((pos == str.begin()) && (color > THAI_CHAR_COLOR_ORANGE)) return false; // White, Dark Green, Green, Orange only
        if ( (prevCharType == THAI_CHAR_COLOR_DARKGREEN) && (color > THAI_CHAR_COLOR_ORANGE)) return false; // White, Dark Green, Green, Orange only after DARKGREEN
        if ( (prevCharType == THAI_CHAR_COLOR_YELLOW) && (color > THAI_CHAR_COLOR_ORANGE)) return false; // White, Dark Green, Green, Orange only after YELLOW
        if ( (prevCharType == THAI_CHAR_COLOR_BLUE) && (color != THAI_CHAR_COLOR_DARKGREY) && (color > THAI_CHAR_COLOR_ORANGE)) return false; // White, Dark Green, Green, Dark Gray, Orange only after BLUE
        if ( (prevCharType == THAI_CHAR_COLOR_PURPLE) && (color != THAI_CHAR_COLOR_DARKGREY) && (color > THAI_CHAR_COLOR_ORANGE)) return false; // White, Dark Green, Green, Dark Gray,, Orange only after PURPLE
        if ( (prevCharType == THAI_CHAR_COLOR_GREEN) && (color > THAI_CHAR_COLOR_WHITE)) return false; // White only after GREEN
        if ( (prevCharType == THAI_CHAR_COLOR_PINK) && (color > THAI_CHAR_COLOR_ORANGE)) return false; // White, Dark Green, Green, Orange only after PINK
        if ( (prevCharType == THAI_CHAR_COLOR_DARKGREY) && (color > THAI_CHAR_COLOR_YELLOW)) return false; // White, Dark Green, Green, Yellow, Orange only after DARKGREY
        if ( (prevCharType == THAI_CHAR_COLOR_ORANGE) && (color > THAI_CHAR_COLOR_ORANGE)) return false; // White, Dark Green, Green, Orange only after ORANGE
        prevCharType = color;
    }
    return true;
}



bool isValidRoleName(const string& rolename)
{
    static Utf8Matcher matcher;
    static bool inited = false;
    if (!inited)
    {
        matcher.appendMatchString("\\\"\'\t\r\n %()*^+-~!?.");
    }
    
    if (rolename.length() > 64)
    {
        return false;
    }
    
    bool isUtf8 = true;
    bool matched = matcher.match(rolename, isUtf8);
    bool isReserved = ReservedNameMgr::find(rolename.c_str());
    
    if (Process::env.getString("lang") == "en") {
        if (!checkThaiNick(rolename)) {
            return false;
        }
    }
    
    return isUtf8 && !matched && !isReserved;
}
