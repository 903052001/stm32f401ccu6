/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.               */
/*                                                                            */
/* 文件名称: Fn_gzip.c                                                        */
/* 内容摘要: GZIP压缩源文件                                                   */
/* 其它说明: 压缩率 70%                                                       */
/* 当前版本: v 1.0.0                                                          */
/* 作    者: Leonard                                                          */
/* 完成日期: 2020-11-11                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号     修改人          修改内容                 */
/* -------------------------------------------------------------------------- */
/*     2020-11-11        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/


/******************************************************************************/
/*               #include(依次为标准库头文件、非标准库头文件)                */
/******************************************************************************/
#include "Fn_gzip.h"

/******************************************************************************/
/*                                外部引用声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 内部宏定义                                 */
/******************************************************************************/
/* All codes must not exceed MAX_BITS bits */
#define MAX_BITS 15

/* Bit length codes must not exceed MAX_BL_BITS bits */
#define MAX_BL_BITS 7

/* number of length codes, not counting the special END_BLOCK code */
#define LENGTH_CODES 29

/* number of literal bytes 0..255 */
#define LITERALS 256

/* end of block literal code */
#define END_BLOCK 256

/* number of Literal or Length codes, including the END_BLOCK code */
#define L_CODES ( LITERALS + 1 + LENGTH_CODES )

/* number of distance codes */
#define D_CODES 30

/* number of codes used to transfer the bit lengths */
#define BL_CODES 19


/******************************************************************************/
/*                              内部数据类型定义                              */
/******************************************************************************/


/******************************************************************************/
/*                                内部函数原型                                */
/******************************************************************************/


/******************************************************************************/
/*                               全局(静态)变量                               */
/******************************************************************************/
static unsigned int crc = 0;                          /* 未压缩文件数据的crc */

static int unzip_mem_inpos = 0;
static int unzip_mem_insize = 0;
static char *unzip_mem_inptr = NULL;

static char *zip_mem_outptr = NULL;
static int zip_mem_outlen = 0;

static char *zip_mem_inptr = NULL;  /* 输入缓存 */
static int zip_mem_insize = 0; /* 输入缓存长度 */
static int zip_mem_inpos = 0; /* 已经使用的位置 */

static unsigned int crc_32_tab[] =
{
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L,
    0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL,
    0xe7b82d07L, 0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL,
    0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
    0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
    0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL, 0x35b5a8faL, 0x42b2986cL,
    0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL,
    0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
    0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL,
    0xb6662d3dL, 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
    0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL,
    0x086d3d2dL, 0x91646c97L, 0xe6635c01L, 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
    0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL,
    0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L, 0x4db26158L, 0x3ab551ceL,
    0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
    0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
    0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L,
    0xce61e49fL, 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L,
    0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L,
    0x9dd277afL, 0x04db2615L, 0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L,
    0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L,
    0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L, 0xd6d6a3e8L,
    0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
    0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
    0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L,
    0x220216b9L, 0x5505262fL, 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L,
    0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
    0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L, 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL,
    0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
    0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L, 0x88085ae6L,
    0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
    0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL,
    0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L,
    0x47b2cf7fL, 0x30b5ffe9L, 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
    0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
    0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL,
};

/******************************************************************************/
/*                                内部函数实现                                */
/******************************************************************************/
/******************************************************************************/
/* 通过crc移位寄存器运行一组字节.如果s是空指针,则初始化crc移位寄存器的内容. */
/* 在任何一种情况下都返回当前的crc.                                          */
/******************************************************************************/
static unsigned int updcrc(unsigned char *s, unsigned int n)
{
    register unsigned int c;
    static unsigned int crc = (unsigned int)0xffffffffL;

    if (s == NULL)
    {
        c = 0xffffffffL; /* 初始化crc */
    }
    else
    {
        c = crc;
        if (n != 0)
        {
            do
            {
                c = crc_32_tab[((int)c ^ (* s ++)) & 0xff] ^ (c >> 8);     /* 计算crc */
            }
            while (--n);
        }
    }
    crc = c;
    return c ^ 0xffffffffL; /* 而不是64位计算机的 ~c */
}

/* ===========================================================================
* Clear input and output buffers
*/
void clear_bufs()
{
    outcnt = 0;
    insize = inptr = 0;
    bytes_in = bytes_out = 0L;
    return;
}

/******************************************************************************/
/* 填充输入缓冲区。只有当缓冲区为空时才会调用此函数.                        */
/******************************************************************************/
int fill_inbuf(void)
{
    int len;
#undef min
#define min(a,b) (((int)(a) < (int)(b)) ? (a) : (b))
    /* Read as munsigned char as possible */
    insize = 0;

    do
    {
        len = min(unzip_mem_insize - unzip_mem_inpos, INBUFSIZ - insize);
        if (len > 0)
        {
            memcpy((char *)inbuf + insize, & unzip_mem_inptr[unzip_mem_inpos], len);
            insize += len;
            unzip_mem_inpos += len;
        }
        else
            break;
    }
    while (insize < INBUFSIZ);

    if (insize == 0)
    {
        read_printf();
    }
    bytes_in += (unsigned int)insize;
    inptr = 1;
    return inbuf[0];
}

void flunsigned short_outbuf()
{
    if (outcnt == 0) return;

    write_buf((char *)outbuf, outcnt);
    bytes_out += (unsigned int)outcnt;
    outcnt = 0;
}


void flunsigned short_window()
{
    if (outcnt == 0) return;
    updcrc(window, outcnt);
    write_buf((char *)window, outcnt);
    bytes_out += (unsigned int)outcnt;
    outcnt = 0;
}

void write_buf(char *buf, unsigned int len)
{
    memcpy(& zip_mem_outptr[zip_mem_outlen], buf, len);
    zip_mem_outlen += len;
    return;
}

static char *xmalloc(unsigned int size)
{
    char *cp = (char *)malloc(size);

    if (cp == NULL)  printf("out of memory");
    return cp;
}


int zip(void)
{
    unsigned char flags = 0;
    unsigned short attr = 0;                /* ascii/binary 标志 */
    unsigned short deflate_flags = 0;

    outcnt = 0;                              /* 将头写入gzip文件 */
    method = DEFLATED;
    put_byte(GZIP_MAGIC[0]);                          /* 魔数头 */
    put_byte(GZIP_MAGIC[1]);
    put_byte(DEFLATED);                       /* 压缩方式 */
    put_byte(flags);
    put_long(time_stamp);
    crc = updcrc(0, 0);
    bi_init();
    ct_init(& attr, & method);
    lm_init(level, & deflate_flags);
    put_byte((unsigned char)deflate_flags);
    put_byte(0);
    (void)deflate();
    put_long(crc);
    put_long(isize);
    flunsigned short_outbuf();
    return 0;
}


int unzip(void)
{
    unsigned int orig_crc = 0;
    unsigned int orig_len = 0;
    int n;
    unsigned char buf[8];

    updcrc(NULL, 0);

    inflate();

    for (n = 0; n < 8; n ++)     /* 得到原始crc长度 */
    {
        buf[n] = (unsigned char)get_byte();
    }

    orig_crc = LG(buf);
    orig_len = LG(buf + 4);

    if (orig_crc != updcrc(outbuf, 0))
    {
        printf("invalid compressed data--crc printf");
    }

    if (orig_len != (unsigned int)bytes_out)
    {
        printf("invalid compressed data--length printf");
    }

    return 0;
}




int mem_read(char *buf, unsigned size)
{
    int len;
#define min( a, b ) ((( int )( a ) < ( int )( b )) ? ( a ) : ( b ))
    len = min(zip_mem_insize - zip_mem_inpos, size);
    if (len > 0)
    {
        memcpy(buf, & zip_mem_inptr[zip_mem_inpos], len);
        crc = updcrc((unsigned char *)buf, len);
        isize += (unsigned int)len;
        zip_mem_inpos += len;
    }
    else
        len = - 1;
    return (int)len;
}

static unsigned short bi_buf = 0;
/* Output buffer. bits are inserted starting at the bottom (least significant
* bits).
*/

#define Buf_size (8 * 2*sizeof(char))
/* Number of bits used within bi_buf. (bi_buf might be implemented on
* more than 16 bits on some systems.)
*/

static int bi_valid = 0;
/* Number of valid bits in bi_buf. All bits above the last valid bit
* are always zero.
*/

int (* read_buf) OF((char *buf, unsigned size));
/* Current input function. Set to mem_read for in-memory compression */

/* ===========================================================================
* Initialize the bit string routines.
*/
void bi_init(void)
{
    bi_buf = 0;
    bi_valid = 0;

    /* Set the defaults for file compression. They are set by memcompress
    * for in-memory compression.
    */
    read_buf = mem_read;
}

/* ===========================================================================
* Send a value on a given number of bits.
* IN assertion: length <= 16 and value fits in length bits.
*/
void send_bits(value, length)
int value; /* value to send */
int length; /* number of bits */
{
    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
    * (16 - bi_valid) bits from value, leaving (width - (16-bi_valid))
    * unused bits in value.
    */
    if (bi_valid > (int)Buf_size - length)
    {
        bi_buf |= (value << bi_valid);
        put_short(bi_buf);
        bi_buf = (unsigned short)value >> (Buf_size - bi_valid);
        bi_valid += length - Buf_size;
    }
    else
    {
        bi_buf |= value << bi_valid;
        bi_valid += length;
    }
}

/* ===========================================================================
* Reverse the first len bits of a code, using straightforward code (a faster
* method would use a table)
* IN assertion: 1 <= len <= 15
*/
unsigned bi_reverse(unsigned code, int len)
{
    register unsigned res = 0;
    do
    {
        res |= code & 1;
        code >>= 1, res <<= 1;
    }
    while (-- len > 0);
    return res >> 1;
}

/* ===========================================================================
* Write out any remaining bits in an incomplete byte.
*/
void bi_windup()
{
    if (bi_valid > 8)  /* 是否大于short长度 */
    {
        put_short(bi_buf);
    }
    else if (bi_valid > 0)
    {
        put_byte(bi_buf);
    }
    bi_buf = 0;
    bi_valid = 0;
}

/* ===========================================================================
* Copy a stored block to the zip file, storing first the length and its
* one's complement if requested.
*/
void copy_block(char *buf, unsigned len, int header)
{
    bi_windup(); /* align on byte boundary */

    if (header)
    {
        put_short((unsigned short)len);
        put_short((unsigned short)~ len);
    }
    while (len --)
    {
        put_byte(* buf ++);
    }
}



/* global buffers */

DECLARE(unsigned char, inbuf, INBUFSIZ + INBUF_EXTRA);
DECLARE(unsigned char, outbuf, OUTBUFSIZ + OUTBUF_EXTRA);
DECLARE(unsigned short, d_buf, DIST_BUFSIZE);
DECLARE(unsigned char, window, 2L * WSIZE);   /* 声明 */
DECLARE(unsigned short, tab_prefix, 1L << BITS);

/* static variables */
int quiet = 0; /* be very quiet (-q) */
int maxbits = BITS; /* max bits per code for LZW */
int method = DEFLATED;/* compression method */
int level = 6; /* compression level */
int exit_code = 0; /* program exit code */
int time_stamp; /* original time stamp (modification time) */

int bytes_in = 0; /* number of input bytes */
int bytes_out = 0; /* number of output bytes */
int total_in = 0; /* input bytes for all files */
int total_out = 0; /* output bytes for all files */
unsigned insize = 0; /* valid bytes in inbuf */
unsigned inptr = 0; /* index of next byte to be processed in inbuf */
unsigned outcnt = 0; /* bytes in output buffer */

