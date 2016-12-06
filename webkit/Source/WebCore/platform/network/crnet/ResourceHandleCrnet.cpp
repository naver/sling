/*
 * Copyright (C) 2004, 2006 Apple Inc.  All rights reserved.
 * Copyright (C) 2005, 2006 Michael Emmel mike.emmel@gmail.com
 * All rights reserved.
 * Copyright (C) 2014, 2015 NAVER Corp.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "config.h"
#include "ResourceHandle.h"

#include "BlobRegistryImpl.h"
#include "CachedResourceLoader.h"
#include "CredentialStorage.h"
#include "ErrorsSling.h"
#include "FileSystem.h"
#include "HTTPHeaderNames.h"
#include "Logging.h"
#include "NetworkingContext.h"
#include "NotImplemented.h"
#include "ResourceHandleClient.h"
#include "ResourceHandleInternal.h"
#include "SharedBuffer.h"
#include <crnet.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class CrNetRequestCompletedInfo {
  public:
    CrNetRequestCompletedInfo(ResourceHandle* rh, struct crnet_handle* ch,
                              struct crnet_request_completed_info* info) {
        job = rh;
        crnetHandle = ch;
        // We do not make a copy of the body data.  crnet does not touch or free it
        // until we call crnet_finish_request().
        data = info->data;
        byteCount = info->byte_count;
        success = info->success;
        errorCode = info->error_code;
        if (info->error_string)
            errorString = String(info->error_string);
    }
    ~CrNetRequestCompletedInfo() {
    }

    ResourceHandle* job;
    struct crnet_handle* crnetHandle;
    const char* data;
    int byteCount;
    bool success;
    int errorCode;
    String errorString;
};

class CrNetResponseStartedInfo {
  public:
    CrNetResponseStartedInfo(ResourceHandle* rh, struct crnet_handle* ch,
                             struct crnet_response_started_info* info) {
        job = rh;
        crnetHandle = ch;
        success = info->success;
#if 0
        statusCode = info->status_code;
        expectedContentSize = info->expected_content_size;
        // FIXME.  Can we avoid malloc?
        if (info->latestUrl)
            latestUrl = fastStrDup(info->latestUrl);
        else
            latestUrl = NULL;
        if (info->mimetype)
            mimetype = fastStrDup(info->mimetype);
        else
            mimetype = NULL;
        if (info->charset)
            charset = fastStrDup(info->charset);
        else
            charset = NULL;
        if (info->status_text)
            statusText = fastStrDup(info->status_text);
        else
            statusText = NULL;
        connectionReused = info->connection_resued;
        connectionId = info->connection_id;
        wasCached = info->was_cached;
#if ENABLE(WEB_TIMING)
        dnsStart = info->dns_start;
        dnsEnd = info->dns_end;
        connectStart = info->connect_start;
        connectEnd = info->connect_end;
        requestStart = info->request_start;
        responseStart = info->response_start;
        sslStart = info->ssl_start;
#endif
#endif
    }
    ~CrNetResponseStartedInfo() {
#if 0
        if (latestUrl)
            fastFree(static_cast<void*>(latestUrl));
        if (mimetype)
            fastFree(static_cast<void*>(mimetype));
        if (charset)
            fastFree(static_cast<void*>(charset));
        if (statusText)
            fastFree(static_cast<void*>(statusText));
#endif
    }

    ResourceHandle* job;
    struct crnet_handle* crnetHandle;
    bool success;
#if 0
    int statusCode;
    int64_t expectedContentSize;
    const char* latestUrl;
    const char* mimetype;
    const char* charset;
    const char* statusText;
    int connectionReused;
    uint32_t connectionId;
    int wasCached;
#if ENABLE(WEB_TIMING)
    int dnsStart;
    int dnsEnd;
    int connectStart;
    int connectEnd;
    int requestStart;
    int responseStart;
    int sslStart;
#endif
#endif
};

class CrNetDataReceivedInfo {
  public:
    CrNetDataReceivedInfo(ResourceHandle* rh, struct crnet_handle* ch,
                          struct crnet_data_received_info* info) {
        job = rh;
        crnetHandle = ch;
        // We do not make a copy of the body data.  crnet does not touch or free it
        // until we call crnet_finish_data_received().
        data = info->data;
        byteCount = info->byte_count;
    }
    ~CrNetDataReceivedInfo() {
    }

    ResourceHandle* job;
    struct crnet_handle* crnetHandle;
    const char* data;
    int byteCount;
};

class CrNetRedirectReceivedInfo {
  public:
    CrNetRedirectReceivedInfo(ResourceHandle* rh, struct crnet_handle* ch,
                              struct crnet_redirect_received_info* info) {
        job = rh;
        crnetHandle = ch;
        statusCode = info->status_code;
        newURL = fastStrDup(info->new_url);
    }
    ~CrNetRedirectReceivedInfo() {
        fastFree(static_cast<void*>(newURL));
    }

    ResourceHandle* job;
    struct crnet_handle* crnetHandle;
    int statusCode;
    char* newURL;
};

class CrNetAuthRequiredInfo {
  public:
    CrNetAuthRequiredInfo(ResourceHandle* rh, struct crnet_handle* ch,
                          struct crnet_auth_required_info* info) {
        job = rh;
        crnetHandle = ch;
        host = fastStrDup(info->host);
        port = info->port;
        realm = fastStrDup(info->realm);
        is_proxy = info->is_proxy;
        previous_failures = info->previous_failures;
        if (!strcmp(info->scheme, "basic"))
            scheme = ProtectionSpaceAuthenticationSchemeHTTPBasic;
        else if (!strcmp(info->scheme, "digest"))
            scheme = ProtectionSpaceAuthenticationSchemeHTTPDigest;
        else
            scheme = ProtectionSpaceAuthenticationSchemeUnknown;
    }
    ~CrNetAuthRequiredInfo() {
        fastFree(static_cast<void*>(host));
        fastFree(static_cast<void*>(realm));
    }

    ResourceHandle* job;
    struct crnet_handle* crnetHandle;
    char* host;
    int port;
    char* realm;
    int is_proxy;
    int previous_failures;
    ProtectionSpaceAuthenticationScheme scheme;
};

class CrNetSslErrorInfo {
  public:
    CrNetSslErrorInfo(ResourceHandle* rh, struct crnet_handle* ch,
                      struct crnet_ssl_info* info) {
        m_job = rh;
        m_crnetHandle = ch;
        m_sslInfo = *info;
        if (info->host)
            m_host = fastStrDup(info->host);
        else
            m_host = nullptr;
    }
    ~CrNetSslErrorInfo() {
        if (m_host)
            fastFree(static_cast<void*>(m_host));
        if (m_sslInfo.valid && m_sslInfo.cert)
            crnet_free_certificate(m_sslInfo.cert);
    }

    ResourceHandle* m_job;
    struct crnet_handle* m_crnetHandle;
    struct crnet_ssl_info m_sslInfo;
    char* m_host;
};

static void crnetCallback(struct crnet_handle* handle, int type, void* info);

static const UChar* temporary16BitStringData(const String& s, String& buffer)
{
    const UChar* data;
    if (s.is8Bit()) {
        buffer = String::make16BitFrom8BitSource(s.characters8(), s.length());
        data = buffer.characters16();
    } else
        data = s.characters16();
    return data;
}

static void crnetRetryAuth(struct crnet_handle* handle, const String& user, const String& pass)
{
    String bufferUser;
    const UChar* userPtr;
    userPtr = temporary16BitStringData(user, bufferUser);

    String bufferPass;
    const UChar* passPtr;
    passPtr = temporary16BitStringData(pass, bufferPass);

    crnet_retry_auth(handle,
                     (const uint16_t*)userPtr, user.length(),
                     (const uint16_t*)passPtr, pass.length());
}

ResourceHandleInternal::~ResourceHandleInternal()
{
}

ResourceHandle::~ResourceHandle()
{
    cancel();
}

bool ResourceHandle::unpackCrnetResponse(struct crnet_handle* handle, ResourceResponse& response, ResourceError& error,
                                         const char** body_data, int64_t* body_byte_count,
                                         bool* secure)
{
    struct crnet_response_info respInfo;
    crnet_get_response_info(handle, &respInfo);

    LOG(Network, "status_code=%d mimetype=%s charset=%s latest_url=%s"
        " expected_content_size=%d",
        respInfo.status_code, respInfo.mimetype, respInfo.charset,
        respInfo.latest_url, (int)respInfo.expected_content_size);
    LOG(Network, "status_text=%s connection_id=%u connection_reused=%d was_cached=%d",
        respInfo.status_text, respInfo.connection_id, respInfo.connection_reused,
        respInfo.was_cached);
    LOG(Network, "dns_start=%d dns_end=%d connect_start=%d connect_end=%d request_start=%d"
        " response_start=%d ssl_start=%d",
        respInfo.dns_start, respInfo.dns_end, respInfo.connect_start, respInfo.connect_end,
        respInfo.request_start, respInfo.response_start, respInfo.ssl_start);
    LOG(Network, "body_data=%p body_byte_count=%d",
        respInfo.body_data, (int)respInfo.body_byte_count);

    if (secure)
        *secure = !!respInfo.secure;

    // Retrieve the certificate if this is a secure request.
    // Below we attach it to the response or the error.
    RefPtr<CrnetCertificate> certificate;
    if (respInfo.secure) {
        struct crnet_ssl_info ssl_info;
        crnet_get_ssl_info(handle, &ssl_info);
        if (ssl_info.valid) {
            certificate = CrnetCertificate::create(ssl_info.cert);
            certificate->setStatus(ssl_info.cert_status);
            certificate->setValidStart(ssl_info.valid_start);
            certificate->setValidExpiry(ssl_info.valid_expiry);
        }
    }

    if (respInfo.status_code != -1) {
        // Got the response
        response.setURL(URL(ParsedURLString, String(respInfo.latest_url)));
        response.setHTTPStatusCode(respInfo.status_code);
        if (respInfo.expected_content_size >= 0)
            response.setExpectedContentLength(respInfo.expected_content_size);
        response.setMimeType(String(respInfo.mimetype));
        response.setTextEncodingName(String(respInfo.charset));
        response.setHTTPStatusText(String(respInfo.status_text));
        response.setSource((respInfo.was_cached) ? ResourceResponseBase::Source::DiskCacheAfterValidation : ResourceResponseBase::Source::Network);
        if (respInfo.suggested_filename_length > 0) {
            const UChar* name = reinterpret_cast<const UChar*>(respInfo.suggested_filename);
            unsigned length = respInfo.suggested_filename_length;
            response.setSuggestedFilename(String(name, length));
        }
#if ENABLE(WEB_TIMING)
        response.resourceLoadTiming().domainLookupStart = respInfo.dns_start;
        response.resourceLoadTiming().domainLookupEnd = respInfo.dns_end;
        response.resourceLoadTiming().connectStart = respInfo.connect_start;
        response.resourceLoadTiming().connectEnd = respInfo.connect_end;
        response.resourceLoadTiming().requestStart = respInfo.request_start;
        response.resourceLoadTiming().responseStart = respInfo.response_start;
        response.resourceLoadTiming().secureConnectionStart = respInfo.ssl_start;
#endif
        // Copy response headers if any.
        const char* headerName;
        const char* headerValue;
        if (!crnet_get_response_header(handle, 1, &headerName, &headerValue)) {
            LOG(Network, "Add response header. %s=%s", headerName, headerValue);
            // FIXME.  Do we need to differentiate {add,set}HTTPHeaderField, like
            // curl/ResourceHandleManager.cpp headerCallback() does using isAppendableHeader()?
            // add() appends the existing value with a comma separator.
            // set() overwrites it.  Neither seems quite correct.

            // The header value might contain non-ASCII strings, in violation of
            // HTTP RFCs.  If it does, we have no way of knowing the string's charset.
            // Assume it is UTF8 and use fromUTF8() to contruct strings.
            // Even if we incorrectly decode charset here, it is harmless as response
            // headers are used only for debugging/diagnostic purposes (e.g. inspector).

            // fromUTF8 returns an empty string if decoding fails, which makes the header
            // not present. So use latin1 fallback to preserve the header.
            response.addHTTPHeaderField(String(headerName), String::fromUTF8WithLatin1Fallback(headerValue, strlen(headerValue)));
            while (!crnet_get_response_header(handle, 0, &headerName, &headerValue)) {
                LOG(Network, "Add response header. %s=%s", headerName, headerValue);
                response.addHTTPHeaderField(String(headerName), String::fromUTF8WithLatin1Fallback(headerValue, strlen(headerValue)));
            }
        }
        if (body_data)
            *body_data = respInfo.body_data;
        if (body_byte_count)
            *body_byte_count = respInfo.body_byte_count;
        response.setCertificate(certificate.get());
        return true;
    } else {
        // Failed.  Need to provide more verbose reasons.  FIXME
        LOG(Network, "Request failed");
        ResourceError resError(errorDomainNetwork, respInfo.error_code, URL(URL(), String(respInfo.latest_url)),
                               String(respInfo.error_string));
        if (crnet_request_timedout(handle))
            resError.setType(ResourceErrorBase::Type::Timeout);
        resError.setCertificate(certificate.get());
        error = resError;
    }
    return false;
}

void ResourceHandle::crnetResponseStarted(CrNetResponseStartedInfo* info)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    ResourceHandleClient* client = d->client();
    ResourceError resError;

    // Do nothing if cancelled.
    if (d->m_crnetCancelled)
        return;

    if (unpackCrnetResponse(info->crnetHandle, d->m_response, resError)) {
        // Special case multipart/x-mixed-replace as required by WebCore.  We are simply
        // using curl's MultipartHandle.  See ResourceHandleManager.cpp:headerCallback().
        // MultipartHandle extracts parts and pass each as a separate response.
        if (d->m_response.isMultipart()) {
            String boundary;
            bool parsed = MultipartHandle::extractBoundary(
                d->m_response.httpHeaderField(HTTPHeaderName::ContentType), boundary);
            LOG(Network, "multipart/x-mixed-replace. boundary=%s", boundary.latin1().data());
            if (parsed) {
                d->m_multipartHandle = MultipartHandle::create(this, boundary);
            }
        }

        if (client) {
            if (client->usesAsyncCallbacks())
                client->didReceiveResponseAsync(this, WTFMove(d->m_response));
            else {
                client->didReceiveResponse(this, WTFMove(d->m_response));
                crnet_continue_after_response(d->m_crnetHandle);
            }
        }
        d->m_response.setResponseFired(true);
    } else if (client) {
        d->m_crnetDidFail = true;
        client->didFail(this, resError);
    }

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

void ResourceHandle::continueWillSendRequest(ResourceRequest&&)
{
    // FIXME: the request may have mutated.  We are supposed to use that
    // as the next request.  For now, let the chromium network use the
    // request it has constructed by itself.  Mutation should not happen
    // in sling at this point, as it has no client features that modify
    // redirection.
    //
    // As an example, the following issue suggests private browsing may
    // mutate the request.
    // https://bugs.webkit.org/show_bug.cgi?id=144533

    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    if (!d->m_crnetCancelled && d->m_crnetHandle)
        crnet_continue_after_redirect(d->m_crnetHandle);
}

typedef HashSet<String, ASCIICaseInsensitiveHash> HostsSet;
static HostsSet& allowsAnyHTTPSCertificateHosts()
{
    DEPRECATED_DEFINE_STATIC_LOCAL(HostsSet, hosts, ());
    return hosts;
}

void ResourceHandle::setHostAllowsAnyHTTPSCertificate(const String& host)
{
    allowsAnyHTTPSCertificateHosts();
}

void ResourceHandle::continueDidReceiveResponse()
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    if (!d->m_crnetCancelled && d->m_crnetHandle && !d->m_continueDidReceiveResponse) {
        // A multipart response may lead to multiple calls to continueDidReceiveResponse.
        // Call crnet only once.
        d->m_continueDidReceiveResponse = true;
        crnet_continue_after_response(d->m_crnetHandle);
    }
}

void ResourceHandle::setClientCertificateInfo(const String& host, const String& certificate, const String& key)
{
    //if (fileExists(certificate))
    //    addAllowedClientCertificate(host, certificate, key);
    //else
    //    LOG(Network, "Invalid client certificate file: %s!\n", certificate.latin1().data());
}

void ResourceHandle::crnetDataReceived(CrNetDataReceivedInfo* info)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    ResourceHandleClient* client = d->client();

    // Do nothing if cancelled.
    if (d->m_crnetCancelled)
        return;

    LOG(Network, "data=%p byte_count=%d", info->data, info->byteCount);

    if (d->m_multipartHandle) {
        // This calls didReceiveResponse and didReceiveData as necessary.
        d->m_multipartHandle->contentReceived(info->data, info->byteCount);
    }
    else if (client)
        client->didReceiveBuffer(this, SharedBuffer::create(info->data, info->byteCount), 0);

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

void ResourceHandle::crnetDataUploaded(unsigned long long sent, unsigned long long total)
{
    // Do nothing if cancelled.
    if (d->m_crnetCancelled)
        return;
    if (d->client())
        d->client()->didSendData(this, sent, total);
}

void ResourceHandle::setClientCertificate(const String&, CFDataRef)
{
}

void ResourceHandle::crnetRequestCompleted(CrNetRequestCompletedInfo* info)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    ResourceHandleClient* client = d->client();
    struct crnet_handle* handle = d->m_crnetHandle;

    LOG(Network, "data=%p byte_count=%d success=%d", info->data, info->byteCount,
        info->success);
    d->m_crnetHandle = nullptr;

    // Do nothing if cancelled.
    if (d->m_crnetCancelled)
        return;

    // If failed, indicate failure if we have not done so.
    if (!info->success) {
        if (client && !d->m_crnetDidFail) {
            d->m_crnetDidFail = true;
            ResourceError resError(errorDomainNetwork, info->errorCode, firstRequest().url(), info->errorString);
            if (crnet_request_timedout(handle))
                resError.setType(ResourceErrorBase::Type::Timeout);
            client->didFail(this, resError);
        }
        return;
    }

    bool hasData = info->data && info->byteCount > 0;
    if (d->m_multipartHandle) {
        if (hasData)
            d->m_multipartHandle->contentReceived(info->data, info->byteCount);
        d->m_multipartHandle->contentEnded();
    }
    if (client) {
        if (hasData && !d->m_multipartHandle)
            client->didReceiveBuffer(this, SharedBuffer::create(info->data, info->byteCount), 0);
        // didReceiveBuffer() may lead to cancel() before returning.  So check if the
        // request has been cancelled, in which case the client object is stale, garbage.
        if (!d->m_crnetCancelled)
            client->didFinishLoading(this, 0);
    }

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

void ResourceHandle::crnetRedirectReceived(CrNetRedirectReceivedInfo* info)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    ResourceHandleClient* client = d->client();

    // Do nothing if cancelled.
    if (d->m_crnetCancelled)
        return;

    LOG(Network, "statusCode=%d newURL=%s", info->statusCode, info->newURL);
    if (client) {
        // Do not clobber the response object in ResourceHandleInternal.
        // Make a new one and pass that to the client.
        // FIXME.  Need more complete response with actual headers and so on.
        ResourceResponse response(firstRequest().url(), String(), 0, String());
        response.setHTTPStatusCode(info->statusCode);

        ResourceRequest request = firstRequest();
        request.setURL(URL(ParsedURLString, String(info->newURL)));
        if (client->usesAsyncCallbacks())
            client->willSendRequestAsync(this, WTFMove(request), WTFMove(response));
        else {
            client->willSendRequest(this, WTFMove(request), WTFMove(response));
            crnet_continue_after_redirect(d->m_crnetHandle);
        }
    }

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

void ResourceHandle::crnetAuthRequired(CrNetAuthRequiredInfo* info)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    // Do nothing if cancelled.
    if (d->m_crnetCancelled)
        return;

    LOG(Network, "host=%s port=%d realm=%s scheme=%d is_proxy=%d previous_failures=%d",
        info->host, info->port, info->realm, info->scheme, info->is_proxy, info->previous_failures);

    // Basically, we need to create ProtectionSpace (e.g. host, port, realm, ...)
    // and AuthenticationChallenge.  AuthenticationChallenge contains
    // the protection space and whatever other data we need to handle
    // the credential that comes back later from the UI process.
    ResourceError resError;
    const char* body_data;
    int64_t body_byte_count;
    bool secure;
    unpackCrnetResponse(d->m_crnetHandle, d->m_response, resError, &body_data, &body_byte_count, &secure);
    ProtectionSpace protectionSpace(String(info->host), info->port,
                                    secure ? ProtectionSpaceServerHTTPS : ProtectionSpaceServerHTTP,
                                    String(info->realm), info->scheme);
    AuthenticationChallenge challenge(protectionSpace, Credential(), info->previous_failures,
                                      d->m_response, ResourceError());
    // AuthenticationManager uses authenticationClient() to call us back.
    challenge.setAuthenticationClient(this);
    didReceiveAuthenticationChallenge(challenge);

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

void ResourceHandle::crnetSslError(CrNetSslErrorInfo* info)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);
    LOG(Network, "valid=%d host=%s port=%d", info->m_sslInfo.valid, info->m_host, info->m_sslInfo.port);

    // Do nothing if cancelled.
    if (d->m_crnetCancelled)
        return;

    // If the error info does not have certificates, cancel right away.
    // The user will probably see a blank page and no indications as to why the request
    // has failed.  FIXME
    if (!info->m_sslInfo.valid) {
        cancel();
        return;
    }

    // Also cancel if we do not have the cllient.  We need the UI.
    if (!client()) {
        cancel();
        return;
    }

    // The rest of the processing steps follows the 401 response handling.
    // We send the authentication challenge to the UI process and waits for
    // the user action.
    ProtectionSpace protectionSpace(String(info->m_host), info->m_sslInfo.port,
                                    ProtectionSpaceServerHTTPS, String(),
                                    ProtectionSpaceAuthenticationSchemeServerTrustEvaluationRequested);

    // Get the certificate data from crnet and attach it to the protection space.
    RefPtr<CrnetCertificate> certificate;
    certificate = CrnetCertificate::create(info->m_sslInfo.cert);
    certificate->setStatus(info->m_sslInfo.cert_status);
    protectionSpace.serverCertificate()->setCertificate(certificate.get());

    // Prevent the caller from deleting the certificate.  Ugly, FIXME.
    info->m_sslInfo.cert = nullptr;

    AuthenticationChallenge challenge(protectionSpace, Credential(), 0,
                                      ResourceResponse(), ResourceError());
    challenge.setAuthenticationClient(this);
    d->m_currentWebChallenge = challenge;
    if (client())
        client()->didReceiveAuthenticationChallenge(this, d->m_currentWebChallenge);

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

static void crnetCallbackRequestCompletedOnMainThread(void* context)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    CrNetRequestCompletedInfo* info = static_cast<CrNetRequestCompletedInfo*>(context);
    info->job->crnetRequestCompleted(info);
    crnet_finish_request(info->crnetHandle);
    // crnet has cleaned up its state, no more references to ResourceHandle.
    // FIXME.  Are you sure about this deref?  Also need to check if we have to finish_request
    // in the destructor.
    info->job->deref();
    delete info;

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

static void crnetCallbackResponseStartedOnMainThread(void* context)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    CrNetResponseStartedInfo* info = static_cast<CrNetResponseStartedInfo*>(context);
    info->job->crnetResponseStarted(info);
    delete info;

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

static void crnetCallbackDataReceivedOnMainThread(void* context)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    CrNetDataReceivedInfo* info = static_cast<CrNetDataReceivedInfo*>(context);
    info->job->crnetDataReceived(info);
    // Tell crnet that we are done with the data buffer.
    crnet_finish_data_received(info->crnetHandle);
    delete info;

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

static void crnetCallbackRedirectReceivedOnMainThread(void* context)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    CrNetRedirectReceivedInfo* info = static_cast<CrNetRedirectReceivedInfo*>(context);
    info->job->crnetRedirectReceived(info);
    delete info;

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

static void crnetCallbackAuthRequiredOnMainThread(void* context)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    CrNetAuthRequiredInfo* info = static_cast<CrNetAuthRequiredInfo*>(context);
    info->job->crnetAuthRequired(info);
    delete info;

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

static void crnetCallbackSslErrorOnMainThread(void* context)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    CrNetSslErrorInfo* info = static_cast<CrNetSslErrorInfo*>(context);
    info->m_job->crnetSslError(info);
    delete info;

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

static void crnetCallback(struct crnet_handle* handle, int type, void* info)
{
    LOG(Network, "--> %s handle=%p", WTF_PRETTY_FUNCTION, (void*)handle);

    // FIXME
    //pthread_init_current_np(static_cast<pthread_main_np_t>(crnet_get_main_thread(handle)));

    // This callback runs on the Chromium thread.  We need to WebCore functions
    // on the main thread.  Package the necessary pieces of data and then
    // schedule a call on the main thread via callOnMainThread().

    std::function<void ()> func = nullptr;
    void* context = nullptr;
    void* caller_data = crnet_get_caller_data(handle);
    // FIXME.  Are you sure this is the right cast?  Sure about the class?
    ResourceHandle* job = reinterpret_cast<ResourceHandle*>(caller_data);
    switch (type) {
        case CRNET_CALLBACK_TYPE_RESPONSE_STARTED: {
            struct crnet_response_started_info* eventInfo =
                    static_cast<struct crnet_response_started_info*>(info);
            CrNetResponseStartedInfo* ctx = new CrNetResponseStartedInfo(job, handle, eventInfo);
            context = static_cast<void*>(ctx);
            func = [=] { crnetCallbackResponseStartedOnMainThread(context); };
            break;
        }
        case CRNET_CALLBACK_TYPE_DATA_RECEIVED: {
            struct crnet_data_received_info* eventInfo =
                    static_cast<struct crnet_data_received_info*>(info);
            CrNetDataReceivedInfo* ctx = new CrNetDataReceivedInfo(job, handle, eventInfo);
            context = static_cast<void*>(ctx);
            func = [=] { crnetCallbackDataReceivedOnMainThread(context); };
            break;
        }
        case CRNET_CALLBACK_TYPE_REQUEST_COMPLETED: {
            struct crnet_request_completed_info* eventInfo =
                    static_cast<struct crnet_request_completed_info*>(info);
            CrNetRequestCompletedInfo* ctx = new CrNetRequestCompletedInfo(job, handle, eventInfo);
            context = static_cast<void*>(ctx);
            func = [=] { crnetCallbackRequestCompletedOnMainThread(context); };
            break;
        }
        case CRNET_CALLBACK_TYPE_REDIRECT_RECEIVED: {
            struct crnet_redirect_received_info* eventInfo =
                    static_cast<struct crnet_redirect_received_info*>(info);
            CrNetRedirectReceivedInfo* ctx = new CrNetRedirectReceivedInfo(job, handle, eventInfo);
            context = static_cast<void*>(ctx);
            func = [=] { crnetCallbackRedirectReceivedOnMainThread(context); };
            break;
        }
        case CRNET_CALLBACK_TYPE_AUTH_REQUIRED: {
            struct crnet_auth_required_info* authInfo =
                    static_cast<struct crnet_auth_required_info*>(info);
            CrNetAuthRequiredInfo* ctx = new CrNetAuthRequiredInfo(job, handle, authInfo);
            context = static_cast<void*>(ctx);
            func = [=] { crnetCallbackAuthRequiredOnMainThread(context); };
            break;
        }
        case CRNET_CALLBACK_TYPE_SSL_ERROR: {
            struct crnet_ssl_info* sslInfo =
                    static_cast<struct crnet_ssl_info*>(info);
            CrNetSslErrorInfo* ctx = new CrNetSslErrorInfo(job, handle, sslInfo);
            context = static_cast<void*>(ctx);
            func = [=] { crnetCallbackSslErrorOnMainThread(context); };
            break;
        }
        case CRNET_CALLBACK_TYPE_DATA_UPLOADED: {
            struct crnet_data_uploaded_info* eventInfo =
                    static_cast<struct crnet_data_uploaded_info*>(info);
            unsigned long long sent = eventInfo->bytes_uploaded;
            unsigned long long total = eventInfo->total_bytes_to_upload;
            func = [job, sent, total, handle] {
                job->crnetDataUploaded(sent, total);
                crnet_finish_data_uploaded(handle);
            };
            break;
        }
        default:
            break;
    }

    if (func)
        callOnMainThread(WTFMove(func));

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

struct crnet_handle* ResourceHandle::makeCrnetRequest(ResourceRequest& request, NetworkingContext* context,
                                                      void* callbackData, void (*callback)(struct crnet_handle*, int, void*))
{
    const URL& url = request.url();
    String urlString = url.string();
    CString urlCStr = urlString.latin1();
    // crnet internally makes a copy of the url if necessary.
    const char *urlRaw = urlCStr.data();

    LOG(Network, "%s %d urlRaw=%s", __FILE__, __LINE__, urlRaw);

    String referrer = request.httpReferrer();
    CString referrerCStr = referrer.latin1();

    String method = request.httpMethod();
    CString methodCStr = method.latin1();

    String firstPartyString = request.firstPartyForCookies().string();
    CString firstPartyCStr = firstPartyString.latin1();

    struct crnet_request req;
    req.method = methodCStr.data();
    req.url = urlRaw;
    req.http_referrer = referrerCStr.data();
    req.caller_data = callbackData;
    req.callback = callback;
    //req.main_thread = static_cast<void*>(pthread_get_main_np());
    req.first_party_url_for_cookies = firstPartyCStr.data();
    req.main_frame = (request.requester() == ResourceRequest::Requester::Main);
    req.report_upload_progress = request.httpBody() && request.reportUploadProgress();

    // We may get a null context from DownloadManager when it does not know which
    // context to use.  Use the default context in that case.
    struct crnet_context* crnetContext;
    if (context)
        crnetContext = context->storageSession().crnetContext();
    else
        crnetContext = NetworkStorageSession::defaultStorageSession().crnetContext();
    struct crnet_handle* handle = crnet_create_request(crnetContext, &req);

    // Set timeout if available.
    double timeout = request.timeoutInterval(); // seconds
    if (timeout > 0)
        crnet_set_timeout(handle, timeout);

    // If the URL has an embedded user:password, pass it to crnet.
    if (!url.user().isEmpty() || !url.pass().isEmpty()) {
        LOG(Network, "Auth user:pass=%s:%s", url.user().latin1().data(), url.pass().latin1().data());

        String bufferUser, bufferPass;
        const UChar* user;
        const UChar* pass;
        user = temporary16BitStringData(url.user(), bufferUser);
        pass = temporary16BitStringData(url.pass(), bufferPass);
        crnet_set_initial_credential(handle, reinterpret_cast<const uint16_t*>(user), url.user().length(),
            reinterpret_cast<const uint16_t*>(pass), url.pass().length());
    }

    // Loop through the headers and add each one.
    const HTTPHeaderMap& httpHeaders = request.httpHeaderFields();
    for (HTTPHeaderMap::const_iterator it = httpHeaders.begin(); it != httpHeaders.end(); ++it) {
        LOG(Network, "HTTP header. %s: %s", it->key.latin1().data(), it->value.latin1().data());

        // Remove headers that crnet overrides or ignores.  For example, we need
        // to explicitly set the referrer.  The chromium network stack removes
        // the referrer in the extra HTTP headers we pass to it.
        //
        // Other stacks take similar actions.  See win/ResourceHandleWin.cpp.
        if (equalIgnoringASCIICase(it->key, "Referer"))
            continue;

        // Let User-Agent through.  WebCore seems to set it for all requests.
        // It overrides the default user agent string in the network stack.

        crnet_add_http_header(handle, it->key.latin1().data(), it->value.latin1().data());
    }

    // Request body.
    if (request.httpBody()) {
        FormData* body = request.httpBody();
        const Vector<FormDataElement>& elements = body->elements();
        for (size_t i = 0; i < elements.size(); ++i) {
            const FormDataElement& e = elements[i];
            if (e.m_type == FormDataElement::Type::Data) {
                // crnet holds the pointer and does not make a copy of the data
                crnet_add_request_body(handle, e.m_data.data(), e.m_data.size());
            } else if (e.m_type == FormDataElement::Type::EncodedFile) {
                CString filenameCStr = e.m_filename.utf8();
                uint64_t length = e.m_fileLength == BlobDataItem::toEndOfFile ?
                        std::numeric_limits<uint64_t>::max() : e.m_fileLength;
                double mtime = e.m_expectedFileModificationTime;
                crnet_add_request_body_file(handle, filenameCStr.data(), e.m_fileStart, length,
                                            isValidFileTime(mtime) ? mtime : 0.0);
            } else if (e.m_type == FormDataElement::Type::EncodedBlob) {
                // Blob code is mostly from ResourceHandleSoup.cpp
                ASSERT(blobRegistry().isBlobRegistryImpl());
                RefPtr<BlobData> blobData = static_cast<BlobRegistryImpl&>(blobRegistry()).getBlobDataFromURL(URL(ParsedURLString, e.m_url));
                if (blobData) {
                    for (size_t j = 0; j < blobData->items().size(); ++j) {
                        const BlobDataItem& blobItem = blobData->items()[j];
                        if (blobItem.type() == BlobDataItem::Type::Data)
                            crnet_add_request_body(handle, reinterpret_cast<const char*>(blobItem.data().data()->data() + static_cast<int>(blobItem.offset())), static_cast<int>(blobItem.length()));
                        else if (blobItem.type() == BlobDataItem::Type::File) {
                            CString pathCStr = blobItem.file()->path().utf8();
                            uint64_t length = blobItem.length() == BlobDataItem::toEndOfFile ?
                                    std::numeric_limits<uint64_t>::max() : blobItem.length();
                            double mtime = blobItem.file()->expectedModificationTime();
                            crnet_add_request_body_file(handle, pathCStr.data(),
                                                        blobItem.offset(), length,
                                                        isValidFileTime(mtime) ? mtime : 0.0);
                        }
                    }
                }
            }
        }
    }

    crnet_set_wait_for_continue(handle, 1);
    return handle;
}

bool ResourceHandle::start()
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    // The frame could be null if the ResourceHandle is not associated to any
    // Frame, e.g. if we are downloading a file.
    // If the frame is not null but the page is null this must be an attempted
    // load from an unload handler, so let's just block it.
    // If both the frame and the page are not null the context is valid.
    if (d->m_context && !d->m_context->isValid())
        return false;

    struct crnet_handle* ch = makeCrnetRequest(firstRequest(), context(), static_cast<void*>(this), crnetCallback);
    bool result = false;
    if (ch) {
        result = true;
        getInternal()->m_crnetHandle = ch;
        // crnet is holding a reference.
        // One way or another, we get a REQUESTED_COMPLETED callback from crnet.
        // The only place where we deref is crnetCallbackRequestCompletedOnMainThread.
        ref();
        crnet_start_request(ch, 0);
    }
    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
    return result;
}

void ResourceHandle::cancel()
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);
    // If we have the crnet handle for this request, tell crnet to cancel.
    // This is asynchronous.  crnet might have scheduled callbacks, which
    // would run shortly afterwards.  Remember that we requested cancellation,
    // and do not call crnet functions using this handle from now on.
    if (!d->m_crnetCancelled && d->m_crnetHandle)
        crnet_cancel_request(d->m_crnetHandle);
    d->m_crnetCancelled = true;
    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
    return;
}

void ResourceHandle::platformSetDefersLoading(bool)
{
    notImplemented();
}

bool ResourceHandle::shouldUseCredentialStorage()
{
    return (!client() || client()->shouldUseCredentialStorage(this)) && firstRequest().url().protocolIsInHTTPFamily();
}

void ResourceHandle::platformLoadResourceSynchronously(NetworkingContext* context, const ResourceRequest& request, StoredCredentials, ResourceError& error, ResourceResponse& response, Vector<char>& data)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    // We do not use ResourceHandleClient's functionality at all.
    // Just use the base implementation to create ResourceHandle.
    ResourceHandleClient loader;
    RefPtr<ResourceHandle> handle =
            adoptRef(new ResourceHandle(context, request, &loader, true, false));

    struct crnet_handle* ch = makeCrnetRequest(handle->firstRequest(), handle->context(), static_cast<void*>(handle.get()), crnetCallback);
    if (ch) {
        ResourceError resError;
        const char* body_data;
        int64_t body_byte_count;

        crnet_start_request(ch, 1);
        if (unpackCrnetResponse(ch, handle->getInternal()->m_response, resError, &body_data, &body_byte_count)) {
            response = handle->getInternal()->m_response;
            if (body_byte_count > 0)
                data.append(body_data, body_byte_count);
            LOG(Network, "Synchronous request completed. body_byte_count=%d",
                (int)body_byte_count);
        } else {
            error = resError;
        }
        crnet_finish_request(ch);
    } else {
        error = ResourceError(errorDomainNetwork, 0, handle->firstRequest().url(),
                              String("makeCrnetRequest failed"));
    }

    // RefPtr derefs ResourceHandle as it goes out of scope.
    // Do not explicitly ref, deref in this function.

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

void ResourceHandle::didReceiveAuthenticationChallenge(const AuthenticationChallenge& challenge)
{
    LOG(Network, "--> %s", WTF_PRETTY_FUNCTION);

    // Almost all the code here is from the curl and/or mac ports.
    // Added comments for clarification.

    // Upon the first challenge, try the cred embedded in the URL.
    if (!d->m_user.isNull() && !d->m_pass.isNull()) {
        LOG(Network, "Try the URL embedded credential");

        // Update the credential cache first
        if (challenge.failureResponse().httpStatusCode() == 401) {
            CredentialStorage::defaultCredentialStorage().set(Credential(d->m_user, d->m_pass, CredentialPersistenceNone),
                                   challenge.protectionSpace(),
                                   challenge.failureResponse().url());
        } else {
            // FIXME.  When do we see this?
        }

        // Then retry the request with the new credential
        crnetRetryAuth(d->m_crnetHandle, d->m_user, d->m_pass);
        // Nullify user, password so we do not try them again.
        d->m_user = String();
        d->m_pass = String();
        return;
    }

    // Update the credential storage as necessary.
    if (shouldUseCredentialStorage()) {
        // m_initialCredential is from the credential storage.  If it failed, so
        // clear it from the storage.  previousFailureCount is 0.
        //
        // If previousFailureCount > 0, then remove the just tried credential.
        // This includes the credential embedded in the request URL.
        if (!d->m_initialCredential.isEmpty() || challenge.previousFailureCount())
            CredentialStorage::defaultCredentialStorage().remove(challenge.protectionSpace());

        // Upon the first challenge, look up the credential cache.
        // See if there is a cached credential for the current protection space, and
        // if that credential (user, password) is not same as the one we just tried
        // (initial credential).  If so, try it without prompting the user.
        if (!challenge.previousFailureCount()) {
            Credential credential = CredentialStorage::defaultCredentialStorage().get(challenge.protectionSpace());
            if (!credential.isEmpty() && credential != d->m_initialCredential) {
                ASSERT(credential.persistence() == CredentialPersistenceNone);
                if (challenge.failureResponse().httpStatusCode() == 401) {
                    // Store the credential back, possibly adding it as a default for this directory.
                    CredentialStorage::defaultCredentialStorage().set(credential, challenge.protectionSpace(), challenge.failureResponse().url());
                }

                // Retry the request with the cached credential.
                crnetRetryAuth(d->m_crnetHandle, credential.user(), credential.password());
                return;
            }
        }
    }

    d->m_currentWebChallenge = challenge;
    if (client())
        client()->didReceiveAuthenticationChallenge(this, d->m_currentWebChallenge);

    LOG(Network, "<-- %s", WTF_PRETTY_FUNCTION);
}

void ResourceHandle::receivedCredential(const AuthenticationChallenge& challenge, const Credential& credential)
{
    // Almost all the code here is from the curl and/or mac ports.

    if (challenge != d->m_currentWebChallenge)
        return;

    // Special case the handling of bad server certificates.
    // The credentials have no effect.
    if (challenge.protectionSpace().authenticationScheme() == ProtectionSpaceAuthenticationSchemeServerTrustEvaluationRequested) {
        if (!d->m_crnetCancelled && d->m_crnetHandle)
            crnet_retry_ignore_sslerror(d->m_crnetHandle);
        clearAuthentication();
        return;
    }

    if (credential.isEmpty()) {
        receivedRequestToContinueWithoutCredential(challenge);
        return;
    }

    if (shouldUseCredentialStorage()) {
        if (challenge.failureResponse().httpStatusCode() == 401) {
            URL urlToStore = challenge.failureResponse().url();
            CredentialStorage::defaultCredentialStorage().set(credential, challenge.protectionSpace(), urlToStore);
        }
    }

    if (!d->m_crnetCancelled && d->m_crnetHandle)
        crnetRetryAuth(d->m_crnetHandle, credential.user(), credential.password());

    clearAuthentication();
}

void ResourceHandle::receivedRequestToContinueWithoutCredential(const AuthenticationChallenge& challenge)
{
    if (challenge != d->m_currentWebChallenge)
        return;

    if (!d->m_crnetCancelled && d->m_crnetHandle) {
        // Special case the handling of bad server certificates.
        // We just want to cancel the request.
        if (challenge.protectionSpace().authenticationScheme() == ProtectionSpaceAuthenticationSchemeServerTrustEvaluationRequested) {
            if (!d->m_crnetCancelled) {
                // Indicate the failure because the caller won't
                if (d->client() && !d->m_crnetDidFail) {
                    d->m_crnetDidFail = true;
                    d->client()->didFail(this, cancelledError(firstRequest()));
                }
                cancel();
            }
            return;
        }

        // This is different from "cancel" in that we want to load the response
        // as usual.  For example, we display the 401 response.
        crnet_cancel_auth(d->m_crnetHandle);
    }

    clearAuthentication();
}

void ResourceHandle::receivedCancellation(const AuthenticationChallenge& challenge)
{
    if (challenge != d->m_currentWebChallenge)
        return;

    // Call ResourceLoader, which in turn calls our cancel().
    if (client())
        client()->receivedCancellation(this, challenge);
}

void ResourceHandle::receivedRequestToPerformDefaultHandling(const AuthenticationChallenge&)
{
    // We do not use this handler with chromium.
    ASSERT_NOT_REACHED();
}

void ResourceHandle::receivedChallengeRejection(const AuthenticationChallenge&)
{
    // We do not use this handler with chromium.
    ASSERT_NOT_REACHED();
}

} // namespace WebCore
