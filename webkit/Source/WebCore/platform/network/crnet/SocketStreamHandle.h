/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

#ifndef SocketStreamHandle_h
#define SocketStreamHandle_h

#include "SocketStreamHandleBase.h"

#include "SessionID.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

struct crnet_socket_stream_handle;

namespace WebCore {

    class AuthenticationChallenge;
    class Credential;
    class NetworkingContext;
    class SocketStreamHandleClient;

    class SocketStreamHandle : public RefCounted<SocketStreamHandle>, public SocketStreamHandleBase {
    public:
        static Ref<SocketStreamHandle> create(const URL& url, SocketStreamHandleClient& client, NetworkingContext&, SessionID) { return adoptRef(*new SocketStreamHandle(url, client)); }

        virtual ~SocketStreamHandle();

        void startCrnetSocketStream();
        // crnet callbacks
        void crnetConnected(int error);
        void crnetDataReceived(const char* data, int byteCount);
        void crnetDataSent(int byteCount);
        void crnetFinished();

    protected:
        virtual int platformSend(const char* data, int length);
        virtual void platformClose();

    private:
        SocketStreamHandle(const URL&, SocketStreamHandleClient&);

        struct crnet_socket_stream_handle* m_crnetHandle;
        bool m_platformClosed;

        // No authentication for streams per se, but proxy may ask for credentials.
        void didReceiveAuthenticationChallenge(const AuthenticationChallenge&);
        void receivedCredential(const AuthenticationChallenge&, const Credential&);
        void receivedRequestToContinueWithoutCredential(const AuthenticationChallenge&);
        void receivedCancellation(const AuthenticationChallenge&);
        void receivedRequestToPerformDefaultHandling(const AuthenticationChallenge&);
        void receivedChallengeRejection(const AuthenticationChallenge&);
    };

}  // namespace WebCore

#endif  // SocketStreamHandle_h
