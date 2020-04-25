////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - treeGenerator.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "treeGenerator.h"
#include "treeHelpers.h"
#include "randomGenerator.h"

#include <fstream>
#include <ctime>

int TreeGenerator::sm_treeNumber = 0;

TreeGenerator::TreeGenerator()
    : MPxCommand()
    , m_dagMod(std::make_unique<MDagModifier>())
    , m_meshdata(8, 8, 2, false, false, true, false)
    , m_leafdata(true, 2.0f, 4.0f, 1.0f, 1.0f, 1.0f, 2)
    , m_treedata(2.0, 0.9, 0.001, 10)
    , m_fxdata(0.732982, 0.495995, 0.388067, 0.083772, 
               0.0572824, 0.013138, true, true, true, 0.2, 0.01)
{
    m_ruleChances.assign(0);
}

MStatus TreeGenerator::doIt(const MArgList& args)
{
    // Get the user input parameters
    MStatus status;
    MArgDatabase argData(syntax(), args, &status);

    if(!status)
    { 
        return status; 
    }

    // Create the default data
    BranchData trunk(1.0, 5.0, 0.2, 22.2, 5.0, 0.9);
    BranchData branch(1.0, 15.0, 0.5, 22.2, 5.0, 0.95);

    // Create the default rules
    m_treedata.rule = "";
    MString prerule = "FGGFGGFGGF";
    MString postrule = "";
    MString start = "A";
    m_ruleIDs[0] = "A";
    m_ruleStrings[0] = "[>FGLLLFGLLLFLLLA]^^^^^[>FGLLLFGLLLFLLLA]^^^^^^^[>FGLLLFGLLLFLLLA]";
    m_ruleChances[0] = 100;
    GetFlagArguments(argData, prerule, postrule, start, branch, trunk);

    // Set preview variables
    if(m_meshdata.preview)
    {
        m_meshdata.createAsCurves = true;
        m_leafdata.treeHasLeaves = false;
        m_fxdata.createTreeShader = false;
        m_fxdata.createLeafShader = false;
    }

    // Generate a new seed if randomize chosen
    if(m_meshdata.randomize) 
    {
        Random::RandomizeSeed();
    }

    // Progress window setup
    int progress = 2;
    if(m_leafdata.treeHasLeaves) 
    { 
        ++progress; 
    }
    StartProgressWindow(progress);
    m_progressStep = 2;

    // Create the rule string
    m_treedata.rule = start.asChar();
    if(!CreateRuleString()) 
    { 
        EndProgressWindow(); 
        return MStatus::kFailure; 
    }

    // Add prerule/postrule
    m_treedata.rule = prerule.asChar() + m_treedata.rule;
    m_treedata.rule += postrule.asChar();

    // Navigate the turtle
    if(!BuildTheTree(branch, trunk)) 
    { 
        EndProgressWindow(); 
        return MStatus::kFailure; 
    }

    // Create the mesh
    if(!MeshTheTree()) 
    { 
        EndProgressWindow(); 
        return MStatus::kFailure; 
    }

    EndProgressWindow();
    return MStatus::kSuccess;
}

