/*
 * Copyright (C) 2014-2015 Naver Corp. All rights reserved.
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

#ifndef CertificateInfo_h
#define CertificateInfo_h

#include "CrnetCertificate.h"
#include "NotImplemented.h"

namespace WebCore {

class ResourceError;
class ResourceResponse;

class CertificateInfo {
public:
    CertificateInfo();
    explicit CertificateInfo(const WebCore::ResourceResponse&);
    explicit CertificateInfo(const WebCore::ResourceError&);
    ~CertificateInfo();

    bool containsNonRootSHA1SignedCertificate() const
    {
        if (!m_certificate)
            return false;
        return m_certificate->containsNonRootSHA1SignedCertificate();
    }

    CrnetCertificate* certificate() const { return m_certificate.get(); }
    void setCertificate(CrnetCertificate* cert) { m_certificate = cert; }

private:
    RefPtr<CrnetCertificate> m_certificate;
};

} // namespace WebCore

#endif // CertificateInfo_h
