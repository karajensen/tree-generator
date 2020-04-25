////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - treeGeneratorGUI.h
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "common.h"

/**
* The Window command will create a gui interface in Maya. 
* This gui communicates with the Command Generation class
*/
class TreeGeneratorGUI : public MPxCommand
{
public: 

    /**
    * @return whether or not this plugin is able to be undone/redone
    */
    virtual bool isUndoable() const override;

    /**
    * @return a new void* to an instance of the plugin
    */
    static void* creator();

    /**
    * The main entry pointfor the plugin
    * @param args The arguments (if any) passed into the plugin
    * @return whether the plugin succeeded or failed
    */
    virtual MStatus doIt(const MArgList& args);
};