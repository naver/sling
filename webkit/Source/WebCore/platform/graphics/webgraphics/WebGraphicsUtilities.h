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

#ifndef WebGraphicsUtilities_h
#define WebGraphicsUtilities_h

#include "Gradient.h"
#include "GraphicsContext.h"
#include "Path.h"
#include "Pattern.h"
#include "PlatformPathCairo.h"
#include <vector>
#include <webgraphics/WebGraphicsContext.h>

namespace WebCore {

static WebGraphics::StrokeStyle toWebGraphics(StrokeStyle in)
{
    WebGraphics::StrokeStyle out = WebGraphics::NoStroke;
    switch (in) {
    case NoStroke:
        out = WebGraphics::NoStroke;
        break;
    case SolidStroke:
        out = WebGraphics::SolidStroke;
        break;
    case DottedStroke:
        out = WebGraphics::DottedStroke;
        break;
    case DashedStroke:
        out = WebGraphics::DashedStroke;
        break;
    case DoubleStroke:
        out = WebGraphics::DoubleStroke;
        break;
    case WavyStroke:
        out = WebGraphics::WavyStroke;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }
    return out;
}

static WebGraphics::FillRule toWebGraphics(WindRule in)
{
    WebGraphics::FillRule out = CAIRO_FILL_RULE_WINDING;
    switch (in) {
    case RULE_NONZERO:
        out = CAIRO_FILL_RULE_WINDING;
        break;
    case RULE_EVENODD:
        out = CAIRO_FILL_RULE_EVEN_ODD;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }
    return out;
}

static WebGraphics::Operator toWebGraphics(CompositeOperator in)
{
    WebGraphics::Operator out = CAIRO_OPERATOR_OVER;
    switch (in) {
    case CompositeClear:
        ASSERT_NOT_REACHED();
        break;
    case CompositeCopy:
        out = CAIRO_OPERATOR_SOURCE;
        break;
    case CompositeSourceOver:
        out = CAIRO_OPERATOR_OVER;
        break;
    case CompositeSourceIn:
        out = CAIRO_OPERATOR_IN;
        break;
    case CompositeSourceOut:
        out = CAIRO_OPERATOR_OUT;
        break;
    case CompositeSourceAtop:
        out = CAIRO_OPERATOR_ATOP;
        break;
    case CompositeDestinationOver:
        out = CAIRO_OPERATOR_DEST_OVER;
        break;
    case CompositeDestinationIn:
        out = CAIRO_OPERATOR_DEST_IN;
        break;
    case CompositeDestinationOut:
        out = CAIRO_OPERATOR_DEST_OUT;
        break;
    case CompositeDestinationAtop:
        out = CAIRO_OPERATOR_DEST_ATOP;
        break;
    case CompositeXOR:
        out = CAIRO_OPERATOR_XOR;
        break;
    case CompositePlusDarker:
        out = CAIRO_OPERATOR_DARKEN;
        break;
    case CompositePlusLighter:
        out = CAIRO_OPERATOR_ADD;
        break;
    case CompositeDifference:
        out = CAIRO_OPERATOR_DIFFERENCE;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }
    return out;
}

static WebGraphics::Operator toWebGraphics(BlendMode in)
{
    WebGraphics::Operator out = CAIRO_OPERATOR_OVER;
    switch (in) {
    case BlendModeNormal:
        out = CAIRO_OPERATOR_OVER;
        break;
    case BlendModeMultiply:
        out = CAIRO_OPERATOR_MULTIPLY;
        break;
    case BlendModeScreen:
        out = CAIRO_OPERATOR_SCREEN;
        break;
    case BlendModeDarken:
        out = CAIRO_OPERATOR_DARKEN;
        break;
    case BlendModeLighten:
        out = CAIRO_OPERATOR_ADD;
        break;
    case BlendModeOverlay:
        out = CAIRO_OPERATOR_OVERLAY;
        break;
    case BlendModeColorDodge:
        out = CAIRO_OPERATOR_COLOR_DODGE;
        break;
    case BlendModeColorBurn:
        out = CAIRO_OPERATOR_COLOR_BURN;
        break;
    case BlendModeHardLight:
        out = CAIRO_OPERATOR_HARD_LIGHT;
        break;
    case BlendModeSoftLight:
        out = CAIRO_OPERATOR_SOFT_LIGHT;
        break;
    case BlendModeDifference:
        out = CAIRO_OPERATOR_DIFFERENCE;
        break;
    case BlendModeExclusion:
        out = CAIRO_OPERATOR_EXCLUSION;
        break;
    case BlendModeHue:
        out = CAIRO_OPERATOR_HSL_HUE;
        break;
    case BlendModeSaturation:
        out = CAIRO_OPERATOR_HSL_SATURATION;
        break;
    case BlendModeColor:
        out = CAIRO_OPERATOR_HSL_COLOR;
        break;
    case BlendModeLuminosity:
        out = CAIRO_OPERATOR_HSL_LUMINOSITY;
        break;
    case BlendModePlusDarker:        
        break;
    case BlendModePlusLighter:
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }
    return out;
}

static WebGraphics::InterpolationQuality toWebGraphics(InterpolationQuality in)
{
    WebGraphics::InterpolationQuality out = WebGraphics::InterpolationQuality::InterpolationDefault;
    switch (in) {
    case InterpolationDefault:
        out = WebGraphics::InterpolationDefault;
        break;
    case InterpolationNone:
        out = WebGraphics::InterpolationNone;
        break;
    case InterpolationLow:
        out = WebGraphics::InterpolationLow;
        break;
    case InterpolationMedium:
        out = WebGraphics::InterpolationMedium;
        break;
    case InterpolationHigh:
        out = WebGraphics::InterpolationHigh;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }
    return out;
}

static WebGraphics::LineCap toWebGraphics(LineCap lineCap)
{
    WebGraphics::LineCap out = CAIRO_LINE_CAP_BUTT;
    switch (lineCap)
    {
    case ButtCap:
        out = CAIRO_LINE_CAP_BUTT;
        break;
    case RoundCap:
        out = CAIRO_LINE_CAP_ROUND;
        break;
    case SquareCap:
        out = CAIRO_LINE_CAP_SQUARE;
        break;
    default:
        ASSERT_NOT_REACHED();
    }
    return out;
}

static WebGraphics::LineJoin toWebGraphics(LineJoin lineJoin)
{
    WebGraphics::LineJoin out = CAIRO_LINE_JOIN_MITER;
    switch (lineJoin)
    {
    case MiterJoin:
        out = CAIRO_LINE_JOIN_MITER;
        break;
    case RoundJoin:
        out = CAIRO_LINE_JOIN_ROUND;
        break;
    case BevelJoin:
        out = CAIRO_LINE_JOIN_BEVEL;
        break;
    default:
        ASSERT_NOT_REACHED();
    }
    return out;
}

static WebGraphics::WebGraphicsContext::DocumentMarkerLineStyle toWebGraphics(GraphicsContext::DocumentMarkerLineStyle lineStyle)
{
    WebGraphics::WebGraphicsContext::DocumentMarkerLineStyle out = WebGraphics::WebGraphicsContext::DocumentMarkerSpellingLineStyle;
    switch (lineStyle)
    {
    case GraphicsContext::DocumentMarkerSpellingLineStyle:
        out = WebGraphics::WebGraphicsContext::DocumentMarkerSpellingLineStyle;
        break;
    case GraphicsContext::DocumentMarkerGrammarLineStyle:
        out = WebGraphics::WebGraphicsContext::DocumentMarkerGrammarLineStyle;
        break;
    case GraphicsContext::DocumentMarkerAutocorrectionReplacementLineStyle:
        out = WebGraphics::WebGraphicsContext::DocumentMarkerAutocorrectionReplacementLineStyle;
        break;
    case GraphicsContext::DocumentMarkerDictationAlternativesLineStyle:
        out = WebGraphics::WebGraphicsContext::DocumentMarkerDictationAlternativesLineStyle;
        break;
    default:
        ASSERT_NOT_REACHED();
    }
    return out;
}

static WebGraphics::WebGraphicsContext::RoundingMode toWebGraphics(GraphicsContext::RoundingMode roundingMode)
{
    WebGraphics::WebGraphicsContext::RoundingMode out = WebGraphics::WebGraphicsContext::RoundAllSides;
    switch (roundingMode)
    {
    case GraphicsContext::RoundAllSides:
        out = WebGraphics::WebGraphicsContext::RoundAllSides;
        break;
    case GraphicsContext::RoundOriginAndDimensions:
        out = WebGraphics::WebGraphicsContext::RoundOriginAndDimensions;
        break;
    default:
        ASSERT_NOT_REACHED();
    }
    return out;
}

static WebGraphics::FloatPoint toWebGraphics(const FloatPoint& in)
{
    return { in.x(), in.y() };
}

static WebGraphics::FloatSize toWebGraphics(const FloatSize& in)
{
    return { in.width(), in.height() };
}

static WebGraphics::FloatRect toWebGraphics(const FloatRect& in)
{
    return { { in.x(), in.y() }, { in.width(), in.height() } };
}

static FloatRect toWebCore(const WebGraphics::FloatRect& in)
{
    return FloatRect(in.first[0], in.first[1], in.second[0], in.second[1]);
}

static WebGraphics::IntRect toWebGraphics(const IntRect& in)
{
    return { { in.x(), in.y() }, { in.width(), in.height() } };
}

static IntRect toWebCore(const WebGraphics::IntRect& in)
{
    return IntRect(in.first[0], in.first[1], in.second[0], in.second[1]);
}

static WebGraphics::FloatRoundedRect toWebGraphics(const FloatRoundedRect& in)
{
    return std::make_pair(toWebGraphics(in.rect()), 
        std::array<WebGraphics::FloatSize, 4> { toWebGraphics(in.radii().topLeft()), toWebGraphics(in.radii().topRight()), toWebGraphics(in.radii().bottomLeft()), toWebGraphics(in.radii().bottomRight()) });
}

static WebGraphics::FloatColor toWebGraphics(const Color& in)
{
    float r, g, b, a;
    in.getRGBA(r, g, b, a);
    return { r, g, b, a };
}

static WebGraphics::Path* toWebGraphics(const Path* in)
{
    if (!in || !in->platformPath())
        return nullptr;
    return in->platformPath()->context();
}

static WebGraphics::Path* toWebGraphics(const Path& in)
{
    return toWebGraphics(&in);
}

static WebGraphics::Gradient* toWebGraphics(Gradient* in)
{
    if (!in)
        return nullptr;
    return in->platformGradient();
}

static WebGraphics::Gradient* toWebGraphics(Gradient& in)
{
    return toWebGraphics(&in);
}

static PassRefPtr<WebGraphics::Pattern> toWebGraphics(Pattern* in)
{
    if (!in)
        return nullptr;
    return adoptRef(in->createPlatformPattern(AffineTransform()));
}

static WebGraphics::AffineTransform toWebGraphics(const AffineTransform& in)
{
    return { in.a(), in.b(), in.c(), in.d(), in.e(), in.f() };
}

static AffineTransform toWebCore(const WebGraphics::AffineTransform& in)
{
    return AffineTransform(in[0], in[1], in[2], in[3], in[4], in[5]);
}

static std::vector<WebGraphics::FloatRect> toWebGraphics(const Vector<FloatRect>& in)
{
    std::vector<WebGraphics::FloatRect> out(in.size());
    std::transform(in.begin(), in.end(), out.begin(), [] (const FloatRect& entry) {
        return WebGraphics::FloatRect { { entry.x(), entry.y() }, { entry.width(), entry.height() } };
    });
    return out;
}

static PassRefPtr<WebGraphics::Image> toWebGraphics(Image& in)
{
    return in.nativeImageForCurrentFrame();
}

static WebGraphics::DashArray toWebGraphics(const DashArray& in)
{
    WebGraphics::DashArray out(in.size());
    std::copy(in.begin(), in.end(), out.begin());
    return out;
}

struct WebGraphicsContextStateWrapper
{
    RefPtr<cairo_pattern_t> m_strokePattern;
    RefPtr<cairo_pattern_t> m_fillPattern;
    WebGraphics::WebGraphicsContextState m_state;

