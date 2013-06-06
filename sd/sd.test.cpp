// Copyright 2013 SRLM and Red9

#include <propeller.h>
#include "unity.h"
#include "securedigitalcard.h"
#include <stdlib.h>

const int kDoPin = 10;
const int kClkPin = 11;
const int kDiPin = 12;
const int kCsPin = 13;

const int kDoPinNoSd = 18;
const int kClkPinNoSd = 20;
const int kDiPinNoSd = 19;
const int kCsPinNoSd = 21;


/**
 * 
 * Hardware requirements:
 * SD card on pins 10-13
 * No SD card on pins 18-21, with safe to set these pins to arbitrary states.
 * 
 * Improvements:
 *  The current implementation uses a single instance of the class. It would be
 * better to use a fresh instance each time.
 * 
 * 
 */


SecureDigitalCard sut;
int mountResult = -1;

int help_DeleteAllFiles(void) {
    const int MAXIMUM_FILE_COUNT = 1000;
    int count;
    for (count = 0; count < MAXIMUM_FILE_COUNT; count++) {

        char filename [13];
        sut.OpenRootDirectory();
        if (sut.NextFile(filename) == -1) {
            break;
        }
        sut.Open(filename, 'd');
    }
    return count;
}

void cog_DoNothing(void * arg) {
    waitcnt(CLKFREQ * 50 + CNT);
}

int help_CountNumberOfFreeCogs(void) {
    const int stacksize = sizeof (_thread_state_t) + sizeof (int) * 10;
    int * cog_stack = (int*) malloc(stacksize);
    int cog_id = cogstart(cog_DoNothing, NULL, cog_stack, stacksize);

    int free_cogs = 0;

    if (cog_id != -1) {
        free_cogs = help_CountNumberOfFreeCogs() + 1;
        cogstop(cog_id);
    }

    free(cog_stack);

    return free_cogs;
}

void setUp(void) {
    mountResult = sut.Mount(kDoPin, kClkPin, kDiPin, kCsPin);
}

void tearDown(void) {
    help_DeleteAllFiles();
    sut.Unmount();
}

// -----------------------------------------------------------------------------
// Mount Operations
// -----------------------------------------------------------------------------

void test_Mount(void) {
    TEST_ASSERT_EQUAL_INT(0, mountResult);
}

void test_MountMultiple(void) {
    TEST_ASSERT_EQUAL_INT(0, sut.Mount(kDoPin, kClkPin, kDiPin, kCsPin));
    TEST_ASSERT_EQUAL_INT(0, sut.Mount(kDoPin, kClkPin, kDiPin, kCsPin));

}

void test_MultipleUnmounts(void) {
    TEST_ASSERT_EQUAL_INT(0, sut.Unmount());
    TEST_ASSERT_EQUAL_INT(0, sut.Unmount());
}

void test_MountNoSd(void) {
    TEST_ASSERT_EQUAL_INT(SecureDigitalCard::kErrorCardNotReset, sut.Mount(kDoPinNoSd, kClkPinNoSd, kDiPinNoSd, kCsPinNoSd));
}

void test_UnmountFreesCog(void) {
    int cogsFreeBefore = help_CountNumberOfFreeCogs();
    sut.Unmount();
    TEST_ASSERT_EQUAL_INT(cogsFreeBefore + 1, help_CountNumberOfFreeCogs());
}

void test_DestructorFreesCog(void) {
    sut.Unmount();
    int cogsFreeBefore = help_CountNumberOfFreeCogs();
    {
        SecureDigitalCard temp;
        temp.Mount(kDoPin, kClkPin, kDiPin, kCsPin);
        TEST_ASSERT_EQUAL_INT(cogsFreeBefore - 1, help_CountNumberOfFreeCogs());
    }
    TEST_ASSERT_EQUAL_INT(cogsFreeBefore, help_CountNumberOfFreeCogs());
}

// -----------------------------------------------------------------------------
// File operations (open, close, etc.)
// -----------------------------------------------------------------------------

void test_OpenNonexistentFileForRead(void) {
    TEST_ASSERT_EQUAL_INT(-1, sut.Open("RANDOM.RND", 'r'));
}

void test_OpenNonexistentFileForWriteThenDeleteFile(void) {
    TEST_ASSERT_EQUAL_INT(0, sut.Open("RANDOM.RND", 'w'));
    TEST_ASSERT_EQUAL_INT(0, sut.Open("RANDOM.RND", 'd')); //Cleanup
}

