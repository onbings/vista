/*
 * Copyright (c) 2024-2044, OnBings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module implements the VISTA engine: Vision Intelligence, Statistics, and Tracking Analysis
 *
 * Author:      Bernard HARMEL: b.harmel@gmail.com
 *
 * History:
 * V 1.00  September 25 2026  BHA : Initial release
 */
#include "onbings-vista/compiled_version.h"
#include "onbings-vista/vista_engine.h"
#include "onbings-vista/vista_web_server.h"
#include <evs-hwfw-chip/rt_thread.h>

constexpr uint32_t VISTA_ENGINE_TIMEOUT_IN_MS = 500;
constexpr uint32_t VISTA_ENGINE_POLL_IN_MS = 500;
BEGIN_VISTA_NAMESPACE();

// Opaque pointer design pattern: defined privately here all private data and functions: all of these can now change without recompiling callers ...
class VistaEngine::VistaEngineImplementation
{
public:
  VistaEngineImplementation(const VISTA_ENGINE_PARAM &_rVistaEngineParam_X) : mVistaEngineParam_X(_rVistaEngineParam_X)
  {
    int Sts_i;
    (void)Sts_i;
    LOG_INFO(mVistaEngineParam_X.psILogger, "Initialize Vista Engine version {}...", ONBINGS::Vista::onbings_vista::get_version());

    Sts_i = Initialize();
    LOG_INFO(mVistaEngineParam_X.psILogger, "Vista Engine Init Param:");
    LOG_INFO(mVistaEngineParam_X.psILogger, "  psLogger: {}", static_cast<void *>(mVistaEngineParam_X.psILogger.get()));
    if (Sts_i != 0)
    {
      LOG_ERROR(mVistaEngineParam_X.psILogger, "Vista Engine initialization failed with status {}", Sts_i);
      throw std::runtime_error("Vista Engine initialization failed");
    }
  }

  ~VistaEngineImplementation()
  {
    int Sts_i;

    Sts_i = Shutdown();
    LOG_INFO(mVistaEngineParam_X.psILogger, "Vista Engine Shutdown status {}", Sts_i);
  }

  int Initialize()
  {
    int Rts_i = -1;
    if (!mEngineInitDone)
    {
      if (mVistaEngineParam_X.ListenIpAddress_X.Port_U16 != 0)
      {
        LOG_INFO(mVistaEngineParam_X.psILogger, "Create web server on {}", mVistaEngineParam_X.ListenIpAddress_X.ToString(true, false, false, true));
        Rts_i = CreateWebServer(mVistaEngineParam_X.ListenIpAddress_X, mVistaEngineParam_X.WebServerParam_X);
      }
      else
      {
        Rts_i = 0;
        LOG_INFO(mVistaEngineParam_X.psILogger, "Web interface disabled");
      }
      if (Rts_i == 0)
      {
        mEngineInitDone = true;
      }
    }
    LOG_INFO(mVistaEngineParam_X.psILogger, "Initialize returns {}", Rts_i);
    return Rts_i;
  }

