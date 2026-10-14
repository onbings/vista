/*
 * Copyright (c) 2024-2044, OnBings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module implement the VISTA engine service: Vision Intelligence, Statistics, and Tracking Analysis
 *
 * Author:      Bernard HARMEL: b.harmel@gmail.com
 *
 * History:
 * V 1.00  September 25 2026  BHA : Initial release
 */
#include "vista_daemon.h"
#include <memory>
#include <filesystem>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <bofstd/bofcommandlineparser.h>

struct VISTA_SERVICE_PARAM
{
  bool AskHelp_B;
  ILF::ILogger::LogSeverity LogLevel_E;

  VISTA::VISTA_ENGINE_PARAM VistaEngineParam_X;

  VISTA_SERVICE_PARAM()
  {
    Reset();
  }

  void Reset()
  {
    AskHelp_B = false;
    LogLevel_E = ILF::ILogger::LogSeverity::LOG_SEVERITY_EMERG;

    VistaEngineParam_X.Reset();
  }
};
static VISTA_SERVICE_PARAM S_VistaServiceParam_X;
static std::vector<BOF::BOFPARAMETER> S_pCommandLineOption_X = {
    // General Options
    {nullptr, std::string("h"), std::string("Ask help."), "", "", BOF::BOFPARAMETER_ARG_FLAG::NONE, BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.AskHelp_B, BOOL, true, 0)},
    {nullptr, std::string("help"), std::string("Ask help."), "", "", BOF::BOFPARAMETER_ARG_FLAG::NONE, BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.AskHelp_B, BOOL, true, 0)},
    {nullptr, std::string("ll"), std::string("Specifies app log level value."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.LogLevel_E, UINT32, 0, ILF::ILogger::LogSeverity::LOG_SEVERITY_MAX - 1)},

    // WebServerParam_X Options
    {nullptr, "lis",
     "Specifies the server ip address to listen on or the ip address to which "
     "the client must connect to as 127.0.0.1:8080 or 0.0.0.0:8090.",
     "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG | BOF::BOFPARAMETER_ARG_FLAG::IP_FORMAT_SCHEME | BOF::BOFPARAMETER_ARG_FLAG::IP_FORMAT_PORT,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.ListenIpAddress_X, IPV4, 0, 0)},
    {nullptr, std::string("cp"), std::string("Specifies certificate path if you need https."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.CertificatePath_S, STDSTRING, 0, 0)},
    {nullptr, std::string("pkp"), std::string("Specifies private key path if you need https."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.PrivateKeyPath_S, STDSTRING, 0, 0)},
    {nullptr, std::string("l"), std::string("Log web request and response."), "", "", BOF::BOFPARAMETER_ARG_FLAG::NONE,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.LogRequestAndResponse_B, BOOL, true, 0)},
    {nullptr, std::string("kac"), std::string("Specifies keep alive count."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.KeepAliveMaxCount_U32, UINT32, 1, 0xFFFFFFFF)},
    {nullptr, std::string("kato"), std::string("Specifies keep alive timeout in ms."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.KeepAliveTimeoutInMs_U32, UINT32, 0, 0xFFFFFFFF)},
    {nullptr, std::string("rto"), std::string("Specifies read timeout in ms."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.ReadTimeoutInMs_U32, UINT32, 1, 0xFFFFFFFF)},
    {nullptr, std::string("wto"), std::string("Specifies write timeout in ms."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.WriteTimeoutInMs_U32, UINT32, 1, 0xFFFFFFFF)},
    {nullptr, std::string("ito"), std::string("Specifies idle timeout in ms."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.IdleIntervalInMs_U32, UINT32, 1, 0xFFFFFFFF)},
    {nullptr, std::string("pml"), std::string("Specifies the payload max length in byte."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.PayloadMaxLengthInByte_U32, UINT32, 0, 0xFFFFFFFF)},
    {nullptr, std::string("tps"), std::string("Specifies the server thread pool size."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.ThreadPoolSize_U32, UINT32, 0, 0xFFFFFFFF)},
    {nullptr, std::string("rd"), std::string("Specifies the server root dir"), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.RootDir_S, STDSTRING, 0, 0)},
    {nullptr, std::string("app"), std::string("Specifies the web app name."), "", "", BOF::BOFPARAMETER_ARG_FLAG::CMDLINE_LONGOPT_NEED_ARG,
     BOF_PARAM_DEF_VARIABLE(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.WebAppParam_X.AppName_S, STDSTRING, 0, 0)},
};
// Global and static var
static std::unique_ptr<VistaDaemon> S_puVistaDaemon = nullptr;
// Original argv, captured at startup so RestartApplication() can re-exec the process with the exact
// same command line (execv needs a null-terminated argv; argv[argc] is guaranteed NULL by the C runtime).
static char **S_ppArgv_c = nullptr;

