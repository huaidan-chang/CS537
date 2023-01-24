#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include "udp.h"
#include "mfs.h"
#include "ufs.h"

int fd;
int sd;
void* image;
int image_size;
super_t* s;
void* inode_bitmap;
void* data_bitmap;
void* inode_region;
void* data_region;
inode_t *inode_table;


unsigned long get_bit(unsigned long *bitmap, int position) {
   int index = position / 32;
   int offset = 31 - (position % 32);
   return (bitmap[index] >> offset) & 0x1;
}

void set_bit(unsigned long *bitmap, int position) {
   int index = position / 32;
   int offset = 31 - (position % 32);
   bitmap[index] |= 0x1 << offset;
}

void clear_bit(unsigned int *bitmap, int position)
{
    int index = position / 32;
    int offset = 31 - (position % 32);
    bitmap[index] &= ~(0x1 << offset);
}

inode_t *findInode(int inum)
{
    printf("findInode inum: %d\n", inum);
    // check if inum is valid
    if (inum < 0 && inum >= s->num_inodes)
        return NULL;
    // check inode bitmap if inode is valid
    if (!get_bit((unsigned long *)inode_bitmap, inum))
        return NULL;

    inode_t *inode = &inode_table[inum];
    printf("findInode\n");
    return inode;
}

int MFS_Stat(int inum, MFS_Stat_t *m){
    printf("MFS_Stat-1\n");
    inode_t *inode = findInode(inum);
    if (inode == NULL)
        return -1;
    m->size = inode->size;
    m->type = inode->type;
    printf("MFS_Stat-2\n");
    // free(inode);
    return 0;
}

int MFS_Lookup(int pinum, char* name){
   inode_t *pinode = findInode(pinum);
    // check if pinum is a directory
    if (pinode == NULL || pinode->type != MFS_DIRECTORY)
    {
        return -1;
    }

    // lookup all direct entries
    for (int i = 0; i < DIRECT_PTRS; i++)
    {
        if (pinode->direct[i] == -1)
            continue;
        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j++)
        {
            // not sure if entry address needs to add image start position.
            // dir_ent_t *entry = (dir_ent_t *)((pinode->direct[i] * UFS_BLOCK_SIZE) + sizeof(dir_ent_t) * j);
            dir_ent_t *entry = (dir_ent_t *)(image + (pinode->direct[i] * UFS_BLOCK_SIZE) + sizeof(dir_ent_t) * j);
            if (strcmp(name, entry->name) == 0)
            {
                return entry->inum;
            }
        }
    }

    return -1;
}

/*
Allocate data block
*/
int data_block_allocate()
{
    for (int i = 0; i < s->num_data; i++)
    {
        // check data bitmap if data block is unused
        if (!get_bit((unsigned long *)data_bitmap, i))
        {
            set_bit((unsigned long *)data_bitmap, i);
            return s->data_region_addr + i;
        }
    }
    return -1;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes){
    if (inum < 0 || offset < 0 || nbytes > MFS_BLOCK_SIZE){
        printf("MFS_Write-1\n");
        return -1;
    }
    inode_t *inode = findInode(inum);
    if (inode == NULL || inode->type == MFS_DIRECTORY || offset > inode->size){
        printf("MFS_Write-2\n");
        return -1;
    }
    printf("MFS_Write-3\n");
    int dir_idx = offset / UFS_BLOCK_SIZE;
    int block_offset = offset % UFS_BLOCK_SIZE;
    int rest_byte = nbytes;
    int write_end = block_offset;
   // int first_block = inode->direct[dir_idx];

    if(inode->direct[dir_idx] == -1){
        int new_data_block = data_block_allocate();
        if(new_data_block != -1){
            inode->direct[dir_idx] = new_data_block;
        } else {
            return -1;
        }
    }

    // int ifSecondBlock = 0;
    // check if second block is needed
    if (UFS_BLOCK_SIZE - block_offset - nbytes < 0)
    {
        if(dir_idx + 1 >= DIRECT_PTRS){
            return -1;
        }
        if(inode->direct[dir_idx + 1] == -1){
            int new_data_block2 = data_block_allocate();
            if(new_data_block2 != -1){
                inode->direct[dir_idx + 1] = new_data_block2;
            } else {
                return -1;
            }
            
        }
    }

    if (!get_bit((unsigned long *)data_bitmap, inode->direct[dir_idx] - s->data_region_addr))
    {
        return -1;
    }

    int i = 0;
    // block start addr
    unsigned long block_addr = (unsigned long)(image + inode->direct[dir_idx] * UFS_BLOCK_SIZE + block_offset);
    while(rest_byte > 0){
        if(write_end < UFS_BLOCK_SIZE){
            // printf("%s", "MFS_Write-2");
            // printf("byte: %s", buffer + i);
            *(char*)(void*) block_addr = *(buffer + i);
            i += 1;
            block_addr += 1;
            rest_byte -= 1;
            write_end += 1;
        } else {
            block_addr = (unsigned long)(image + inode->direct[dir_idx + 1] * UFS_BLOCK_SIZE);
            write_end = 0;
            continue;
        }
    }

    // update inode size
    inode->size += nbytes;

    if(msync(image, image_size, MS_SYNC) == -1){
        return -1;
    }

    fsync(fd);
    return 0;
}

