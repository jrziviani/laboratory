/*
 * This is a sample to understand how GPT partition table is filled
 * and written into the disk. It's not a tool so it's better not try
 * using it in your system. If you want to test, do it an a virtual
 * machine where you can loose data.
 *
 * THE CODE HERE WILL DESTROY ANY DATA IN THE DISK, NO RECOVERY IS
 * POSSIBLE. I'm not responsible for any damage you do in your system.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301, USA.
*/

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include <fcntl.h>

#include <uuid/uuid.h>
#include <sys/ioctl.h>
#include <linux/fs.h>


/**********************************************************************
 * Constants
 *********************************************************************/
// hardcoding the disk sector size, must be re-done for 4Kn disks
#define SECTOR_SIZE 512
// the swap guid is 0657FD6D-A4AB-43C4-84E5-0933C84B4F4F but it must be
// reorganized for little endian systems
#define SWAP_UUID  "6DFD5706-ABA4-C443-84E5-0933C84B4F4F"
// the linux guid is 0FC63DAF-8483-4772-8E79-3D69D8477DE4 but it must be
// reorganized for little endian systems
#define LINUX_UUID "AF3DC60F-8384-7247-8E79-3D69D8477DE4"


/**********************************************************************
 * Structures
 *********************************************************************/
typedef struct gpt_partition_
{
    // GUID partition type to define the purpose of it
    uuid_t   partition_type;
    // unique id for this partition
    uuid_t   unique_partition;
    // starting lba of this partition
    uint64_t first_lba;
    // ending lba of this partition
    uint64_t last_lba;
    // partitions attributes (hidden, legacy_boot, etc)
    uint64_t attributes;
    // partition name UTF-16le)
    uint16_t  name[36];
} gpt_partition_t;


typedef struct gpt_header_
{
    // GPT signature 0x5452415020494645
    uint64_t signature;
    // GPT version - 1.0
    uint32_t revision;
    // size of this struct in bytes. Must be > 92 and < logical sctor size
    uint32_t header_size;
    // CRC32 of this struct
    uint32_t crc_header;
    // zero'ed
    uint32_t reserved;
    // lba of this struct
    uint64_t current_lba;
    // lba of the backup struct
    uint64_t backup_lba;
    // first lba that can be used by a partition entry
    uint64_t first_usable_lba;
    // last lba that can be used by a partition entry
    uint64_t last_usable_lba;
    // unique id for the disk
    uuid_t   disk_guid;
    // lba address to the first partition entry
    uint64_t starting_lba;
    // number of partitions
    uint32_t number_partitions;
    // size of each partition entry (128 x 2^N - today is 128)
    uint32_t size_partition_entries;
    // CRC32 for the partitions (number_partitions * size_partition_entries)
    uint32_t crc_partition;
    // 420 bytes for a disc with 512b of sector size...
    // it should be sector size - 92
    unsigned char reserved2[420];
} gpt_header_t;


/**********************************************************************
 * Disk utilities
 *********************************************************************/
uint64_t get_disk_size(const char *device)
{
    long size;

    int fd = open(device, O_RDONLY);
    if (fd == -1)
        exit(2);

    if (ioctl(fd, BLKGETSIZE, &size))
        exit(3);

    close(fd);

    return (uint64_t)size;
}


void partition_write(const char *device, gpt_header_t *header, gpt_partition_t *partitions)
{
    int fd = open(device, O_WRONLY, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);

    // write the partition table first, move the offset to LBA 1 and write
    // the array
    lseek64(fd, SECTOR_SIZE * 2, SEEK_SET);
    write(fd, partitions, header->size_partition_entries * header->number_partitions);

    // move the offset to LBA 0 and write the header
    lseek64(fd, 512, SEEK_SET);
    write(fd, header, 512);

    close(fd);
}


/**********************************************************************
 * Partitioning code
 *********************************************************************/
void createPartitions(gpt_partition_t *partitions)
{
    uuid_t tmp;

    /**** first partition - SWAP *************************************/
    // set the swap uuid
    uuid_parse(SWAP_UUID, tmp);
    uuid_copy(partitions[0].partition_type, tmp);
    uuid_clear(tmp);

    // set an unique id for this partition
    uuid_generate(tmp);
    uuid_copy(partitions[0].unique_partition, tmp);
    uuid_clear(tmp);

    // start sector - the first partition could start on sector 34, 
    // however to achieve the optimum alignment it starts on 2048 (1MB)
    partitions[0].first_lba  = 2048;

    // end sector - 1GB
    partitions[0].last_lba   = 2099199;

    // no attributes
    partitions[0].attributes = UINT64_C(0);

    // name - too lazy to add UTF16 support
    int tmpi = 0x0053; // S
    memcpy(&partitions[0].name[0], &tmpi, 2);
    tmpi = 0x0077; // w
    memcpy(&partitions[0].name[1], &tmpi, 2);
    tmpi = 0x0061; // a
    memcpy(&partitions[0].name[2], &tmpi, 2);
    tmpi = 0x0070; // p
    memcpy(&partitions[0].name[3], &tmpi, 2);
    tmpi = 0x0000; // /0
    memcpy(&partitions[0].name[4], &tmpi, 2);

    /**** second partition - LINUX ***********************************/
    // set the swap uuid
    uuid_parse(LINUX_UUID, tmp);
    uuid_copy(partitions[1].partition_type, tmp);
    uuid_clear(tmp);

    // set an unique id for this partition
    uuid_generate(tmp);
    uuid_copy(partitions[1].unique_partition, tmp);
    uuid_clear(tmp);

    // start sector - starts just after last partition sector
    partitions[1].first_lba  = partitions[0].last_lba + 1;

    // end sector - all free space
    // it's the disk size (in sectors) - 34 sectors of the backup
    // GPT table:
    //    10 GB = 10 * 1024 ^ 3 / 512 = 20971520 sectors
    //    20971520 - 34 = 20971486
    partitions[1].last_lba   = 20971486;

    // mbr boot flag (legacy boot)
    partitions[1].attributes = UINT64_C(0x4);

    // name - too lazy to add UTF16 support
    tmpi = 0x0052; // P
    memcpy(&partitions[1].name[0], &tmpi, 2);
    tmpi = 0x006F; // a
    memcpy(&partitions[1].name[1], &tmpi, 2);
    tmpi = 0x006F; // r
    memcpy(&partitions[1].name[2], &tmpi, 2);
    tmpi = 0x0074; // t
    memcpy(&partitions[1].name[3], &tmpi, 2);
    tmpi = 0x0000; // /0
    memcpy(&partitions[1].name[4], &tmpi, 2);
}

