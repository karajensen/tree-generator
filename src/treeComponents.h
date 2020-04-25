////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - treeComponents.h
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "common.h"
#include "vector3.h"
#include "matrix.h"

#include <string>
#include <deque>

/**
* Holds Shading data for the tree/leaves
*/
struct ShadingData
{
    double lightcolorR;     ///< Red component of the light color for shading 
    double lightcolorB;     ///< Blue component of the light color for shading
    double lightcolorG;     ///< Green component of the light color for shading
    double darkcolorR;      ///< Red component of the dark color for shading
    double darkcolorB;      ///< Blue component of the dark color for shading
    double darkcolorG;      ///< Green component of the dark color for shading
    double bumpAmount;      ///< Bump depth for tree shading
    double uvBleedSpace;    ///< Space allowed between UV points and edge
    bool createTreeShader;  ///< Whether to create a shader for the tree
    bool createLeafShader;  ///< Whether to create a shader for the leaves
    bool createBump;        ///< Whether to use bump mapping with the tree shader

    /**
    * Constructor
    * @param lightRGB The light color components for shading
    * @param darkRGB The dark color components for shading
    * @param usetreeShader Whether to create a tree shader or not
    * @param useleafshader Whether to create a shader for the leaves
    * @param bump Whether to use bump mapping with the tree shader
    * @param bleedAmount Space allowed between UV points and edge
    */
    ShadingData(double lightR, double lightG, double lightB, double darkR, 
        double darkG, double darkB, bool usetreeShader, bool useleafshader, 
        bool bump, double bump_amount, double bleedAmount) :
            lightcolorR(lightR), 
            lightcolorG(lightG),   
            lightcolorB(lightB),  
            darkcolorR(darkR),
            darkcolorG(darkG),
            darkcolorB(darkB),
            bumpAmount(bump_amount),
            uvBleedSpace(bleedAmount),
            createLeafShader(useleafshader),
            createTreeShader(usetreeShader),
            createBump(bump)
    {
    }
};

/**
* Holds rule data for designing the overall tree
*/
struct TreeData
{
    double initialRadius;              ///< The starting radius for the tree
    double branchRadiusDecrease;       ///< The amount to decrease the radius
    double minimumRadius;              ///< The minimum allowed radius
    unsigned branchDeathProbability;   ///< The probability from 0-100% of the branch dying
    std::string rule;                  ///< The rule string the tree abides by
    MString treename;                  ///< The name of the tree
    MString treeshadername;            ///< The name of the tree's shader
    MObject tree;                      ///< Tree Maya object

    /**
    * Constructor
    * @param radius The starting radius for the tree
    * @param radiusDecrease The amount to decrease the radius
    * @param minRadius The minimum allowed radius
    * @param deathProbability The probability from 0-100% of the branch dying
    */
    TreeData(double radius, double radiusDecrease, double minRadius, unsigned deathProbability) :
        initialRadius(radius),
        branchRadiusDecrease(radiusDecrease),
        minimumRadius(minRadius),
        branchDeathProbability(deathProbability)
    {
    }
};

/**
* Holds rule data for an individual trunk/branch
*/
struct BranchData
{ 
    double forward;             ///< The amount to move forward for the branch
    double forwardAngle;        ///< The angle to move forward at in degrees
    double forwardVariance;     ///< The amount to vary the forward movement
    double angle;               ///< The amount to rotate when creating the branch in degrees
    double angleVariance;       ///< The amount to vary the angle of the branch during creation
    double radiusDecrease;      ///< The amount to decrease the branch radius

    /**
    * Constructor
    * @param forwardAmount The amount to move forward for the branch
    * @param forward_angle The angle to move forward at in degrees
    * @param forward_variance The amount to vary the forward movement
    * @param branchAngle The amount to rotate when creating the branch in degrees
    * @param branchAngleVariance The amount to vary the angle of the branch during creation
    * @param radius_decrease The amount to decrease the branch radius
    */
    BranchData(double forwardAmount, double forward_angle, double forward_variance, 
        double branchAngle, double branchAngleVariance, double radius_decrease) :
            forward(forwardAmount),
            forwardAngle(forward_angle),
            forwardVariance(forward_variance),
            angle(branchAngle),
            angleVariance(branchAngleVariance),
            radiusDecrease(radius_decrease)
    {
    }
};

/**
* Holds data for a mesh of a branch
*/
struct MeshData
{
    int maxLayers;                      ///< Number of mesh layers for the branch
    bool preview;                       ///< Whether or not this is a preview tree   
    bool capEnds;                       ///< Whether to Fill in tips of tree with polygons
    bool createAsCurves;                ///< Whether the tree is created via curves or mesh
    bool randomize;                     ///< Whether or not to randomize the tree
    unsigned int trunkfaces;            ///< Number of faces around the trunk
    unsigned int branchfaces;           ///< Number of faces around branches
    unsigned int faceDecrease;          ///< Number of faces to reduce per branch layer

