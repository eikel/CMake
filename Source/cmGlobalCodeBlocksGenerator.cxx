/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Benjamin Eikel

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmGlobalCodeBlocksGenerator.h"

cmGlobalCodeBlocksGenerator::cmGlobalCodeBlocksGenerator()
{
  this->FindMakeProgramFile = "CMakeFindCodeBlocks.cmake";
}

cmGlobalCodeBlocksGenerator::~cmGlobalCodeBlocksGenerator()
{
}

void cmGlobalCodeBlocksGenerator::GetDocumentation(
  cmDocumentationEntry & entry)
{
  entry.Name = cmGlobalCodeBlocksGenerator::GetActualName();
  entry.Brief = "Generates projects for the Code::Blocks IDE.";
  entry.Full =
    "A project file for the Code::Blocks IDE is generated into the build "
    "tree.";
}

std::string cmGlobalCodeBlocksGenerator::GenerateBuildCommand(
  const char * makeProgram,
  const char * projectName,
  const char * additionalOptions,
  const char * targetName,
  const char * /*config*/,
  bool /*ignoreErrors*/,
  bool /*fast*/)
{
  if (makeProgram == NULL || strlen(makeProgram) == 0)
    {
    cmSystemTools::Error(
      "Generator cannot find the appropriate make command.");
    return "";
    }
  std::string makeCommand =
    cmSystemTools::ConvertToOutputPath(makeProgram);

  makeCommand += " --no-splash-screen";
#if defined (_WIN32)
  makeCommand += " --no-check-associations --no-dde";
#else
  makeCommand += " --no-ipc";
#endif

  if (targetName != NULL)
    {
    if (strcmp(targetName, "clean") == 0)
      {
      makeCommand += " --clean";
      }
    else if (strlen(targetName) != 0)
      {
      makeCommand += " --build --target=";
      makeCommand += targetName;
      }
    }

  if (additionalOptions)
    {
    makeCommand += ' ';
    makeCommand += additionalOptions;
    }

  makeCommand += ' ';
  makeCommand += projectName;
  makeCommand += ".cbp";
  return makeCommand;
}

void cmGlobalCodeBlocksGenerator::Generate()
{
}
