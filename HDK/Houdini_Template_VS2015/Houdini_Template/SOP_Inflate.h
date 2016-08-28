#ifndef _SOP_INFLATE_H_
#define _SOP_INFLATE_H_

#include <SOP/SOP_Node.h>

class SOP_Inflate : public SOP_Node
{
public:
	static PRM_Template TemplateList[];

private:
	const GA_PointGroup* PointGroup;

public:
	SOP_Inflate(OP_Network* Network, const char *Name, OP_Operator* Operator);
	virtual ~SOP_Inflate();

	static OP_Node* myConstructor(OP_Network* Network, const char *Name, OP_Operator* Operator);

	virtual OP_ERROR cookInputGroups(OP_Context& Context, int Alone = 0);

protected:
	virtual OP_ERROR cookMySop(OP_Context& Context);

private:
	void getGroups(UT_String& Str) { evalString(Str, "group", 0, 0); }

	fpreal SCALE(fpreal t) { return evalFloat("scale", 0, t); }
};

#endif //_SOP_INFLATE_H