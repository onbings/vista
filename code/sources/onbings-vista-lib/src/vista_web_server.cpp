/*
 * Copyright (c) 2024-2044, OnBings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module implements the VISTA web server: Vision Intelligence, Statistics, and Tracking Analysis
 *
 * Author:      Bernard HARMEL: b.harmel@gmail.com
 *
 * History:
 * V 1.00  September 25 2026  BHA : Initial release
 */
#include <onbings-vista/vista_web_server.h>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

BEGIN_VISTA_NAMESPACE()

VistaWebServer::VistaWebServer(const VISTA_WEB_SERVER_PARAM &_rVistaWebServerParam_X, const BOF::BOF_WEB_SERVER_PARAM &_rBofWebServerParam_X)
    : BOF::BofWebServer(nullptr, _rBofWebServerParam_X), mVistaWebServerParam_X(_rVistaWebServerParam_X)
{
  std::string Path_S;
  BOFERR Sts_E;
  VISTA_STATIC_FILE_SERVER StaticFileServerParam_X;

  LOG_INFO(mVistaWebServerParam_X.psILogger, "Instanciate Vista web server");
  // Base
  // Get("/", [this](const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) { this->GetRoot(_rReq, _rRes); });

  Get("/vista/", [this](const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) { this->GetRoot(_rReq, _rRes); });
  Get("/vista/v1/", [this](const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) { this->GetRoot(_rReq, _rRes); });
  Get("/vista/v1/doc", [this](const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) { this->GetDoc(_rReq, _rRes); });
  // Stream control (pause, resume, select)
  RegisterStreamControlRoutes();
  // Jwt Auth
#if defined(ONBINGS_VISTA_ADD_AUTH)
  Post("/vista/v1/auth", [this](const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) { this->PostAuth(_rReq, _rRes); });
#endif

  Sts_E = BOF::Bof_GetListOfNetworkInterface(mListOfNetworkInterface_X);
  if (Sts_E != BOF_ERR_NO_ERROR)
  {
    LOG_ERROR(mVistaWebServerParam_X.psILogger, "Failed to get list of network interfaces ({})", BOF::Bof_ErrorCode(Sts_E));
  }

  // Check if directory exists
  if ((std::filesystem::exists(mWebServerParam_X.RootDir_S)) && (std::filesystem::is_directory(mWebServerParam_X.RootDir_S)))
  {
    LOG_INFO(mVistaWebServerParam_X.psILogger, "Root directory '{}' already exists", mWebServerParam_X.RootDir_S);
  }
  else
  {
    // Create directory recursively (creates all intermediate directories) with default permissions
    try
    {
      std::filesystem::create_directories(mWebServerParam_X.RootDir_S);
      LOG_INFO(mVistaWebServerParam_X.psILogger, "Root directory '{}' created, setting permissions", mWebServerParam_X.RootDir_S);
      std::filesystem::permissions(mWebServerParam_X.RootDir_S, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write | std::filesystem::perms::owner_exec,
                                   std::filesystem::perm_options::replace);
    }
    catch (const std::filesystem::filesystem_error &e)
    {
      LOG_ERROR(mVistaWebServerParam_X.psILogger, "Failed to create root directory '{}' ({})", mWebServerParam_X.RootDir_S, e.what());
    }
  }
  StaticFileServerParam_X.psILogger = mVistaWebServerParam_X.psILogger;
#if _WIN32
  StaticFileServerParam_X.PublicDir_S = "../assets/www";
#else
  StaticFileServerParam_X.PublicDir_S = "./assets/www";
#endif
  StaticFileServerParam_X.NbMaxFileCached_U32 = 8;
  StaticFileServerParam_X.MaxFileSizeToCache = 4 * 1024 * 1024;
  StaticFileServerParam_X.DefaultPageName_S = "index.html";
  mpuStaticFileServer = std::make_unique<StaticFileServer>(StaticFileServerParam_X);
}
VistaWebServer ::~VistaWebServer()
{
  LOG_INFO(mVistaWebServerParam_X.psILogger, "Delete Vista web server");
}

