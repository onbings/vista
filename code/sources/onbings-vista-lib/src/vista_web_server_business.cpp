/*
 * Copyright (c) 2024-2044, OnBings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module defines the VISTA REST Api: Vision Intelligence, Statistics, and Tracking Analysis
 *
 * Route layout
 * ============
 * GET  /vista/v1/config                               – system configuration

 * Author:      Bernard HARMEL: b.harmel@gmail.com
 *
 * History:
 * V 1.00  September 25 2026  BHA : Initial release
 */
#include <onbings-vista/vista_web_server.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <chrono>

#if defined(ONBINGS_VISTA_ADD_AUTH)
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#endif

BEGIN_VISTA_NAMESPACE()

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

//! \brief Parse a uint32 from a string supporting decimal ("123") and hex ("0x1F") formats.
//! \param _rParam_S  The string to parse.
//! \param _rValid_B  Set to true on success, false on parse error.
//! \return The parsed value.
static uint64_t S_ParseValue(const std::string &_rParam_S, bool &_rValid_B)
{
  uint64_t Rts_U64 = 0;
  size_t Pos;

  try
  {
    Rts_U64 = static_cast<uint64_t>(std::stoull(_rParam_S, &Pos, 0)); // base 0: auto-detect dec/hex/oct
    _rValid_B = (Pos == _rParam_S.size());
  }
  catch (...)
  {
    _rValid_B = false;
  }
  return Rts_U64;
}

