/******************************************************************************
 *
 * (c) 2005 by BECK IPC GmbH
 *
 *******************************************************************************
 *
 * Module:    httpcli.h
 * Function:  Prototypes for httpcli.c
 *
 *******************************************************************************
 *
 * $Id: httpcli.h 338 2009-11-06 13:39:46Z pib $
 *
 ******************************************************************************/
#ifndef _HTTP_CLI_H__
#define _HTTP_CLI_H__
//------------------------------------------------------------------------------

//ErrorCode constants
#define HTTP_REQ_OK             0
#define HTTP_INVALID_REQ       -1     //Invalid request format
#define HTTP_DNS_FAILED        -2     //dns req. failed

//prototype
int httpGet(char *request_line, char *result, // pointer to buffer where to store the result
    int maxlen, // length of the buffer
    char *dns); // IP-Addr. string of dns to use for resolving the hostname );
//------------------------------------------------------------------------------
#endif