bool TreeGenerator::BuildTheTree(BranchData& branch, BranchData& trunk)
{
    /* TURTLE COMMANDS
    * F: draw forward
    * G: move forward without drawing
    * v: anticlockwise around z axis
    * ^: clockwise around z axis
    * >: clockwise around x axis
    * <: anticlockwise around x axis
    * -: anticlockwise around y axis
    * +: clockwise around y axis
    * L: create leaf
    * [: Push turtle onto stack
    * ]: Pop turtle off stack
    */

    // Set up progress
    DescribeProgressWindow("Building:");
    unsigned int progressMod = static_cast<unsigned int>(
        (m_treedata.rule.size() / m_progressIncrease) * m_progressStep);

    // Create the turtle
    Turtle turtle;
    turtle.world.RotateXLocal(static_cast<float>(DegToRad(90.0f)));
    turtle.radius = m_treedata.initialRadius;
    turtle.branchIndex = 0;
    turtle.sectionIndex = 0;
    turtle.layerIndex = 0;
    turtle.branchParent = -1;
    turtle.branchEnded = false;
    std::deque<Turtle> stack;

    // Set up tree
    int trunkIndex = 0;
    BranchData* values = &trunk;
    m_branches.push_back(Branch());
    m_branches[trunkIndex].layer = 0;
    m_branches[trunkIndex].parentIndex = -1;
    m_branches[trunkIndex].sections.push_back(Section(
        0, 0, 0, static_cast<float>(m_treedata.initialRadius)));

    // Navigate the turtle
    for(unsigned int j = 0, progress = 0; j < m_treedata.rule.size(); ++j, ++progress)
    {
        Float3 result;
        double angle = 0.0;

        switch(m_treedata.rule[j])
        {
            case 'F':
            {
                // Move forward with drawing 
                result = DetermineForwardMovement(turtle, values->forward, 
                    values->forwardAngle, values->forwardVariance);

                turtle.world.Translate(result);
                
                // Change radius
                turtle.radius *= values->radiusDecrease;
                if(turtle.radius < m_treedata.minimumRadius)
                { 
                    turtle.radius  = m_treedata.minimumRadius; 
                }

                // Add section to branch
                m_branches[turtle.branchIndex].sections.push_back(
                    Section(turtle.world.Position(), static_cast<float>(turtle.radius)));
                turtle.sectionIndex++;
                break;
            }
            case 'G':
            {
                // Move forward without drawing
                result = DetermineForwardMovement(turtle, values->forward, 
                    values->forwardAngle, values->forwardVariance);

                turtle.world.Translate(result);
                break;
            }
            case '[':
            {
                // Push current tutle onto the stack
                if(!TryKillBranch(j))
                {
                    turtle.branchEnded = true;
                    stack.push_back(Turtle(turtle));
                    BuildNewBranch(turtle, trunkIndex, branch, &values);
                }
                break;
            }
            case ']':
            {
                // Pop back tutle from stack
                if(stack.size() > 0)
                {
                    turtle = stack[stack.size()-1];
                    stack.pop_back();
                    values = (turtle.branchIndex == trunkIndex) ? &trunk : &branch;
                }
                break;
            }
            case '+':
            {
                // Rotate positive Y
                angle = Random::Generate(-1.0, 1.0);
                turtle.world.RotateYLocal(static_cast<float>(
                    DegToRad(values->angle + (values->angleVariance * angle))));
                break;
            }
            case '-':
            {
                // Rotate negative y
                angle = Random::Generate(-1.0, 1.0);
                turtle.world.RotateYLocal(static_cast<float>(
                    DegToRad(-values->angle + (values->angleVariance * angle))));
                break;
            }
            case '>':
            {
                // Rotate positive x
                angle = Random::Generate(-1.0, 1.0);
                turtle.world.RotateXLocal(static_cast<float>(
                    DegToRad(values->angle + (values->angleVariance * angle))));
                break;
            }
            case '<':
            {
                // Rotate negative x
                angle = Random::Generate(-1.0, 1.0);
                turtle.world.RotateXLocal(static_cast<float>(
                    DegToRad(-values->angle + (values->angleVariance * angle))));
                break;
            }
            case '^':
            {
                // Rotate positive z
                angle = Random::Generate(-1.0, 1.0);
                turtle.world.RotateZLocal(static_cast<float>(
                    DegToRad(values->angle + (values->angleVariance * angle))));
                break;
            }
            case 'v':
            {
                // Rotate negative z
                angle = Random::Generate(-1.0, 1.0);
                turtle.world.RotateZLocal(static_cast<float>(
                    DegToRad(-values->angle + (values->angleVariance * angle))));
                break;
            }
            case 'L':
            {
                // Create a leaf
                if(m_leafdata.treeHasLeaves 
                   && (turtle.branchIndex != trunkIndex)
                   && (turtle.layerIndex >= static_cast<int>(m_leafdata.leafLayer)) 
                   && (turtle.sectionIndex != 0))
                {
                    Float3 axis = m_branches[turtle.branchIndex].sections[turtle.sectionIndex].position 
                        - m_branches[turtle.branchIndex].sections[turtle.sectionIndex-1].position;

                    m_leaves.push_back(Leaf(turtle.world.Position(),
                        axis.GetNormalized(), turtle.layerIndex, static_cast<float>(turtle.radius)));
                }
                break;
            }
        }

        // Increase progress window
        if(progress >= progressMod) 
        { 
            progress = 0; 
            AdvanceProgressWindow(m_progressStep);
        }

        // Check plugin is continuing
        if(PluginIsCancelled())
        { 
            return false; 
        }
    }
    return true;
}

bool TreeGenerator::CreateRuleString()
{
    std::string temprule = "";
    int ruleNum; 
    char rulechar;
    for(unsigned int i = 0; i < m_iterations; ++i)
    {
        // Ror each letter in the string
        for(unsigned int j = 0; j < m_treedata.rule.size(); ++j)
        {
            // Test against each possible symbol
            rulechar = m_treedata.rule[j];
            for(ruleNum = 0; ruleNum < RULE_NUMBER; ++ruleNum)
            {
                if(rulechar == (m_ruleIDs[ruleNum].asChar())[0])
                {
                    if(m_ruleChances[ruleNum] == 100)
                    {
                        temprule += m_ruleStrings[ruleNum].asChar();
                    }
                    else if (m_ruleChances[ruleNum] != 0)
                    {
                        if(Random::Generate(0, 100) <= static_cast<int>(m_ruleChances[ruleNum]))
                        {
                            temprule += m_ruleStrings[ruleNum].asChar();
                        }
                    }
                    break;
                }
            }

            // No rule found, leave in string
            if(ruleNum == RULE_NUMBER)
            {
                temprule += rulechar;
            }
        }

        m_treedata.rule = temprule;
        temprule = "";

        if(PluginIsCancelled())
        {
            return false;
        }
    }
    return true;
}

bool TreeGenerator::TryKillBranch(unsigned int& index)
{
    // Check probability of branch dying
    if(Random::Generate(0, 100) < static_cast<int>(m_treedata.branchDeathProbability))
    {
        // Remove all symbols up until corresponding ]
        int searchnumber = 1;
        while(index < m_treedata.rule.size())
        {
            ++index;
            if(m_treedata.rule[index] == '[')
            {
                searchnumber++;
            }
            if(m_treedata.rule[index] == ']')
            {
                searchnumber--;
            }
            if(searchnumber == 0)
            {
                break;
            }
        }
        return true;
    }
    return false;
}

void TreeGenerator::BuildNewBranch(Turtle& turtle, int trunkIndex, BranchData& trunk, BranchData** values)
{
    // Change values used if moving from trunk to branch
    if(turtle.branchIndex == trunkIndex)
    {
        *values = &trunk;
    }

    // Change radius
    turtle.radius *= m_treedata.branchRadiusDecrease;
    if(turtle.radius < m_treedata.minimumRadius)
    { 
        turtle.radius = m_treedata.minimumRadius;
    }
    
    // Start a new branch
    turtle.layerIndex++;
    m_meshdata.maxLayers = max(m_meshdata.maxLayers, turtle.layerIndex); 
    turtle.branchParent = turtle.branchIndex;
    turtle.branchIndex = static_cast<int>(m_branches.size());
    turtle.branchEnded = false;

    m_branches.push_back(Branch());
    m_branches[turtle.branchIndex].sections.push_back(
        Section(turtle.world.Position(), static_cast<float>(turtle.radius)));

    m_branches[turtle.branchIndex].layer = turtle.layerIndex;
    m_branches[turtle.branchIndex].parentIndex = turtle.branchParent;
    m_branches[turtle.branchIndex].sectionIndex = turtle.sectionIndex;
    m_branches[turtle.branchParent].children.push_back(turtle.branchIndex);
    turtle.sectionIndex = 0;
}