void test_OpenForDeleteNonexistentFile(void) {
    TEST_ASSERT_EQUAL_INT(-1, sut.Open("RANDOM.RND", 'd'));
}

void test_OpenForAppendNonexistentFile(void) {
    TEST_ASSERT_EQUAL_INT(-1, sut.Open("RANDOM.RND", 'd'));
}

void test_OpenTooLongFilename(void) {
    TEST_ASSERT_EQUAL_INT(0, sut.Open("REALLONGNAME.RND", 'w'));
    TEST_ASSERT_EQUAL_INT(0, sut.Open("REALLONG.RND", 'd'));
}

void test_CloseFileTwice(void) {
    TEST_ASSERT_EQUAL_INT(0, sut.Close());
    TEST_ASSERT_EQUAL_INT(0, sut.Close());
}

void help_TestFilename(const char * filename) {
    char letter = CNT & 0x7F; //Semi random noise...

    sut.Open(filename, 'w');
    sut.Put(letter);
    sut.Open(filename, 'r');
    TEST_ASSERT_EQUAL_INT(letter, sut.Get());
    TEST_ASSERT_EQUAL_INT(-1, sut.Get());
}

void test_OpenSpecialCharacterFilenames(void) {
    help_TestFilename("$%-_@~`!.(){");
    help_TestFilename("}^#& ");
}

void test_OpenFilenameWithExtensionOnly(void) {
    help_TestFilename(".WAT");
}

void test_OpenFilenameWithoutExtension(void) {
    help_TestFilename("Hello");
}

void test_OpenShorterFilenameThan8dot3(void) {
    help_TestFilename("a.a");
}

void test_OpenEmptyFilename(void) {
    help_TestFilename("");
}



//TO DO(SRLM): I don't think I can test to tell if the pins are Tristated. This
//is because each cog has it's own DIRA register, and the results, although OR'd
//together with the other DIRA registers, is not accessible.
//One solution might be to
// 1. Tristate pins
// 2. Set DIRA for SD pin to output
// 3. Set OUTA for SD pin to low
// 4. Set a neighboring pin (with resistor to SD pin) to input
// 5. Read neighboring pin, check that it's low.
// This wouldn't work in the case that the SPI driver is holding the pin low,
// but in the other cases it would work.
// 6. Set SD pin high
// 7. Read neighboring pin, check that it's high.
//void test_CloseReleasePinsToTristate(void)
//{
//	const unsigned int kDoPinMask  = 1 << kDoPin;
//	const unsigned int kClkPinMask = 1 << kClkPin;
//	const unsigned int kDiPinMask  = 1 << kDiPin;
//	const unsigned int kCsPinMask  = 1 << kCsPin;
//	
//	const unsigned int kSdPinMask = kDoPinMask | kClkPinMask | kDiPinMask | kCsPinMask;
//	
//	TEST_ASSERT_EQUAL_INT(0, sut.Open("RANDOM.RND", 'w'));
//	TEST_ASSERT_BITS_HIGH(kSdPinMask, DIRA);
//	sut.Close();
//	TEST_ASSERT_BITS_LOW(kSdPinMask, 0xFFFFFFFF);
//}



// -----------------------------------------------------------------------------
// Writing to and from files
// -----------------------------------------------------------------------------

void test_PutChar(void) {
    sut.Open("RANDOM.RND", 'w');
    TEST_ASSERT_EQUAL_INT(0, sut.Put('a'));
}

void test_GetCharFromExistingFile(void) {
    sut.Open("RANDOM.RND", 'd');
    sut.Open("RANDOM.RND", 'w');
    sut.Put('x');
    sut.Open("RANDOM.RND", 'r');
    TEST_ASSERT_EQUAL_INT('x', sut.Get());
}

void test_GetCharAfterEndOfFile(void) {
    sut.Open("RANDOM.RND", 'd');
    sut.Open("RANDOM.RND", 'w');
    sut.Put('x');
    sut.Open("RANDOM.RND", 'r');
    sut.Get();
    TEST_ASSERT_EQUAL_INT(-1, sut.Get());
}

void test_PutCharAppend(void) {
    int size = sut.Open("APPEND.TXT", 'a');
    TEST_ASSERT_TRUE(size >= 0);
    TEST_ASSERT_EQUAL_INT(0, sut.Put('-'));
    TEST_ASSERT_EQUAL_INT(size + 1, sut.Open("APPEND.TXT", 'r'));
}

