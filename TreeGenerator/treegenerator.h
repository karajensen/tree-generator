/****************************************************************
* Kara Jensen (mail@karajensen.com) 2012
* Window and Generation Plugin Code for the Tree Generator
* The Window command will create a gui interface in Maya. 
* This gui communicates with the Command Generation class
*****************************************************************/

#pragma once

#include "global.h"
#include "treegenerator_components.h"

class TreeGenerator : public MPxCommand
{
public: 

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
    * @param the arguments (if any) passed into the plugin
    * @return whether the plugin succeeded or failed
    */
    virtual MStatus doIt(const MArgList& args) override;

    /**
    * Initialises the arguments that can be passed into the plugin
    * @return the object containing the new flags
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
    * @param the number of iterations to generate the rule string
    * @param the rules to replace the rule character
    * @param the rule character array
    * @param the probability for each rule character
    * @return whether generation succeeded
    */
    bool CreateRuleString(int iterations, MString rules[], MString rulec[], unsigned rulep[]);

    /**
    * Builds the tree from the generated rule string using a turtle object
    * @param data for the tree trunk
    * @param data for the tree branches
    * @return whether the call succeeded
    */
    bool BuildTheTree(BranchData& BF, BranchData& TF);

    /**
    * Checks whether branch is alive or dead and removes any 
    * successive rules after the branch if it is dead
    * @param the index in the rule string of the current branch
    * @return if the branch is alive or not
    */
    bool BranchIsAlive(unsigned int& j);

    /**
    * Creates a new branch and changes branch values as necessary
    * @param the tutle navigating the rule string
    * @param the index for the trunk of the tree
    * @param the current data used for generating branches
    * @param the data used for generating trunks
    */
    void BuildNewBranch(Turtle& turtle, int TrunkIndex, BranchData* values, BranchData& BF);

    /**
    * Determines the forward position of the turtle
    * @param the turtle object
    * @param the position generated
    * @param how far forward to move
    * @param how much should the turtle rotate from the initial position
    * @param how much variation of the distance/rotation values
    */
    void DetermineForwardMovement(Turtle* turtle, FLOAT3& result, 
        double forward, double ang, double variation);
    
    /**
    * Create all the meshes of the tree
    */
    bool CreateMeshes();

    /**
    * Create an individual branch mesh
    * @param the branch object
    * @param the branch's parent
    * @param the vertex disc information for the dimensions of the mesh
    * @param the name of the mesh generated
    * @param the layer the mesh exists in
    */
    void CreateMesh(Branch* branch, Branch* parent, Disk* disk, 
        MString& meshname, MObject& layer);

    /**
    * Create all the leaves of tree
    */
    bool CreateLeaves();

    /**
    * Create an individual leaf
    * @param the leave object
    * @param the name of the leaf mesh
    * @param what layer the mesh lives in
    */
    void CreateLeaf(Leaf* leaf, MString& meshname, MObject& layer);

    /**
    * Create all the curves of the tree
    */
    bool CreateCurves();

    /**
    * Create an individual curve for the curve tree
    * @param the branch object
    * @param the name of the curve created
    * @param the layer the curve exists in
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
    * Turn on node construction history for Maya
    */
    void TurnOnHistory(int turnon);

    /**
    * Turn off node construction history for Maya
    */
    void TurnOffHistory();

    /**
    * Begin a progress window
    * @param the amount of overall steps for the plugin
    */
    void StartProgressWindow(int stepNumber);

    /**
    * Sets the descriptive text of the progress window
    * @param the text to display
    */
    void DescribeProgressWindow(char* Description);

    /**
    * Advance the progress window by an amount
    * @param the amount of steps to increase progress by
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
    * @param the arguement data
    * @param the prerule, postrule and starting rule
    * @param the number of iterations
    * @param array user given rules
    * @param the branch and tree datas
    */
    void GetFlagArguments(const MArgDatabase& argData, MString prerule,
        MString postrule, MString start, unsigned int& iterations, 
        MString* rules, MString* rulec, unsigned int* rulep, 
        BranchData& branch, BranchData& trunk);

    static MDagModifier sm_dagMod;         ///< Maya DAG node modifier object
    static int sm_treeNumber;              ///< Number of trees generated in the current Maya session
    static unsigned int sm_treeSeed;       ///< Randomly generated seed
    static const int sm_ruleNumber = 10;   ///< Maximum amount of rules allowed

    unsigned int m_progressIncreaseAmount; ///< How much each step can increase the progress bar overall by
    unsigned int m_progressStepAmount;     ///< Minimum amount at one time the progress bar can increase by

    MeshStruct m_meshdata;      ///< Holds data for a mesh of a branch
    LeafData m_leafdata;      ///< Holds data for a mesh of a leaf
    ShadingData m_fxdata;     ///< Holds Shading data for the tree/leaves
    TreeData m_treedata;      ///< Holds rule data for the overall tree
    deque<Layer> m_layers;      ///< All layers of the tree
    deque<Branch> m_branches;   ///< All branches of the tree including the trunk
    deque<Leaf> m_leaves;       ///< All leaves of the tree
    deque<FLOAT3> m_leafverts;  ///< Vertices for a leaf
    MIntArray m_leafpolycounts; ///< Poly count for a leaf
    MIntArray m_leafindices;    ///< Indices for a leaf
    MFloatArray m_leafU;        ///< U value for UVs for a leaf
    MFloatArray m_leafV;        ///< V value for UVs for a leaf
    MIntArray m_leafUVids;      ///< UV IDS for UVs for a leaf
};

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
    * @param the arguments (if any) passed into the plugin
    * @return whether the plugin succeeded or failed
    */
    virtual MStatus doIt(const MArgList& args);
};