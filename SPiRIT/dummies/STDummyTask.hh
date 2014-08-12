//---------------------------------------------------------------------
// Description:
//      Dummy task class
//
// Author List:
//      Genie Jhang     Korea Univ.       (original author)
//----------------------------------------------------------------------

#ifndef _STDUMMYTASK_H_
#define _STDUMMYTASK_H_

class STDummyTask : public FairTask
{
  public:
    // Constructor and Destructor
    STDummyTask();
    ~STDummyTask();
    
    // Operators
    // Getters
    // Setters
    
    // Main methods
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:

  ClassDef(STDummyTask, 1);
};

#endif
