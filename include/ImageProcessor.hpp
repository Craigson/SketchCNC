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
#include "cinder/Capture.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "cinder/Utilities.h"
#include "cinder/Log.h"

typedef std::shared_ptr<class ImageProcessor>   ImageProcessorRef;

class ImageProcessor
{
public:
    
    static ImageProcessorRef create()
    {
        return ImageProcessorRef(new ImageProcessor());
    }
    
    
    ImageProcessor();
    ~ImageProcessor();
    
    void                    loadImage();
    void                    initCamera();
    void                    renderImage();
    void                    captureImage();
    void                    updateThreshold(int _updateValue);
    std::vector<ci::vec2>   getPixelLocations();
    void                    processImage();
    void                    drawPortrait();
    void                    update();
    void                    displayPixels();
    void                    enableCapture();
    
    ci::ivec2               getCameraSize();
    void                    showCapture(ci::ivec2 _loc);
    
    int*                    getThreshold();
    float*                  getScale();
    int*                    getPixelSize();
    void                    invertImage();
    
protected:
    ci::CaptureRef			mCapture;
    
    ci::Surface             mPixels;
    
    int                     getAverage(ci::Surface* _surface, ci::vec2 _ULvertex);
    
    int                     numX, numY, threshold, pixelSize, imageWidth, imageHeight;
    
    float                   mScale;
    
    ci::ivec2               mCanvasPos;
    
    //Surface8u           mPixels;
    
    std::vector<int>        mThresholdValues;
    std::vector<ci::ivec2>  mPixelPositions;
    
    ci::DataSourceRef       mImageData;
    
    ci::gl::TextureRef      mTexture;
    
    bool                    fileLoaded, capturingImage, showPixels, displayLoaded;
    
    
};