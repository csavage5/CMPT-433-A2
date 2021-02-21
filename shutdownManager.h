#ifndef _SHUTDOWNMANAGER_
#define _SHUTDOWNMANAGER_

// Check for shutdown command
int sm_isShutdown();

// Start shutdown process
void sm_startShutdown();

// Wait on main for shutdown command
void sm_waitForShutdownOnMainThread();

#endif