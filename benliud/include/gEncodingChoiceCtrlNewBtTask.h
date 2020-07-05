// gEncodingChoiceCtrlNewBtTask.h: interface for the gEncodingChoiceCtrlNewBtTask class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GENCODINGCHOICECTRLNEWBTTASK_H
#define _GENCODINGCHOICECTRLNEWBTTASK_H

#include "gEncodingChoiceCtrl.h"

class gNewBTTaskNotePageBasic;
class gEncodingChoiceCtrlNewBtTask : public gEncodingChoiceCtrl 
{
public:
	gEncodingChoiceCtrlNewBtTask(gNewBTTaskNotePageBasic* parent);
	virtual ~gEncodingChoiceCtrlNewBtTask();
protected:
	virtual void OnEncodingChanged(wxCommandEvent& event);

	
};

#endif 
