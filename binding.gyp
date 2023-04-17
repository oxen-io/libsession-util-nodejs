{
  "targets": [
    {
      "target_name": "session_util_wrapper",
      "sources": [ "src/utilities.cpp", "src/base_config.cpp", "src/user_config.cpp", "src/contacts_config.cpp", "src/user_groups_config.cpp", "src/convo_info_volatile_config.cpp", "src/addon.cpp" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "<(module_root_dir)/include/"
        ],
      "cflags_cc": [
        "-std=c++17",
        "-fexceptions",
        "-frtti",
      ],
      "cflags": [
        "-std=c++17",
        "-fexceptions",
        "-frtti",
      ],
      'conditions': [
        ['OS == "mac"', {
          "libraries": [
            "-lsession-util",
            "-L<(module_root_dir)/lib/mac/",
          ],
        }, {}],
        ['OS == "linux"', {
          "libraries": [
            "-lsession-util",
            "-L<(module_root_dir)/lib/linux/amd64"
          ],
        }, {}],
        ['OS == "win"', {
          "libraries": [
            "-lsession-util",
            "-L<(module_root_dir)/lib/win/amd64",
          ],
        }, {}],
      ],
      "xcode_settings": {
        "OTHER_CFLAGS": [
          "-std=c++17",
          "-Wno-c++11-narrowing",
          "-fexceptions",
          "-frtti",
        ],
      },
      "msbuild_settings": {
        "ClCompile": {
            "LanguageStandard": "stdcpp17"
        }
      },
      'msvs_settings': {
        'VCCLCompilerTool': { "ExceptionHandling": 1, 'AdditionalOptions': [ '-std:c++17' ] }
      }
    }
  ]
}