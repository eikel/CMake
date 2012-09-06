/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Benjamin Eikel

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmGlobalCodeBlocksGenerator_h
#define cmGlobalCodeBlocksGenerator_h

#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"

/** \class cmGlobalCodeBlocksGenerator
 * \brief Write a project for the Code::Blocks IDE
 *
 * Generate a project file for the Code::Blocks IDE for a tree
 */
class cmGlobalCodeBlocksGenerator : public cmGlobalGenerator
{
public:
  cmGlobalCodeBlocksGenerator();

  /// Convenience method for creating an instance of this class.
  static cmGlobalGeneratorFactory* NewFactory() {
    return new cmGlobalGeneratorSimpleFactory<cmGlobalCodeBlocksGenerator>(); }

  virtual ~cmGlobalCodeBlocksGenerator();

  static cmGlobalGenerator * New()
  {
    return new cmGlobalCodeBlocksGenerator;
  }

  static const char * GetActualName()
  {
    return "CodeBlocks";
  }

  ///! Get the name for this generator
  virtual const char * GetName() const
  {
    return this->GetActualName();
  }

  ///! Get the documentation entry for this generator.
  static void GetDocumentation(cmDocumentationEntry& entry);

  virtual std::string GenerateBuildCommand(const char * makeProgram,
                                           const char * projectName,
                                           const char * additionalOptions,
                                           const char * targetName,
                                           const char * config,
                                           bool ignoreErrors,
                                           bool fast);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.
   */
  virtual void Generate();
};

#endif
