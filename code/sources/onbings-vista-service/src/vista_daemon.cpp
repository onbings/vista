/*
 * Copyright (c) 2024-2044, OnBings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module implement the VISTA daemon: Vision Intelligence, Statistics, and Tracking Analysis
 *
 * Author:      Bernard HARMEL: b.harmel@gmail.com
 *
 * History:
 * V 1.00  September 25 2026  BHA : Initial release
 */
#include "vista_daemon.h"
#if defined(VISTA_USE_SYSTEMD_NOTIFY)
#include <systemd/sd-daemon.h>
#endif
#include <thread>

VistaDaemon::VistaDaemon(const VISTA::VISTA_ENGINE_PARAM &_rVistaEngineParam_X) : mVistaEngineParam_X(_rVistaEngineParam_X)
{
}

VistaDaemon::~VistaDaemon()
{
  LOG_INFO(mVistaEngineParam_X.psILogger, "Delete Vista Daemon");
  Stop();
}
int VistaDaemon::VistaEngineProcessingLoop()
{
  return 0;
}
bool VistaDaemon::Start()
{
  bool Rts_B = false;
#if defined(VISTA_USE_SYSTEMD_NOTIFY)
  auto LastKeepAliveTime = std::chrono::steady_clock::now();
  const auto KeepAliveInterval = std::chrono::seconds(30);
#endif

  if (!mIsDaemonRunning)
  {
    mpsVistaEngine = VISTA::VistaEngine::S_VistaEngineFactory(mVistaEngineParam_X);
    if (mpsVistaEngine->StartEngine() == 0)
    {
      Rts_B = true;
      mIsDaemonRunning = true;
#if defined(VISTA_USE_SYSTEMD_NOTIFY)
      // Notify systemd that we're ready (requires Type=notify in service file)
      sd_notify(0, "READY=1");
      LOG_INFO(mVistaEngineParam_X.psILogger, "Notified systemd that services are ready");
#endif

      // Main daemon loop
      while ((mIsDaemonRunning) && !(mIsShutdownRequested) && !(mIsRestartRequested))
      {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // A client can request a full application restart (re-exec) through the web API; the
        // MmEngine holds the flag. Poll it here so the loop exits and main() re-execs the process.
        if (mpsVistaEngine->IsRestartRequested())
        {
          mIsRestartRequested = true;
        }

#if defined(VISTA_USE_SYSTEMD_NOTIFY)
        // Send systemd watchdog keepalive (requires WatchdogSec= in service file)
        auto Now = std::chrono::steady_clock::now();
        if ((Now - LastKeepAliveTime) >= KeepAliveInterval)
        {
          sd_notify(0, "WATCHDOG=1");
          LastKeepAliveTime = Now;
        }
#endif
      }
    }
    else
    {
      mpsVistaEngine.reset();
    }

    LOG_INFO(mVistaEngineParam_X.psILogger, "Vista Daemon loop exiting...");
  }
  return Rts_B;
}

void VistaDaemon::RequestStop()
{
  // Async-signal-safe: storing to a lock-free std::atomic<bool> is permitted in a
  // signal handler ([support.signal]). Do NOT call Stop()/StopEngine()/logging here:
  // those lock mutexes, allocate and join threads, which is undefined behaviour in a
  // signal handler and can deadlock. The Start() loop polls mIsShutdownRequested and
  // will exit, after which main() performs the actual cleanup in normal context.
  mIsShutdownRequested = true;
}

bool VistaDaemon::IsRestartRequested() const
{
  return mIsRestartRequested;
}

void VistaDaemon::Stop()
{
  if (mIsDaemonRunning)
  {
    mIsShutdownRequested = true;
    LOG_INFO(mVistaEngineParam_X.psILogger, "Shutdown requested");
    mpsVistaEngine->StopEngine();
    mIsDaemonRunning = false;
    LOG_INFO(mVistaEngineParam_X.psILogger, "Vista Daemon stopped");
  }
  else
  {
    LOG_INFO(mVistaEngineParam_X.psILogger, "Vista Daemon not running");
  }
  // Destroy the engine here, in normal context while BOF and the async logger are still alive, so the
  // SDL render/AI worker threads and the web server are stopped and joined now (Shutdown() runs from
  // ~VistaEngineImplementation). Otherwise the engine would only be torn down at static destruction,
  // AFTER main() called Bof_Shutdown() and the spdlog async thread pool was gone: the still-running
  // render thread would then spin logging "thread pool doesn't exist anymore" and systemd's
  // stop-sigterm would hang until SIGKILL.
  if (mpsVistaEngine)
  {
    mpsVistaEngine.reset();
  }
}
