#ifndef _SHUTDOWNMANAGER_
#define _SHUTDOWNMANAGER_

int sm_isShutdown();
void sm_startShutdown();
void sm_waitForShutdownOnMainThread();

#endif