  int Shutdown()
  {
    int Rts_i = -1;
    if (mEngineInitDone)
    {
      if (mVistaEngineParam_X.ListenIpAddress_X.Port_U16 != 0)
      {
        LOG_INFO(mVistaEngineParam_X.psILogger, "Destory web server on {}", mVistaEngineParam_X.ListenIpAddress_X.ToString(true, false, false, true));
        Rts_i = DestroyWebServer();
      }
      else
      {
        Rts_i = 0;
        LOG_ERROR(mVistaEngineParam_X.psILogger, "No web component specified");
      }
      StopEngine();

      mEngineInitDone = false;
      Rts_i = 0;
    }
    LOG_INFO(mVistaEngineParam_X.psILogger, "Shutdown returns {}", Rts_i);
    return Rts_i;
  }
  int CreateWebServer(const BOF::BOF_SOCKET_ADDRESS_COMPONENT &_rListenIpAddress_X, const BOF::BOF_WEB_SERVER_PARAM &_rWebServerParam_X)
  {
    int Rts_i = 10;
    VISTA_WEB_SERVER_PARAM VistaWebServerParam_X;

    if (!mpuVistaWebServer)
    {
      Rts_i = 11;
      LOG_INFO(mVistaEngineParam_X.psILogger, "Create {} Vista web server '{}' with:",
               ((!mVistaEngineParam_X.WebServerParam_X.CertificatePath_S.empty()) && (!mVistaEngineParam_X.WebServerParam_X.PrivateKeyPath_S.empty())) ? "Secure" : "Non-Secure",
               mVistaEngineParam_X.WebServerParam_X.WebAppParam_X.AppName_S);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  Listen:                '{}'", mVistaEngineParam_X.ListenIpAddress_X.ToString(true, false, false, true));
      LOG_INFO(mVistaEngineParam_X.psILogger, "  RootDir:                '{}'", mVistaEngineParam_X.WebServerParam_X.RootDir_S);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  ThreadPoolSize:         {}", _rWebServerParam_X.ThreadPoolSize_U32);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  LogRequestAndResponse:  '{}'", _rWebServerParam_X.LogRequestAndResponse_B ? "True" : "False");
      LOG_INFO(mVistaEngineParam_X.psILogger, "  DisableCors:            '{}'", _rWebServerParam_X.DisableCors_B ? "True" : "False");
      LOG_INFO(mVistaEngineParam_X.psILogger, "  CertificatePath_S:      '{}'", _rWebServerParam_X.CertificatePath_S);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  PrivateKeyPath:         '{}'", _rWebServerParam_X.PrivateKeyPath_S);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  ServerStartStopTimeout: {} ms", _rWebServerParam_X.ServerStartStopTimeoutInMs_U32);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  KeepAliveMaxCount:      {}", _rWebServerParam_X.KeepAliveMaxCount_U32);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  KeepAliveTimeout:       {} sec", _rWebServerParam_X.KeepAliveTimeoutInMs_U32 / 1000);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  ReadTimeout:            {} ms", _rWebServerParam_X.ReadTimeoutInMs_U32);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  WriteTimeout:           {} ms", _rWebServerParam_X.WriteTimeoutInMs_U32);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  IdleInterval:           {} ms", _rWebServerParam_X.IdleIntervalInMs_U32);
      LOG_INFO(mVistaEngineParam_X.psILogger, "  PayloadMaxLength:       {:08X}", _rWebServerParam_X.PayloadMaxLengthInByte_U32);
      /*
      MmEngine::Start() waits for mWorkerThreadEngineRunning which is set after mpsMmEngineBackend and mpsVistaAiEngine are created (line 627 in WorkerThreadEngine).
      So by the time Start() returns, the backend and AI engine are guaranteed to exist.
      Since CreateWebServer is called after mpuMmEngine->Start() succeeds, the getters will return valid pointers.
      */
      VistaWebServerParam_X.psILogger = mVistaEngineParam_X.psILogger;
      // VistaWebServerParam_X.psMmEngineBackend = mpsMmEngineBackend;
      mpuVistaWebServer = std::make_unique<VistaWebServer>(VistaWebServerParam_X, _rWebServerParam_X);
      Rts_i = 12;
      LOG_INFO(mVistaEngineParam_X.psILogger, "Starting Vista web server...");
      if (mpuVistaWebServer->Start(_rListenIpAddress_X.IpAddress_S.c_str(), _rListenIpAddress_X.Port_U16))
      {
        LOG_INFO(mVistaEngineParam_X.psILogger, "Vista web server started");
        Rts_i = 0;
      }
      else
      {
        LOG_ERROR(mVistaEngineParam_X.psILogger, "Cannot start Vista web server...");
        mpuVistaWebServer.reset();
      }
    }
    else
    {
      LOG_ERROR(mVistaEngineParam_X.psILogger, "Vista web component already created.");
    }
    return Rts_i;
  }
  int DestroyWebServer()
  {
    int Rts_i = 0;

    LOG_INFO(mVistaEngineParam_X.psILogger, "Stopping Vista web server...");
    return Rts_i;
  }

  int StartEngine()
  {
    int Rts_i = -1;
    // Check if thread is already running
    if (!mWorkerThreadRunning)
    {
      Rts_i = -2;
      mStopWorkerThreadRequested = false;
      // Start the worker thread
      mpuWorkerThread = std::make_unique<CHIP::RtThread>(50, "Vista Engine", [this]() { this->WorkerThreadFunction(); });
      if (mpuWorkerThread)
      {
        Rts_i = -3;
        // Wait for the worker thread to start (max VISTA_ENGINE_TIMEOUT_IN_MS*2)
        auto Start = std::chrono::steady_clock::now();
        auto TimeoutInMs = std::chrono::milliseconds(VISTA_ENGINE_TIMEOUT_IN_MS * 2);

        while (!mWorkerThreadRunning && (std::chrono::steady_clock::now() - Start) < TimeoutInMs)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(VISTA_ENGINE_POLL_IN_MS));
        }

        if (mWorkerThreadRunning)
        {
          mStartTimeInMicro_U64 = BOF::Bof_GetUsTickCount();
          Rts_i = 0;
        }
        else
        {
          LOG_ERROR(mVistaEngineParam_X.psILogger, "Worker thread failed to start within timeout");
        }
      }
    }
    LOG_INFO(mVistaEngineParam_X.psILogger, "StartEngine returns {}", Rts_i);
    return Rts_i;
  }
  int StopEngine()
  {
    int Rts_i = -1;

    if (mWorkerThreadRunning)
    {
      Rts_i = -2;
      mStopWorkerThreadRequested = true;

      // Wait for the worker thread to stop (max VISTA_ENGINE_TIMEOUT_IN_MS*2)
      auto Start = std::chrono::steady_clock::now();
      auto TimeoutInMs = std::chrono::milliseconds(VISTA_ENGINE_TIMEOUT_IN_MS * 2);

      while (mWorkerThreadRunning && (std::chrono::steady_clock::now() - Start) < TimeoutInMs)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(VISTA_ENGINE_POLL_IN_MS));
      }

      if (!mWorkerThreadRunning)
      {
        Rts_i = 0;
      }
      else
      {
        LOG_ERROR(mVistaEngineParam_X.psILogger, "Worker thread did not stop within timeout");
      }

      // Join the thread to clean up resources
      if ((mpuWorkerThread) && (mpuWorkerThread->joinable()))
      {
        mpuWorkerThread->join();
      }
    }
    else
    {
      Rts_i = 0;
    }
    LOG_INFO(mVistaEngineParam_X.psILogger, "StopEngine returns {}", Rts_i);
    return Rts_i;
  }

  bool IsRestartRequested()
  {
    bool Rts_B = false;
    if (mpuVistaWebServer)
    {
      // Rts_B = mpuVistaWebServer->CleanupExpiredService();
    }
    return Rts_B;
  }
  void WorkerThreadFunction()
  {
    mWorkerThreadRunning = true;
    LOG_INFO(mVistaEngineParam_X.psILogger, "Worker thread started");

    while (!mStopWorkerThreadRequested)
    {
      try
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(VISTA_ENGINE_POLL_IN_MS));
      }
      catch (const std::exception &e)
      {
        LOG_ERROR(mVistaEngineParam_X.psILogger, "Error in worker thread: {}", e.what());
        // Continue running even if there's an error
        std::this_thread::sleep_for(std::chrono::milliseconds(VISTA_ENGINE_POLL_IN_MS));
      }
    }

    LOG_INFO(mVistaEngineParam_X.psILogger, "Worker thread stopped");
    mWorkerThreadRunning = false;
  }