void CleanUp()
{
  // LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Shutdown system...");
  S_puVistaDaemon->Stop();
  BOF::Bof_Shutdown();
}

#if defined(_WIN32)
void SetupSignalHandlers()
{
}
void RestartApplication()
{
}
#else
// Mark every file descriptor >= 3 as close-on-exec so the upcoming execv() closes them
// automatically: listening sockets (REST 5001, stream 9001), the DRM fd, epoll/eventfds, log files,
// etc. This frees the ports for the fresh process and lets the kernel drop the DRM master when the
// last reference to the DRM fd is closed. stdin/stdout/stderr (0,1,2) are left untouched.
static void S_MarkFileDescriptorsCloseOnExec()
{
  DIR *pDir_X;
  struct dirent *pEntry_X;
  int Fd_i, DirFd_i, Flags_i;

  pDir_X = opendir("/proc/self/fd");
  if (pDir_X)
  {
    DirFd_i = dirfd(pDir_X);
    while ((pEntry_X = readdir(pDir_X)) != nullptr)
    {
      Fd_i = atoi(pEntry_X->d_name);
      if ((Fd_i > 2) && (Fd_i != DirFd_i))
      {
        Flags_i = fcntl(Fd_i, F_GETFD);
        if (Flags_i != -1)
        {
          fcntl(Fd_i, F_SETFD, Flags_i | FD_CLOEXEC);
        }
      }
    }
    closedir(pDir_X);
  }
}

// Re-exec the current process image to bring the application back to its pristine startup state
// (the web "Reset" button). This is the only "restart" that behaves identically whether the service
// was launched by systemd (as an AppImage) or from a bare CLI: the PID is preserved so no supervisor
// is required, and execv() never returns on success.
// We deliberately do NOT run the C++ teardown here: destroying the engine/web server joins dozens of
// threads and terminate()s on shutdown. execv() atomically discards every thread and all memory, so
// destructors are unnecessary. The only cross-exec state that matters is open file descriptors, which
// we mark close-on-exec first. The DRM master was already dropped by RequestApplicationRestart()
// (SetDisplayActive(false)); closing the DRM fd on exec guarantees its release regardless.
void RestartApplication()
{
  const char *pExe_c;

  LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Restart requested: re-exec (marking fds close-on-exec)...");
  LogToDmesg("Vista restart requested: re-exec");

  S_MarkFileDescriptorsCloseOnExec();

  // Prefer the AppImage path when launched as an AppImage (systemd), fall back to the real binary
  // for a plain CLI launch.
  pExe_c = getenv("APPIMAGE");
  if ((pExe_c == nullptr) || (pExe_c[0] == '\0'))
  {
    pExe_c = "/proc/self/exe";
  }
  execv(pExe_c, S_ppArgv_c);
  // execv() only returns on failure.
  LogToDmesg("Vista restart failed: execv('%s') -> %s", pExe_c, strerror(errno));
}

