/*
 *  TCP/IP or UDP/IP networking functions
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdlib.h>
#endif

#include "mbedtls/net_sockets.h"

#define kBufSize	4096	/* Size for TCP stream buffer and receive buffer */
#define kTimeOut	0		/* Timeout for TCP commands */

/*
 * Prepare for using the sockets interface
 */
static int net_prepare( void )
{
	OSErr err;

	err = InitNetwork();
	if (err != noErr)
	{
		return err;
	}

    return( 0 );
}

/*
 * Initialize a context
 */
void mbedtls_net_init( mbedtls_net_context *ctx, GiveTimePtr giveTimePtr)
{
	ctx->giveTime = giveTimePtr;
}

/*
 * Initiate a TCP connection with host:port and the given protocol
 */
int mbedtls_net_connect( mbedtls_net_context *ctx, const char *host,
                         const char *port, int proto)
{
	OSErr err;
    int ret;
	unsigned long ipAddress;

	(void)(proto); // Unused

    if( ( ret = net_prepare() ) != 0 )
        return( ret );

    /* Do name resolution */
	err = ConvertStringToAddr((char *)host, &ipAddress, ctx->giveTime);
	if (err != noErr)
	{
		return (MBEDTLS_ERR_NET_UNKNOWN_HOST);
	}

	/* Create the stream */
	err = CreateStream(&(ctx->stream), kBufSize, ctx->giveTime, &(ctx->cancel));
	if (err != noErr)
	{
		return(MBEDTLS_ERR_NET_SOCKET_FAILED);
	}

	/* Open the connection */
	err = OpenConnection(ctx->stream, ipAddress, atoi(port), kTimeOut, ctx->giveTime, &(ctx->cancel));
	if (err != noErr) {
		return (MBEDTLS_ERR_NET_CONNECT_FAILED);
	}

    return( 0 );
}

/*
 * Create a listening socket on bind_ip:port
 */
int mbedtls_net_bind( mbedtls_net_context *ctx, const char *bind_ip, const char *port, int proto)
{
	OSErr err;
	int ret;
	unsigned long ipAddress;
	short remotePort;

	(void)(proto); // Unused

	if ((ret = net_prepare()) != 0)
		return(ret);

	remotePort = (short)atoi(port);

	/* Do name resolution */
	err = ConvertStringToAddr((char *)bind_ip, &ipAddress, ctx->giveTime);
	if (err != noErr)
	{
		return (MBEDTLS_ERR_NET_UNKNOWN_HOST);
	}

	/* Create the stream */
	err = CreateStream(&(ctx->stream), kBufSize, ctx->giveTime, &(ctx->cancel));
	if (err != noErr)
	{
		return(MBEDTLS_ERR_NET_SOCKET_FAILED);
	}

	/* Open the connection */
	err = WaitForConnection(ctx->stream, kTimeOut, atoi(port), (long *)&ipAddress, &remotePort, ctx->giveTime, &(ctx->cancel));

	if (err != noErr) {
		return (MBEDTLS_ERR_NET_LISTEN_FAILED);
	}

	return(0);
}

/*
 * Accept a connection from a remote client
 */
int mbedtls_net_accept( mbedtls_net_context *bind_ctx,
                        mbedtls_net_context *client_ctx,
                        void *client_ip, size_t buf_size, size_t *ip_len )
{
	// Not implemented
	(void)(bind_ctx);
	(void)(client_ctx);
	(void)(client_ip);
	(void)(buf_size);
	(void)(ip_len);

    return( 0 );
}

/*
 * Set the socket blocking or non-blocking
 */
int mbedtls_net_set_block( mbedtls_net_context *ctx )
{
	// Not implemented
	(void)(ctx);

	return 0;
}

int mbedtls_net_set_nonblock( mbedtls_net_context *ctx )
{
	// Not implemented
	(void)(ctx);

	return 0;
}

/*
 * Portable usleep helper
 */
void mbedtls_net_usleep( unsigned long usec )
{
	// Not implemented
	(void)(usec);
}

/*
 * Read at most 'len' characters
 */
int mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len)
{
	OSErr err;
	mbedtls_net_context* mbedCtx = ((mbedtls_net_context *)ctx);
	unsigned long stream = mbedCtx->stream;
	unsigned short dataLength = len;

	err = RecvData(stream, (Ptr)buf, &dataLength, false, mbedCtx->giveTime, &(mbedCtx->cancel));

	if (err != noErr) {
		return(MBEDTLS_ERR_NET_RECV_FAILED);
	}

    return(dataLength);
}

/*
 * Read at most 'len' characters, blocking for at most 'timeout' ms
 */
int mbedtls_net_recv_timeout( void *ctx, unsigned char *buf, size_t len,
                      uint32_t timeout)
{
	// (timeout not implemented)
	(void)(timeout);

    /* This call will not block */
    return( mbedtls_net_recv( ctx, buf, len) );
}

/*
 * Write at most 'len' characters
 */
int mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len)
{
	OSErr err;
	mbedtls_net_context* mbedCtx = ((mbedtls_net_context *)ctx);
	unsigned long stream = mbedCtx->stream;

	err = SendData(stream, (Ptr)buf, len, false, mbedCtx->giveTime, &(mbedCtx->cancel));

	if (err != noErr) {
		return(MBEDTLS_ERR_NET_SEND_FAILED);
	}

	return(len);
}

/*
 * Gracefully close the connection
 */
void mbedtls_net_free( mbedtls_net_context *ctx)
{
	CloseConnection(ctx->stream, ctx->giveTime, &(ctx->cancel));
	ReleaseStream(ctx->stream, ctx->giveTime, &(ctx->cancel));
}