#ifndef SPIN_H_
#define SPIN_H_

//
// spin lock type, unlike a mutex, it's running under user-level
// reference: http://www.gaccob.com/?p=1053
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct spin_lock_t;
struct spin_lock_t* spin_create();
void spin_release(struct spin_lock_t* lock);
void spin_lock(struct spin_lock_t* lock);
int spin_trylock(struct spin_lock_t* lock);
void spin_unlock(struct spin_lock_t* lock);

#ifdef __cplusplus
}
#endif

#endif // SPIN_H_

