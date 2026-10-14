/*
 * Copyright (c) 2024-2044, OnBings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module defines the VISTA daemon interface: Vision Intelligence, Statistics, and Tracking Analysis
 *
 * Author:      Bernard HARMEL: b.harmel@gmail.com
 *
 * History:
 * V 1.00  September 25 2026  BHA : Initial release
 */
#pragma once

#include <memory>
#include <onbings-vista/vista_engine.h>

class VistaDaemon
{
public:
  VistaDaemon(const VISTA::VISTA_ENGINE_PARAM &_rVistaEngineParam_X);
  ~VistaDaemon();

  bool Start();
  void Stop();
  // Async-signal-safe: only flips the shutdown flag (a lock-free std::atomic<bool>)
  // so it can be safely called from a signal handler. The blocking loop in Start()
  // polls this flag and exits; the real teardown (Stop()) then runs in normal context.
  void RequestStop();

  // Returns true when a client asked the application to restart (re-exec) rather than just stop.
  // main() checks this after Start() returns to decide between a clean exit and a re-exec.
  bool IsRestartRequested() const;

private:
  int VistaEngineProcessingLoop();

private:
  VISTA::VISTA_ENGINE_PARAM mVistaEngineParam_X;
  std::shared_ptr<VISTA::VistaEngine> mpsVistaEngine = nullptr;
  std::atomic<bool> mIsDaemonRunning{false};
  std::atomic<bool> mIsShutdownRequested{false};
  std::atomic<bool> mIsRestartRequested{false};
};
