
/*
==================================================
	lib/cstd.cpp

	C++ runtime library routines
==================================================.
*/

#include <stdint.h>

/*
===========================
	Dynamic initializer sections
===========================
*/

// Function pointer typedef for less typing //
typedef void (__cdecl *_PVFV)(void);

// C initializers

typedef void(__cdecl* proc_t)(void);
typedef int(__cdecl* func_t)(void);

#pragma data_seg(".CRT$XIA")
func_t __xi_a[] = { 0 };

#pragma data_seg(".CRT$XIZ")
func_t __xi_z[] = { 0 };

// C++ initializers

#pragma data_seg(".CRT$XCA")
proc_t __xc_a[] = { 0 };

#pragma data_seg(".CRT$XCZ")
proc_t __xc_z[] = { 0 };

// C pre-terminators

#pragma data_seg(".CRT$XPA")
proc_t __xp_a[] = { 0 };

#pragma data_seg(".CRT$XPZ")
proc_t __xp_z[] = { 0 };

// C terminators

#pragma data_seg(".CRT$XTA")
proc_t __xt_a[] = { 0 };

#pragma data_seg(".CRT$XTZ")
proc_t __xt_z[] = { 0 };

#pragma data_seg()  // reset

#if _MSC_FULL_VER >= 140050214
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#pragma comment(linker, "/merge:.CRT=.data")
#endif

/*
===========================
	Globals
===========================
*/

// function pointer table to global deinitializer table //
static _PVFV * pf_atexitlist = 0;

// Maximum entries allowed in table //
static unsigned max_atexitlist_entries = 32;

// Current amount of entries in table //
static unsigned cur_atexitlist_entries = 0;

/*
===========================
	Initialize global initializaters (Global constructs, et al)
===========================
*/
static void __cdecl _initterm ( _PVFV * pfbegin,    _PVFV * pfend )
{
	// Go through each initializer
    while ( pfbegin < pfend )
    {
	  // Execute the global initializer
		if (*pfbegin != 0)
		{
			(**pfbegin) ();
		}
	    // Go to next initializer inside the initializer table
        ++pfbegin;
    }
}

/*
===================================
	Initialize the deinitializer function ptr table
===================================
*/
char runtimeTempBuffer[5000];
void __cdecl _atexit_init(void)
{
    max_atexitlist_entries = 32;

	// Warning: Normally, the STDC will dynamically allocate this. Because we have no memory manager, just choose
	// a base address that you will never use for now
 //   pf_atexitlist = (_PVFV *)0x500000;

	pf_atexitlist = (_PVFV *)runtimeTempBuffer;
}

/*
===================================
	Add entry into atexit deinitialzer table. Called by MSVC++ code.
===================================
*/
int __cdecl atexit(_PVFV fn)
{
	// Insure we have enough free space
	if (cur_atexitlist_entries>=max_atexitlist_entries)
		return 1;
	else {

		// Add the exit routine
		*(pf_atexitlist++) = fn;
		cur_atexitlist_entries++;
	}

	return 0;
}

/*
===================================
	Shutdown the CRT, and execute all global dtors.
===================================
*/
void _cdecl Exit () {

	// Go through the list, and execute all global exit routines
	while (cur_atexitlist_entries--) {

			// execute function
			(*(--pf_atexitlist)) ();
	}
}

/*
===================================
	Executes all global dynamic initializers
===================================


*/
static int initcrt();
void _cdecl InitCRT()
{
	_atexit_init();
	_initterm(__xc_a, __xc_z);
	//initcrt();

	
}

/*
===================================
	MSVC++ calls this routine if a pure virtual function is called
===================================
*/
int __cdecl _purecall_handler()
{
	// for now, halt the system 'til we have a way to display error
	for (;;);

	// print error message here
}


static void initterm(proc_t* begin, proc_t* end) {
	while (begin < end) {
		if (*begin != 0) (**begin)();
		++begin;
	}
}



static int inittermi(func_t* begin, func_t* end) {
	int rc = 0;

	while (begin < end && rc == 0) {
		if (*begin != 0) rc = (**begin)();
		++begin;
	}

	return rc;
}

static int initcrt() {
	int rc;


	// Execute C initializers
	rc = inittermi(__xi_a, __xi_z);
	if (rc != 0) return rc;

	// Execute C++ initializers
	initterm(__xc_a, __xc_z);


	return 0;
}