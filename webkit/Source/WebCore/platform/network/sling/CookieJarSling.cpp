/*
 * Copyright (C) 2015 Naver Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "CookieJarSling.h"

#include "Cookie.h"
#include "URL.h"
#include "NetworkingContext.h"
#include "NotImplemented.h"
#include <wtf/text/CString.h>

#if USE(CRNET)
#include <crnet.h>
#endif

namespace WebCore {

String cookiesForURL(const NetworkStorageSession& session, const URL& url)
{
#if USE(CRNET)
    CString urlStr = url.string().latin1();
    const char* cookies;
    int length;
    // Use first_party = null to bypass the cookie accept policy, as it
    // does not apply to this method
    crnet_get_cookies(session.crnetContext(),
                      urlStr.data(), urlStr.length(), &cookies, &length,
                      1, /* exclude httponly cookies */
                      nullptr, 0);
    if (cookies && length > 0)
        return String::fromUTF8(cookies, length);
#else
    UNUSED_PARAM(url);

    notImplemented();
#endif
    return String();
}

void setCookiesFromURL(const NetworkStorageSession& session, const URL& url, const String& value)
{
#if USE(CRNET)
    CString urlStr = url.string().latin1();
    CString valStr = value.utf8();
    crnet_set_cookie(session.crnetContext(),
                     urlStr.data(), urlStr.length(), valStr.data(), valStr.length(), NULL, 0);
#else
    UNUSED_PARAM(url);
    UNUSED_PARAM(value);

    notImplemented();
#endif
}

void deleteAllExpiredCookies(const NetworkStorageSession& session)
{
#if USE(CRNET)
    // Delete only expired cookies.  As a side effect of get_raw_cookies(),
    // chromium's cookie monster garbage collects expired cookies.
    const struct crnet_cookie_info* info_list;
    int count;
    crnet_get_raw_cookies(session.crnetContext(), NULL, 0, &info_list, &count, NULL, 0);
#else
    notImplemented();
#endif
}

void deleteAllSessionCookies(const NetworkStorageSession& session)
{
#if USE(CRNET)
    crnet_delete_session_cookies(session.crnetContext());
#else
    notImplemented();
#endif
}

void addCookie(const NetworkStorageSession&, const URL&, const Cookie&)
{
    notImplemented();
}

}