static int get_method OF(());

int (* work) OF(()) = zip;  /* function to call */

/* ========================================================================
* Compress
*/
int zipmem(char *mem_inptr, int mem_insize, char *mem_outptr)
{
    time_stamp = 0;
    ALLOC(unsigned char, inbuf, INBUFSIZ + INBUF_EXTRA);
    ALLOC(unsigned char, outbuf, OUTBUFSIZ + OUTBUF_EXTRA);
    ALLOC(unsigned short, d_buf, DIST_BUFSIZE);
    ALLOC(unsigned char, window, 2L * WSIZE);   /* 分配 */
    ALLOC(unsigned short, tab_prefix, 1L << BITS);
    clear_bufs(); /* clear input and output buffers */
    zip_mem_outptr = mem_outptr; /* 输出缓存 */
    zip_mem_outlen = 0; /* 输出缓存大小 */
    zip_mem_inptr = mem_inptr; /* 输入指针 */
    zip_mem_insize = mem_insize; /* 输入缓存长度 */
    zip_mem_inpos = 0; /* 输入缓存当前位置 */
    zip();
    FREE(inbuf);
    FREE(outbuf);
    FREE(d_buf);
    FREE(window);
    FREE(tab_prefix);
    return zip_mem_outlen;
}

/* ========================================================================
* deCompress
*/
int unzipmem(char *mem_inptr, int mem_insize, char *mem_outptr)
{
    time_stamp = 0;
    ALLOC(unsigned char, inbuf, INBUFSIZ + INBUF_EXTRA);
    ALLOC(unsigned char, outbuf, OUTBUFSIZ + OUTBUF_EXTRA);
    ALLOC(unsigned short, d_buf, DIST_BUFSIZE);
    ALLOC(unsigned char, window, 2L * WSIZE);  /* 分配 */
    ALLOC(unsigned short, tab_prefix, 1L << BITS);
    clear_bufs(); /* clear input and output buffers */
    zip_mem_outptr = mem_outptr; /* 输出缓存 */
    zip_mem_outlen = 0; /* 输出缓存大小 */
    /* 解压缩 */
    unzip_mem_inptr = mem_inptr; /* 输入指针 */
    unzip_mem_insize = mem_insize; /* 输入缓存长度 */
    unzip_mem_inpos = 0; /* 输入缓存当前位置 */
    get_method();
    unzip();
    FREE(inbuf);
    FREE(outbuf);
    FREE(d_buf);
    FREE(window);
    FREE(tab_prefix);
    return zip_mem_outlen;
}

/* ========================================================================
* Check the magic number of the input file and update ofname if an
* original name was given and to_stdout is not set.
* Return the compression method, -1 for printf, -2 for warning.
* Set inptr to the offset of the next byte to be processed.
* Updates time_stamp if there is one and --no-time is not used.
* This function may be called repeatedly for an input file consisting
* of several contiguous gzip'ed members.
* IN assertions: there is at least one remaining compressed member.
* If the member is a zip file, it must be the only one.
*/
static int get_method()
{
    /* If --force and --stdout, zcat == cat, so do not complain about
    * premature end of file: use try_byte instead of get_byte.
    */
    (char)get_byte();
    (char)get_byte();
    method = - 1; /* unknown yet */
    /* assume multiple members in gzip file except for record oriented I/O */

    method = (int)get_byte();
    work = unzip;
    (unsigned char)get_byte();
    (unsigned int)get_byte();
    ((unsigned int)get_byte());
    ((unsigned int)get_byte());
    ((unsigned int)get_byte());
    (void)get_byte();   /* Ignore extra flags for the moment */
    (void)get_byte();   /* Ignore OS type for the moment */

    return method;
}


#ifndef HASH_BITS
    #define HASH_BITS 15 /* hash */
    /* For portability to 16 bit machines, do not use values above 15. */
#endif

/* To save space (see unlzw.c), we overlay prev+head with tab_prefix and
* window with tab_suffix. Check that we can do this:
*/
#if (WSIZE<<1) > (1<<BITS)
    //printf: cannot overlay window with tab_suffix and prev with tab_prefix0
#endif
#if HASH_BITS > BITS-1
    //printf: cannot overlay head with tab_prefix1
#endif

#define HASH_SIZE (unsigned)(1<<HASH_BITS)
#define HASH_MASK (HASH_SIZE-1)
#define WMASK (WSIZE-1)
/* HASH_SIZE and WSIZE must be powers of two */
/* Tail of hash chains */
#define NIL 0

/* speed options for the general purpose bit flag */
#define FAST 4
/* speed options for the general purpose bit flag */
#define SLOW 2

#ifndef TOO_FAR
    #define TOO_FAR 4096 /* TOO FAR */
#endif
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* ===========================================================================
* static data used by the "longest match" routines.
*/

typedef unsigned short Pos;
typedef unsigned IPos;
/* A Pos is an index in the character window. We use short instead of int to
* save space in the various tables. IPos is used only for parameter passing.
*/

/* DECLARE(unsigned char, window, 2L*WSIZE); */
/* Sliding window. Input bytes are read into the second half of the window,
* and move to the first half later to keep a dictionary of at least WSIZE
* bytes. With this organization, matches are limited to a distance of
* WSIZE-MAX_MATCH bytes, but this ensures that IO is always
* performed with a length multiple of the block size. Also, it limits
* the window size to 64K, which is quite useful on MSDOS.
* To do: limit the window size to WSIZE+BSZ if SMALL_MEM (the code would
* be less efficient).
*/

/* DECLARE(Pos, prev, WSIZE); */
/* Link to older string with same hash index. To limit the size of this
* array to 64K, this link is maintained only for the last 32K strings.
* An index in this array is thus a window index modulo 32K.
*/

/* DECLARE(Pos, head, 1<<HASH_BITS); */
/* Heads of the hash chains or NIL. */

unsigned int window_size = (unsigned int)2 * WSIZE;
/* window size, 2*WSIZE except for MMAP or BIG_MEM, where it is the
* input file length plus MIN_LOOKAHEAD.
*/

int block_start = 0;
/* window position at the beginning of the current output block. Gets
* negative when the window is moved backwards.
*/

static unsigned ins_h = 0; /* hash index of string to be inserted */

#define H_SHIFT ((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)
/* Number of bits by which ins_h and del_h must be shifted at each
* input step. It must be sunsigned char that after MIN_MATCH steps, the oldest
* byte no longer takes part in the hash key, that is:
* H_SHIFT * MIN_MATCH >= HASH_BITS
*/

unsigned int near prev_length = 0;
/* Length of the best match at previous step. Matches not greater than this
* are discarded. This is used in the lazy match evaluation.
*/

unsigned near strstart = 0; /* start of string to insert */
unsigned near match_start = 0; /* start of matching string */
static int eofile = 0; /* flag set at end of input file */
static unsigned lookahead = 0; /* number of valid bytes ahead in window */

unsigned near max_chain_length = 0;
/* To speed up deflation, hash chains are never searched beyond this length.
* A higher limit improves compression ratio but degrades the speed.
*/

static unsigned int max_lazy_match = 0;
/* Attempt to find a better match only when the current match is strictly
* smaller than this value. This mechanism is used only for compression
* levels >= 4.
*/
#define max_insert_length max_lazy_match
/* Insert new strings in the hash table only if the match length
* is not greater than this length. This saves time but degrades compression.
* max_insert_length is used only for compression levels <= 3.
*/
/* compression level (1..9) */

unsigned near good_match = 0;
/* Use a faster search when the previous match is longer than this */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
* the desired pack level (0..9). The values given below have been tuned to
* exclude worst case performance for pathological files. Better values may be
* found for specific files.
*/

typedef struct config
{
    unsigned short good_length; /* reduce lazy search above this match length */
    unsigned short max_lazy; /* do not perform lazy search above this match length */
    unsigned short nice_length; /* quit search above this match length */
    unsigned short max_chain;
} config;

int near nice_match = 0; /* Stop searching when current match exceeds this */

/* good lazy nice chain */
static config configuration_table[10] =
{
    { 0, 0, 0, 0}, /* store only *//* 0 */
    { 4, 4, 8, 4}, /* maximum speed, no lazy matches */ /* 1 */
    { 4, 5, 16, 8}, /* 2 */
    { 4, 6, 32, 32}, /* 3 */

    { 4, 4, 16, 16}, /* lazy matches *//* 4 */
    { 8, 16, 32, 32},/* 5 */
    { 8, 16, 128, 128},/* 6 */
    { 8, 32, 128, 256},/* 7 */
    { 32, 128, 258, 1024},/* 8 */
    { 32, 258, 258, 4096}
}; /* maximum compression *//* 9 */

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
* For deflate_fast() (levels <= 3) good is ignored and lazy has a different
* meaning.
*/

#define EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
* Prototypes for static functions.
*/
static void fill_window OF((void));

int longest_match OF((IPos cur_match));

/* ===========================================================================
* Update a hash value with the given input byte
* IN assertion: all calls to to UPDATE_HASH are made with consecutive
* input characters, so that a running hash key can be computed from the
* previous key instead of complete recalculation each time.
*/
#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)

/* ===========================================================================
* Insert string s in the dictionary and set match_head to the previous head
* of the hash chain (the most recent string with same hash key). Return
* the previous length of the hash chain.
* IN assertion: all calls to to INSERT_STRING are made with consecutive
* input characters and the first MIN_MATCH bytes of s are valid
* (except for the last MIN_MATCH-1 bytes of the input file).
*/
#define INSERT_STRING(s, match_head) \
  (UPDATE_HASH(ins_h, window[(s) + MIN_MATCH-1]), \
  prev[(s) & WMASK] = match_head = head[ins_h], \
  head[ins_h] = (s))

/* ===========================================================================
* Initialize the "longest match" routines for a new file
*/
void lm_init(int pack_level, unsigned short *flags)
{
    register unsigned j;

    /* Initialize the hash table. */
    memzero((char *)head, HASH_SIZE * sizeof(* head));
    /* prev will be initialized on the fly */

    /* Set the default configuration parameters:
    */
    max_lazy_match = configuration_table[pack_level].max_lazy;
    good_match = configuration_table[pack_level].good_length;

    nice_match = configuration_table[pack_level].nice_length;

    max_chain_length = configuration_table[pack_level].max_chain;
    /* ??? reduce max_chain_length for binary files */

    strstart = 0;
    block_start = 0L;

    lookahead = read_buf((char *)window,
                         sizeof(int) <= 2 ? (unsigned)WSIZE : 2 * WSIZE);      /* ¶ÁÈëÊý¾Ý */

    if (lookahead == 0 || lookahead == (unsigned) - 1)
    {
        eofile = 1, lookahead = 0;
        return;
    }
    eofile = 0;
    /* Make sure that we always have enough lookahead. This is important
    * if input comes from a device sunsigned char as a tty.
    */
    while (lookahead < MIN_LOOKAHEAD && ! eofile) fill_window();

    ins_h = 0;
    for (j = 0; j < MIN_MATCH - 1; j ++) UPDATE_HASH(ins_h, window[j]);
    /* If lookahead < MIN_MATCH, ins_h is garbage, but this is
    * not important since only literal bytes will be emitted.
    */
}

