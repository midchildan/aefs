/* fileio.c -- File I/O.
   Copyright (C) 1999 Eelco Dolstra (edolstra@students.cs.uu.nl).

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <assert.h>

#include "aefsdmn.h"


APIRET fsRead(ServerData * pServerData, struct read * pread)
{
   CoreResult cr;
   CryptedVolume * pVolume = pread->pVolData->pVolume;
   CryptedFilePos cbRead;
   ULONG cbLen = pread->cbLen;

   pread->cbLen = 0; /* on output, # of bytes read */ 

   logMsg(L_DBG,
      "FS_READ, cbLen=%hu, fsIOFlag=%04hx",
      cbLen, pread->fsIOFlag);
   logsffsi(&pread->sffsi);

   cr = coreReadFromFile(pVolume, pread->pOpenFileData->idFile,
      pread->sffsi.sfi_position, cbLen,
      (octet *) pServerData->pData, &cbRead);
   if (cr) return coreResultToOS2(cr);

   assert(cbRead <= cbLen);

   logMsg(L_DBG, "cbRead=%ld", cbRead);

   pread->cbLen = (USHORT) cbRead;
   pread->sffsi.sfi_position += cbRead;
   pread->sffsi.sfi_tstamp |= (ST_SREAD | ST_PREAD);

   return NO_ERROR;
}


APIRET fsWrite(ServerData * pServerData, struct write * pwrite)
{
   CoreResult cr;
   CryptedVolume * pVolume = pwrite->pVolData->pVolume;
   CryptedFilePos cbWritten;
   ULONG cbLen = pwrite->cbLen;

   pwrite->cbLen = 0; /* on output, # of bytes written */ 

   logMsg(L_DBG,
      "FS_WRITE, cbLen=%hu, fsIOFlag=%04hx",
      cbLen, pwrite->fsIOFlag);
   logsffsi(&pwrite->sffsi);

   if (pwrite->pVolData->fReadOnly) return ERROR_WRITE_PROTECT;

   cr = coreWriteToFile(pVolume, pwrite->pOpenFileData->idFile,
      pwrite->sffsi.sfi_position, cbLen,
      (octet *) pServerData->pData, &cbWritten);
   if (cr) return coreResultToOS2(cr);

   assert(cbWritten <= 0xffff && cbLen == cbWritten);

   logMsg(L_DBG, "cbWritten=%ld", cbWritten);

   pwrite->cbLen = (USHORT) cbWritten;
   pwrite->sffsi.sfi_position += cbWritten;
   pwrite->sffsi.sfi_tstamp |= (ST_SWRITE | ST_PWRITE);

   return NO_ERROR;
}


APIRET fsChgFilePtr(ServerData * pServerData,
   struct chgfileptr * pchgfileptr)
{
   LONG lNewPos;
   
   logMsg(L_DBG,
      "FS_CHGFILEPTR, ibOffset=%ld, usType=%hu, fsIOFlag=%04hx",
      pchgfileptr->ibOffset, pchgfileptr->usType, pchgfileptr->fsIOFlag);
   logsffsi(&pchgfileptr->sffsi);

   switch (pchgfileptr->usType) {

      case FILE_BEGIN:
         pchgfileptr->sffsi.sfi_position = pchgfileptr->ibOffset;
         return NO_ERROR;

      case FILE_CURRENT:
         lNewPos = (LONG) pchgfileptr->sffsi.sfi_position +
            pchgfileptr->ibOffset;
         if (lNewPos < 0)
            return ERROR_NEGATIVE_SEEK;
         pchgfileptr->sffsi.sfi_position = lNewPos;
         return NO_ERROR;

      case FILE_END:
         lNewPos = (LONG) pchgfileptr->sffsi.sfi_size +
            pchgfileptr->ibOffset;
         if (lNewPos < 0)
            return ERROR_NEGATIVE_SEEK;
         pchgfileptr->sffsi.sfi_position = lNewPos;
         return NO_ERROR;

      default:
         logMsg(L_EVIL, "unknown FS_CHGFILEPTR type: %d",
            pchgfileptr->usType);
         return ERROR_NOT_SUPPORTED;
   }
}


APIRET fsNewSize(ServerData * pServerData, struct newsize * pnewsize)
{
   CoreResult cr;
   CryptedVolume * pVolume = pnewsize->pVolData->pVolume;
   
   logMsg(L_DBG,
      "FS_NEWSIZE, cbLen=%ld, fsIOFlag=%04hx",
      pnewsize->cbLen, pnewsize->fsIOFlag);
   logsffsi(&pnewsize->sffsi);

   if (pnewsize->pVolData->fReadOnly) return ERROR_WRITE_PROTECT;
   cr = coreSetFileSize(pVolume, pnewsize->pOpenFileData->idFile,
      pnewsize->cbLen);
   if (cr) return coreResultToOS2(cr);

   pnewsize->sffsi.sfi_size = pnewsize->cbLen;

   return NO_ERROR;
}


APIRET fsCommit(ServerData * pServerData, struct commit * pcommit)
{
   logMsg(L_DBG, "FS_COMMIT, usType=%hd, fsIOFlag=%hx",
      pcommit->usType, pcommit->fsIOFlag);
   logsffsi(&pcommit->sffsi);
   
   return stampFileAndFlush(pcommit->pVolData,
      pcommit->pOpenFileData->idFile, &pcommit->sffsi, SFAF_FLUSHALL);
}


APIRET fsIOCtl(ServerData * pServerData, struct ioctl * pioctl)
{
   logMsg(L_DBG,
      "FS_IOCTL, usCat=%hd, usFunc=%hd, cbParm=%d, cbMaxData=%d",
      pioctl->usCat, pioctl->usFunc, pioctl->cbParm,
      pioctl->cbMaxData);
   logsffsi(&pioctl->sffsi);

   return ERROR_NOT_SUPPORTED;
}
