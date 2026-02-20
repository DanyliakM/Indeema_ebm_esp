# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/myroslav/esp/esp-idf/components/bootloader/subproject"
  "/home/myroslav/Indeema_emb_esp/managed_components/ble/blehr/build/bootloader"
  "/home/myroslav/Indeema_emb_esp/managed_components/ble/blehr/build/bootloader-prefix"
  "/home/myroslav/Indeema_emb_esp/managed_components/ble/blehr/build/bootloader-prefix/tmp"
  "/home/myroslav/Indeema_emb_esp/managed_components/ble/blehr/build/bootloader-prefix/src/bootloader-stamp"
  "/home/myroslav/Indeema_emb_esp/managed_components/ble/blehr/build/bootloader-prefix/src"
  "/home/myroslav/Indeema_emb_esp/managed_components/ble/blehr/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/myroslav/Indeema_emb_esp/managed_components/ble/blehr/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/myroslav/Indeema_emb_esp/managed_components/ble/blehr/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