bool TreeGenerator::MeshTheTree()
{
    // Turn off history
    MString hResult = MGlobal::executeCommandStringResult(
        MString("constructionHistory -q -tgl"));
    TurnOffHistory();

    CreateTreeGroup();
    CreateShaders();

    // Check okay to continue
    if(PluginIsCancelled()) 
    { 
        DeleteNodes(); 
        TurnOnHistory(hResult.asInt()); 
        return false; 
    }

    if(m_meshdata.createAsCurves)
    {
        // Create curve tree
        if(!CreateCurves())
        { 
            DeleteNodes(); 
            TurnOnHistory(hResult.asInt()); 
            return false; 
        }
    } 
    else
    {
        // Create mesh tree
        if(!CreateMeshes())
        { 
            DeleteNodes(); 
            TurnOnHistory(hResult.asInt()); 
            return false; 
        }
    }

    // Leaf the tree
    if(m_leafdata.treeHasLeaves)
    {
        if(!CreateLeaves())
        { 
            DeleteNodes(); 
            TurnOnHistory(hResult.asInt()); 
            return false; 
        }
    }

    // Rename all
    m_dagMod->doIt();

    TurnOnHistory(hResult.asInt());
    return true;
}

bool TreeGenerator::CreateTreeGroup()
{
    ++sm_treeNumber;
    m_treedata.treename = MString("tf_tree_") + sm_treeNumber;

    MFnTransform transFn;
    m_treedata.tree = transFn.create();
    ++m_meshdata.maxLayers;

    for(int i = 0; i < m_meshdata.maxLayers; ++i)
    {
        m_layers.push_back(Layer());
        m_layers[i].layer = transFn.create();
        m_dagMod->renameNode(m_layers[i].layer, m_treedata.treename + "_Layer" + i);
        m_dagMod->reparentNode(m_layers[i].layer, m_treedata.tree);
        
        if(m_leafdata.treeHasLeaves)
        {
            m_layers[i].leaves = transFn.create();
            m_dagMod->renameNode(m_layers[i].leaves, m_treedata.treename+"_Layer" + i + "_Leaves");
            m_dagMod->reparentNode(m_layers[i].leaves, m_layers[i].layer);
        }

        if(!m_meshdata.createAsCurves)
        {
            m_layers[i].branches = transFn.create();
            m_dagMod->renameNode(m_layers[i].branches, m_treedata.treename + "_Layer" + i + "_Branches");
            m_dagMod->reparentNode(m_layers[i].branches, m_layers[i].layer);
        }
    }

    m_dagMod->renameNode(m_treedata.tree, m_treedata.treename);
    
    return !PluginIsCancelled();
}

bool TreeGenerator::CreateCurves()
{
    DescribeProgressWindow("Meshing:");
    unsigned int progressMod = static_cast<unsigned int>(
        (m_branches.size() / m_progressIncrease) * m_progressStep); 

    // Create each branch
    for(unsigned int j = 0, progress = 0; j < m_branches.size(); ++j, ++progress)
    {
        if(m_branches[j].sections.size() > 1)
        {
            CreateCurve(m_branches[j], m_treedata.treename + "_B" + j, 
                m_layers[m_branches[j].layer].layer);
        }

        if(progress >= progressMod) 
        { 
            progress = 0; 
            AdvanceProgressWindow(m_progressStep); 
        }

        if(PluginIsCancelled())
        {
            return false;
        }
    }
    return true;
}

bool TreeGenerator::CreateMeshes()
{
    DescribeProgressWindow("Meshing:");
    unsigned int progressMod = static_cast<unsigned int>(
        (m_branches.size() / m_progressIncrease) * m_progressStep); 

    // Create the disks
    std::deque<Disk> disk;
    disk.push_back(Disk());
    float angle = 360.0f / m_meshdata.trunkfaces;

    for(unsigned int i = 0; i < m_meshdata.trunkfaces; ++i)
    {
        disk[0].points.push_back(Float3(
            cos(DegToRad(i * angle)), 0.0f, 
            sin(DegToRad(i * angle))));
    }

    const int MAX_FACES = 3;
    for(unsigned int j = 1; j < m_layers.size(); ++j)
    {
        disk.push_back(Disk());
        int facenumber = m_meshdata.branchfaces - (m_meshdata.faceDecrease*j);
        if(facenumber < MAX_FACES)
        { 
            facenumber = MAX_FACES; 
        }

        angle = 360.0f / facenumber;
        for(int i = 0; i < facenumber; ++i)
        {
            disk[j].points.push_back(Float3(
                static_cast<float>(cos(DegToRad(i*angle))), 0, 
                static_cast<float>(sin(DegToRad(i*angle)))));
        }
    }

    // Create each branch
    for(unsigned int j = 0, progress = 0; j < m_branches.size(); ++j, ++progress)
    {
        if(m_branches[j].sections.size() > 1)
        {
            Branch* parent = m_branches[j].parentIndex >= 0 ?
                &m_branches[m_branches[j].parentIndex] : nullptr;

            CreateMesh(&m_branches[j], parent, 
                disk[m_branches[j].layer], 
                m_treedata.treename+"_BRN" + j, 
                m_layers[m_branches[j].layer].branches);
        }

        // Advance progress bar
        if(progress >= progressMod)
        { 
            progress = 0; 
            AdvanceProgressWindow(m_progressStep); 
        }

        if(PluginIsCancelled())
        {
            return false;
        }
    }
    return true;
}

