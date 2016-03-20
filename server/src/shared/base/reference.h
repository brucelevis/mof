//
//  reference.h
//  GameSrv
//
//  Created by prcv on 13-9-10.
//
//

#ifndef shared_reference_h
#define shared_reference_h


class RefCountedObject
/// A base class for objects that employ
/// reference counting based garbage collection.
///
/// Reference-counted objects inhibit construction
/// by copying and assignment.
{
public:
	RefCountedObject();
    /// Creates the RefCountedObject.
    /// The initial reference count is one.
    
	void retain();
    /// Increments the object's reference count.
    
	void release();
    /// Decrements the object's reference count
    /// and deletes the object if the count
    /// reaches zero.
    
	int referenceCount() const;
    /// Returns the reference count.
    
protected:
	virtual ~RefCountedObject();
    /// Destroys the RefCountedObject.
    
private:
	RefCountedObject(const RefCountedObject&);
	RefCountedObject& operator = (const RefCountedObject&);
    
	volatile int mCounter;
};
#endif
