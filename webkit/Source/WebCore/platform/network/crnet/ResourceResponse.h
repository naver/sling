/*
 * Copyright (C) 2006 Apple Inc.  All rights reserved.
 * Copyright (C) 2014-2015 Naver Corp. All rights reserved.
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

#ifndef ResourceResponse_h
#define ResourceResponse_h

#include "CrnetCertificate.h"
#include "ResourceResponseBase.h"

namespace WebCore {

class ResourceResponse : public ResourceResponseBase {
public:
    ResourceResponse()
        : m_responseFired(false)
    {
    }

    ResourceResponse(const URL& url, const String& mimeType, long long expectedLength, const String& textEncodingName)
        : ResourceResponseBase(url, mimeType, expectedLength, textEncodingName)
        , m_responseFired(false)
    {
    }

    void setResponseFired(bool fired) { m_responseFired = fired; }
    bool responseFired() { return m_responseFired; }

    void setSuggestedFilename(const String& name) { m_suggestedFilename = name; }
    void setCertificate(CrnetCertificate* cert) { m_certificate = cert; }
    CrnetCertificate* getCertificate() const { return m_certificate.get(); }

    template<class Encoder> void encode(Encoder&) const;
    template<class Decoder> static bool decode(Decoder&, ResourceResponse&);

private:
    friend class ResourceResponseBase;

    //cory std::unique_ptr<CrossThreadResourceResponseData> doPlatformCopyData(std::unique_ptr<CrossThreadResourceResponseData> data) const { return data; }
    //cory void doPlatformAdopt(std::unique_ptr<CrossThreadResourceResponseData>) { }

    virtual String platformSuggestedFilename() const { return m_suggestedFilename; }
    CertificateInfo platformCertificateInfo() const { return CertificateInfo(*this); }

    bool m_responseFired;
    String m_suggestedFilename;
    RefPtr<CrnetCertificate> m_certificate;
};

template<class Encoder>
void ResourceResponse::encode(Encoder& encoder) const
{
    ResourceResponseBase::encode(encoder);
    encoder.encode(m_suggestedFilename);
}

template<class Decoder>
bool ResourceResponse::decode(Decoder& decoder, ResourceResponse& response)
{
    if (!ResourceResponseBase::decode(decoder, response))
        return false;
    if (!decoder.decode(response.m_suggestedFilename))
        return false;
    return true;
}

//cory struct CrossThreadResourceResponseData : public CrossThreadResourceResponseDataBase {
//cory };

} // namespace WebCore

#endif // ResourceResponse_h