void VistaWebServer::V_OnFileRequest(const BOF::BOF_WEB_REQUEST & /*_rReq*/, BOF::BOF_WEB_RESPONSE & /*_rRes*/)
{
  LOG_INFO(mVistaWebServerParam_X.psILogger, "V_OnFileRequest");
}
void VistaWebServer::V_OnError(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes)
{
  LOG_INFO(mVistaWebServerParam_X.psILogger, "V_OnError '{}' '{}' ->{}", _rReq.method, _rReq.path, _rRes.status);
}
void VistaWebServer::V_OnException(const BOF::BOF_WEB_REQUEST & /*_rReq*/, BOF::BOF_WEB_RESPONSE &_rRes, std::exception_ptr ep)
{
  char pBuffer_c[0x100];
  LOG_INFO(mVistaWebServerParam_X.psILogger, "V_OnException");
  try
  {
    std::rethrow_exception(ep);
  }
  catch (std::exception &rE)
  {
    snprintf(pBuffer_c, sizeof(pBuffer_c), "<h1>Error 500</h1><p>%s</p>", rE.what());
  }
  catch (...)
  { // See the following NOTE
    snprintf(pBuffer_c, sizeof(pBuffer_c), "<h1>Error 500</h1><p>%s</p>", "Unknown Exception");
  }
  _rRes.set_content(pBuffer_c, "text/html");
  _rRes.status = BOF::BOF_WEB_STATUS::InternalServerError_500;
}
bool VistaWebServer::V_OnPreRouting(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE & /*_rRes*/)
{
  LOG_INFO(mVistaWebServerParam_X.psILogger, "GetStaticAssetGetStaticAsset - Method: {}, Path: {}, Remote Addr: {}, User Agent: {}", _rReq.method, _rReq.path, _rReq.remote_addr,
           _rReq.get_header_value("User-Agent"));

  // Log additional request details if body is present
  if (!_rReq.body.empty())
  {
    LOG_INFO(mVistaWebServerParam_X.psILogger, "Request Body Length: {}, Content-Type: {}", _rReq.body.length(), _rReq.get_header_value("Content-Type"));
  }

  // Cleanup expired services before building response
  return false;
}
void VistaWebServer::V_OnPostRouting(const BOF::BOF_WEB_REQUEST & /*_rReq*/, BOF::BOF_WEB_RESPONSE & /*_rRes*/)
{
  // LOG_INFO(mVistaWebServerParam_X.psILogger, "V_OnPostRouting");
}
// By default, the server sends a 100 Continue response for an Expect:
// 100-continue header.
BOF::BOF_WEB_STATUS VistaWebServer::V_OnExpect100Continue(const BOF::BOF_WEB_REQUEST & /*_rReq*/, BOF::BOF_WEB_RESPONSE & /*_rRes*/)
{
  LOG_INFO(mVistaWebServerParam_X.psILogger, "V_OnExpect100Continue");
#if 0
    // Send a '417 Expectation Failed' response.
    BOF::BOF_WEB_STATUS Rts = BOF::BOF_WEB_STATUS::ExpectationFailed_417;
    // Send a final status httplib::detail::without reading the message body.
    Rts = BOF::BOF_WEB_STATUS::Unauthorized_401;
    _rRes_X.status = Rts;

    // Custom logic to determine if the request should be continued
    // For example, check if the Content-Length is acceptable
    auto it = _rReq_X.headers.find("Content-Length");
    if (it != _rReq_X.headers.end())
    {
      size_t ContentLength = std::stoull(it->second);
      if (ContentLength > 1024 * 1024)
      {                                                // Arbitrary limit of 1MB
        Rts = BOF::BOF_WEB_STATUS::LengthRequired_411; // Reject requests larger than 1MB
      }
    }
    Rts = BOF::BOF_WEB_STATUS::OK_200; // Continue with the request
#endif
  return BOF::BOF_WEB_STATUS::Continue_100;
}
void VistaWebServer::V_OnSetSocketOption(BOF::BOF_WEB_SOCKET _Socket)
{
  LOG_INFO(mVistaWebServerParam_X.psILogger, "V_OnSetSocketOption({})", _Socket);
  int OptVal_i = 1; // Enable the option
#ifdef _WIN32
  setsockopt(_Socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&OptVal_i), sizeof(OptVal_i));
  setsockopt(_Socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&OptVal_i), sizeof(OptVal_i));
#else
  setsockopt(_Socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const void *>(&OptVal_i), sizeof(OptVal_i));
  setsockopt(_Socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const void *>(&OptVal_i), sizeof(OptVal_i));
#endif
}

