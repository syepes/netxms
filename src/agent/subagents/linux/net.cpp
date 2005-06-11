/* $Id: net.cpp,v 1.6 2005-06-11 16:28:24 victor Exp $ */

/* 
** NetXMS subagent for GNU/Linux
** Copyright (C) 2004 Alex Kirhenshtein
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**/

#include <nms_common.h>
#include <nms_agent.h>

#include <linux/sysctl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "net.h"


LONG H_NetIpForwarding(char *pszParam, char *pArg, char *pValue)
{
	int nVer = (int)pArg;
	int nRet = SYSINFO_RC_ERROR;
	FILE *hFile;
	char *pFileName = NULL;

	switch (nVer)
	{
	case 4:
		pFileName = "/proc/sys/net/ipv4/conf/all/forwarding";
		break;
	case 6:
		pFileName = "/proc/sys/net/ipv6/conf/all/forwarding";
		break;
	}

	if (pFileName != NULL)
	{
		hFile = fopen(pFileName, "r");
		if (hFile != NULL)
		{
			char szBuff[4];

			if (fgets(szBuff, sizeof(szBuff), hFile) != NULL)
			{
				nRet = SYSINFO_RC_SUCCESS;
				switch (szBuff[0])
				{
				case '0':
					ret_int(pValue, 0);
					break;
				case '1':
					ret_int(pValue, 1);
					break;
				default:
					nRet = SYSINFO_RC_ERROR;
					break;
				}
			}
			fclose(hFile);
		}
	}

	return nRet;
}

LONG H_NetArpCache(char *pszParam, char *pArg, NETXMS_VALUES_LIST *pValue)
{
	int nRet = SYSINFO_RC_ERROR;
	FILE *hFile;

	hFile = fopen("/proc/net/arp", "r");
	if (hFile != NULL)
	{
		char szBuff[256];
		int nFd;

		nFd = socket(AF_INET, SOCK_DGRAM, 0);
		if (nFd > 0)
		{
			nRet = SYSINFO_RC_SUCCESS;

			fgets(szBuff, sizeof(szBuff), hFile); // skip first line

			while (fgets(szBuff, sizeof(szBuff), hFile) != NULL)
			{
				int nIP1, nIP2, nIP3, nIP4;
				int nMAC1, nMAC2, nMAC3, nMAC4, nMAC5, nMAC6;
				char szTmp1[256];
				char szTmp2[256];
				char szTmp3[256];
				char szIf[256];

				if (sscanf(szBuff,
						"%d.%d.%d.%d %s %s %02X:%02X:%02X:%02X:%02X:%02X %s %s",
						&nIP1, &nIP2, &nIP3, &nIP4,
						szTmp1, szTmp2,
						&nMAC1, &nMAC2, &nMAC3, &nMAC4, &nMAC5, &nMAC6,
						szTmp3, szIf) == 14)
				{
					int nIndex;
					struct ifreq irq;

					if (nMAC1 == 0 && nMAC2 == 0 &&
						nMAC3 == 0 && nMAC4 == 0 &&
						nMAC5 == 0 && nMAC6 == 0)
					{
						// incomplete
						continue;
					}

					strncpy(irq.ifr_name, szIf, IFNAMSIZ);
					if (ioctl(nFd, SIOCGIFINDEX, &irq) != 0)
					{
						perror("ioctl()");
						nIndex = 0;
					}
					else
					{
						nIndex = irq.ifr_ifindex;
					}
					
					snprintf(szBuff, sizeof(szBuff),
							"%02X%02X%02X%02X%02X%02X %d.%d.%d.%d %d",
							nMAC1, nMAC2, nMAC3, nMAC4, nMAC5, nMAC6,
							nIP1, nIP2, nIP3, nIP4,
							nIndex);

					NxAddResultString(pValue, szBuff);
				}
			}

			close(nFd);
		}
		
		fclose(hFile);
	}

	return nRet;
}

