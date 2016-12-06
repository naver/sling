/*
 * Copyright (C) 2009 Brent Fulgham.  All rights reserved.
 * Copyright (C) 2009 Google Inc.  All rights reserved.
 * Copyright (C) 2014 Naver Labs.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "SocketStreamHandle.h"

#include "Logging.h"
#include "NetworkingContext.h"
#include "NotImplemented.h"
#include "SocketStreamError.h"
#include "SocketStreamHandleClient.h"
#include <wtf/MainThread.h>
#include <wtf/text/CString.h>

#include <crnet.h>

namespace WebCore {

static void crnetCallback(int type, void* info)
{
    struct crnet_socket_stream_callback_info* eventInfo =
            static_cast<struct crnet_socket_stream_callback_info*>(info);
    SocketStreamHandle* handle = reinterpret_cast<SocketStreamHandle*>(eventInfo->data);
    // FIXME: Why is this necessary for callOnMainThread?
    //pthread_init_current_np(static_cast<pthread_main_np_t>(eventInfo->main_thread));
    switch (type) {
        case CRNET_SOCKET_STREAM_CALLBACK_TYPE_CONNECTED: {
            int error = eventInfo->u.connection.error_code;
            callOnMainThread([handle, error] {
                handle->crnetConnected(error);
            });
            break;
        }
        case CRNET_SOCKET_STREAM_CALLBACK_TYPE_DATA_RECEIVED: {
            const char* data = eventInfo->u.data_received.data;
            int count = eventInfo->u.data_received.byte_count;
            callOnMainThread([handle, data, count] {
                handle->crnetDataReceived(data, count);
            });
            break;
        }
        case CRNET_SOCKET_STREAM_CALLBACK_TYPE_DATA_SENT: {
            int count = eventInfo->u.data_sent.byte_count;
            callOnMainThread([handle, count] {
                handle->crnetDataSent(count);
            });
            break;
        }
        case CRNET_SOCKET_STREAM_CALLBACK_TYPE_FINISHED: {
            callOnMainThread([handle] {
                handle->crnetFinished();
            });
            break;
        }
        default:
            ASSERT(0);
            break;
    }
}

SocketStreamHandle::SocketStreamHandle(const URL& url, SocketStreamHandleClient& client)
    : SocketStreamHandleBase(url, client)
    , m_crnetHandle(nullptr)
    , m_platformClosed(false)
{
    LOG(Network, "SocketStreamHandle %p new client %p", this, m_client);

    callOnMainThread([this] {
        this->startCrnetSocketStream();
    });
}

SocketStreamHandle::~SocketStreamHandle()
{
    LOG(Network, "SocketStreamHandle %p delete", this);
    //cory setClient(0);
    // crnet should have called FINISHED, and we should have called
    // crnet_finish_socket_stream and dropped the reference taken above in the constructor.
    ASSERT(m_crnetHandle == nullptr);
}

void SocketStreamHandle::startCrnetSocketStream()
{
    ASSERT(isMainThread());
    if (m_state != Connecting)
        return;

    CString urlCStr = m_url.string().latin1();
    struct crnet_socket_stream_request request;
    request.url = urlCStr.data();
    request.callback_data = static_cast<void*>(this);
    //request.callback_main_thread = static_cast<void*>(pthread_get_main_np());
    request.callback = crnetCallback;

    // Need a context even for socket streams because they belong to socket pools.
    struct crnet_context* context = NetworkStorageSession::defaultStorageSession().crnetContext();
    m_crnetHandle = crnet_start_socket_stream(context, &request);
    if (m_crnetHandle)
        ref(); // until crnetFinished
    else
        m_client.didFailSocketStream(*this, SocketStreamError());
}

void SocketStreamHandle::crnetConnected(int error)
{
    ASSERT(isMainThread());
    // If we are closing the channel, do nothing.
    if (m_state >= Closing)
        return;
    if (error) {
        // m_state is Connecting.  The client should call disconnect() which then
        // calls platformClose().
        m_client.didFailSocketStream(*this, SocketStreamError(error));
    } else {
        m_state = Open;
        m_client.didOpenSocketStream(*this);
    }
}

void SocketStreamHandle::crnetDataReceived(const char* data, int byteCount)
{
    ASSERT(isMainThread());
    // If we are closing the channel, do nothing.
    if (m_state != Open)
        return;
    if (byteCount <= 0)
        close();
    else {
        m_client.didReceiveSocketStreamData(*this, data, byteCount);
        // didReceiveSocketStreamData may call platformClose before returning.
        // Make sure we do not crnet in that case
        if (!m_platformClosed)
            crnet_finish_socket_stream_data_received(m_crnetHandle);
    }
}

void SocketStreamHandle::crnetFinished()
{
    ASSERT(isMainThread());
    ASSERT(m_crnetHandle);
    m_crnetHandle = nullptr;
    deref(); // deref the ref taken in startCrnetSocketStream
}

void SocketStreamHandle::crnetDataSent(int byteCount)
{
    ASSERT(isMainThread());
    if (m_state != Open && m_state != Closing) // see SocketStreamHandleBase::sendPendingData()
        return;
    if (byteCount < 0)
        m_client.didFailSocketStream(*this, SocketStreamError());
    else
        sendPendingData();
}

int SocketStreamHandle::platformSend(const char* data, int bytes)
{
    ASSERT(isMainThread());
    if (m_crnetHandle && !m_platformClosed)
        return !data ? 0 : crnet_send_socket_stream_data(m_crnetHandle, data, bytes);
    else
        return -1;
}

void SocketStreamHandle::platformClose()
{
    ASSERT(isMainThread());

    // The caller may invoke platformClose() multiple times.
    // Process only the first.  The subsequent calls are no-op.
    if (m_state == Closed)
        return;
    RELEASE_ASSERT(!m_platformClosed);
    m_platformClosed = true;
    if (m_crnetHandle)
        crnet_finish_socket_stream(m_crnetHandle);
    // The caller expects that we call didCloseSocketStream() before returning.
    // The caller will deref the handle, but with ref() in startCrnetSocketStream, the handle
    // object survives until crnetFinished().
    m_client.didCloseSocketStream(*this);
}

void SocketStreamHandle::didReceiveAuthenticationChallenge(const AuthenticationChallenge&)
{
    notImplemented();
}

void SocketStreamHandle::receivedCredential(const AuthenticationChallenge&, const Credential&)
{
    notImplemented();
}

void SocketStreamHandle::receivedRequestToContinueWithoutCredential(const AuthenticationChallenge&)
{
    notImplemented();
}

void SocketStreamHandle::receivedCancellation(const AuthenticationChallenge&)
{
    notImplemented();
}

void SocketStreamHandle::receivedRequestToPerformDefaultHandling(const AuthenticationChallenge&)
{
    notImplemented();
}

void SocketStreamHandle::receivedChallengeRejection(const AuthenticationChallenge&)
{
    notImplemented();
}

}  // namespace WebCore
