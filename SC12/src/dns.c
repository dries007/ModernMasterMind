/******************************************************************************
 *
 * (c) 2004 by BECK IPC GmbH
 *
 *******************************************************************************
 *
 * Disclaimer: This program is an example and should be used as such.
 *             If you wish to use this program or parts of it in your application,
 *             you must validate the code yourself. BECK IPC GmbH can not be held
 *             responsible for the correct functioning or coding of this example.
 *
 * Module:     dns.c
 * Function:   Get host by name
 *
 *******************************************************************************
 * Written By Ernest Schloesser
 * Copyright BECK IPC GmbH
 *
 * $Id: dns.c 338 2009-11-06 13:39:46Z pib $
 *
 ******************************************************************************/
/*
 You my freely use and change this example, but please state it's origin.

 This example will use a UDP message to the DNS server to obtain the
 numerical address string belonging to a domain name.

 Example: the domain www.beck-ipc.com has the address 195.243.140.4

 First I used the book "TCP/IP Illustrated" by W. Richard Stevens as a source.
 Then I found that theory at praxis are not the same, so I analyzed a
 hex dump of the packet returned. The analysis is included in this source.
 Although I could not understand every byte, I did get the valid IP's for
 microsoft and for beck.

 What is not checked: if the time to live is correct. Could be that it is stored
 in a different byte order then I expected. I got a ttl for Microsoft of 2 minutes....
 */

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "clib.h"
#include "dns.h"

#if (!defined SC123) && (!defined SC23) && (!defined SC243)

/******************************************************************************
 * Defines
 ******************************************************************************/
//#define TRACE   // comment out if you do not want prints of results & progress
//#define MONITOR // use this to show the cute little hex dump!
#define DNS_PORT    53          // Well known DNS port
#define BUFSIZE    600
#define RRBUF      200

/* Define values for RR-types as specified in RFC1035 */
#define DN_TYPE_A     1
#define DN_TYPE_NS    2
#define DN_TYPE_CNAME 5
#define DN_TYPE_MX    15

/* Define values for RR-class as specified in RFC1035 */
#define DN_CLASS_IN   1

/******************************************************************************
 * Variables
 ******************************************************************************/
static unsigned int ident; // ident counter, count up at every request

typedef struct dn_rr_struct
{
  char rrname[RRBUF];
  unsigned int rrtype;
  unsigned int rrclass;
  unsigned long rrttl;
  char rrrdata[RRBUF];
} dn_rr;

/******************************************************************************
 * Functions
 ******************************************************************************/
#ifdef MONITOR
/* pointer to begin of packet and length - shows up a cute hex-screen */
void packetmonitor(int len2, char *buffer)
{
  int j,i,k;

  j=0;
  for(i=0;i<len2;i++)
  {
    if(j==0)
    {
      printf("\n\r%3X  ",i);
    }
    printf("%02X ",buffer[i]&0xff);
    j++;
    if(j==16)
    {
      for(k=0;k<j;k++)
      {
        if(isprint(buffer[i-j+k+1]&0xff))
        {
          printf("%c",buffer[i-j+k+1]&0xff);
        }
        else
        {
          printf(".");
        }
      }
      j=0;
    }
  }
  i--;
  for(k=0;k<j;k++)
  {
    if(isprint(buffer[i-j+k+1]&0xff))
    {
      printf("%c",buffer[i-j+k+1]&0xff);
    }
    else
    {
      printf(".");
    }
  }
  j=0;
}
#endif

/*
 convert a compressed domain-name to its ASCII representation
 packet: pointer to a DNS-query-packet within a DNS answer or reply
 name: pointer to the name-field
 dest: destination of the domain-name
 len: length of dest
 returns: length of the compressed name field, -1 if length of
 dest is too small for dest to hold the whole domain-name
 */
