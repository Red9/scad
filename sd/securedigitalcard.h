//TODO(SRLM): Mount_explicit seems to hang when there is no card in the slot.

//TODO(SRLM): I think the functions with buffer in them should have "volatile"
//in the function declaration (since the buffer can be modified by PASM).

//TODO(SRLM): I should add a unit test or something for the "no SD card" condition...
// At this time, it should return -1 (and *not* hang/freeze).


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

@author SRLM (srlm@srlmproductions.com)
@date 2012-12-12
@version 1.0

Version History
	+ 1.0 Ported from FSRW.


*/

#ifndef SRLM_PROPGCC_SECUREDIGITAL_CARD_H__
#define SRLM_PROPGCC_SECUREDIGITAL_CARD_H__

#include <stdint.h>
#include "sdsafespi.h"

class SecureDigitalCard {
public:


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
+ -999: No card detected
*/
int Mount(int Basepin);

/**
Mount a volume with explicit pin numbers. Does not require adjacent pins.


@param DO  The SPI Data Out pin (ouput relative to Propeller).
@param CLK The SPI Clock pin.
@param DI  The SPI Data In pin (input relative to Propeller).
@param CS  The Chip Select pin.
@return see @a Mount(int) for return codes
*/
int Mount(int DO, int CLK, int DI, int CS);

/**
Closes any open files, and unmounts the SD card. Frees a cog.
*/
int Unmount(void);

/**
Close any currently open file, and open a new one with the given file name and
mode.

If the file did not exist, and the mode was not "w" or "a", -1 will be returned.
 Otherwise a negative error code will be returned.

If the file is successfully opened then the current file size in bytes will be
returned.

If the mode is 'd', and the file exists, a 0 will be returned. If the file
doesn't exist, then a -1 will be returned.

If the mode is 'a', and the file exists, a ??? will be returned. If the file
doesn't exist, then a -1 will be returned.

@todo (SRLM): the return code conditions need to be completed.

@param Filename Filename in 8.3 format. The filename will be converted to
                uppercase.
@param Mode can be
+ 'r' (read)
+ 'w' (write)
+ 'a' (append)
+ 'd' (delete)
@return See discussion above.

*/
int Open(const char * Filename, const char Mode);
  
/**
Flush and close the currently open file if any.  Also reset the pointers to
valid values. Also, releases the SD pins to tristate.

@return If there is no error, 0 will be returned.
*/
int Close(void);

/** Read bytes into the buffer from currently open file.

@todo(SRLM): The return value in this function is pretty useless. You could know
that you have reached the end when return != count, but if it returns -1 then
you have no way of knowing how many bytes are in the buffer.

If you try to read past the end of a file, then the remaining bytes will be put
into the buffer, and a -1 will be returned (Note: the number of bytes read is
NOT returned in this case).

@param Ubuf The buffer to store the data. The buffer may be as large as you want.
@param Count the number of bytes to read.
@return  Returns the number of bytes successfully read, or a negative number if
         there is an error. Returns -1 if the end of the file is reached. (Note:
         the number of bytes read is NOT returned in this case).

*/
int Get(char * Ubuf, int Count);
  
/** Read and return a single character from the currently open file.

@return -1 if the end of the file is reached. A negative number for an error.
        Otherwise, returns the character in the lower byte.
*/
int Get(void);
  
//TODO(SRLM): Write a function that gets a string:
//  int   Get(char * Ubuf, char EndOfStringChar);


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

@warning paramater limits are not checked. Ie, a month of 13 will not generate
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

@param position The position to seek to, relative to the beginning of the file.
@returns 0 on success, a negative number on failure (such as seeking during
         write)
*/
int Seek(int position);

/*
Close the currently open file, and set up the read buffer for
   calls to nextfile().
   Returns negative on error (only occurs if there is an error closing the file).
 TODO(SRLM): UNTESTED
*/
//  int	Opendir(void);
  
/*
Find the next file in the root directory and extract its
 (8.3) name into fbuf.  Fbuf must be sized to hold at least
 13 characters (8 + 1 + 3 + 1).  If there is no next file,
 -1 will be returned.  If there is, 0 will be returned.
 
 TODO(SRLM): UNTESTED
*/
//  int	Nextfile(char * Fbuf);
  
  
/** Get the FAT cluster size.
@returns the size of the cluster, in bytes.
*/
int GetClusterSize(void);
  
/** Get the current FAT cluster count.

@return the cluster count.
*/
//What does this mean? I (SRLM) don't know. I also don't know how to test it, so it is not tested.
int GetClusterCount(void);
  
private:

  static const int Sectorsize = 512;
  static const int Sectorshift = 9;
  static const int Dirsize = 32;
  static const int Dirshift = 5;
  SdSafeSPI Sdspi;

  int32_t	Fclust;
  int32_t	Filesize;
  int32_t	Floc;
  int32_t	Frem;
  int32_t	Bufat;
  int32_t	Bufend;
  int32_t	Direntry;
  int32_t	Writelink;
  int32_t	Fatptr;
  int32_t	Firstcluster;
  int32_t	Errno;
  int32_t	Filesystem;
  int32_t	Rootdir;
  int32_t	Rootdirend;
  int32_t	Dataregion;
  int32_t	Clustershift;
  int32_t	Clustersize;
  int32_t	Fat1;
  int32_t	Totclusters;
  int32_t	Sectorsperfat;
  int32_t	Endofchain;
  int32_t	Pdate;
  int32_t	Lastread;
  int32_t	Dirty;
  
/*
  Buffering:  two sector buffers.  These two buffers must be longword
  aligned!  To ensure this, make sure they are the first byte variables
  defined in this object.
*/
  uint8_t	volatile Buf[512];
  uint8_t	volatile Buf2[512];
  uint8_t	Padname[11];
  
/*
On metadata writes, if we are updating the FAT region, also update
 the second FAT region.
 
 Returns negative error or 0 for success
*/
  int	Writeblock2(int N, char * B);
  int32_t	Flushifdirty(void);
  int32_t	Readblockc(int32_t N);
  
  
  
/*
Read a byte-reversed word from a (possibly odd) address.
*/
  int	Brword(char * b);
  
/*
Read a byte-reversed long from a (possibly odd) address.
*/
  int	Brlong(char * b);

/*
Read a cluster entry.
*/
  int	Brclust(char * B);
  
/*
Write a byte-reversed word to a (possibly odd) address, and
  mark the metadata buffer as dirty.
*/
  void	Brwword(char * w, int v);
  
/*
Write a byte-reversed long to a (possibly odd) address, and
 mark the metadata buffer as dirty.
*/
  void	Brwlong(char * w, int v);
  
/*
 Write a cluster entry.
*/
  void	Brwclust(char * w, int v);
  int	Getfstype(void);
  
/*
Read a byte address from the disk through the metadata buffer and
 return a pointer to that location.
*/
  char *	Readbytec(int32_t Byteloc);
  
/*
Read a fat location and return a pointer to the location of that
 entry.
*/
  char *	Readfat(int32_t Clust);
  
/*
Follow the fat chain and update the writelink.
*/
  int32_t	Followchain(void);
  
  int32_t	Nextcluster(void);
  
/*
Free an entire cluster chain.  Used by remove and by overwrite.
  Assumes the pointer has already been cleared/set to end of chain.
*/
  int32_t	Freeclusters(int32_t Clust);

/*
This is just a pass-through function to allow the block layer
 to tristate the I/O pins to the card.
*/
  void	Release(void);
  
  int32_t	Datablock(void);
  int32_t	Uc(int32_t C);
  int32_t	Pflushbuf(int32_t Rcnt, int32_t Metadata);
  int32_t	Pflush(void);
  int32_t	Pfillbuf(void);
};

#endif // SRLM_PROPGCC_SECUREDIGITAL_CARD_H__