//void test_PutCharNoOpenFile(void)
//{
//	//turns out that it doesn't actually check to make sure the file is open...
//	TEST_ASSERT_EQUAL_INT(0, sut.Close());
//	TEST_ASSERT_EQUAL_INT(0, sut.Put('a'));
//}

void test_Put(void) {
    sut.Open("RANDOM.RND", 'w');
    TEST_ASSERT_EQUAL_INT(5, sut.Put("Hello"));
    sut.Open("RANDOM.RND", 'r');
    TEST_ASSERT_EQUAL_INT('H', sut.Get());
    TEST_ASSERT_EQUAL_INT('e', sut.Get());
    TEST_ASSERT_EQUAL_INT('l', sut.Get());
    TEST_ASSERT_EQUAL_INT('l', sut.Get());
    TEST_ASSERT_EQUAL_INT('o', sut.Get());
    TEST_ASSERT_EQUAL_INT(-1, sut.Get());
}

void test_PutSEmptyString(void) {
    sut.Open("RANDOM.RND", 'w');
    TEST_ASSERT_EQUAL_INT(0, sut.Put(""));
}

void test_Get(void) {
    sut.Open("RANDOM.RND", 'w');
    sut.Put("World\0ABC", 6);

    char buffer[6];
    sut.Open("RANDOM.RND", 'r');
    TEST_ASSERT_EQUAL_INT(6, sut.Get(buffer, 6));
    TEST_ASSERT_EQUAL_STRING("World", buffer);
}

void test_GetBufferPastEndOfFile(void) {
    sut.Open("RANDOM.RND", 'w');
    sut.Put("World\0", 6);

    char buffer[10];
    sut.Open("RANDOM.RND", 'r');
    TEST_ASSERT_EQUAL_INT(-1, sut.Get(buffer, 10));
    TEST_ASSERT_EQUAL_STRING("World", buffer);
}

void test_WriteLargeFile(void) {
    sut.Open("RANDOM.RND", 'w');

    const int kAlphabetCount = 2048;

    for (int i = 0; i < kAlphabetCount; i++) {
        for (int letter = 'a'; letter <= 'z'; letter++) {
            TEST_ASSERT_EQUAL_INT(0, sut.Put(letter));
        }
    }

    sut.Open("RANDOM.RND", 'r');
    for (int i = 0; i < kAlphabetCount; i++) {
        for (int letter = 'a'; letter <= 'z'; letter++) {
            TEST_ASSERT_EQUAL_INT(letter, sut.Get());
        }
    }

    TEST_ASSERT_EQUAL_INT(-1, sut.Get());
}


// -----------------------------------------------------------------------------
// Test file system functionality
// -----------------------------------------------------------------------------

void test_GetFilesizeAfterOpenForRead(void) {
    sut.Open("RANDOM.RND", 'w');
    sut.Put("Hello");
    TEST_ASSERT_EQUAL_INT(5, sut.Open("RANDOM.RND", 'r'));
}

void test_GetFilesizeForWrite(void) {
    TEST_ASSERT_EQUAL_INT(0, sut.Open("RANDOM.RND", 'w'));
}

void test_GetFilesizeAfterWrite(void) {
    sut.Open("RANDOM.RND", 'w');
    sut.Put("Hello");
    TEST_ASSERT_EQUAL_INT(5, sut.Open("RANDOM.RND", 'r'));
}

void test_GetFilesizeAfterOpenForAppend(void) {
    sut.Open("RANDOM.RND", 'a');
    sut.Put("Hello");
    sut.Open("RANDOM.RND", 'a');
    sut.Put("World");
    TEST_ASSERT_EQUAL_INT(10, sut.Open("RANDOM.RND", 'r'));

}

void test_SetDate(void) {
    // Fat16 date and time information here:
    // http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html#LastWriteTime

    //Hour = 3, minute = 30, seconds = 58
    //         0bHHHHHMMMMMMSSSSS
    int time = 0b0001101111011101;

    // Year = 2000, month = 1, day = 2
    //         0bYYYYYYYMMMMDDDDD
    int date = 0b0010100000100010;

    int datetime = (date << 16) + time;

    TEST_ASSERT_BITS(0xFFFFFFFF, datetime, sut.SetDate(2000, 1, 2, 3, 30, 58));
}

