=====================================================================================
TREE GENERATER PLUGIN
=====================================================================================
mail@KaraJensen.com
www.KaraJensen.com
https://github.com/karajensen/tree-generator.git
=====================================================================================

RELEASE REQUIREMENTS: Windows, Autodesk Maya 2011, 2012 or 2014
BUILD REQUIREMENTS: Windows, Visual Studio 2013, Autodesk Maya 2014

TIPS ON USING:
• Increasing the iterations will 'grow' the tree but also increase the poly count
• Each section has help annotations when the mouse is hovered over
• Use the prerule to generate your trunk

TIPS ON REDUCING POLY COUNT:
• Reduce the amount of faces used for a branch under Tree meshing
• Reduce the amount of faces used for leaves by setting leaf bending to 0
• Use a combination of 'G' and 'F' to give height rather than solely 'F'

HOW TO INSTALL:
• Open the dated folder that corresponds to your version of Maya
• Place "TreeGenerator.mll" and "TreeGeneratorGUI.mel" in Maya20XX\bin\plug-ins
• Inside Maya, go to Windows -> Settings/Preferences -> Plugin Manager
• If the plugin isn't showing up in the list, go to browse and select open it
• Once the plugin is installed, make sure "Loaded" is clicked
• Use 'GenerateTree' for a default tree 
• Use 'TreeGenerator' for the GUI in the command window

HOW TO DEBUG:
• Change the lib/include directory to your Maya path
  eg. C:\Program Files\Autodesk\Maya2014\
  
• Click Debug. Click okay for any popups that appear.
  In Maya, you will need to load the plugin in the plugin manager. 
  It's useful to write a MEL script that does this for you:
  loadPlugin "D:\\Projects\\TreeGenerator\\TreeGenerator\\TreeGenerator.mll;

• Use 'GenerateTree' for a default tree 
  Use 'TreeGenerator' for the GUI in the command window