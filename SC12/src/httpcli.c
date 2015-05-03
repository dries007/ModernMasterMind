/******************************************************************************
*
* (c) 2005-2009 by BECK IPC GmbH
*
*******************************************************************************
*
* Disclaimer: This program is an example and should be used as such.
*             If you wish to use this program or parts of it in your application,
*             you must validate the code yourself. BECK IPC GmbH can not be held
*             responsible for the correct functioning or coding of this example.
*
* Module:     httpcli.c
* Function:   HTTP client
*
*******************************************************************************
* Written By Ernest Schloesser
* Copyright BECK IPC GmbH
*
* Extended by ralph hofmann
* Copyright rh-Technik ralph hofmann
*
* Use RTOS built-in DNS services on SC1x3/SC2x/SC2x3
* Add HTTPS support
* by pib
* Copyright BECK IPC GmbH
*
* $Id: httpcli.c 1138 2014-09-25 07:27:18Z pib $
*
******************************************************************************/

/*
 Usage:
 httpGet(char *server, char *page, char *result, int maxlen, char *dns)
 with: server = [user:pass@]servername[:port]
                servername may be a resolvable hostname or a dotted-decimal IP-address
                the "server" string might be changed by this function.
                so if you still need your original string, give this function a copy of it !
                if an authorization is given, httpGet always inserts it in the HTTP request
                 but does not check the server´s reaction on an unauthorized request.
       page   = [path/][filename]   no leading slash !
       result = pointer where received data is to be stored
       maxlen = maximum number of bytes to receive (remember your buffer size !)
       dns    = pointer to a string with the IP-address of the DNS to use for
                                                 domain-name resolving
       httpGet returns the number of bytes received.
                the terminating zero is not counted,
                so this return value cannot extend (maxlen-1).
*/


/******************************************************************************
* Includes
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "httpcli.h"
#include "base64.h"
#include "dns.h"
#include "clib.h"

/******************************************************************************
* Defines
******************************************************************************/
#define MAX_HTTP_REQ_BUFLEN       2048
#define HTTPCLI_RCVBUFSIZE        512
#define MAX_ARG_CMDLEN            128
#define MAX_BASE64_ENCODE_BUFSIZE 500

/******************************************************************************
* global
******************************************************************************/
static char    request        [MAX_HTTP_REQ_BUFLEN + 1];
static char    uri_buf        [MAX_ARG_CMDLEN];
static char    ReceiveBuffer  [HTTPCLI_RCVBUFSIZE];
static char    Base64EncodeBuf[MAX_BASE64_ENCODE_BUFSIZE];

