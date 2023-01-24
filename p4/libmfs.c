#include "mfs.h"
#include "udp.h"

// socket descriptor
int sd = -1;
// socket address
struct sockaddr_in *sockaddr = NULL;

/*
Takes a host name and port number and uses those
to find the server exporting the file system.
*/
int MFS_Init(char *hostname, int port)
{
    if ((sd = UDP_Open(0)) < 0)
    {
        return -1;
    }

    sockaddr = malloc(sizeof(struct sockaddr_in));
    if (UDP_FillSockAddr(sockaddr, hostname, port) == -1)
    {
        free(sockaddr);
        sockaddr = NULL;
        return -1;
    }

    // set UDP timeout to 5 seconds
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        return -1;
    }

    return 0;
}

/*
Takes the parent inode number (which should be the inode number of a directory)
and looks up the entry name in it. The inode number of name is returned.
Success: return inode number of name
Failure: return -1. Failure modes: invalid pinum, name does not exist in pinum.
*/
int MFS_Lookup(int pinum, char *name)
{
    if (sd == -1 || sockaddr == NULL || name == NULL || strlen(name) > 28)
    {
        return -1;
    }
    MFS_Msg_t msg;
    msg.type = MFS_LOOKUP;
    msg.inum = pinum;
    strcpy(msg.name, name);
    while (1)
    {
        UDP_Write(sd, sockaddr, (char *)&msg, sizeof(MFS_Msg_t));
        MFS_Res_t res;
        int rc = UDP_Read(sd, sockaddr, (char *)&res, sizeof(MFS_Res_t));
        if (rc < 0)
        {
            continue;
        }
        return res.ret;
    }

    return -1;
}

/*
Returns some information about the file specified by inum
Success: return 0
Failure: return -1. Failure modes: inum does not exist.
*/
int MFS_Stat(int inum, MFS_Stat_t *m)
{
    if (sd == -1 || sockaddr == NULL)
    {
        return -1;
    }

    MFS_Msg_t msg;
    msg.type = MFS_STAT;
    msg.inum = inum;

    while (1)
    {
        UDP_Write(sd, sockaddr, (char *)&msg, sizeof(MFS_Msg_t));
        MFS_Res_t res;
        int rc = UDP_Read(sd, sockaddr, (char *)&res, sizeof(MFS_Res_t));
        if (rc < 0)
        {
            continue;
        }
        memcpy(m, &res.stat, sizeof(MFS_Stat_t));
        return 0;
    }

    return -1;
}

/*
Writes a buffer of size nbytes (max size: 4096 bytes) at the byte offset specified by offset.
Success: return 0
Failure: return -1. Failure modes: invalid inum, invalid nbytes, invalid offset, not a regular file
*/
int MFS_Write(int inum, char *buffer, int offset, int nbytes)
{
    if (sd == -1 || sockaddr == NULL)
    {
        return -1;
    }

    MFS_Msg_t msg;
    msg.type = MFS_WRITE;
    msg.inum = inum;
    msg.offset = offset;
    msg.nbytes = nbytes;
    memcpy(&msg.buf, buffer, MFS_BLOCK_SIZE);
    // memcpy(&msg.buf, buffer, nbytes);
    while (1)
    {
        UDP_Write(sd, sockaddr, (char *)&msg, sizeof(MFS_Msg_t));
        MFS_Res_t res;
        int rc = UDP_Read(sd, sockaddr, (char *)&res, sizeof(MFS_Res_t));
        if (rc < 0)
        {
            continue;
        }
        return 0;
    }
    return -1;
}

/*
Reads nbytes of data (max size 4096 bytes) specified by the byte offset offset
into the buffer from file specified by inum.
Success: return 0
Failure: return -1. Failure modes: invalid inum, invalid offset, invalid nbytes
 */
int MFS_Read(int inum, char *buffer, int offset, int nbytes)
{
    if (sd == -1 || sockaddr == NULL)
    {
        return -1;
    }

    MFS_Msg_t msg;
    msg.type = MFS_READ;
    msg.inum = inum;
    msg.offset = offset;
    msg.nbytes = nbytes;

    while (1)
    {
        UDP_Write(sd, sockaddr, (char *)&msg, sizeof(MFS_Msg_t));
        MFS_Res_t res;
        int rc = UDP_Read(sd, sockaddr, (char *)&res, sizeof(MFS_Res_t));
        if (rc < 0)
        {
            continue;
        }
        memcpy(buffer, &res.buf, MFS_BLOCK_SIZE);
        // memcpy(buffer, &res.buf, nbytes);
        return 0;
    }
    return -1;
}

/*
Makes a file (type == MFS_REGULAR_FILE) or directory (type == MFS_DIRECTORY)
in the parent directory specified by pinum of name name
Success: return 0
Failure: return -1. Failure modes: pinum does not exist, or name is too long. If name already exists, return success.
*/
int MFS_Creat(int pinum, int type, char *name)
{
    if (sd == -1 || sockaddr == NULL || name == NULL || strlen(name) > 28)
    {
        return -1;
    }

    MFS_Msg_t msg;
    msg.type = MFS_CREAT;
    msg.inum = pinum;
    msg.filetype = type;
    strcpy(msg.name, name);
    while (1)
    {
        UDP_Write(sd, sockaddr, (char *)&msg, sizeof(MFS_Msg_t));
        MFS_Res_t res;
        int rc = UDP_Read(sd, sockaddr, (char *)&res, sizeof(MFS_Res_t));
        if (rc < 0)
        {
            continue;
        }
        return 0;
    }

    return -1;
}

/*
Removes the file or directory name from the directory specified by pinum
Success: return 0
Failure: return -1, Failure modes: pinum does not exist, directory is NOT empty.
Note that the name not existing is NOT a failure by our definition
*/
int MFS_Unlink(int pinum, char *name)
{
    if (sd == -1 || sockaddr == NULL || name == NULL || strlen(name) > 28)
    {
        return -1;
    }

    MFS_Msg_t msg;
    msg.type = MFS_UNLINK;
    msg.inum = pinum;
    strcpy(msg.name, name);

    while (1)
    {
        UDP_Write(sd, sockaddr, (char *)&msg, sizeof(MFS_Msg_t));
        MFS_Res_t res;
        int rc = UDP_Read(sd, sockaddr, (char *)&res, sizeof(MFS_Res_t));
        if (rc < 0)
        {
            continue;
        }
        return 0;
    }
    return -1;
}

/*
Tells the server to force all of its data structures to disk and shutdown by calling exit(0).
*/
int MFS_Shutdown()
{
    if (sd == -1 || sockaddr == NULL)
    {
        return -1;
    }

    MFS_Msg_t msg;
    msg.type = MFS_SHUTDOWN;
    UDP_Write(sd, sockaddr, (char *)&msg, sizeof(MFS_Msg_t));
    return 0;
}