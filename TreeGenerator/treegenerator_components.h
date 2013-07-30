/****************************************************************
* Kara Jensen (KaraPeaceJensen@gmail.com) 2012
* Generation Plugin Structure Information
*****************************************************************/

#pragma once
#include "global.h"

/**
* Holds Shading data for the tree/leaves
*/
struct ShadingData
{
    double lightcolorR, lightcolorB, lightcolorG;
    double darkcolorR, darkcolorB, darkcolorG;
    double bumpAmount;
    double uvBleedSpace;
    bool createTreeShader;
    bool createLeafShader;
    bool createBump;

    void Set(double lr, double lg, double lb, double dr, 
        double dg, double db, bool treeShader, bool leafShader, 
        bool bump, double bumpamount, double bleed)
    {
        uvBleedSpace = bleed;
        lightcolorR = lr;   
        darkcolorR = dr;
        lightcolorG = lg;   
        darkcolorG = dg;
        lightcolorB = lb;   
        darkcolorB = db;
        createTreeShader = treeShader;
        createLeafShader = leafShader;
        createBump = bump;
        bumpAmount = bumpamount;
    }
};

/**
* Holds rule data for the overall tree
*/
struct TreeData
{
    double initialRadius;
    double branchRadDec;
    double minRadius;
    unsigned branchDeathProb;
    string rule;
    MString treename;
    MString treeshadername;
    MObject tree;

    void Set(double rad, double raddec, 
        double minrad, unsigned branchdeath)
    {
        minRadius = minrad;
        rule = "";
        initialRadius = rad;
        branchRadDec = raddec;
        branchDeathProb = branchdeath;
    }
};

/**
* Holds rule data an individual trunk/branch
*/
struct BranchData
{ 
    double forward, forwardang, forwardvar;
    double angle, anglevar;
    double radiusdec;
    BranchData(double f, double fangle, double fvar, 
        double a, double avar, double raddec)
    {
        forward = f;            
        angle = a;      
        forwardang = fangle;    
        anglevar = avar;
        forwardvar = fvar;      
        radiusdec = raddec;
    }
};

/**
* Holds data for a mesh of a branch
*/
struct MeshStruct
{
    int maxLayers;
    bool preview, merge, capends, createAsCurves, randomize;
    unsigned trunkfaces, branchfaces, facedec;

    void Set(unsigned TrunkFaces, unsigned BranchFaces, 
        unsigned FaceDec, bool Curves, bool Capends, 
        bool Rand, bool Merge, bool Preview)
    { 
        maxLayers = 0;              
        preview = Preview; 
        randomize = Rand;           
        merge = Merge;
        trunkfaces = TrunkFaces;    
        branchfaces = BranchFaces; 
        facedec = FaceDec;          
        createAsCurves = Curves; 
        capends = Capends;
    }
};

/**
* Holds data for a mesh of a leaf
*/
struct LeafData
{
    MString leafshadername;
    unsigned leafStartLayer;
    bool leafTree;
    double w, h, wv, hv, bend;
    MString file;

    void Set(bool LeafTree, double W, double H, double WVar, 
        double HVar, double Bend, unsigned LeafLayer)
    {
        leafshadername = "";
        leafStartLayer = LeafLayer;
        w = W;  
        h = H;  
        wv = WVar;  
        hv = HVar;
        bend = Bend;
        leafTree = LeafTree;
        file = "";
    }
};

/**
* Holds data for a section of a branch
*/
struct Section
{
    FLOAT3 position;
    float radius;

    Section()
    {
    }

    Section(const FLOAT3& p, float r)
    { 
        position = p; 
        radius = r; 
    }

    Section(float x, float y, float z, float r)
    { 
        position.x = x; 
        position.y = y;
        position.z = z;
        radius = r;
    }
};

/**
* Holds data for a branch
*/
struct Branch
{
    MATRIX rotmat;
    MATRIX scalemat;
    int parentIndex;
    int sectionIndex;
    int layer;
    int vertno;
    bool mergedtop;
    bool mergedbot;
    MObject mesh;
    deque<Section> sections;
    deque<int> children;

    Branch()
    { 
        mergedtop = mergedbot = false; 
    }
};

/**
* Holds radius disk vertex information of a branch
*/
struct Disk
{
    deque<FLOAT3> points;
};

/**
* Holds data for a leaf
*/
struct Leaf
{
    int layer;
    MObject mesh;
    FLOAT3 position;
    FLOAT3 sectionAxis;
    float sectionRadius;

    Leaf(const FLOAT3& p, const FLOAT3& a, int l, float r)
    { 
        position = p; 
        sectionAxis = a; 
        layer = l; 
        sectionRadius = r; 
    }
};

/**
* Turtle object that navigates a rule string
*/
struct Turtle
{
    MATRIX world;
    double radius;
    int branchIndex;
    int sectionIndex;
    int branchParent;
    int layerIndex;
    bool branchEnded;
};

/**
* A layer of the tree 
*/
struct Layer
{
    MObject layer;
    MObject branches;
    MObject leaves;
};