bool TreeGenerator::CreateLeaves()
{
    DescribeProgressWindow("Leafing:");
    unsigned int progressMod = static_cast<unsigned int>(
        (m_leaves.size() / m_progressIncrease) * m_progressStep);

    int vertno = 0;
    float bleed = static_cast<float>(m_fxdata.uvBleedSpace);
    if(m_leafdata.bendAmount == 0)
    {
        vertno = 4;
        m_leafPolycounts.append(4);

        //Create indices (face1)
        m_leafIndices.append(0);    
        m_leafIndices.append(1);    
        m_leafIndices.append(3);    
        m_leafIndices.append(2);

        // Create the uvs (face1)
        m_leafU.append(0.0f);   
        m_leafV.append(0.0f);
        m_leafU.append(1.0f);   
        m_leafV.append(0.0f); 
        m_leafU.append(1.0f); 
        m_leafV.append(1.0f); 
        m_leafU.append(0.0f);  
        m_leafV.append(1.0f); 
        m_leafUVids.append(0);  
        m_leafUVids.append(1);  
        m_leafUVids.append(2);  
        m_leafUVids.append(3);
    }
    else
    {
        vertno = 6;
        m_leafPolycounts.append(4); 
        m_leafPolycounts.append(4);

        // Create indices for face 1
        m_leafIndices.append(0);    
        m_leafIndices.append(1);    
        m_leafIndices.append(3);    
        m_leafIndices.append(2);

        // Create indices for face 2
        m_leafIndices.append(2);    
        m_leafIndices.append(3);    
        m_leafIndices.append(5);    
        m_leafIndices.append(4); 

        // Create the uvs
        m_leafU.append(0.0f + bleed);  
        m_leafV.append(0.0f + bleed);
        m_leafU.append(1.0f - bleed); 
        m_leafV.append(0.0f + bleed); 
        m_leafU.append(1.0f - bleed); 
        m_leafV.append(0.5f); 
        m_leafU.append(0.0f + bleed);  
        m_leafV.append(0.5f); 
        m_leafU.append(1.0f - bleed);  
        m_leafV.append(1.0f - bleed); 
        m_leafU.append(0.0f + bleed);  
        m_leafV.append(1.0f - bleed); 

        // Face1
        m_leafUVids.append(0);  
        m_leafUVids.append(1);  
        m_leafUVids.append(2);  
        m_leafUVids.append(3);

        // Face2
        m_leafUVids.append(3);  
        m_leafUVids.append(2);  
        m_leafUVids.append(4);  
        m_leafUVids.append(5);
    }

    // Create leaves
    for(int i = 0; i < vertno; ++i)
    {
        m_leafVertices.push_back(Float3());
    }

    for(unsigned int i = 0, progress = 0; i < m_leaves.size(); ++i, ++progress)
    {
        CreateLeaf(m_leaves[i], m_treedata.treename + "_LVS" + i,
            m_layers[m_leaves[i].layer].leaves);

        // Advance progress bar
        if(progress >= progressMod)
        { 
            progress = 0; 
            AdvanceProgressWindow(m_progressStep); 
        }

        if(PluginIsCancelled())
        {
            return false;
        }
    }
    return true;
}

void TreeGenerator::CreateLeaf(Leaf& leaf, MString& meshname, MObject& layer)
{
    MFloatPointArray vertices;
    MFnMesh meshfn;

    // Create the verts
    double angle = Random::Generate(-360, 360);

    float width = static_cast<float>(m_leafdata.width + 
        (m_leafdata.widthVariance * Random::Generate(-1.0, 1.0)));

    float height = static_cast<float>(m_leafdata.height + 
        (m_leafdata.heightVariance * Random::Generate(-1.0, 1.0)));

    Matrix rotation = Matrix::CreateRotateArbitrary(
        leaf.sectionAxis, static_cast<float>(DegToRad(angle)));

    if(m_leafdata.bendAmount != 0)
    {
        m_leafVertices[0].Set(-width/2, 0, 0);
        m_leafVertices[1].Set(width/2, 0, 0);

        m_leafVertices[2].Set(-width/2, static_cast<float>(
            m_leafdata.bendAmount * Random::Generate(-1.0, 1.0)), height/2);

        m_leafVertices[3].Set(width/2, static_cast<float>(
            m_leafdata.bendAmount * Random::Generate(-1.0, 1.0)), height/2);

        m_leafVertices[4].Set(-width/2, 0, height);
        m_leafVertices[5].Set(width/2, 0, height);
    }
    else
    {
        m_leafVertices[0].Set(-width/2, 0, 0);
        m_leafVertices[1].Set(width/2, 0, 0);
        m_leafVertices[2].Set(-width/2, 0, height);
        m_leafVertices[3].Set(width/2, 0, height);
    }

    // Rotate verts
    for (Float3& vertex : m_leafVertices)
    {
        vertex *= rotation;  
    }

    // Move verts roughly outside branch
    Float3 offset = m_leafVertices[3] - m_leafVertices[1];
    offset = (leaf.sectionAxis.Cross(offset)).Cross(leaf.sectionAxis);
    offset.Normalize();
    offset *= leaf.sectionRadius / 2.0f;
    leaf.position += offset;

    // Save verts
    for(unsigned int i = 0; i < m_leafVertices.size(); ++i)    
    {
        vertices.append(leaf.position.x + m_leafVertices[i].x, 
            leaf.position.y + m_leafVertices[i].y, 
            leaf.position.z + m_leafVertices[i].z);
    }

    // Create the mesh
    leaf.mesh = meshfn.create(vertices.length(), m_leafPolycounts.length(), 
        vertices, m_leafPolycounts, m_leafIndices, m_leafU, m_leafV);

    meshfn.assignUVs(m_leafPolycounts, m_leafUVids); 
    m_dagMod->renameNode(leaf.mesh, meshname);
    m_dagMod->reparentNode(leaf.mesh, layer);

    // Shade the mesh
    const auto shader = m_fxdata.createLeafShader ? 
        m_leafdata.leafshadername + "SG " : "initialShadingGroup ";

    MGlobal::executeCommand("sets -e -fe " + shader + meshfn.name());
}