int MFS_Read(int inum, char *buffer, int offset, int nbytes){
    printf("%s", "MFS_Read-0");
    if(nbytes > UFS_BLOCK_SIZE || sizeof(buffer) > MFS_BLOCK_SIZE){
        return -1;
    }
    inode_t *inode = findInode(inum);
    if (inode == NULL)
        return -1;

    // invalid offset + nbytes
    if (offset >= inode->size)
        return -1;

    if(inode->type != MFS_REGULAR_FILE && inode->type != MFS_DIRECTORY){
        return -1;
    }

    int dir_idx = offset / UFS_BLOCK_SIZE;
    int block_offset = offset % UFS_BLOCK_SIZE;
    int rest_byte = nbytes;
    int read_end = block_offset;
    
    printf("%s", "MFS_Read-1");
    int i = 0;
    int j = 1;
    // block start addr
    unsigned long block_addr = (unsigned long)(image + inode->direct[dir_idx] * UFS_BLOCK_SIZE + block_offset);
    while(rest_byte > 0){
        if(read_end < UFS_BLOCK_SIZE){
            // printf("%s", "MFS_Read-2");
            // printf("byte: %s", (char*)(void*) block_addr);
            *(buffer + i) = *(char*)(void*) block_addr;
            i += 1;
            block_addr += 1;
            rest_byte -= 1;
            read_end += 1;
        } else {
            int flag = 0;
            while(dir_idx + j < DIRECT_PTRS && inode->direct[dir_idx + j] != -1){
                if(inode->direct[dir_idx + j] != -1){
                    block_addr = (unsigned long)(image + inode->direct[dir_idx + j] * UFS_BLOCK_SIZE);
                    read_end = 0;
                    flag = 1;
                    break;
                }
                j++;
            }

            if(!flag){
                return -1;
            }
        }
    }
    
    return 0;
}

int find_free_inum(){
    for(int i = 0; i < s->num_inodes; i++){
        int bit = get_bit(inode_bitmap, i);
        if(bit == 0) return i;
    }
    return -1;
}

int find_free_datanum(){
    for(int i = 0; i < s->num_data; i++){
        int bit = get_bit(data_bitmap, i);
        if(bit == 0) return i;
    }
    return -1;
}

