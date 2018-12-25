////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - common.h
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//required for maya 32bit
#pragma warning(disable: 4005)
#ifdef WIN32
#define NT_PLUGIN
#endif

#define _USE_MATH_DEFINES
#include <math.h>

//required for maya includes
#include <iostream>

#include "maya/MPxCommand.h"
#include "maya/MFloatArray.h"
#include "maya/MIntArray.h"
#include "maya/MGlobal.h"
#include "maya/MArgDatabase.h"
#include "maya/MDagModifier.h"
#include "maya/MArgList.h"
#include "maya/MStatus.h"
#include "maya/MObject.h"
#include "maya/MSyntax.h"
#include "maya/MProgressWindow.h"
#include "maya/MFnMesh.h"
#include "maya/MDagPath.h"
#include "maya/MFloatPointArray.h"
#include "maya/MPointArray.h"
#include "maya/MFnTransform.h"
#include "maya/MFnNurbsCurve.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MSelectionList.h"

//required for maya includes
using namespace std;