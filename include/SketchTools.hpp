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

namespace SketchTools
{
    static float    CANVAS_STAGE_RATIO_X,
                    CANVAS_STAGE_RATIO_Y;
    
    
    double convertPixelsTOmmX(int _pixelDist);
    double convertPixelsTOmmY(int _pixelDist);
    
    enum Tool { LINE_TOOL, PENCIL_TOOL, CIRCLE_TOOL };
    
    typedef std::shared_ptr<class DragLine>		DragLineRef;
    
    class DragLine
    {
    
    public:
    
        static DragLineRef create(const ci::ivec2 &_mStart)
        {
            return DragLineRef( new DragLine(_mStart) );
        }
        
        DragLine();
        DragLine(const ci::ivec2 &_mStart);
        
        ~DragLine();
        
        
        void display();
        void update(ci::ivec2 _currentPos);
        void setEndPoint(ci::ivec2 _endPoint);
        float getLength();
        ci::vec2 getStartPos();
         ci::vec2 getEndPos();

        
    private:

        ci::ivec2           mStart,
                            mEnd;
    
        float               mLength;
    
    };
    
    //*********************************************************************************************************************
    
    typedef std::shared_ptr <class PencilLine>  PencilLineRef;
    
    class PencilLine {
    public:
        
        static PencilLineRef create(const ci::ivec2 &_mStart)
        {
            return PencilLineRef( new PencilLine(_mStart) );
        }
        
        PencilLine();
        PencilLine(const ci::ivec2 &_mStart);
        
        ~PencilLine();
        
        void display();
        void update(ci::ivec2 _currentPos);
        void setEndPoint(ci::ivec2 _endPoint);
        float getLength();
        ci::vec2 getStartPos();
        ci::vec2 getEndPos();
      //  int getNumSegments();
        std::vector<ci::ivec2>      getPoints();
        
        //void createSegments(int _stepModeValue, double _mVel, double _stepTravelDist);
        
    private:
        
        
        
        ci::ivec2                   mStart,
                                    mEnd,
                                    mPrevPoint,
                                    mCurrentPoint;
        
        float                       mLength;
        
        std::vector<ci::ivec2>      mPoints;
        
        
    private:
        
    };
    
     //*********************************************************************************************************************
    
    typedef std::shared_ptr <class Circle>  CircleRef;
    
    class Circle
    {
        
    public:
        
        static CircleRef create(const ci::ivec2 &_mCenter)
        {
            return CircleRef( new Circle(_mCenter) );
        }
        
        Circle();
        Circle(const ci::ivec2 &_mCenter);
        ~Circle();
        
        void display();
        void update(const ci::ivec2 &_tempRadiusPoint);
        void setCenter(const ci::ivec2 &_tempCenter);
        void setRadius(const ci::ivec2 &_radiusPoint);
        
        ci::vec2 getCentrePos();
        std::vector<ci::ivec2>      getPoints();
        
    private:
        
        double mRadius;
        
        int numSegments;
        
        bool isSet;
        
        ci::ivec2 mCenter, mRadiusPoint;
        
        std::vector<ci::ivec2>      mPoints;
        

    };
    
    
     //*********************************************************************************************************************
    
    //THIS METHOD SETS THE RATIO FOR THE SCREEN VS. MACHINE STAGING AREA (IE DRAWING BED)
    void setRatio(float _canvasWidth, float _stageWidth, float _canvasHeight, float _stageHeight);
    
    //THIS METHOD INITIALISES THE TEMPORARY DRAWING FEATURE ON MOUSEDOWN.
    void createTemporary(Tool mTool, ci::ivec2 _tempBegin);
    
    //THIS METHOD UPDATES THE TEMPORARY FEATURE'S ENDPOINT, WHICH ALLOWS FOR DRAWING THE FEATURE DURING MOUSE DRAGGED.
    void updateTemporary(Tool mTool, ci::ivec2 _currentLocation);
    
    //this method sets the endpoint of the current temporary feature, pushes it to the relevant features container, and creates a command pair (all on mouse up);
    void setTemporaryEndpoint(Tool mTool, ci::ivec2 _tempEnd, int _stepVal, double _vel, double _stepDist );
    
    //this method displays all the shapes / lines to the screen
    void displayCanvas();
    
    static Tool mTool;
    
}