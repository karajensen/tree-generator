////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - treeGenerator.h
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "common.h"
#include "treeComponents.h"

#include <memory>
#include <array>

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
    virtual bool isUndoable() const override;

    /**
    * @return a new void* to an instance of the plugin
    */
    static void* creator();

    /**
    * The main entry point for the plugin
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
    * Create the main rule string which determines the three shape
    * @return Whether generation succeeded
    */
    bool CreateRuleString();

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
    * @return if the branch is dead or not
    */
    bool TryKillBranch(unsigned int& index);

    /**
    * Creates a new branch and changes branch values as necessary
    * @param turtle The tutle navigating the rule string
    * @param trunkIndex The index for the trunk of the tree
    * @param trunk The data used for generating trunks
    * @param values The current data used for generating branches
    */
    void BuildNewBranch(Turtle& turtle, 
                        int trunkIndex, 
                        BranchData& trunk,
                        BranchData** values);

    /**
    * Determines the forward position of the turtle
    * @param turtle The turtle object
    * @param forward How far forward to move
    * @param angle How much should the turtle rotate from the initial position
    * @param variation How much variation of the distance/rotation values
    * @return The position generated
    */
    Float3 DetermineForwardMovement(const Turtle& turtle,
                                    double forward, 
                                    double angle, 
                                    double variation) const;
    
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
    void CreateMesh(Branch* branch, 
                    Branch* parent, 
                    Disk& disk, 
                    MString& meshname, 
                    MObject& layer);

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
    void CreateLeaf(Leaf& leaf, MString& meshname, MObject& layer);

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
    void CreateCurve(Branch& branch, MString& meshname, MObject& layer);

    /**
    * Create all the groups for the layers of the tree 
    */
    bool CreateTreeGroup();

    /**
    * Create all the shaders for the tree
    */
    bool CreateShaders();

    /**
    * Turn on node construction history for Maya if required
    * @param shouldTurnOn value of 1 will turn on history
    */
    void TurnOnHistory(int shouldTurnOn = 1);

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
    void DescribeProgressWindow(const char* description);

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
    * @param branch/trunk The branch and tree data to fill in
    */
    void GetFlagArguments(const MArgDatabase& argData, 
                          MString prerule,
                          MString postrule, 
                          MString start, 
                          BranchData& branch, 
                          BranchData& trunk);

    static int sm_treeNumber;                   ///< Number of trees generated in the current Maya session
    unsigned int m_progressIncrease = 0;        ///< How much each step can increase the progress bar overall by
    unsigned int m_progressStep = 0;            ///< Minimum amount at one time the progress bar can increase by
    unsigned int m_iterations = 4;              ///< The number of iterations of the rules to do
    std::unique_ptr<MDagModifier> m_dagMod;     ///< Maya DAG node modifier object
    MeshData m_meshdata;                        ///< Holds data for a mesh of a branch
    LeafData m_leafdata;                        ///< Holds data for a mesh of a leaf
    TreeData m_treedata;                        ///< Holds rule data for the overall tree
    ShadingData m_fxdata;                       ///< Holds Shading data for the tree/leaves
    std::deque<Layer> m_layers;                 ///< All layers of the tree
    std::deque<Branch> m_branches;              ///< All branches of the tree including the trunk
    std::deque<Leaf> m_leaves;                  ///< All leaves of the tree
    std::deque<Float3> m_leafVertices;          ///< Container of vertices for a leaf
    MIntArray m_leafPolycounts;                 ///< Poly count for a leaf
    MIntArray m_leafIndices;                    ///< Indices for a leaf
    MFloatArray m_leafU;                        ///< U value for UVs for a leaf
    MFloatArray m_leafV;                        ///< V value for UVs for a leaf
    MIntArray m_leafUVids;                      ///< UV IDS for UVs for a leaf

    static const int RULE_NUMBER = 10;                    ///< Maximum amount of rules allowed
    std::array<MString, RULE_NUMBER> m_ruleIDs;           ///< The rule characters
    std::array<MString, RULE_NUMBER> m_ruleStrings;       ///< The rules to replace the rule characters
    std::array<unsigned int, RULE_NUMBER> m_ruleChances;  ///< The probability for each rule character
};                                           