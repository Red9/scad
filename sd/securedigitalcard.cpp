
#include "securedigitalcard.h"

#define RET_IF_ERROR_INT if(HasError()){return GetError();}
#define RET_IF_ERROR if(HasError()){return;}
#define THROW_INT(value) {SetErrorCode((value)); return GetError();}
#define THROW(value) {SetErrorCode((value)); return;}


static int Min__(int a, int b) {
    return a < b ? a : b;
}

static int Shr__(unsigned int a, unsigned int b) {
    return (a >> b);
}

SecureDigitalCard::~SecureDigitalCard(void){
    Unmount();
}

void SecureDigitalCard::Release(void) {
    Sdspi.ReleaseCard();
}

void SecureDigitalCard::Writeblock2(int N, char * B) {
    Sdspi.WriteBlock(N, B);
    RET_IF_ERROR;
    if (N >= Fat1) {
        if (N < (Fat1 + Sectorsperfat)) {
            Sdspi.WriteBlock((N + Sectorsperfat), B);
            RET_IF_ERROR;
        }
    }
}

void SecureDigitalCard::Flushifdirty(void) {
    if (Dirty) {
        Writeblock2(Lastread, (char *) (&Buf2));
        RET_IF_ERROR;
        Dirty = 0;
    }
   
}

int SecureDigitalCard::Readblockc(int N) {
    if (N != Lastread) {
        Flushifdirty();
        RET_IF_ERROR_INT;
        Sdspi.ReadBlock(N, (char *) (&Buf2));
        RET_IF_ERROR_INT;
        Lastread = N;
    }
    return 0;
}

int SecureDigitalCard::Brword(char * b) {
    return ((b)[0] + ((b)[1] << 8));
}

int SecureDigitalCard::Brlong(char * B) {
    return (Brword(B) + (Brword((B + 2)) << 16));
}

int SecureDigitalCard::Brclust(char * B) {
    if (filesystem == 1) {
        return Brword(B);
    } else {
        return Brlong(B);
    }
}

void SecureDigitalCard::Brwword(char *w, int v) {
    w++[0] = v;
    w[0] = v >> 8;
    Dirty = 1;
}

void SecureDigitalCard::Brwlong(char *w, int v) {
    Brwword(w, v);
    Brwword(w + 2, v >> 16);
}

void SecureDigitalCard::Brwclust(char *w, int v) {
    //   Write a cluster entry.
    if (filesystem == 1) {
        Brwword(w, v);
    } else {
        Brwlong(w, v);
    }
}

int SecureDigitalCard::Unmount(void) {
    Close();
    RET_IF_ERROR_INT;
    Sdspi.Stop();
    return 0;
}