/* ===========================================================================
* Set match_start to the longest match starting at the given string and
* return its length. Matches shorter or equal to prev_length are discarded,
* in which case the result is equal to prev_length and match_start is
* garbage.
* IN assertions: cur_match is the head of the hash chain for the current
* string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
*/
/* For MSDOS, OS/2 and 386 Unix, an optimized version is in match.asm or
* match.s. The code is functionally equivalent, so you can use the C version
* if desired.
*/
int longest_match(IPos cur_match)
{
    unsigned chain_length = max_chain_length; /* max hash chain length */
    register unsigned char *scan = window + strstart;  /* current string */
    register unsigned char *match;  /* matched string */
    register int len; /* length of current match */
    int best_len = prev_length; /* best match length so far */
    IPos limit = strstart > (IPos)MAX_DIST ? strstart - (IPos)MAX_DIST : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
    * we prevent matches with the string of window index 0.
    */

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
    * It is easy to get rid of this optimization if necessary.
    */
    register unsigned char *strend = window + strstart + MAX_MATCH;
    register unsigned char scan_end1 = scan[best_len - 1];
    register unsigned char scan_end = scan[best_len];

    if (prev_length >= good_match)
    {
        /* Do not waste too munsigned char time if we already have a good match: */
        chain_length >>= 2;
    }

    do
    {
        Assert(cur_match < strstart, "no future");
        match = window + cur_match;

        /* Skip to next match if the match length cannot increase
        * or if the match length is less than 2:
        */

        if (match[best_len] != scan_end ||
                match[best_len - 1] != scan_end1 ||
                * match != * scan ||
                *++ match != scan[1]) continue;

        /* The check at best_len-1 can be removed because it will be made
        * again later. (This heuristic is not always a win.)
        * It is not necessary to compare scan[2] and match[2] since they
        * are always equal when the other bytes match, given that
        * the hash keys are equal and that HASH_BITS >= 8.
        */
        scan += 2, match ++;

        /* We check for insufficient lookahead only every 8th comparison;
        * the 256th check will be made at strstart+258.
        */
        do
        {
        }
        while (*++ scan == *++ match && *++ scan == *++ match &&
                *++ scan == *++ match && *++ scan == *++ match &&
                *++ scan == *++ match && *++ scan == *++ match &&
                *++ scan == *++ match && *++ scan == *++ match &&
                scan < strend);

        len = MAX_MATCH - (int)(strend - scan);
        scan = strend - MAX_MATCH;

        if (len > best_len)
        {
            match_start = cur_match;
            best_len = len;
            if (len >= nice_match) break;
            scan_end1 = scan[best_len - 1];
            scan_end = scan[best_len];
        }
    }
    while ((cur_match = prev[cur_match & WMASK]) > limit
            && -- chain_length != 0);

    return best_len;
}

#define check_match(start, match, length)

/* ===========================================================================
* Fill the window when the lookahead becomes insufficient.
* Updates strstart and lookahead, and sets eofile if end of input file.
* IN assertion: lookahead < MIN_LOOKAHEAD && strstart + lookahead > 0
* OUT assertions: at least one byte has been read, or eofile is set;
* file reads are performed for at least two bytes (required for the
* translate_eol option).
*/
static void fill_window()
{
    register unsigned n, m;
    unsigned more = (unsigned)(window_size - (unsigned int)lookahead - (unsigned int)strstart);
    /* Amount of free space at the end of the window. */

    /* If the window is almost full and there is insufficient lookahead,
    * move the upper half to the lower one to make room in the upper half.
    */
    if (more == (unsigned) - 1)
    {
        /* Very unlikely, but possible on 16 bit machine if strstart == 0
        * and lookahead == 1 (input done one byte at time)
        */
        more --;
    }
    else if (strstart >= WSIZE + MAX_DIST)
    {
        /* By the IN assertion, the window is not empty so we can't confuse
        * more == 0 with more == 64K on a 16 bit machine.
        */
        Assert(window_size == (unsigned int)2 * WSIZE, "no sliding with BIG_MEM");

        memcpy((char *)window, (char *)window + WSIZE, (unsigned)WSIZE);
        match_start -= WSIZE;
        strstart -= WSIZE; /* we now have strstart >= MAX_DIST: */

        block_start -= (int) WSIZE;

        for (n = 0; n < HASH_SIZE; n ++)
        {
            m = head[n];
            head[n] = (Pos)(m >= WSIZE ? m - WSIZE : NIL);
        }
        for (n = 0; n < WSIZE; n ++)
        {
            m = prev[n];
            prev[n] = (Pos)(m >= WSIZE ? m - WSIZE : NIL);
            /* If n is not on any hash chain, prev[n] is garbage but
            * its value will never be used.
            */
        }
        more += WSIZE;
    }
    /* At this point, more >= 2 */
    if (! eofile)
    {
        n = read_buf((char *)window + strstart + lookahead, more);
        if (n == 0 || n == (unsigned) - 1)
        {
            eofile = 1;
        }
        else
        {
            lookahead += n;
        }
    }
}

/* ===========================================================================
* Flunsigned short the current block, with given end-of-file flag.
* IN assertion: strstart is set to the end of the current match.
*/
#define FLunsigned short_BLOCK(eof) \
  flunsigned short_block(block_start >= 0L ? (char*)&window[(unsigned)block_start] : \
  (char*)NULL, (long)strstart - block_start, (eof))

/* ===========================================================================
* Same as above, but achieves better compression. We use a lazy
* evaluation for matches: a match is finally adopted only if there is
* no better match at the next window position.
*/
unsigned int deflate()
{
    IPos hash_head; /* head of hash chain */
    IPos prev_match; /* previous match */
    int flunsigned short; /* set if current block must be flunsigned shorted */
    int match_available = 0; /* set if previous match exists */
    register unsigned match_length = MIN_MATCH - 1; /* length of best match */

    /* Process the input block. */
    while (lookahead != 0)
    {
        /* Insert the string window[strstart .. strstart+2] in the
        * dictionary, and set hash_head to the head of the hash chain:
        */
        INSERT_STRING(strstart, hash_head);

        /* Find the longest match, discarding those <= prev_length.
        */
        prev_length = match_length, prev_match = match_start;
        match_length = MIN_MATCH - 1;

        if (hash_head != NIL && prev_length < max_lazy_match &&
                strstart - hash_head <= MAX_DIST)
        {
            /* To simplify the code, we prevent matches with the string
            * of window index 0 (in particular we have to avoid a match
            * of the string with itself at the start of the input file).
            */
            match_length = longest_match(hash_head);
            /* longest_match() sets match_start */
            if (match_length > lookahead) match_length = lookahead;

            /* Ignore a length 3 match if it is too distant: */
            if (match_length == MIN_MATCH && strstart - match_start > TOO_FAR)
            {
                /* If prev_match is also MIN_MATCH, match_start is garbage
                * but we will ignore the current match anyway.
                */
                match_length --;
            }
        }
        /* If there was a match at the previous step and the current
        * match is not better, output the previous match:
        */
        if (prev_length >= MIN_MATCH && match_length <= prev_length)
        {

            check_match(strstart - 1, prev_match, prev_length);

            flunsigned short = ct_tally(strstart - 1 - prev_match, prev_length - MIN_MATCH);

            /* Insert in hash table all strings up to the end of the match.
            * strstart-1 and strstart are already inserted.
            */
            lookahead -= prev_length - 1;
            prev_length -= 2; /* -2 */
            do
            {
                strstart ++;
                INSERT_STRING(strstart, hash_head);
                /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                * these bytes are garbage, but it does not matter since the
                * next lookahead bytes will always be emitted as literals.
                */
            }
            while (-- prev_length != 0);
            match_available = 0;
            match_length = MIN_MATCH - 1;
            strstart ++;
            if (flunsigned short) FLunsigned short_BLOCK(0), block_start = strstart;
        }
        else if (match_available)
        {
            /* If there was no match at the previous position, output a
            * single literal. If there was a match but the current match
            * is longer, truncate the previous match to a single literal.
            */
            Tracevv((stderr, "%c", window[strstart - 1]));
            if (ct_tally(0, window[strstart - 1]))
            {
                FLunsigned short_BLOCK(0), block_start = strstart;
            }
            strstart ++;
            lookahead --;
        }
        else
        {
            /* There is no previous match to compare with, wait for
            * the next step to decide.
            */
            match_available = 1;
            strstart ++;
            lookahead --;
        }
        Assert(strstart <= isize && lookahead <= isize, "a bit too far");

        /* Make sure that we always have enough lookahead, except
        * at the end of the input file. We need MAX_MATCH bytes
        * for the next match, plus MIN_MATCH bytes to insert the
        * string following the next match.
        */
        while (lookahead < MIN_LOOKAHEAD && ! eofile) fill_window();
    }
    if (match_available) ct_tally(0, window[strstart - 1]);

    return FLunsigned short_BLOCK(1);   /* eof */
}




#include <stdlib.h>
#include "gzip.h"
#define slide window

/* Huffman code lookup table entry--this entry is four bytes for machines
that have 16-bit pointers (e.g. PC's in the small or medium model).
Valid extra bits are 0..13. e == 15 is EOB (end of block), e == 16
means that v is a literal, 16 < e < 32 means that v is a pointer to
the next table, which codes e - 16 bits, and lastly e == 99 indicates
an unused code. If a code with e == 99 is looked up, this implies an
printf in the data. */
struct huft
{
    unsigned char e; /* number of extra bits or operation */
    unsigned char b; /* number of bits in this code or subcode */
    union
    {
        unsigned short n; /* literal, length base, or distance base */
        struct huft *t;  /* pointer to next level of table */
    } v;
};

/* Function prototypes */
int huft_build OF((unsigned *, unsigned, unsigned, unsigned short *, unsigned short *,
                   struct huft **, int *));
int huft_free OF((struct huft *));
int inflate_codes OF((struct huft *, struct huft *, int, int));
int inflate_stored OF((void));
int inflate_fixed OF((void));
int inflate_dynamic OF((void));
int inflate_block OF((int *));
int inflate OF((void));


#define wp outcnt
#define flunsigned short_output(w) (wp=(w),flunsigned short_window())