void TreeGenerator::CreateMesh(Branch* branch, 
                               Branch* parent, 
                               Disk& disk, 
                               MString& meshname, 
                               MObject& layer)
{
    MFloatPointArray vertices;
    MIntArray polycounts;
    MIntArray indices;
    MIntArray uvIDs;
    MFnMesh meshfn;
    MFloatArray uCoord;
    MFloatArray vCoord;

    int facenumber = static_cast<int>(disk.points.size());
    int sectionnumber = static_cast<int>(branch->sections.size());

    int pastindex = 0;
    int index = 0;
    int sIndex = 0;
    int sPastindex = 0;
    int uvPastindex = 0;
    int uvIndex = 0;
    int uvringnumber = facenumber+1;
    float bleed = (float)m_fxdata.uvBleedSpace;

    // Get matrices for initial ring
    if(parent != nullptr) 
    { 
        branch->scaleMat = parent->scaleMat;
        branch->rotationMat = parent->rotationMat; 
    }   
    else          
    { 
        branch->scaleMat.Scale(branch->sections[0].radius); 
    }

    for(int j = 0; j < facenumber; ++j)
    {
        Float3 position = disk.points[j];
        position *= branch->scaleMat;
        position *= branch->rotationMat;
        position += branch->sections[0].position; 
        vertices.append(position.x, position.y, position.z);

        vCoord.append(bleed);
        uCoord.append(ChangeRange(static_cast<float>(j), 0.0f,
            static_cast<float>(facenumber), bleed,  1.0f - bleed));
    }
    uCoord.append(1.0f - bleed);
    vCoord.append(bleed);

    // Other branch rings
    for(int i = 1; i < sectionnumber; ++i)
    {
        sIndex = i * facenumber;
        sPastindex = (i-1) * facenumber;
        uvIndex = i * uvringnumber;
        uvPastindex = (i-1) * uvringnumber;

        // Create scale matrix
        const Section& section = branch->sections[i];
        branch->scaleMat.MakeIdentity();
        branch->scaleMat.Scale(section.radius);

        // Create rotation matrix
        branch->rotationMat.MakeIdentity();
        if(i == sectionnumber - 1)
        {
            // Rotate in direction of past axis
            Float3 up(0.0f, 1.0f, 0.0f);
            Float3 pastAxis = branch->sections[i].position - branch->sections[i-1].position;
            Float3 rotAxis = pastAxis.Cross(up);
            rotAxis.Normalize();
            float angle = up.Angle(pastAxis);
            branch->rotationMat = Matrix::CreateRotateArbitrary(rotAxis, angle);
        }
        else
        {
            // Rotate half way between past/future
            Float3 up(0.0f, 1.0f, 0.0f);
            Float3 axis = (branch->sections[i].position - branch->sections[i-1].position) 
                + (branch->sections[i+1].position - branch->sections[i].position); //past axis+future axis

            Float3 rotAxis = axis.Cross(up);
            rotAxis.Normalize();
            const float angle = up.Angle(axis);
            branch->rotationMat = Matrix::CreateRotateArbitrary(rotAxis, angle);
        }

        // Find v coordinate
        float vcoordinate = ChangeRange(static_cast<float>(i),
            0.0f,static_cast<float>(sectionnumber-1),bleed,1.0f-bleed);

        // For each vertex/face
        for(int j = 0; j < facenumber; ++j)
        {
            // Create vertex
            Float3 position = disk.points[j];
            position *= branch->scaleMat; // scale
            position *= branch->rotationMat; // rotate
            position += section.position; // translate
            vertices.append(position.x, position.y, position.z);
            uCoord.append(uCoord[j]);
            vCoord.append(vcoordinate);

            // Create faces
            polycounts.append(4);
            index = sIndex + j;
            pastindex = sPastindex + j;
            indices.append(index);  
            indices.append(index + 1 == sIndex + facenumber ? sIndex : (index + 1));
            indices.append(pastindex + 1 == sPastindex + facenumber ? sPastindex : (pastindex + 1));
            indices.append(pastindex);

            // Create uvids
            uvIDs.append(uvIndex + j);
            uvIDs.append(uvIndex + j + 1);
            uvIDs.append(uvPastindex + j + 1);
            uvIDs.append(uvPastindex + j);
        }

        uCoord.append(1.0f - bleed);
        vCoord.append(vcoordinate);
    }

    // Cap the end of the branch
    if(branch->children.empty() && m_meshdata.capEnds)
    {
        // Create middle vert
        Float3 middle = branch->sections[branch->sections.size()-1].position;
        vertices.append(middle.x, middle.y, middle.z);

        // Create middle uvs
        Float3 middlepos(0.5f, 0.0f, 0.5f);
        uCoord.append(middlepos.x);
        vCoord.append(middlepos.z);
        int middleuv = uCoord.length()-1;
        int startuv = uCoord.length();
        int topindex = vertices.length()-2;
        int midindex = vertices.length()-1;
        int topj = facenumber-1;
        Matrix capscale;
        capscale.Scale(0.25f);

        // Note, this goes backwards
        for(int j = 0; j < facenumber; ++j)
        {
            // Create faces
            polycounts.append(3);
            int index1 = topindex-j;
            int index2 = j == topj ? topindex : index1-1;
            indices.append(index2);
            indices.append(midindex);
            indices.append(index1);

            // Create uvs
            Float3 position = disk.points[topj - j];
            position *= capscale;
            uCoord.append(position.x + middlepos.x); 
            vCoord.append(position.z + middlepos.z);
            uvIDs.append(startuv + j);
            uvIDs.append(middleuv);
            uvIDs.append(j == topj ? startuv : startuv + j + 1);
        }
    }

    // Create the mesh
    branch->vertNumber = vertices.length();
    branch->mesh = meshfn.create(vertices.length(), 
        polycounts.length(), vertices, polycounts, indices, uCoord, vCoord);

    meshfn.assignUVs(polycounts, uvIDs);
    m_dagMod->renameNode(branch->mesh, meshname);
    m_dagMod->reparentNode(branch->mesh, layer);

    // Shade the mesh
    const auto shader = m_fxdata.createTreeShader ? 
        m_treedata.treeshadername + "SG " : "initialShadingGroup ";

    MGlobal::executeCommand("sets -e -fe " + shader + meshfn.name());
}

