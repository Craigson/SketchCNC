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
#include "cinder/Log.h"
#include "cinder/Serial.h"
#include <sstream>

//create the EiBotBoard object using a shared pointer for automatic memory management
typedef std::shared_ptr<class EiBotBoard>		EiBotBoardRef;

class EiBotBoard
{
public:
    
    static EiBotBoardRef create()
    {
        return EiBotBoardRef(new EiBotBoard());
    }
    
    EiBotBoard();
    ~EiBotBoard();
    
    void init();
    void update();
    void sendCommand(std::string _command);
    void flushBuffer();
    int getStepModeValue();
    
    friend class PlotBot;
    
protected:
    
    enum StepMode { SIXTEENTH, EIGHTH, QUARTER, HALF, FULL };  //ENUMERATOR FOR RECORDING / SETTING THE STEPPER MOTOR STEP MODES
    
    ci::SerialRef                   mSerial;        //serial device for communicating with the board
    std::string                     mPortName;      //variable to store the name of the computer's serial port
    
    void setMicroSteps(StepMode _setting);
    StepMode                        mStepMode;
    
private:

    
};
