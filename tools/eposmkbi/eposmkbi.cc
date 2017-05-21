/*=======================================================================*/
/* MKBI.C                                                                */
/*                                                                       */
/* Desc: Tool to generate an EPOS bootable image.                        */
/*                                                                       */
/* Parm: <boot image> <os image> <app1> <app2> ...                       */
/*                                                                       */
/* Auth: Guto - Changed By Fauze                                         */
/*=======================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include <system/info.h>

// CONSTANTS
static const unsigned int MAX_SI_LEN = 512;
static const char CFG_FILE[] = "etc/eposmkbi.conf";

// TYPES

// Target Machine Description
struct Configuration
{
    char          mode[16];
    char          arch[16];
    char          mach[16];
    char          mmod[16];
    unsigned short n_cpus;
    unsigned int  clock;
    unsigned char word_size;
    bool          endianess; // true => little, false => big
    unsigned int  mem_base;
    unsigned int  mem_top;
    unsigned int  boot_length_min;
    unsigned int  boot_length_max;
    short         node_id;   // node id in SAN (-1 => get from net)
    short         n_nodes;   // nodes in SAN (-1 => dynamic)
    unsigned char uuid[8];   // EPOS image Universally Unique Identifier
};

// System_Info
typedef _SYS::System_Info System_Info;

// PROTOTYPES
bool parse_config(FILE * cfg_file, Configuration * cfg);
void strtolower (char *dst,const char* src);
bool add_machine_secrets(int fd_img, unsigned int i_size, char * mach, char * mmod);

bool file_exist(char *file);

int put_buf(int fd_out, void *buf, int size);
int put_file(int fd_out, char *file);
int pad(int fd_out, int size);
bool lil_endian();

template<typename T> void invert(T &n);
template<typename T> int put_number(int fd, T num);
template<typename T> bool add_boot_map(int fd_out, System_Info * si);

// GLOBALS
Configuration CONFIG;

//=============================================================================
// MAIN
//=============================================================================
int main(int argc, char **argv)
{
    // Say hello
    printf("\nEPOS bootable image tool\n\n");

    // Read configuration
    char file[256];
    sprintf(file, "%s/%s", argv[1], CFG_FILE);
    FILE * cfg_file = fopen(file, "rb");
    if(!cfg_file) {
        fprintf(stderr, "Error: can't read configuration file \"%s\"!\n", file);
        return 1;
    }
    if(!parse_config(cfg_file, &CONFIG)) {
        fprintf(stderr, "Error: invalid configuration file \"%s\"!\n", file);
        return 1;
    }

    // Open destination file (rewrite)
    int fd_img = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 00644);
    if(fd_img < 0) {
        fprintf(stderr, "Error: can't create boot image \"%s\"!\n", argv[2]);
        return 1;
    }

    // Check ARGS
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <options> <EPOS root> <boot image> <app1> <app2> ...\n", argv[0]);
        return 1;
    }
    if(!strcmp(CONFIG.mode, "library") && (argc > 4)) {
        fprintf(stderr, "Error: library mode supports a single application!\n");
        return 1;
    }

    // Show configuration
    printf("  EPOS mode: %s\n", CONFIG.mode);
    printf("  Machine: %s\n", CONFIG.mach);
    printf("  Model: %s\n", CONFIG.mmod);
    printf("  Processor: %s (%d bits, %s-endian)\n", CONFIG.arch, CONFIG.word_size, CONFIG.endianess ? "little" : "big");
    printf("  Memory: %d KBytes\n", (CONFIG.mem_top - CONFIG.mem_base) / 1024);
    printf("  Boot Length: %d - %d (min - max) KBytes\n", CONFIG.boot_length_min, CONFIG.boot_length_max);
    if(CONFIG.node_id == -1)
        printf("  Node id: will get from the network\n");
    else
        printf("  Node id: %d\n", CONFIG.node_id);
    printf("  EPOS Image UUID: ");
    for(unsigned int i = 0; i < 8; i++)
        printf("%.2x", CONFIG.uuid[i]);

    // Create the boot image
    unsigned int image_size = 0;
    printf("\n  Creating EPOS bootable image in \"%s\":\n", argv[2]);

    // Add BOOT
    if(CONFIG.boot_length_max > 0) {
        sprintf(file, "%s/img/%s_boot", argv[1], CONFIG.mach);
        printf("    Adding boot strap \"%s\":", file);
        image_size += put_file(fd_img, file);
        if(image_size > CONFIG.boot_length_max) {
            printf(" failed!\n");
            fprintf(stderr, "Boot strap \"%s\" is too large! (%d bytes)\n", file, image_size);
            return 1;
        } else {
            while((image_size % CONFIG.boot_length_min != 0))
                image_size += pad(fd_img, 1);
        }
    }
    unsigned int boot_size = image_size;


    // Reserve space for System_Info if necessary
    System_Info si;
    bool need_si = true;
    if(image_size == 0) {
        need_si = false;
    } else
        if(sizeof(System_Info) > MAX_SI_LEN) {
            printf(" failed!\n");
            fprintf(stderr, "System_Info structure is too large (%d)!\n", sizeof(System_Info));
            return 1;
        } else
            image_size += pad(fd_img, MAX_SI_LEN);

    // Initialize the Boot_Map in System_Info
    si.bm.n_cpus   = CONFIG.n_cpus; // can be adjusted by SETUP in some machines
    si.bm.mem_base = CONFIG.mem_base;
    si.bm.mem_top  = CONFIG.mem_top;
    si.bm.io_base  = 0; // will be adjusted by SETUP
    si.bm.io_top   = 0; // will be adjusted by SETUP
    si.bm.node_id  = CONFIG.node_id;
    si.bm.n_nodes  = CONFIG.n_nodes;
    for(unsigned int i = 0; i < 8; i++)
        si.bm.uuid[i]  = CONFIG.uuid[i];

    // Add SETUP
    sprintf(file, "%s/img/%s_setup", argv[1], CONFIG.mach);
    if(file_exist(file)) {
        si.bm.setup_offset = image_size - boot_size;
        printf("    Adding setup \"%s\":", file);
        image_size += put_file(fd_img, file);
    } else
        si.bm.setup_offset = -1;

    // Add INIT and OS (for mode != library only)
    if(!strcmp(CONFIG.mode, "library")) {
        si.bm.init_offset = -1;
        si.bm.system_offset = -1;
    } else {
        // Add INIT
        si.bm.init_offset = image_size - boot_size;
        sprintf(file, "%s/img/%s_init", argv[1], CONFIG.mach);
        printf("    Adding init \"%s\":", file);
        image_size += put_file(fd_img, file);

        // Add SYSTEM
        si.bm.system_offset = image_size - boot_size;
        sprintf(file, "%s/img/%s_system", argv[1], CONFIG.mach);
        printf("    Adding system \"%s\":", file);
        image_size += put_file(fd_img, file);
    }

    // Add LOADER (if multiple applications) or the single application otherwise
    si.bm.application_offset = image_size - boot_size;
//    if((argc == 4) && strcmp(CONFIG.mode, "kernel")) { // Add Single APP
    if(argc == 4) { // Add Single APP
        printf("    Adding application \"%s\":", argv[3]);
        image_size += put_file(fd_img, argv[3]);
        si.bm.extras_offset = -1;
    } else { // Add LOADER
        sprintf(file, "%s/img/%s_loader", argv[1], CONFIG.mach);
        printf("    Adding loader \"%s\":", file);
        image_size += put_file(fd_img, file);

        // Add APPs
        si.bm.extras_offset = image_size - boot_size;
        struct stat file_stat;
        for(int i = 3; i < argc; i++) {
            printf("    Adding application \"%s\":", argv[i]);
            stat(argv[i], &file_stat);
            image_size += put_number(fd_img, file_stat.st_size);
            image_size += put_file(fd_img, argv[i]);
        }
        // Signalize last application by setting its size to 0
        image_size += put_number(fd_img, 0);
    }

    // Add the size of the image to the Boot_Map in System_Info (excluding BOOT)
    si.bm.img_size = image_size - boot_size;

    // Add System_Info
    if(need_si) {
        printf("    Adding system info:");
        if(lseek(fd_img, boot_size, SEEK_SET) < 0) {
            fprintf(stderr, "Error: can't seek the boot image!\n");
            return 1;
        }
        switch(CONFIG.word_size) {
        case  8: if(!add_boot_map<char>(fd_img, &si)) return 1; break;
        case 16: if(!add_boot_map<short>(fd_img, &si)) return 1; break;
        case 32: if(!add_boot_map<long>(fd_img, &si)) return 1; break;
        case 64: if(!add_boot_map<long long>(fd_img, &si)) return 1; break;
        default: return 1;
        }
        printf(" done.\n");
    }

    // Adding MACH specificities
    printf("\n  Adding specific boot features of \"%s\":", CONFIG.mach);
    if(!(add_machine_secrets(fd_img, image_size, CONFIG.mach, CONFIG.mmod))) {
        fprintf(stderr, "Error: specific features error!\n");
        return 1;
    }
    printf(" done.\n");

    //Finish
    close(fd_img);
    printf("\n  Image successfully generated (%d bytes)!\n\n", image_size);

    return 0;
}

//=============================================================================
// PARSE_CONFIG
//=============================================================================
bool parse_config(FILE * cfg_file, Configuration * cfg)
{
    char line[256];
    char * token;

    // EPOS Mode
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read MODE from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "MODE") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid MODE in configuration!\n");
        return false;
    }
    strtolower(cfg->mode, token);

    // Arch
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read ARCH from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "ARCH") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid ARCH in configuration!\n");
        return false;
    }
    strtolower(cfg->arch, token);

    // Machine
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read MACH from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "MACH") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid MACH in configuration!\n");
        return false;
    }
    strtolower(cfg->mach, token);

    // Model
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read MMOD from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "MMOD") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid MMOD in configuration!\n");
        return false;
    }
    strtolower(cfg->mmod, token);

    // CPUS
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read CPUS from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "CPUS") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid CPUS in configuration!\n");
        return false;
    }
    cfg->n_cpus = atoi(token);

    // Clock
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read CLOCK from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "CLOCK") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid CLOCK in configuration!\n");
        return false;
    }
    cfg->clock = atoi(token);

    // Word Size
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read WORD_SIZE from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "WORD_SIZE") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid WORD_SIZE in configuration!\n");
        return false;
    }
    cfg->word_size = atoi(token);

    // Endianess
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read ENDIANESS from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "ENDIANESS") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid ENDIANESS in configuration!\n");
        return false;
    }
    cfg->endianess = !strcmp(token, "little");

    // Memory Base
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read MEM_BASE from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "MEM_BASE") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid MEM_BASE in configuration!\n");
        return false;
    }
    cfg->mem_base = strtol(token, 0, 16);

    // Memory Top
    if(fgets(line, 256, cfg_file) != line) {
        fprintf(stderr, "Error: failed to read MEM_TOP from configuration file!\n");
        return false;
    }
    token = strtok(line, "=");
    if(strcmp(token, "MEM_TOP") || !(token = strtok(NULL, "\n"))) {
        fprintf(stderr, "Error: no valid MEM_TOP in configuration!\n");
        return false;
    }
    cfg->mem_top=strtol(token, 0, 16);

    // Boot Length Min
    if(fgets(line, 256, cfg_file) != line)
        cfg->boot_length_min = 0;
    else {
        token = strtok(line, "=");
        if(!strcmp(token, "BOOT_LENGTH_MIN") && (token = strtok(NULL, "\n")))
            cfg->boot_length_min = atoi(token);
        else
            cfg->boot_length_min = 0;
    }

    // Boot Length Max
    if(fgets(line, 256, cfg_file) != line)
        cfg->boot_length_max = 0;
    else {
        token = strtok(line, "=");
        if(!strcmp(token, "BOOT_LENGTH_MAX") && (token = strtok(NULL, "\n")))
            cfg->boot_length_max = atoi(token);
        else
            cfg->boot_length_max = 0;
    }

    // Node Id
    if(fgets(line, 256, cfg_file) != line)
        cfg->node_id = -1; // get from net
    else {
        token = strtok(line, "=");
        if(!strcmp(token, "NODE_ID") && (token = strtok(NULL, "\n")))
            cfg->node_id = atoi(token);
        else
            cfg->node_id = -1; // get from net
    }

    // Number of Nodes in SAN
    if(fgets(line, 256, cfg_file) != line)
        cfg->n_nodes = -1; // dynamic
    else {
        token = strtok(line, "=");
        if(!strcmp(token, "N_NODES") && (token = strtok(NULL, "\n")))
            cfg->n_nodes = atoi(token);
        else
            cfg->n_nodes = -1; // dynamic
    }

    // UUID
    if(fgets(line, 256, cfg_file) == line) {
        token = strtok(line, "=");
        if(!strcmp(token, "UUID") && (token = strtok(NULL, "\n"))) {
            unsigned int buf[16];
            unsigned int i, j;
            for(i = j = 0; (i < 16) && (sscanf(&token[j], "%2x", &buf[i]) == 1); i++, j+=2);
            for(i = j = 0; i < 8; i++, j+=2)
                cfg->uuid[i] = buf[j] ^ buf[j+1];
        }
    }

    return true;
}

//=============================================================================
// ADD_BOOT_MAP
//=============================================================================
template<typename T> bool add_boot_map(int fd, System_Info * si)
{
    if(!put_number(fd, static_cast<T>(si->bm.n_cpus)))
        return false;
    if(!put_number(fd, static_cast<T>(si->bm.mem_base)))
        return false;
    if(!put_number(fd, static_cast<T>(si->bm.mem_top)))
        return false;

    if(!put_number(fd, static_cast<T>(0)))
        return false;
    if(!put_number(fd, static_cast<T>(0)))
        return false;

    if(!put_number(fd, si->bm.node_id))
        return false;
    if(!put_number(fd, si->bm.n_nodes))
        return false;
    for(unsigned int i = 0; i < 8; i++)
        if(!put_number(fd, si->bm.uuid[i]))
            return false;

    if(!put_number(fd, static_cast<T>(si->bm.img_size)))
        return false;
    if(!put_number(fd, static_cast<T>(si->bm.setup_offset)))
        return false;
    if(!put_number(fd, static_cast<T>(si->bm.init_offset)))
        return false;
    if(!put_number(fd, static_cast<T>(si->bm.system_offset)))
        return false;
    if(!put_number(fd, static_cast<T>(si->bm.application_offset)))
        return false;
    if(!put_number(fd, static_cast<T>(si->bm.extras_offset)))
        return false;

    return true;
}

//=============================================================================
// ADD_MACHINE_SCRETS
//=============================================================================
bool add_machine_secrets(int fd, unsigned int i_size, char * mach, char * mmod)
{
    if (!strcmp(mach, "pc")) { // PC
        const unsigned int floppy_size   = 1474560;
        const unsigned int secrets_offset   = CONFIG.boot_length_min - 6;
        const unsigned short boot_id        = 0xaa55;
        const unsigned short num_sect       = ((i_size + 511) / 512);
        const unsigned short last_track_sec = num_sect <= 2880 ? 19 : 49; // either 144 tracks with 20 sectors or 144 tracks with 50 sectors

        // Pad the image to the size of a standard floppy
        if(lseek(fd, 0, SEEK_END) < 0) {
            fprintf(stderr, "Error: can't seek the boot image!\n");
            return false;
        }
        pad(fd, (floppy_size  - i_size));

        // Write the number of sectors to be read
        if(lseek(fd, secrets_offset, SEEK_SET) < 0) {
            fprintf(stderr, "Error: can't seek the boot image!\n");
            return false;
        }
        put_number(fd, last_track_sec);
        put_number(fd, num_sect);
        put_number(fd, boot_id);
    } else if (!strcmp(mach, "rcx")) { // RCX
        char key_string[] = "Do you byte, when I knock?";
        const unsigned short key_offset = 128 - (strlen(key_string) + 1);

        // Write key string to unlock epos
        if(lseek(fd,key_offset,SEEK_SET) < 0) {
            fprintf(stderr, "Error: can't seek the boot image!\n");
            return false;
        }
        put_buf(fd, key_string, (strlen(key_string)+1));
    }
    else if (!strcmp(mmod, "emote3")) { // EPOSMoteIII
        // Customer Configuration Area (CCA)
        //char key_string[] = ":020000040027D3\r\n:0CFFD400FFFFFFF700000000000020000D\r\n:00000001FF\r\n"; // Bootloader Enabled, enter by setting pin PA7 to low
        //char key_string[] = ":020000040027D3\r\n:0CFFD400FFFFFFFF000000000000200005\r\n:00000001FF\r\n"; // Bootloader Enabled, enter by setting pin PA7 to high
        char key_string[] = ":020000040027D3\r\n:0CFFD400FFFFFFEF000000000000200015\r\n:00000001FF\r\n"; // Bootloader Disabled
        const int key_offset = -strlen(":00000001FF\r\n");

        // Write key string to unlock epos
        if(lseek(fd,key_offset,SEEK_END) < 0) {
            fprintf(stderr, "Error: can't seek the boot image!\n");
            return false;
        }
        put_buf(fd, key_string, strlen(key_string));
    }

    return true;
}

//=============================================================================
// FILE_EXIST
//=============================================================================
bool file_exist(char *file)
{
    int fd_in;
    struct stat stat;

    fd_in = open(file, O_RDONLY);
    if(fd_in < 0)
        return false;

    if(fstat(fd_in, &stat) < 0)
        return false;

    return true;
}

//=============================================================================
// PUT_FILE
//=============================================================================
int put_file(int fd_out, char * file)
{
    int fd_in;
    struct stat stat;
    char * buffer;

    fd_in = open(file, O_RDONLY);
    if(fd_in < 0) {
        printf(" failed! (open)\n");
        return 0;
    }

    if(fstat(fd_in, &stat) < 0)  {
        printf(" failed! (stat)\n");
        return 0;
    }

    buffer = (char *) malloc(stat.st_size);
    if(!buffer) {
        printf(" failed! (malloc)\n");
        return 0;
    }
    memset(buffer, '\1', stat.st_size);

    if(read(fd_in, buffer, stat.st_size) < 0) {
        printf(" failed! (read)\n");
        free(buffer);
        return 0;
    }

    if(write(fd_out, buffer, stat.st_size) < 0) {
        printf(" failed! (write)\n");
        free(buffer);
        return 0;
    }

    free(buffer);
    close(fd_in);

    printf(" done.\n");

    return stat.st_size;
}

//=============================================================================
// PUT_BUF
//=============================================================================
int put_buf(int fd, void * buf, int size)
{
    if(!size)
        return 0;
    int written = write(fd, buf, size);
    if(written < 0) {
        fprintf(stderr, "Error: can't write to file!\n");
        written = 0;
    }
    return written;
}

//=============================================================================
// PUT_NUMBER
//=============================================================================
template<typename T> int put_number(int fd, T num)
{
    if((CONFIG.endianess != lil_endian()) && (sizeof(T) > 1))
        invert(num);
    if(write(fd, &num, sizeof(T)) < 0) {
        fprintf(stderr, "Error: can't write to file!\n");
        return 0;
    }
    return sizeof(T);
}

//=============================================================================
// PAD
//=============================================================================
int pad(int fd, int size)
{
    if(!size)
        return 0;

    char * buffer = (char *) malloc(size);
    if(!buffer) {
        fprintf(stderr, "Error: not enough memory!\n");
        return 0;
    }

    memset(buffer, '\1', size);
    if(write(fd, buffer, size) < 0) {
        fprintf(stderr, "Error: can't write to the boot image!\n");
        return 0;
    }

    free(buffer);
    return size;
}

//=============================================================================
// STRTOLOWER
//=============================================================================
void strtolower(char* dst, const char* src) {
    int i = 0;
    strcpy(dst,src);
    while(src[i] != '\0') {
        dst[i] = tolower(dst[i]);
        i++;
    }
}

//=============================================================================
// LIL_ENDIAN
//=============================================================================
bool lil_endian() {
    int test = 1;
    return (*((char*)&test)) ? true : false;
}

//=============================================================================
// INVERT
//=============================================================================
template<typename T> void invert(T & n)
{
    for(int i = 0, j = sizeof(T) - 1; i < (int)sizeof(T) / 2; i++, j--) {
        char * h = &(((char *)&n)[i]);
        char * l = &(((char *)&n)[j]);
        *h ^= *l;
        *l ^= *h;
        *h ^= *l;
    }
}