// man 7 signal-safety: avoid LOG_INFO in signal handler....
void SignalHandler(int _Signal_i)
{
  if (S_puVistaDaemon)
  {
    switch (_Signal_i)
    {
      case SIGINT:
      case SIGTERM:
        // man 7 signal-safety: only request shutdown here (sets a lock-free atomic flag).
        // The daemon's blocking loop polls this flag and exits, then main() runs CleanUp()
        // (Stop()/Bof_Shutdown()) in normal context. Calling CleanUp() directly from the
        // signal handler is unsafe: Stop()/Bof_Shutdown() lock mutexes, free memory and
        // join threads, which is UB in a signal handler and can deadlock.
        S_puVistaDaemon->RequestStop();
        break;
      case SIGHUP:
        // LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "SignalHandler: Received SIGHUP - ignoring (could implement config reload)");
        break;
        /*
  By adding a SIGCHLD signal handler, you ensure that any time a child process
  exits, your application automatically cleans up the zombie entry without you
  needing to manually manage a wait() call.
  The flag SA_NOCLDWAIT is set in SetupSignalHandlers for SIGCHLD, so the system
  will not create zombie processes when child processes terminate; the kernel will
  automatically reap them. As a result, you do not need to call wait() or
  waitpid() for your child processes—they will not become zombies.
        */
      case SIGCHLD:
        // LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "SignalHandler: Received SIGCHLD");
        //  while (waitpid((pid_t)-1, 0, WNOHANG) > 0)        {   }
        break;
      default:
        // LOG_WARNING(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "SignalHandler: Received unknown signal: {}", _Signal_i);
        break;
    }
  }
}

void SetupSignalHandlers()
{
  struct sigaction Sa_X;
  LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Setup Signal Handler for SIGINT, SIGTERM and SIGHUP");

  Sa_X.sa_handler = SignalHandler;
  // initializes the signal mask to be empty, meaning no signals will be blocked
  // while the signal handler is executing.
  sigemptyset(&Sa_X.sa_mask);
  Sa_X.sa_flags = SA_RESTART; // 0;

  sigaction(SIGINT, &Sa_X, nullptr);
  sigaction(SIGTERM, &Sa_X, nullptr);
  sigaction(SIGHUP, &Sa_X, nullptr);

  // Do NOT install any SIGCHLD disposition here (no handler, no SIG_IGN, no
  // SA_NOCLDWAIT). This process never forks children of its own, so there are
  // no zombies to reap. Crucially, SIG_IGN / SA_NOCLDWAIT make the kernel
  // auto-reap EVERY child, which breaks the waitpid()/system() that the Axelera
  // runtime uses to fork riscv64-unknown-elf-gcc when loading a multi-core
  // model: that waitpid() fails with ECHILD, system() returns -1, and
  // zeModuleCreate reports "Command failed (-1)". Leaving SIGCHLD at its default
  // (SIG_DFL) lets the runtime reap its own compiler child normally.

  /*
This flag is only meaningful for SIGCHLD. When set, the system will not create
zombie processes when child processes terminate; the kernel will automatically
reap them. As a result, you do not need to call wait() or waitpid() for your
child processes—they will not become zombies.
*/
  // Sa_X.sa_flags = SA_RESTART | SA_NOCLDWAIT;
  // sigaction(SIGCHLD, &Sa_X, nullptr);
}
#endif

