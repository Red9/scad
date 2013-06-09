/** FAT32 SD card interface.

This class is based on the Spin version of FSRW 2.6 by Rokicki and Lonesock.

This object provides FAT16/32 file read/write access on a block device. Only one
file open at a time. Open modes are 'r' (read), 'a' (append), 'w' (write), and
'd' (delete). Only the root directory is supported. No long filenames are
supported. This object also supports traversing the root directory.

This object requires pullup resistors on the four SD card I/O lines. The pullup
resistors should be approximately 10kOhms each.

The SPI DO, DI, and CLK lines can be shared, as long as the @a
SdSafeSPI::Release() command is called after doing SD card activities and before
doing other SPI activities. Note: this has not been tested at all.

Cluster size: If it's not 32768, it should be (32K clusters). To format a drive
with FAT32 with 32K clusters, use the following command under linux:

    sudo fdisk -l
    sudo mkdosfs /dev/sdX -s 64 -F 32 -I

Note: the mkdosfs command will format the entire disk, and erase all information
on it.
 * 
 * @warning Untested with multiple instances!!!

@author SRLM (srlm@srlmproductions.com)

 * Possible improvements:
 * Write a function that gets a string:
     int   Get(char * Ubuf, char EndOfStringChar);
 * 
 * 
 */

#ifndef SRLM_PROPGCC_SECUREDIGITAL_CARD_H__
#define SRLM_PROPGCC_SECUREDIGITAL_CARD_H__

#include <propeller.h>
#include <stdio.h>

#include <stdint.h>
#include "sdsafespi.h"

class SecureDigitalCard {
public:
    ~SecureDigitalCard();

    /**
    Mounts a volume. Closes any open files (if this is a remount). Requires a cog
    for the SD SPI driver.

    @param basepin pins must be in the following order, from basepin up:
    -# Basepin +0: DO
    -# Basepin +1: CLK
    -# Basepin +2: DI
    -# Basepin +3: CS

    @return One of the following values:
    + 0:    Success.
    + -1:   card not reset (usually means no card detected)
    + -2:   3.3v not supported (?)
    + -3:   OCR failed (?)
    + -4:   block not long aligned (from @a SdSafeSPI)
    ...
    + -20:  not a FAT16 or FAT32 volume
    + -21:  bad bytes per sector
    + -22:  bad sectors per cluster
    + -23:  not two FATs (what does this mean?)
    + -24:  bad FAT signature
    ...
    + -512: Buf and Buf2 not longword aligned (was the class data members modified?)
     */
    void Mount(int Basepin);

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
    // These errors are negated since the are thrown as negative in ASM section.
    static const int kErrorAsmNoReadToken = -SdSafeSPI::kErrorAsmNoReadToken;
    static const int kErrorAsmBlockNotWritten = -SdSafeSPI::kErrorAsmBlockNotWritten;
    // NOTE: errors -128 to -255 are reserved for reporting R1 response errors (SRLM ???)
    static const int kErrorSpiEngineNotRunning = SdSafeSPI::kErrorSpiEngineNotRunning;
    static const int kErrorCardBusyTimeout = SdSafeSPI::kErrorCardBusyTimeout;
    
    
    /**
    Mount a volume with explicit pin numbers. Does not require adjacent pins.

     * May set an error code (you should check!)

    @param DO  The SPI Data Out pin (ouput relative to Propeller).
    @param CLK The SPI Clock pin.
    @param DI  The SPI Data In pin (input relative to Propeller).
    @param CS  The Chip Select pin.
     */
    void Mount(int DO, int CLK, int DI, int CS);

    /**
    Closes any open files, and unmounts the SD card. Frees a cog.
     */
    void Unmount(void);

    /**
    Close any currently open file, and open a new one with the given file name and
    mode.
     * 
     
     * Deleting a file will not result in kErrorFileNotFound, even if the file is not found

    If the file did not exist, and the mode was not "w" or "a" , -1 will be returned.
     Otherwise a negative error code will be returned.

    If the file is successfully opened then the current file size in bytes will be
    returned.

    If the mode is 'd', and no error occurs, a 0 will be returned. File not found is not an error.

    If the mode is 'a' and no error occurs a 0 will be returned. File not found is not an error.

     Return error codes include (found below):
     * + SDSPI error codes
     * 

    @param Filename Filename in 8.3 format. The filename will be converted to
                    uppercase. Valid characters include A through Z, digits 0 through 9, space, and '$', '%', '-', '_', '@', '~', '`', '!', '(', ')', '{', '}', '^', '#', '&' and a single '.'. Filenames can be shorter than 8.3. The filename is not correct, and the behavior for invalid filenames is undefined.
    @param Mode can be
    + 'r' (read)
    + 'w' (write)
    + 'a' (append)
    + 'd' (delete)
    @return See discussion above.

     */
    int Open(const char * Filename, const char Mode);
    


