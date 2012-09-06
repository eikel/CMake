/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)
  Copyright 2012 Benjamin Eikel

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmGlobalCodeBlocksGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmXMLSafe.h"

/* CBTree is used to create a "Virtual Folder" in CodeBlocks, in which all
 CMake files this project depends on will be put. This means additionally
 to the "Sources" and "Headers" virtual folders of CodeBlocks, there will
 now also be a "CMake Files" virtual folder.
 Patch by Daniel Teske <daniel.teske AT nokia.com> (which use C::B project
 files in QtCreator).*/
struct CBTree
{
  std::string path; //only one component of the path
  std::vector<CBTree> folders;
  std::vector<std::string> files;
  void InsertPath(const std::vector<std::string> & splitted,
                  std::vector<std::string>::size_type start,
                  const std::string & fileName);
  void BuildVirtualFolder(std::string & virtualFolders) const;
  void BuildVirtualFolderImpl(std::string & virtualFolders,
                              const std::string & prefix) const;
  void BuildUnit(std::string & unitString, const std::string & fsPath) const;
  void BuildUnitImpl(std::string & unitString,
                     const std::string & virtualFolderPath,
                     const std::string & fsPath) const;
};


void CBTree::InsertPath(const std::vector<std::string> & splitted,
                        std::vector<std::string>::size_type start,
                        const std::string & fileName)
{
  if (start == splitted.size())
    {
    files.push_back(fileName);
    return;
    }
  for (std::vector<CBTree>::iterator
       it = folders.begin();
       it != folders.end();
       ++it)
    {
    if ((*it).path == splitted[start])
      {
      if (start + 1 < splitted.size())
        {
        it->InsertPath(splitted, start + 1, fileName);
        return;
        }
      else
        {
        // last part of splitted
        it->files.push_back(fileName);
        return;
        }
      }
    }
  // Not found in folders, thus insert
  CBTree newFolder;
  newFolder.path = splitted[start];
  if (start + 1 < splitted.size())
    {
    newFolder.InsertPath(splitted, start + 1, fileName);
    folders.push_back(newFolder);
    return;
    }
  else
    {
    // last part of splitted
    newFolder.files.push_back(fileName);
    folders.push_back(newFolder);
    return;
    }
}


void CBTree::BuildVirtualFolder(std::string & virtualFolders) const
{
  virtualFolders += "<Option virtualFolders=\"CMake Files\\;";
  for (std::vector<CBTree>::const_iterator it = folders.begin();
       it != folders.end();
       ++it)
    {
    it->BuildVirtualFolderImpl(virtualFolders, "");
    }
  virtualFolders += "\" />";
}


void CBTree::BuildVirtualFolderImpl(std::string & virtualFolders,
                                    const std::string & prefix) const
{
  virtualFolders += "CMake Files\\" + prefix + path + "\\;";
  for (std::vector<CBTree>::const_iterator it = folders.begin();
       it != folders.end();
       ++it)
    {
    it->BuildVirtualFolderImpl(virtualFolders, prefix + path + "\\");
    }
}


void CBTree::BuildUnit(std::string & unitString,
                       const std::string & fsPath) const
{
  for (std::vector<std::string>::const_iterator it = files.begin();
       it != files.end();
       ++it)
    {
    unitString += "      <Unit filename=\"" + fsPath + *it + "\">\n";
    unitString += "          <Option virtualFolder=\"CMake Files\\\" />\n";
    unitString += "      </Unit>\n";
    }
  for (std::vector<CBTree>::const_iterator it = folders.begin();
       it != folders.end();
       ++it)
    {
    it->BuildUnitImpl(unitString, "", fsPath);
    }
}


void CBTree::BuildUnitImpl(std::string & unitString,
                           const std::string & virtualFolderPath,
                           const std::string & fsPath) const
{
  for (std::vector<std::string>::const_iterator it = files.begin();
       it != files.end();
       ++it)
    {
    unitString += "      <Unit filename=\"" + fsPath + path + "/" + *it +
                  "\">\n";
    unitString += "          <Option virtualFolder=\"CMake Files\\" +
                  virtualFolderPath + path + "\\\" />\n";
    unitString += "      </Unit>\n";
    }
  for (std::vector<CBTree>::const_iterator it = folders.begin();
       it != folders.end();
       ++it)
    {
    it->BuildUnitImpl(unitString,
                      virtualFolderPath + path + "\\", fsPath + path + "/");
    }
}


