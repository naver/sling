/*
 * Copyright (C) 2014 Naver Corp. All rights reserved.
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
#include "PlatformCookieJar.h"

#include "Cookie.h"
#include "Logging.h"
#include "NetworkStorageSession.h"
#include "URL.h"
#include <crnet.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

void setCookiesFromDOM(const NetworkStorageSession& session, const URL& firstParty, const URL& url, const String& value)
{
    // String uses 16-bit unicode internally.  Chromium wants 8-bit utf8.
    // URL is already encoded.  That is, punycode for hostname, utf8-then-percent-encoding
    // for parameters, and so on.
    //
    // WebCore does not encode cookie names and values.  For example, javascript
    // can pass whatever string to document.cookie.  Well written pages should only
    // pass ASCII characters, but current browsers do let utf8 or other charset
    // strings go into the cookie store without any encoding.
    // So encode the 16-bit unicode string to utf8 and then pass it to chromium.

    CString urlStr = url.string().latin1();
    CString valStr = value.utf8();
    CString firstPartyStr = firstParty.string().latin1();
    crnet_set_cookie(session.crnetContext(),
                     urlStr.data(), urlStr.length(), valStr.data(), valStr.length(),
                     firstPartyStr.data(), firstPartyStr.length());
}

String cookiesForDOM(const NetworkStorageSession& session, const URL& firstParty, const URL& url)
{
    CString urlStr = url.string().latin1();
    CString firstPartyStr = firstParty.string().latin1();
    const char* cookies;
    int length;
    crnet_get_cookies(session.crnetContext(),
                      urlStr.data(), urlStr.length(), &cookies, &length,
                      1, /* exclude httponly cookies */
                      firstPartyStr.data(), firstPartyStr.length());
    if (cookies && length > 0) {
        LOG(Network, "crnet_get_cookies: %s", cookies);
        // Cookie string is not restricted to ASCII in practice.
        // It may contain raw utf8 binary values.
        return String::fromUTF8(cookies, length);
    }
    return String();
}

String cookieRequestHeaderFieldValue(const NetworkStorageSession& session, const URL& firstParty, const URL& url)
{
    CString urlStr = url.string().latin1();
    CString firstPartyStr = firstParty.string().latin1();
    const char* cookies;
    int length;
    crnet_get_cookies(session.crnetContext(),
                      urlStr.data(), urlStr.length(), &cookies, &length,
                      0, /* include httponly cookies */
                      firstPartyStr.data(), firstPartyStr.length());
    if (cookies && length > 0) {
        LOG(Network, "crnet_get_cookies: %s", cookies);
        return String::fromUTF8(cookies, length);
    }
    return String();
}

bool cookiesEnabled(const NetworkStorageSession&, const URL& /*firstParty*/, const URL& /*url*/)
{
    return crnet_cookie_enabled();
}

bool getRawCookies(const NetworkStorageSession& session, const URL& firstParty, const URL& url, Vector<Cookie>& rawCookies)
{
    rawCookies.clear();

    CString urlStr = url.string().latin1();
    CString firstPartyStr = firstParty.string().latin1();
    const struct crnet_cookie_info* info_list, *info;
    int count;
    crnet_get_raw_cookies(session.crnetContext(),
                          urlStr.data(), urlStr.length(), &info_list, &count,
                          firstPartyStr.data(), firstPartyStr.length());
    for (int i = 0; i < count; i++) {
        info = &info_list[i];
        rawCookies.append(Cookie(
            String::fromUTF8(info->name), String::fromUTF8(info->value), String(info->domain),
            String(info->path),
            /* Cookie wants expiration time in milliseconds, not seconds */
            info->expires * 1000,
            info->http_only, info->secure,
            info->expires == 0 ? true : false));
#if !LOG_DISABLED
        Cookie& c = rawCookies[i];
        LOG(Network, "Raw cookie %d: name=%s value=%s domain=%s path=%s"
            " expires=%f httpOnly=%d secure=%d session=%d",
            i, c.name.utf8().data(), c.value.utf8().data(),
            c.domain.latin1().data(), c.path.latin1().data(),
            c.expires, c.httpOnly, c.secure, c.session);
#endif
    }
    return true;
}

void deleteCookie(const NetworkStorageSession& session, const URL& url, const String& cookie)
{
    CString urlStr = url.string().latin1();
    CString cookieStr = cookie.utf8();
    crnet_delete_cookie(session.crnetContext(),
                        urlStr.data(), urlStr.length(), cookieStr.data(),
                        cookieStr.length());
}

void getHostnamesWithCookies(const NetworkStorageSession& session, HashSet<String>& hostnames)
{
    hostnames.clear();

    // Retrieve all cookies and then check host names.
    const struct crnet_cookie_info* info_list, *info;
    int count;
    crnet_get_raw_cookies(session.crnetContext(), NULL, 0, &info_list, &count, NULL, 0);
    for (int i = 0; i < count; i++) {
        info = &info_list[i];
        // Domain cookies start with '.', so skip those.  Only add host cookies.
        if (info->domain && info->domain[0] != '.') {
            hostnames.add(String(info->domain));
        }
    }
}

void deleteCookiesForHostnames(const NetworkStorageSession& session, const Vector<String>& hostnames)
{
    for (auto& hostname : hostnames) {
        CString hostStr = hostname.latin1();
        crnet_delete_all_cookies(session.crnetContext(), hostStr.data(), hostStr.length(), 0, 0);
    }
}

void deleteAllCookies(const NetworkStorageSession& session)
{
    crnet_delete_all_cookies(session.crnetContext(), NULL, 0, 0, 0);
}

void deleteAllCookiesModifiedSince(const NetworkStorageSession& session, std::chrono::system_clock::time_point begin)
{
    crnet_delete_all_cookies(session.crnetContext(), NULL, 0, begin.time_since_epoch().count(), 0);
}

}
