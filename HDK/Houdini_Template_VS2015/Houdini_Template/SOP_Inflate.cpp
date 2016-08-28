#include "SOP_Inflate.h"

#include <GU/GU_Detail.h>
#include <GA/GA_Handle.h>
#include <OP/OP_AutoLockInputs.h>
#include <OP/OP_Director.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <UT/UT_DSOVersion.h>
#include <SYS/SYS_Math.h>
#include <stddef.h>

void newSopOperator(OP_OperatorTable* Table)
{
	Table->addOperator(new OP_Operator(
		"HDK_Inflate",
		"HDK Inflate",
		SOP_Inflate::myConstructor,
		SOP_Inflate::TemplateList,
		1,
		1,
		0));
}

static PRM_Name Names[] = {
	PRM_Name("scale",     "Scale"),
};

PRM_Template SOP_Inflate::TemplateList[] = {
	PRM_Template(PRM_STRING, 1, &PRMgroupName, 0, &SOP_Node::pointGroupMenu, 0, 0, SOP_Node::getGroupSelectButton(
	GA_GROUP_POINT)),
	PRM_Template(PRM_FLT_J,	1, &Names[0], PRMoneDefaults, 0, &PRMscaleRange),
	PRM_Template(),
};

OP_Node* SOP_Inflate::myConstructor(OP_Network* Network, const char *Name, OP_Operator* Operator)
{
	return new SOP_Inflate(Network, Name, Operator);
}

SOP_Inflate::SOP_Inflate(OP_Network* Network, const char *Name, OP_Operator* Operator)
	: SOP_Node(Network, Name, Operator),
	PointGroup(NULL)
{
	mySopFlags.setManagesDataIDs(true);
}

SOP_Inflate::~SOP_Inflate()
{
}

OP_ERROR SOP_Inflate::cookInputGroups(OP_Context& Context, int Alone)
{
	// The SOP_Node::cookInputPointGroups() provides a good default
	// implementation for just handling a point selection.
	return cookInputPointGroups(
		Context, // This is needed for cooking the group parameter, and cooking the input if alone.
		PointGroup, // The group (or NULL) is written to PointGroup if not alone.
		Alone,   // This is true iff called outside of cookMySop to update handles.
				 // true means the group will be for the input geometry.
				 // false means the group will be for gdp (the working/output geometry).
		true,    // (default) true means to set the selection to the group if not alone and the highlight flag is on.
		0,       // (default) Parameter index of the group field
		-1,      // (default) Parameter index of the group type field (-1 since there isn't one)
		true,    // (default) true means that a pointer to an existing group is okay; false means group is always new.
		false,   // (default) false means new groups should be unordered; true means new groups should be ordered.
		true,    // (default) true means that all new groups should be detached, so not owned by the detail;
				 //           false means that new point and primitive groups on gdp will be owned by gdp.
		0        // (default) Index of the input whose geometry the group will be made for if alone.
		);
}

OP_ERROR SOP_Inflate::cookMySop(OP_Context& Context)
{
	// We must lock our inputs before we try to access their geometry.
	// OP_AutoLockInputs will automatically unlock our inputs when we return.
	// NOTE: Don't call unlockInputs yourself when using this!
	OP_AutoLockInputs inputs(this);
	if (inputs.lock(Context) >= UT_ERROR_ABORT)
		return error();

	// Duplicate our incoming geometry with the hint that we only
	// altered points.  Thus, if our input was unchanged, we can
	// easily roll back our changes by copying point values.
	duplicatePointSource(0, Context);

	fpreal t = Context.getTime();

	// We evaluate our parameters outside the loop for speed.  If we
	// wanted local variable support, we'd have to do more setup
	// (see SOP_Flatten) and also move these inside the loop.
	float Scale = SCALE(t);

	if (error() >= UT_ERROR_ABORT)
		return error();

	// Here we determine which groups we have to work on.  We only
	// handle point groups.
	if (cookInputGroups(Context) >= UT_ERROR_ABORT)
		return error();

	GA_RWHandleV3 P_handle(gdp->findAttribute(GA_ATTRIB_POINT, "P"));
	GA_RWHandleV3 N_handle(gdp->findAttribute(GA_ATTRIB_POINT, "N"));

	GA_Offset ptoff;
	GA_FOR_ALL_GROUP_PTOFF(gdp, PointGroup, ptoff)
	{
		UT_Vector3 P = P_handle.get(ptoff);
		UT_Vector3 N = N_handle.get(ptoff);

		P = P + (Scale * N);

		P_handle.set(ptoff, P);
	}

	// If we've modified P, and we're managing our own data IDs,
	// we must bump the data ID for P.
	if (!PointGroup || !PointGroup->isEmpty())
		gdp->getP()->bumpDataId();

	return error();
}