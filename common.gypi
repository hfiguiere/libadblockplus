{
  'variables': {
    'visibility%': 'hidden',
    'library%': 'static_library',
    'component%': '',
    'want_separate_host_toolset': 0,
    'v8_optimized_debug': 0,
    'v8_enable_i18n_support': 0,
    'v8_use_external_startup_data%': 0,
    'v8_random_seed%': 0,
  },

  'conditions': [
    ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris" \
       or OS=="netbsd" or OS=="android"', {
      'target_defaults': {
        'cflags': [ '-Wall', '-W', '-Wno-unused-parameter',
                    '-Wnon-virtual-dtor', '-pthread', '-fno-rtti',
                    '-pedantic', '-std=c++0x', '-fexceptions', ],
        'cflags!': [ '-Werror', ],
        'ldflags': [ '-pthread', ],
      },
    }],
    ['OS=="mac"', {
      'xcode_settings': {
        'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',
        'CLANG_CXX_LIBRARY': 'libc++',
        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11', '-stdlib=libc++'],
      },
    }],
    ['OS=="android"', {
      'target_defaults': {
        'cflags!': [
          '-pthread',  # Not supported by Android toolchain.
        ],
        'ldflags!': [
          '-pthread',  # Not supported by Android toolchain.
        ],
      },
    }],
  ],

  'target_defaults': {
    'msvs_cygwin_shell': 0,
    'target_conditions': [[
        'OS=="mac" and _type=="executable"', {
          'xcode_settings': {
            'OTHER_LDFLAGS': ['-stdlib=libc++'],
          },
        }
      ],
      ['OS=="win"', {
        'defines': ['WIN32'],
        'msvs_configuration_attributes': {
          'OutputDirectory': '<(DEPTH)\\build\\$(ConfigurationName)',
          'IntermediateDirectory': '$(OutDir)\\obj\\$(ProjectName)',
          'CharacterSet': '1',
        },
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeTypeInfo': 'true', # we need it because there is dynamic_call
            'StringPooling': 'true', # pool string literals
            'DebugInformationFormat': 3, # Generate a PDB
            'WarningLevel': 3, # Unfortunately v8 headers produces a lot of warnings and there is no way to say -ipath-to-v8-include
            'ExceptionHandling': 1, # /EHsc
          },
          'VCLinkerTool': {
            'GenerateDebugInformation': 'true',
            'DataExecutionPrevention': 2, # enable DEP
            'AllowIsolation': 'true',
          }
        },
      }],
      ['OS=="win" and target_arch=="x64"', {
        'msvs_configuration_platform': 'x64'
      }],
    ],
    'configurations': {
      'Debug': {
        'defines': ['DEBUG', '_DEBUG'],
        'cflags': [ '-g', '-O0' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 1 # /MTd (debug static)
          },
          'VCLinkerTool': {
            'LinkTimeCodeGeneration': 1,
            'OptimizeReferences': 2,
            'EnableCOMDATFolding': 2,
            'LinkIncremental': 1,
            'GenerateDebugInformation': 'true',
          }
        }
      },
      'Release': {
        'cflags': [ '-O3', '-fdata-sections', '-ffunction-sections' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
              'RuntimeLibrary': 0, #/MT static release
              'Optimization': 3, # /Ox, full optimization
              'FavorSizeOrSpeed': 1, # /Ot, favour speed over size
              'InlineFunctionExpansion': 2, # /Ob2, inline anything eligible
              'OmitFramePointers': 'true',
              'EnableFunctionLevelLinking': 'true',
              'EnableIntrinsicFunctions': 'true',
            },
          'VCLibrarianTool': {
            'AdditionalOptions': [
              '/LTCG', # link time code generation
            ],
          },
          'VCLinkerTool': {
            'LinkTimeCodeGeneration': 1, # link-time code generation
            'OptimizeReferences': 2, # /OPT:REF
            'EnableCOMDATFolding': 2, # /OPT:ICF
            'LinkIncremental': 1, # disable incremental linking
          },
        }
      }
    }
  }
}
