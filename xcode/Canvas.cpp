/*
 Copyright (c) 2016, Craig Pickard - All rights reserved.
 
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "Canvas.hpp"

Canvas::Canvas():
mSize(ci::ivec2(CANVAS_WIDTH, CANVAS_HEIGHT)),
mCanvasColour(ci::Color(0.7,0.7,0.7)),
mCanvas(ci::Rectf(ci::vec2(0), ci::vec2(CANVAS_WIDTH - 50,CANVAS_HEIGHT - 80)))
{}

Canvas::~Canvas(){}

void Canvas::addLine(SketchTools::DragLineRef _tempLine)
{
    mLines.push_back(_tempLine);
}

void Canvas::addPencilLine(SketchTools::PencilLineRef _tempPencilLine)
{
    mPencilLines.push_back(_tempPencilLine);
}

void Canvas::addCircle(SketchTools::CircleRef _tempCircle)
{
    mCircles.push_back(_tempCircle);
}

void Canvas::showPenPosition(const ci::vec2 &_penPos)
{
    ci::gl::color(ci::ColorAf(1.0,0.,0.,0.2));
    ci::gl::drawSolidEllipse(_penPos, 3.f, 3.f);
    ci::gl::drawLine(ci::vec2(0,_penPos.y), ci::vec2(CANVAS_WIDTH, _penPos.y));
    ci::gl::drawLine(ci::vec2(_penPos.x,0), ci::vec2(_penPos.x, CANVAS_HEIGHT));
}


void Canvas::render()
{
    ci::gl::color(mCanvasColour);
    ci::gl::drawSolidRect(mCanvas);
    if (mLines.size() > 0)          for (auto l : mLines) l->display();
    if (mPencilLines.size() > 0)    for (auto p : mPencilLines) p->display();
    if (mCircles.size() > 0)        for (auto c : mCircles) c->display();
    
}
