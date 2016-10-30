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

#pragma once

#include <stdio.h>
#include "SketchTools.hpp"

#define CANVAS_WIDTH 1155
#define CANVAS_HEIGHT 900
#define CANVAS_OFFSET_X 20
#define CANVAS_OFFSET_Y 20

typedef std::shared_ptr<class Canvas>       CanvasRef;

class Canvas {
public:
    static CanvasRef create()
    {
        return CanvasRef(new Canvas());
    }
    
    Canvas();
    ~Canvas();
    
    void render();
    void addLine(SketchTools::DragLineRef _tempLine);
    void addPencilLine(SketchTools::PencilLineRef _tempPencilLine);
    void addCircle(SketchTools::CircleRef _tempCircle);
    void showPenPosition(const ci::vec2 &_penPos);

    
protected:
    ci::ivec2   mSize, mPosition;
    ci::Color   mCanvasColour;
    
    //these containers are for storing and drawing lines to the canvas only - they have nothing to do with communicating with the board
    std::vector<SketchTools::DragLineRef>         mLines;
    std::vector<SketchTools::PencilLineRef>       mPencilLines;
    std::vector<SketchTools::CircleRef>           mCircles;
    
    ci::Rectf     mCanvas;
    
};