    operator const WebGraphics::WebGraphicsContextState&() const { return m_state; }
};

static WebGraphicsContextStateWrapper toWebGraphics(const GraphicsContextState& in)
{
    WebGraphicsContextStateWrapper wrapper;
    WebGraphics::WebGraphicsContextState& out = wrapper.m_state;

    out.strokeGradient = toWebGraphics(in.strokeGradient.get());
    wrapper.m_strokePattern = toWebGraphics(in.strokePattern.get());
    out.strokePattern = wrapper.m_strokePattern.get();

    out.fillGradient = toWebGraphics(in.fillGradient.get());
    wrapper.m_fillPattern = toWebGraphics(in.fillPattern.get());
    out.fillPattern = wrapper.m_fillPattern.get();

    out.shadowOffset = toWebGraphics(in.shadowOffset);

    out.strokeThickness = in.strokeThickness;
    out.shadowBlur = in.shadowBlur;

    out.textDrawingMode = in.textDrawingMode;

    out.strokeColor = toWebGraphics(in.strokeColor);
    out.fillColor = toWebGraphics(in.fillColor);
    out.shadowColor = toWebGraphics(in.shadowColor);

    out.strokeStyle = toWebGraphics(in.strokeStyle);
    out.fillRule = toWebGraphics(in.fillRule);

    out.alpha = in.alpha;
    out.compositeOperator = toWebGraphics(in.compositeOperator);
    out.imageInterpolationQuality = toWebGraphics(in.imageInterpolationQuality);
    
    out.shouldAntialias = in.shouldAntialias;
    out.shouldSmoothFonts  = in.shouldSmoothFonts;
    out.shouldSubpixelQuantizeFonts = in.shouldSubpixelQuantizeFonts;
    out.shadowsIgnoreTransforms = in.shadowsIgnoreTransforms;
    out.drawLuminanceMask = in.drawLuminanceMask;
    
    return wrapper;
}

} // namespace WebCore

#endif // WebGraphicsUtilities