void VistaWebServer::GetRoot(const BOF::BOF_WEB_REQUEST & /*_rReq*/, BOF::BOF_WEB_RESPONSE &_rRes)
{
  LOG_INFO(mVistaWebServerParam_X.psILogger, "GetRoot:redirect to doc");
  _rRes.set_redirect("/vista/v1/doc");
}
void VistaWebServer::GetDoc(const BOF::BOF_WEB_REQUEST & /*_rReq*/, BOF::BOF_WEB_RESPONSE &_rRes)
{
  std::string HtmlContent_S, Cwd_S; //, Placeholder_S, Replacement_S;
#if _WIN32
  std::filesystem::path AssetPath = "../assets/www/rest-vista.html";
#else
  std::filesystem::path AssetPath = "./assets/www/rest-vista.html";
#endif
  std::stringstream Buffer;
  // size_t Pos;

  /*
  #
  # In /home/bha/pro/onbings-vista/code/sources/onbings-vista-lib/assets use
  # pip install PyYAML
  # python3 swagger-yaml-to-html.py --input ./rest-vista.yaml --output rest-vista.html
  # to generate rest-vista.html
  # Deploy the assets directory
  # If you go to the /doc route you will have a test web page to test the api
  #
  # This step is made in the CMakeLists.txt file by execute_process
  */
  Cwd_S = std::filesystem::current_path().string();
  if (!std::filesystem::exists(AssetPath))
  {
    LOG_ERROR(mVistaWebServerParam_X.psILogger, "Failed to read file '{}' Cwd '{}' - file does not exist", AssetPath.string(), Cwd_S);
    _rRes.status = BOF::BOF_WEB_STATUS::ServiceUnavailable_503;
  }
  else
  {
    try
    {
      std::ifstream File(AssetPath);
      if (!File.is_open())
      {
        LOG_ERROR(mVistaWebServerParam_X.psILogger, "Failed to open file '{}' Cwd '{}'", AssetPath.string(), Cwd_S);
        _rRes.status = BOF::BOF_WEB_STATUS::ServiceUnavailable_503;
      }
      else
      {
        Buffer << File.rdbuf();
        File.close();
        HtmlContent_S = Buffer.str();

        LOG_INFO(mVistaWebServerParam_X.psILogger, "GetDoc Cwd '{}' Content length '{}'", Cwd_S, HtmlContent_S.length());
        _rRes.status = BOF::BOF_WEB_STATUS::OK_200;
        _rRes.set_content(HtmlContent_S.c_str(), HtmlContent_S.size(), "text/html");
      }
    }
    catch (const std::exception &e)
    {
      LOG_ERROR(mVistaWebServerParam_X.psILogger, "Exception reading file '{}' Cwd '{}': {}", AssetPath.string(), Cwd_S, e.what());
      _rRes.status = BOF::BOF_WEB_STATUS::ServiceUnavailable_503;
    }
  }
}

// Initialize with the directory to serve and the max number of files to cache
StaticFileServer::StaticFileServer(const VISTA_STATIC_FILE_SERVER &_rStaticFileServerParam_X) : mStaticFileServerParam_X(_rStaticFileServerParam_X)
{
  LOG_INFO(mStaticFileServerParam_X.psILogger, "StaticFileServer initialized with PublicDir '{}', MaxFileCached {} and MaxFileSizeToCache {} B. Pwd is {}", mStaticFileServerParam_X.PublicDir_S,
           mStaticFileServerParam_X.NbMaxFileCached_U32, mStaticFileServerParam_X.MaxFileSizeToCache, std::filesystem::current_path().string());
  // Ensure the public directory exists
  if (!std::filesystem::exists(mStaticFileServerParam_X.PublicDir_S) || !std::filesystem::is_directory(mStaticFileServerParam_X.PublicDir_S))
  {
    LOG_ERROR(mStaticFileServerParam_X.psILogger, "Public directory '{}' does not exist or is not a directory", mStaticFileServerParam_X.PublicDir_S);
  }
}

