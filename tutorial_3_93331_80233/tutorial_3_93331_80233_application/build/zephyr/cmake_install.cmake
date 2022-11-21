# Install script for directory: /Users/bernardofalle/ncs/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Users/bernardofalle/zephyr-sdk-0.15.1/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/soc/arm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/civetweb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/liblc3codec/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/tinycbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/TraceRecorder/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/bernardofalle/Documents/SOTR/tutorial_3_93331_80233/tutorial_3_93331_80233_application/build/zephyr/cmake/reports/cmake_install.cmake")
endif()