#define VISTA_ON_CORE 15 // Need max perf
int main(int _Argc_i, char *_pArgv_c[])
{
  int Rts_i = 1;
  BOFERR Sts_E;
  BOF::BOFSTDPARAM StdParam_X;
  BOF::BofPath AppName(_pArgv_c[0]);
  BOF::BofCommandLineParser CmdLineParser;
  BOF::BofPath Cwd;
  std::string HelpString_S, AppName_S, Arg_S, Output_S;
  const char *pFile_c;
  BOF::BOF_SOCKET_ADDRESS_COMPONENT Endpoint_X;
  char pBuffer_c[0x100];
  std::shared_ptr<ILF::Backend::SpdLogManager> psLoggerManager;
  std::shared_ptr<ILF::Backend::SpdLogFactory> psLoggerFactory;
  bool HasCertPath_B, HasKeyPath_B, CertExists_B, KeyExists_B;

  ILF::Backend::SpdLogManager::Config LoggerGlobalWebServerParam_X{
      .msg_queue_size = 1024,                     // Number of messages slots in queue (a message
                                                  // slot is 512 bytes)
      .global_log_name = "Vista_All",             // Set a name if you want a global log channel
                                                  // (empty means no global channel)
      .global_log_max_log_size = 4 * 1024 * 1024, // Maximum size in bytes of a log of the global channel
      .global_log_max_nb_of_logs = 4              // Maximum number of logs rotation of the global channel
  };
  ILF::Backend::SpdLogFactory::Config LoggerFactoryWebServerParam_X{
      .to_file = true,                                               // Log to a file
      .to_stdout = true,                                             // Log to stdout
      .to_syslog = false,                                            // Log to syslog (if supported)
      .to_systemd = false,                                           // Log to systemd (if supported)
      .to_global_log = true,                                         // Log to a global log file (if defined in global config)
      .max_log_size = 8 * 1024 * 1024,                               // Maximum size in bytes of a log
      .max_nb_of_logs = 2,                                           // Maximum number of logs
      .default_level = ILF::ILogger::LogSeverity::LOG_SEVERITY_DEBUG // Default level of the created logger
  };

  // Set CPU affinity to E-core 16
  S_ppArgv_c = _pArgv_c;
#if defined(_WIN32)
  psLoggerManager = std::make_shared<ILF::Backend::SpdLogManager>("C:\\tmp", LoggerGlobalWebServerParam_X);
#else
  cpu_set_t CpuSet_X;
  CPU_ZERO(&CpuSet_X);
  CPU_SET(VISTA_ON_CORE, &CpuSet_X);
  if (sched_setaffinity(0, sizeof(cpu_set_t), &CpuSet_X) == 0)
  {
    printf("Successfully bound process to E-core %d\n", VISTA_ON_CORE);
  }
  else
  {
    printf("Warning: Failed to set CPU affinity to core %d\n", VISTA_ON_CORE);
  }
  psLoggerManager = std::make_shared<ILF::Backend::SpdLogManager>("/var/log/evs", LoggerGlobalWebServerParam_X);
#endif
  if (psLoggerManager)
  {
    psLoggerFactory = std::make_shared<ILF::Backend::SpdLogFactory>(psLoggerManager, LoggerFactoryWebServerParam_X);
    if (psLoggerFactory)
    {
      S_VistaServiceParam_X.VistaEngineParam_X.psILogger = psLoggerFactory->create("Vista", "Main");
      if (S_VistaServiceParam_X.VistaEngineParam_X.psILogger)
      {
        S_VistaServiceParam_X.VistaEngineParam_X.psILogger->set_severity(ILF::ILogger::LogSeverity::LOG_SEVERITY_DEBUG);
        Sts_E = BOF::Bof_Initialize(StdParam_X);
        if (BOF::Bof_GetCurrentAppDirectory(Cwd) == BOF_ERR_NO_ERROR)
        {
          Sts_E = BOF::Bof_SetCurrentDirectory(Cwd);
        }
        LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "\nPwd {}\nRunning BofStd V {} on {} under {} (Err {})", Cwd.FullPathName(false), StdParam_X.Version_S, StdParam_X.ComputerName_S,
                 StdParam_X.OsName_S, BOF::Bof_ErrorCode(Sts_E));
        assert(Sts_E == BOF_ERR_NO_ERROR);
        if (Sts_E == BOF_ERR_NO_ERROR)
        {
          LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Set application's argument default value");
          // App default value different from .Reset()

          S_VistaServiceParam_X.LogLevel_E = ILF::ILogger::LogSeverity::LOG_SEVERITY_INFO;
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.ServerStartStopTimeoutInMs_U32 = 2000;
          /*
          KeepAliveMaxCount_U32 sets the maximum number of requests that can be handled over a single persistent TCP connection.
          KeepAliveTimeoutInMs_U32 sets the maximum time, in milliseconds, that the server will wait for the next request on a
          persistent connection.
          */
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.KeepAliveMaxCount_U32 = 5000;
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.KeepAliveTimeoutInMs_U32 = 10000;
#pragma message("Fix Cors behaviour in release of the product")
#if defined(NDEBUG)                                                                       // We are in Release compil
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.DisableCors_B = true; // false;
#else
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.DisableCors_B = true;
#endif
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.ReadTimeoutInMs_U32 = 1000;
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.WriteTimeoutInMs_U32 = 1000;
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.IdleIntervalInMs_U32 = 2000;
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.PayloadMaxLengthInByte_U32 = 0x100000;
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.ThreadPoolSize_U32 = 8;

          AppName_S = AppName.FullPathName(false);
          BOF_GET_FILE_FROM_PATH(AppName_S.c_str(), pFile_c);
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.WebAppParam_X.AppName_S = pFile_c;
          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.RootDir_S = Cwd.FullPathName(false) + "www/";
          LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "{} argument specified", _Argc_i);
          if (_Argc_i == 1)
          {
#if _WIN32
            snprintf(pBuffer_c, sizeof(pBuffer_c), "--lis=tcp://0.0.0.0:7000;--cp=../assets/ssl/certs/vista-cert.crt;--pkp=../assets/ssl/private/vista-cert.key");
            snprintf(pBuffer_c, sizeof(pBuffer_c), "--lis=tcp://0.0.0.0:7000"); // Not secure (to test using swagger)
#else
            snprintf(pBuffer_c, sizeof(pBuffer_c), "--lis=tcp://0.0.0.0:7000;--cp=./assets/ssl/certs/vista-cert.crt;--pkp=./assets/ssl/private/vista-cert.key");
#endif
            LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Force command line to '{}'", pBuffer_c);
            CmdLineParser.ToByte(pBuffer_c, S_pCommandLineOption_X, nullptr, nullptr);
          }
          else
          {
            CmdLineParser.ToByte(_Argc_i, _pArgv_c, S_pCommandLineOption_X, nullptr, nullptr);
          }

          // HTTPS requires both files. If one is missing, disable HTTPS cleanly.
          HasCertPath_B = !S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.CertificatePath_S.empty();
          HasKeyPath_B = !S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.PrivateKeyPath_S.empty();
          CertExists_B = false;
          KeyExists_B = false;
          if ((HasCertPath_B) || (HasKeyPath_B))
          {
            CertExists_B = HasCertPath_B && std::filesystem::exists(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.CertificatePath_S);
            KeyExists_B = HasKeyPath_B && std::filesystem::exists(S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.PrivateKeyPath_S);
            if (!(CertExists_B && KeyExists_B))
            {
              LOG_WARNING(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "HTTPS disabled: certificate/key file missing (cert='{}' exists={} key='{}' exists={})",
                          S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.CertificatePath_S, CertExists_B, S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.PrivateKeyPath_S,
                          KeyExists_B);
              S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.CertificatePath_S.clear();
              S_VistaServiceParam_X.VistaEngineParam_X.WebServerParam_X.PrivateKeyPath_S.clear();
            }
          }
          LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "HTTP(S) Status: cert_path_present={} key_path_present={} cert_exists={} key_exists={}", HasCertPath_B, HasKeyPath_B,
                   CertExists_B, KeyExists_B);
          CmdLineParser.BuildHelpString(S_pCommandLineOption_X, AppName.FileNameWithoutExtension() + '\n', HelpString_S);

          LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Vista Service is starting...");
          LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Cli:\n{}", HelpString_S);

          if (S_VistaServiceParam_X.AskHelp_B)
          {
            LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "App working parameters:\n{}", HelpString_S);
          }
          else
          {
            S_VistaServiceParam_X.VistaEngineParam_X.psILogger->set_severity(S_VistaServiceParam_X.LogLevel_E);
            // Setup signal handling
            SetupSignalHandlers();
            //  Create daemon instance
            S_puVistaDaemon = std::make_unique<VistaDaemon>(S_VistaServiceParam_X.VistaEngineParam_X);
            // LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Create daemon {}", static_cast<void *>(S_puVistaDaemon.get()));
            if (S_puVistaDaemon)
            {
              Rts_i = 2;

              // Initialize
              LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Start Daemon");
              if (S_puVistaDaemon->Start())
              {
                Rts_i = 0;
              }
              else
              {
                LOG_ERROR(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Failed to start daemon");
              }
            }
            else
            {
              LOG_ERROR(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Unable to create Vista Daemon");
            }
          } // else  if (S_VistaServiceParam_X.AskHelp_B)
          LOG_INFO(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Clean up resources...");
          if (S_puVistaDaemon && S_puVistaDaemon->IsRestartRequested())
          {
            RestartApplication(); // re-exec back to initial state; only returns if execv() failed
          }
          CleanUp();
        }
        else
        {
          LOG_ERROR(S_VistaServiceParam_X.VistaEngineParam_X.psILogger, "Unable to initialize Bof ({})", BOF::Bof_ErrorCode(Sts_E));
        }
      }
      else
      {
        printf("Logger creation failed\n");
      }
    }
    else
    {
      printf("Logger factory creation failed\n");
    }
  }
  else
  {
    printf("Logger manager creation failed\n");
  }

  return Rts_i;
}