bool StaticFileServer::Serve(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes)
{
  bool Rts_B = false;
  std::string Cwd_S, FilePath_S, Extension_S, MimeType_S, LruPath_S, FileContent_S, Url_S(_rReq.path);
  std::error_code Ec;
  fs::file_time_type LastWriteTime_X;

  LOG_INFO(mStaticFileServerParam_X.psILogger, "Serve Request URL '{}'", Url_S);
  if (Url_S.find("..") != std::string::npos)
  {
    _rRes.status = BOF::BOF_WEB_STATUS::Forbidden_403;
    _rRes.set_content("403 Forbidden", "text/plain");
  }
  else
  {
    if (Url_S == "/" || Url_S.back() == '/')
    {
      Url_S += mStaticFileServerParam_X.DefaultPageName_S;
    }

    FilePath_S = mStaticFileServerParam_X.PublicDir_S + Url_S;
    Cwd_S = std::filesystem::current_path().string();
    LOG_INFO(mStaticFileServerParam_X.psILogger, "Look for '{}', Cwd '{}'", FilePath_S, Cwd_S);
    auto It = mCachedFiles.find(FilePath_S);
    bool CacheHit_B = false;
    if (It != mCachedFiles.end())
    {
      Ec.clear();
      LastWriteTime_X = fs::last_write_time(FilePath_S, Ec);
      if (Ec || (LastWriteTime_X != It->second.first.LastWriteTime_X))
      {
        LOG_INFO(mStaticFileServerParam_X.psILogger, "Cache STALE for '{}'", FilePath_S);
        mLastFilesAccessed.erase(It->second.second);
        mCachedFiles.erase(It);
      }
      else
      {
        mLastFilesAccessed.erase(It->second.second);
        mLastFilesAccessed.push_front(FilePath_S);
        It->second.second = mLastFilesAccessed.begin();
        CacheHit_B = true;
      }
    }

    if (CacheHit_B)
    {
      auto ItHit = mCachedFiles.find(FilePath_S);
      _rRes.status = BOF::BOF_WEB_STATUS::OK_200;
      _rRes.set_header("X-Cache", "HIT");
      _rRes.set_content(ItHit->second.first.Content_S.data(), ItHit->second.first.Content_S.size(), ItHit->second.first.MimeType_S.c_str());
      LOG_INFO(mStaticFileServerParam_X.psILogger, "Cache HIT for '{}'", FilePath_S);
      Rts_B = true;
    }
    else
    {
      LOG_INFO(mStaticFileServerParam_X.psILogger, "Cache MISS for '{}', Cwd '{}'", FilePath_S, Cwd_S);
      if (!fs::exists(FilePath_S) || fs::is_directory(FilePath_S))
      {
        _rRes.status = BOF::BOF_WEB_STATUS::NotFound_404;
        _rRes.set_content("404 Not Found", "text/plain");
      }
      else if (!S_ReadFile(FilePath_S, FileContent_S))
      {
        _rRes.status = BOF::BOF_WEB_STATUS::InternalServerError_500;
        _rRes.set_content("Error reading file", "text/plain");
      }
      else
      {
        LOG_INFO(mStaticFileServerParam_X.psILogger, "File read successfully from disk '{}', Size: {} B, Max: {} B", FilePath_S, FileContent_S.size(), mStaticFileServerParam_X.MaxFileSizeToCache);
        Extension_S = fs::path(FilePath_S).extension().string();
        MimeType_S = S_GetMimeType(Extension_S);

        if (FileContent_S.size() <= mStaticFileServerParam_X.MaxFileSizeToCache)
        {
          LOG_INFO(mStaticFileServerParam_X.psILogger, "Update cache {}/{} entries", mCachedFiles.size(), mStaticFileServerParam_X.NbMaxFileCached_U32);
          if (mCachedFiles.size() >= mStaticFileServerParam_X.NbMaxFileCached_U32)
          {
            LruPath_S = mLastFilesAccessed.back();
            mLastFilesAccessed.pop_back();
            mCachedFiles.erase(LruPath_S);
          }
          Ec.clear();
          LastWriteTime_X = fs::last_write_time(FilePath_S, Ec);
          mLastFilesAccessed.push_front(FilePath_S);
          mCachedFiles[FilePath_S] = {{FileContent_S, MimeType_S, Ec ? fs::file_time_type{} : LastWriteTime_X}, mLastFilesAccessed.begin()};
          _rRes.set_header("X-Cache", "MISS");
        }
        else
        {
          _rRes.set_header("X-Cache", "MISS (Uncached - Too Large)");
        }

        _rRes.status = BOF::BOF_WEB_STATUS::OK_200;
        _rRes.set_content(FileContent_S.data(), FileContent_S.size(), MimeType_S.c_str());
        Rts_B = true;
      }
    }
  }

  return Rts_B;
}

// Helper: Determine MIME type from extension
std::string StaticFileServer::S_GetMimeType(const std::string &_rExt_S)
{
  static const std::unordered_map<std::string, std::string> mime_types = {{".html", "text/html; charset=utf-8"},
                                                                          {".css", "text/css; charset=utf-8"},
                                                                          {".js", "application/javascript; charset=utf-8"},
                                                                          {".json", "application/json; charset=utf-8"},
                                                                          {".png", "image/png"},
                                                                          {".jpg", "image/jpeg"},
                                                                          {".jpeg", "image/jpeg"},
                                                                          {".svg", "image/svg+xml"},
                                                                          {".ico", "image/x-icon"}};
  auto It = mime_types.find(_rExt_S);
  return It != mime_types.end() ? It->second : "application/octet-stream"; //{".bin", "application/octet-stream"}
}

// Helper: Read file from disk
bool StaticFileServer::S_ReadFile(const std::string &_rFilepath_S, std::string &_rOutContent_S)
{
  bool Rts_B = false;
  std::ifstream File(_rFilepath_S, std::ios::binary);
  std::ostringstream Ss;

  if (File)
  {
    Ss << File.rdbuf();
    _rOutContent_S = Ss.str();
    Rts_B = true;
  }

  return Rts_B;
}

END_VISTA_NAMESPACE()