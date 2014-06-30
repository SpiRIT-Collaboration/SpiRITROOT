//---------------------------------------------------------------------
// Description:
//      Dummy task class
//
// Author List:
//      Genie Jhang     Korea Univ.       (original author)
//----------------------------------------------------------------------

#ifndef _STDUMMYTASK_H_
#define _STDUMMYTASK_H_

class STdummyTask : public FairTask
{
  public:
    // Constructor and Destructor
    STdummyTask();
    ~STdummyTask();
    
    // Operators
    // Getters
    // Setters
    
    // Main methods
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:

  ClassDef(STdummyTask, 1);
};

#endif