int MFS_Creat(int pinum, int type, char *name){
    if (name == NULL || strlen(name) >= 28)
    {
        return -1;
    }

    int name_inode_num = MFS_Lookup(pinum, name);
    if(name_inode_num != -1){
        return 0;
    }

    // check parent inode type
    inode_t *pinode = findInode(pinum);
    if (pinode == NULL || pinode->type != MFS_DIRECTORY)
    {
        printf("MFS_Creat-2\n");
        return -1;
    }

    // locate new entry in the directory
    int dir_idx = 0;
    while((int)pinode->direct[dir_idx] != -1){
        dir_idx++;
    }
    // last valid block
    dir_idx--;

    int rest_entry = (pinode->size % (UFS_BLOCK_SIZE + 1)) / sizeof(dir_ent_t); 
    unsigned long dir_block = pinode->direct[dir_idx]; 
    unsigned long block_addr = dir_block * UFS_BLOCK_SIZE;

    dir_ent_t* entries[rest_entry];
    void* dir_block_addr = (void*)((unsigned long)block_addr + (unsigned long)image); 
    int j;
    for(j = 0; j < rest_entry; j++){
        entries[j] = (dir_ent_t*)dir_block_addr;
        dir_block_addr = (void*)dir_block_addr;
        dir_block_addr += sizeof(dir_ent_t);
    }
    j -= 1;
    entries[rest_entry] = (dir_ent_t*)dir_block_addr;

    if(sizeof(entries) < (UFS_BLOCK_SIZE-32)){
        strcpy(entries[rest_entry]->name, name);
        int free_inum = find_free_inum();
        if(free_inum != -1){
            entries[rest_entry]->inum = free_inum;
        } else {
            return -1;
        }
    }
    
    if(sizeof(entries) >= (UFS_BLOCK_SIZE-32)){
        unsigned long new_block;
        unsigned long new_block_addr;
        int has_place = 0;
        int k;
        for(k = 0; k < s->num_data; k++){
            if(get_bit(data_bitmap, j)) continue;
            new_block = k + s->data_region_addr;
            new_block_addr = new_block * UFS_BLOCK_SIZE;
            break;                   
        }
        
        if(!has_place){
            return -1;
        }
        entries[rest_entry] = (dir_ent_t*)(void*)(unsigned long)new_block_addr;
        strcpy(entries[k+1]->name, name);
        entries[rest_entry]->inum = find_free_inum();
        set_bit(inode_bitmap, find_free_inum());
    }
    pinode->size += sizeof(dir_ent_t);
    int new_inode_inum;
    if(type == MFS_REGULAR_FILE){
        new_inode_inum = find_free_inum();
         if(new_inode_inum == -1){
            return -1;
        }
        inode_t* new_inode = inode_region + new_inode_inum * sizeof(inode_t);
        new_inode->type = MFS_REGULAR_FILE;
        new_inode->size = 0;
        for(int i = 0; i < DIRECT_PTRS; i++){
            new_inode->direct[i] = -1;
        }
        set_bit(inode_bitmap, new_inode_inum);
    }

    if(type == MFS_DIRECTORY){
        new_inode_inum = find_free_inum();
         if(new_inode_inum == -1){
            return -1;
        }
        inode_t* new_inode = inode_region + new_inode_inum * sizeof(inode_t);
        new_inode->type = MFS_DIRECTORY;
        new_inode->size = sizeof(dir_ent_t)*2;
        int new_data_num = find_free_datanum();
        if(new_data_num == -1){
            return -1;
        }
        void* data_addr_ptr = (void*)(unsigned long)(data_region + new_data_num * MFS_BLOCK_SIZE); 
        dir_ent_t* entry1 = (dir_ent_t*)data_addr_ptr;
        entry1->inum = find_free_inum();
        strcpy(entry1->name, ".");
        void* data_addr_ptr2 = (void*)(unsigned long)(data_region + new_data_num * MFS_BLOCK_SIZE + sizeof(dir_ent_t)); 
        dir_ent_t* entry2 = (dir_ent_t*)data_addr_ptr2;
        entry2->inum = pinum;
        strcpy(entry2->name, "..");

        new_inode->direct[0] = new_data_num + s->data_region_addr;
        for(int i = 1; i < DIRECT_PTRS; i++){
            new_inode->direct[i] = -1;
        }
        set_bit(inode_bitmap, new_inode_inum);
        set_bit(data_bitmap, new_data_num);
    }
    if(msync(image, image_size, MS_SYNC) == -1){
        return -1;
    }

    return 0;
}