cmGlobalCodeBlocksGenerator::cmGlobalCodeBlocksGenerator()
{
  this->FindMakeProgramFile = "CMakeCodeBlocksFindMake.cmake";
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
  this->cmGlobalGenerator::Generate();
  if (cmSystemTools::GetErrorOccuredFlag())
    {
    return;
    }
  std::map<cmStdString, std::vector<cmLocalGenerator *> >::iterator it;
  for (it = this->ProjectMap.begin(); it != this->ProjectMap.end(); ++it)
    {
    this->OutputCodeBlocksProject(it->second);
    }
}

void cmGlobalCodeBlocksGenerator::OutputCodeBlocksProject(
  const std::vector<cmLocalGenerator *> & localGenerators)
{
  if (localGenerators.empty())
    {
    return;
    }
  const cmMakefile * makefile = localGenerators[0]->GetMakefile();
  const std::string outputDir = makefile->GetStartOutputDirectory();
  const std::string projectName = makefile->GetProjectName();

  const std::string projectFileName = outputDir + "/" + projectName + ".cbp";

  cmGeneratedFileStream fout(projectFileName.c_str());
  if (!fout)
    {
    return;
    }

  CBTree tree;

  // build tree of virtual folders
  for (std::map<cmStdString, std::vector<cmLocalGenerator *> >::const_iterator
       it = this->ProjectMap.begin();
       it != this->ProjectMap.end();
       ++it)
    {
    // Collect all files
    std::vector<std::string> listFiles;
    for (std::vector<cmLocalGenerator *>::const_iterator
         jt = it->second.begin();
         jt != it->second.end();
         ++jt)
      {
      const std::vector<std::string> & files =
        (*jt)->GetMakefile()->GetListFiles();
      listFiles.insert(listFiles.end(), files.begin(), files.end());
      }

    // Convert
    const char * cmakeRoot = makefile->GetDefinition("CMAKE_ROOT");
    for (std::vector<std::string>::const_iterator jt = listFiles.begin();
         jt != listFiles.end();
         ++jt)
      {
      // don't put cmake's own files into the project (#12110):
      if (jt->find(cmakeRoot) == 0)
        {
        continue;
        }

      const std::string & relative = cmSystemTools::RelativePath(
        it->second[0]->GetMakefile()->GetHomeDirectory(),
        jt->c_str());
      std::vector<std::string> splitted;
      cmSystemTools::SplitPath(relative.c_str(), splitted, false);
      // Split filename from path
      std::string fileName = *(splitted.end() - 1);
      splitted.erase(splitted.end() - 1, splitted.end());

      // We don't want paths with CMakeFiles in them
      // or do we?
      // In speedcrunch those where purely internal
      if (splitted.size() >= 1 &&
          relative.find("CMakeFiles") == std::string::npos)
        {
        tree.InsertPath(splitted, 1, fileName);
        }
      }
    }

  // Now build a virtual tree string
  std::string virtualFolders;
  tree.BuildVirtualFolder(virtualFolders);
  // And one for <Unit>
  std::string unitFiles;
  tree.BuildUnit(unitFiles, std::string(makefile->GetHomeDirectory()) + "/");

  // figure out the compiler
  std::string compiler = this->GetCodeBlocksCompilerId(makefile);
  std::string make = makefile->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");

  fout << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n"
  "<CodeBlocks_project_file>\n"
  "   <FileVersion major=\"1\" minor=\"6\" />\n"
  "   <Project>\n"
  "      <Option title=\"" << makefile->GetProjectName() << "\" />\n"
  "      <Option makefile_is_custom=\"0\" />\n"
  "      <Option compiler=\"" << compiler << "\" />\n"
  "      " << virtualFolders << "\n"
  "      <Build>\n";

  // Add a target for running CMake
  const char * cmakeCommand = makefile->GetRequiredDefinition("CMAKE_COMMAND");
  fout << "      <Target title=\"rebuild_cache\">\n"
  "         <Option type=\"4\" />\n" // commands only target
  "         <ExtraCommands>\n"
  "            <Add before=\"" << cmakeCommand << " chdir " <<
  makefile->GetHomeOutputDirectory() <<
  " &amp;&amp; " <<
  cmakeCommand << ' ' << makefile->GetHomeDirectory() << "\" />\n"
  "            <Mode after=\"always\" />\n"
  "         </ExtraCommands>\n"
  "      </Target>\n";

  std::vector<std::string> virtualTargetDeps;
  // add all executable and library targets and some of the GLOBAL
  // and UTILITY targets
  for (std::vector<cmLocalGenerator *>::const_iterator lg =
         localGenerators.begin();
       lg != localGenerators.end(); lg++)
    {
    cmMakefile * makefile = (*lg)->GetMakefile();
    cmTargets & targets = makefile->GetTargets();
    for (cmTargets::iterator ti = targets.begin();
         ti != targets.end(); ti++)
      {
      switch (ti->second.GetType())
        {
        case cmTarget::UTILITY:
          // Add all utility targets, except the Nightly/Continuous/
          // Experimental-"sub"targets as e.g. NightlyStart
          if (((ti->first.find("Nightly") == 0) &&
               (ti->first != "Nightly")) ||
              ((ti->first.find("Continuous") == 0) &&
               (ti->first != "Continuous")) ||
              ((ti->first.find("Experimental") == 0) &&
               (ti->first != "Experimental")))
            {
            break;
            }

          this->AppendTarget(fout, ti->first.c_str(), 0,
                             make.c_str(), makefile, compiler.c_str());
          break;
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
          {
          this->AppendTarget(fout, ti->first.c_str(), &ti->second,
                             make.c_str(), makefile, compiler.c_str());
          virtualTargetDeps.push_back(ti->first);
          }
          break;
        default:
          break;
        }
      }
    }

  fout << "      </Build>\n";

  fout << "      <VirtualTargets>\n";
  fout << "         <Add alias=\"All\" targets=\"";
  std::vector<std::string>::const_iterator dep;
  for (dep = virtualTargetDeps.begin(); dep != virtualTargetDeps.end(); ++dep)
    {
    fout << *dep << ';';
    }
  fout << "\" />\n";
  fout << "      </VirtualTargets>\n";

  // Collect all used source files in the project
  // Sort them into two containers, one for C/C++ implementation files
  // which may have an acompanying header, one for all other files
  std::map<std::string, std::deque<std::string> > cFiles;
  std::set<std::string> otherFiles;
  for (std::vector<cmLocalGenerator *>::const_iterator lg =
         localGenerators.begin();
       lg != localGenerators.end(); lg++)
    {
    cmMakefile * makefile = (*lg)->GetMakefile();
    cmTargets & targets = makefile->GetTargets();
    for (cmTargets::iterator ti = targets.begin();
         ti != targets.end(); ti++)
      {
      switch (ti->second.GetType())
        {
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
        case cmTarget::UTILITY: // can have sources since 2.6.3
          {
          const std::vector<cmSourceFile *> & sources =
            ti->second.GetSourceFiles();
          std::vector<cmSourceFile *>::const_iterator si;
          for (si = sources.begin(); si != sources.end(); si++)
            {
            // don't add source files which have the GENERATED property set:
            if ((*si)->GetPropertyAsBool("GENERATED"))
              {
              continue;
              }

            // check whether it is a C/C++ implementation file
            bool isCFile = false;
            if ((*si)->GetLanguage() && (*(*si)->GetLanguage() == 'C'))
              {
              for (std::vector<std::string>::const_iterator
                   ext = makefile->GetSourceExtensions().begin();
                   ext != makefile->GetSourceExtensions().end();
                   ++ext)
                {
                if ((*si)->GetExtension() == *ext)
                  {
                  isCFile = true;
                  break;
                  }
                }
              }

            // then put it accordingly into one of the two containers
            if (isCFile)
              {
              const std::string fullPath = (*si)->GetFullPath();
              std::map<std::string, std::deque<std::string> >::iterator cfe =
                cFiles.find(fullPath);
              if (cfe != cFiles.end())
                {
                cfe->second.push_back(ti->first);
                }
              else
                {
                std::deque<std::string> newTargets;
                newTargets.push_back(ti->first);
                cFiles.insert(std::make_pair(fullPath, newTargets));
                }
              }
            else
              {
              otherFiles.insert((*si)->GetFullPath());
              }
            }
          }
        default:  // intended fallthrough
          break;
        }
      }
    }

  // The following loop tries to add header files matching to implementation
  // files to the project. It does that by iterating over all source files,
  // replacing the file name extension with ".h" and checks whether such a
  // file exists. If it does, it is inserted into the map of files.
  // A very similar version of that code exists also in the kdevelop
  // project generator.
  for (std::map<std::string, std::deque<std::string> >::const_iterator
       sit = cFiles.begin();
       sit != cFiles.end();
       ++sit)
    {
    std::string headerBasename = cmSystemTools::GetFilenamePath(sit->first);
    headerBasename += "/";
    headerBasename += cmSystemTools::GetFilenameWithoutExtension(sit->first);

    // check if there's a matching header around
    for (std::vector<std::string>::const_iterator
         ext = makefile->GetHeaderExtensions().begin();
         ext != makefile->GetHeaderExtensions().end();
         ++ext)
      {
      std::string hname = headerBasename;
      hname += ".";
      hname += *ext;
      // if it's already in the set, don't check if it exists on disk
      std::set<std::string>::const_iterator headerIt = otherFiles.find(hname);
      if (headerIt != otherFiles.end())
        {
        break;
        }

      if (cmSystemTools::FileExists(hname.c_str()))
        {
        otherFiles.insert(hname);
        break;
        }
      }
    }

  // insert all source files in the CodeBlocks project
  // first the C/C++ implementation files, then all others
  for (std::map<std::string, std::deque<std::string> >::const_iterator
       sit = cFiles.begin();
       sit != cFiles.end();
       ++sit)
    {
    fout << "      <Unit filename=\"" << sit->first << "\">\n";
    for (std::deque<std::string>::const_iterator taIt = sit->second.begin();
         taIt != sit->second.end();
         ++taIt)
      {
      fout << "         <Option target=\"" << *taIt << "\" />\n";
      }
    fout << "      </Unit>\n";
    }
  for (std::set<std::string>::const_iterator
       sit = otherFiles.begin();
       sit != otherFiles.end();
       ++sit)
    {
    fout << "      <Unit filename=\"" << sit->c_str() << "\">\n"
    "      </Unit>\n";
    }

  // Add CMakeLists.txt
  fout << unitFiles;

  fout << "   </Project>\n"
  "</CodeBlocks_project_file>\n";
}

