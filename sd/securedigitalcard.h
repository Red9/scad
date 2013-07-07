/** FAT16/32 SD card interface.
 * 
 * This class is based on the Spin version of FSRW 2.6 by Rokicki and Lonesock.
 * 
 * This object provides FAT16/32 file read/write access on a block device. Only 
 * one file can be open at a time. Open modes are 'r' (read), 'a' (append), 
 * 'w' (write), and 'd' (delete). Only the root directory is supported. No long 
 * filenames are supported. This object also supports traversing the root 
 * directory.
 * 
 * This object requires pullup resistors on the four SD card I/O lines. The 
 * pullup resistors should be approximately 10kOhms each.
 * 
 * The SPI DO, DI, and CLK lines can be shared with other SPI devices, as long 
 * as the @a Release() command is called after doing SD card activities and 
 * before doing other SPI activities. Note: this has not been tested at all.
 * 
 * Cluster size: If it's not 32768, it should be (32K clusters). To format a 
 * drive with FAT32 with 32K clusters, use the following command under linux:
 *     sudo fdisk -l
 *     sudo mkdosfs /dev/sdX -s 64 -F 32 -I
 * 
 * To check the filesystem under linux:
 *      sudo dosfsck -v /dev/sdX
 * 
 * Note: the mkdosfs command will format the entire disk, and erase all 
 * information on it.
 * 
 * If an exceptional error occurs then the @a HasError() function will return 
 * true. To see what went wrong, query the @a GetError() function. After you fix
 * the error, clear it with @a ClearError(); Every function may set the error,
 * although you're generally OK with just checking after @a Mount() and 
 * @a Open()
 * 
 * 
 * @todo(SRLM) :add the get Filesize method.
 * 
 * @warning Untested with multiple instances!!!
 * 
 * @warning The various @a Get() and @a Put() methods don't check to make sure
 * that a file is open. It's up to your code to make sure that a file is 
 * successfully opened before you need to use it.
 * 
 * @author SRLM (srlm@srlmproductions.com)
 * 
 * Possible improvements:
 * Write a function that gets a string:
 *       int   Get(char * Ubuf, char EndOfStringChar); 
 */

#ifndef SRLM_PROPGCC_SECUREDIGITAL_CARD_H__
#define SRLM_PROPGCC_SECUREDIGITAL_CARD_H__

#include <propeller.h>
#include "sdsafespi.h"

class SecureDigitalCard {
public:
    static const int kNoError = SdSafeSPI::kNoError;

    // Mount Errors
    static const int kErrorNotFatVolume = -20;
    static const int kErrorBadBytesPerSector = -21;
    static const int kErrorBadSectorsPerCluster = -22;
    static const int kErrorNotTwoFats = -23;
    static const int kErrorBadFatSignature = -24;
    static const int kErrorBufNotLongwordAligned = -512;

    //Open Errors
    static const int kErrorFileNotFound = -1; //TODO(SRLM): For some reason, if I change the value of this error code then things slow way down. Why???
    static const int kErrorNoEmptyDirectoryEntry = -2;
    static const int kErrorBadArgument = -3;
    static const int kErrorNoWritePermission = -6;
    static const int kErrorEofWhileFollowingChain = -7;
    static const int kErrorBadClusterValue = -9;
    static const int kErrorBadClusterNumber = -26;
    static const int kErrorFileNotOpenForWriting = -27;

    // SdSafeSPI Errors
    static const int kErrorCardNotReset = SdSafeSPI::kErrorCardNotReset;
    static const int kError3v3NotSupported = SdSafeSPI::kError3v3NotSupported;
    static const int kErrorOcrFailed = SdSafeSPI::kErrorOcrFailed;
    static const int kErrorBlockNotLongAligned = SdSafeSPI::kErrorBlockNotLongAligned;
    // These errors are negated since they are thrown as negative in ASM section.
    static const int kErrorAsmNoReadToken = -SdSafeSPI::kErrorAsmNoReadToken;
    static const int kErrorAsmBlockNotWritten = -SdSafeSPI::kErrorAsmBlockNotWritten;
    // NOTE: errors -128 to -255 are reserved for reporting R1 response errors (SRLM ???)
    static const int kErrorSpiEngineNotRunning = SdSafeSPI::kErrorSpiEngineNotRunning;
    static const int kErrorCardBusyTimeout = SdSafeSPI::kErrorCardBusyTimeout;

    /** Stops the SPI driver cog.     
     */
    ~SecureDigitalCard();

    /** Mounts a volume. Closes any open files (if this is a remount). Requires 
     * a cog for the SD SPI driver.
     * 
     * @param basepin pins must be in the following order, from basepin up:
     * -# Basepin +0: DO
     * -# Basepin +1: CLK
     * -# Basepin +2: DI
     * -# Basepin +3: CS
     */
    void Mount(int Basepin);

