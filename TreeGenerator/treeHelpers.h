////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - common.h
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

/**
* Converts degrees to radians
*/
template<typename T> T DegToRad(T degrees)
{
    return static_cast<T>(M_PI/180.0)*degrees;
}

/**
* Converts radians to degrees
*/
template<typename T> T RadToDeg(T radians)
{
    return static_cast<T>(180.0/M_PI)*radians;
}

/**
* Changes the range of a value
*/
template<typename T> T ChangeRange(T value, 
                                   T currentRangeInner, 
                                   T currentRangeOuter, 
                                   T newRangeInner, 
                                   T newRangeOuter)
{
    return ((value-currentRangeInner)*((newRangeOuter-newRangeInner)
        /(currentRangeOuter-currentRangeInner)))+newRangeInner;
}