void TreeGenerator::CreateCurve(Branch& branch, MString& meshname, MObject& layer)
{
    MPointArray editPoints;

    for(unsigned int i = 0; i < branch.sections.size(); ++i)
    {
        const Float3& position = branch.sections[i].position;
        editPoints.append(position.x, position.y, position.z);
    }

    MFnNurbsCurve curveFn;
    branch.mesh = curveFn.createWithEditPoints(editPoints,
        1, MFnNurbsCurve::kOpen, false, true, true);

    m_dagMod->renameNode(branch.mesh, meshname);
    m_dagMod->reparentNode(branch.mesh, layer);
}

bool TreeGenerator::CreateShaders()
{
    if(m_fxdata.createTreeShader)
    {
        // Create branch shader
        m_treedata.treeshadername = m_treedata.treename + "_branchshader";
        MString texnoise = m_treedata.treeshadername + "_noise";
        MString bump = m_treedata.treeshadername + "_bump";

        MGlobal::executeCommand("shadingNode -name " + 
            m_treedata.treeshadername + " -asShader lambert");

        MGlobal::executeCommand("sets -renderable true -noSurfaceShader true -empty -name "
            + m_treedata.treeshadername + "SG");

        MGlobal::executeCommand("connectAttr -force " + m_treedata.treeshadername + 
            ".outColor " + m_treedata.treeshadername + "SG.surfaceShader");

        MGlobal::executeCommand("shadingNode -name " + 
            texnoise + " -asTexture volumeNoise");

        MGlobal::executeCommand("connectAttr -force " + 
            texnoise + ".outColor " + m_treedata.treeshadername + ".color");

        // Set branch shader attributes
        MString lightcolor = MString("-type double3 ") + m_fxdata.lightcolorR + 
            " " + m_fxdata.lightcolorG + " " + m_fxdata.lightcolorB;

        MString darkcolor = MString("-type double3 ") + m_fxdata.darkcolorR + 
            " " + m_fxdata.darkcolorG + " " + m_fxdata.darkcolorB;

        MGlobal::executeCommand("setAttr " + texnoise + ".colorGain " + lightcolor);
        MGlobal::executeCommand("setAttr " + texnoise + ".colorOffset " + darkcolor);
        MGlobal::executeCommand("setAttr " + texnoise + ".noiseType 3");
        MGlobal::executeCommand("setAttr " + texnoise + ".alphaIsLuminance true");
        MGlobal::executeCommand("setAttr " + texnoise + ".frequencyRatio 0.5");

        // Create bump
        if(m_fxdata.createBump)
        {
            MGlobal::executeCommand("shadingNode -name " +
                bump + " -asUtility bump3d");

            MGlobal::executeCommand("connectAttr -force " +
                texnoise + ".outAlpha " + bump + ".bumpValue");

            MGlobal::executeCommand("connectAttr -force " + bump +
                ".outNormal " + m_treedata.treeshadername + ".normalCamera");

            MGlobal::executeCommand("setAttr " + bump + 
                ".bumpDepth " + m_fxdata.bumpAmount);
        }
    }
    // Create leaf shader
    if(m_fxdata.createLeafShader)
    {
        m_leafdata.leafshadername = m_treedata.treename + "_leafshader";
        MString texname = m_leafdata.leafshadername + "_file";
        MString str = "\"string\"";
        MString path = "\"" + m_leafdata.file + "\"";

        MGlobal::executeCommand("shadingNode -name " +
            m_leafdata.leafshadername + " -asShader lambert");

        MGlobal::executeCommand("sets -renderable true -noSurfaceShader true -empty -name " +
            m_leafdata.leafshadername + "SG");

        MGlobal::executeCommand("connectAttr -force " + m_leafdata.leafshadername + 
            ".outColor " + m_leafdata.leafshadername + "SG.surfaceShader");

        MGlobal::executeCommand("shadingNode -name " + texname + " -asTexture file");

        MGlobal::executeCommand("connectAttr -force " + texname + 
            ".outColor " + m_leafdata.leafshadername + ".color");

        MGlobal::executeCommand("connectAttr -force " + texname + 
            ".outTransparency " + m_leafdata.leafshadername + ".transparency");

        MGlobal::executeCommand("setAttr -type " + str + " " +
            texname + ".fileTextureName " + path);

        MGlobal::executeCommand("setAttr " +
            m_leafdata.leafshadername + ".shadowAttenuation 0");
    }
    return true;
}