void test_SeekSmallFile(void) {
    sut.Open("RANDOM.RND", 'w');
    sut.Put("Hello World!");
    sut.Open("RANDOM.RND", 'r');

    for (int i = 0; i < 5; i++)
        sut.Get();

    TEST_ASSERT_EQUAL_INT(0, sut.Seek(2));
    TEST_ASSERT_EQUAL_INT('l', sut.Get());
}

void test_SeekOnWriteAfterOpening(void) {
    sut.Open("RANDOM.RND", 'w');
    TEST_ASSERT_EQUAL_INT(-1, sut.Seek(0));
}

void test_SeekOnWriteAfterWriting(void) {
    sut.Open("RANDOM.RND", 'w');
    sut.Put("Hello World!");
    TEST_ASSERT_EQUAL_INT(-1, sut.Seek(0));
}

void test_SeekOnWriteAndCanStillWriteAfter(void) {
    sut.Open("RANDOM.RND", 'w');
    sut.Put("Hello");
    sut.Seek(0);
    sut.Put("World");
    sut.Open("RANDOM.RND", 'r');
    for (int i = 0; i < 5; i++)
        sut.Get();
    TEST_ASSERT_EQUAL_INT('W', sut.Get());
}

void test_SeekOnRead(void) {
    sut.Open("RANDOM.RND", 'w');
    sut.Put("Hello World");
    sut.Open("RANDOM.RND", 'r');
    sut.Seek(6);
    TEST_ASSERT_EQUAL_INT('W', sut.Get());
}

void test_SeekOnLargeFile(void) {
    //Should be more than 32KB	
    //32KB clusters * 1024 B/Clust / 16 byte test sequence * 1.5 cluster span = 3072
    //Repeat a 16 byte test sequence across 1.5 32KB clusters.

    sut.Open("RANDOM.RND", 'w');

    for (int i = 0; i < 3072; i++)
        for (char testchar = 'a'; testchar <= 'p'; testchar++)
            sut.Put(testchar);

    sut.Open("RANDOM.RND", 'r');

    //Back across cluster boundry
    TEST_ASSERT_EQUAL_INT(0, sut.Seek(0));
    TEST_ASSERT_EQUAL_INT('a', sut.Get());

    //Within 32KB cluster
    TEST_ASSERT_EQUAL_INT(0, sut.Seek(16 * 1024 + 3));
    TEST_ASSERT_EQUAL_INT('d', sut.Get());

    //Across cluster boundry
    TEST_ASSERT_EQUAL_INT(0, sut.Seek(40 * 1024 + 8));
    TEST_ASSERT_EQUAL_INT('i', sut.Get());
}

void test_GetClusterSize(void) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(32768, sut.GetClusterSize(), "SD card should be formatted in 32K clusters.");
}

void test_getNextFileFindsAllFiles(void) {

    const int MAXIMUM_FILE_COUNT = 100;
    int count;
    for (count = 0; count < MAXIMUM_FILE_COUNT; count++) {

        char filename [13];
        sut.OpenRootDirectory();
        if (sut.NextFile(filename) == -1) {
            break;
        }
        sut.Open(filename, 'd');
    }

    TEST_ASSERT_TRUE(count != MAXIMUM_FILE_COUNT);

}

void test_getNextFileFindsCorrectFiles(void) {


    const int FILECOUNT = 3;

    //Note: filenames must be uppercase!
    const char * filenames[FILECOUNT];
    filenames[0] = "NEXTA.TXT";
    filenames[1] = "NEXTB.TXT";
    filenames[2] = "NEXTC.TXT";

    bool filenameFound[FILECOUNT];
    filenameFound[0] = false;
    filenameFound[1] = false;
    filenameFound[2] = false;


    sut.Open(filenames[0], 'w');
    sut.Put('A');

    sut.Open(filenames[1], 'w');
    sut.Put('B');

    sut.Open(filenames[2], 'w');
    sut.Put('C');

    sut.OpenRootDirectory();

    char nextFilename[13];
    while (sut.NextFile(nextFilename) != -1) {
        int i;
        for (i = 0; i < FILECOUNT; i++) {
            if (strcmp(filenames[i], nextFilename) == 0) {
                TEST_ASSERT_FALSE_MESSAGE(filenameFound[i], "Should not already be found.");
                filenameFound[i] = true;
                break;
            }
        }
        TEST_ASSERT_TRUE_MESSAGE(i != FILECOUNT, "Unmatched filename!");
    }

    for (int i = 0; i < FILECOUNT; i++) {
        TEST_ASSERT_TRUE(filenameFound[i]);
    }
}

































