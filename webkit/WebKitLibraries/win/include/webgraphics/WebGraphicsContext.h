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

#ifndef WebGraphicsContext_h
#define WebGraphicsContext_h

#include "WebGraphicsCairo.h"

namespace mozilla {
namespace gfx {
class DrawTargetBackend;
}
}

namespace WebGraphics {

enum TextDrawingMode {
    TextModeFill = 1 << 0,
    TextModeStroke = 1 << 1,
};

typedef unsigned TextDrawingModeFlags;

enum StrokeStyle {
    NoStroke,
    SolidStroke,
    DottedStroke,
    DashedStroke,
    DoubleStroke,
    WavyStroke,
};

enum InterpolationQuality {
    InterpolationDefault,
    InterpolationNone,
    InterpolationLow,
    InterpolationMedium,
    InterpolationHigh
};

struct WebGraphicsContextState {
    WebGraphicsContextState()
        : shouldAntialias(true)
        , shouldSmoothFonts(true)
        , shouldSubpixelQuantizeFonts(true)
        , shadowsIgnoreTransforms(false)
        , drawLuminanceMask(false)
    {
    }

    Gradient* strokeGradient { nullptr };
    Pattern* strokePattern { nullptr };
    
    Gradient* fillGradient { nullptr };
    Pattern* fillPattern { nullptr };

    FloatSize shadowOffset;

    float strokeThickness { 0 };
    float shadowBlur { 0 };

    TextDrawingModeFlags textDrawingMode { TextModeFill };
    float syntheticBoldOffset { 0 };

    FloatColor strokeColor { 0, 0, 0, 1 };
    FloatColor fillColor { 0, 0, 0, 1 };
    FloatColor shadowColor { 0, 0, 0, 0 };

    StrokeStyle strokeStyle { SolidStroke };
    FillRule fillRule { CAIRO_FILL_RULE_WINDING };

    float alpha { 1 };
    Operator compositeOperator { CAIRO_OPERATOR_OVER };
    InterpolationQuality imageInterpolationQuality { InterpolationDefault };

    bool shouldAntialias : 1;
    bool shouldSmoothFonts : 1;
    bool shouldSubpixelQuantizeFonts : 1;
    bool shadowsIgnoreTransforms : 1;
    bool drawLuminanceMask : 1;

    bool hasVisibleShadow() const { return shadowColor[3] > 0; }
    bool hasShadow() const { return hasVisibleShadow() && (shadowBlur || shadowOffset[0] || shadowOffset[1]); }
    bool hasBlurredShadow() const { return hasVisibleShadow() && shadowBlur; }
};

class WebGraphicsContextPrivate;

using BackendDrawTarget = mozilla::gfx::DrawTargetBackend;

class WebGraphicsContext {
public:
    WebGraphicsContext(GraphicsSurface*);
    WebGraphicsContext(BackendDrawTarget*);
    ~WebGraphicsContext();

    BackendDrawTarget& drawTarget() const;

    bool isAcceleratedContext() const;

    void save();
    void restore();

    void drawRect(const FloatRect&, float borderThickness, const WebGraphicsContextState&);
    void drawLine(const FloatPoint&, const FloatPoint&, const WebGraphicsContextState&);

    void drawEllipse(const FloatRect&, const WebGraphicsContextState& state);

    void fillPath(Path*, const WebGraphicsContextState&);
    void strokePath(Path*, const WebGraphicsContextState&);

    void fillEllipse(const FloatRect&);
    void strokeEllipse(const FloatRect&);

    void fillRect(const FloatRect&, const WebGraphicsContextState&);
    void fillRect(const FloatRect&, const FloatColor&, const WebGraphicsContextState&);
    void fillRect(const FloatRect&, Gradient*, const WebGraphicsContextState&);
    void fillRect(const FloatRect&, const FloatColor&, Operator, const WebGraphicsContextState&);
    void fillRoundedRect(const FloatRoundedRect&, const FloatColor&, const WebGraphicsContextState&);
    void fillRectWithRoundedHole(const FloatRect&, const FloatRoundedRect& roundedHoleRect, const FloatColor&, const WebGraphicsContextState&);

    void clearRect(const FloatRect&);

    void strokeRect(const FloatRect&, float lineWidth, const WebGraphicsContextState&);

    void drawSurfaceToContext(Image*, const FloatRect& destRect, const FloatRect& originalSrcRect, const WebGraphicsContextState& state);

    void drawPattern(Image*, const FloatRect& tileRect, const AffineTransform&, const FloatPoint& phase, Operator, const FloatRect& destRect, const WebGraphicsContextState&);

    void clip(const IntRect&);
    void clip(const FloatRect&);
    void clipRoundedRect(const FloatRoundedRect&);

    void clipOut(const FloatRect&);
    void clipOutRoundedRect(const FloatRoundedRect&);
    void clipPath(Path*, const WebGraphicsContextState&);
    void clipToImageBuffer(ImageBuffer*, const FloatRect&);

    IntRect clipBounds() const;

    void drawGlyphs(Font*, const Glyph*, int numGlyphs, const WebGraphicsContextState&);

    enum RoundingMode {
        RoundAllSides,
        RoundOriginAndDimensions
    };
    FloatRect roundToDevicePixels(const FloatRect&, RoundingMode = RoundAllSides);

    enum DocumentMarkerLineStyle {
        DocumentMarkerSpellingLineStyle,
        DocumentMarkerGrammarLineStyle,
        DocumentMarkerAutocorrectionReplacementLineStyle,
        DocumentMarkerDictationAlternativesLineStyle
    };
    void drawLineForDocumentMarker(const FloatPoint& origin, float width, DocumentMarkerLineStyle, int misspellingLineThickness, const WebGraphicsContextState&);

    void beginTransparencyLayer(float opacity, const WebGraphicsContextState&);
    void endTransparencyLayer();
    bool isInTransparencyLayer() const;

    void drawFocusRing(const std::vector<FloatRect>&, float width, float offset, const FloatColor&, const WebGraphicsContextState&);
    void drawFocusRing(Path*, float width, float offset, const FloatColor&, const WebGraphicsContextState&);

    void setLineCap(LineCap);
    void setLineJoin(LineJoin);
    void setMiterLimit(float);
    void setLineDash(const DashArray&, float dashOffset);

    void canvasClip(Path*, const WebGraphicsContextState&);
    void clipOut(Path*);

    void scale(const FloatSize&);
    void rotate(float angleInRadians);
    void translate(const FloatSize& size);
    void translate(float x, float y);

    void concatCTM(const AffineTransform&);
    void setCTM(const AffineTransform&);

    AffineTransform getCTM() const;

    void concat3DTransform(const TransformationMatrix&);
    void set3DTransform(const TransformationMatrix&);
    TransformationMatrix get3DTransform() const;

    void popClip();

    void flush();

private:
    WebGraphicsContextPrivate* m_private;
};

} // namespace WebGraphics

#endif // WebGraphicsContext_h
