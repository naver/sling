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
#include "CrnetCertificate.h"

#include <crnet.h>

namespace WebCore {

CrnetCertificate::CrnetCertificate(struct crnet_ssl_certificate* certificate)
    : m_certificate(certificate)
    , m_retrievedDer(false)
    , m_status(0)
{
}

CrnetCertificate::~CrnetCertificate()
{
    if (m_certificate) {
        crnet_free_certificate(m_certificate);
        m_certificate = nullptr;
    }
}

const Vector<char>& CrnetCertificate::derEncoded()
{
    // Get the DER encoded certificate from crnet.  Retrieve it once.
    if (!m_retrievedDer && m_certificate) {
        const char* array;
        int length;
        if (!crnet_get_der_encoded_certificate(m_certificate, &array, &length))
            m_derEncodedCert.append(array, length);
        m_retrievedDer = true;
    }
    return m_derEncodedCert;
}

void CrnetCertificate::setDerEncoded(Vector<char>& cert)
{
    m_retrievedDer = true;
    m_derEncodedCert = cert;
}

bool CrnetCertificate::containsNonRootSHA1SignedCertificate() const
{
    return !!(m_status & CRNET_CERT_STATUS_SHA1_SIGNATURE_PRESENT);
}

} // namespace WebCore