    /** Mount a volume with explicit pin numbers. Does not require adjacent pins.
     * 
     * @param DO  The SPI Data Out pin (ouput relative to Propeller).
     * @param CLK The SPI Clock pin.
     * @param DI  The SPI Data In pin (input relative to Propeller).
     * @param CS  The Chip Select pin.
     */
    void Mount(int DO, int CLK, int DI, int CS);

    /** Closes any open files, and unmounts the SD card. Frees a cog.
     */
    void Unmount(void);

    /** Close any currently open file, and open a new one with the given file 
     * name and mode.
     * 
     * The filename should be in 8.3 format (up to eight characters, a period,
     * and up to three characters.  The filename will be converted to uppercase. 
     * Valid characters include A through Z, digits 0 through 9, space, and '$', 
     * '%', '-', '_', '@', '~', '`', '!', '(', ')', '{', '}', '^', '#', '&' and 
     * a single '.'. Filenames can be shorter than 8.3. The behavior for invalid 
     * filenames is undefined.
     * 
     * Modes:
     * - 'd' Delete a file.  Deleting a file will not result in 
     * kErrorFileNotFound, even if nothing was deleted.
     * - 'a' Append to a file. If the file exists then calls to @a Put() will 
     * add the bytes to the end of the file. Otherwise, the file is created.
     * - 'w' Write to a file. If the file exists, it will be replaced.
     * - 'r' Read from a file. If the file does not exist, then an error is set.
     * 
     * @param Filename Filename in 8.3 format.
     * @param Mode can be
     * + 'r' (read)
     * + 'w' (write)
     * + 'a' (append)
     * + 'd' (delete)
     */
    void Open(const char * Filename, const char Mode);

    /** Flush and close the currently open file if any.  
     * 
     * Clears any errors.
     * 
     * Also reset the pointers 
     * to valid values. Also, releases the SD pins to tristate.
     */
    void Close(void);

    /** Read and return a single character from the currently open file.
     * 
     * @return -1 if the end of the file is reached. Otherwise, returns the 
     * character in the lower byte.
     */
    int Get(void);

    /** Read bytes into a buffer from currently open file.
     * 
     * @param read_buffer The buffer to store the data. The buffer may be as large as you want.
     * @param bytes_to_read_count The number of bytes to read.
     * @return  Returns the number of bytes successfully read, or a negative 
     * number if there is an error. If the end of file has been reached, then 
     * this may be less than bytes_to_read_count.
     */
    int Get(char * read_buffer, int bytes_to_read_count);


    /** Write a single character to the file.
     * 
     * @param    C The character to write.
     * @return  0 if successful, a negative number if an error occurred.
     */
    int Put(const char C);

    /** Write a null-terminated string to the file. 
     * 
     * @param B The null-terminated string to write. No size limitations. Does 
     * not write the null terminator.
     * @return the number of bytes successfully written, or a negative number 
     * if there is an error.
     */
    int Put(const char * B);

    /** Write bytes from buffer into the currently open file.
     * 
     * @param The buffer to pull the data from. The buffer may be as large as 
     * you want.
     * @param Count the number of bytes to write.
     * @return the number of bytes successfully written, or a negative number 
     * if there was an error.
     */
    int Put(const char * Ubuf, int Count);

    /** Set up for a directory file listing.
     * 
     * Close the currently open file, and set up the read buffer for calls to 
     * @a nextfile().
     */
    void OpenRootDirectory(void);

    /** Find the next file in the root directory and extract its (8.3) name into 
     * filename.  The buffer must be sized to hold at least 13 characters 
     * (8 + 1 + 3 + 1).
     * 
     * @param filename The extracted filename
     * @return true if there is a valid filename, false otherwise.
     */
    bool NextFile(char * filename);

    /** Change the read pointer to a different position in the file.
     * 
     * Seek() works only in 'r' (read) mode.
     * 
     * @param position The position to seek to, relative to the beginning of the file. Units?
     * 
     * @return 0 on success, a negative number on failure (such as seeking 
     * during write). Failures may include seeking outside the file size.
     */
    int Seek(int position);

    /** Set the current date and time for file creation and last modified.
     * 
     * This date and time will remain constant until the next time SetDate() is 
     * called.
     * 
     * @warning parameter limits are not checked. Ie, a month of 13 will not 
     * generate an error.
     * 
     * @param Year   The year   (range 1980 - 2106, all 4 digits!)
     * @param Month  The month  (range 1-12)
     * @param Day    The day    (range 1-31)
     * @param Hour   The hour   (range 0-23)
     * @param Minute The minute (range 0-59)
     * @param Second The second (range 0-59)
     * @return the FAT16 date format (you can safely ignore the return in all 
     * cases, unless you want to test the correctness of the function).
     */
    int SetDate(int Year, int Month, int Day, int Hour, int Minute, int Second);

    /** If there was an error in the SD routines then this function will return
     * an error code.
     * 
     * @return The error code.
     */
    bool HasError(void);

    /** Resets the error flag to kNoError.
     */
    void ClearError(void);

