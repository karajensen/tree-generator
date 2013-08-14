////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - treegenerator.h
////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "global.h"
#include "treegenerator_components.h"

/**
* Core command class for generating the trees. The GUI
* window will pass in arguments to customise this process
*/
class TreeGenerator : public MPxCommand
{
public: 

    /**
    * Constructor
    */
    TreeGenerator();

    /**
    * @return whether or not this plugin is able to be undone/redone
    */
    virtual bool isUndoable() const override { return false; }

    /**
    * @return a new void* to an instance of the plugin
    */
    static void* creator() { return new TreeGenerator; }

    /**
    * The main entry pointfor the plugin
    * @param args The arguments (if any) passed into the plugin
    * @return Whether the plugin succeeded or failed
    */
    virtual MStatus doIt(const MArgList& args) override;

    /**
    * Initialises the arguments that can be passed into the plugin
    * @return The object containing the new flags
    */
    static MSyntax newSyntax();

private:

    /**
    * Delete all currently constructed nodes 
    */
    void DeleteNodes();

    /**
    * Begin creating a mesh around the generated tree
    * @return whether the call succeeded
    */
    bool MeshTheTree();

    /**
    * Create the main rule string 
    * @param iterations The number of iterations to generate the rule string
    * @param rules The rules to replace the rule character
    * @param rulec The rule character array
    * @param rulep The probability for each rule character
    * @return Whether generation succeeded
    */
    bool CreateRuleString(int iterations, MString rules[], MString rulec[], unsigned rulep[]);

    /**
    * Builds the tree from the generated rule string using a turtle object
    * @param branch Data for the tree trunk
    * @param trunk Data for the tree branches
    * @return Whether the call succeeded
    */
    bool BuildTheTree(BranchData& branch, BranchData& trunk);

    /**
    * Checks whether branch is alive or dead and removes any 
    * successive rules after the branch if it is dead
    * @param index The index in the rule string of the current branch
    * @return if the branch is alive or not
    */
    bool BranchIsAlive(unsigned int& index);

    /**
    * Creates a new branch and changes branch values as necessary
    * @param turtle The tutle navigating the rule string
    * @param TrunkIndex The index for the trunk of the tree
    * @param values The current data used for generating branches
    * @param trunk The data used for generating trunks
    */
    void BuildNewBranch(Turtle& turtle, int TrunkIndex, BranchData* values, BranchData& trunk);

    /**
    * Determines the forward position of the turtle
    * @param turtle The turtle object
    * @param result The position generated
    * @param forward How far forward to move
    * @param angle How much should the turtle rotate from the initial position
    * @param variation How much variation of the distance/rotation values
    */
    void DetermineForwardMovement(Turtle* turtle, Float3& result, 
        double forward, double angle, double variation);
    
    /**
    * Create all the meshes of the tree
    * @param whether or not creation was successful
    */
    bool CreateMeshes();

    /**
    * Create an individual branch mesh
    * @param branch The branch object
    * @param parent The branch's parent
    * @param disk The vertex disc information for the dimensions of the mesh
    * @param meshname The name of the mesh generated
    * @param layer The layer the mesh exists in
    */
    void CreateMesh(Branch* branch, Branch* parent, Disk* disk, 
        MString& meshname, MObject& layer);

    /**
    * Create all the leaves of tree
    * @param whether or not creation was successful
    */
    bool CreateLeaves();

    /**
    * Create an individual leaf
    * @param leaf The leaf object to create
    * @param meshname The name of the leaf mesh
    * @param layer What layer the mesh lives in
    */
    void CreateLeaf(Leaf* leaf, MString& meshname, MObject& layer);

    /**
    * Create all the curves of the tree
    * @return Whether or not creation was successful
    */
    bool CreateCurves();

    /**
    * Create an individual curve for the curve tree
    * @param branch The branch object
    * @param meshname The name of the curve created
    * @param layer The layer the curve exists in
    */
    void CreateCurve(Branch* branch, MString& meshname, MObject& layer);

    /**
    * Create all the groups for the layers of the tree 
    */
    bool CreateTreeGroup();

    /**
    * Create all the shaders for the tree
    */
    bool CreateShaders();

    /**
    * Turn on/off node construction history for Maya
    * @param turnon whether to turn on/off history
    */
    void TurnOnHistory(int turnon);

    /**
    * Turn off node construction history for Maya
    */
    void TurnOffHistory();

    /**
    * Begin a progress window
    * @param stepNumber The amount of overall steps for the plugin
    */
    void StartProgressWindow(int stepNumber);

    /**
    * Sets the descriptive text of the progress window
    * @param description The text to display in the progress window
    */
    void DescribeProgressWindow(char* description);

    /**
    * Advance the progress window by an amount
    * @param amount The amount of steps to increase progress by
    */
    void AdvanceProgressWindow(int amount);

    /**
    * @return whether or not the plugin was cancelled mid operation
    */
    bool PluginIsCancelled();

    /**
    * Closes the progress window
    */
    void EndProgressWindow();

    /**
    * Get all flag arguments passed from the gui
    * @param argData the arguement data
    * @param prerule/postrule/start The rules governing the tree appearance
    * @param iterations the number of iterations of the rules
    * @param rules The rules to replace the rule character
    * @param rulec The rule character array
    * @param rulep The probability for each rule character
    * @param branch/trunk The branch and tree data to fill in
    */
    void GetFlagArguments(const MArgDatabase& argData, MString prerule,
        MString postrule, MString start, unsigned int& iterations, 
        MString* rules, MString* rulec, unsigned int* rulep, 
        BranchData& branch, BranchData& trunk);


    static MDagModifier sm_dagMod;         ///< Maya DAG node modifier object
    static int sm_treeNumber;              ///< Number of trees generated in the current Maya session
    static unsigned int sm_treeSeed;       ///< Randomly generated seed

    unsigned int m_progressIncreaseAmount; ///< How much each step can increase the progress bar overall by
    unsigned int m_progressStepAmount;     ///< Minimum amount at one time the progress bar can increase by

    MeshData m_meshdata;        ///< Holds data for a mesh of a branch
    LeafData m_leafdata;        ///< Holds data for a mesh of a leaf
    TreeData m_treedata;        ///< Holds rule data for the overall tree
    ShadingData m_fxdata;       ///< Holds Shading data for the tree/leaves
    deque<Layer> m_layers;      ///< All layers of the tree
    deque<Branch> m_branches;   ///< All branches of the tree including the trunk
    deque<Leaf> m_leaves;       ///< All leaves of the tree
    deque<Float3> m_leafverts;  ///< Vertices for a leaf
    MIntArray m_leafpolycounts; ///< Poly count for a leaf
    MIntArray m_leafindices;    ///< Indices for a leaf
    MFloatArray m_leafU;        ///< U value for UVs for a leaf
    MFloatArray m_leafV;        ///< V value for UVs for a leaf
    MIntArray m_leafUVids;      ///< UV IDS for UVs for a leaf
};

/**
* The Window command will create a gui interface in Maya. 
* This gui communicates with the Command Generation class
*/
class TreeGeneratorWindow : public MPxCommand
{
public: 

    /**
    * @return whether or not this plugin is able to be undone/redone
    */
    virtual bool isUndoable() const override { return false; }

    /**
    * @return a new void* to an instance of the plugin
    */
    static void* creator() { return new TreeGeneratorWindow; }

    /**
    * The main entry pointfor the plugin
    * @param args The arguments (if any) passed into the plugin
    * @return whether the plugin succeeded or failed
    */
    virtual MStatus doIt(const MArgList& args);
};