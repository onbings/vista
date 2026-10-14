/*
 * Copyright (c) 2024-2044, OnBings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module defines the VISTA engine: Vision Intelligence, Statistics, and Tracking Analysis
 *
 * Author:      Bernard HARMEL: b.harmel@gmail.com
 *
 * History:
 * V 1.00  September 25 2026  BHA : Initial release
 */
#pragma once
#include <onbings-vista/vista.h>
#include <bofstd/bofwebserver.h>
#include <bofstd/bofsocketos.h>

BEGIN_VISTA_NAMESPACE()

/**
 * @struct VISTA_ENGINE_PARAM
 * @brief Configuration parameters for the VistaEngine.
 *
 * This structure contains all the configuration options needed to initialize
 * a VistaEngine instance, including logging configuration and ANC metadata settings.
 */
struct VISTA_ENGINE_PARAM
{
  std::shared_ptr<EVS::HwFw::Ilf::ILogger> psILogger; //!< Logger instances. Set to nullptr to disable logging
  BOF::BOF_SOCKET_ADDRESS_COMPONENT ListenIpAddress_X;
  BOF::BOF_WEB_SERVER_PARAM WebServerParam_X;
  /**
   *@brief Constructs a new VISTA_ENGINE_PARAM object.**Initializes the parameters with their default values by calling `Reset()`.
   */
  VISTA_ENGINE_PARAM()
  {
    Reset();
  }

  /**
   * @brief Resets all parameters to their default values.
   *
   * Sets `psLoggerFactory` to `nullptr`, ensuring a clean initial state.
   */
  void Reset()
  {
    psILogger = nullptr;
    ListenIpAddress_X.Reset();
    WebServerParam_X.Reset();
  }
};

/**
 * @class VistaEngine
 * @brief Central service for processing OSD metadata commands and dispatching to hardware devices.
 *
 * The VistaEngine is the core component of the VISTA system. It:
 * - Receives commands from multiple VistaEngineClient instances via IPC circular buffers
 * - Manages the state of all OSD targets (players and recorders) across all module targets
 * - Handles TTL-based tag lifecycle management
 * - Sends aggregated metadata to hardware devices via UDP on genlock triggers
 *
 * The server uses the Opaque Pointer (PIMPL) design pattern to hide implementation details
 * and ensure ABI stability across versions.
 *
 * @note Only one VistaEngine instance should be running at a time.
 * @note Use the factory method S_VistaEngineFactory() to create instances.
 *
 */
class ONBINGS_VISTA_EXPORT VistaEngine
{
private:
  /**
   * @brief Construct a new VistaEngine object
   * @param _rVistaEngineParam_X A constant reference to a `VISTA_ENGINE_PARAM` structure,
   * containing configuration settings for the VistaEngine instance.
   */
  VistaEngine(const VISTA_ENGINE_PARAM &_rVistaEngineParam_X);
  // Opaque pointer design pattern: all public and protected stuff goes here ...
public:
  /**
   * @brief Get the version string of the VistaEngine Module.
   * @return const char* Null-terminated version string.
   */
  static const char *S_GetVersion();
  /**
   * @brief Creates and returns a shared pointer to a new VistaEngine instance.
   *
   * This static factory method provides a controlled way to instantiate the `VistaEngine` class.
   * It encapsulates the internal creation logic and returns a shared pointer,
   * promoting safe memory management.
   *
   * @param _rVistaEngineParam_X A constant reference to a `VISTA_ENGINE_PARAM` object,
   * containing configuration settings required for the `VistaEngine` instance.
   *
   * @return A `std::shared_ptr<VistaEngine>` to the newly created `VistaEngine` instance.
   * The caller is responsible for holding onto this shared pointer to manage the lifetime of the object.
   */
  static std::shared_ptr<VistaEngine> S_VistaEngineFactory(const VISTA_ENGINE_PARAM &_rVistaEngineParam_X);

  /**
   * @brief Destructor - stops server and releases all resources.
   */
  ~VistaEngine();

  /**
   * @brief Start the server worker thread.
   *
   * Initializes the IPC circular buffers (if not already done) and starts the
   * worker thread that processes incoming commands from clients.
   *
   * @return 0 on success, non-zero on failure (e.g., already running)
   */
  int StartEngine();

  /**
   * @brief Stop the server worker thread.
   *
   * Signals the worker thread to stop and waits for it to complete.
   * After this call, no more commands will be processed until StartServer() is called again.
   *
   * @return 0 on success, non-zero on failure (e.g., not running)
   */
  int StopEngine();
  /**
   * @brief Returns true once a client requested an application restart (re-exec to initial state).
   */
  bool IsRestartRequested();

  //  Opaque pointer design pattern: opaque type here
private:
  class VistaEngineImplementation;                                         //!< Forward declaration of implementation class
  std::unique_ptr<VistaEngineImplementation> mpuVistaEngineImplementation; //!< Opaque pointer to implementation
};

END_VISTA_NAMESPACE()
