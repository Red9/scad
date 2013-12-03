
#ifndef SRLM_PROPGCC_BOARD_SCAD__
#define SRLM_PROPGCC_BOARD_SCAD__

namespace board {
    
    // Warning: the variables are short, but the values they point to are
    // integers (4 bytes). Don't be confused!
    const unsigned short kEepromUnitAddress             = 0xFFFC;
    const unsigned short kEepromBoardAddress            = 0xFFF8;
    const unsigned short kEepromCanonNumberAddress      = 0xFFF4;
    const unsigned short kEepromTimeZoneMinutes         = 0xFFF0;
    const unsigned short kEepromTimeZoneHours           = 0xFFEC;
    const unsigned short kEepromTimeZoneSign            = 0xFFE8;
    

    const int kBoardAlpha = 0x0000000A;
    const int kBoardBeta  = 0x0000000B;
    const int kBoardBeta2 = 0x000000B2;
    const int kBoardGamma = 0x00000004;

    const int kMAX_SIZE_COMMAND = 10;
    const int kMAX_SIZE_PARAMETER = 40;
}

#endif // SRLM_PROPGCC_BOARD_SCAD__