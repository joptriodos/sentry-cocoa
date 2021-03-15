#include "SentryHook.h"
#include "fishhook.h"
#include <dispatch/dispatch.h>
#include <execinfo.h>
#include <pthread.h>

/**
 * This is a poor-mans concurrent hashtable.
 * We have N slots, using modulo of the thread ID. Using atomic load / compare-exchange to make sure that the slot indeed
 * belongs to the thread we want to work with.
 */

#define SENTRY_MAX_ASYNC_THREADS 32

typedef struct {
    SentryCrashThread thread;
    sentry_async_backtrace_t *backtrace;
} sentry_async_caller_t;

static sentry_async_caller_t sentry_async_callers[SENTRY_MAX_ASYNC_THREADS] = {0};

sentry_async_backtrace_t* sentry_get_async_caller_for_thread(SentryCrashThread thread) {
    size_t idx = thread % SENTRY_MAX_ASYNC_THREADS;
    sentry_async_caller_t *caller = &sentry_async_callers[idx];
    if (__atomic_load_n(&caller->thread, __ATOMIC_SEQ_CST) == thread) {
        sentry_async_backtrace_t* backtrace = __atomic_load_n(&caller->backtrace, __ATOMIC_SEQ_CST);
        // we read the thread id *again*, if it is still the same, the backtrace pointer we
        // read in between is valid
        if (__atomic_load_n(&caller->thread, __ATOMIC_SEQ_CST) == thread) {
            return backtrace;
        }
    }
    return NULL;
}

static bool sentry__set_async_caller_for_thread(SentryCrashThread old_thread, SentryCrashThread new_thread, sentry_async_backtrace_t *backtrace) {
    size_t idx = new_thread % SENTRY_MAX_ASYNC_THREADS;
    sentry_async_caller_t *caller = &sentry_async_callers[idx];
    
    SentryCrashThread expected = old_thread;
    bool success = __atomic_compare_exchange_n(&caller->thread, &expected, new_thread, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    if (success) {
        __atomic_store_n(&caller->backtrace, backtrace, __ATOMIC_SEQ_CST);
    }
    
    return success;
}

void sentry__async_backtrace_incref(sentry_async_backtrace_t* bt) {
    if (!bt) {
        return;
    }
    __atomic_fetch_add(&bt->refcount, 1, __ATOMIC_SEQ_CST);
}

void sentry__async_backtrace_decref(sentry_async_backtrace_t* bt) {
    if (!bt) {
        return;
    }
    if (__atomic_fetch_add(&bt->refcount, -1, __ATOMIC_SEQ_CST) == 1) {
        sentry__async_backtrace_decref(bt->async_caller);
        free(bt);
    }
}

static void
(*real_dispatch_async)(dispatch_queue_t queue, dispatch_block_t block);

sentry_async_backtrace_t* sentry__async_backtrace_capture(void) {
    sentry_async_backtrace_t *bt = malloc(sizeof(sentry_async_backtrace_t));
    bt->refcount = 1;
    
    bt->len = backtrace(bt->backtrace, MAX_BACKTRACE_FRAMES);
    
    sentry_async_backtrace_t *caller = sentry_get_async_caller_for_thread(sentrycrashthread_self());
    sentry__async_backtrace_incref(caller);
    bt->async_caller = caller;
    
    return bt;
}

void sentry__hook_dispatch_async(dispatch_queue_t queue, dispatch_block_t block) {
    // create a backtrace, capturing the async callsite
    sentry_async_backtrace_t *bt = sentry__async_backtrace_capture();
    
    return real_dispatch_async(queue, ^{
        SentryCrashThread thread = sentrycrashthread_self();
        
        // inside the async context, save the backtrace in a thread local for later consumption
        sentry__set_async_caller_for_thread((SentryCrashThread)NULL, thread, bt);

        // call through to the original block
        block();
        
        // and decref our current backtrace
        sentry__set_async_caller_for_thread(thread, (SentryCrashThread)NULL, NULL);
        sentry__async_backtrace_decref(bt);
    });
}

void sentry_install_async_hooks(void)
{
    rebind_symbols((struct rebinding[1]){
        {"dispatch_async", sentry__hook_dispatch_async, (void *)&real_dispatch_async},
    }, 1);
    // TODO:
    //dispatch_async_f
    //dispatch_after
    //dispatch_after_f
}