LONG H_NetIfList(char *pszParam, char *pArg, NETXMS_VALUES_LIST *pValue)
{
	int nRet = SYSINFO_RC_ERROR;
   struct if_nameindex *pIndex;
   struct ifreq irq;
   struct sockaddr_in *sa;
	int nFd;

	pIndex = if_nameindex();
	if (pIndex != NULL)
	{
		nFd = socket(AF_INET, SOCK_DGRAM, 0);
		if (nFd >= 0)
		{
			for (int i = 0; pIndex[i].if_index != 0; i++)
			{
				char szOut[256];
				struct sockaddr_in *sa;
				struct in_addr in;
				char szIpAddr[32];
				char szMacAddr[32];
				int nMask;

				nRet = SYSINFO_RC_SUCCESS;

				strcpy(irq.ifr_name, pIndex[i].if_name);
				if (ioctl(nFd, SIOCGIFADDR, &irq) == 0)
				{
					sa = (struct sockaddr_in *)&irq.ifr_addr;
					strncpy(szIpAddr, inet_ntoa(sa->sin_addr), sizeof(szIpAddr));
				}
				else
				{
					nRet = SYSINFO_RC_ERROR;
				}

				if (nRet == SYSINFO_RC_SUCCESS)
				{
					if (ioctl(nFd, SIOCGIFNETMASK, &irq) == 0)
					{
						sa = (struct sockaddr_in *)&irq.ifr_addr;
						nMask = BitsInMask(htonl(sa->sin_addr.s_addr));
					}
				}
				else
				{
					nRet = SYSINFO_RC_ERROR;
				}

				if (nRet == SYSINFO_RC_SUCCESS)
				{
					if (ioctl(nFd, SIOCGIFHWADDR, &irq) == 0)
					{
						for (int z = 0; z < 6; z++)
						{
							sprintf(&szMacAddr[z << 1], "%02X",
									(unsigned char)irq.ifr_hwaddr.sa_data[z]);
						}
					}
               else
               {
					   nRet = SYSINFO_RC_ERROR;
               }
				}
				else
				{
					nRet = SYSINFO_RC_ERROR;
				}

				if (nRet == SYSINFO_RC_SUCCESS)
				{
					snprintf(szOut, sizeof(szOut), "%d %s/%d %d %s %s",
							pIndex[i].if_index,
							szIpAddr,
							nMask,
							IFTYPE_OTHER,
							szMacAddr,
							pIndex[i].if_name);
					NxAddResultString(pValue, szOut);
				}
            else
            {
               break;   // Stop enumerating interfaces on error
            }
			}

			close(nFd);
		}
      if_freenameindex(pIndex);
	}

	return nRet;
}

LONG H_NetIfInfoFromIOCTL(char *pszParam, char *pArg, char *pValue)
{
   char *eptr, szBuffer[256];
   LONG nRet = SYSINFO_RC_SUCCESS;
   struct ifreq ifr;
   int fd;

   if (!NxGetParameterArg(pszParam, 1, szBuffer, 256))
      return SYSINFO_RC_UNSUPPORTED;

   fd = socket(AF_INET, SOCK_DGRAM, 0);
   if (fd == -1)
   {
      return 1;
   }

   // Check if we have interface name or index
   ifr.ifr_ifindex = strtol(szBuffer, &eptr, 10);
   if (*eptr == 0)
   {
      // Index passed as argument, convert to name
      if (ioctl(fd, SIOCGIFNAME, &ifr) != 0)
         nRet = SYSINFO_RC_ERROR;
   }
   else
   {
      // Name passed as argument
      strncpy(ifr.ifr_name, szBuffer, IFNAMSIZ);
   }

   // Get interface information
   if (nRet == SYSINFO_RC_SUCCESS)
   {
      switch((int)pArg)
      {
         case IF_INFO_ADMIN_STATUS:
            if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0)
            {
               ret_int(pValue, (ifr.ifr_flags & IFF_UP) ? 1 : 0);
            }
            else
            {
               nRet = SYSINFO_RC_ERROR;
            }
            break;
         case IF_INFO_OPER_STATUS:
            if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0)
            {
               // IFF_RUNNING should be set only if interface can
               // transmit/receive data, but in fact looks like it
               // always set. I have unverified information that
               // newer kernels set this flag correctly.
               ret_int(pValue, (ifr.ifr_flags & IFF_RUNNING) ? 1 : 0);
            }
            else
            {
               nRet = SYSINFO_RC_ERROR;
            }
            break;
         case IF_INFO_DESCRIPTION:
            ret_string(pValue, ifr.ifr_name);
            break;
         default:
            nRet = SYSINFO_RC_UNSUPPORTED;
            break;
      }
   }

   // Cleanup
   close(fd);

   return nRet;
}

