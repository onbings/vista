/*
 * Copyright (c) 2024-2044, OnBings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module defines the VISTA web server: Vision Intelligence, Statistics, and Tracking Analysis
 *
 * Author:      Bernard HARMEL: b.harmel@gmail.com
 *
 * History:
 * V 1.00  September 25 2026  BHA : Initial release
 */

#pragma once
#include <onbings-vista/vista.h>
#include <bofstd/bofwebserver.h>
#include <bofstd/bofserviceendpoint.h>
#include <filesystem>

BEGIN_VISTA_NAMESPACE()

struct VISTA_WEB_SERVER_PARAM
{
  std::shared_ptr<EVS::HwFw::Ilf::ILogger> psILogger;
  VISTA_WEB_SERVER_PARAM()
  {
    Reset();
  }
  void Reset()
  {
    psILogger = nullptr;
  }
};
struct VISTA_STATIC_FILE_SERVER
{
  std::shared_ptr<EVS::HwFw::Ilf::ILogger> psILogger;
  std::string PublicDir_S;
  std::string DefaultPageName_S;
  uint32_t NbMaxFileCached_U32;
  size_t MaxFileSizeToCache;
  VISTA_STATIC_FILE_SERVER()
  {
    Reset();
  }
  void Reset()
  {
    psILogger = nullptr;
    PublicDir_S.clear();
    DefaultPageName_S.clear();
    NbMaxFileCached_U32 = 0;
    MaxFileSizeToCache = 0;
  }
};
class StaticFileServer
{
public:
  // Initialize with the directory to serve and the max number of files to cache
  StaticFileServer(const VISTA_STATIC_FILE_SERVER &_rStaticFileServerParam_X);

  // BOF web-server path
  bool Serve(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes);

private:
  struct CACHED_FILE
  {
    std::string Content_S;
    std::string MimeType_S;
    std::filesystem::file_time_type LastWriteTime_X;
  };

  VISTA_STATIC_FILE_SERVER mStaticFileServerParam_X;

  // LRU Cache structures
  std::list<std::string> mLastFilesAccessed;
  std::unordered_map<std::string, std::pair<CACHED_FILE, std::list<std::string>::iterator>> mCachedFiles;

  // Helper: Determine MIME type from extension
  static std::string S_GetMimeType(const std::string &_rExt_S);
  // Helper: Read file from disk
  static bool S_ReadFile(const std::string &_rFilepath_S, std::string &_rOutContent_S);
};
//! \class VistaWebServer
//! \brief Web server implementation for the Vista REST API.
class VistaWebServer : public BOF::BofWebServer
{
public:
  //! \brief Constructor.
  //! \param _rVistaWebServerParam_X Configuration parameters for the Vista web server.
  //! \param _rBofWebServerParam_X Configuration parameters for the BOF web server.
  VistaWebServer(const VISTA_WEB_SERVER_PARAM &_rVistaWebServerParam_X, const BOF::BOF_WEB_SERVER_PARAM &_rBofWebServerParam_X);

  //! \brief Destructor.
  ~VistaWebServer();

private:
  //! \brief Callback invoked when a file is requested.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  void V_OnFileRequest(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) override;

  //! \brief Callback invoked on error.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  void V_OnError(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) override;

  //! \brief Callback invoked when an exception occurs.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  //! \param ep Pointer to the exception.
  void V_OnException(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes, std::exception_ptr ep) override;

  //! \brief Callback invoked before routing the request.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  //! \return True to continue routing, false to stop.
  bool V_OnPreRouting(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) override;

  //! \brief Callback invoked after routing the request.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  void V_OnPostRouting(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) override;

  //! \brief Callback invoked when an Expect: 100-continue header is received.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  //! \return The status of the operation.
  BOF::BOF_WEB_STATUS V_OnExpect100Continue(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) override;

  //! \brief Callback invoked to set custom socket options.
  //! \param _Socket The socket to configure.
  void V_OnSetSocketOption(BOF::BOF_WEB_SOCKET _Socket) override;

  //! \brief Handler for the root endpoint.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  void GetRoot(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes);

  //! \brief Handler for the documentation endpoint.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  void GetDoc(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes);

  //! \brief Register all stream-control routes into the web server.
  //! Called once from the constructor.
  void RegisterStreamControlRoutes();

  // HTTP wrappers: decode path params, delegate to the matching business method, format the JSON response.
  // -------------------------------------------------------------------------
  // Configuration, snapshot, Axelera control
  // -------------------------------------------------------------------------

  //! \brief GET /vista/v1/config – return system configuration.
  void GetConfig(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes);

  //! \brief Collect system configuration (modules, channels, Axelera availability).
  //! \param _rConfigJson  Receives the configuration payload.
  //! \return true on success.
  bool GetConfig(BOF_WEB_JSON &_rConfigJson);

  //! \brief GET /* – serve static assets from the Vista web-server side.
  void GetStaticAsset(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes);

#if defined(ONBINGS_VISTA_ADD_AUTH)
  //! \brief Handler for authentication requests.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  void PostAuth(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes);

  //! \brief Authenticates a user via PAM.
  //! \param _rUsername_S The username.
  //! \param _rPassword_S The password.
  //! \param _rPamService_S The PAM service name.
  //! \return True if authentication succeeded, false otherwise.
  bool PamAuthenticate(const std::string &_rUsername_S, const std::string &_rPassword_S, const std::string &_rPamService_S);
  //! \brief Checks if an IP address is local.
  //! \param _rIpAddress_S The IP address to check.
  //! \return True if local, false otherwise.
  bool IsLocalAddress(const std::string &_rIpAddress_S);

  //! \brief Validates a JWT token from the request.
  //! \param _rReq The web request.
  //! \param _rRes The web response.
  //! \return True if the token is valid, false otherwise.
  bool CheckJwt(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes);
#endif

private:
  VISTA_WEB_SERVER_PARAM mVistaWebServerParam_X;
  std::vector<BOF::BOF_NETWORK_INTERFACE_PARAM> mListOfNetworkInterface_X;
  std::unique_ptr<StaticFileServer> mpuStaticFileServer;
  std::mutex mVistaWebServerMtx;
};

END_VISTA_NAMESPACE()