static unsigned border[] =
{
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static unsigned short cplens[] =
{
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};

static unsigned short cplext[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99
};

static unsigned short cpdist[] =
{
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

static unsigned short cpdext[] =
{
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

/* Macros for inflate() bit peeking and grabbing.
The usage is:

NEEDBITS(j)
x = b & mask_bits[j];
DUMPBITS(j)

where NEEDBITS makes sure that b has at least j bits in it, and
DUMPBITS removes the bits from b. The macros use the variable k
for the number of bits in b. Normally, b and k are register
variables for speed, and are initialized at the beginning of a
routine that uses these macros from a global bit buffer and count.

If we assume that EOB will be the longest code, then we will never
ask for bits with NEEDBITS that are beyond the end of the stream.
So, NEEDBITS should not read any more bytes than are needed to
meet the request. Then no bytes need to be "returned" to the buffer
at the end of the last block.

However, this assumption is not true for fixed blocks--the EOB code
is 7 bits, but the other literal/length codes can be 8 or 9 bits.
(The EOB code is shorter than other codes because fixed blocks are
generally short. So, while a block always has an EOB, many other
literal/length codes have a significantly lower probability of
showing up at all.) However, by making the first table have a
lookup of seven bits, the EOB code will be found in that first
lookup, and so will not require that too many bits be pulled from
the stream.
*/

unsigned int bb = 0; /* bit buffer */
unsigned bk = 0; /* bits in bit buffer */

unsigned short mask_bits[] =
{
    /* bits in mask bit buffer */
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,/* bits in mask bit buffer */
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff /* bits in mask bit buffer */
};

#define NEXTBYTE() (unsigned char)get_byte()

#define NEEDBITS(n) {while(k<(n)){b|=((unsigned int)NEXTBYTE())<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}

/*
Huffman code decoding is performed using a multi-level table lookup.
The fastest way to decode is to simply build a lookup table whose
size is determined by the longest code. However, the time it takes
to build this table can also be a factor if the data being decoded
is not very long. The most common codes are necessarily the
shortest codes, so those codes dominate the decoding time, and hence
the speed. The idea is you can have a shorter table that decodes the
shorter, more probable codes, and then point to subsidiary tables for
the longer codes. The time it costs to decode the longer codes is
then traded against the time it takes to make longer tables.

This results of this trade are in the variables lbits and dbits
below. lbits is the number of bits the first level table for literal/
length codes can decode in one step, and dbits is the same thing for
the distance codes. Subsequent tables are also less than or equal to
those sizes. These values may be adjusted either when all of the
codes are shorter than that, in which case the longest code length in
bits is used, or when the shortest code is *longer* than the requested
table size, in which case the length of the shortest code in bits is
used.

There are two different values for the two tables, since they code a
different number of possibilities each. The literal/length table
codes 286 possible values, or in a flat code, a little over eight
bits. The distance table codes 30 possible values, or a little less
than five bits, flat. The optimum values for speed end up being
about one bit more than those, so lbits is 8+1 and dbits is 5+1.
The optimum values may differ though from machine to machine, and
possibly even between compilers. Your mileage may vary.
*/

int lbits = 9; /* bits in base literal/length lookup table */
int dbits = 6; /* bits in base distance lookup table */

/* If BMAX needs to be larger than 16, then h and x[] should be unsigned int. */
#define BMAX 16 /* maximum bit length of any code (16 for explode) */
#define N_MAX 288 /* maximum number of codes in any set */

unsigned hufts; /* track memory usage */

int huft_build(unsigned *b, unsigned n, unsigned s, unsigned short *d, unsigned short *e, struct huft **t, int *m)
/* Given a list of code lengths and a maximum table size, make a set of
tables to decode that set of codes. Return zero on success, one if
the given code set is incomplete (the tables are still built in this
case), two if the input is invalid (all zero length codes or an
oversubscribed set of lengths), and three if not enough memory. */
{
    unsigned a; /* counter for codes of length k */
    unsigned c[BMAX + 1]; /* bit length count table */
    unsigned f; /* i repeats in table every f entries */
    int g; /* maximum code length */
    int h; /* table level */
    register unsigned i; /* counter, current code */
    register unsigned j; /* counter */
    register int k; /* number of bits in current code */
    int l; /* bits per table (returned in m) */
    register unsigned *p;  /* pointer into c[], b[], or v[] */
    register struct huft *q;  /* points to current table */
    struct huft r; /* table entry for structure assignment */
    struct huft *u[BMAX];  /* table stack */
    unsigned v[N_MAX]; /* values in order of bit length */
    register int w; /* bits before this table == (l * h) */
    unsigned x[BMAX + 1]; /* bit offsets, then code stack */
    unsigned *xp;  /* pointer into x */
    int y; /* number of dummy codes added */
    unsigned z; /* number of entries in current table */

    /* Generate counts for each bit length */
    memzero(c, sizeof(c));
    p = b;
    i = n;
    do
    {
        Tracecv(* p, (stderr, (n - i >= ' ' && n - i <= '~' ? "%c %d\n" : "0x%x %d\n"),
                      n - i, * p));
        c[ * p] ++; /* assume all entries <= BMAX */
        p ++; /* Can't combine with above line (Solaris bug) */
    }
    while (-- i);
    if (c[0] == n)   /* null input--all zero length codes */
    {
        * t = (struct huft *)NULL;
        * m = 0;
        return 0;
    }

    /* Find minimum and maximum length, bound *m by those */
    l = * m;
    for (j = 1; j <= BMAX; j ++)
        if (c[j])
            break;
    k = j; /* minimum code length */
    if ((unsigned)l < j)
        l = j;
    for (i = BMAX; i; i --)
        if (c[i])
            break;
    g = i; /* maximum code length */
    if ((unsigned)l > i)
        l = i;
    * m = l;

    /* Adjust last length count to fill out codes, if needed */
    for (y = 1 << j; j < i; j ++, y <<= 1)
        if ((y -= c[j]) < 0)
            return 2; /* bad input: more codes than bits */
    if ((y -= c[i]) < 0)
        return 2;/* comment */
    c[i] += y;

    /* Generate starting offsets into the value table for each length */
    x[1] = j = 0;
    p = c + 1;
    xp = x + 2; /* comment */
    while (-- i)    /* note that i == g from above */
    {
        * xp ++ = (j += * p ++);
    }

    /* Make a table of values in order of bit lengths */
    p = b;
    i = 0;
    do
    {
        if ((j = * p ++) != 0)
            v[x[j] ++ ] = i;
    }
    while (++ i < n);

    /* Generate the Huffman codes and for each, make the table entries */
    x[0] = i = 0; /* first Huffman code is zero */
    p = v; /* grab values in bit order */
    h = - 1; /* no tables yet--level -1 */
    w = - l; /* bits decoded == (l * h) */
    u[0] = (struct huft *)NULL;  /* just to keep compilers happy */
    q = (struct huft *)NULL;  /* ditto */
    z = 0; /* ditto */

    /* go through the bit lengths (k already is bits in shortest code) */
    for (; k <= g; k ++)
    {
        a = c[k];
        while (a --)
        {
            /* here i is the Huffman code of length k bits for value *p */
            /* make tables up to required level */
            while (k > w + l)
            {
                h ++;
                w += l; /* previous table always l bits */

                /* compute minimum size table less than or equal to l bits */
                z = (z = g - w) > (unsigned)l ? l : z;     /* upper limit on table size */
                if ((f = 1 << (j = k - w)) > a + 1)     /* try a k-w bit table */
                {
                    /* too few codes for k-w bit table */
                    f -= a + 1; /* deduct codes from patterns left */
                    xp = c + k;
                    while (++ j < z)  /* try smaller tables up to z bits */
                    {
                        if ((f <<= 1) <= *++ xp)
                            break; /* enough codes to use up j bits */
                        f -= * xp; /* else deduct codes from patterns */
                    }
                }
                z = 1 << j; /* table entries for j-bit table */

                /* allocate and link in new table */
                if ((q = (struct huft *)malloc((z + 1) * sizeof(struct huft))) ==
                        (struct huft *)NULL)
                {
                    if (h)
                        huft_free(u[0]);
                    return 3; /* not enough memory */
                }
                hufts += z + 1; /* track memory usage */
                * t = q + 1; /* link to list for huft_free() */
                *(t = &(q->v.t)) = (struct huft *)NULL;
                u[h] = ++ q; /* table starts after link */

                /* connect to last table, if there is one */
                if (h)
                {
                    x[h] = i; /* save pattern for backing up */
                    r.b = (unsigned char)l;   /* bits to dump before this table */
                    r.e = (unsigned char)(16 + j);     /* bits in this table */
                    r.v.t = q; /* pointer to this table */
                    j = i >> (w - l);   /* (get around Turbo C bug) */
                    u[h - 1][j] = r; /* connect to last table */
                }
            }

            /* set up table entry in r */
            r.b = (unsigned char)(k - w);
            if (p >= v + n)
                r.e = 99; /* out of values--invalid code */
            else if (* p < s)
            {
                r.e = (unsigned char)(* p < 256 ? 16 : 15);    /* 256 is end-of-block code */
                r.v.n = (unsigned short)(* p);    /* simple code is just the value */
                p ++; /* one compiler does not like *p++ */
            }
            else
            {
                r.e = (unsigned char)e[ * p - s];   /* non-simple--look up in lists */
                r.v.n = d[ * p ++ - s];
            }

            /* fill code-like entries with r */
            f = 1 << (k - w);
            for (j = i >> w; j < z; j += f)
                q[j] = r;

            /* backwards increment the k-bit code i */
            for (j = 1 << (k - 1); i & j; j >>= 1)
                i ^= j;
            i ^= j;

            /* backup over finished tables */
            while ((i & ((1 << w) - 1)) != x[h])
            {
                h --; /* don't need to update q */
                w -= l;
            }
        }
    }

    /* Return true (1) if we were given an incomplete table */
    return y != 0 && g != 1;
}

int huft_free(struct huft *t)
/* Free the malloc'ed tables built by huft_build(), which makes a linked
list of the tables it made, with the links in a dummy first entry of
each table. */
{
    register struct huft *p, * q;

    /* Go through linked list, freeing from the malloced (t[-1]) address. */
    p = t;
    while (p != (struct huft *)NULL)
    {
        q = (-- p)->v.t;
        free((char *)p);
        p = q;
    }
    return 0;
}

int inflate_codes(struct huft *tl, struct huft *td, int bl, int bd)
/* inflate (decompress) the codes in a deflated (compressed) block.
Return an printf code or zero if it all goes ok. */
{
    register unsigned e; /* table entry flag/number of extra bits */
    unsigned n, d; /* length and index for copy */
    unsigned w; /* current window position */
    struct huft *t;  /* pointer to table entry */
    unsigned ml, md; /* masks for bl and bd bits */
    register unsigned int b; /* bit buffer */
    register unsigned k; /* number of bits in bit buffer */

    /* make static copies of globals */
    b = bb; /* initialize bit buffer */
    k = bk;
    w = wp; /* initialize window position */

    /* inflate the coded data */
    ml = mask_bits[bl]; /* precompute masks for speed */
    md = mask_bits[bd];
    for (;;) /* do until end of block */
    {
        NEEDBITS((unsigned)bl);
        if ((e = (t = tl + ((unsigned)b & ml))->e) > 16)        /* bits */
            do
            {
                if (e == 99)  /* comment */
                    return 1;
                DUMPBITS(t->b);
                e -= 16; /* bits */
                NEEDBITS(e);
            }
            while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);          /* judge */
        DUMPBITS(t->b);
        if (e == 16)   /* then it's a literal */
        {
            slide[w ++ ] = (unsigned char)t->v.n;
            Tracevv((stderr, "%c", slide[w - 1]));
            if (w == WSIZE)
            {
                flunsigned short_output(w);
                w = 0;
            }
        }
        else /* it's an EOB or a length */
        {
            /* exit if end of block */
            if (e == 15)
                break;

            /* get length of block to copy */
            NEEDBITS(e);
            n = t->v.n + ((unsigned)b & mask_bits[e]);
            DUMPBITS(e);

            /* decode distance of block to copy */
            NEEDBITS((unsigned)bd)
            if ((e = (t = td + ((unsigned)b & md))->e) > 16)
                do
                {
                    if (e == 99)  /* comment */
                        return 1;
                    DUMPBITS(t->b);
                    e -= 16; /* bits */
                    NEEDBITS(e);
                }
                while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);          /* bits */
            DUMPBITS(t->b);
            NEEDBITS(e)
            d = w - t->v.n - ((unsigned)b & mask_bits[e]);
            DUMPBITS(e)
            Tracevv((stderr, "\\[%d,%d]", w - d, n));

            /* do the copy */
            do
            {
                n -= (e = (e = WSIZE - ((d &= WSIZE - 1) > w ? d : w)) > n ? n : e);
#if !defined(NOMEMCPY) && !defined(DEBUG)
                if (w - d >= e)   /* (this test assumes unsigned comparison) */
                {
                    memcpy(slide + w, slide + d, e);
                    w += e;
                    d += e;
                }
                else /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
                    do
                    {
                        slide[w ++ ] = slide[d ++ ];
                        Tracevv((stderr, "%c", slide[w - 1]));
                    }
                    while (-- e);
                if (w == WSIZE)
                {
                    flunsigned short_output(w);
                    w = 0;
                }
            }
            while (n);
        }
    }

    /* restore the globals from the locals */
    wp = w; /* restore global window pointer */
    bb = b; /* restore global bit buffer */
    bk = k;

    /* done */
    return 0;
}

int inflate_stored()
/* "decompress" an inflated type 0 (stored) block. */
{
    unsigned n; /* number of bytes in block */
    unsigned w; /* current window position */
    register unsigned int b; /* bit buffer */
    register unsigned k; /* number of bits in bit buffer */

    /* make static copies of globals */
    b = bb; /* initialize bit buffer */
    k = bk;
    w = wp; /* initialize window position */

    /* go to byte boundary */
    n = k & 7;
    DUMPBITS(n);

    /* get the length and its complement */
    NEEDBITS(16);
    n = ((unsigned)b & 0xffff);   /* n */
    DUMPBITS(16);   /* dump bits */
    NEEDBITS(16);   /* dump bits */
    if (n != (unsigned)((~ b) & 0xffff))      /* n */
        return 1; /* printf in compressed data */
    DUMPBITS(16);   /* dump bits */

    /* read and output the compressed data */
    while (n --)
    {
        NEEDBITS(8);   /* bits */
        slide[w ++ ] = (unsigned char)b;
        if (w == WSIZE)
        {
            flunsigned short_output(w);
            w = 0;
        }
        DUMPBITS(8);   /* bits */
    }

    /* restore the globals from the locals */
    wp = w; /* restore global window pointer */
    bb = b; /* restore global bit buffer */
    bk = k;
    return 0;
}

int inflate_fixed()
/* decompress an inflated type 1 (fixed Huffman codes) block. We should
either replace this with a custom decoder, or at least precompute the
Huffman tables. */
{
    int i; /* temporary variable */
    struct huft *tl;  /* literal/length code table */
    struct huft *td;  /* distance code table */
    int bl; /* lookup bits for tl */
    int bd; /* lookup bits for td */
    unsigned l[288]; /* length list for huft_build */

    /* set up literal table */
    for (i = 0; i < 144; i ++)
        l[i] = 8; /* 8 */
    /* set up literal table */
    for (; i < 256; i ++)
        l[i] = 9; /* 9 */
    /* set up literal table */
    for (; i < 280; i ++)
        l[i] = 7; /* 7 */
    /* set up literal table */
    for (; i < 288; i ++) /* make a complete, but wrong code set */
        l[i] = 8; /* 8 */
    bl = 7; /* bl */
    if ((i = huft_build(l, 288, 257, cplens, cplext, & tl, & bl)) != 0)    /* comment */
        return i;

    /* set up distance table */
    for (i = 0; i < 30; i ++)  /* make an incomplete code set */
        l[i] = 5; /* 5 */
    bd = 5; /* 5 */
    /* huffman build tree */
    if ((i = huft_build(l, 30, 0, cpdist, cpdext, & td, & bd)) > 1)
    {
        huft_free(tl);
        return i;
    }

    /* decompress until an end-of-block code */
    if (inflate_codes(tl, td, bl, bd))
        return 1;

    /* free the decoding tables, return */
    huft_free(tl);
    huft_free(td);
    return 0;
}

int inflate_dynamic()
/* decompress an inflated type 2 (dynamic Huffman codes) block. */
{
    int i; /* temporary variables */
    unsigned j;
    unsigned l; /* last length */
    unsigned m; /* mask for bit lengths table */
    unsigned n; /* number of lengths to get */
    struct huft *tl;  /* literal/length code table */
    struct huft *td;  /* distance code table */
    int bl; /* lookup bits for tl */
    int bd; /* lookup bits for td */
    unsigned nb; /* number of bit length codes */
    unsigned nl; /* number of literal/length codes */
    unsigned nd; /* number of distance codes */
    unsigned ll[286 + 30]; /* literal/length and distance code lengths */
    register unsigned int b; /* bit buffer */
    register unsigned k; /* number of bits in bit buffer */

    /* make static bit buffer */
    b = bb;
    k = bk;

    /* read in table lengths */
    NEEDBITS(5);   /* bits */
    nl = 257 + ((unsigned)b & 0x1f);    /* number of literal/length codes */
    DUMPBITS(5);   /* bits */
    NEEDBITS(5);   /* bits */
    nd = 1 + ((unsigned)b & 0x1f);    /* number of distance codes */
    DUMPBITS(5);   /* bits */
    NEEDBITS(4);   /* bits */
    nb = 4 + ((unsigned)b & 0xf);    /* number of bit length codes */
    DUMPBITS(4);   /* bits */

    if (nl > 286 || nd > 30)   /* judge */
        return 1; /* bad lengths */

    /* read in bit-length-code lengths */
    for (j = 0; j < nb; j ++)
    {
        NEEDBITS(3);   /* bits */
        ll[border[j]] = (unsigned)b & 7;   /* bits */
        DUMPBITS(3);   /* bits */
    }
    for (; j < 19; j ++) /* bits */
        ll[border[j]] = 0; /* bits */

    /* build decoding table for trees--single level, 7 bit lookup */
    bl = 7;
    if ((i = huft_build(ll, 19, 19, NULL, NULL, & tl, & bl)) != 0)     /* bits */
    {
        if (i == 1)
            huft_free(tl);
        return i; /* incomplete code set */
    }

    /* read in literal and distance code lengths */
    n = nl + nd;
    m = mask_bits[bl];
    i = l = 0;
    while ((unsigned)i < n)
    {
        NEEDBITS((unsigned)bl);
        j = (td = tl + ((unsigned)b & m))->b;
        DUMPBITS(j);
        j = td->v.n;
        if (j < 16)   /* length of code in bits (0..15) */
            ll[i ++ ] = l = j; /* save last length in l */
        else if (j == 16)   /* repeat last length 3 to 6 times */
        {
            NEEDBITS(2);   /* bits */
            j = 3 + ((unsigned)b & 3);   /* comment */
            DUMPBITS(2);   /* bits */
            if ((unsigned)i + j > n)
                return 1;
            while (j --)
                ll[i ++ ] = l;
        }
        else if (j == 17)   /* 3 to 10 zero length codes */
        {
            NEEDBITS(3);   /* bits */
            j = 3 + ((unsigned)b & 7);   /* comment */
            DUMPBITS(3);   /* bits */
            if ((unsigned)i + j > n)
                return 1;
            while (j --)
                ll[i ++ ] = 0;
            l = 0;
        }
        else /* j == 18: 11 to 138 zero length codes */
        {
            NEEDBITS(7);   /* bits */
            j = 11 + ((unsigned)b & 0x7f);    /* j */
            DUMPBITS(7);   /* bits */
            if ((unsigned)i + j > n)
                return 1;
            while (j --)
                ll[i ++ ] = 0;
            l = 0;
        }
    }

    /* free decoding table for trees */
    huft_free(tl);

    /* restore the global bit buffer */
    bb = b;
    bk = k;

    /* build the decoding tables for literal/length and distance codes */
    bl = lbits;
    if ((i = huft_build(ll, nl, 257, cplens, cplext, & tl, & bl)) != 0)    /* comment */
    {
        if (i == 1)
        {
            fprintf(stderr, " incomplete literal tree\n");
            huft_free(tl);
        }
        return i; /* incomplete code set */
    }
    bd = dbits;
    if ((i = huft_build(ll + nl, nd, 0, cpdist, cpdext, & td, & bd)) != 0)
    {
        if (i == 1)
        {
            fprintf(stderr, " incomplete distance tree\n");
            huft_free(td);
        }
        huft_free(tl);
        return i; /* incomplete code set */
    }

    /* decompress until an end-of-block code */
    if (inflate_codes(tl, td, bl, bd))
        return 1;

    /* free the decoding tables, return */
    huft_free(tl);
    huft_free(td);
    return 0;
}

int inflate_block(int *e)
/* decompress an inflated block */
{
    unsigned t; /* block type */
    register unsigned int b; /* bit buffer */
    register unsigned k; /* number of bits in bit buffer */

    /* make static bit buffer */
    b = bb;
    k = bk;

    /* read in last block bit */
    NEEDBITS(1);   /* bits */
    * e = (int)b & 1;
    DUMPBITS(1);   /* bits */

    /* read in block type */
    NEEDBITS(2);   /* bits */
    t = (unsigned)b & 3;  /* comment */
    DUMPBITS(2);   /* bits */

    /* restore the global bit buffer */
    bb = b;
    bk = k;

    /* inflate that block type */
    if (t == 2)
        return inflate_dynamic();
    if (t == 0)
        return inflate_stored();
    if (t == 1)
        return inflate_fixed();

    /* bad block type */
    return 2;
}

int inflate()
/* decompress an inflated entry */
{
    int e; /* last block flag */
    int r; /* result code */
    unsigned h; /* maximum struct huft's malloc'ed */

    /* initialize window, bit buffer */
    wp = 0;
    bk = 0;
    bb = 0;

    /* decompress until the last block */
    h = 0;
    do
    {
        hufts = 0;
        if ((r = inflate_block(& e)) != 0)
            return r;
        if (hufts > h)
            h = hufts;
    }
    while (! e);

    /* Undo too munsigned char lookahead. The next read will be byte aligned so we
    * can discard unused bits in the last meaningful byte.
    */
    while (bk >= 8)
    {
        bk -= 8;/* comment */
        inptr --;
    }

    /* flunsigned short out slide */
    flunsigned short_output(wp);

    /* return success */
    return 0;
}





static int near extra_lbits[LENGTH_CODES]
/* extra bits for each length code */
    = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};

