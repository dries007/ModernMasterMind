/******************************************************************************
 *
 * (c) 2005 by BECK IPC GmbH
 *
 *******************************************************************************
 *
 * Disclaimer: This program is an example and should be used as such.
 *             If you wish to use this program or parts of it in your application,
 *             you must validate the code yourself. BECK IPC GmbH can not be held
 *             responsible for the correct functioning or coding of this example.
 *
 * Module:     base64.c
 * Function:   Encoding of binary data according to RFC2045 base64 encoding
 *
 *******************************************************************************
 *
 * Written By ralph hofmann
 * Copyright rh-Technik ralph hofmann
 * http:\\rh-technik.de
 *
 * $Id: base64.c 144 2009-03-30 11:21:42Z pib $
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdlib.h>

/******************************************************************************
 * Defines
 ******************************************************************************/
#define ENCODE_BUFSIZE 512

/******************************************************************************
 * Translate 64
 ******************************************************************************/
unsigned char Translate64(unsigned char Input)
{
  if (Input <= 25) return (Input + 65);
  if ((Input >= 26) && (Input <= 51)) return (Input + 71);
  if ((Input >= 52) && (Input <= 61)) return (Input - 4);
  if (Input == 62) return (unsigned char) '+';
  if (Input == 63) return (unsigned char) '/';
  return (unsigned char) '=';
}

/******************************************************************************
 * Base 64 Encode ,  not reentrant!!!
 ******************************************************************************/
int base64encode(unsigned char* SourcePtr, unsigned char * DestPtr,
    unsigned src_len, int dst_len_max)
/* Translate binary data pointed to by 'Source' into base64 encoded data.
 Returns pointer to encoded data string (zero terminated)
 Note that encoded data size is at least 133.333% of the input size !
 For fully meeting RFC2045 base64 encoded data sent as ascii text
 need a CRLF after every 76 charcters which the user has to apply.

 Return length of destination buffer (zero terminated)

 */
{
  unsigned i, j, dst_len;
  unsigned short IN[3], OUT[4], OverRead;

  dst_len = i = 0;
  dst_len_max -= 1;

  while (i < src_len)
  {
    // Get 24 bits from source
    OverRead = 0;
    for (j = 0; j < 3; j++)
    {
      if (i < src_len)
      {
        IN[j] = *SourcePtr;
      }
      else
      {
        IN[j] = 0; // fill with zero if all input data read
        OverRead++; // count padding input bytes
      }
      SourcePtr++;
      i++;
    }
    // swirl da bits
    OUT[0] = IN[0] >> 2;
    OUT[1] = (IN[1] >> 4) | ((IN[0] << 4) & 0x3F);
    OUT[2] = ((IN[1] << 2) & 0x3C) | (IN[2] >> 6);
    OUT[3] = IN[2] & 0x3F;

    // Translate into base64 alphabet
    for (j = 0; j < 4; j++)
    {
      if (dst_len < dst_len_max)
      {
        DestPtr[dst_len++] = Translate64(
            j < (4 - OverRead) ? OUT[j] : 0x40);
      }
    }
  } //while(i
  DestPtr[dst_len] = 0; // Terminate by zero
  return dst_len;
}
//------------------------------------------------------------------------------
//end of file
