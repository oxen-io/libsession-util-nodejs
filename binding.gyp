{
  "targets": [
    {
      "target_name": "session_util_wrapper",
      "sources": [ "src/utilities.cpp", "src/base_config.cpp", "src/user_config.cpp", "src/contacts_config.cpp", "src/user_groups_config.cpp", "src/convo_info_volatile_config.cpp", "src/addon.cpp" ],
      "defines": [ "_STL_COMPILER_PREPROCESSOR=1", "NOMINMAX=1" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "<(module_root_dir)/include/"
        ],
      "libraries": [
        "C:/Users/audri/libsession-util-nodejs/lib/win/amd64/session-util.lib"
      ],
      "cflags_cc": [
        "-std=c++17",
        "-fno-exceptions"
      ],
      "cflags": [
        "-std=c++17",
        "-fno-exceptions"
      ],
      "conditions": [
        ['OS=="win"',
          {
            "configurations": {
              "Debug": {
                "msvs_settings": {
                  "VCCLCompilerTool": {
                    "RuntimeTypeInfo": "true",
                  },
                }
              },
              "Release": {
                "VCCLCompilerTool": {
                    "RuntimeTypeInfo": "true",
                  },
                },
              },
            },
          ],
    ],
      "xcode_settings": {
        "OTHER_CFLAGS": [
          "-std=c++17",
          "-Wno-c++11-narrowing",
          "-fno-exceptions",
          "-frtti"
        ]
      },
      "msvs_settings": {
        "VCCLCompilerTool": {
          "AdditionalOptions": [
            "/Zc:__cplusplus",
            "-std:c++17",
            "/GR"

        ],
        "RuntimeTypeInfo": "true",
        "DebugInformationFormat": 1,
        "ExceptionHandling": 1
        }
      }
    }
  ]
}