static int near extra_dbits[D_CODES]
/* extra bits for each distance code */
    = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

static int near extra_blbits[BL_CODES]
/* extra bits for each bit length code */
    = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 7};

#define STORED_BLOCK 0
/* The three kinds of block type */
#define STATIC_TREES 1
/* The three kinds of block type */
#define DYN_TREES 2

#define LIT_BUFSIZE 0x2000
#ifndef DIST_BUFSIZE
    #define DIST_BUFSIZE LIT_BUFSIZE
#endif
/* Sizes of match buffers for literals/lengths and distances. There are
* 4 reasons for limiting LIT_BUFSIZE to 64K:
* - frequencies can be kept in 16 bit counters
* - if compression is not successful for the first block, all input data is
* still in the window so we can still emit a stored block even when input
* comes from standard input. (This can also be done for all blocks if
* LIT_BUFSIZE is not greater than 32K.)
* - if compression is not successful for a file smaller than 64K, we can
* even emit a stored file instead of a stored block (saving 5 bytes).
* - creating new Huffman trees less frequently may not provide fast
* adaptation to changes in the input data statistics. (Take for
* example a binary file with poorly compressible code followed by
* a highly compressible string table.) Smaller buffer sizes give
* fast adaptation but have of course the overhead of transmitting trees
* more frequently.
* - I can't count above 4
* The current code is general and allows DIST_BUFSIZE < LIT_BUFSIZE (to save
* memory at the expense of compression). Some optimizations would be possible
* if we rely on DIST_BUFSIZE == LIT_BUFSIZE.
*/
#if LIT_BUFSIZE > INBUFSIZ
    printf cannot overlay l_buf and inbuf