void TreeGenerator::DeleteNodes()
{
    sm_treeNumber--;

    if(m_fxdata.createLeafShader)
    {
        MGlobal::executeCommand("delete " + m_leafdata.leafshadername);
        MGlobal::executeCommand("delete " + m_leafdata.leafshadername + "SG");
        MGlobal::executeCommand("delete " + m_leafdata.leafshadername + "_file");
    }

    if(m_fxdata.createTreeShader)
    {
        MGlobal::executeCommand("delete " + m_treedata.treeshadername);
        MGlobal::executeCommand("delete " + m_treedata.treeshadername + "SG");
        MGlobal::executeCommand("delete " + m_treedata.treeshadername + "_noise");
        if(m_fxdata.createBump)
        {
            MGlobal::executeCommand("delete " + m_treedata.treeshadername + "_bump");
        }
    }

    m_dagMod->deleteNode(m_treedata.tree);
    m_dagMod->doIt();
}

Float3 TreeGenerator::DetermineForwardMovement(const Turtle& turtle, 
                                               double forward, 
                                               double angle, 
                                               double variation) const
{
    const double x = Random::Generate(-1.0, 1.0);
    const double y = Random::Generate(-1.0, 1.0);
    const double z = Random::Generate(-1.0, 1.0);
    const float length = Random::Generate(-1.0f, 1.0f);

    // Determine direction, axis must be normalized
    Float3 result = turtle.world.Forward();
    result *= Matrix::CreateRotateY(static_cast<float>(DegToRad(angle*y)));
    result *= Matrix::CreateRotateX(static_cast<float>(DegToRad(angle*x)));
    result *= Matrix::CreateRotateZ(static_cast<float>(DegToRad(angle*z)));

    // Determine forward amount
    result *= static_cast<float>(forward + (variation * length));
    return result;
}

void TreeGenerator::StartProgressWindow(int stepNumber)
{
    m_progressIncrease = 100 / stepNumber;

    // Initialise the progress window
    if(!MProgressWindow::reserve())
    {
        EndProgressWindow();
        MProgressWindow::reserve();
    }

    MProgressWindow::setTitle(MString("Progress"));
    MProgressWindow::setProgressRange(0, 100);
    MProgressWindow::setInterruptable(true);
    MProgressWindow::setProgress(0);
    MProgressWindow::setProgressStatus("Starting:");
    MProgressWindow::startProgress();
}

bool TreeGenerator::PluginIsCancelled()
{
    if(MProgressWindow::isCancelled()) 
    {
        MProgressWindow::setProgressStatus("Deleting:");
        MProgressWindow::setProgress(0);
        return true;
    }
    return false;
}

void TreeGenerator::DescribeProgressWindow(const char* Description)
{
    MProgressWindow::setProgressStatus(Description);
}

void TreeGenerator::AdvanceProgressWindow(int amount)
{
    MProgressWindow::advanceProgress(amount);
}

void TreeGenerator::EndProgressWindow()
{
    MProgressWindow::endProgress();
}

void TreeGenerator::TurnOnHistory(int shouldTurnOn)
{ 
    if(shouldTurnOn == 1)
    { 
        MGlobal::executeCommand(MString("constructionHistory -tgl on")); 
    } 
}
void TreeGenerator::TurnOffHistory()
{ 
    MGlobal::executeCommand(MString("constructionHistory -tgl off")); 
}

MSyntax TreeGenerator::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag("-i", "-iterations", MSyntax::kUnsigned);
    syntax.addFlag("-bd", "-branchdeath", MSyntax::kUnsigned);
    syntax.addFlag("-v", "-preview", MSyntax::kBoolean);
    syntax.addFlag("-fi", "-file", MSyntax::kString);

    syntax.addFlag("-l", "-leaf", MSyntax::kBoolean, MSyntax::kUnsigned);
    syntax.addFlag("-a", "-angle", MSyntax::kDouble, MSyntax::kDouble);
    syntax.addFlag("-ta", "-tangle", MSyntax::kDouble, MSyntax::kDouble);

    syntax.addFlag("-m", "-meshdata", MSyntax::kBoolean, 
        MSyntax::kBoolean, MSyntax::kBoolean );

    syntax.addFlag("-tf", "-tforward", MSyntax::kDouble, 
        MSyntax::kDouble, MSyntax::kDouble);

    syntax.addFlag("-f", "-forward", MSyntax::kDouble, 
        MSyntax::kDouble, MSyntax::kDouble);

    syntax.addFlag("-fa", "-faces", MSyntax::kUnsigned,
        MSyntax::kUnsigned, MSyntax::kUnsigned);

    syntax.addFlag("-rp","-prerule", MSyntax::kString, 
        MSyntax::kString, MSyntax::kString);

    syntax.addFlag("-r", "-radius", MSyntax::kDouble, MSyntax::kDouble, 
        MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble);

    syntax.addFlag("-ld", "-leafdata", MSyntax::kDouble, MSyntax::kDouble, 
        MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble);

    syntax.addFlag("-c", "-color", MSyntax::kDouble, MSyntax::kDouble,
        MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble);

    syntax.addFlag("-cd", "-colordata", MSyntax::kBoolean, MSyntax::kBoolean, 
        MSyntax::kBoolean, MSyntax::kDouble, MSyntax::kDouble);

    syntax.addFlag("-r1","-rule1", MSyntax::kString, MSyntax::kString, 
        MSyntax::kString, MSyntax::kString, MSyntax::kString);

    syntax.addFlag("-r2","-rule2", MSyntax::kString, MSyntax::kString, 
        MSyntax::kString, MSyntax::kString, MSyntax::kString);

    syntax.addFlag("-rc1","-rulec1", MSyntax::kString, MSyntax::kString,
        MSyntax::kString, MSyntax::kString, MSyntax::kString);

    syntax.addFlag("-rc2","-rulec2", MSyntax::kString, MSyntax::kString, 
        MSyntax::kString, MSyntax::kString, MSyntax::kString);

    syntax.addFlag("-rp1","-rulep1", MSyntax::kUnsigned, MSyntax::kUnsigned,
        MSyntax::kUnsigned, MSyntax::kUnsigned, MSyntax::kUnsigned);

    syntax.addFlag("-rp2","-rulep2", MSyntax::kUnsigned, MSyntax::kUnsigned, 
        MSyntax::kUnsigned, MSyntax::kUnsigned, MSyntax::kUnsigned);

    syntax.enableQuery(false);
    syntax.enableEdit(false);
    return syntax;
}

