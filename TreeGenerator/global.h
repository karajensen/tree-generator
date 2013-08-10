////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//required for maya 32bit
#pragma warning(disable: 4005)
#ifdef WIN32
#define NT_PLUGIN
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <string>
#include <deque>
#include <ctime>
#include "vector3.h"
#include "matrix.h"

#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MProgressWindow.h>
#include <maya/MFnMesh.h>
#include <maya/MDagPath.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MPointArray.h>
#include <maya/MDagModifier.h>
#include <maya/MFnTransform.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSelectionList.h>

using namespace std;

template<typename T> 
T DegToRad(T degrees)
{
    return static_cast<T>(M_PI/180.0)*degrees;
}

template<typename T> 
T RadToDeg(T radians)
{
    return static_cast<T>(180.0/M_PI)*radians;
}

template<typename T>
T ChangeRange(T value, T currentRangeInner,
    T currentRangeOuter, T newRangeInner, T newRangeOuter)
{
    return ((value-currentRangeInner)*((newRangeOuter-newRangeInner)
        /(currentRangeOuter-currentRangeInner)))+newRangeInner;
}