int SecureDigitalCard::Getfstype(void) {
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

int SecureDigitalCard::Mount(int Do, int Clk, int Di, int Cs) {
    int Start, Sectorspercluster, Reserved, Rootentries, Sectors;
    if (Pdate == 0) {
        SetDate(2010, 1, 1, 0, 0, 0);
    }

    //SRLM Addition: check to make sure that Buf and Buf2 are longword aligned.
    //Theoretically, this should have no runtime cost, but it looks like in CMM
    //and -Os it takes 16 bytes. It can be commented out if you're sure that
    //Buf and Buf2 are longword aligned.
    if ((((int) Buf) & 0b11) != 0)
        THROW_INT(kErrorBufNotLongwordAligned);
    if ((((int) Buf2) & 0b11) != 0)
        THROW_INT(kErrorBufNotLongwordAligned);

    Unmount();
    RET_IF_ERROR_INT;
    
    Sdspi.Start(Do, Clk, Di, Cs);
    RET_IF_ERROR_INT;
    
    Lastread = (-1);
    Dirty = 0;
    Sdspi.ReadBlock(0, (char *) (&Buf));
    RET_IF_ERROR_INT;
    
    if (Getfstype() != kFileSystemUnknown) {
        Start = 0;
    } else {
        Start = Brlong(((char *) (&Buf) + 454));
        Sdspi.ReadBlock(Start, (char *) (&Buf));
        RET_IF_ERROR_INT;
    }
    filesystem = Getfstype();
    if (filesystem == kFileSystemUnknown) {
        THROW_INT(kErrorNotFatVolume);
    }
    if (Brword(((char *) (&Buf) + 11)) != Sectorsize) {
        THROW_INT(kErrorBadBytesPerSector);
    }
    Sectorspercluster = Buf[13];
    if (Sectorspercluster & (Sectorspercluster - 1)) {
        THROW_INT(kErrorBadSectorsPerCluster);
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
        THROW_INT(kErrorNotTwoFats);
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
        THROW_INT(kErrorBadFatSignature);
    }
    Totclusters = (Shr__(((Sectors - dataregion) + Start), clustershift));
    return 0;
}

int SecureDigitalCard::Mount(int Basepin) {
    return Mount(Basepin, (Basepin + 1), (Basepin + 2), (Basepin + 3));
}

char * SecureDigitalCard::Readbytec(int Byteloc) {
    Readblockc((Shr__(Byteloc, Sectorshift)));
    RET_IF_ERROR_INT;
    
    return ((char *) (&Buf2) + (Byteloc & 0x1ff));
}

char * SecureDigitalCard::Readfat(int Clust) {
    last_fat_entry_ = ((Fat1 << Sectorshift) + (Clust << filesystem));
    return Readbytec(last_fat_entry_);
}

int SecureDigitalCard::Followchain(void) {
    char * temp;
    int R = 0;
    temp = Readfat(current_cluster_);
    RET_IF_ERROR_INT;
    R = Brclust(temp);
    cluster_write_offset_ = last_fat_entry_;
    return R;
}

int SecureDigitalCard::Nextcluster(void) {
    int R = Followchain();
    RET_IF_ERROR_INT;
    if ((R < 2) || (R >= Totclusters)) {
        THROW_INT(kErrorBadClusterValue);
    }
    return R;
}

int SecureDigitalCard::Freeclusters(int Clust) {
    

    while (Clust < Endofchain) {
        if (Clust < 2) {
            THROW_INT(kErrorBadClusterNumber);
        }
        char * Bp = Readfat(Clust);
        RET_IF_ERROR_INT;
        
        Clust = Brclust(Bp);
        Brwclust(Bp, 0);
    }
    Flushifdirty();
    RET_IF_ERROR_INT;
    
    return 0;
}

int SecureDigitalCard::Datablock(void) {
    return (((current_cluster_ << clustershift) + dataregion) + ((Shr__(seek_position_, Sectorshift)) & ((1 << clustershift) - 1)));
}

char SecureDigitalCard::ConvertToUppercase(char C) {
    if (('a' <= C) && (C <= 'z')) {
        return (C - 32);
    }
    return C;
}

int SecureDigitalCard::Pflushbuf(int Rcnt, int Metadata) {
    if (directory_entry_position_ == 0) {
        THROW_INT(kErrorFileNotOpenForWriting);
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
                RET_IF_ERROR_INT;
                
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
            RET_IF_ERROR_INT;
            
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
        RET_IF_ERROR_INT;
        
        Brwlong((((char *) (&Buf2) + (directory_entry_position_ & (Sectorsize - filesystem))) + 28), (seek_position_ + current_buffer_location_));
        Flushifdirty();
        RET_IF_ERROR_INT;
    }
    if (Rcnt < 0) {
        THROW_INT(Rcnt);
    }
    return Rcnt;
}

int SecureDigitalCard::Pflush(void) {
    return Pflushbuf(current_buffer_location_, 1);
}

int SecureDigitalCard::Pfillbuf(void) {
    
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
    RET_IF_ERROR_INT;
    
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

int SecureDigitalCard::Close(void) {
    int R = 0;
    if (directory_entry_position_) {
        R = Pflush();
        RET_IF_ERROR_INT;
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
    return R;
}

int SecureDigitalCard::SetDate(int Year, int Month, int Day, int Hour, int Minute, int Second) {

    Pdate = ((((Year - 1980) << 25) + (Month << 21)) + (Day << 16));
    Pdate = (Pdate + (((Hour << 11) + (Minute << 5)) + (Shr__(Second, 1))));
    return Pdate;
}

int SecureDigitalCard::Open(const char * filename, const char Mode) {
    char * S; // = Filename;


    Close();
    RET_IF_ERROR_INT;


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
            RET_IF_ERROR_INT;
            
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
                    return total_filesize_;
                }
                if ((S)[11] & 0xd9) {
                    THROW_INT(kErrorNoWritePermission);
                }

                //Mode is Delete
                if (Mode == 'd') {
                    Brwword(S, 0xe5);
                    if (current_cluster_) {
                        Freeclusters(current_cluster_);
                        RET_IF_ERROR_INT;
                    }
                    Flushifdirty();
                    RET_IF_ERROR_INT;
                    
                    return 0;
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
                        RET_IF_ERROR_INT;
                        
                    }
                    bufend = Sectorsize;
                    current_cluster_ = 0;
                    total_filesize_ = 0;
                    remaining_cluster_bytes_ = 0;
                    return 0;
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
                                THROW_INT(kErrorEofWhileFollowingChain);
                            }
                            current_cluster_ = Nextcluster();
                            RET_IF_ERROR_INT;
                            
                            remaining_cluster_bytes_ = (remaining_cluster_bytes_ - Freeentry);
                        }
                        seek_position_ = (total_filesize_ & 0xfffffe00);
                        bufend = Sectorsize;
                        current_buffer_location_ = (remaining_cluster_bytes_ & 0x1ff);
                        cluster_write_offset_ = 0;
                        directory_entry_position_ = Dirptr;
                        if (current_buffer_location_) {
                            Sdspi.ReadBlock(Datablock(), (char *) (&Buf));
                            RET_IF_ERROR_INT;
                            
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
                            RET_IF_ERROR_INT;
                            
                        }
                        return 0;
                    } else {
                        THROW_INT(kErrorBadArgument);
                    }
                }
            }
            Dirptr = (Dirptr + _step__0028);
        } while (((_step__0028 > 0) && (Dirptr <= _limit__0027)) || ((_step__0028 < 0) && (Dirptr >= _limit__0027)));
    }
    
    if(Mode == 'd'){ //If we got here it's because we didn't find anything to delete.
        return 0;
    }
    
    if ((Mode != 'w') && (Mode != 'a')) {
        THROW_INT(kErrorFileNotFound);
    }
    directory_entry_position_ = Freeentry;
    if (directory_entry_position_ == 0) {
        THROW_INT(kErrorNoEmptyDirectoryEntry);
    }
    // write (or new append): create valid directory entry
    S = Readbytec(directory_entry_position_);
    RET_IF_ERROR_INT;
    
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
    RET_IF_ERROR_INT;
    
    cluster_write_offset_ = 0;
    current_cluster_ = 0;
    bufend = Sectorsize;
    return 0;
}

