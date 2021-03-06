/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_thread_utils_h__
#define INCLUDE_thread_utils_h__

/* Common operations even if threading has been disabled */
typedef struct {
#if defined(GIT_WIN32)
	volatile long val;
#else
	volatile int val;
#endif
} git_atomic;

#ifdef GIT_ARCH_64

typedef struct {
#if defined(GIT_WIN32)
	__int64 val;
#else
	int64_t val;
#endif
} git_atomic64;

typedef git_atomic64 git_atomic_ssize;

#define git_atomic_ssize_add git_atomic64_add

#else

typedef git_atomic git_atomic_ssize;

#define git_atomic_ssize_add git_atomic_add

#endif

GIT_INLINE(void) git_atomic_set(git_atomic *a, int val)
{
	a->val = val;
}

#ifdef GIT_THREADS

#define git_thread pthread_t
#define git_thread_create(thread, attr, start_routine, arg) pthread_create(thread, attr, start_routine, arg)
#define git_thread_kill(thread) pthread_cancel(thread)
#define git_thread_exit(status)	pthread_exit(status)
#define git_thread_join(id, status) pthread_join(id, status)

/* Pthreads Mutex */
#define git_mutex pthread_mutex_t
#define git_mutex_init(a)	pthread_mutex_init(a, NULL)
#define git_mutex_lock(a)	pthread_mutex_lock(a)
#define git_mutex_unlock(a) pthread_mutex_unlock(a)
#define git_mutex_free(a)	pthread_mutex_destroy(a)

/* Pthreads condition vars */
#define git_cond pthread_cond_t
#define git_cond_init(c)	pthread_cond_init(c, NULL)
#define git_cond_free(c) 	pthread_cond_destroy(c)
#define git_cond_wait(c, l)	pthread_cond_wait(c, l)
#define git_cond_signal(c)	pthread_cond_signal(c)
#define git_cond_broadcast(c)	pthread_cond_broadcast(c)

GIT_INLINE(int) git_atomic_inc(git_atomic *a)
{
#if defined(GIT_WIN32)
	return InterlockedIncrement(&a->val);
#elif defined(__GNUC__)
	return __sync_add_and_fetch(&a->val, 1);
#else
#	error "Unsupported architecture for atomic operations"
#endif
}

GIT_INLINE(int) git_atomic_add(git_atomic *a, int32_t addend)
{
#if defined(GIT_WIN32)
	return InterlockedExchangeAdd(&a->val, addend);
#elif defined(__GNUC__)
	return __sync_add_and_fetch(&a->val, addend);
#else
#	error "Unsupported architecture for atomic operations"
#endif
}

GIT_INLINE(int) git_atomic_dec(git_atomic *a)
{
#if defined(GIT_WIN32)
	return InterlockedDecrement(&a->val);
#elif defined(__GNUC__)
	return __sync_sub_and_fetch(&a->val, 1);
#else
#	error "Unsupported architecture for atomic operations"
#endif
}

GIT_INLINE(void *) git___compare_and_swap(
	volatile void **ptr, void *oldval, void *newval)
{
	volatile void *foundval;
#if defined(GIT_WIN32)
	foundval = InterlockedCompareExchangePointer((volatile PVOID *)ptr, newval, oldval);
#elif defined(__GNUC__)
	foundval = __sync_val_compare_and_swap(ptr, oldval, newval);
#else
#	error "Unsupported architecture for atomic operations"
#endif
	return (foundval == oldval) ? oldval : newval;
}

#ifdef GIT_ARCH_64

GIT_INLINE(int64_t) git_atomic64_add(git_atomic64 *a, int64_t addend)
{
#if defined(GIT_WIN32)
	return InterlockedExchangeAdd64(&a->val, addend);
#elif defined(__GNUC__)
	return __sync_add_and_fetch(&a->val, addend);
#else
#	error "Unsupported architecture for atomic operations"
#endif
}

#endif

#else

#define git_thread unsigned int
#define git_thread_create(thread, attr, start_routine, arg) (void)0
#define git_thread_kill(thread) (void)0
#define git_thread_exit(status) (void)0
#define git_thread_join(id, status) (void)0

/* Pthreads Mutex */
#define git_mutex unsigned int
#define git_mutex_init(a) 0
#define git_mutex_lock(a) 0
#define git_mutex_unlock(a) (void)0
#define git_mutex_free(a) (void)0

/* Pthreads condition vars */
#define git_cond unsigned int
#define git_cond_init(c, a)	(void)0
#define git_cond_free(c) (void)0
#define git_cond_wait(c, l)	(void)0
#define git_cond_signal(c) (void)0
#define git_cond_broadcast(c) (void)0

GIT_INLINE(int) git_atomic_inc(git_atomic *a)
{
	return ++a->val;
}

GIT_INLINE(int) git_atomic_add(git_atomic *a, int32_t addend)
{
	a->val += addend;
	return a->val;
}

GIT_INLINE(int) git_atomic_dec(git_atomic *a)
{
	return --a->val;
}

GIT_INLINE(void *) git___compare_and_swap(
	volatile void **ptr, void *oldval, void *newval)
{
	if (*ptr == oldval)
		*ptr = newval;
	else
		oldval = newval;
	return oldval;
}

#ifdef GIT_ARCH_64

GIT_INLINE(int64_t) git_atomic64_add(git_atomic64 *a, int64_t addend)
{
	a->val += addend;
	return a->val;
}

#endif

#endif

/* Atomically replace oldval with newval
 * @return oldval if it was replaced or newval if it was not
 */
#define git__compare_and_swap(P,O,N) \
	git___compare_and_swap((volatile void **)P, O, N)

#define git__swap(ptr, val) git__compare_and_swap(&ptr, ptr, val)

extern int git_online_cpus(void);

#if defined(GIT_THREADS) && defined(GIT_WIN32)
# define GIT_MEMORY_BARRIER MemoryBarrier()
#elif defined(GIT_THREADS)
# define GIT_MEMORY_BARRIER __sync_synchronize()
#else
# define GIT_MEMORY_BARRIER /* noop */
#endif

#endif /* INCLUDE_thread_utils_h__ */
