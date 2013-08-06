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


#define RET_IF_ERROR_NULL if(HasError()){return NULL;}
#define RET_IF_ERROR if(HasError()){return;}
#define THROW_NULL(value) {SetErrorCode((value)); return NULL;}
//#define THROW_FALSE(value) {SetErrorCode((value)); return false;}
#define THROW(value) {SetErrorCode((value)); return;}

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
    ~SecureDigitalCard() {
        Unmount();
    }

    /** Mounts a volume. Closes any open files (if this is a remount). Requires 
     * a cog for the SD SPI driver.
     * 
     * @param basepin pins must be in the following order, from basepin up:
     * -# Basepin +0: DO
     * -# Basepin +1: CLK
     * -# Basepin +2: DI
     * -# Basepin +3: CS
     */
    void Mount(int Basepin) {
        Mount(Basepin, (Basepin + 1), (Basepin + 2), (Basepin + 3));
    }

    /** Mount a volume with explicit pin numbers. Does not require adjacent pins.
     * 
     * @param DO  The SPI Data Out pin (ouput relative to Propeller).
     * @param CLK The SPI Clock pin.
     * @param DI  The SPI Data In pin (input relative to Propeller).
     * @param CS  The Chip Select pin.
     */
    void Mount(int Do, int Clk, int Di, int Cs) {
        int Start, Sectorspercluster, Reserved, Rootentries, Sectors;
        if (Pdate == 0) {
            SetDate(2010, 1, 1, 0, 0, 0);
        }

        //SRLM Addition: check to make sure that Buf and Buf2 are longword aligned.
        //Theoretically, this should have no runtime cost, but it looks like in CMM
        //and -Os it takes 16 bytes. It can be commented out if you're sure that
        //Buf and Buf2 are longword aligned.
        if ((((int) Buf) & 0b11) != 0)
            THROW(kErrorBufNotLongwordAligned);
        if ((((int) Buf2) & 0b11) != 0)
            THROW(kErrorBufNotLongwordAligned);

        Unmount();
        RET_IF_ERROR;

        Sdspi.Start(Do, Clk, Di, Cs);
        RET_IF_ERROR;

        Lastread = (-1);
        Dirty = 0;
        Sdspi.ReadBlock(0, (char *) (&Buf));
        RET_IF_ERROR;

        if (Getfstype() != kFileSystemUnknown) {
            Start = 0;
        } else {
            Start = Brlong(((char *) (&Buf) + 454));
            Sdspi.ReadBlock(Start, (char *) (&Buf));
            RET_IF_ERROR;
        }
        filesystem = Getfstype();
        if (filesystem == kFileSystemUnknown) {
            THROW(kErrorNotFatVolume);
        }
        if (Brword(((char *) (&Buf) + 11)) != Sectorsize) {
            THROW(kErrorBadBytesPerSector);
        }
        Sectorspercluster = Buf[13];
        if (Sectorspercluster & (Sectorspercluster - 1)) {
            THROW(kErrorBadSectorsPerCluster);
        }
        clustershift = 0;
        while (Sectorspercluster > 1) {
            (clustershift++);
            Sectorspercluster = (Shr__(Sectorspercluster, 1));
        }
        Sectorspercluster = (1 << clustershift);
        clustersize = (Sectorsize << clustershift);
        Reserved = Brword(((char *) (&Buf) + 14));
        if (Buf[16] != 2) {
            THROW(kErrorNotTwoFats);
        }
        Sectors = Brword(((char *) (&Buf) + 19));
        if (Sectors == 0) {
            Sectors = Brlong(((char *) (&Buf) + 32));
        }
        Fat1 = (Start + Reserved);
        if (filesystem == kFileSystemFAT32) {
            Rootentries = (16 << clustershift);
            Sectorsperfat = Brlong(((char *) (&Buf) + 36));
            dataregion = ((Fat1 + (2 * Sectorsperfat)) - (2 * Sectorspercluster));
            rootdir = ((dataregion + (Brword(((char *) (&Buf) + 44)) << clustershift)) << Sectorshift);
            rootdirend = (rootdir + (Rootentries << Dirshift));
            Endofchain = 268435440;
        } else {
            Rootentries = Brword(((char *) (&Buf) + 17));
            Sectorsperfat = Brword(((char *) (&Buf) + 22));
            rootdir = ((Fat1 + (2 * Sectorsperfat)) << Sectorshift);
            rootdirend = (rootdir + (Rootentries << Dirshift));
            dataregion = ((1 + (Shr__((rootdirend - 1), Sectorshift))) - (2 * Sectorspercluster));
            Endofchain = 65520;
        }
        if (Brword(((char *) (&Buf) + 510)) != 43605) {
            THROW(kErrorBadFatSignature);
        }
        Totclusters = (Shr__(((Sectors - dataregion) + Start), clustershift));
    }

    /** Closes any open files, and unmounts the SD card. Frees a cog.
     */
    void Unmount(void) {
        Close();
        Sdspi.Stop();
    }

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
    void Open(const char * filename, const char Mode) {
        char * S; // = Filename;


        Close();
        RET_IF_ERROR;


        int I = 0;
        while (((I < 8) && ((filename)[0])) && ((filename)[0] != '.')) {
            Padname[(I++)] = ConvertToUppercase((filename++)[0]);
        }
        while (I < 8) {
            Padname[(I++)] = ' ';
        }
        while (((filename)[0]) && ((filename)[0] != '.')) {
            (filename++);
        }
        if ((filename)[0] == '.') {
            (filename++);
        }
        while ((I < 11) && ((filename)[0])) {
            Padname[(I++)] = ConvertToUppercase((filename++)[0]);
        }
        while (I < 11) {
            Padname[(I++)] = ' ';
        }
        int Sentinel = 0;
        int Freeentry = 0;
        {
            int _limit__0027 = (rootdirend - Dirsize);
            int _step__0028 = Dirsize;
            int Dirptr = rootdir;
            if (Dirptr >= _limit__0027) _step__0028 = -_step__0028;
            do {
                S = Readbytec(Dirptr);
                RET_IF_ERROR;

                if ((Freeentry == 0) && (((S)[0] == 0) || ((S)[0] == 0xe5))) {
                    Freeentry = Dirptr;
                }
                if ((S)[0] == 0) {
                    Sentinel = Dirptr;
                    break;
                }
                I = 0;
                do {
                    if (Padname[I] != (S)[I]) {
                        break;
                    }
                    I = (I + 1);
                } while (I <= 10);
                if ((I == 11) && (0 == ((S)[0x0b] & 0x18))) { // this always returns
                    current_cluster_ = Brword((S + 0x1a));
                    if (filesystem == 2) {
                        current_cluster_ = (current_cluster_ + (Brword((S + 0x14)) << 16));
                    }
                    first_cluster_of_file_ = current_cluster_;
                    total_filesize_ = Brlong((S + 0x1c));

                    //Mode is Read
                    if (Mode == 'r') {
                        remaining_cluster_bytes_ = (Min__(clustersize, total_filesize_));
                        return;
                    }
                    if ((S)[11] & 0xd9) {
                        THROW(kErrorNoWritePermission);
                    }

                    //Mode is Delete
                    if (Mode == 'd') {
                        Brwword(S, 0xe5);
                        if (current_cluster_) {
                            Freeclusters(current_cluster_);
                            RET_IF_ERROR;
                        }
                        Flushifdirty();
                        RET_IF_ERROR;

                        return;
                    }

                    //Mode is Write
                    if (Mode == 'w') {
                        Brwword((S + 0x1a), 0);
                        Brwword((S + 0x14), 0);
                        Brwlong((S + 0x1c), 0);
                        cluster_write_offset_ = 0;
                        directory_entry_position_ = Dirptr;
                        if (current_cluster_) {
                            Freeclusters(current_cluster_);
                            RET_IF_ERROR;

                        }
                        bufend = Sectorsize;
                        current_cluster_ = 0;
                        total_filesize_ = 0;
                        remaining_cluster_bytes_ = 0;
                        return;
                    }//Mode is Append
                    else {
                        if (Mode == 'a') {
                            // this code will eventually be moved to seek
                            remaining_cluster_bytes_ = total_filesize_;
                            Freeentry = clustersize;
                            if (current_cluster_ >= Endofchain) {
                                current_cluster_ = 0;
                            }
                            while (remaining_cluster_bytes_ > Freeentry) {
                                if (current_cluster_ < 2) {
                                    THROW(kErrorEofWhileFollowingChain);
                                }
                                current_cluster_ = Nextcluster();
                                RET_IF_ERROR;

                                remaining_cluster_bytes_ = (remaining_cluster_bytes_ - Freeentry);
                            }
                            seek_position_ = (total_filesize_ & 0xfffffe00);
                            bufend = Sectorsize;
                            current_buffer_location_ = (remaining_cluster_bytes_ & 0x1ff);
                            cluster_write_offset_ = 0;
                            directory_entry_position_ = Dirptr;
                            if (current_buffer_location_) {
                                Sdspi.ReadBlock(Datablock(), (char *) (&Buf));
                                RET_IF_ERROR;

                                remaining_cluster_bytes_ = (Freeentry - (seek_position_ & (Freeentry - 1)));
                            } else {
                                if ((current_cluster_ < 2) || (remaining_cluster_bytes_ == Freeentry)) {
                                    remaining_cluster_bytes_ = 0;
                                } else {
                                    remaining_cluster_bytes_ = (Freeentry - (seek_position_ & (Freeentry - 1)));
                                }
                            }
                            if (current_cluster_ >= 2) {
                                Followchain();
                                RET_IF_ERROR;

                            }
                            return;
                        } else {
                            THROW(kErrorBadArgument);
                        }
                    }
                }
                Dirptr = (Dirptr + _step__0028);
            } while (((_step__0028 > 0) && (Dirptr <= _limit__0027)) || ((_step__0028 < 0) && (Dirptr >= _limit__0027)));
        }

        if (Mode == 'd') { //If we got here it's because we didn't find anything to delete.
            return;
        }

        if ((Mode != 'w') && (Mode != 'a')) {
            THROW(kErrorFileNotFound);
        }
        directory_entry_position_ = Freeentry;
        if (directory_entry_position_ == 0) {
            THROW(kErrorNoEmptyDirectoryEntry);
        }
        // write (or new append): create valid directory entry
        S = Readbytec(directory_entry_position_);
        RET_IF_ERROR;

        memset((void *) S, 0, 1 * (Dirsize));
        memcpy((void *) S, (void *) &Padname, 1 * (11));
        Brwword((S + 0x1a), 0);
        Brwword((S + 0x14), 0);
        I = Pdate;
        Brwlong((S + 0x0e), I); // write create time and date
        Brwlong((S + 0x16), I); // write last modified date and time
        if ((directory_entry_position_ == Sentinel) && ((directory_entry_position_ + Dirsize) < rootdirend)) {
            Brwword(Readbytec((directory_entry_position_ + Dirsize)), 0);
        }
        Flushifdirty();
        RET_IF_ERROR;

        cluster_write_offset_ = 0;
        current_cluster_ = 0;
        bufend = Sectorsize;
    }

    /** Flush and close the currently open file if any.  
     * 
     * Clears any errors.
     * 
     * Also reset the pointers 
     * to valid values. Also, releases the SD pins to tristate.
     */
    void Close(void) {
        ClearError();
        if (directory_entry_position_) {
            Pflush();
            RET_IF_ERROR;
        }
        current_buffer_location_ = 0;
        bufend = 0;
        total_filesize_ = 0;
        seek_position_ = 0;
        remaining_cluster_bytes_ = 0;
        cluster_write_offset_ = 0;
        directory_entry_position_ = 0;
        current_cluster_ = 0;
        first_cluster_of_file_ = 0;
        Sdspi.ReleaseCard();
    }

    /** Read and return a single character from the currently open file.
     * 
     * @return -1 if the end of the file is reached. Otherwise, returns the 
     * character in the lower byte.
     */
    int Get(void) {
        int T;
        if (current_buffer_location_ >= bufend) {
            T = Pfillbuf();
            RET_IF_ERROR_NULL;
            if (T <= 0) {
                return (-1);
            }
        }
        return Buf[(current_buffer_location_++)];
    }

    /** Read bytes into a buffer from currently open file.
     * 
     * Note that this function does not null terminate a string.
     * 
     * @param read_buffer The buffer to store the data. The buffer may be as large as you want.
     * @param bytes_to_read_count The number of bytes to read.
     * @return  Returns the number of bytes successfully read, or a negative 
     * number if there is an error. If the end of file has been reached, then 
     * this may be less than bytes_to_read_count.
     */
    int Get(char * read_buffer, int bytes_to_read_count) {
        int T;
        int R = 0;
        while (bytes_to_read_count > 0) {
            if (current_buffer_location_ >= bufend) {
                T = Pfillbuf();
                if (T <= 0) {
                    if (R > 0) {
                        return R;
                    }
                    return T;
                }
            }
            T = (Min__((bufend - current_buffer_location_), bytes_to_read_count));
            if (((T | (int) read_buffer) | current_buffer_location_) & 0x3) {
                memcpy((void *) read_buffer, (void *) (void *) (((int) (&Buf) + current_buffer_location_)), 1 * (T));
            } else {
                memmove((void *) read_buffer, (void *) (void *) (((int) (&Buf) + current_buffer_location_)), 4 * ((Shr__(T, 2))));
            }
            current_buffer_location_ = (current_buffer_location_ + T);
            R = (R + T);
            read_buffer = (read_buffer + T);
            bytes_to_read_count = (bytes_to_read_count - T);
        }
        return R;
    }

    /** Write a single character to the file.
     * 
     * @param    C The character to write.
     * @return  0 if successful, a negative number if an error occurred.
     */
    int Put(const char C) {
        if (current_buffer_location_ == Sectorsize) {
            if (Pflushbuf(Sectorsize, 0) < 0) {
                return (-1);
            }
        }
        Buf[(current_buffer_location_++)] = C;
        return 0;
    }

    /** Write a null-terminated string to the file. 
     * 
     * @param B The null-terminated string to write. No size limitations. Does 
     * not write the null terminator.
     * @return the number of bytes successfully written, or a negative number 
     * if there is an error.
     */
    int Put(const char * B) {
        return Put(B, strlen(B));
    }

    /** Write bytes from buffer into the currently open file.
     * 
     * @param The buffer to pull the data from. The buffer may be as large as 
     * you want.
     * @param Count the number of bytes to write.
     * @return the number of bytes successfully written, or a negative number 
     * if there was an error.
     */
    int Put(const char * Ubuf, int Count) {
        int T;
        int R = 0;
        while (Count > 0) {
            if (current_buffer_location_ >= bufend) {
                Pflushbuf(current_buffer_location_, 0);
                RET_IF_ERROR_NULL;

            }
            T = (Min__((bufend - current_buffer_location_), Count));
            if (((T | (int) Ubuf) | current_buffer_location_) & 0x3) {
                memcpy((void *) (void *) (((int) (&Buf) + current_buffer_location_)), (void *) Ubuf, 1 * (T));
            } else {
                memmove((void *) (void *) (((int) (&Buf) + current_buffer_location_)), (void *) Ubuf, 4 * ((Shr__(T, 2))));
            }
            R = (R + T);
            current_buffer_location_ = (current_buffer_location_ + T);
            Ubuf = (Ubuf + T);
            Count = (Count - T);
        }
        return R;
    }

    /** Set up for a directory file listing.
     * 
     * Close the currently open file, and set up the read buffer for calls to 
     * @a nextfile().
     */
    void OpenRootDirectory(void) {
        Close();
        RET_IF_ERROR;

        int Off = (rootdir - (dataregion << Sectorshift));
        current_cluster_ = (Shr__(Off, (clustershift + Sectorshift)));
        seek_position_ = (Off - (current_cluster_ << (clustershift + Sectorshift)));
        remaining_cluster_bytes_ = (rootdirend - rootdir);
        total_filesize_ = (seek_position_ + remaining_cluster_bytes_);
    }

    /** Find the next file in the root directory and extract its (8.3) name into 
     * filename.  The buffer must be sized to hold at least 13 characters 
     * (8 + 1 + 3 + 1).
     * 
     * @param filename The extracted filename
     * @return true if there is a valid filename, false otherwise.
     */
    bool NextFile(char * filename) {
        while (true) {
            if (current_buffer_location_ >= bufend) {
                int T = Pfillbuf();
                if (T < 0) {
                    return false;
                }
                if (((Shr__(seek_position_, Sectorshift)) & ((1 << clustershift) - 1)) == 0) {
                    (current_cluster_++);
                }
            }

            unsigned char * at = (unsigned char *) ((int) &Buf + current_buffer_location_);

            if ((at)[0] == 0) {
                return false;
            }
            current_buffer_location_ = (current_buffer_location_ + Dirsize);
            if (((at)[0] != 0xe5)
                    && (((at)[0x0b] & 0x18) == 0)) {
                char * lns = filename;

                for (int i = 0; i <= 10; i++) {
                    filename[0] = (at)[i];
                    filename++;
                    if (at[i] != ' ') {
                        lns = filename;
                    }
                    if (i == 7 || i == 10) {
                        filename = lns;
                        if (i == 7) {
                            filename[0] = '.';
                            filename++;
                        }
                    }
                }
                filename[0] = 0;
                return true;
            }
        }
    }

    /** Change the read pointer to a different position in the file.
     * 
     * Seek() works only in 'r' (read) mode.
     * 
     * @param position The position to seek to, relative to the beginning of the file. Units?
     * 
     * @return 0 on success, a negative number on failure (such as seeking 
     * during write). Failures may include seeking outside the file size.
     */
    int Seek(int position) {
        int delta;
        if (((directory_entry_position_) || (position < 0)) || (position > total_filesize_)) {
            return (-1);
        }
        delta = ((seek_position_ - bufend) & (-clustersize));
        if (position < delta) {
            current_cluster_ = first_cluster_of_file_;
            remaining_cluster_bytes_ = (Min__(clustersize, total_filesize_));
            seek_position_ = 0;
            current_buffer_location_ = 0;
            bufend = 0;
            delta = 0;
        }
        while (position >= (delta + clustersize)) {
            current_cluster_ = Nextcluster();
            RET_IF_ERROR_NULL;

            seek_position_ = (seek_position_ + clustersize);
            delta = (delta + clustersize);
            remaining_cluster_bytes_ = (Min__(clustersize, (total_filesize_ - seek_position_)));
            current_buffer_location_ = 0;
            bufend = 0;
        }
        if (bufend == 0
                || position < (seek_position_ - bufend)
                || position >= (seek_position_ - bufend) + Sectorsize
                ) { // must change buffer
            //Warning: this section does not seem to be covered by unit tests. What's required for coverage?
            delta = (seek_position_ + remaining_cluster_bytes_);
            seek_position_ = (position & -Sectorsize);
            remaining_cluster_bytes_ = (delta - seek_position_);
            Pfillbuf();
            RET_IF_ERROR_NULL;
        }
        current_buffer_location_ = position & (Sectorsize - 1);
        return 0;
    }

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
    int SetDate(int Year, int Month, int Day, int Hour, int Minute, int Second) {
        Pdate = ((((Year - 1980) << 25) + (Month << 21)) + (Day << 16));
        Pdate = (Pdate + (((Hour << 11) + (Minute << 5)) + (Shr__(Second, 1))));
        return Pdate;
    }

    /** If there was an error in the SD routines then this function will return
     * an error code.
     * 
     * @return The error code.
     */
    bool HasError(void) {
        return (error != kNoError) || Sdspi.HasError();
    }

    /** Resets the error flag to kNoError.
     */
    void ClearError(void) {
        error = kNoError;
        Sdspi.ClearError();
    }

    /** Get the error code.
     * 
     * @return The error code.
     */
    int GetError(void) {
        if (error != kNoError) {
            return error;
        } else {
            return Sdspi.GetError();
        }
    }

    /** Get the FAT cluster size.
     * @return the size of the cluster, in bytes.
     */
    int GetClusterSize(void) {
        return clustersize;
    }

    /** Get the current FAT cluster count.
     * 
     * What does this mean? I (SRLM) don't know. I also don't know how to test it, so it is not tested.
     * 
     * @return the cluster count.
     */
    int GetClusterCount(void) {
        return Totclusters;
    }

    /**
     * 
     * @return 
     */
    int GetFilesize(void) {
        return total_filesize_;
    }

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
     * @param abort_code passed through to return.
     */
    void SetErrorCode(int abort_code) {
        error = abort_code;
    }

    /*
    On metadata writes, if we are updating the FAT region, also update
     the second FAT region.
     */
    void Writeblock2(int N, char * B) {
        Sdspi.WriteBlock(N, B);
        RET_IF_ERROR;
        if (N >= Fat1) {
            if (N < (Fat1 + Sectorsperfat)) {
                Sdspi.WriteBlock((N + Sectorsperfat), B);
                RET_IF_ERROR;
            }
        }
    }

    /* If the metadata block is dirty, write it out.
     */
    void Flushifdirty(void) {
        if (Dirty) {
            Writeblock2(Lastread, (char *) (&Buf2));
            RET_IF_ERROR;
            Dirty = 0;
        }
    }

    /* Read a block into the metadata buffer, if that block is not already
'   there.
     */
    void Readblockc(int N) {
        if (N != Lastread) {
            Flushifdirty();
            RET_IF_ERROR;
            Sdspi.ReadBlock(N, (char *) (&Buf2));
            RET_IF_ERROR;
            Lastread = N;
        }
    }

    /*
    Read a byte-reversed word from a (possibly odd) address.
     */
    int Brword(char * b) {
        return ((b)[0] + ((b)[1] << 8));
    }

    /*
    Read a byte-reversed long from a (possibly odd) address.
     */
    int Brlong(char * B) {
        return (Brword(B) + (Brword((B + 2)) << 16));
    }

    /*
    Read a cluster entry.
     */
    int Brclust(char * B) {
        if (filesystem == 1) {
            return Brword(B);
        } else {
            return Brlong(B);
        }
    }

    /*
    Write a byte-reversed word to a (possibly odd) address, and
      mark the metadata buffer as dirty.
     */
    void Brwword(char * w, int v) {
        w++[0] = v;
        w[0] = v >> 8;
        Dirty = 1;
    }

    /*
    Write a byte-reversed long to a (possibly odd) address, and
     mark the metadata buffer as dirty.
     */
    void Brwlong(char * w, int v) {
        Brwword(w, v);
        Brwword(w + 2, v >> 16);
    }

    /*
     Write a cluster entry.
     */
    void Brwclust(char * w, int v) {
        //   Write a cluster entry.
        if (filesystem == 1) {
            Brwword(w, v);
        } else {
            Brwlong(w, v);
        }
    }

    int Getfstype(void) {
        const int kFAT1 = 'F' + ('A' << 8) + ('T' << 16) + ('1' << 24);
        const int kFAT3 = 'F' + ('A' << 8) + ('T' << 16) + ('3' << 24);

        if ((Brlong(&Buf[0x36]) == kFAT1) && (Buf[58] == '6')) {
            return kFileSystemFAT16;
        }
        if ((Brlong(&Buf[0x52]) == kFAT3) && (Buf[86] == '2')) {
            return kFileSystemFAT32;
        }
        return kFileSystemUnknown;
    }

    /*
    Read a byte address from the disk through the metadata buffer and
     return a pointer to that location.
     * @todo(SRLM): Make sure all calls have a RET_IF_ERROR
     */
    char * Readbytec(int Byteloc) {
        Readblockc((Shr__(Byteloc, Sectorshift)));
        RET_IF_ERROR_NULL;

        return ((char *) (&Buf2) + (Byteloc & 0x1ff));
    }

    /*
    Read a fat location and return a pointer to the location of that
     entry.
     */
    char * Readfat(int Clust) {
        last_fat_entry_ = ((Fat1 << Sectorshift) + (Clust << filesystem));
        return Readbytec(last_fat_entry_);
    }

    /*
    Follow the fat chain and update the writelink.
     */
    int Followchain(void) {
        char * temp;
        int R = 0;
        temp = Readfat(current_cluster_);
        RET_IF_ERROR_NULL;
        R = Brclust(temp);
        cluster_write_offset_ = last_fat_entry_;
        return R;
    }

    /*
     Read the next cluster and return it.  Set up writelink to point to the cluster we just read, for later updating.  If the cluster number is bad, return a negative number.
     */

    int Nextcluster(void) {
        int R = Followchain();
        RET_IF_ERROR_NULL;
        if ((R < 2) || (R >= Totclusters)) {
            THROW_NULL(kErrorBadClusterValue);
        }
        return R;
    }

    /*
    Free an entire cluster chain.  Used by remove and by overwrite.
      Assumes the pointer has already been cleared/set to end of chain.
     */
    void Freeclusters(int Clust) {
        while (Clust < Endofchain) {
            if (Clust < 2) {
                THROW(kErrorBadClusterNumber);
            }
            char * Bp = Readfat(Clust);
            RET_IF_ERROR;

            Clust = Brclust(Bp);
            Brwclust(Bp, 0);
        }
        Flushifdirty();
        RET_IF_ERROR;
    }

    /*
    This is just a pass-through function to allow the block layer
     to tristate the I/O pins to the card.
     */
    void Release(void) {
        Sdspi.ReleaseCard();
    }

    /* Calculate the block address of the current data location.
     */
    int Datablock(void) {
        return (((current_cluster_ << clustershift) + dataregion) + ((Shr__(seek_position_, Sectorshift)) & ((1 << clustershift) - 1)));

    }

    /*
    Compute the upper case version of a character.
     * */
    char ConvertToUppercase(char C) {
        if (('a' <= C) && (C <= 'z')) {
            return (C - 32);
        }
        return C;
    }

    /*
     Flush the current buffer, if we are open for write.  This may
'   allocate a new cluster if needed.  If metadata is true, the
'   metadata is written through to disk including any FAT cluster
'   allocations and also the file size in the directory entry.
     */
    int Pflushbuf(int Rcnt, int Metadata) {
        if (directory_entry_position_ == 0) {
            THROW_NULL(kErrorFileNotOpenForWriting);
        }
        if (Rcnt > 0) { // must *not* allocate cluster if flushing an empty buffer
            if (remaining_cluster_bytes_ < Sectorsize) {
                // find a new cluster could be anywhere!  If possible, stay on the
                // same page used for the last cluster.
                int Newcluster = (-1);
                int Cluststart = (current_cluster_ & (~((Shr__(Sectorsize, filesystem)) - 1)));
                int Count = 2;
                while (1) {
                    Readfat(Cluststart);
                    RET_IF_ERROR_NULL;

                    int I;
                    {
                        int _limit__0025 = (Sectorsize - (1 << filesystem));
                        int _step__0026 = (1 << filesystem);
                        I = 0;
                        if (I >= _limit__0025) _step__0026 = -_step__0026;
                        do {
                            if (Buf2[I] == 0) {
                                if (Brclust(((char *) (&Buf2) + I)) == 0) {
                                    Newcluster = (Cluststart + (Shr__(I, filesystem)));
                                    if (Newcluster >= Totclusters) {
                                        Newcluster = (-1);
                                    }
                                    break;
                                }
                            }
                            I = (I + _step__0026);
                        } while (((_step__0026 > 0) && (I <= _limit__0025)) || ((_step__0026 < 0) && (I >= _limit__0025)));
                    }
                    if (Newcluster > 1) {
                        Brwclust(((char *) (&Buf2) + I), (Endofchain + 15));
                        if (cluster_write_offset_ == 0) {
                            Brwword((Readbytec(directory_entry_position_) + 26), Newcluster);
                            cluster_write_offset_ = (directory_entry_position_ & (Sectorsize - filesystem));
                            Brwlong((((char *) (&Buf2) + cluster_write_offset_) + 28), (seek_position_ + current_buffer_location_));
                            if (filesystem == 2) {
                                Brwword((((char *) (&Buf2) + cluster_write_offset_) + 20), (Shr__(Newcluster, 16)));
                            }
                        } else {
                            Brwclust(Readbytec(cluster_write_offset_), Newcluster);
                        }
                        cluster_write_offset_ = (last_fat_entry_ + I);
                        current_cluster_ = Newcluster;
                        remaining_cluster_bytes_ = clustersize;
                        break;
                    } else {
                        Cluststart = (Cluststart + (Shr__(Sectorsize, filesystem)));
                        if (Cluststart >= Totclusters) {
                            Cluststart = 0;
                            (Count--);
                            if (Rcnt < 0) {
                                Rcnt = -5; //No space left on device
                                break;
                            }
                        }
                    }
                }
            }
            if (remaining_cluster_bytes_ >= Sectorsize) {
                Sdspi.WriteBlock(Datablock(), (char *) (&Buf));
                RET_IF_ERROR_NULL;

                if (Rcnt == Sectorsize) { // full buffer, clear it
                    seek_position_ = (seek_position_ + Rcnt);
                    remaining_cluster_bytes_ = (remaining_cluster_bytes_ - Rcnt);
                    current_buffer_location_ = 0;
                    bufend = Rcnt;
                }
            }
        }
        if ((Rcnt < 0) || (Metadata)) { // update metadata even if error
            Readblockc((Shr__(directory_entry_position_, Sectorshift))); // flushes unwritten FAT too
            RET_IF_ERROR_NULL;

            Brwlong((((char *) (&Buf2) + (directory_entry_position_ & (Sectorsize - filesystem))) + 28), (seek_position_ + current_buffer_location_));
            Flushifdirty();
            RET_IF_ERROR_NULL;
        }
        if (Rcnt < 0) {
            THROW_NULL(Rcnt);
        }
        return Rcnt;
    }

    /*
     * Call flush with the current data buffer location, and the flush
'   metadata flag set.
     * 
     */
    int Pflush(void) {
        return Pflushbuf(current_buffer_location_, 1);
    }

    /*
    Get some data into an empty buffer.  If no more data is available,
'   return -1.  Otherwise return the number of bytes read into the
'   buffer.
     * */
    int Pfillbuf(void) {
        if (seek_position_ >= total_filesize_) {
            return -1;
        }
        if (remaining_cluster_bytes_ == 0) {
            current_cluster_ = Nextcluster();
            if (current_cluster_ < 0) {
                return current_cluster_;
            }
            remaining_cluster_bytes_ = (Min__(clustersize, (total_filesize_ - seek_position_)));
        }
        Sdspi.ReadBlock(Datablock(), (char *) (&Buf));
        RET_IF_ERROR_NULL;

        int R = Sectorsize;
        if ((seek_position_ + R) >= total_filesize_) {
            R = (total_filesize_ - seek_position_);
        }
        seek_position_ = (seek_position_ + R);
        remaining_cluster_bytes_ = (remaining_cluster_bytes_ - R);
        current_buffer_location_ = 0;
        bufend = R;
        return R;
    }

    static int Min__(int a, int b) {
        return a < b ? a : b;
    }

    static int Shr__(unsigned int a, unsigned int b) {
        return (a >> b);
    }


};

#endif // SRLM_PROPGCC_SECUREDIGITAL_CARD_H__