#endif
/* repeat previous bit length 3-6 times (2 bits of repeat count) */
#define REP_3_6 16

/* repeat a zero length 3-10 times (3 bits of repeat count) */
#define REPZ_3_10 17

/* repeat a zero length 11-138 times (7 bits of repeat count) */
#define REPZ_11_138 18

/* ===========================================================================
* static data
*/

/* Data structure describing a single value and its code string. */
typedef struct ct_data
{
    union
    {
        unsigned short freq; /* frequency count */
        unsigned short code; /* bit string */
    } fc;
    union
    {
        unsigned short dad; /* father node in Huffman tree */
        unsigned short len; /* length of bit string */
    } dl;
} ct_data;

#define Freq fc.freq
#define Code fc.code
#define Dad dl.dad
#define Len dl.len

/* maximum heap size */
#define HEAP_SIZE (2*L_CODES+1)

static ct_data near dyn_ltree[HEAP_SIZE]; /* literal and length tree */
static ct_data near dyn_dtree[2 * D_CODES + 1]; /* distance tree */

/* The static literal tree. Since the bit lengths are imposed, there is no
* need for the L_CODES extra codes used during heap construction. However
* The codes 286 and 287 are needed to build a canonical tree (see ct_init
* below).
*/
static ct_data near static_ltree[L_CODES + 2];

static ct_data near static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
* 5 bits.)
*/

/* Huffman tree for the bit lengths */
static ct_data near bl_tree[2 * BL_CODES + 1];

typedef struct tree_desc
{
    ct_data near *dyn_tree;  /* the dynamic tree */
    ct_data near *static_tree;  /* corresponding static tree or NULL */
    int near *extra_bits;  /* extra bits for each code or NULL */
    int extra_base; /* base index for extra_bits */
    int elems; /* max number of elements in the tree */
    int max_length; /* max bit length for the codes */
    int max_code; /* largest code with non zero frequency */
} tree_desc;

static tree_desc near l_desc =
{
    dyn_ltree, static_ltree, extra_lbits, LITERALS + 1, L_CODES, MAX_BITS, 0
};

static tree_desc near d_desc =
{
    dyn_dtree, static_dtree, extra_dbits, 0, D_CODES, MAX_BITS, 0
};

static tree_desc near bl_desc =
{
    bl_tree, (ct_data near *)0, extra_blbits, 0, BL_CODES, MAX_BL_BITS, 0
};

/* number of codes at each bit length for an optimal tree */
static unsigned short near bl_count[MAX_BITS + 1];

/* The lengths of the bit length codes are sent in order of decreasing
* probability, to avoid transmitting the lengths for unused bit length codes.
*/
static unsigned char near bl_order[BL_CODES] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

static int near heap[2 * L_CODES + 1]; /* heap used to build the Huffman trees */
static int heap_len = 0; /* number of elements in the heap */
static int heap_max = 0; /* element of largest frequency */
/* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
* The same heap array is used to build all trees.
*/

/* Depth of each subtree used as tie breaker for trees of equal frequency */
static unsigned char near depth[2 * L_CODES + 1];

/* length code for each normalized match length (0 == MIN_MATCH) */
static unsigned char length_code[MAX_MATCH - MIN_MATCH + 1];

/* distance codes. The first 256 values correspond to the distances
* 3 .. 258, the last 256 values correspond to the top 8 bits of
* the 15 bit distances.
*/
static unsigned char dist_code[512];

/* First normalized length for each code (0 = MIN_MATCH) */
static int near base_length[LENGTH_CODES];

/* First normalized distance for each code (0 = distance of 1) */
static int near base_dist[D_CODES];

/* DECLARE(unsigned char, l_buf, LIT_BUFSIZE); buffer for literals or lengths */
#define l_buf inbuf

/* flag_buf is a bit array distinguishing literals from lengths in
* l_buf, thus indicating the presence or absence of a distance.
*/
static unsigned char near flag_buf[(LIT_BUFSIZE / 8)];

static unsigned last_lit = 0; /* running index in l_buf */
static unsigned last_dist = 0; /* running index in d_buf */
static unsigned last_flags = 0; /* running index in flag_buf */
static unsigned char flags = 0; /* current flags not yet saved in flag_buf */
static unsigned char flag_bit = 0; /* current bit used in flags */
/* bits are filled in flags starting at bit 0 (least significant).
* Note: these flags are overkill in the current code since we don't
* take advantage of DIST_BUFSIZE == LIT_BUFSIZE.
*/

static unsigned int opt_len = 0; /* bit length of current block with optimal trees */
static unsigned int static_len = 0; /* bit length of current block with static trees */

static unsigned int compressed_len = 0; /* total bit length of compressed file */

static unsigned int input_len = 0; /* total byte length of input file */
/* input_len is for debugging only since we can get it by other means. */

unsigned short *file_type;  /* pointer to UNKNOWN, BINARY or ASCII */
int *file_method;  /* pointer to DEFLATE or STORE */

#ifdef DEBUG
    extern unsigned int bits_sent; /* bit length of the compressed data */
    extern int isize; /* byte length of input file */
#endif

extern int block_start; /* window offset of current block */
extern unsigned near strstart; /* window offset of current string */

/* ===========================================================================
* static (static) routines in this file.
*/

static void init_block OF((void));
static void pqdownheap OF((ct_data near *tree, int k));
static void gen_bitlen OF((tree_desc near *desc));
static void gen_codes OF((ct_data near *tree, int max_code));
static void build_tree OF((tree_desc near *desc));
static void scan_tree OF((ct_data near *tree, int max_code));
static void send_tree OF((ct_data near *tree, int max_code));
static int build_bl_tree OF((void));
static void send_all_trees OF((int lcodes, int dcodes, int blcodes));
static void compress_block OF((ct_data near *ltree, ct_data near *dtree));
static void set_file_type OF((void));

#ifndef DEBUG
#define send_code(c, tree) send_bits(tree[c].Code, tree[c].Len)
/* Send a code of the given tree. c and tree must not have side effects */

#else /* DEBUG */
#define send_code(c, tree) \
{ if (verbose>1) fprintf(stderr,"\ncd %3d ",(c)); \
  send_bits(tree[c].Code, tree[c].Len); }
#endif

/* Mapping from a distance to a distance code. dist is the distance - 1 and
* must not have side effects. dist_code[256] and dist_code[257] are never
* used.
*/
#define d_code(dist) ((dist) < 256 ? dist_code[dist] : dist_code[256+((dist)>>7)])

#define MAX(a,b) (a >= b ? a : b)
/* the arguments must not have side effects */

/* ===========================================================================
* Allocate the match buffer, initialize the various tables and save the
* location of the internal file attribute (ascii/binary) and method
* (DEFLATE/STORE).
*/
void ct_init(unsigned short *attr, int *methodp)
{
    int n; /* iterates over tree elements */
    int bits; /* bit counter */
    int length; /* length value */
    int code; /* code value */
    int dist; /* distance index */

    file_type = attr;
    file_method = methodp;
    compressed_len = input_len = 0L;

    if (static_dtree[0].Len != 0) return;   /* ct_init already called */

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    length = 0;
    for (code = 0; code < LENGTH_CODES - 1; code ++)
    {
        base_length[code] = length;
        for (n = 0; n < (1 << extra_lbits[code]); n ++)
        {
            length_code[length ++ ] = (unsigned char)code;
        }
    }
    /* Note that the length 255 (match length 258) can be represented
    * in two different ways: code 284 + 5 bits or code 285, so we
    * overwrite length_code[255] to use the best encoding:
    */
    length_code[length - 1] = (unsigned char)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    for (code = 0 ; code < 16; code ++)
    {
        base_dist[code] = dist;
        for (n = 0; n < (1 << extra_dbits[code]); n ++)
        {
            dist_code[dist ++ ] = (unsigned char)code;
        }
    }
    dist >>= 7; /* from now on, all distances are divided by 128 */
    for (; code < D_CODES; code ++)
    {
        base_dist[code] = dist << 7;/* base dist */
        for (n = 0; n < (1 << (extra_dbits[code] - 7)); n ++)
        {
            dist_code[256 + dist ++ ] = (unsigned char)code;   /* dist code */
        }
    }

    /* Construct the codes of the static literal tree */
    for (bits = 0; bits <= MAX_BITS; bits ++) bl_count[bits] = 0;
    n = 0;
    while (n <= 143) static_ltree[n ++ ].Len = 8, bl_count[8] ++;   /* 初始化 */
    while (n <= 255) static_ltree[n ++ ].Len = 9, bl_count[9] ++;   /* 初始化 */
    while (n <= 279) static_ltree[n ++ ].Len = 7, bl_count[7] ++;   /* 初始化 */
    while (n <= 287) static_ltree[n ++ ].Len = 8, bl_count[8] ++;   /* 初始化 */
    /* Codes 286 and 287 do not exist, but we must include them in the
    * tree construction to get a canonical Huffman tree (longest code
    * all ones)
    */
    gen_codes((ct_data near *)static_ltree, L_CODES + 1);

    /* The static distance tree is trivial: */
    for (n = 0; n < D_CODES; n ++)
    {
        static_dtree[n].Len = 5; /* 初始化 */
        static_dtree[n].Code = bi_reverse(n, 5);   /* 初始化 */
    }

    /* Initialize the first block of the first file: */
    init_block();
}

