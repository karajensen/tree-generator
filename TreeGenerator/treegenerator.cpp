////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com
////////////////////////////////////////////////////////////////////////////////////////

#include "treegenerator.h"
#include <fstream>

namespace
{
    const MString GENERATE_COMMAND("GenerateTree");
    const MString GUI_COMMAND("TreeGenerator");
}

int TreeGenerator::sm_treeNumber = 0;
unsigned int TreeGenerator::sm_treeSeed = 0;
MDagModifier TreeGenerator::sm_dagMod;

TreeGenerator::TreeGenerator() :
    MPxCommand(),
    m_progressIncreaseAmount(0),
    m_progressStepAmount(0),
    m_meshdata(8, 8, 2, false, false, true, false),
    m_leafdata(true, 2.0f, 4.0f, 1.0f, 1.0f, 1.0f, 2),
    m_treedata(2.0, 0.9, 0.001, 10),
    m_fxdata(0.732982, 0.495995, 0.388067, 0.083772, 
        0.0572824, 0.013138, true, true, true, 0.2, 0.01)
{
}

MStatus initializePlugin(MObject obj)
{
    MFnPlugin pluginFn(obj,"Kara Jensen", "1.0");

    MStatus success = pluginFn.registerCommand(GENERATE_COMMAND, 
        TreeGenerator::creator, TreeGenerator::newSyntax);

    if(!success)
    { 
        success.perror("Register of " + GENERATE_COMMAND + " failed"); 
        return success;
    }

    success = pluginFn.registerCommand(GUI_COMMAND, 
        TreeGeneratorWindow::creator);

    if(!success)
    { 
        success.perror("Register of " + GUI_COMMAND + " failed"); 
        return success;
    }
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

MStatus TreeGeneratorWindow::doIt(const MArgList& args)
{
    // Get the absoulute path of this .dll
    char buffer[256];
    HMODULE hm = nullptr;

    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
        (LPCSTR)"treegenerator.mll", &hm);
    GetModuleFileNameA(hm, buffer, sizeof(buffer));

    // Convert the path to the gui .mel
    MString guipath(buffer);
    guipath = guipath.substring(0, guipath.length()-strlen("treegenerator.mll")-1);
    guipath += "treegeneratorGUI.mel";

    // Open the GUI .mel file
    std::ifstream gui(guipath.asChar());
    if(!gui.is_open())
    {
        MGlobal::executeCommand("error \"" + guipath + " could not open\"");
        return MStatus::kFailure;
    }

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

MStatus TreeGenerator::doIt(const MArgList& args)
{
    MStatus stat;
    MArgDatabase argData(syntax(), args, &stat);
    if(!stat)
    { 
        return stat; 
    }

    //Create the default data
    unsigned int iterations = 4;
    BranchData trunk(1.0, 5.0, 0.2, 22.2, 5.0, 0.9);
    BranchData branch(1.0, 15.0, 0.5, 22.2, 5.0, 0.95);

    //Create the default rules
    unsigned int rulep[sm_ruleNumber];
    MString rules[sm_ruleNumber];
    MString rulec[sm_ruleNumber];
    m_treedata.rule = "";
    MString prerule = "FGGFGGFGGF";
    MString postrule = "";
    MString start = "A";
    rulec[0] = "A";
    rules[0] = "[>FGLLLFGLLLFLLLA]^^^^^[>FGLLLFGLLLFLLLA]^^^^^^^[>FGLLLFGLLLFLLLA]";
    rulep[0] = 100;

    for(int i = 1; i < sm_ruleNumber; ++i)
    {
        rulep[i] = 0;
        rules[i] = "";
        rulec[i] = "";
    }

    GetFlagArguments(argData, prerule, postrule, start, 
        iterations, rules, rulec, rulep, branch, trunk);

    //set preview variables
    if(m_meshdata.preview)
    {
        m_meshdata.createAsCurves = true;
        m_leafdata.treeHasLeaves = false;
        m_fxdata.createTreeShader = false;
        m_fxdata.createLeafShader = false;
    }

    //Generate a new seed if randomize chosen
    if(m_meshdata.randomize) 
    {
        sm_treeSeed = static_cast<unsigned int>(time(0));
    }
    srand(sm_treeSeed);

    //Progress window setup
    int progress = 2;
    if(m_leafdata.treeHasLeaves) 
    { 
        ++progress; 
    }
    StartProgressWindow(progress);
    m_progressStepAmount = 2;

    //Create the rule string
    m_treedata.rule = start.asChar();
    if(!CreateRuleString(iterations, rules, rulec, rulep)) 
    { 
        EndProgressWindow(); 
        return MStatus::kFailure; 
    }

    //Add prerule/postrule
    m_treedata.rule = prerule.asChar() + m_treedata.rule;
    m_treedata.rule += postrule.asChar();

    //Navigate the turtle
    if(!BuildTheTree(branch, trunk)) 
    { 
        EndProgressWindow(); 
        return MStatus::kFailure; 
    }

    //Create the mesh
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

    //Set up progress
    DescribeProgressWindow("Building:");
    unsigned int progressMod = (m_treedata.rule.size()/
        m_progressIncreaseAmount)*m_progressStepAmount;

    //Create the turtle
    Turtle turtle;
    turtle.world.RotateXLocal(static_cast<float>(DegToRad(90.0f)));
    BranchData* values = &trunk;
    turtle.radius = m_treedata.initialRadius;
    turtle.branchIndex = 0;
    turtle.sectionIndex = 0;
    turtle.layerIndex = 0;
    turtle.branchParent = -1;
    turtle.branchEnded = false;
    deque<Turtle> stack;

    //Set up variables
    Float3 result;
    int TrunkIndex = 0;
    char current;
    double a = 0;

    //Set up tree
    m_branches.push_back(Branch());
    m_branches[TrunkIndex].layer = 0;
    m_branches[TrunkIndex].parentIndex = -1;
    m_branches[TrunkIndex].sections.push_back(Section(
        0,0,0,static_cast<float>(m_treedata.initialRadius)));

    //navigate the turtle
    for(unsigned int j = 0, c = 0; j < m_treedata.rule.size(); ++j, ++c)
    {
        current = m_treedata.rule[j];
        switch(current)
        {
            case 'F':
            {
                //move forward with drawing 
                DetermineForwardMovement(&turtle, result, values->forward, 
                    values->forwardAngle, values->forwardVariance);
                turtle.world.Translate(result);
                
                //change radius
                turtle.radius *= values->radiusDecrease;
                if(turtle.radius < m_treedata.minimumRadius)
                { 
                    turtle.radius  = m_treedata.minimumRadius; 
                }

                //add section to branch
                m_branches[turtle.branchIndex].sections.push_back(
                    Section(turtle.world.Position(), static_cast<float>(turtle.radius)));
                turtle.sectionIndex++;
                break;
            }
            case 'G':
            {
                //move forward without drawing
                DetermineForwardMovement(&turtle, result, values->forward, 
                    values->forwardAngle, values->forwardVariance);
                turtle.world.Translate(result);
                break;
            }
            case '[':
            {
                //push current tutle onto the stack
                if(BranchIsAlive(j))
                {
                    turtle.branchEnded = true;
                    stack.push_back(Turtle(turtle));
                    BuildNewBranch(turtle,TrunkIndex,values,branch);
                }
                break;
            }
            case ']':
            {
                //pop back tutle from stack
                if(stack.size() > 0)
                {
                    turtle = stack[stack.size()-1];
                    stack.pop_back();
                    (turtle.branchIndex == TrunkIndex) ? values = &trunk : values = &branch;
                }
                break;
            }
            case '+':
            {
                //rotate positive Y
                a = (double(rand()%201)/100.0f)-1.0f;
                turtle.world.RotateYLocal(static_cast<float>(
                    DegToRad(values->angle+(values->angleVariance*a))));
                break;
            }
            case '-':
            {
                //rotate negative y
                a = (double(rand()%201)/100.0f)-1.0f;
                turtle.world.RotateYLocal(static_cast<float>(
                    DegToRad(-values->angle+(values->angleVariance*a))));
                break;
            }
            case '>':
            {
                //rotate positive x
                a = (double(rand()%201)/100.0f)-1.0f;
                turtle.world.RotateXLocal(static_cast<float>(
                    DegToRad(values->angle+(values->angleVariance*a))));
                break;
            }
            case '<':
            {
                //rotate negative x
                a = (double(rand()%201)/100.0f)-1.0f;
                turtle.world.RotateXLocal(static_cast<float>(
                    DegToRad(-values->angle+(values->angleVariance*a))));
                break;
            }
            case '^':
            {
                //rotate positive z
                a = (double(rand()%201)/100.0f)-1.0f;
                turtle.world.RotateZLocal(static_cast<float>(
                    DegToRad(values->angle+(values->angleVariance*a))));
                break;
            }
            case 'v':
            {
                //rotate negative z
                a = (double(rand()%201)/100.0f)-1.0f;
                turtle.world.RotateZLocal(static_cast<float>(
                    DegToRad(-values->angle+(values->angleVariance*a))));
                break;
            }
            case 'L':
            {
                //Create a leaf
                if(m_leafdata.treeHasLeaves && (turtle.branchIndex != TrunkIndex)
                    && (turtle.layerIndex >= static_cast<int>(m_leafdata.leafLayer)) 
                    && (turtle.sectionIndex != 0))
                {
                    Float3 axis = m_branches[turtle.branchIndex].sections[turtle.sectionIndex].position 
                        - m_branches[turtle.branchIndex].sections[turtle.sectionIndex-1].position;

                    axis.Normalize();
                    m_leaves.push_back(Leaf(turtle.world.Position(),
                        axis,turtle.layerIndex,(float)turtle.radius));
                }
                break;
            }
        }
        //Increase progress window
        if(c >= progressMod) 
        { 
            c = 0; 
            AdvanceProgressWindow(m_progressStepAmount);
        }

        //Check plugin is continuing
        if(PluginIsCancelled())
        { 
            return false; 
        }
    }
    return true;
}

bool TreeGenerator::CreateRuleString(int iterations, MString rules[], MString rulec[], unsigned rulep[])
{
    string temprule = "";
    int k; 
    char c;
    for(int i = 0; i < iterations; ++i)
    {
        //for each letter in the string
        for(unsigned int j = 0; j < m_treedata.rule.size(); ++j)
        {
            //test against each possible symbol
            c = m_treedata.rule[j];
            for(k = 0; k < sm_ruleNumber; ++k)
            {
                if(c == (rulec[k].asChar())[0])
                {
                    if(rulep[k] == 100)
                    {
                        temprule += rules[k].asChar();
                    }
                    else if(rulep[k] == 0)     
                    {
                        break;
                    }
                    else
                    {
                        unsigned prob = rand()%101;
                        if(prob <= rulep[k])
                        {
                            temprule += rules[k].asChar();
                        }
                    }
                    break;
                }
            }
            //no rule found, leave in string
            if(k == sm_ruleNumber)
            {
                temprule += c;
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

bool TreeGenerator::BranchIsAlive(unsigned int& index)
{
    //check prob of branch dying
    unsigned int prob = rand()%101;
    if(prob < m_treedata.branchDeathProbability)
    {
        //remove all symbols up until corresponding ]
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
        return false;
    }
    return true;
}

void TreeGenerator::BuildNewBranch(Turtle& turtle, int TrunkIndex, BranchData* values, BranchData& trunk)
{
    //change values used if moving from trunk to branch
    if(turtle.branchIndex == TrunkIndex)
    {
        values = &trunk;
    }

    //change radius
    turtle.radius *= m_treedata.branchRadiusDecrease;
    if(turtle.radius < m_treedata.minimumRadius)
    { 
        turtle.radius = m_treedata.minimumRadius;
    }
    
    //start a new branch
    turtle.layerIndex++;
    m_meshdata.maxLayers = max(m_meshdata.maxLayers,turtle.layerIndex); 
    turtle.branchParent = turtle.branchIndex;
    turtle.branchIndex = m_branches.size();
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
    //Turn off history
    MString hResult = MGlobal::executeCommandStringResult(
        MString("constructionHistory -q -tgl"));
    TurnOffHistory();

    //create tree group
    CreateTreeGroup();

    //create shaders
    CreateShaders();

    //Check okay to continue
    if(PluginIsCancelled()) 
    { 
        DeleteNodes(); 
        TurnOnHistory(hResult.asInt()); 
        return false; 
    }

    //create meshes
    if(m_meshdata.createAsCurves)
    {
        //create curve tree
        if(!CreateCurves())
        { 
            DeleteNodes(); 
            TurnOnHistory(hResult.asInt()); 
            return false; 
        }
    } 
    else
    {
        //create mesh tree
        if(!CreateMeshes())
        { 
            DeleteNodes(); 
            TurnOnHistory(hResult.asInt()); 
            return false; 
        }
    }
    //leaf the tree
    if(m_leafdata.treeHasLeaves)
    {
        if(!CreateLeaves())
        { 
            DeleteNodes(); 
            TurnOnHistory(hResult.asInt()); 
            return false; 
        }
    }

    //rename all
    sm_dagMod.doIt();

    //finish the tree
    TurnOnHistory(hResult.asInt());
    return true;
}

bool TreeGenerator::CreateTreeGroup()
{
    sm_treeNumber++;
    m_treedata.treename = MString("tf_tree_")+sm_treeNumber;

    MFnTransform transFn;
    m_treedata.tree = transFn.create();
    m_meshdata.maxLayers++;
    for(int i = 0; i < m_meshdata.maxLayers; ++i)
    {
        m_layers.push_back(Layer());
        m_layers[i].layer = transFn.create();
        sm_dagMod.renameNode(m_layers[i].layer,m_treedata.treename+"_Layer"+i);
        sm_dagMod.reparentNode(m_layers[i].layer,m_treedata.tree);
        
        if(m_leafdata.treeHasLeaves)
        {
            m_layers[i].leaves = transFn.create();
            sm_dagMod.renameNode(m_layers[i].leaves,m_treedata.treename+"_Layer"+i+"_Leaves");
            sm_dagMod.reparentNode(m_layers[i].leaves,m_layers[i].layer);
        }
        if(!m_meshdata.createAsCurves)
        {
            m_layers[i].branches = transFn.create();
            sm_dagMod.renameNode(m_layers[i].branches,m_treedata.treename+"_Layer"+i+"_Branches");
            sm_dagMod.reparentNode(m_layers[i].branches,m_layers[i].layer);
        }
    }
    sm_dagMod.renameNode(m_treedata.tree,m_treedata.treename);
    
    if(PluginIsCancelled())
    {
        return false;
    }
    return true;
}

bool TreeGenerator::CreateCurves()
{
    DescribeProgressWindow("Meshing:");
    unsigned int progressMod = (m_branches.size()/
        m_progressIncreaseAmount)*m_progressStepAmount; 

    //Create each branch
    for(unsigned int j = 0, c = 0; j < m_branches.size(); ++j, ++c)
    {
        if(m_branches[j].sections.size() > 1)
        {
            CreateCurve(&m_branches[j], (m_treedata.treename+"_B"+j), 
                m_layers[m_branches[j].layer].layer);
        }

        if(c >= progressMod) 
        { 
            c = 0; 
            AdvanceProgressWindow(m_progressStepAmount); 
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
    unsigned int progressMod = (m_branches.size()/
        m_progressIncreaseAmount)*m_progressStepAmount; 

    //Create the disks
    deque<Disk> disk;
    disk.push_back(Disk());
    float angle = 360.0f/m_meshdata.trunkfaces;
    for(unsigned int i = 0; i < m_meshdata.trunkfaces; ++i)
    {
        disk[0].points.push_back(Float3(
            static_cast<float>(cos(DegToRad(i*angle))), 0, 
            static_cast<float>(sin(DegToRad(i*angle)))));
    }
    for(unsigned int j = 1; j < m_layers.size(); ++j)
    {
        disk.push_back(Disk());
        int facenumber = m_meshdata.branchfaces - (m_meshdata.faceDecrease*j);
        if(facenumber < 3)
        { 
            facenumber = 3; 
        }
        angle = 360.0f/facenumber;
        for(int i = 0; i < facenumber; ++i)
        {
            disk[j].points.push_back(Float3(
                static_cast<float>(cos(DegToRad(i*angle))), 0, 
                static_cast<float>(sin(DegToRad(i*angle)))));
        }
    }

    //Create each branch
    Branch* Parent;
    for(unsigned int j = 0, c = 0; j < m_branches.size(); ++j, ++c)
    {
        if(m_branches[j].sections.size() > 1)
        {
            Parent = nullptr;
            if(m_branches[j].parentIndex >= 0) 
            { 
                Parent = &m_branches[m_branches[j].parentIndex]; 
            }

            CreateMesh(&m_branches[j], Parent, 
                &disk[m_branches[j].layer], 
                (m_treedata.treename+"_BRN"+j), 
                m_layers[m_branches[j].layer].branches);
        }

        // Advance progress bar
        if(c >= progressMod)
        { 
            c = 0; 
            AdvanceProgressWindow(m_progressStepAmount); 
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
    unsigned int progressMod = (m_leaves.size()/
        m_progressIncreaseAmount)*m_progressStepAmount;

    int vertno = 0;
    float bleed = static_cast<float>(m_fxdata.uvBleedSpace);
    if(m_leafdata.bendAmount == 0)
    {
        vertno = 4;
        m_leafpolycounts.append(4);

        //Create indices (face1)
        m_leafindices.append(0);    
        m_leafindices.append(1);    
        m_leafindices.append(3);    
        m_leafindices.append(2);

        //Create the uvs (face1)
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
        m_leafpolycounts.append(4); 
        m_leafpolycounts.append(4);

        //create indices for face 1
        m_leafindices.append(0);    
        m_leafindices.append(1);    
        m_leafindices.append(3);    
        m_leafindices.append(2);

        //create indices for face 2
        m_leafindices.append(2);    
        m_leafindices.append(3);    
        m_leafindices.append(5);    
        m_leafindices.append(4); 

        //Create the uvs
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

        //Face1
        m_leafUVids.append(0);  
        m_leafUVids.append(1);  
        m_leafUVids.append(2);  
        m_leafUVids.append(3);

        //Face2
        m_leafUVids.append(3);  
        m_leafUVids.append(2);  
        m_leafUVids.append(4);  
        m_leafUVids.append(5);
    }

    //create leaves
    for(int i = 0; i < vertno; ++i)
    {
        m_leafverts.push_back(Float3());
    }
    for(unsigned int i = 0, c = 0; i < m_leaves.size(); ++i, ++c)
    {
        CreateLeaf(&m_leaves[i],(m_treedata.treename+"_LVS"+i),
            m_layers[m_leaves[i].layer].leaves);

        //Advance progress bar
        if(c >= progressMod)
        { 
            c = 0; 
            AdvanceProgressWindow(m_progressStepAmount); 
        }
        if(PluginIsCancelled())
        {
            return false;
        }
    }
    return true;
}

void TreeGenerator::CreateLeaf(Leaf* leaf, MString& meshname, MObject& layer)
{
    MFloatPointArray vertices;
    MFnMesh meshfn;

    //Create the verts
    double a = double(rand()%721)-360.0f;

    float w = static_cast<float>(m_leafdata.width + 
        (m_leafdata.widthVariance*((static_cast<double>(
        rand()%201)/100.0f)-1.0f)));

    float h = static_cast<float>(m_leafdata.height + 
        (m_leafdata.heightVariance*((static_cast<double>(
        rand()%201)/100.0f)-1.0f)));

    Matrix mat = Matrix::CreateRotateArbitrary(
        leaf->sectionAxis,static_cast<float>(DegToRad(a)));

    if(m_leafdata.bendAmount != 0)
    {
        m_leafverts[0].Set(-(w/2),0,0);
        m_leafverts[1].Set((w/2),0,0);

        m_leafverts[2].Set(-(w/2),static_cast<float>(m_leafdata.bendAmount
            *((static_cast<double>(rand()%201)/100.0f)-1.0f)),(h/2));

        m_leafverts[3].Set((w/2),static_cast<float>(m_leafdata.bendAmount
            *((static_cast<double>(rand()%201)/100.0f)-1.0f)),(h/2));

        m_leafverts[4].Set(-(w/2),0,h);
        m_leafverts[5].Set((w/2),0,h);
    }
    else
    {
        m_leafverts[0].Set(-(w/2),0,0);
        m_leafverts[1].Set((w/2),0,0);
        m_leafverts[2].Set(-(w/2),0,h);
        m_leafverts[3].Set((w/2),0,h);
    }

    //rotate verts
    for(unsigned int i = 0; i < m_leafverts.size(); ++i)
    {
        m_leafverts[i] *= mat;  
    }

    //move verts roughly outside branch
    Float3 extra = m_leafverts[3]-m_leafverts[1];
    extra = (leaf->sectionAxis.Cross(extra)).Cross(leaf->sectionAxis);
    extra.Normalize();
    extra *= (leaf->sectionRadius/2.0f);
    leaf->position += extra;

    //save verts
    for(unsigned int i = 0; i < m_leafverts.size(); ++i)    
    {
        vertices.append(leaf->position.x+m_leafverts[i].x, 
            leaf->position.y+m_leafverts[i].y, leaf->position.z+m_leafverts[i].z);
    }

    //Create the mesh
    leaf->mesh = meshfn.create(vertices.length(), m_leafpolycounts.length(), 
        vertices, m_leafpolycounts, m_leafindices, m_leafU, m_leafV);

    meshfn.assignUVs(m_leafpolycounts, m_leafUVids); 
    sm_dagMod.renameNode(leaf->mesh,meshname);
    sm_dagMod.reparentNode(leaf->mesh,layer);

    //Shade the mesh
    if(m_fxdata.createLeafShader)
    {
        MGlobal::executeCommand("sets -e -fe " + 
            m_leafdata.leafshadername + "SG " + meshfn.name());
    }
    else
    {
        MGlobal::executeCommand("sets -e -fe initialShadingGroup " + meshfn.name());
    }
}

void TreeGenerator::CreateMesh(Branch* branch, Branch* parent, 
    Disk* disk, MString& meshname, MObject& layer)
{
    MFloatPointArray vertices;
    MIntArray polycounts;
    MIntArray indices;
    MIntArray uvIDs;
    MFnMesh meshfn;
    MFloatArray u;
    MFloatArray v;

    int facenumber = static_cast<int>(disk->points.size());
    int sectionnumber = static_cast<int>(branch->sections.size());

    int pastindex = 0;
    int index = 0;
    int sIndex = 0;
    int sPastindex = 0;
    int uvPastindex = 0;
    int uvIndex = 0;
    int uvringnumber = facenumber+1;
    float bleed = (float)m_fxdata.uvBleedSpace;

    //INITIAL BRANCH RING
    //Get matrices for initial ring
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
        Float3 p = disk->points[j];
        p *= branch->scaleMat;
        p *= branch->rotationMat;
        p += branch->sections[0].position; 
        vertices.append(p.x,p.y,p.z);
        u.append(ChangeRange(static_cast<float>(j),0.0f,
            static_cast<float>(facenumber),bleed,(1.0f-bleed)));
        v.append(bleed);
    }
    u.append(1.0f-bleed);
    v.append(bleed);

    //other branch rings
    for(int i = 1; i < sectionnumber; ++i)
    {
        sIndex = i*facenumber;
        sPastindex = (i-1)*facenumber;
        uvIndex = i*uvringnumber;
        uvPastindex = (i-1)*uvringnumber;

        //create scale matrix
        Section* S = &branch->sections[i];
        branch->scaleMat.MakeIdentity();
        branch->scaleMat.Scale(S->radius);

        //create rotation matrix
        branch->rotationMat.MakeIdentity();
        if(i == sectionnumber-1)
        {
            //rotate in direction of past axis
            Float3 up(0,1.0f,0);
            Float3 pastaxis = branch->sections[i].position-branch->sections[i-1].position;
            Float3 rotaxis = pastaxis.Cross(up);
            rotaxis.Normalize();
            float angle = up.Angle(pastaxis);
            branch->rotationMat = Matrix::CreateRotateArbitrary(rotaxis,angle);
        }
        else
        {
            //rotate half way between past/future
            Float3 up(0,1.0f,0);
            Float3 axis = (branch->sections[i].position-branch->sections[i-1].position) 
                + (branch->sections[i+1].position-branch->sections[i].position); //past axis+future axis

            Float3 rotaxis = axis.Cross(up);
            rotaxis.Normalize();
            float angle = up.Angle(axis);
            branch->rotationMat = Matrix::CreateRotateArbitrary(rotaxis,angle);
        }

        //find v coordinate
        float vcoordinate = ChangeRange(static_cast<float>(i),
            0.0f,static_cast<float>(sectionnumber-1),bleed,1.0f-bleed);

        //for each vertex/face
        for(int j = 0; j < facenumber; ++j)
        {
            //create vertex
            Float3 p = disk->points[j];
            p *= branch->scaleMat; //scale
            p *= branch->rotationMat; //rotate
            p += S->position; //translate
            vertices.append(p.x,p.y,p.z);
            u.append(u[j]);
            v.append(vcoordinate);

            //create faces
            polycounts.append(4);
            index = sIndex+j;
            pastindex = sPastindex+j;
            indices.append(index);  
            indices.append(((index+1) == (sIndex+facenumber)) ? sIndex : (index+1));
            indices.append(((pastindex+1) == (sPastindex+facenumber)) ? sPastindex : (pastindex+1));
            indices.append(pastindex);

            //create uvids
            uvIDs.append(uvIndex+j);
            uvIDs.append(uvIndex+j+1);
            uvIDs.append(uvPastindex+j+1);
            uvIDs.append(uvPastindex+j);
        }
        u.append(1.0f-bleed);
        v.append(vcoordinate);
    }

    //CAP THE END OF THE BRANCH
    if((branch->children.size() == 0) && m_meshdata.capEnds)
    {
        //create middle vert
        Float3 middle = branch->sections[branch->sections.size()-1].position;
        vertices.append(middle.x, middle.y, middle.z);

        //create middle uvs
        Float3 middlepos(0.5,0,0.5);
        u.append(middlepos.x);
        v.append(middlepos.z);
        int middleuv = u.length()-1;
        int startuv = u.length();

        int index1, index2;
        int topindex = vertices.length()-2;
        int midindex = vertices.length()-1;
        int topj = facenumber-1;
        Matrix capscale;
        capscale.Scale(0.25f);
        Float3 p1;

        //note, this goes backwards
        for(int j = 0; j < facenumber; ++j)
        {
            //create faces
            polycounts.append(3);
            index1 = topindex-j;
            index2 = (j == topj) ? topindex : index1-1;
            indices.append(index2);
            indices.append(midindex);
            indices.append(index1);

            //create uvs
            p1 = (disk->points[topj-j]);
            p1 *= capscale;
            u.append(p1.x + middlepos.x); 
            v.append(p1.z + middlepos.z);
            uvIDs.append(startuv+j);
            uvIDs.append(middleuv);
            uvIDs.append((j == topj) ? startuv : startuv+j+1);
        }
    }

    //Create the mesh
    branch->vertNumber = vertices.length();
    branch->mesh = meshfn.create(vertices.length(), 
        polycounts.length(), vertices, polycounts, indices, u, v);

    meshfn.assignUVs(polycounts, uvIDs);
    sm_dagMod.renameNode(branch->mesh, meshname);
    sm_dagMod.reparentNode(branch->mesh, layer);

    //Shade the mesh
    if(m_fxdata.createTreeShader)
    {
        MGlobal::executeCommand("sets -e -fe " + 
            m_treedata.treeshadername + "SG " + meshfn.name());
    }
    else
    {
        MGlobal::executeCommand("sets -e -fe initialShadingGroup " + meshfn.name());
    }
}

void TreeGenerator::CreateCurve(Branch* branch, MString& meshname, MObject& layer)
{
    MPointArray editPoints;
    MFnNurbsCurve curveFn;
    Float3 p;

    for(unsigned int i = 0; i < branch->sections.size(); ++i)
    {
        p = branch->sections[i].position;
        editPoints.append(p.x, p.y, p.z);
    }

    branch->mesh = curveFn.createWithEditPoints(editPoints,
        1,MFnNurbsCurve::kOpen,false,true,true);

    sm_dagMod.renameNode(branch->mesh,meshname);
    sm_dagMod.reparentNode(branch->mesh,layer);
}

bool TreeGenerator::CreateShaders()
{
    if(m_fxdata.createTreeShader)
    {
        //Create branch shader
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

        //Set branch shader attributes
        MString lightcolor = MString("-type double3 ") + m_fxdata.lightcolorR + 
            " " + m_fxdata.lightcolorG + " " + m_fxdata.lightcolorB;

        MString darkcolor = MString("-type double3 ") + m_fxdata.darkcolorR + 
            " " + m_fxdata.darkcolorG + " " + m_fxdata.darkcolorB;

        MGlobal::executeCommand("setAttr " + texnoise + ".colorGain " + lightcolor);
        MGlobal::executeCommand("setAttr " + texnoise + ".colorOffset " + darkcolor);
        MGlobal::executeCommand("setAttr " + texnoise + ".noiseType 3");
        MGlobal::executeCommand("setAttr " + texnoise + ".alphaIsLuminance true");
        MGlobal::executeCommand("setAttr " + texnoise + ".frequencyRatio 0.5");

        //Create bump
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
    //Create leaf shader
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
    sm_dagMod.deleteNode(m_treedata.tree);
    sm_dagMod.doIt();
}

void TreeGenerator::DetermineForwardMovement(Turtle* turtle, Float3& result, 
    double forward, double ang, double variation)
{
    //Get number between -1 and 1
    double x = static_cast<double>(((rand()%201)/100.0f)-1.0f);
    double y = static_cast<double>(((rand()%201)/100.0f)-1.0f);
    double z = static_cast<double>(((rand()%201)/100.0f)-1.0f);
    float l = static_cast<float>(((rand()%201)/100.0f)-1.0f);

    //determine direction, axis must be normalized
    result = turtle->world.Forward();
    result *= Matrix::CreateRotateY(static_cast<float>(DegToRad(ang*y)));
    result *= Matrix::CreateRotateX(static_cast<float>(DegToRad(ang*x)));
    result *= Matrix::CreateRotateZ(static_cast<float>(DegToRad(ang*z)));

    //determine forward amount
    result *= static_cast<float>(forward + (variation*l));
}

void TreeGenerator::StartProgressWindow(int stepNumber)
{
    m_progressIncreaseAmount = 100/stepNumber;

    //Initialise the progress window
    if(!MProgressWindow::reserve())
    {
        EndProgressWindow();
        MProgressWindow::reserve();
    }
    MProgressWindow::setTitle(MString("Progress"));
    MProgressWindow::setProgressRange(0,100);
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

void TreeGenerator::DescribeProgressWindow(char* Description)
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

void TreeGenerator::TurnOnHistory(int turnon)
{ 
    if(turnon == 1)
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

void TreeGenerator::GetFlagArguments(const MArgDatabase& argData, MString prerule, 
    MString postrule, MString start, unsigned int& iterations, MString* rules, 
    MString* rulec, unsigned int* rulep, BranchData& branch, BranchData& trunk)
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
        argData.getFlagArgument("-i", 0, iterations);
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

        const int HALF_MAX_ROWS = 5;
        for(int i = 0; i < HALF_MAX_ROWS; ++i)
        {
            argData.getFlagArgument("-r1",i,rules[i]);    
            argData.getFlagArgument("-rc1",i,rulec[i]);  
            argData.getFlagArgument("-rp1",i,rulep[i]);
            argData.getFlagArgument("-r2",i,rules[HALF_MAX_ROWS+i]);  
            argData.getFlagArgument("-rc2",i,rulec[HALF_MAX_ROWS+i]); 
            argData.getFlagArgument("-rp2",i,rulep[HALF_MAX_ROWS+i]);
        }
    }
}