void TreeGenerator::GetFlagArguments(const MArgDatabase& argData, 
                                     MString prerule, 
                                     MString postrule, 
                                     MString start, 
                                     BranchData& branch,
                                     BranchData& trunk)
{
    if(argData.numberOfFlagsUsed() > 0)
    {
        argData.getFlagArgument("-fi", 0, m_leafdata.file);             
        argData.getFlagArgument("-l", 0, m_leafdata.treeHasLeaves);
        argData.getFlagArgument("-l", 1, m_leafdata.leafLayer);    
        argData.getFlagArgument("-ld", 0, m_leafdata.bendAmount);
        argData.getFlagArgument("-ld", 1, m_leafdata.height);                
        argData.getFlagArgument("-ld", 2, m_leafdata.width);
        argData.getFlagArgument("-ld", 3, m_leafdata.heightVariance);               
        argData.getFlagArgument("-ld", 4, m_leafdata.widthVariance);
        argData.getFlagArgument("-m", 0, m_meshdata.createAsCurves);   
        argData.getFlagArgument("-m", 1, m_meshdata.capEnds);   
        argData.getFlagArgument("-m", 2, m_meshdata.randomize);        
        argData.getFlagArgument("-v", 0, m_meshdata.preview);
        argData.getFlagArgument("-fa", 0, m_meshdata.trunkfaces);      
        argData.getFlagArgument("-fa", 1, m_meshdata.branchfaces);
        argData.getFlagArgument("-fa", 2, m_meshdata.faceDecrease);         
        argData.getFlagArgument("-i", 0, m_iterations);
        argData.getFlagArgument("-a", 0, branch.angle);                
        argData.getFlagArgument("-a", 1, branch.angleVariance);          
        argData.getFlagArgument("-f", 0, branch.forward);              
        argData.getFlagArgument("-f", 1, branch.forwardVariance);        
        argData.getFlagArgument("-f", 2, branch.forwardAngle);           
        argData.getFlagArgument("-r", 2, branch.radiusDecrease);
        argData.getFlagArgument("-ta", 0, trunk.angle);                
        argData.getFlagArgument("-ta", 1, trunk.angleVariance); 
        argData.getFlagArgument("-tf", 0, trunk.forward);               
        argData.getFlagArgument("-tf", 1, trunk.forwardVariance); 
        argData.getFlagArgument("-tf", 2, trunk.forwardAngle);           
        argData.getFlagArgument("-r", 3, trunk.radiusDecrease); 
        argData.getFlagArgument("-r", 0, m_treedata.initialRadius);    
        argData.getFlagArgument("-r", 1, m_treedata.branchRadiusDecrease);  
        argData.getFlagArgument("-r", 4, m_treedata.minimumRadius);       
        argData.getFlagArgument("-bd", 0, m_treedata.branchDeathProbability);
        argData.getFlagArgument("-c", 0, m_fxdata.lightcolorR);       
        argData.getFlagArgument("-c", 1, m_fxdata.lightcolorG);     
        argData.getFlagArgument("-c", 2, m_fxdata.lightcolorB);       
        argData.getFlagArgument("-c", 3, m_fxdata.darkcolorR);      
        argData.getFlagArgument("-c", 4, m_fxdata.darkcolorG);        
        argData.getFlagArgument("-c", 5, m_fxdata.darkcolorB);      
        argData.getFlagArgument("-cd", 0, m_fxdata.createTreeShader);  
        argData.getFlagArgument("-cd", 1, m_fxdata.createLeafShader);   
        argData.getFlagArgument("-cd", 2, m_fxdata.createBump);        
        argData.getFlagArgument("-cd", 3, m_fxdata.bumpAmount);     
        argData.getFlagArgument("-cd", 4, m_fxdata.uvBleedSpace);      
        argData.getFlagArgument("-rp", 0, prerule);                 
        argData.getFlagArgument("-rp", 1, start);                      
        argData.getFlagArgument("-rp", 2, postrule);

        const int HALF_MAX_RULES = RULE_NUMBER/2;
        for(int i = 0; i < HALF_MAX_RULES; ++i)
        {
            argData.getFlagArgument("-r1", i, m_ruleStrings[i]);    
            argData.getFlagArgument("-rc1", i, m_ruleIDs[i]);  
            argData.getFlagArgument("-rp1", i, m_ruleChances[i]);
            argData.getFlagArgument("-r2", i, m_ruleStrings[HALF_MAX_RULES + i]);  
            argData.getFlagArgument("-rc2", i, m_ruleIDs[HALF_MAX_RULES + i]); 
            argData.getFlagArgument("-rp2", i, m_ruleChances[HALF_MAX_RULES + i]);
        }
    }
}

bool TreeGenerator::isUndoable() const
{ 
    return false; 
}

void* TreeGenerator::creator() 
{ 
    return new TreeGenerator();
}