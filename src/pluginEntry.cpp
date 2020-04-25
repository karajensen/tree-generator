////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - pluginEntry.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "common.h"
#include "randomGenerator.h"
#include "treeGenerator.h"
#include "treeGeneratorGUI.h"

#include <maya/MFnPlugin.h> // only include once per project

namespace
{
    const MString GENERATE_COMMAND("GenerateTree");
    const MString GUI_COMMAND("TreeGenerator");
}

MStatus initializePlugin(MObject obj)
{
    MFnPlugin pluginFn(obj, "Kara Jensen", "1.0");

    MStatus success = pluginFn.registerCommand(GENERATE_COMMAND, 
        TreeGenerator::creator, TreeGenerator::newSyntax);

    if(!success)
    { 
        success.perror("Register of " + GENERATE_COMMAND + " failed"); 
        return success;
    }

    success = pluginFn.registerCommand(GUI_COMMAND, 
        TreeGeneratorGUI::creator);

    if(!success)
    { 
        success.perror("Register of " + GUI_COMMAND + " failed"); 
        return success;
    }

    Random::Initialise();

    return success;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin pluginFn(obj);

    MStatus success = pluginFn.deregisterCommand(GENERATE_COMMAND);
    if(!success)
    { 
        success.perror("Deregister of " + GENERATE_COMMAND + " failed"); 
        return success;
    }

    success = pluginFn.deregisterCommand(GUI_COMMAND);
    if(!success)
    { 
        success.perror("Deregister of " + GUI_COMMAND + " failed"); 
        return success;
    }

    return success;
}