private:
  VISTA_ENGINE_PARAM mVistaEngineParam_X;
  std::atomic<bool> mEngineInitDone{false};
  std::unique_ptr<VistaWebServer> mpuVistaWebServer = nullptr;
  std::unique_ptr<CHIP::RtThread> mpuWorkerThread = nullptr;
  std::atomic<bool> mStopWorkerThreadRequested{false};
  std::atomic<bool> mWorkerThreadRunning{false};
  uint64_t mStartTimeInMicro_U64 = 0;
};

// Opaque pointer design pattern: initialize the pointer to the implementation class
std::shared_ptr<VistaEngine> VistaEngine::S_VistaEngineFactory(const VISTA_ENGINE_PARAM &_rVistaEngineParam_X)
{
  std::shared_ptr<VistaEngine> pRts;

  pRts = std::shared_ptr<VistaEngine>(new VistaEngine(_rVistaEngineParam_X));
  return pRts;
}

VistaEngine::VistaEngine(const VISTA_ENGINE_PARAM &_rVistaEngineParam_X) : mpuVistaEngineImplementation(new VistaEngineImplementation(_rVistaEngineParam_X))
{
}
VistaEngine::~VistaEngine()
{
}
const char *VistaEngine::S_GetVersion()
{
  return (ONBINGS::Vista::onbings_vista::get_version());
}
int VistaEngine::StartEngine()
{
  return mpuVistaEngineImplementation->StartEngine();
}
int VistaEngine::StopEngine()
{
  return mpuVistaEngineImplementation->StopEngine();
}
bool VistaEngine::IsRestartRequested()
{
  return mpuVistaEngineImplementation->IsRestartRequested();
}
END_VISTA_NAMESPACE()