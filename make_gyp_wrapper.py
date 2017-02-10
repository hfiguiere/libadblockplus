#!/usr/bin/env python
# coding: utf-8
"""This script fixes the support of manually compiled static libraries for
android NDK build system.

Issue:
Let's say there are some manually compiled libraries specified in
link_settings.libraries or in ldflags. In this case ndk-build passes them to a
linker after system libraries like c++_static and consequently linker cannot
find functions from system libraries.

Desire:
Pass manually compiled libraries after regular dependecies.

How it works:
The recommended way to do it is to use LOCAL_STATIC_LIBRARIES. However, to
simply add libraries to the list is not enough because here ndk-build is trying
to be too smart and removes any libraries which are not defined in Makefiles.

So, the idea is to monkey patch gyp to inject definition of static libraries
and add them to LOCAL_STATIC_LIBRARIES. The former is to prevent them from
being removed.

The only class which is writing Makefiles is MakefileWriter and the "entry
point" is MakefileWriter.Write. To write LOCAL_STATIC_LIBRARIES it uses
MakefileWriter.WriteList, so one could simply override Write method to store
all required information and to define libraries and WriteList to change the
value of LOCAL_STATIC_LIBRARIES. However merely to reduce any potential
influence we override MakefileWriter.WriteAndroidNdkModuleRule to define
manually compiled libraries and override WriteList only for the method
WriteAndroidNdkModuleRule. The aim of this method is exactly to write Rules for
ndk-build and it is called at the end of Write. However since
WriteAndroidNdkModuleRule lacks required information, we override method Write
to store configurations as _abp_configs attribute of instance of MakefileWriter
.

"""

import os
import sys
import types

base_dir = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(base_dir, 'third_party', 'gyp', 'pylib'))
import gyp
from gyp.generator.make import MakefileWriter, QuoteIfNecessary


orig_MakefileWriter_Write = MakefileWriter.Write
orig_MakefileWriter_WriteAndroidNdkModuleRule = MakefileWriter.WriteAndroidNdkModuleRule

def overridden_Write(self, qualified_target, base_path, output_filename, spec, configs, part_of_all):
    if hasattr(self, "_abp_configs"):
      sys.exit("MakefileWriter already has property _abp_configs")
    self._abp_configs = configs
    orig_MakefileWriter_Write(self, qualified_target, base_path, output_filename, spec, configs, part_of_all)
    delattr(self, "_abp_configs")
    print spec

def overridden_WriteAndroidNdkModuleRule(self, module_name, all_sources, link_deps):
    for config in self._abp_configs:
        libs = self._abp_configs[config].get("user_libraries")
        if not libs:
            continue
        self.WriteLn("ifeq (${{BUILDTYPE}}, {})".format(config))
        for lib in libs:
            self.WriteLn("include $(CLEAR_VARS)")
            self.WriteLn("LOCAL_MODULE := {}".format(lib))
            self.WriteLn("LOCAL_SRC_FILES := {}".format(lib))
            self.WriteLn("include $(PREBUILT_STATIC_LIBRARY)")
            self.WriteLn("ABP_STATIC_LIBRARIES_${{BUILDTYPE}} += {}".format(lib))
        self.WriteLn("endif")
    orig_WriteList = self.WriteList 
    def overridden_WriteList(self, orig_value_list, variable=None, prefix='', quoter=QuoteIfNecessary):
        value_list = orig_value_list[:]
        if variable == "LOCAL_STATIC_LIBRARIES":
            value_list.append("${ABP_STATIC_LIBRARIES_${BUILDTYPE}}")
        orig_WriteList(value_list, variable, prefix, quoter)
    self.WriteList = types.MethodType(overridden_WriteList, self)
    orig_MakefileWriter_WriteAndroidNdkModuleRule(self, module_name, all_sources, link_deps)
    self.WriteList = orig_WriteList

#    libraries = spec.get('libraries')
#    if libraries:
#      # Remove duplicate entries
#      libraries = gyp.common.uniquer(libraries)
#      if self.flavor == 'mac':
#        libraries = self.xcode_settings.AdjustLibraries(libraries)
#    self.WriteList(libraries, 'LIBS')
#    print libraries
#

MakefileWriter.Write = overridden_Write
MakefileWriter.WriteAndroidNdkModuleRule = overridden_WriteAndroidNdkModuleRule

#def _FixPath(path):
#    if path == 'CORE' or path == 'EXPERIMENTAL' or path == 'off':
#        # Don't touch js2c parameters
#        return path
#    return orig_fix_path(path)
#gyp.generator.msvs._FixPath = _FixPath

if __name__ == '__main__':
    gyp.main(sys.argv[1:])
