/* config.c
 *
 * Future me: I'm sorry.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vomit.h"
#include "debug.h"

static bool reloading = false;

typedef struct
{
    char *name;
    word sectors_per_track;
    word heads;
    dword sectors;
    word bytes_per_sector;
    byte media_type;
} disk_type_t;

static disk_type_t floppy_types[] =
{
    { "1.44M", 18, 2, 2880, 512, 4 },
    { "720kB",  9, 2, 1440, 512, 3 },
    { "1.2M",  15, 2, 2400, 512, 2 },
    { "360kB",  9, 2,  720, 512, 1 },
    { "320kB",  8, 2,  640, 512, 0 },
    { "160kB",  8, 1,  320, 512, 0 },
    { 0L,       0, 0,    0,   0, 0 }
};

static void
unspeakable_abomination()
{

    FILE *fconf, *ftmp;
    char curline[256], *curtok; char lfname[MAX_FN_LENGTH];
    byte tb;
    word lseg, loff, ldrv, lspt, lhds, lsect, lsectsize;

    fconf = fopen("vm.conf", "r");
    if(fconf == NULL) {
        vlog( VM_CONFIGMSG, "Couldn't load vm.conf" );
        //vm_exit(1);
        return;
    }

    while( !feof( fconf ))
    {
        if( !fgets(curline, 256, fconf) )
            break;
        if((curline[0]!='#')&&(strlen(curline)>1)) {
            curtok = strtok(curline, " \t\n");
            if(strcmp(curtok,"loadfile")==0) {
                strcpy(lfname, strtok(NULL, " \t\n"));
                lseg = (word)strtol(strtok(NULL, ": \t\n"),NULL,16);
                loff = (word)strtol(strtok(NULL, " \t\n"),NULL,16);

                vlog( VM_INITMSG, "Loading %s to %04X:%04X", lfname, lseg, loff );

                ftmp = fopen(lfname, "rb");
                if (ftmp == NULL)
                {
                    vlog( VM_CONFIGMSG, "Error while processing \"loadfile\" line." );
                    vm_exit(1);
                }

                if (!fread(g_cpu->memory+lseg*16+loff, 1, MAX_FILESIZE, ftmp)) {
                    vlog(VM_CONFIGMSG, "Failure reading from %s", lfname);
                    vm_exit(1);
                }
                fclose(ftmp);
            }
            else if( !reloading && !strcmp( curtok, "setbyte" ))
            {
                lseg = strtol(strtok(NULL,": \t\n"), NULL, 16);
                loff = strtol(strtok(NULL," \t\n"), NULL, 16);
                curtok = strtok(NULL, " \t\n");

                //vlog( VM_INITMSG, "Entering at %04X:%04X", lseg, loff );

                while( curtok )
                {
                    tb = strtol(curtok,NULL,16);
                    //vlog( VM_INITMSG, "  [%04X] = %02X", loff, tb );
                    vomit_cpu_memory_write16(g_cpu, lseg, loff++, tb);
                    curtok = strtok(NULL, " \t\n");
                }
                curtok=curline;
            }
            else if( !strcmp( curtok, "floppy" ))
            {
                char type[32];
                disk_type_t *dt;

                ldrv = strtol( strtok( 0L, " \t\n" ), 0L, 10 );
                strcpy( type, strtok( 0L, " \t\n" ));
                strcpy( lfname, strtok( 0L, "\n" ));

                for( dt = floppy_types; dt->name; ++dt )
                {
                    if( !strcmp( type, dt->name ))
                        break;
                }

                if( !dt->name )
                {
                    vlog( VM_INITMSG, "Invalid floppy type: \"%s\"", type );
                    continue;
                }

                strcpy(drv_imgfile[ldrv], lfname);
                drv_spt[ldrv] = dt->sectors_per_track;
                drv_heads[ldrv] = dt->heads;
                drv_sectors[ldrv] = dt->sectors;
                drv_sectsize[ldrv] = dt->bytes_per_sector;
                drv_type[ldrv] = dt->media_type;
                drv_status[ldrv] = 1;
                /* TODO: What should the media type be? */
                drv_type[ldrv] = 0;

                vlog( VM_INITMSG, "Floppy %d: %s (%dspt, %dh, %ds (%db))", ldrv, lfname, dt->sectors_per_track, dt->heads, dt->sectors, dt->bytes_per_sector );
            }
            else if(strcmp(curtok,"drive")==0) {    /* segfaults ahead! */
                ldrv = strtol(strtok(NULL, " \t\n"), NULL, 16);
                strcpy(lfname, strtok(NULL, " \t\n"));
                lspt = strtol(strtok(NULL, " \t\n"), NULL, 16);
                lhds = strtol(strtok(NULL, " \t\n"), NULL, 16);
                lsect = strtol(strtok(NULL, " \t\n"), NULL, 16);
                lsectsize = strtol(strtok(NULL, " \t\n"), NULL, 16);

                vlog( VM_INITMSG, "Drive %d: %s (%dspt, %dh, %ds (%db))", ldrv, lfname, lspt, lhds, lsect, lsectsize );

                ldrv += 2;

                strcpy(drv_imgfile[ldrv], lfname);
                drv_spt[ldrv] = lspt;
                drv_heads[ldrv] = lhds;
                drv_sectors[ldrv] = lsect;
                drv_sectsize[ldrv] = lsectsize;
                drv_status[ldrv] = 1;
            }
            else if( !strcmp( curtok, "fixed" ))
            {
                long megabytes;

                ldrv = strtol( strtok(NULL, " \t\n"), NULL, 16 );
                strcpy(lfname, strtok(NULL, " \t\n"));

                megabytes = strtol( strtok(NULL, " \t\n"), NULL, 10 );

                vlog( VM_INITMSG, "Fixed drive %d: %s (%ld MB)", ldrv, lfname, megabytes );

                ldrv += 2;

                strcpy( drv_imgfile[ldrv], lfname );
                drv_spt[ldrv] = 63;
                drv_heads[ldrv] = 16;
                drv_sectsize[ldrv] = 512;
                drv_status[ldrv] = 1;
                drv_sectors[ldrv] = (megabytes * 1048576) / drv_sectsize[ldrv];
            }
            else if( !reloading && !strcmp( curtok, "memory" ))
            {
                curtok = strtok(NULL, " \t\n");
                g_cpu->memory_size = (word)(strtol(curtok, NULL, 10));
                vlog(VM_INITMSG, "Memory size: %d kilobytes", g_cpu->memory_size);
            }
            else if( !reloading && !strcmp( curtok, "entry" ))
            {
                WORD entry_cs, entry_ip;
                curtok = strtok(NULL, ": \t\n");
                entry_cs = (WORD)strtol(curtok, NULL, 16);
                curtok = strtok(NULL, " \t\n");
                entry_ip = (WORD)strtol(curtok, NULL, 16);
                g_cpu->jump(entry_cs, entry_ip);
            }
            else if (!reloading && !strcmp(curtok, "addint")) {
                BYTE isr = strtol(strtok(NULL, " \t\n"), NULL, 16);
                WORD segment = strtol(strtok(NULL, ": \t\n"), NULL, 16);
                WORD offset = strtol(strtok(NULL, " \t\n"), NULL, 16);
                vlog(VM_INITMSG, "Software interrupt %02X at %04X:%04X", isr, segment, offset);
                g_cpu->setInterruptHandler(isr, segment, offset);
            }
        }
    }
    fclose(fconf);
}

void
vm_loadconf()
{
    reloading = false;
    unspeakable_abomination();
}

void
config_reload()
{
    reloading = true;
    unspeakable_abomination();
}