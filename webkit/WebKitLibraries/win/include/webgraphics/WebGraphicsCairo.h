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

#ifndef WebGraphicsCairo_h
#define WebGraphicsCairo_h

#include <array>
#include <vector>

#include <cairo.h>

namespace WebGraphics {

using GraphicsSurface = cairo_surface_t;

using Font = cairo_scaled_font_t;
using Glyph = cairo_glyph_t;
using Gradient = cairo_pattern_t;
using Image = cairo_surface_t;
using ImageBuffer = cairo_surface_t;
using Path = cairo_t;
using Pattern = cairo_pattern_t;

using FillRule = cairo_fill_rule_t;
using Operator = cairo_operator_t;
using LineCap = cairo_line_cap_t;
using LineJoin = cairo_line_join_t;

using FloatPoint = std::array<float, 2>;
using FloatSize = std::array<float, 2>;
using IntPoint = std::array<int, 2>;
using IntSize = std::array<int, 2>;

using FloatRect = std::pair<FloatPoint, FloatSize>;
using FloatRoundedRect = std::pair<FloatRect, std::array<FloatSize, 4>>;
using IntRect = std::pair<IntPoint, IntSize>;

using FloatColor = std::array<float, 4>;
using AffineTransform = std::array<double, 6>;
using TransformationMatrix = std::array<std::array<double, 4>, 4>;

using DashArray = std::vector<float>;

} // namespace WebGraphics

#endif // WebGraphicsCairo_h