// Generate the xml code for one target.
void cmGlobalCodeBlocksGenerator::AppendTarget(cmGeneratedFileStream & fout,
                                               const char * targetName,
                                               cmTarget * target,
                                               const char * make,
                                               const cmMakefile * makefile,
                                               const char * compiler)
{
  std::string makefileName = makefile->GetStartOutputDirectory();
  makefileName += "/Makefile";

  fout << "      <Target title=\"" << targetName << "\">\n";
  if (target != 0)
    {
    int cbTargetType = this->GetCodeBlocksTargetType(target);
    std::string workingDir = makefile->GetStartOutputDirectory();
    if (target->GetType() == cmTarget::EXECUTABLE)
      {
      // Determine the directory where the executable target is created, and
      // set the working directory to this dir.
      const char * runtimeOutputDir = makefile->GetDefinition(
        "CMAKE_RUNTIME_OUTPUT_DIRECTORY");
      if (runtimeOutputDir != 0)
        {
        workingDir = runtimeOutputDir;
        }
      else
        {
        const char * executableOutputDir = makefile->GetDefinition(
          "EXECUTABLE_OUTPUT_PATH");
        if (executableOutputDir != 0)
          {
          workingDir = executableOutputDir;
          }
        }
      }

    const char * buildType = makefile->GetDefinition("CMAKE_BUILD_TYPE");
    std::string location;
    if (target->GetType() == cmTarget::OBJECT_LIBRARY)
      {
      location = this->CreateDummyTargetFile(
        const_cast<cmMakefile *>(makefile), target);
      }
    else
      {
      location = target->GetLocation(buildType);
      }

    fout << "         <Option output=\"" << location <<
    "\" prefix_auto=\"0\" extension_auto=\"0\" />\n"
    "         <Option working_dir=\"" << workingDir << "\" />\n"
    "         <Option object_output=\"./\" />\n"
    "         <Option type=\"" << cbTargetType << "\" />\n"
    "         <Option compiler=\"" << compiler << "\" />\n"
    "         <Compiler>\n";

    // the compilerdefines for this target
    const char * cdefs = target->GetMakefile()->GetProperty(
      "COMPILE_DEFINITIONS");
    if (cdefs)
      {
      // Expand the list.
      std::vector<std::string> defs;
      cmSystemTools::ExpandListArgument(cdefs, defs);
      for (std::vector<std::string>::const_iterator di = defs.begin();
           di != defs.end(); ++di)
        {
        cmXMLSafe safedef(di->c_str());
        fout << "            <Add option=\"-D" << safedef.str() << "\" />\n";
        }
      }
    // the compilerdefines for this target
    const char * cdefs2 = target->GetProperty("COMPILE_DEFINITIONS");
    if (cdefs2)
      {
      // Expand the list.
      std::vector<std::string> defs;
      cmSystemTools::ExpandListArgument(cdefs2, defs);
      for (std::vector<std::string>::const_iterator di = defs.begin();
           di != defs.end(); ++di)
        {
        cmXMLSafe safedef(di->c_str());
        fout << "            <Add option=\"-D" << safedef.str() << "\" />\n";
        }
      }
    const char * cflags = target->GetProperty("COMPILE_FLAGS");
    if (cflags)
      {
      // Expand the list.
      std::vector<std::string> flags;
      cmSystemTools::ExpandListArgument(cflags, flags);
      for (std::vector<std::string>::const_iterator fi = flags.begin();
           fi != flags.end(); ++fi)
        {
        cmXMLSafe safeflag(fi->c_str());
        fout << "            <Add option=\"" << safeflag.str() << "\" />\n";
        }
      }

    std::string sharedLibFlagsVar = "CMAKE_SHARED_LIBRARY_CXX_FLAGS";
    if (this->GetLanguageEnabled("CXX") == false)
      {
      sharedLibFlagsVar = "CMAKE_SHARED_LIBRARY_C_FLAGS";
      }
    const char * sldefs = target->GetMakefile()->GetSafeDefinition(
      "CMAKE_SHARED_LIBRARY_CXX_FLAGS");
    if (sldefs)
      {
      // Expand the list.
      std::vector<std::string> defs;
      cmSystemTools::ExpandListArgument(sldefs, defs);
      for (std::vector<std::string>::const_iterator di = defs.begin();
           di != defs.end(); ++di)
        {
        cmXMLSafe safedef(di->c_str());
        fout << "            <Add option=\"" << safedef.str() << "\" />\n";
        }
      }

    // the include directories for this target
    std::set<std::string> uniqIncludeDirs;

    std::vector<std::string> includes;
    target->GetMakefile()->GetLocalGenerator()->
    GetIncludeDirectories(includes, target);
    for (std::vector<std::string>::const_iterator dirIt = includes.begin();
         dirIt != includes.end();
         ++dirIt)
      {
      uniqIncludeDirs.insert(*dirIt);
      }

    std::string systemIncludeDirs = makefile->GetSafeDefinition(
      "CMAKE_EXTRA_GENERATOR_C_SYSTEM_INCLUDE_DIRS");
    if (!systemIncludeDirs.empty())
      {
      std::vector<std::string> dirs;
      cmSystemTools::ExpandListArgument(systemIncludeDirs.c_str(), dirs);
      for (std::vector<std::string>::const_iterator dirIt = dirs.begin();
           dirIt != dirs.end();
           ++dirIt)
        {
        uniqIncludeDirs.insert(*dirIt);
        }
      }

    systemIncludeDirs = makefile->GetSafeDefinition(
      "CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_INCLUDE_DIRS");
    if (!systemIncludeDirs.empty())
      {
      std::vector<std::string> dirs;
      cmSystemTools::ExpandListArgument(systemIncludeDirs.c_str(), dirs);
      for (std::vector<std::string>::const_iterator dirIt = dirs.begin();
           dirIt != dirs.end();
           ++dirIt)
        {
        uniqIncludeDirs.insert(*dirIt);
        }
      }

    for (std::set<std::string>::const_iterator dirIt = uniqIncludeDirs.begin();
         dirIt != uniqIncludeDirs.end();
         ++dirIt)
      {
      fout << "            <Add directory=\"" << dirIt->c_str() << "\" />\n";
      }

    fout << "         </Compiler>\n";
    }
  else   // e.g. all and the GLOBAL and UTILITY targets
    {
    fout << "         <Option working_dir=\"" <<
    makefile->GetStartOutputDirectory() << "\" />\n" <<
    "         <Option type=\"" << 4 << "\" />\n";
    }

  // Add link dependencies
  const std::vector<std::string> & linkDeps =
    target->GetLinkImplementation("")->Libraries;
  if (!linkDeps.empty())
    {
    fout << "         <Linker>\n";
    for (std::vector<std::string>::const_iterator dep = linkDeps.begin();
         dep != linkDeps.end(); ++dep)
      {
      std::string depName = *dep;
      cmTarget * depTarget =
        const_cast<cmMakefile *>(makefile)->FindTargetToUse(dep->c_str());
      if (depTarget)
        {
        const std::string fullPath = depTarget->GetFullPath();
        const std::size_t lastSlash = fullPath.find_last_of('/');
        fout << "            <Add directory=\"" <<
        fullPath.substr(0, lastSlash) << "\" />\n";
        }
      if (depName.substr(0, 2) == "-l")
        {
        depName = depName.substr(2);
        }
      fout << "            <Add library=\"" << depName << "\" />\n";
      }
    fout << "         </Linker>\n";
    }

  fout << "      </Target>\n";

}

