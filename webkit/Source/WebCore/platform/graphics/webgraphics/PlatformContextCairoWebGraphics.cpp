/*
 * Copyright (C) 2016 Naver Corp. All rights reserved.
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
#include "PlatformContextCairo.h"

#if USE(CAIRO)

#include "CairoUtilities.h"
#include "Gradient.h"
#include "GraphicsContext.h"
#include "Pattern.h"
#include "WebGraphicsUtilities.h"
#include <cairo.h>

using WebGraphics::WebGraphicsContext;

namespace WebCore {

cairo_user_data_key_t graphicsContextUserDataKey;

void graphicsContextDestroyUserData(void* userData)
{
    WebGraphicsContext* context = reinterpret_cast<WebGraphicsContext*>(userData);
    delete context;
}

class PlatformContextCairo::State {
public:
    State(const RefPtr<cairo_t>& cr)
        : m_cr(cr)
        , m_context(nullptr)
    {
        if (m_cr) {
            m_context = reinterpret_cast<WebGraphicsContext*>(cairo_get_user_data(m_cr.get(), &graphicsContextUserDataKey));
            ASSERT(m_context);
        }
    }

    State(const State& state)
        : m_cr(state.m_cr)
        , m_context(state.m_context)
    {
    }

    RefPtr<cairo_t> m_cr;
    WebGraphicsContext* m_context;
};

PlatformContextCairo::PlatformContextCairo(cairo_t* cr)
    : m_cr(cr)
{
    if (m_cr) {
        cairo_surface_t* surface = cairo_get_target(cr);
        WebGraphicsContext* context = new WebGraphicsContext(surface);
        cairo_set_user_data(cr, &graphicsContextUserDataKey, context, graphicsContextDestroyUserData);
        save();
    }
}

PlatformContextCairo::~PlatformContextCairo()
{
}

void PlatformContextCairo::save()
{
    m_stateStack.append(State(m_cr));
    m_state = &m_stateStack.last();
}

void PlatformContextCairo::restore()
{
    m_stateStack.removeLast();
    ASSERT(!m_stateStack.isEmpty());
    m_state = &m_stateStack.last();
}

void PlatformContextCairo::pushImageMask(cairo_surface_t* surface, const FloatRect& rect)
{
    m_state->m_context->clipToImageBuffer(surface, toWebGraphics(rect));
}

void PlatformContextCairo::drawSurfaceToContext(cairo_surface_t* surface, const FloatRect& destRect, const FloatRect& originalSrcRect, GraphicsContext& graphicsContext)
{
    m_state->m_context->drawSurfaceToContext(surface, toWebGraphics(destRect), toWebGraphics(originalSrcRect), toWebGraphics(graphicsContext.state()));
}

void PlatformContextCairo::setImageInterpolationQuality(InterpolationQuality quality)
{
    ASSERT_NOT_REACHED();
}

InterpolationQuality PlatformContextCairo::imageInterpolationQuality() const
{
    ASSERT_NOT_REACHED();
    return InterpolationDefault;
}

float PlatformContextCairo::globalAlpha() const
{
    ASSERT_NOT_REACHED();
    return 1.0f;
}

void PlatformContextCairo::setGlobalAlpha(float globalAlpha)
{
    ASSERT_NOT_REACHED();
}

void PlatformContextCairo::prepareForFilling(const GraphicsContextState& state, PatternAdjustment patternAdjustment)
{
    ASSERT_NOT_REACHED();
}

void PlatformContextCairo::prepareForStroking(const GraphicsContextState& state, AlphaPreservation alphaPreservation)
{
    ASSERT_NOT_REACHED();
}

void PlatformContextCairo::clipForPatternFilling(const GraphicsContextState& state)
{
    ASSERT_NOT_REACHED();
}

} // namespace WebCore

#endif // USE(CAIRO)