int SecureDigitalCard::Get(char * read_buffer, int bytes_to_read_count) {
    int T;
    int R = 0;
    while (bytes_to_read_count > 0) {
        if (current_buffer_location_ >= bufend) {
            T = Pfillbuf();
            if (T <= 0) {
                if(R > 0){
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

int SecureDigitalCard::Get(void) {
    int T;
    if (current_buffer_location_ >= bufend) {
        T = Pfillbuf();
        RET_IF_ERROR_INT;
        if (T <= 0) {
            return (-1);
        }
    }
    return Buf[(current_buffer_location_++)];
}

int SecureDigitalCard::Put(const char * Ubuf, int Count) {
    int T;
    int R = 0;
    while (Count > 0) {
        if (current_buffer_location_ >= bufend) {
            Pflushbuf(current_buffer_location_, 0);
            RET_IF_ERROR_INT;
            
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

int SecureDigitalCard::Put(const char * B) {
    return Put(B, strlen(B));
}

int SecureDigitalCard::Put(const char C) {
    if (current_buffer_location_ == Sectorsize) {
        if (Pflushbuf(Sectorsize, 0) < 0) {
            return (-1);
        }
    }
    Buf[(current_buffer_location_++)] = C;
    return 0;
}

int SecureDigitalCard::Seek(int position) {
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
        RET_IF_ERROR_INT;
        
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
        RET_IF_ERROR_INT;
    }
    current_buffer_location_ = position & (Sectorsize - 1);
    return 0;
}

int SecureDigitalCard::OpenRootDirectory(void) {

    Close();
    RET_IF_ERROR_INT;
    
    int Off = (rootdir - (dataregion << Sectorshift));
    current_cluster_ = (Shr__(Off, (clustershift + Sectorshift)));
    seek_position_ = (Off - (current_cluster_ << (clustershift + Sectorshift)));
    remaining_cluster_bytes_ = (rootdirend - rootdir);
    total_filesize_ = (seek_position_ + remaining_cluster_bytes_);
    return 0;
}

int SecureDigitalCard::NextFile(char * filename) {

    while (true) {
        if (current_buffer_location_ >= bufend) {
            int T = Pfillbuf();
            if (T < 0) {
                return T;
            }
            if (((Shr__(seek_position_, Sectorshift)) & ((1 << clustershift) - 1)) == 0) {
                (current_cluster_++);
            }
        }

        unsigned char * at = (unsigned char *) ((int) &Buf + current_buffer_location_);

        if ((at)[0] == 0) {
            THROW_INT(kErrorFileNotFound);
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
            return 0;
        }
    }
}

int SecureDigitalCard::GetClusterSize(void) {
    return clustersize;
}

int SecureDigitalCard::GetClusterCount(void) {
    return Totclusters;
}

void SecureDigitalCard::SetErrorCode(int new_error_code){
    error = new_error_code;
}

bool SecureDigitalCard::HasError(void){
    return (error != kNoError) || Sdspi.HasError();
}

void SecureDigitalCard::ClearError(void){
    error = kNoError;
    Sdspi.ClearError();
}

int SecureDigitalCard::GetError(void){
    if(error != kNoError){
        return error;
    }else{
        return Sdspi.GetError();
    }
}