std::string cmGlobalCodeBlocksGenerator::CreateDummyTargetFile(
  cmMakefile * mf, cmTarget * target) const
{
  // this file doesn't seem to be used by C::B in custom makefile mode,
  // but we generate a unique file for each OBJECT library so in case
  // C::B uses it in some way, the targets don't interfere with each other.
  std::string filename = mf->GetCurrentOutputDirectory();
  filename += "/";
  filename += mf->GetLocalGenerator()->GetTargetDirectory(*target);
  filename += "/";
  filename += target->GetName();
  filename += ".objlib";
  cmGeneratedFileStream fout(filename.c_str());
  if (fout)
    {
    fout << "# This is a dummy file for the OBJECT library " <<
    target->GetName() <<
    " for the CMake CodeBlocks project generator.\n" <<
    "# Don't edit, this file will be overwritten.\n";
    }
  return filename;
}

std::string cmGlobalCodeBlocksGenerator::GetCodeBlocksCompilerId(
  const cmMakefile * makefile) const
{
  // figure out which language to use
  // for now care only for C and C++
  std::string compilerIdVar = "CMAKE_CXX_COMPILER_ID";
  if (this->GetLanguageEnabled("CXX") == false)
    {
    compilerIdVar = "CMAKE_C_COMPILER_ID";
    }

  const std::string hostSystemName = makefile->GetSafeDefinition(
    "CMAKE_HOST_SYSTEM_NAME");
  const std::string systemName = makefile->GetSafeDefinition(
    "CMAKE_SYSTEM_NAME");
  const std::string compilerId = makefile->GetSafeDefinition(
    compilerIdVar.c_str());
  std::string compiler = "gcc"; // default to gcc
  if (compilerId == "MSVC")
    {
    compiler = "msvc8";
    }
  else if (compilerId == "Borland")
    {
    compiler = "bcc";
    }
  else if (compilerId == "SDCC")
    {
    compiler = "sdcc";
    }
  else if (compilerId == "Intel")
    {
    compiler = "icc";
    }
  else if (compilerId == "Watcom")
    {
    compiler = "ow";
    }
  else if (compilerId == "GNU")
    {
    compiler = "gcc";
    }
  return compiler;
}

int cmGlobalCodeBlocksGenerator::GetCodeBlocksTargetType(cmTarget * target)
const
{
  if (target->GetType() == cmTarget::EXECUTABLE)
    {
    if ((target->GetPropertyAsBool("WIN32_EXECUTABLE")) ||
        (target->GetPropertyAsBool("MACOSX_BUNDLE")))
      {
      return 0;
      }
    else
      {
      return 1;
      }
    }
  else if ((target->GetType() == cmTarget::STATIC_LIBRARY) ||
           (target->GetType() == cmTarget::OBJECT_LIBRARY))
    {
    return 2;
    }
  else if ((target->GetType() == cmTarget::SHARED_LIBRARY) ||
           (target->GetType() == cmTarget::MODULE_LIBRARY))
    {
    return 3;
    }
  return 4;
}