static LONG ValueFromLine(char *pszLine, int nPos, char *pValue)
{
   int i;
   char *eptr, *pszWord, szBuffer[256];
   DWORD dwValue;
   LONG nRet = SYSINFO_RC_ERROR;

   for(i = 0, pszWord = pszLine; i <= nPos; i++)
      pszWord = ExtractWord(pszWord, szBuffer);
   dwValue = strtoul(szBuffer, &eptr, 0);
   if (*eptr == 0)
   {
      ret_uint(pValue, dwValue);
      nRet = SYSINFO_RC_SUCCESS;
   }
   return nRet;
}

LONG H_NetIfInfoFromProc(char *pszParam, char *pArg, char *pValue)
{
   char *ptr, szBuffer[256], szName[IFNAMSIZ];
   LONG nIndex, nRet = SYSINFO_RC_SUCCESS;
   FILE *fp;

   if (!NxGetParameterArg(pszParam, 1, szBuffer, 256))
      return SYSINFO_RC_UNSUPPORTED;

   // Check if we have interface name or index
   nIndex = strtol(szBuffer, &ptr, 10);
   if (*ptr == 0)
   {
      // Index passed as argument, convert to name
      if (if_indextoname(nIndex, szName) == NULL)
         nRet = SYSINFO_RC_ERROR;
   }
   else
   {
      // Name passed as argument
      strncpy(szName, szBuffer, IFNAMSIZ);
   }

   // Get interface information
   if (nRet == SYSINFO_RC_SUCCESS)
   {
      // If name is an alias (i.e. eth0:1), remove alias number
      ptr = strchr(szName, ':');
      if (ptr != NULL)
         *ptr = 0;

      fp = fopen("/proc/net/dev", "r");
      if (fp != NULL)
      {
         while(1)
         {
            fgets(szBuffer, 256, fp);
            if (feof(fp))
            {
               nRet = SYSINFO_RC_ERROR;   // Interface record not found
               break;
            }

            // We expect line in form interface:stats
            StrStrip(szBuffer);
            ptr = strchr(szBuffer, ':');
            if (ptr == NULL)
               continue;
            *ptr = 0;

            if (!stricmp(szBuffer, szName))
            {
               ptr++;
               break;
            }
         }
         fclose(fp);
      }
      else
      {
         nRet = SYSINFO_RC_ERROR;
      }

      if (nRet == SYSINFO_RC_SUCCESS)
      {
         StrStrip(ptr);
         switch((int)pArg)
         {
            case IF_INFO_BYTES_IN:
               nRet = ValueFromLine(ptr, 0, pValue);
               break;
            case IF_INFO_PACKETS_IN:
               nRet = ValueFromLine(ptr, 1, pValue);
               break;
            case IF_INFO_IN_ERRORS:
               nRet = ValueFromLine(ptr, 2, pValue);
               break;
            case IF_INFO_BYTES_OUT:
               nRet = ValueFromLine(ptr, 8, pValue);
               break;
            case IF_INFO_PACKETS_OUT:
               nRet = ValueFromLine(ptr, 9, pValue);
               break;
            case IF_INFO_OUT_ERRORS:
               nRet = ValueFromLine(ptr, 10, pValue);
               break;
            default:
               nRet = SYSINFO_RC_UNSUPPORTED;
               break;
         }
      }
   }

   return nRet;
}

///////////////////////////////////////////////////////////////////////////////
/*

$Log: not supported by cvs2svn $
Revision 1.5  2005/06/09 12:15:43  victor
Added support for Net.Interface.AdminStatus and Net.Interface.Link parameters

Revision 1.4  2005/01/05 12:21:24  victor
- Added wrappers for new and delete from gcc2 libraries
- sys/stat.h and fcntl.h included in nms_common.h

Revision 1.3  2004/11/25 08:01:27  victor
Processing of interface list will be stopped on error

Revision 1.2  2004/10/23 22:53:23  alk
ArpCache: ignore incomplete entries

Revision 1.1  2004/10/22 22:08:34  alk
source restructured;
implemented:
	Net.IP.Forwarding
	Net.IP6.Forwarding
	Process.Count(*)
	Net.ArpCache
	Net.InterfaceList (if-type not implemented yet)
	System.ProcessList


*/
