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

#include "EiBotBoard.hpp"

/************************************************************************
 *
 *                      C O N S T R U C T O R
 *
 ************************************************************************/

EiBotBoard::EiBotBoard():
mSerial(nullptr),
mPortName("null"),
mStepMode(SIXTEENTH)
{}




/************************************************************************
 *
 *                      D E S T R U C T O R
 *
 ************************************************************************/

EiBotBoard::~EiBotBoard(){}




/************************************************************************
 *
 *                          I N I T
 *
 ************************************************************************/

void EiBotBoard::init()
{

    for( const auto &dev : ci::Serial::getDevices() )
    {
        std::cout << "Device: " << dev.getName() << std::endl;
        if (dev.getName().compare("cu.usbmodem1411") == 0) mPortName = dev.getName();
        else if (dev.getName().compare("cu.usbmodem1451") == 0) mPortName = dev.getName();
    }
    
    std::cout << "Board is connected to port: " << mPortName << std::endl;
    
    try {
        ci::Serial::Device dev = ci::Serial::findDeviceByNameContains( mPortName );
        mSerial = ci::Serial::create( dev, 9600 );
        std::cout << std::endl;
        std::cout << "Serial Port Initialization Successful" << std::endl;
    }
    catch( ci::SerialExc &exc ) {
        CI_LOG_EXCEPTION( "coult not initialize the serial device", exc );
        exit( -1 );
    }
    
    mSerial->flush();
    
}



/************************************************************************
 *
 *                          S E N D  C O M M A N D
 *
 ************************************************************************/

void EiBotBoard::sendCommand(std::string _command)
{
    std::cout << "sending command: " << _command << std::endl;
    mSerial->writeString(_command);
}




/************************************************************************
 *
 *                          F L U S H  B U F F E R
 *
 ************************************************************************/

void EiBotBoard::flushBuffer()
{
    mSerial->flush();
}


/************************************************************************
 *
 *                    G E T  S T E P  M O D E  V A L U E
 *
 ************************************************************************/

int EiBotBoard::getStepModeValue()
{
    int stepValue;
    
    switch (mStepMode)
    {
        case SIXTEENTH:
            stepValue = 16;
            break;
            
        case EIGHTH:
            stepValue = 8;
            break;
            
        case QUARTER:
            stepValue = 4;
            break;
            
        case HALF:
            stepValue = 2;
            break;
            
        case FULL:
            stepValue = 1;
            break;
            
        default:
            stepValue = 16;     //THE EIBOTBOARD IS SET TO 1/16 STEP MODE BY DEFAULT
            break;
    }
    
    return stepValue;
}