/*
 * Copyright (c) 2024-2044, OnBings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module tests the VISTA engine: Vision Intelligence, Statistics, and Tracking Analysis
 *
 * Author:      Bernard HARMEL: b.harmel@gmail.com
 *
 * History:
 * V 1.00  September 25 2026  BHA : Initial release
 */

#include "onbings-vista/vista_engine.h"
#include "evs-hwfw-chip/printf_logger.h"
#include "gtestrunner.h"

// Test fixture for VistaEngine tests
class onbings_vista_engine_tests : public ::testing::Test
{
protected:
  void SetUp() override
  {
    printf("Starting VistaEngine service (v) %s\n", VISTA::VistaEngine::S_GetVersion());
    std::shared_ptr<CHIP::PrintfLogFactory> psFactory = std::make_shared<CHIP::PrintfLogFactory>(ILF::ILogger::LogSeverity::LOG_SEVERITY_DEBUG);
    std::shared_ptr<EVS::HwFw::Ilf::ILogger> psLogger = psFactory->create("Vista", "Engine");

    mVistaEngineParam_X.psILogger = psLogger;
    mpsVistaEngine = VISTA::VistaEngine::S_VistaEngineFactory(mVistaEngineParam_X);
  }

  VISTA::VISTA_ENGINE_PARAM mVistaEngineParam_X;
  std::shared_ptr<VISTA::VistaEngine> mpsVistaEngine;
};
// Deploy and run engin on a xts system with vista service running to run this test
TEST_F(onbings_vista_engine_tests, TalkToRunningService)
{
}

TEST_F(onbings_vista_engine_tests, EngineNewDelete)
{
  int i;

  EXPECT_TRUE(mpsVistaEngine);
  for (i = 0; i < 10; i++)
  {
    mpsVistaEngine.reset();
    mVistaEngineParam_X.psILogger = nullptr;

    mpsVistaEngine = VISTA::VistaEngine::S_VistaEngineFactory(mVistaEngineParam_X);
    EXPECT_TRUE(mpsVistaEngine);
  }
}

// Need hbuf&mailbox->Not under tc
TEST_F(onbings_vista_engine_tests, EngineStartStop)
{
  int i;
  EXPECT_TRUE(mpsVistaEngine);
  for (i = 0; i < 10; i++)
  {
    EXPECT_EQ(mpsVistaEngine->StartEngine(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // std::this_thread::sleep_for(std::chrono::seconds(1000000));
    EXPECT_EQ(mpsVistaEngine->StopEngine(), 0);
  }
}