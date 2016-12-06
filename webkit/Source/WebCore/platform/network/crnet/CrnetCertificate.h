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

#ifndef CrnetCertificate_h
#define CrnetCertificate_h

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>

struct crnet_ssl_certificate;

namespace WebCore {

class CrnetCertificate : public RefCounted<CrnetCertificate> {
public:
   static PassRefPtr<CrnetCertificate> create(struct crnet_ssl_certificate* certificate)
   {
       return adoptRef(new CrnetCertificate(certificate));
   }
   ~CrnetCertificate();

   void setStatus(int status) { m_status = status; }
   int status() { return m_status; }
   void setDerEncoded(Vector<char>& cert);
   const Vector<char>& derEncoded();
   bool containsNonRootSHA1SignedCertificate() const;
   int64_t validStart() const { return m_validStart; }
   int64_t validExpiry() const { return m_validExpiry; }
   void setValidStart(int64_t time) { m_validStart = time; }
   void setValidExpiry(int64_t time) { m_validExpiry = time; }

private:
    CrnetCertificate(struct crnet_ssl_certificate*);

    struct crnet_ssl_certificate* m_certificate;
    Vector<char> m_derEncodedCert;
    bool m_retrievedDer;
    int m_status;
    /* valid_start and valid_expiry are milliseconds since the epoch.
     * 0 means invalid or not present.
     */
    int64_t m_validStart; /* Validity start date */
    int64_t m_validExpiry; /* Expiration date */
};

} // namespace WebCore

#endif // CrnetCertificate_h
