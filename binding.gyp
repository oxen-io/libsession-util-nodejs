{
  "targets": [
    {
      "target_name": "session_util_wrapper",
      "sources": [ "session_util.cc", "user_config.cc" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "<(module_root_dir)/include/",
        "<(module_root_dir)/oxen-encoding/"
      ],
      "libraries": [ "<(module_root_dir)/lib/libsession-util.a"],
      "cflags_cc": [
        "-std=c++17",
        "-fexceptions"
      ]
    }
  ]
}