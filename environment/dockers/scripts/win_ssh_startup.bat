@echo off

:: This scripts works in concert with docker_run.py which mounts
:: The user .ssh directory as a read-only directory if it exists
:: The user on the host will be identified by the environment variable
:: DOCKER_USER which is also set by docker_run.py. Thanks to that,
:: this script can detect that this directory exists and copy it
:: to the container user (ContainerAdministrator) so that is gets
:: the proper ssh credentials
if exist C:\Users\%DOCKER_USER%\.ssh\ (
  robocopy C:\Users\%DOCKER_USER%\.ssh %userprofile%\.ssh /S /E

  if not ERRORLEVEL 0 (
    echo Failed
    exit /B 1
  )
)

:: Because we have plugged-in an additional script
:: We need to relaunch cmd ourself either interactively
:: (if there are no arguments) or non-interactively otherwise

:: Check if there are arguments provided
if ["%~1"]==[""] (
  cmd /c cmd
) else (
  cmd /c %*
)