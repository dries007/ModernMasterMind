/******************************************************************************
 *
 * (c) 2005 by BECK IPC GmbH
 *
 *******************************************************************************
 *
 * Module:    base64.h
 * Function:  Prototypes for base64.c
 *
 *******************************************************************************
 *
 * $Id: base64.h 144 2009-03-30 11:21:42Z pib $
 *
 ******************************************************************************/
#ifndef _BASE_64_H_
#define _BASE_64_H_
//------------------------------------------------------------------------------
int base64encode(unsigned char* SourcePtr, unsigned char * DestPtr,
    unsigned src_len, int dst_len_max);

//------------------------------------------------------------------------------
#endif

