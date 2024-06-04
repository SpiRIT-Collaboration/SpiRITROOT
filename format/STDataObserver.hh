#ifndef STDATAOBSERVER_H
#define STDATAOBSERVER_H

namespace DataHandling {
class STSubject;
/**
 * This is the base class for any object that may observe data from a Subject and reacto on an update.
 *
 * It can register itself with any Subject and of that subject changes this class will be informed.
 * @ingroup DataHandling
 */
class STObserver {
public:
   virtual ~STObserver() = default;
   virtual void Update(STSubject *) = 0;
};
} // namespace DataHandling
#endif
