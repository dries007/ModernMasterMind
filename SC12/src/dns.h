/******************************************************************************
 *
 * (c) 2004 by BECK IPC GmbH
 *
 *******************************************************************************
 *
 * Module:    dns.h
 * Function:  Prototypes for DNS.C
 *
 *******************************************************************************
 *
 * $Id: dns.h 144 2009-03-30 11:21:42Z pib $
 *
 ******************************************************************************/

int gethostbyname(const char *dnsServerIP, // where to ask for the dns resolution
    const char *domainName, // The domain name "www.beck-ipc.com"
    unsigned long *ttl, // Time to live in seconds
    unsigned long *IP, // The 32 bit IP
    char *dest); // The IP as text (make sure it is long enough)