int dn_uncompress(char *dest, int len, char *packet, char *name)
{
  int nlen, nlenok;
  unsigned int u;

  *dest = 0;
  nlen = nlenok = 0;
  while (*name)
  {
    if ((*name & 0xc0) == 0xc0) /* we found a compressed sign change read pointer */
    {
      u = ((*name & 0x3f) << 8) + (*(name + 1)); /* resolve pointer */
      if (packet + u > name) /* pointer not allowed abort */
      {
        return -1;
      }
      name = packet + u; //u
      if (nlenok == 0)
      { /* after first compressed sign block setting the length field */
        nlen += 2; /* set the length field for the return value */
        nlenok = 1;
      }
    }
    else
    {
      if (*name > (len - 2)) /* domain-name is too long for dest */
      {
        return -1;
      }
      len -= *name;
      strncat(dest, name + 1, *name); /* copy next domain-part to the end of dest */
      if (nlenok == 0)
      {
        nlen += *name + 1;
      }
      name += *name + 1;
      if (*name) /* if name not finished append '.' to dest */
      {
        len--;
        strcat(dest, ".");
      }
    }
  }
  if (nlenok == 0) /* if there was no compress sign in the domain-name include 0 for the length */
  {
    nlen++;
  }
  return nlen; /* return length of name-field */
}

/*
 extract a dns-rr message pointed to by message and return
 the values in an dn_rr struct
 rr: pointer to the dn_rr struct
 packet: pointer to the begin of the packet
 message: pointer in a packet pointing to the message to extract
 returns: the total length of the rr, or -1 if error occured
 */
int dn_unpackrr(dn_rr *rr, char *packet, char *message)
{
  int i;
  unsigned int u;

  if ((i = dn_uncompress(rr->rrname, RRBUF, packet, message)) == -1) /* get name */
  {
    return -1;
  }
  message += i;
  rr->rrtype = ((*message) << 8) + (*(message + 1)); /* get type */
  message += 2;
  rr->rrclass = ((*message) << 8) + (*(message + 1)); /* get class */
  message += 2;
  rr->rrttl = ((*message) << 24) & 0xff000000L; /* get ttl */
  rr->rrttl += ((*(message + 1)) << 16) & 0xff0000L;
  rr->rrttl += ((*(message + 2)) << 8) & 0xff00;
  rr->rrttl += ((*(message + 3))) & 0xff;
  message += 4;
  u = ((*message) << 8) + (*(message + 1)); /* get rdlength */
  message += 2;
  switch (rr->rrtype)
  {
    case DN_TYPE_CNAME:
    case DN_TYPE_NS:
      if (dn_uncompress(rr->rrrdata, RRBUF, packet, message) == -1)
      {
        return -1;
      }
      break;

    case DN_TYPE_MX:
      *(rr->rrrdata) = *message;
      *(rr->rrrdata + 1) = *(message + 1);
      message += 2;
      if (dn_uncompress(rr->rrrdata + 2, RRBUF - 2, packet, message) == -1)
      {
        return -1;
      }
      break;

    case DN_TYPE_A:
      if (rr->rrclass == DN_CLASS_IN)
      {
        *(rr->rrrdata) = *message; /* copy IP address to struct */
        *(rr->rrrdata + 1) = *(message + 1);
        *(rr->rrrdata + 2) = *(message + 2);
        *(rr->rrrdata + 3) = *(message + 3);
        *(rr->rrrdata + 4) = 0;
      }
      else
      {
        return -2; /* unknown CLASS */
      }
      break;

    default:
      *(rr->rrrdata) = 0;
  }
  return u + i + 10; /* return total length of rr */
}

/*
 convert a string like "www.hello.de" to a string with following
 byte values: 3 119 119 199 5 104 101 108 108 111 2 100 101 0
 The bytes mean: length, ASCII chars, length, ASCII chars, ...., length=0
 returns length (including terminator) of string.
 */
static int packdomain(char * dest, const char *src)
{
  int i, n, cnt;

  n = strlen(src);
  dest[n + 1] = 0; // terminator

  // walk back trough the string
  cnt = 0;
  for (i = n; i > 0; i--)
  {
    if (src[i - 1] == '.')
    {
      dest[i] = cnt;
      cnt = 0;
    }
    else
    {
      dest[i] = src[i - 1];
      cnt++;
    }
  }
  dest[0] = cnt;
  return n + 2;
}

/*
 gethostbyname
 This is the function it is all about.
 It prepares the request
 Opens the socket
 Sends the request
 Receives the answer
 Interprets the answer
 Closes the socket

 return 1 on succes
 */