    /**
    * Constructor
    * @param numTrunkFaces Number of faces around the trunk
    * @param numBranchFaces Number of faces around branches
    * @param numFaceDecrease Number of faces to reduce per branch layer
    * @param useCurves Whether the tree is created via curves or mesh
    * @param capBranchEnds Whether to Fill in tips of tree with polygons
    * @param randomizeTree Number of faces around the trunk
    * @param previewTree Whether or not this is a preview tree   
    */
    MeshData(unsigned numTrunkFaces, unsigned numBranchFaces, unsigned numFaceDecrease,
        bool useCurves, bool capBranchEnds, bool randomizeTree, bool previewTree) :
            maxLayers(0),
            preview(previewTree),
            capEnds(capBranchEnds),
            createAsCurves(useCurves),
            randomize(randomizeTree),
            trunkfaces(numTrunkFaces),
            branchfaces(numBranchFaces),
            faceDecrease(numFaceDecrease)
    {
    }
};

/**
* Holds data for a mesh of a leaf
*/
struct LeafData
{
    MString leafshadername;     ///< Name of the shader
    unsigned leafLayer;         ///< Current layer that is being leafed
    bool treeHasLeaves;         ///< Whether or not the tree has leaves
    double width;               ///< Width of the leaf mesh
    double height;              ///< Height of the leaf mesh
    double widthVariance;       ///< Amount to vary the width of the leaf
    double heightVariance;      ///< Amount to vary the height of the leaf
    double bendAmount;          ///< Amount to bend the leaf
    MString file;               ///< Filename for the leaf texture

    /**
    * Constructor
    * @param leafTree Whether or not the tree has leaves
    * @param leafWidth Width of the leaf mesh
    * @param leafHeight Height of the leaf mesh
    * @param leafWidthVariance Amount to vary the width of the leaf
    * @param leafHeightVariance Amount to vary the height of the leaf
    * @param bend Amount to bend the leaf
    * @param layerNumber Current layer that is being leafed
    */
    LeafData(bool leafTree, double leafWidth, double leafHeight, double leafWidthVariance, 
        double leafHeightVariance, double bend, unsigned layerNumber) :
            leafLayer(layerNumber),
            treeHasLeaves(leafTree),
            width(leafWidth),
            height(leafHeight),
            widthVariance(leafWidthVariance),
            heightVariance(leafHeightVariance),
            bendAmount(bend)
    {
    }
};

/**
* Holds data for a section of a branch
*/
struct Section
{
    Float3 position;    ///< Position of the section
    float radius;       ///< Radius of the section

    /**
    * Constructor
    */
    Section() :
        radius(0.0f)
    {
    }

    /**
    * Constructor
    * @param pos The position of the section
    * @param rad The radius of the section
    */
    Section(const Float3& pos, float rad) :
        position(pos),
        radius(rad)
    { 
    }

    /**
    * Constructor
    * @param x/y/z The position of the section
    * @param rad The radius of the section
    */
    Section(float x, float y, float z, float rad) :
        position(x, y, z),
        radius(rad)
    { 
    }
};

/**
* Holds data for a branch
*/
struct Branch
{
    Matrix rotationMat;       ///< Branch rotation matrix
    Matrix scaleMat;          ///< Branch scale matrix
    int parentIndex;          ///< Index of the parent to this branch
    int sectionIndex;         ///< Index of the branch
    int layer;                ///< Layer that the branch exists on
    int vertNumber;           ///< Number of vertices of this branch
    MObject mesh;             ///< The maya mesh for the branch
    std::deque<Section> sections;  ///< The number of sections for this branch
    std::deque<int> children;      ///< A container of children extending from this branch

    /**
    * Constructor
    */
    Branch() :
        parentIndex(-1),
        sectionIndex(-1),
        layer(0),
        vertNumber(0)
    { 
    }
};

/**
* Holds disk vertex information of a branch
*/
struct Disk
{
    std::deque<Float3> points; ///< vertices of a branch disk
};

/**
* Holds data for a leaf
*/
struct Leaf
{
    int layer;              ///< Layer the leaf exists on
    MObject mesh;           ///< Mesh for the leaf
    Float3 position;        ///< Position of the leaf mesh
    Float3 sectionAxis;     ///< Axis for the branch section that leaf lives on
    float sectionRadius;    ///< Radius for the branch section that leaf lives on

    /**
    * Constructor
    * @param pos Position of the leaf mesh
    * @param axis Axis for the branch section that leaf lives on
    * @param layerIndex Layer the leaf exists on
    * @param radius Radius for the branch section that leaf lives on
    */
    Leaf(const Float3& pos, const Float3& axis, int layerIndex, float radius) :
        layer(layerIndex),
        position(pos),
        sectionAxis(axis),
        sectionRadius(radius)
    {
    }
};

/**
* Turtle object that navigates a rule string
*/
struct Turtle
{
    Matrix world;       ///< Turtle world matrix
    double radius;      ///< Current radius of the tree section generating
    int branchIndex;    ///< Current index of the branch generating
    int sectionIndex;   ///< Current index of the branch section generation
    int branchParent;   ///< Index of the parent to the branch generating
    int layerIndex;     ///< Current index of the layer generating
    bool branchEnded;   ///< Whether branch has ended or not

    /**
    * Constructor
    */
    Turtle() :
        radius(0.0f),
        branchIndex(-1),
        sectionIndex(-1),
        branchParent(0),
        layerIndex(0),
        branchEnded(false)
    {
    }
};

/**
* A layer of the tree 
*/
struct Layer
{
    MObject layer;      ///< Layer Maya object
    MObject branches;   ///< Branches Maya object
    MObject leaves;     ///< Leaves Maya object
};