int MFS_Unlink(int pinum, char *name){
    inode_t *pinode = findInode(pinum);
    if (pinode == NULL || pinode->type != MFS_DIRECTORY)
    {
        return -1;
    }
    int found = -1;
    for (int i = 0; i < DIRECT_PTRS; i++)
    {
        if (pinode->direct[i] == -1)
            continue;

        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j++)
        {
            // not sure if entry address needs to add image start position.
            // dir_ent_t *entry = (dir_ent_t *)((pinode->direct[i] * UFS_BLOCK_SIZE) + sizeof(dir_ent_t) * j);
            dir_ent_t *entry = (dir_ent_t *)(image + (pinode->direct[i] * UFS_BLOCK_SIZE) + sizeof(dir_ent_t) * j);
            if (entry->inum == -1)
                continue;
            if (strcmp(name, entry->name) == 0)
            {
                // child inode
                inode_t *cinode = findInode(entry->inum);

                // if child inode type is directory and size is not 0
                // directory is NOT empty
                // one entry is 32 byte
                // an empty directory has two entry . and ..
                if (cinode->type == MFS_DIRECTORY && cinode->size > 64)
                {
                    // free(pinode);
                    // free(cinode);
                    return -1;
                }

                // free(cinode);
                found = entry->inum;

                // clear data bitmap's data block allocation
                set_bit((unsigned long *)data_bitmap, i);
                for (int k = 0; k < DIRECT_PTRS; k++)
                {
                    if (cinode->direct[k] != -1)
                    {
                        clear_bit((unsigned int *)data_bitmap, cinode->direct[k] - s->data_region_addr);
                    }
                }

                // update inode bitmap's inode block allocation
                clear_bit((unsigned int *)data_bitmap, entry->inum);

                // update pinode's entry
                entry->inum = -1;

                pinode->size -= sizeof(dir_ent_t);
                break;
            }
        }
        if (found != -1)
            break;
    }

    // free(pinode);
    return 0;

}

int MFS_Shutdown()
{
    fsync(fd);
    close(fd);
    exit(0);
}

void intHandler(int dummy) {
        UDP_Close(sd);
        exit(130);
}

// server code
int main(int argc, char *argv[])
{
    // This will make it so that you can safely exit your server using ctrl+c,
    // the interrupt handler will close the socket, then exit like normal.
    signal(SIGINT, intHandler);

    // server [portnum] [file-system-image]
    if (argc != 3)
    {
        printf("usage: server [portnum] [file-system-image]");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    char *fs_img = argv[2];

    // open img file, convert it into ptr
    fd = open(fs_img, O_RDWR);
    assert(fd > -1);

    struct stat sbuf;
    int rc = fstat(fd, &sbuf);
    assert(rc > -1);

    image_size = sbuf.st_size;
    image = mmap(NULL, image_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    assert(image != MAP_FAILED);

    // superblock
    s = (super_t *)image;
    printf("inode bitmap address %d [len %d]\n", s->inode_bitmap_addr, s->inode_bitmap_len);
    printf("data bitmap address %d [len %d]\n", s->data_bitmap_addr, s->data_bitmap_len);

    inode_table = image + (s->inode_region_addr * UFS_BLOCK_SIZE);

    inode_bitmap = image + s->inode_bitmap_addr * UFS_BLOCK_SIZE;
    data_bitmap = image + s->data_bitmap_addr * UFS_BLOCK_SIZE;
    inode_region = image + s->inode_region_addr * UFS_BLOCK_SIZE;
    data_region = image + s->data_region_addr * UFS_BLOCK_SIZE;

    sd = UDP_Open(port);
    assert(sd > -1);
    while (1)
    {
        struct sockaddr_in addr;
        MFS_Msg_t msg;

        // Read data from client
        int rc = UDP_Read(sd, &addr, (char *)&msg, sizeof(MFS_Msg_t));
        if (rc < 0)
        {
            continue;
        }
        // int ret;
        MFS_Res_t res;
        memset(&res, 0, sizeof(MFS_Res_t));
        switch (msg.type)
        {
        case MFS_LOOKUP:
            res.ret = MFS_Lookup(msg.inum, msg.name);
            break;
        case MFS_STAT:
            res.ret = MFS_Stat(msg.inum, &res.stat);
            break;
        case MFS_WRITE:
            res.ret = MFS_Write(msg.inum, msg.buf, msg.offset, msg.nbytes);
            break;
        case MFS_READ:
            res.ret = MFS_Read(msg.inum, res.buf, msg.offset, msg.nbytes);
            break;
        case MFS_CREAT:
            res.ret = MFS_Creat(msg.inum, msg.filetype, msg.name);
            break;
        case MFS_UNLINK:
            res.ret = MFS_Unlink(msg.inum, msg.name);
            break;
        case MFS_SHUTDOWN:
            MFS_Shutdown();
            break;
        default:
            return -1;
        }
        UDP_Write(sd, &addr, (char *)&res, sizeof(MFS_Res_t));
    }
    return 0;
}
