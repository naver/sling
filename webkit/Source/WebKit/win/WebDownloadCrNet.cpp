/*
 * Copyright (C) 2008 Brent Fulgham <bfulgham@gmail.com>. All Rights Reserved.
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

#include "WebKitDLL.h"
#include "WebDownload.h"

#include "DefaultDownloadDelegate.h"
#include "MarshallingHelpers.h"
#include "WebError.h"
#include "WebKit.h"
#include "WebKitLogging.h"
#include "WebMutableURLRequest.h"
#include "WebURLAuthenticationChallenge.h"
#include "WebURLCredential.h"
#include "WebURLResponse.h"

#include <wtf/text/CString.h>

#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <WebCore/BString.h>
#include <WebCore/NotImplemented.h>
#include <WebCore/ResourceError.h>
#include <WebCore/ResourceHandle.h>
#include <WebCore/ResourceRequest.h>
#include <WebCore/ResourceResponse.h>

using namespace WebCore;

// WebDownload ----------------------------------------------------------------

void WebDownload::init(ResourceHandle* handle, const ResourceRequest& request, const ResourceResponse& response, IWebDownloadDelegate* delegate)
{
   notImplemented();
   return;
}

void WebDownload::init(const URL& url, IWebDownloadDelegate* delegate)
{
   notImplemented();
   return;
}

// IWebDownload -------------------------------------------------------------------

HRESULT WebDownload::initWithRequest(
        /* [in] */ IWebURLRequest* request, 
        /* [in] */ IWebDownloadDelegate* delegate)
{
   notImplemented();
   return E_FAIL;
}

HRESULT WebDownload::initToResumeWithBundle(
        /* [in] */ BSTR bundlePath, 
        /* [in] */ IWebDownloadDelegate* delegate)
{
   notImplemented();
   return E_FAIL;
}

HRESULT WebDownload::start()
{
   notImplemented();
   return E_FAIL;
}

HRESULT WebDownload::cancel()
{
   notImplemented();
   return E_FAIL;
}

HRESULT WebDownload::cancelForResume()
{
   notImplemented();
   return E_FAIL;
}

HRESULT WebDownload::deletesFileUponFailure(
        /* [out, retval] */ BOOL* result)
{
   notImplemented();
   return E_FAIL;
}

HRESULT WebDownload::setDeletesFileUponFailure(
        /* [in] */ BOOL deletesFileUponFailure)
{
   notImplemented();
   return E_FAIL;
}

HRESULT WebDownload::setDestination(
        /* [in] */ BSTR path, 
        /* [in] */ BOOL allowOverwrite)
{
   notImplemented();
   return E_FAIL;
}

// IWebURLAuthenticationChallengeSender -------------------------------------------------------------------

HRESULT WebDownload::cancelAuthenticationChallenge(
        /* [in] */ IWebURLAuthenticationChallenge*)
{
   notImplemented();
   return E_FAIL;
}

HRESULT WebDownload::continueWithoutCredentialForAuthenticationChallenge(
        /* [in] */ IWebURLAuthenticationChallenge* challenge)
{
   notImplemented();
   return E_FAIL;
}

HRESULT WebDownload::useCredential(
        /* [in] */ IWebURLCredential* credential, 
        /* [in] */ IWebURLAuthenticationChallenge* challenge)
{
   notImplemented();
   return E_FAIL;
}