void initGPTHeader(gpt_header_t *gpt, uint64_t diskSize, gpt_partition_t *partitions)
{
    // https://en.wikipedia.org/wiki/GUID_Partition_Table
    // little endian format
    // GPT Signature
    gpt->signature         = 0x5452415020494645;
    // 1.0 version
    gpt->revision          = 0x00010000;
    // 512 bytes header size
    gpt->header_size       = UINT32_C(512);
    // 0'ed revered field
    gpt->reserved          = UINT32_C(0);
    // primary header lba
    gpt->current_lba       = UINT64_C(1);
    // backup header lba
    gpt->backup_lba        = UINT64_C(diskSize - 1);
    // number of partitions
    gpt->number_partitions = UINT32_C(2);

    // first/last usable lba's
    gpt->first_usable_lba = partitions[0].first_lba;
    gpt->last_usable_lba  = partitions[1].last_lba;

    // first partition entry address
    gpt->starting_lba     = UINT64_C(2);

    // init CRC to 0
    gpt->crc_header       = UINT32_C(0);
    gpt->crc_partition    = UINT32_C(0);

    // create unique id for the disk
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_copy(gpt->disk_guid, uuid);

    // today' size of each partition entry
    gpt->size_partition_entries = UINT32_C(128);

    // 0'ed last bytes...
    memset(gpt->reserved2, '\0', 420);

    // calculate the CRC32 of partitions entries
    gpt->crc_partition = crc32(0, (unsigned char*)partitions, gpt->number_partitions * gpt->size_partition_entries);

    // calculate the CRC32 of this header
    gpt->header_size = sizeof(*gpt);
    gpt->crc_header = crc32(0, (unsigned char*)gpt, gpt->header_size);
}

void print_gpt(gpt_header_t *gpt)
{
    printf("---------- GPT HEADER ----------\n");
    printf("GPT signature: %" PRIu64 "\n", gpt->signature);
    printf("GPT revision: %" PRIu32 "\n", gpt->revision);
    printf("GPT size: %" PRIu32 "\n", gpt->header_size);
    printf("GPT part size: %" PRIu32 "\n", gpt->size_partition_entries);
    printf("GPT current lba: %" PRIu64 "\n", gpt->current_lba);
    printf("GPT backup lba: %" PRIu64 "\n", gpt->backup_lba);
    printf("GPT n partiions: %" PRIu32 "\n", gpt->number_partitions);
    printf("GPT first lba: %" PRIu64 "\n", gpt->first_usable_lba);
    printf("GPT last lba: %" PRIu64 "\n", gpt->last_usable_lba);
    printf("GPT starting lba: %" PRIu64 "\n", gpt->starting_lba);
    printf("GPT CRC header: %" PRIu32 "\n", gpt->crc_header);
    printf("GPT CRC partitions: %" PRIu32 "\n", gpt->crc_partition);
    printf("GPT size partition entry: %" PRIu32 "\n", gpt->size_partition_entries);

    char suuid[37]; // 36 bytes + /0
    uuid_unparse(gpt->disk_guid, suuid);
    printf("Disk UUID: %s\n", suuid);

    printf("---------- %zu bytes ----------\n", sizeof(*gpt));
}

void print_partitions(gpt_partition_t *partitions, uint32_t number)
{
    printf("    ---------- GPT PARTITIONS ----------\n");
    while (number--)
    {
        printf("\t[%d] ----------\n", number);
        printf("\tpartitions first lba: %" PRIu64 "\n", partitions[number].first_lba);
        printf("\tpartitions last lba: %" PRIu64 "\n", partitions[number].last_lba);
        printf("\tpartitions attributes: %" PRIu64 "\n", partitions[number].attributes);

        char suuid[37];
        uuid_unparse(partitions[number].partition_type, suuid);
        printf("\tType UUID: %s\n", suuid);

        uuid_unparse(partitions[number].unique_partition, suuid);
        printf("\tUnique UUID: %s\n", suuid);
        printf("\t--------------\n");
    }
}

int main(int argc, char *argv[])
{
    // basic assertions
    assert(sizeof(gpt_partition_t) == 128);
    assert(sizeof(gpt_header_t)    == 512);

    // require root privilege
    if (geteuid() != 0)
    {
        printf("Must run as root user.\n");
        exit(4);
    }

    // get disk size
    uint64_t size = get_disk_size("/dev/sdb");

    // create partitions entry
    gpt_partition_t partitions[2];
    createPartitions(&partitions[0]);

    // create the header
    gpt_header_t gpt;
    initGPTHeader(&gpt, size, &partitions[0]);

    // save in disk
    partition_write("/dev/sdb", &gpt, &partitions[0]);

    return 0;
}