static bool S_ParseBoolValue(const std::string &_rParam_S, bool &_rValid_B)
{
  std::string Value_S;
  Value_S.resize(_rParam_S.size());
  std::transform(_rParam_S.begin(), _rParam_S.end(), Value_S.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  if ((Value_S == "1") || (Value_S == "true") || (Value_S == "yes") || (Value_S == "on"))
  {
    _rValid_B = true;
    return true;
  }
  if ((Value_S == "0") || (Value_S == "false") || (Value_S == "no") || (Value_S == "off"))
  {
    _rValid_B = true;
    return false;
  }
  _rValid_B = false;
  return false;
}

// ---------------------------------------------------------------------------
// Route registration
// ---------------------------------------------------------------------------

void VistaWebServer::RegisterStreamControlRoutes()
{
  // Configuration  ->  GET /vista/v1/config
  Get("/vista/v1/config", [this](const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) { this->GetConfig(_rReq, _rRes); });

  // --- Final fallback: Serve static files for any unmatched GET requests ---
  Get("/(.*)", [this](const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes) { this->GetStaticAsset(_rReq, _rRes); });
}

// ===========================================================================
// GET /vista/v1/config  -  System configuration
// ===========================================================================

void VistaWebServer::GetConfig(const BOF::BOF_WEB_REQUEST & /*_rReq*/, BOF::BOF_WEB_RESPONSE &_rRes)
{
  BOF_WEB_JSON ResponseJson;
  BOF_WEB_JSON ConfigJson;
  bool Sts_B;

#if defined(ONBINGS_VISTA_ADD_AUTH)
  Sts_B = CheckJwt(_rReq, _rRes);
  if (Sts_B)
  {
    Sts_B = GetConfig(ConfigJson);
  }
#else
  Sts_B = GetConfig(ConfigJson);
#endif
  // To test Sts_B = false;
  if (Sts_B)
  {
    _rRes.status = BOF::BOF_WEB_STATUS::OK_200;
    ResponseJson["code"] = 0;
    ResponseJson["msg"] = "Configuration retrieved";
    ResponseJson["config"] = ConfigJson;
  }
  else
  {
    _rRes.status = BOF::BOF_WEB_STATUS::ServiceUnavailable_503;
    ResponseJson["code"] = -1;
    ResponseJson["msg"] = "Failed to retrieve configuration";
  }

  try
  {
    _rRes.set_content(ResponseJson.dump(), "application/json");
  }
  catch (const std::exception &e)
  {
    LOG_ERROR(mVistaWebServerParam_X.psILogger, "GetConfig: Failed to serialize JSON response: {}", e.what());
    _rRes.status = BOF::BOF_WEB_STATUS::InternalServerError_500;
    _rRes.set_content(R"({"code": -999, "msg": "JSON serialization error"})", "application/json");
  }
}

// ---------------------------------------------------------------------------
// Business stub - GetConfig
// ---------------------------------------------------------------------------

bool VistaWebServer::GetConfig(BOF_WEB_JSON &_rConfigJson)
{
  bool Rts_B = false;

  LOG_INFO(mVistaWebServerParam_X.psILogger, "GetConfig");
  Rts_B = true;
  _rConfigJson["dummy"] = false;
  return Rts_B;
}

// ===========================================================================
// GET /*  -  Static asset fallback
// ===========================================================================

void VistaWebServer::GetStaticAsset(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes)
{
  LOG_INFO(mVistaWebServerParam_X.psILogger, "GetStaticAsset: method='{}' path='{}'", _rReq.method, _rReq.path);

  const bool Served_B = mpuStaticFileServer->Serve(_rReq, _rRes);
  LOG_INFO(mVistaWebServerParam_X.psILogger, "GetStaticAsset: served={} status={}", Served_B, static_cast<int>(_rRes.status));
}
#if defined(ONBINGS_VISTA_ADD_AUTH)
// Security
// Define the SECRET_KEY key. In a real application, this should be securely stored and not hard-coded.
const std::string SECRET_KEY = "xts_is_a_great_project_bha";
const uint32_t TOKEN_ALIVE_TIME_IN_SEC = (30 * 60);
const std::string TOKEN_ISSUER = "vista";

bool VistaWebServer::IsLocalAddress(const std::string &_rIpAddress_S)
{
  bool Rts_B = false;

  if ((_rIpAddress_S == "127.0.0.1") || (_rIpAddress_S == "::1"))
  {
    Rts_B = true;
  }
  else
  {
    for (const auto &rNetworkInterface : mListOfNetworkInterface_X)
    {
      if (rNetworkInterface.IpAddress_S == _rIpAddress_S)
      {
        Rts_B = true;
        break;
      }
    }
  }
  return Rts_B;
}
bool VistaWebServer::CheckJwt(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes)
{
  bool Rts_B = false;
  BOF_WEB_JSON ResponseJson;
  std::string Token_S;
  const char *pEnvSecret_c = std::getenv("ONBINGS_VISTA_JWT_SECRET");
  const std::string SecretKey_S = pEnvSecret_c ? pEnvSecret_c : SECRET_KEY;

  LOG_INFO(mpsILogger, "CheckJwt from {}", _rReq.remote_addr);
  // #pragma message("JWT IS DISABLED FOR TEST, REMOVE IT...")
  //   if ((1) || (IsLocalAddress(_rReq.remote_addr))) //(1) Disable auth
  if (IsLocalAddress(_rReq.remote_addr))
  {
    // Local request, skip JWT check
    Rts_B = true;
  }
  else
  {
    // --- JWT Authorization check ---
    auto itAuth = _rReq.headers.find("Authorization");
    if ((itAuth == _rReq.headers.end()) || (itAuth->second.find("Bearer ") != 0))
    {
      _rRes.status = BOF::BOF_WEB_STATUS::Unauthorized_401;
      ResponseJson["code"] = -103;
      ResponseJson["msg"] = "Missing or invalid Authorization header";
    }
    else
    {
      Token_S = itAuth->second.substr(7); // Remove "Bearer " prefix
      try
      {
        auto Decoded = jwt::decode<jwt::traits::nlohmann_json>(Token_S);
        jwt::verify<jwt::traits::nlohmann_json>().allow_algorithm(jwt::algorithm::hs256{SecretKey_S}).with_issuer(TOKEN_ISSUER).verify(Decoded);
        // Optionally: check claims, expiration, etc.
        Rts_B = true;
      }
      catch (const std::exception &e)
      {
        _rRes.status = BOF::BOF_WEB_STATUS::Unauthorized_401;
        ResponseJson["code"] = -104;
        ResponseJson["msg"] = "Invalid or expired token";
      }
    }
    if (!Rts_B)
    {
      try
      {
        _rRes.set_content(ResponseJson.dump(), "application/json");
      }
      catch (const std::exception &e)
      {
        LOG_ERROR(mpsILogger, "Failed to serialize JSON response: {}", e.what());
        _rRes.status = BOF::BOF_WEB_STATUS::InternalServerError_500;
        _rRes.set_content(R"({"code": -999, "msg": "JSON serialization error"})", "application/json");
      }
    }
  }
  return Rts_B;
}

// Pam
static int S_PamConversation(int _NbMsg_i, const struct pam_message **_ppMsg_X, struct pam_response **_ppResp_X, void *pUser)
{
  int Rts_i = PAM_CONV_ERR, i;
  struct pam_response *pReply_X = (struct pam_response *)calloc(_NbMsg_i, sizeof(struct pam_response));
  if (!pReply_X)
  {
    printf("S_PamConversation: unable to calloc NbItem %d Size %zu\n", _NbMsg_i, sizeof(struct pam_response));
  }
  else
  {
    for (i = 0; i < _NbMsg_i; i++)
    {
      if (_ppMsg_X[i]->msg_style == PAM_PROMPT_ECHO_ON)
      {
        printf("S_PamConversation: PAM_PROMPT_ECHO_ON: Return password %s\n", (const char *)pUser);
        pReply_X[i].resp = strdup((const char *)pUser); // Password passed as pUser
      }
    }
    *_ppResp_X = pReply_X;
    printf("S_PamConversation: PAM_SUCCESS\n");

    Rts_i = PAM_SUCCESS;
  }
  return Rts_i;
}

bool VistaWebServer::PamAuthenticate(const std::string &_rUsername_S, const std::string &_rPassword_S, const std::string &_rPamService_S)
{
  pam_handle_t *pPam_X = nullptr;
  struct pam_conv PamConversation_X = {S_PamConversation, (void *)_rPassword_S.c_str()};
  int Sts_i = pam_start(_rPamService_S.c_str(), _rUsername_S.c_str(), &PamConversation_X, &pPam_X);
  LOG_INFO(mpsILogger, "pam_start: Usr '{}' Pw '{}' Svc '{}' -> Sts {}", _rUsername_S, _rPassword_S, _rPamService_S, Sts_i);
  if (Sts_i == PAM_SUCCESS)
  {
    Sts_i = pam_authenticate(pPam_X, 0);
    LOG_INFO(mpsILogger, "pam_authenticate: -> Sts {}", Sts_i);
    if (Sts_i == PAM_SUCCESS)
    {
      Sts_i = pam_acct_mgmt(pPam_X, 0);
      LOG_INFO(mpsILogger, "pam_acct_mgmt: -> Sts {}", Sts_i);
    }
  }
  pam_end(pPam_X, Sts_i);
  LOG_INFO(mpsILogger, "pam_end: -> Sts {}", Sts_i);
  return (Sts_i == PAM_SUCCESS);
}

void VistaWebServer::PostAuth(const BOF::BOF_WEB_REQUEST &_rReq, BOF::BOF_WEB_RESPONSE &_rRes)
{
  BOF_WEB_JSON ResponseJson;
  std::string Username_S, Password_S, PamService_S;
  const char *pEnvSecret_c = std::getenv("ONBINGS_VISTA_JWT_SECRET");
  const std::string SecretKey_S = pEnvSecret_c ? pEnvSecret_c : SECRET_KEY;

  try
  {
    BOF_WEB_JSON RequestJson = BOF_WEB_JSON::parse(_rReq.body);
    auto itUser = RequestJson.find("username");
    auto itPass = RequestJson.find("password");
    auto itPam = RequestJson.find("pam-service");

    if (itUser != RequestJson.end())
    {
      itUser->get_to(Username_S);
    }
    if (itPass != RequestJson.end())
    {
      itPass->get_to(Password_S);
    }
    if (itPam != RequestJson.end())
    {
      itPam->get_to(PamService_S);
    }

    //    if ((Username_S.empty()) || (Password_S.empty()) || (PamService_S.empty()))
    if ((Username_S.empty()) || (PamService_S.empty()))
    {
      _rRes.status = BOF::BOF_WEB_STATUS::BadRequest_400;
      ResponseJson["code"] = -100;
      //      ResponseJson["msg"] = "Missing username, password, or pam-service";
      ResponseJson["msg"] = "Missing username or pam-service";
    }
    // Simple credential check (replace with PAM or your own logic)
    // else if ((Username_S == "xts") && (Password_S == "xts!") && (PamService_S == "xts-service")) // Replace with real check
    else if (PamAuthenticate(Username_S, Password_S, PamService_S))
    {
      // Generate JWT token
      auto Token = jwt::create<jwt::traits::nlohmann_json>()
                       .set_issuer(TOKEN_ISSUER)
                       .set_type("JWS")
                       .set_payload_claim("username", jwt::claim(Username_S))
                       .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(TOKEN_ALIVE_TIME_IN_SEC))
                       .sign(jwt::algorithm::hs256{SecretKey_S});

      _rRes.status = BOF::BOF_WEB_STATUS::OK_200;
      ResponseJson["status"] = "success";
      ResponseJson["token"] = Token;
    }
    else
    {
      _rRes.status = BOF::BOF_WEB_STATUS::Unauthorized_401;
      ResponseJson["code"] = -101;
      ResponseJson["msg"] = "Invalid credentials";
    }
  }
  catch (const std::exception &e)
  {
    _rRes.status = BOF::BOF_WEB_STATUS::BadRequest_400;
    ResponseJson["code"] = -102;
    ResponseJson["msg"] = std::string("Invalid JSON format: ") + e.what();
  }

  try
  {
    _rRes.set_content(ResponseJson.dump(), "application/json");
  }
  catch (const std::exception &e)
  {
    _rRes.status = BOF::BOF_WEB_STATUS::InternalServerError_500;
    _rRes.set_content(R"({"status": "error", "msg": "JSON serialization error"})", "application/json");
  }
}
#endif
END_VISTA_NAMESPACE()
