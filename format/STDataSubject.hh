#ifndef STDATASUBJECT_H
#define STDATASUBJECT_H

#include <set>

/**
 @defgroup DataHandling Data Handling
 *
 * Classes for storing information  that may be updated at arbitrary times. All classes in this
 group derive either from Subject or Observer. Subject classes store data and if that data is updated will notify every
 Observer attached to them of the change. The observer can then decide how to respond.

*/
namespace DataHandling {
class STObserver;

/**
 * This is the base class for any object that may contain data to send to Observer on an update.
 *
 * Any Observer may regiter itself with these subjects, and they will be notified when the state of
 * this object changes, then they can retrieve the updated information and act on it.
 * @ingroup DataHandling
 */
class STSubject {
private:
   std::set<STObserver *> fObservers;

protected:
   STSubject() = default;

public:
   virtual ~STSubject() = default;

   /// Attach an observer to get notified when this subject changes
   void Attach(STObserver *observer);
   /// Detach an observer to stop getting notified when this subject changes
   void Detach(STObserver *observer) { fObservers.erase(observer); }
   /// Notify all attached subjects that something changed
   void Notify();
};

/**
 * @brief Subject for any simple type.
 *
 * Assumes `==` and copy asignment operator do something reasonable.
 *
 * @ingroup DataHandling
 */

template <typename T>
class STSimpleType : public STSubject {
protected:
   T fData;

public:
   STSimpleType(T data) { fData = data; } // Must use assigment operator for primitive types.
   T Get() const { return fData; }
   void Set(T data, bool notify = true)
   {
      if (fData == data)
         return;
      fData = data;
      if (notify)
         Notify();
   }
};

} // namespace DataHandling
#endif
