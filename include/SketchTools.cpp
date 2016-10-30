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

#include "SketchTools.hpp"

namespace SketchTools
{
    
    void setRatio(float _canvasWidth, float _stageWidth, float _canvasHeight, float _stageHeight)
    {
        CANVAS_STAGE_RATIO_X = _canvasWidth / _stageWidth;
        CANVAS_STAGE_RATIO_Y = _canvasHeight / _stageHeight;
        
        std::cout << "Canvas/Stage ratio X: " << CANVAS_STAGE_RATIO_X << " , " << "Canvas/Stage ration Y: " << CANVAS_STAGE_RATIO_Y << std::endl << std::endl;
    }
    
    double convertPixelsTOmmX(int _pixelDist)
    {
        double dist = _pixelDist / CANVAS_STAGE_RATIO_X;
        return dist;
    }
    
    double convertPixelsTOmmY(int _pixelDist)
    {
        double dist = _pixelDist / CANVAS_STAGE_RATIO_Y;
        return dist;
    }

/************************************************************************
 *
 *                          D R A G  L I N E
 *
 ************************************************************************/

    DragLine::DragLine(const ci::ivec2 &_mStart):
    mStart(_mStart),
    mEnd(_mStart),
    mLength(0)
    {}

    DragLine::DragLine(){}
    
    DragLine::~DragLine(){}
    
    void DragLine::display()
    {
        ci::gl::color(ci::Color::black());
        ci::gl::drawLine(mStart, mEnd);
    }
    
    void DragLine::update(ci::ivec2 _currentPos )
    {
        mEnd = _currentPos;
        display();
        
    }
    
    void DragLine::setEndPoint(ci::ivec2 _endPoint)
    {
        mEnd = _endPoint;
        ci::vec2 dir = mEnd - mStart;
        mLength = length(dir);
    }
    
    float DragLine::getLength()
    {
        return mLength;
    }
    
    ci::vec2 DragLine::getStartPos()
    {
        return mStart;
    }
    
    ci::vec2 DragLine::getEndPos()
    {
        return mEnd;
    }





/************************************************************************
 *
 *                          P E N C I L   L I N E
 *
 ************************************************************************/

    PencilLine::PencilLine(){}

    PencilLine::PencilLine(const ci::ivec2 &_mStart):
    mStart( _mStart),
    mCurrentPoint(_mStart),
    mPrevPoint(_mStart)
    {
        mPoints.push_back(_mStart);
    }

    PencilLine::~PencilLine(){}

    void PencilLine::display()
    {
        if (mPoints.size() > 1)
        {
            
            
            for (int i = 0; i < mPoints.size() - 1; i++)
            {
                ci::gl::color(ci::Color::black());
                ci::gl::drawLine(mPoints[i], mPoints[i+1]);
            }
            
        }
        
    }

    void PencilLine::update(ci::ivec2 _currentPos)
    {
        //mCurrentPoint = _currentPos;
        
        //ci::vec2 dir = mPrevPoint - mCurrentPoint;
        //float len = length(dir);
        
        //if (len > 2)
        
        mPoints.push_back(_currentPos), std::cout << "adding point: " << _currentPos << std::endl;
        
        //mPrevPoint = mCurrentPoint;
    }

    void PencilLine::setEndPoint(ci::ivec2 _endPoint)
    {
        mPoints.push_back(_endPoint);
        mEnd = _endPoint;
        
        ci::vec2 dir = mEnd - mStart;
        mLength = length(dir);
    }

    float PencilLine::getLength()
    {
        return mLength;
    }

//    int PencilLine::getNumSegments()
//    {
//       // return mSegmentCommands.size();
//    }
    
    ci::vec2 PencilLine::getStartPos()
    {
        return mStart;
    }

    ci::vec2 PencilLine::getEndPos()
    {
        return mEnd;
    }
    
    std::vector<ci::ivec2> PencilLine::getPoints()
    {
        return mPoints;
    }
    
    
    
    /************************************************************************
     *
     *                          C I R C L E
     *
     ************************************************************************/
    
    Circle::Circle(){}
    
    Circle::Circle(const ci::ivec2 &_mCenter):
    mCenter(_mCenter),
    mRadiusPoint(_mCenter),
    numSegments(36),
    isSet(false)
    {
        for (int i = 0; i < numSegments; i++) mPoints.push_back(ci::vec2(0)); //fill the container with empty values
    }
    
    Circle::~Circle(){}
    
    void Circle::setCenter(const ci::ivec2 &_tempCenter)
    {
        mCenter = _tempCenter;
    }

    void Circle::update(const ci::ivec2 &_tempRadiusPoint)
    {
        mRadiusPoint = _tempRadiusPoint;
        
        ci::vec2 dir = mRadiusPoint - mCenter;
        double dist = length(dir);
        mRadius = dist;
        
        double dTheta = ( 2 * M_PI ) / numSegments;
        double theta = 0;
        
        for (int i = 0; i < numSegments; i++)
        {
            mPoints[i] = ci::ivec2(mCenter.x + mRadius * cos(theta), mCenter.y + mRadius * sin(theta));
            theta += dTheta;
        }
    }
    
    void Circle::setRadius(const ci::ivec2 &_tempRadiusPoint)
    {
        mRadiusPoint = _tempRadiusPoint;
        
        ci::vec2 dir = mRadiusPoint - mCenter;
        double dist = length(dir);
        mRadius = dist;
        
        double dTheta = ( 2 * M_PI ) / numSegments;
        double theta = 0;
        
        for (int i = 0; i < numSegments; i++)
        {
            mPoints[i] = ci::ivec2(mCenter.x + mRadius * cos(theta), mCenter.y + mRadius * sin(theta));
            theta += dTheta;
        }
        
        isSet = true;
    }
    
    
//    void Circle::setRadius(const ci::ivec2 &_radiusPoint)
//    {
//        mRadiusPoint = _radiusPoint;
//    }
    
    void Circle::display()
    {
       // std::cout << "circle should be displaying" << std::endl;
        ci::gl::color(ci::Color::black());
        
       // std::cout << mCenter << " " << mRadiusPoint << std::endl;
        
        if (!isSet) ci::gl::drawLine(mCenter, mRadiusPoint); //only draw the line while the circle is being dragged/drawn
        
        ci::ivec2 currentPoint, prevPoint;
        
        for (int i = 1 ; i < numSegments; i++)
        {
            currentPoint = mPoints[i];
            prevPoint = mPoints[i-1];
            ci::gl::drawLine(currentPoint, prevPoint);
        }
        
        ci::gl::drawLine(mPoints[mPoints.size() - 1], mPoints[0]); //connect the first and last points to close the circle
    }
    
    ci::vec2 Circle::getCentrePos()
    {
        return mCenter;
    }
    
    std::vector<ci::ivec2>      Circle::getPoints()
    {
        return mPoints;
    }
    
    
    

} //end of namespace