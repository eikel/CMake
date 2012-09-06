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

void cmGlobalCodeBlocksGenerator::Generate()
{
}