    /** Flush and close the currently open file if any.  Also reset the pointers to valid values. Also, releases the SD pins to tristate.
     * 
     * @return If there is no error, 0 will be returned.
     */
    int Close(void);

    /** Read bytes into the buffer from currently open file.

    If you try to read past the end of a file, then the remaining bytes will be put
    into the buffer, and a -1 will be returned (Note: the number of bytes read is
    NOT returned in this case).

    @param read_buffer The buffer to store the data. The buffer may be as large as you want.
    @param bytes_to_read_count The number of bytes to read.
    @return  Returns the number of bytes successfully read, or a negative number if
             there is an error. If the end of file has been reached, then this may be less than bytes_to_read_count.

     */
    int Get(char * read_buffer, int bytes_to_read_count);

    /** Read and return a single character from the currently open file.

    @return -1 if the end of the file is reached. A negative number for an error.
            Otherwise, returns the character in the lower byte.
     */
    int Get(void);

   


    /** Write bytes from buffer into the currently open file.

    @warning does not check to make sure that a file is actually open...

    @param The buffer to pull the data from. The buffer may be as large as you want.
    @param Count the number of bytes to write.
    @return the number of bytes successfully written, or a negative number if there
            was an error.
     */
    int Put(const char * Ubuf, int Count);

    /** Write a null-terminated string to the file.

    @warning does not check to make sure that a file is actually open...
    @param B The null-terminated string to write. No size limitations.
    @return the number of bytes successfully written, or a negative number if there
            is an error.
     */
    int Put(const char * B);

    /** Write a single character into the file open for write.

    @warning does not check to make sure that a file is actually open...
    @param    C The character to write.
    @return  0 if successful, a negative number if an error occurred.
     */
    int Put(const char C);

    /** Set the current date and time for file creation and last modified.

    This date and time will remain constant until the next time SetDate() is called.

    @warning parameter limits are not checked. Ie, a month of 13 will not generate
    an error.

    @param Year   The year   (range 1980 - 2106, all 4 digits!)
    @param Month  The month  (range 1-12)
    @param Day    The day    (range 1-31)
    @param Hour   The hour   (range 0-23)
    @param Minute The minute (range 0-59)
    @param Second The second (range 0-59)
    @return the FAT16 date format (you can safely ignore the return in all cases,
            unless you want to test the correctness of the function).
     */
    int SetDate(int Year, int Month, int Day, int Hour, int Minute, int Second);

    /** Change the read pointer to a different position in the file.

    Seek() works only in 'r' (read) mode.

    @param position The position to seek to, relative to the beginning of the file. Units?
    @returns 0 on success, a negative number on failure (such as seeking during
             write)
     */
    int Seek(int position);

    /*
    Close the currently open file, and set up the read buffer for
       calls to nextfile().
     
     */
    void OpenRootDirectory(void);

    /*
    Find the next file in the root directory and extract its
     (8.3) name into fbuf.  Fbuf must be sized to hold at least
     13 characters (8 + 1 + 3 + 1).  If there is no next file,
     kErrorFileNotFound will be returned.  If there is, 0 will be returned.
 
     If an error occurs a negative number is returned.
     */
    int NextFile(char * filename);
    
    


    /** Get the FAT cluster size.
    @returns the size of the cluster, in bytes.
     */
    int GetClusterSize(void);

    /** Get the current FAT cluster count.

    @return the cluster count.
     */
    //What does this mean? I (SRLM) don't know. I also don't know how to test it, so it is not tested.
    int GetClusterCount(void);

    
    
    
    
    /** If there was an error in the SD routines then this function will return
     * an error code.
     * 
     * @return The error code.
     */
    bool HasError(void);

    /** Resets the error flag to kNoError.
     */
    void ClearError(void);
    
    /**
     */
    int GetError(void);
    
    
    
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