/* ===========================================================================
* Initialize a new block.
*/
static void init_block()
{
    int n; /* iterates over tree elements */

    /* Initialize the trees. */
    for (n = 0; n < L_CODES; n ++) dyn_ltree[n].Freq = 0;
    for (n = 0; n < D_CODES; n ++) dyn_dtree[n].Freq = 0;
    for (n = 0; n < BL_CODES; n ++) bl_tree[n].Freq = 0;

    dyn_ltree[END_BLOCK].Freq = 1;
    opt_len = static_len = 0L;
    last_lit = last_dist = last_flags = 0;
    flags = 0;
    flag_bit = 1;
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */

/* ===========================================================================
* Remove the smallest element from the heap and recreate the heap with
* one less element. Updates heap and heap_len.
*/
#define pqremove(tree, top) \
{\
  top = heap[SMALLEST]; \
  heap[SMALLEST] = heap[heap_len--]; \
  pqdownheap(tree, SMALLEST); \
}

/* ===========================================================================
* Compares to subtrees, using the tree depth as tie breaker when
* the subtrees have equal frequency. This minimizes the worst case length.
*/
#define smaller(tree, n, m) \
  (tree[n].Freq < tree[m].Freq || \
  (tree[n].Freq == tree[m].Freq && depth[n] <= depth[m]))

/* ===========================================================================
* Restore the heap property by moving down the tree starting at node k,
* exchanging a node with the smallest of its two sons if necessary, stopping
* when the heap property is re-established (each father smaller than its
* two sons).
*/
static void pqdownheap(ct_data near *tree, int k)
{
    int v = heap[k];
    int j = k << 1; /* left son of k */
    while (j <= heap_len)
    {
        /* Set j to the smallest of the two sons: */
        if (j < heap_len && smaller(tree, heap[j + 1], heap[j])) j ++;

        /* Exit if v is smaller than both sons */
        if (smaller(tree, v, heap[j])) break;

        /* Exchange v with the smallest son */
        heap[k] = heap[j];
        k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    heap[k] = v;
}

/* ===========================================================================
* Compute the optimal bit lengths for a tree and update the total bit length
* for the current block.
* IN assertion: the fields freq and dad are set, heap[heap_max] and
* above are the tree nodes sorted by increasing frequency.
* OUT assertions: the field len is set to the optimal bit length, the
* array bl_count contains the frequencies for each bit length.
* The length opt_len is updated; static_len is also updated if stree is
* not null.
*/
static void gen_bitlen(tree_desc near *desc)
{
    ct_data near *tree = desc->dyn_tree;
    int near *extra = desc->extra_bits;
    int base = desc->extra_base;
    int max_code = desc->max_code;
    int max_length = desc->max_length;
    ct_data near *stree = desc->static_tree;
    int h; /* heap index */
    int n, m; /* iterate over the tree elements */
    int bits; /* bit length */
    int xbits; /* extra bits */
    unsigned short f; /* frequency */
    int overflow = 0; /* number of elements with bit length too large */

    for (bits = 0; bits <= MAX_BITS; bits ++) bl_count[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
    * overflow in the case of the bit length tree).
    */
    tree[heap[heap_max]].Len = 0; /* root of the heap */

    for (h = heap_max + 1; h < HEAP_SIZE; h ++)
    {
        n = heap[h];
        bits = tree[tree[n].Dad].Len + 1;
        if (bits > max_length) bits = max_length, overflow ++;
        tree[n].Len = (unsigned short)bits;
        /* We overwrite tree[n].Dad which is no longer needed */

        if (n > max_code) continue;   /* not a leaf node */

        bl_count[bits] ++;
        xbits = 0;
        if (n >= base) xbits = extra[n - base];
        f = tree[n].Freq;
        opt_len += (unsigned int)f * (bits + xbits);
        if (stree) static_len += (unsigned int)f * (stree[n].Len + xbits);
    }
    if (overflow == 0) return;

    Trace((stderr, "\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    do
    {
        bits = max_length - 1;
        while (bl_count[bits] == 0) bits --;
        bl_count[bits] --; /* move one leaf down the tree */
        bl_count[bits + 1] += 2; /* move one overflow item as its brother */
        bl_count[max_length] --;
        /* The brother of the overflow item also moves one step up,
        * but this does not affect bl_count[max_length]
        */
        overflow -= 2;
    }
    while (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
    * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
    * lengths instead of fixing only the wrong ones. This idea is taken
    * from 'ar' written by Haruhiko Okumura.)
    */
    for (bits = max_length; bits != 0; bits --)
    {
        n = bl_count[bits];
        while (n != 0)
        {
            m = heap[ -- h];
            if (m > max_code) continue;
            if (tree[m].Len != (unsigned) bits)
            {
                Trace((stderr, "code %d bits %d->%d\n", m, tree[m].Len, bits));
                opt_len += ((int)bits - (int)tree[m].Len) * (int)tree[m].Freq;
                tree[m].Len = (unsigned short)bits;
            }
            n --;
        }
    }
}

/* ===========================================================================
* Generate the codes for a given tree and bit counts (which need not be
* optimal).
* IN assertion: the array bl_count contains the bit length statistics for
* the given tree and the field len is set for all tree elements.
* OUT assertion: the field code is set for all tree elements of non
* zero code length.
*/
static void gen_codes(ct_data near *tree, int max_code)
{
    unsigned short next_code[MAX_BITS + 1]; /* next code value for each bit length */
    unsigned short code = 0; /* running code value */
    int bits; /* bit index */
    int n; /* code index */

    /* The distribution counts are first used to generate the code values
    * without bit reversal.
    */
    for (bits = 1; bits <= MAX_BITS; bits ++)
    {
        next_code[bits] = code = (code + bl_count[bits - 1]) << 1;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
    * must be all ones.
    */
    Assert(code + bl_count[MAX_BITS] - 1 == (1 << MAX_BITS) - 1,
           "inconsistent bit counts");
    Tracev((stderr, "\ngen_codes: max_code %d ", max_code));

    for (n = 0; n <= max_code; n ++)
    {
        int len = tree[n].Len;
        if (len == 0) continue;
        /* Now reverse the bits */
        tree[n].Code = bi_reverse(next_code[len] ++, len);

        Tracec(tree != static_ltree, (stderr, "\nn %3d %c l %2d c %4x (%x) ",
                                      n, (isgraph(n) ? n : ' '), len, tree[n].Code, next_code[len] - 1));
    }
}

/* ===========================================================================
* Construct one Huffman tree and assigns the code bit strings and lengths.
* Update the total bit length for the current block.
* IN assertion: the field freq is set for all tree elements.
* OUT assertions: the fields len and code are set to the optimal bit length
* and corresponding code. The length opt_len is updated; static_len is
* also updated if stree is not null. The field max_code is set.
*/
static void build_tree(tree_desc near *desc)
{
    ct_data near *tree = desc->dyn_tree;
    ct_data near *stree = desc->static_tree;
    int elems = desc->elems;
    int n, m; /* iterate over heap elements */
    int max_code = - 1; /* largest code with non zero frequency */
    int node = elems; /* next internal node of the tree */

    /* Construct the initial heap, with least frequent element in
    * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
    * heap[0] is not used.
    */
    heap_len = 0, heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n ++)
    {
        if (tree[n].Freq != 0)
        {
            heap[ ++ heap_len] = max_code = n;
            depth[n] = 0;
        }
        else
        {
            tree[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
    * and that at least one bit should be sent even if there is only one
    * possible code. So to avoid special checks later on we force at least
    * two codes of non zero frequency.
    */
    while (heap_len < 2)
    {
        int new = heap[ ++ heap_len] = (max_code < 2 ? ++ max_code : 0);   /* 初始化 */
        tree[new].Freq = 1;
        depth[new] = 0;
        opt_len --;
        if (stree) static_len -= stree[new].Len;
        /* new is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
    * establish sub-heaps of increasing lengths:
    */
    for (n = heap_len / 2; n >= 1; n --) pqdownheap(tree, n);

    /* Construct the Huffman tree by repeatedly combining the least two
    * frequent nodes.
    */
    do
    {
        pqremove(tree, n);   /* n = node of least frequency */
        m = heap[SMALLEST]; /* m = node of next least frequency */

        heap[ -- heap_max] = n; /* keep the nodes sorted by frequency */
        heap[ -- heap_max] = m;

        /* Create a new node father of n and m */
        tree[node].Freq = tree[n].Freq + tree[m].Freq;
        depth[node] = (unsigned char)(MAX(depth[n], depth[m]) + 1);
        tree[n].Dad = tree[m].Dad = (unsigned short)node;
#ifdef DUMP_BL_TREE
        if (tree == bl_tree)
        {
            1 fprintf(stderr, "\nnode %d(%d), sons %d(%d) %d(%d)",
                      node, tree[node].Freq, n, tree[n].Freq, m, tree[m].Freq);
        }
#endif
        /* and insert the new node in the heap */
        heap[SMALLEST] = node ++;
        pqdownheap(tree, SMALLEST);
    }
    while (heap_len >= 2);    /* 堆深度判断 */

    heap[ -- heap_max] = heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
    * generate the bit lengths.
    */
    gen_bitlen((tree_desc near *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes((ct_data near *)tree, max_code);
}

/* ===========================================================================
* Scan a literal or distance tree to determine the frequencies of the codes
* in the bit length tree. Updates opt_len to take into account the repeat
* counts. (The contribution of the bit length codes will be added later
* during the construction of bl_tree.)
*/
static void scan_tree(ct_data near *tree, int max_code)
{
    int n; /* iterates over all tree elements */
    int prevlen = - 1; /* last emitted length */
    int curlen; /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0; /* repeat count of the current code */
    int max_count = 7; /* max repeat count */
    int min_count = 4; /* min repeat count */

    if (nextlen == 0) max_count = 138, min_count = 3;   /* 初始化 */
    tree[max_code + 1].Len = (unsigned short)0xffff;   /* guard */

    for (n = 0; n <= max_code; n ++)
    {
        curlen = nextlen;
        nextlen = tree[n + 1].Len;
        if (++ count < max_count && curlen == nextlen)
        {
            continue;
        }
        else if (count < min_count)
        {
            bl_tree[curlen].Freq += count;
        }
        else if (curlen != 0)
        {
            if (curlen != prevlen) bl_tree[curlen].Freq ++;
            bl_tree[REP_3_6].Freq ++;
        }
        else if (count <= 10)       /* 长度比较 */
        {
            bl_tree[REPZ_3_10].Freq ++;
        }
        else
        {
            bl_tree[REPZ_11_138].Freq ++;
        }
        count = 0;
        prevlen = curlen;
        if (nextlen == 0)
        {
            max_count = 138, min_count = 3; /* 初始化 */
        }
        else if (curlen == nextlen)
        {
            max_count = 6, min_count = 3; /* 初始化 */
        }
        else
        {
            max_count = 7, min_count = 4; /* 初始化 */
        }
    }
}

/* ===========================================================================
* Send a literal or distance tree in compressed form, using the codes in
* bl_tree.
*/
static void send_tree(ct_data near *tree, int max_code)
{
    int n; /* iterates over all tree elements */
    int prevlen = - 1; /* last emitted length */
    int curlen; /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0; /* repeat count of the current code */
    int max_count = 7; /* max repeat count */
    int min_count = 4; /* min repeat count */

    /* tree[max_code+1].Len = -1; */ /* guard already set */
    if (nextlen == 0) max_count = 138, min_count = 3;

    for (n = 0; n <= max_code; n ++)
    {
        curlen = nextlen;
        nextlen = tree[n + 1].Len;
        if (++ count < max_count && curlen == nextlen)
        {
            continue;
        }
        else if (count < min_count)
        {
            do
            {
                send_code(curlen, bl_tree);
            }
            while (-- count != 0);
        }
        else if (curlen != 0)
        {
            if (curlen != prevlen)
            {
                send_code(curlen, bl_tree);
                count --;
            }
            send_code(REP_3_6, bl_tree);
            send_bits(count - 3, 2);   /* 发送到缓存 */
        }
        else if (count <= 10)       /* 数据长度 */
        {
            send_code(REPZ_3_10, bl_tree);
            send_bits(count - 3, 3);   /* 发送到缓存 */
        }
        else
        {
            send_code(REPZ_11_138, bl_tree);
            send_bits(count - 11, 7);   /* 发送到缓存 */
        }
        count = 0;
        prevlen = curlen;
        if (nextlen == 0)
        {
            max_count = 138, min_count = 3; /* 初始化 */
        }
        else if (curlen == nextlen)
        {
            max_count = 6, min_count = 3; /* 初始化 */
        }
        else
        {
            max_count = 7, min_count = 4; /* 初始化 */
        }
    }
}

/* ===========================================================================
* Construct the Huffman tree for the bit lengths and return the index in
* bl_order of the last bit length code to send.
*/
static int build_bl_tree()
{
    int max_blindex; /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_tree((ct_data near *)dyn_ltree, l_desc.max_code);
    scan_tree((ct_data near *)dyn_dtree, d_desc.max_code);

    /* Build the bit length tree: */
    build_tree((tree_desc near *)(& bl_desc));
    /* opt_len now includes the length of the tree representations, except
    * the lengths of the bit lengths codes and the 5+5+4 bits for the counts.
    */

    /* Determine the number of bit length codes to send. The pkzip format
    * requires that at least 4 bit length codes be sent. (appnote.txt says
    * 3 but the actual value used is 4.)
    */
    for (max_blindex = BL_CODES - 1; max_blindex >= 3; max_blindex --)
    {
        if (bl_tree[bl_order[max_blindex]].Len != 0) break;
    }
    /* Update opt_len to include the bit length tree and counts */
    opt_len += 3 * (max_blindex + 1) + 5 + 5 + 4;  /* 初始化 */
    Tracev((stderr, "\ndyn trees: dyn %ld, stat %ld", opt_len, static_len));

    return max_blindex;
}

/* ===========================================================================
* Send the header for a block using dynamic Huffman trees: the counts, the
* lengths of the bit length codes, the literal tree and the distance tree.
* IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
*/
static void send_all_trees(int lcodes, int dcodes, int blcodes)
{
    int rank; /* index in bl_order */

    Assert(lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
           "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_bits(lcodes - 257, 5);   /* not +255 as stated in appnote.txt */
    send_bits(dcodes - 1, 5);   /* 写入缓存 */
    send_bits(blcodes - 4, 4);   /* not -3 as stated in appnote.txt */
    for (rank = 0; rank < blcodes; rank ++)
    {
        Tracev((stderr, "\nbl code %2d ", bl_order[rank]));
        send_bits(bl_tree[bl_order[rank]].Len, 3);   /* 写入缓存 */
    }
    Tracev((stderr, "\nbl tree: sent %ld", bits_sent));

    send_tree((ct_data near *)dyn_ltree, lcodes - 1);   /* send the literal tree */
    Tracev((stderr, "\nlit tree: sent %ld", bits_sent));

    send_tree((ct_data near *)dyn_dtree, dcodes - 1);   /* send the distance tree */
    Tracev((stderr, "\ndist tree: sent %ld", bits_sent));
}

/* ===========================================================================
* Determine the best encoding for the current block: dynamic trees, static
* trees or store, and output the encoded block to the zip file. This function
* returns the total compressed length for the file so far.
*/
unsigned int flunsigned short_block(char *buf, unsigned int stored_len, int eof)
{
    unsigned int opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    int max_blindex; /* index of last bit length code of non zero freq */

    flag_buf[last_flags] = flags; /* Save the flags for the last 8 items */

    /* Check if the file is ascii or binary */
    if (* file_type == (unsigned short)UNKNOWN) set_file_type();

    /* Construct the literal and distance trees */
    build_tree((tree_desc near *)(& l_desc));
    Tracev((stderr, "\nlit data: dyn %ld, stat %ld", opt_len, static_len));

    build_tree((tree_desc near *)(& d_desc));
    Tracev((stderr, "\ndist data: dyn %ld, stat %ld", opt_len, static_len));
    /* At this point, opt_len and static_len are the total bit lengths of
    * the compressed block data, excluding the tree representations.
    */

    /* Build the bit length tree for the above two trees, and get the index
    * in bl_order of the last bit length code to send.
    */
    max_blindex = build_bl_tree();

    /* Determine the best encoding. Compute first the block length in bytes */
    opt_lenb = (opt_len + 3 + 7) >> 3;  /* 初始化 */
    static_lenb = (static_len + 3 + 7) >> 3;  /* 初始化 */
    input_len += stored_len; /* for debugging only */

    Trace((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u dist %u ",
           opt_lenb, opt_len, static_lenb, static_len, stored_len,
           last_lit, last_dist));

    if (static_lenb <= opt_lenb) opt_lenb = static_lenb;

    /* If compression failed and this is the first and last block,
    * and if the zip file can be seeked (to rewrite the static header),
    * the whole file is transformed into a stored file:
    */
    if (stored_len <= opt_lenb && eof && compressed_len == 0L && seekable())
    {
        /* Since LIT_BUFSIZE <= 2*WSIZE, the input data must be there: */
        if (buf == (char *)0) printf("block vanished");

        copy_block(buf, (unsigned)stored_len, 0);     /* without header */
        compressed_len = stored_len << 3; /* 压缩长度 */
        * file_method = STORED;
    }
    else if (stored_len + 4 <= opt_lenb && buf != (char *)0)        /* 4: two words for the lengths */
    {
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
        * Otherwise we can't have processed more than WSIZE input bytes since
        * the last block flunsigned short, because compression would have been
        * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
        * transform a block into a stored block.
        */
        send_bits((STORED_BLOCK << 1) + eof, 3);   /* send block type */
        compressed_len = (compressed_len + 3 + 7) & ~ 7L;   /* 压缩长度 */
        compressed_len += (stored_len + 4) << 3;   /* 压缩长度 */

        copy_block(buf, (unsigned)stored_len, 1);     /* with header */
    }
    else if (static_lenb == opt_lenb)
    {
        send_bits((STATIC_TREES << 1) + eof, 3);   /* 发送到缓存 */
        compress_block((ct_data near *)static_ltree, (ct_data near *)static_dtree);
        compressed_len += 3 + static_len;/* 块压缩 */
    }
    else
    {
        send_bits((DYN_TREES << 1) + eof, 3);   /* 发送到缓存 */
        send_all_trees(l_desc.max_code + 1, d_desc.max_code + 1, max_blindex + 1);
        compress_block((ct_data near *)dyn_ltree, (ct_data near *)dyn_dtree);
        compressed_len += 3 + opt_len; /* 压缩长度 */
    }
    Assert(compressed_len == bits_sent, "bad compressed size");
    init_block();

    if (eof)
    {
        Assert(input_len == isize, "bad input size");
        bi_windup();
        compressed_len += 7; /* align on byte boundary */
    }

    return compressed_len >> 3; /* 压缩长度 */
}

/* ===========================================================================
* Save the match info and tally the frequency counts. Return true if
* the current block must be flunsigned shorted.
*/
int ct_tally(int dist, int lc)
{
    l_buf[last_lit ++ ] = (unsigned char)lc;
    if (dist == 0)
    {
        /* lc is the unmatched char */
        dyn_ltree[lc].Freq ++;
    }
    else
    {
        /* Here, lc is the match length - MIN_MATCH */
        dist --; /* dist = match distance - 1 */
        Assert((unsigned short)dist < (unsigned short)MAX_DIST &&
               (unsigned short)lc <= (unsigned short)(MAX_MATCH - MIN_MATCH) &&
               (unsigned short)d_code(dist) < (unsigned short)D_CODES, "ct_tally: bad match");

        dyn_ltree[length_code[lc] + LITERALS + 1].Freq ++;
        dyn_dtree[d_code(dist)].Freq ++;

        d_buf[last_dist ++ ] = (unsigned short)dist;
        flags |= flag_bit;
    }
    flag_bit <<= 1;

    /* Output the flags if they fill a byte: */
    if ((last_lit & 7) == 0)
    {
        flag_buf[last_flags ++ ] = flags;
        flags = 0, flag_bit = 1;
    }
    /* Try to guess if it is profitable to stop the current block here */
    if (level > 2 && (last_lit & 0xfff) == 0)
    {
        /* Compute an upper bound for the compressed length */
        unsigned int out_length = (unsigned int)last_lit * 8L;
        unsigned int in_length = (unsigned int)strstart - block_start;
        int dcode;
        for (dcode = 0; dcode < D_CODES; dcode ++)
        {
            out_length += (unsigned int)dyn_dtree[dcode].Freq * (5L + extra_dbits[dcode]);   /* 轮询 */
        }
        out_length >>= 3; /* 输出长度 */
        if (last_dist < last_lit / 2 && out_length < in_length / 2) return 1;  /* 长度判断 */
    }
    return (last_lit == LIT_BUFSIZE - 1 || last_dist == DIST_BUFSIZE);
    /* We avoid equality with LIT_BUFSIZE because of wraparound at 64K
    * on 16 bit machines and because stored blocks are restricted to
    * 64K-1 bytes.
    */
}

/* ===========================================================================
* Send the block data compressed using the given Huffman trees
*/
static void compress_block(ct_data near *ltree, ct_data near *dtree)
{
    unsigned int dist; /* 匹配串距离 */
    int lc; /* match length or unmatched char (if dist == 0) */
    unsigned int lx = 0; /* running index in l_buf */
    unsigned int dx = 0; /* running index in d_buf */
    unsigned int fx = 0; /* running index in flag_buf */
    unsigned char flag = 0; /* current flags */
    unsigned int code; /* the code to send */
    int extra; /* number of extra bits to send */

    if (last_lit != 0) do
        {
            if ((lx & 7) == 0) flag = flag_buf[fx ++ ];    /* 标志 */
            lc = l_buf[lx ++ ];
            if ((flag & 1) == 0)
            {
                send_code(lc, ltree);   /* send a literal byte */
                Tracecv(isgraph(lc), (stderr, " '%c' ", lc));
            }
            else
            {
                /* Here, lc is the match length - MIN_MATCH */
                code = length_code[lc];
                send_code(code + LITERALS + 1, ltree);   /* send the length code */
                extra = extra_lbits[code];
                if (extra != 0)
                {
                    lc -= base_length[code];
                    send_bits(lc, extra);   /* send the extra length bits */
                }
                dist = d_buf[dx ++ ];
                /* Here, dist is the match distance - 1 */
                code = d_code(dist);
                Assert(code < D_CODES, "bad d_code");

                send_code(code, dtree);   /* send the distance code */
                extra = extra_dbits[code];
                if (extra != 0)
                {
                    dist -= base_dist[code];
                    send_bits(dist, extra);   /* send the extra distance bits */
                }
            } /* literal or match pair ? */
            flag >>= 1;
        }
        while (lx < last_lit);

    send_code(END_BLOCK, ltree);
}

/*
 * 使用粗略近似值将文件类型设置为ASCII或二进制：二进制如果超过20%的字节<=6或>=128，则为ascii。
 * 断言：设置动态树的freq字段和所有频率不超过64K（以适合16位机器上的整数）。
*/
static void set_file_type(void)
{
    int n = 0;
    unsigned int bin_freq = 0;
    unsigned int ascii_freq = 0;

    while (n < 7) bin_freq += dyn_ltree[n++].Freq;   /* 初始化 */
    while (n < 128) ascii_freq += dyn_ltree[n++].Freq;   /* 初始化 */
    while (n < LITERALS) bin_freq += dyn_ltree[n++].Freq;
    * file_type = bin_freq > (ascii_freq >> 2) ? BINARY : ASCII;   /* 设置文件打开方式 */
    if (* file_type == BINARY && translate_eol)
    {
        printf("-l used on binary file");
    }
    return;
}

/******************************************************************************/
/*                                全局函数实现                                */
/******************************************************************************/
unsigned char pucBuf[] = "this is the content for zip xxbababababababababaabababababababababababababababababababababbababababa";
unsigned char aucOut[100];
unsigned char aucOut1[100];
void main()
{
    int uiTargetBufLen;
    int uiTargetBufLen1;
    int i = 0;
    int aaa;
    memset(aucOut, 0x0, 100);
    uiTargetBufLen = zipmem(pucBuf, strlen(pucBuf), aucOut);
    printf("压缩后的数据长度：");
    printf("%d\n", uiTargetBufLen);
    printf("压缩后的数据内容：");
    for (aaa = 0; aaa < uiTargetBufLen; aaa++)
    {
        printf("%d", aucOut[aaa]);
        printf(" ");
    }
    printf(" \n");
    uiTargetBufLen1 = unzipmem(aucOut, uiTargetBufLen, aucOut1);
    printf("解压后的数据内容:");
    printf(aucOut1);
    while (1)
    {
    }
    return;
}