/******************************************************************************
* HTTP Get   not reentrant!!!
  Parameters:
  char * request_line: The HTTP request
  char * request_buf : User provided storage for the received page, zero terminated
  int    max_len     : Max. length of storage request_buf
  char * dns         : Optional, IP address of dns server
  Return >=0, number  of received bytes stored at request_bufsult
          <0, error
******************************************************************************/
int httpGet( char *request_line, char *request_buf, int maxlen, char *dns )
{
   char ipHttpServerIPStr[16];
   struct sockaddr_in addr;
   int sd, error,rcvd;
   #if (defined SC123) || (defined SC23) || (defined SC243)
   int sslSession;
   #endif
   char  *page_ptr, *help_ptr, *uri_ptr;
   unsigned int httpport;
   unsigned int uri_length;
   unsigned int auth_length;
   unsigned int cnt = 0;

   // Find end of URI ('/')
   page_ptr = strchr( request_line, '/' );
   if ( !page_ptr )
   {
      page_ptr = "/";
      uri_length = strlen(request_line)+1;
   }
   else
   {
      uri_length = (unsigned int)(page_ptr - request_line);
   }
   strncpy( uri_buf, request_line, uri_length);
   uri_buf[uri_length] = 0;

   // Check if request line contains user and password
   help_ptr = strchr( uri_buf, '@' );
   if (help_ptr )
   {
      // The uri buffer contains a '@' => Authorization required!!!
      *help_ptr = 0;           // divide the combined string into authorization and address
      uri_ptr = help_ptr+1 ;   // uri_ptr points to the address string only
      auth_length = base64encode((unsigned char *)uri_buf, (unsigned char *)Base64EncodeBuf,
                                 strlen(uri_buf),MAX_BASE64_ENCODE_BUFSIZE);
   }
   else
   {
      // No authorization information given
      uri_ptr = uri_buf;
      auth_length = 0;
   }

   // Check the server-string (without possible authentication) for a specified http-port
   help_ptr = strchr( uri_ptr, ':' ); // is there a ':' in the server ?
   if ( help_ptr )
   {
      *help_ptr = 0; // cut the server string here
      sscanf( help_ptr + 1, "%u", &httpport );  // and read the port
   }
   else
   {
      httpport = 80;
   }

   // Build server address
   addr.sin_family = PF_INET;
   addr.sin_port   = htons( httpport );

   // Is the server name already an IP address?
   if (inet_addr( uri_ptr, &addr.sin_addr.s_addr) != 0)
   {
      // Given string is not an IP address, get the address from DNS

      #if (defined SC123) || (defined SC23) || (defined SC243)
      // Use built-in RTOS DNS API
      struct in_addr dns_addr;

      if (inet_addr( dns, &dns_addr.s_addr) == 0)
      {
        setNameServer(AF_INET, &dns_addr);
      }
      error = getHostByName(uri_ptr, 1, &addr.sin_addr);
      if (error <= 0)
      {
         printf( "\r\n\nHTTP-Client Error: %d DNS failed",
                 error);
         return HTTP_DNS_FAILED;
      }
      InetToAscii(&addr.sin_addr.s_addr, ipHttpServerIPStr);

      #else  // SC1x

      // Use own DNS implementation in dns.c
      unsigned long ttl;

      error = gethostbyname( dns, uri_ptr, &ttl, &addr.sin_addr.s_addr, ipHttpServerIPStr);
      if(error != 1)
      {
         printf( "\r\n\nHTTP-Client Error: %d DNS failed",
                 error);
         return HTTP_DNS_FAILED;
      }
      #endif
   }
   else
   {
      strcpy( ipHttpServerIPStr, uri_ptr);
   }

   sprintf( request, "GET %s HTTP/1.1\r\n"
                     "User-Agent: HTTP Client IPC@CHIP\r\n"
                     "Host: %s\r\n",
                     page_ptr, uri_ptr);
   if ( auth_length )
   {
      sprintf(request + strlen(request), "Authorization: Basic %s\r\n", Base64EncodeBuf);
   }
   strcat( request, "\r\n" );

#if DEBUG

   /*******************************************************************/
   // Show request parameters
   /*******************************************************************/
   printf( "\r\n\nSending to %s on port %d:"
           "\r\n****************************************************"
           "\r\n%s"
           "\r\n****************************************************",
           ipHttpServerIPStr, httpport,
           request);
#endif

   /*******************************************************************/
   // API call open socket
   /*******************************************************************/
   sd = opensocket( SOCK_STREAM, &error );
   if(sd == API_ERROR)
   {
      printf( "\r\n\nHTTP-Client Error: %d socket open failed",
              error);
      return 0;
   }

   #if (defined SC123) || (defined SC23) || (defined SC243)
   // SSL connection?
   if (httpport == 443)
   {
      PKI_CERT_ADD_Entry        addEntry;
      SSL_SESSION_NEW_Entry     sslSessionEntry;
      SetSocketOption           option;
      int value;

      /**********************************************************************/
      // Add root certificate to PKI
      /**********************************************************************/
      addEntry.fileNamePtr = "cacert.der";    // FileName of certificate
      addEntry.fileFormat  = 0x02;            // DER format
      addEntry.certType    = 0x0A;            // Root CA
      addEntry.idPtr       = "MyRootCA_Cert"; // Name
      addEntry.idLength    = strlen(addEntry.idPtr);
      addEntry.caIdPtr     = NULL;
      if (PKI_CertificateAdd(&addEntry, &error) == API_ERROR)
      {
        printf( "\r\n\nHTTP-Client Error: %d add of root certificate failed",
                error);
        goto EndTransfer;
      }

      /**********************************************************************/
      // New SSL session
      /**********************************************************************/
      sslSessionEntry.certIdentity = NULL; // No client authentication
      sslSessionEntry.maxCaches    = 32;   // Cache entries
      sslSessionEntry.version      = 3;    // SSL 3.0 and TLS 1.0
      sslSession = SSL_SessionNew(&sslSessionEntry, &error);
      if (sslSession == API_ERROR)
      {
        printf( "\r\n\nHTTP-Client Error: %d new SSL session failed",
                error);
        goto EndTransfer;
      }

      /**********************************************************************/
      // Set SSL socket options
      /**********************************************************************/

      // Set server negotiation
      option.protocol_level = IP_PROTOTCP_LEVEL;
      option.optionName     = TCP_SSL_CLIENT;
      value                 = 1;
      option.optionValue    = (const char far *) &value;
      option.optionLength   = sizeof(int);
      if (setsockopt(sd, &option, &error) == API_ERROR)
      {
        printf( "\r\n\nHTTP-Client Error: %d set socket options 1 failed",
                error);
        goto EndTransfer;
      }

      // Set SSL session
      option.protocol_level = IP_PROTOTCP_LEVEL;
      option.optionName     = TCP_SSLSESSION;
      option.optionValue    = (const char far *) &sslSession;
      option.optionLength   = sizeof(int);
      if (setsockopt(sd, &option, &error) == API_ERROR)
      {
        printf( "\r\n\nHTTP-Client Error: %d set socket options 2 failed",
                error);
        goto EndTransfer;
      }
   }
   #endif


   /**********************************************************************/
   // API-Call connect, establish a connection
   /**********************************************************************/
   if(connect( sd, (const struct sockaddr *)&addr, &error ) == API_ERROR)
   {
     if(error==261)  //connection refused from server host
     {
       printf( "\r\n\nHTTP-Client Error: connection refused from %s",
               ipHttpServerIPStr);
     }
     else
     {
       printf( "\r\n\nHTTP-Client Error: %d socket connect failed",
               error);
     }
     goto EndTransfer;
   }

   #if (defined SC123) || (defined SC23) || (defined SC243)
   // SSL connection?
   if (httpport == 443)
   {
     /**********************************************************************/
     // Start SSL Client
     /**********************************************************************/
     //if (SSL_ClientStart(sd, NULL, &error) == API_ERROR)            // don't check certificate
     if (SSL_ClientStart(sd, "SSL Test Server", &error) == API_ERROR) // must be the SSL Test Server
     {
       printf( "\r\n\nHTTP-Client Error: %d start of SSL client failed",
               error);
       goto EndTransfer;
     }

     /**********************************************************************/
     // Wait for SSL Handshake to complete
     /**********************************************************************/
     if ((error = SSL_HandshakeComplete(sd, 30000)) != 1)
     {
       printf( "\r\n\nHTTP-Client Error: %d SSL handshake failed",
               error);
       goto EndTransfer;
     }
   }
   #endif

   if ( send( sd, request, strlen( request ), 0, &error ) < 0 )
   {
      printf( "\r\n\nHTTP-Client Error: %d sending request",
              error );
      goto EndTransfer;
   }

   help_ptr = request_buf;  //incoming data is to be stored here
   do
   {
      rcvd = recv( sd, ReceiveBuffer, HTTPCLI_RCVBUFSIZE, MSG_TIMEOUT, 10000L, &error );
      if ( error == 0 )
      {
         if ( cnt + rcvd > maxlen - 1 )
         {
            rcvd=( maxlen - 1 ) - cnt;  // reduce number of bytes to copy
            printf( "\r\n\nHTTP-Client Error: didn't load whole page because of too small memory" );
         }

         memcpy( help_ptr, ReceiveBuffer, rcvd);
         help_ptr += rcvd;    // ptr within buffer
         cnt += rcvd;         // number of bytes received
      }
      else if ( ( rcvd == -1 ) && ( error != 235 ) )
      {
         printf( "\r\n\nHTTP-Client Error: %d receiving page terminated with error\n",
                 error);
      }
   }
   while ( ( ! error ) && ( help_ptr < ( request_buf + maxlen - 1 ) ) );

   *help_ptr = 0;   // Terminate the received-page-string

EndTransfer:
   if ( closesocket( sd, &error ) < 0 )
   {
      printf( "\r\n\nHTTP-Client Error: %d closing connection to %s port %u",
              error, ipHttpServerIPStr, httpport );
   }

   #if (defined SC123) || (defined SC23) || (defined SC243)
   // SSL connection
   if (httpport == 443)
   {
     if (sslSession >= 0)
     {
       if(SSL_SessionClose(sslSession, &error) == API_ERROR)
       {
         printf( "\r\n\nHTTP-Client Error: %d session close failed",
                 error);
       }
     }

     PKI_CertificateDel("MyRootCA_Cert", 0x02, &error);  // 0x02 = non-local list
   }
   #endif

   return cnt;
}

//------------------------------------------------------------------------------
// End of file