int gethostbyname(const char *dnsServerIP, // where to ask for the DNS resolution
    const char *domainName, // The domain name "www.beck-ipc.com"
    unsigned long *ttl, // Time to live in seconds
    unsigned long *IP, // The 32 bit IP
    char *dest) // The IP as text (make sure it is long enough)
{
  unsigned char buffer[BUFSIZE + 1];
  int sd, error;
  struct sockaddr_in addr;
  int i, j, k, len1, len2, nRR;
  dn_rr myrr;

#ifdef TRACE
  char s[200];
  printf("\r\ndns server: %s  Host: %s\r\n",dnsServerIP,domainName);
#endif

  memset(buffer, 0, BUFSIZE);

  // ident is a sequence number
  if (++ident > 126) ident = 1;
  buffer[0] = (ident >> 8);
  buffer[1] = (ident & 0xff);
  // flags
  buffer[2] = 1;
  buffer[3] = 0;
  // number of questions
  buffer[4] = 0;
  buffer[5] = 1;
  // 12 bytes in header
  len1 = 12;
  len1 += packdomain((char *) &buffer[12], domainName);
  buffer[len1++] = 0;
  buffer[len1++] = 1; // query type is IP address
  buffer[len1++] = 0;
  buffer[len1++] = 1; // query class is also IP address

#ifdef TRACE
  printf("Request holds %d bytes\r\n",len1);
  for (i=0; i<len1; i++) printf(" %X",buffer[i]&0x00ff);
#endif

  sd = opensocket(SOCK_DGRAM, &error); /* open the socket */
  if (sd == API_ERROR)
  {
    printf("\r\nError %d opening socket\r\n", error);
    return 0;
  }

  addr.sin_family = AF_INET; /* set address family to IP */
  if (inet_addr((char *) dnsServerIP, &addr.sin_addr.s_addr)) /* convert the sting holding the ns-ip to binary */
  {
    printf("\r\nError converting DNS Server address %s \r\n", dnsServerIP);
    printf("\r\nIP %lX", addr.sin_addr.s_addr);
    return 0;
  }
  addr.sin_port = htons(DNS_PORT); /* set target port number */
  // send data
  if ( API_ERROR
      == sendto(sd, (char *) buffer, len1, MSG_BLOCKING,
          (const struct sockaddr *) &addr, &error))
  {
    printf("\r\nError %d sending data in line %d\r\n", error, __LINE__);
    closesocket(sd, &error);
    return 0;
  }

  RTX_Sleep_Time(100); /* wait a little */
  // receive answer, with timeout
  len2 = recvfrom(sd, (char *) buffer, BUFSIZE, MSG_TIMEOUT, 10000,
      (struct sockaddr *) &addr, &error);
  closesocket(sd, &error);
  if (len2 <= len1)
  {
    printf("\r\nError %d receiving data\r\n", error);
    return 0;
  }

#ifdef MONITOR
  packetmonitor(len2, buffer);
#endif
#ifdef TRACE
  printf("\r\nReceived %d bytes\r\n",len2);
  printf("Header:");
  for (i=0; i<12; i++)
  printf(" %X",buffer[i]&0xff);
  printf("\r\n");
#endif

  // At this point I just assume that it is a valid answer
  // really good would be to check..... (I leave this to you)
  // the number of answers
  nRR = buffer[6];
  nRR = (nRR << 8) + buffer[7];
#ifdef TRACE
  printf("\r\nGot %d answers",nRR);
#endif
  // we only use one answer
  // the first answer starts where the question stopped
  i = len1;
  // at this point &buffer[i] points to the first answer rr
  k = 1;
  while (nRR--)
  {
    if ((j = dn_unpackrr(&myrr, (char *) buffer, (char *) &buffer[i]))
        == -1) /* get answer-rr from buffer */
    {
      printf("\r\nError in answer section from DNS-server");
      return 0;
    }
#ifdef TRACE
    printf("\r\nA%i: name=%s type=%i class=%i ttl=%lu",k++,myrr.rrname,myrr.rrtype, myrr.rrclass, myrr.rrttl);
    if(myrr.rrtype == DN_TYPE_CNAME)
    {
      printf(" cname=%s",myrr.rrrdata);
    }
#endif
    if (myrr.rrtype == DN_TYPE_A && myrr.rrclass == DN_CLASS_IN) /* if it is a A record, we found what we want */
    {
      k = -1;
      break;
    }
    i += j; /* move position to next answer record */
  }
  if (k != -1)
  {
    return 0;
  }
  memcpy((void *) IP, (void *) myrr.rrrdata, 4);
  *ttl = myrr.rrttl;
  InetToAscii(IP, dest);
  return 1;
}

#endif
