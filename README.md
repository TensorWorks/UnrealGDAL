# UnrealGDAL: Unreal Engine GDAL plugin

This plugin provides access to the [GDAL/OGR](https://gdal.org/) C++ API inside the Unreal Engine, allowing Unreal projects and plugins to easily import and manipulate geospatial data. In addition to providing access to the full GDAL/OGR C++ API surface, the plugin bundles the header-only [mergetiff](https://github.com/adamrehn/mergetiff-cxx) library for providing [convenient smart pointer types](./Source/UnrealGDAL/Public/SmartPointers.h), and includes [interoperability functionality](./Source/UnrealGDAL/Public/GDALHelpers.h) for accessing the GDAL/OGR API using native Unreal Engine datatypes.

This plugin uses [conan-ue4cli](https://github.com/adamrehn/conan-ue4cli) to bundle GDAL and its dependencies in a cross-platform manner. Note that you will not need conan-ue4cli installed if you [use a binary release of the plugin](#using-release-binaries), but you will need conan-ue4cli installed and properly configured in order to [build the plugin and all of its dependencies from source](#building-everything-from-source).

**Note that this plugin requires Unreal Engine version 4.25.0 or newer.**


## Contents

- [Installation](#installation)
  - [Using release binaries](#using-release-binaries)
  - [Building everything from source](#building-everything-from-source)
- [Usage](#usage)
- [Legal](#legal)


## Installation

### Using release binaries

You can download release binaries for the plugin from the [releases page](https://github.com/TensorWorks/UnrealGDAL/releases). These releases include [precomputed dependency data](https://docs.adamrehn.com/conan-ue4cli/read-these-first/concepts#precomputed-dependency-data) for GDAL and its dependencies, so you can simply copy the plugin directory into your project or Unreal Engine installation and start using it immediately without the need to install or configure conan-ue4cli. Release binaries are currently provided for 64-bit versions of Windows and Linux.

### Building everything from source

To build the plugin from source without using the [precomputed dependency data](https://docs.adamrehn.com/conan-ue4cli/read-these-first/concepts#precomputed-dependency-data) included with the release binaries, you will need to [install and configure conan-ue4cli](https://docs.adamrehn.com/conan-ue4cli/workflow/installation). Once conan-ue4cli is installed and you have generated the wrapper packages for your version of the Unreal Engine, perform the following steps:

1. Update the conan-ue4cli [recipe cache](https://docs.adamrehn.com/conan-ue4cli/read-these-first/concepts#recipe-cache) by running the [ue4 conan update](https://docs.adamrehn.com/conan-ue4cli/commands/update) command.

2. Build the Conan packages for the plugin's dependencies using the [ue4 conan build](https://docs.adamrehn.com/conan-ue4cli/commands/build) command:
    
    ```bash
    # Build the packages for GDAL version 2.4.0 and mergetiff-cxx version 0.0.6
    ue4 conan build "gdal-ue4==2.4.0" "mergetiff-ue4==0.0.6"
    ```

3. Once the Conan packages are built, you will be able to build or package any Unreal project which uses the UnrealGDAL plugin as normal.


## Usage

**Step 1: List the UnrealGDAL plugin as a dependency.** In the descriptor file (`.uproject` or `.uplugin`) for any project or plugin which will consume the GDAL/OGR API, add the following lines:

```json
"Plugins": [
  {
    "Name": "UnrealGDAL",
    "Enabled": true
  }
]
```

**Step 2: List the GDAL and UnrealGDAL modules as build dependencies.** In the module rules file (`.Build.cs`) for any C++ code module which will consume the GDAL/OGR API, add the following lines:

```csharp
PublicDependencyModuleNames.AddRange(
  new string[]
  {
    "GDAL",
    "UnrealGDAL"
  }
);
```

**Step 3: Ensure the UnrealGDAL module is loaded at startup and GDAL is correctly configured.** In the source file containing the [StartupModule()](https://docs.unrealengine.com/en-US/API/Runtime/Core/Modules/IModuleInterface/StartupModule/index.html) method of the module implementation class for any C++ code module which will consume the GDAL/OGR API, add the following lines:

```cxx
//Add this line at the top of the source file
#include "UnrealGDAL.h"

//Add these lines to your StartupModule() method
void YourModuleNameGoesHere::StartupModule()
{
  FUnrealGDALModule* UnrealGDAL = FModuleManager::Get().LoadModulePtr<FUnrealGDALModule>("UnrealGDAL");
  UnrealGDAL->InitGDAL();
}
```

This will ensure the UnrealGDAL module is loaded into memory, all GDAL drivers are registered and GDAL is able locate its bundled data files, which are necessary for certain functionality (e.g. manipulating spatial reference systems) to work correctly. **This code must be added to every C++ code module which will consume the GDAL/OGR API in order to ensure correct behaviour.**


## Legal

Copyright &copy; 2020, TensorWorks Pty Ltd. Licensed under the MIT License, see the file [LICENSE](./LICENSE) for details.

Binary releases of this plugin contain the following third-party libraries, which are covered by their respective licenses:

- [mergetiff-cxx](https://github.com/adamrehn/mergetiff-cxx) ([MIT License](https://github.com/adamrehn/mergetiff-cxx/blob/master/LICENSE))
- [GDAL](https://gdal.org/) ([X/MIT License](https://github.com/OSGeo/gdal/blob/master/gdal/LICENSE.TXT))
- [GEOS](https://trac.osgeo.org/geos) ([LGPL-2.1 License](https://github.com/libgeos/geos/blob/master/COPYING))
- [PROJ](https://proj.org/) ([X/MIT License](https://github.com/OSGeo/PROJ/blob/master/COPYING))

GDAL and PROJ are linked statically, whereas GEOS is linked dynamically due to the [static linking requirements of the LGPL License](https://www.gnu.org/licenses/gpl-faq.en.html#LGPLStaticVsDynamic) and mergetiff is a header-only library. The source code for all of these libraries is bundled with binary releases of the plugin under the [ThirdParty](./ThirdParty) directory.
