/**
 *
 * Copyright (c) 2010, Zed A. Shaw and Mongrel2 Project Contributors.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 *     * Neither the name of the Mongrel2 Project, Zed A. Shaw, nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <dbg.h>
#include <proxy.h>
#include <assert.h>
#include <stdlib.h>
#include <mem/halloc.h>
#include <connection.h>
#include <http11/httpclient_parser.h>


void Proxy_destroy(Proxy *proxy)
{
    if(proxy) {
        if(proxy->server) bdestroy(proxy->server);
        h_free(proxy);
    }
}

Proxy *Proxy_create(bstring server, int port)
{
    Proxy *proxy = h_calloc(sizeof(Proxy), 1);
    check_mem(proxy);
    
    proxy->server = server;
    proxy->port = port;

    return proxy;

error:
    Proxy_destroy(proxy);
    return NULL;
}


static inline int scan_chunks(Connection *conn, httpclient_parser *client, int nread, int from)
{
    int end = from;
    int rc = 0;

    sentinel("REWRITE NEEDED");

    do {
        // find all the possible chunks we've got so far
        httpclient_parser_init(client);
        // rc = httpclient_parser_execute(client, conn->proxy_buf, nread, end);
        check(!httpclient_parser_has_error(client), "Parsing error from server.");

        if(!client->chunked) {
            return end;
        } else {
            end = client->body_start + client->content_len + 2; // +2 for the crlf
        }
    } while(!client->chunks_done && client->content_len > 0 && end < nread - 1);

    return end;

error:
    return -1;
}

int Proxy_read_and_parse(Connection *conn, int start)
{
    httpclient_parser *client = conn->client;
    int nread = 0;
    int nparsed = 0;

    assert(client && "httpclient_parser not configured.");
    httpclient_parser_init(client);

    sentinel("REWRITE NEEDED");

    nparsed = httpclient_parser_execute(client, IOBuf_start(conn->proxy_iob), nread, 0);

    check(!httpclient_parser_has_error(client), "Parsing error from server.");
    check(httpclient_parser_finish(client), "Parser didn't get a full response.");

    return nread;
error:
    return -1;
}


int Proxy_stream_chunks(Connection *conn, int nread)
{
    int rc = 0;
    int end = 0;
    httpclient_parser *client = conn->client;
    assert(client && "httpclient_parser not configured.");

    sentinel("REWRITE NEEDED");

    return 1;

error:
    return -1;
}
