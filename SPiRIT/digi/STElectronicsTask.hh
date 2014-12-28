#ifndef STELECTRONICSTASK_H
#define STELECTRONICSTASK_H

#include "FairTask.h"

class TClonesArray;

class STElectronicsTask : public FairTask
{
  public:

    /** Default constructor **/
    STElectronicsTask();

    /** Constructor with parameters (Optional)
    //  STElectronicsTask(Int_t verbose);


    /** Destructor **/
    ~STElectronicsTask();


    /** Initiliazation of task at the beginning of a run **/
    virtual InitStatus Init();

    /** ReInitiliazation of task when the runID changes **/
    virtual InitStatus ReInit();


    /** Executed for each event. **/
    virtual void Exec(Option_t* opt);

    /** Load the parameter container from the runtime database **/
    virtual void SetParContainers();

    /** Finish task called at the end of the run **/
    virtual void Finish();

  private:

    /** Input array from previous already existing data level **/
    //  TClonesArray* <InputDataLevel>;

    /** Output array to  new data level**/
    //  TClonesArray* <OutputDataLevel>;

    STElectronicsTask(const STElectronicsTask&);
    STElectronicsTask operator=(const STElectronicsTask&);

    ClassDef(STElectronicsTask,1);
};

#endif
