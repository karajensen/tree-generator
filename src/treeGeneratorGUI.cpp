////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - treeGeneratorGUI.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "treeGeneratorGUI.h"

#include <fstream>
#include <string>
#include <Windows.h>

MStatus TreeGeneratorGUI::doIt(const MArgList& args)
{
    // Get the absoulute path of this .dll
    char buffer[256];
    HMODULE handle = nullptr;

    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
        (LPCSTR)"TreeGenerator.mll", &handle);

    GetModuleFileNameA(handle, buffer, sizeof(buffer));

    // Convert the path to the gui .mel
    MString path(buffer);
    const int size = static_cast<int>(path.length()-std::string("TreeGenerator.mll").size());
    path = path.substring(0, size - 1);
    path += "TreeGeneratorGUI.mel";

    // Open the GUI .mel file
    std::ifstream gui(path.asChar());
    if(!gui.is_open())
    {
        MGlobal::executeCommand("error \"" + path + " could not open\"");
        return MStatus::kFailure;
    }

    // Execute the mel file
    std::string line;
    MString command;

    while(!gui.eof())
    {
        std::getline(gui, line);
        command += line.c_str();
    }

    MGlobal::executeCommand(command);
    return MStatus::kSuccess;
}

bool TreeGeneratorGUI::isUndoable() const
{ 
    return false; 
}

void* TreeGeneratorGUI::creator() 
{ 
    return new TreeGeneratorGUI();
}