    /** Get the error code.
     * 
     * @return The error code.
     */
    int GetError(void);

    /** Get the FAT cluster size.
     * @return the size of the cluster, in bytes.
     */
    int GetClusterSize(void);

    /** Get the current FAT cluster count.
     * 
     * What does this mean? I (SRLM) don't know. I also don't know how to test it, so it is not tested.
     * 
     * @return the cluster count.
     */
    int GetClusterCount(void);
    
    /**
     * 
     * @return 
     */
    int GetFilesize(void);

private:

    // Note: these filesystem numbers should not be changed!
    static const int kFileSystemUnknown = 0;
    static const int kFileSystemFAT16 = 1;
    static const int kFileSystemFAT32 = 2;


    static const int Sectorsize = 512;
    static const int Sectorshift = 9;
    static const int Dirsize = 32;
    static const int Dirshift = 5;
    SdSafeSPI Sdspi;

    int current_cluster_;
    int total_filesize_;
    int seek_position_;
    int remaining_cluster_bytes_;
    int current_buffer_location_;
    int bufend; // The last valid character (read) or free position (write)
    int directory_entry_position_;
    int cluster_write_offset_;
    int last_fat_entry_;
    int first_cluster_of_file_;
    //int error_code_;
    int filesystem;
    int rootdir;
    int rootdirend;
    int dataregion;
    int clustershift;
    int clustersize;
    int Fat1;
    int Totclusters;
    int Sectorsperfat;
    int Endofchain;
    int Pdate;
    int Lastread;
    int Dirty;


    int error;

    /*
      Buffering:  two sector buffers.  These two buffers must be longword
      aligned!  To ensure this, make sure they are the first byte variables
      defined in this object.
     * 
     * These buffers don't seem to need to be volatile (all unit tests pass
     * whether they are or not), but for some reason the code seems to run 4%
     * faster if they are declared volatile. So, here they are.
     */
    char Buf[512];
    char Buf2[512];
    char Padname[11];

    /** In case of Bad Things(TM) happening, exit as gracefully as possible.
     * 
     * @param Abort_code passed through to return.
     */
    void SetErrorCode(int abort_code);


    /*
    On metadata writes, if we are updating the FAT region, also update
     the second FAT region.
     */
    void Writeblock2(int N, char * B);

    /* If the metadata block is dirty, write it out.
     */
    void Flushifdirty(void);

    /* Read a block into the metadata buffer, if that block is not already
'   there.
     */
    void Readblockc(int N);



    /*
    Read a byte-reversed word from a (possibly odd) address.
     */
    int Brword(char * b);

    /*
    Read a byte-reversed long from a (possibly odd) address.
     */
    int Brlong(char * b);

    /*
    Read a cluster entry.
     */
    int Brclust(char * B);

    /*
    Write a byte-reversed word to a (possibly odd) address, and
      mark the metadata buffer as dirty.
     */
    void Brwword(char * w, int v);

    /*
    Write a byte-reversed long to a (possibly odd) address, and
     mark the metadata buffer as dirty.
     */
    void Brwlong(char * w, int v);

    /*
     Write a cluster entry.
     */
    void Brwclust(char * w, int v);
    int Getfstype(void);

    /*
    Read a byte address from the disk through the metadata buffer and
     return a pointer to that location.
     * @todo(SRLM): Make sure all calls have a RET_IF_ERROR
     */
    char * Readbytec(int Byteloc);

    /*
    Read a fat location and return a pointer to the location of that
     entry.
     */
    char * Readfat(int Clust);

    /*
    Follow the fat chain and update the writelink.
     */
    int Followchain(void);

    /*
     Read the next cluster and return it.  Set up writelink to point to the cluster we just read, for later updating.  If the cluster number is bad, return a negative number.
     */

    int Nextcluster(void);

    /*
    Free an entire cluster chain.  Used by remove and by overwrite.
      Assumes the pointer has already been cleared/set to end of chain.
     */
    void Freeclusters(int Clust);

    /*
    This is just a pass-through function to allow the block layer
     to tristate the I/O pins to the card.
     */
    void Release(void);

    /* Calculate the block address of the current data location.
     */
    int Datablock(void);

    /*
    Compute the upper case version of a character.
     * */
    char ConvertToUppercase(char C);


    /*
     Flush the current buffer, if we are open for write.  This may
'   allocate a new cluster if needed.  If metadata is true, the
'   metadata is written through to disk including any FAT cluster
'   allocations and also the file size in the directory entry.
     */
    int Pflushbuf(int Rcnt, int Metadata);



    /*
     * Call flush with the current data buffer location, and the flush
'   metadata flag set.
     * 
     */
    int Pflush(void);
    /*
    Get some data into an empty buffer.  If no more data is available,
'   return -1.  Otherwise return the number of bytes read into the
'   buffer.
     * */
    int Pfillbuf(void);


};

#endif // SRLM_PROPGCC_SECUREDIGITAL_CARD_H__
