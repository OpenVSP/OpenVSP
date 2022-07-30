# This makefile assumes the ndk-build tool is executed from the sdk/tests/test_feature/projects/android directory
# Change the next line to full path if there are any errors to link or find files
SDK_BASE_PATH := $(call my-dir)/../../../../..

ANGELSCRIPT_INCLUDE := $(SDK_BASE_PATH)/angelscript/include/

# -----------------------------------------------------
# Build the AngelScript library
# -----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := libangelscript

# Android API: Checks if can use pthreads. Version 2.3 fully supports threads and atomic instructions
# ifeq ($(TARGET_PLATFORM),android-3)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-4)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-5)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-6)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-7)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# else
# ifeq ($(TARGET_PLATFORM),android-8)
#     LOCAL_CFLAGS := -DAS_NO_THREADS
# endif
# endif
# endif
# endif
# endif
# endif

LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES := $(wildcard $(SDK_BASE_PATH)/angelscript/source/*.S)
LOCAL_SRC_FILES += $(wildcard $(SDK_BASE_PATH)/angelscript/source/*.cpp)
LOCAL_PATH := .
LOCAL_ARM_MODE := arm
include $(BUILD_STATIC_LIBRARY)

# -----------------------------------------------------
# Build the add-ons
# -----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := libaddons

addons:=$(SDK_BASE_PATH)/add_on/scriptarray/scriptarray.cpp               \
		$(SDK_BASE_PATH)/add_on/scriptgrid/scriptgrid.cpp                 \
		$(SDK_BASE_PATH)/add_on/scripthandle/scripthandle.cpp             \
		$(SDK_BASE_PATH)/add_on/scripthelper/scripthelper.cpp             \
		$(SDK_BASE_PATH)/add_on/scriptstdstring/scriptstdstring.cpp       \
		$(SDK_BASE_PATH)/add_on/scriptstdstring/scriptstdstring_utils.cpp \
		$(SDK_BASE_PATH)/add_on/scriptdictionary/scriptdictionary.cpp     \
		$(SDK_BASE_PATH)/add_on/scriptmath/scriptmath.cpp                 \
		$(SDK_BASE_PATH)/add_on/scriptmath/scriptmathcomplex.cpp          \
		$(SDK_BASE_PATH)/add_on/scriptany/scriptany.cpp                   \
		$(SDK_BASE_PATH)/add_on/scriptfile/scriptfile.cpp                 \
		$(SDK_BASE_PATH)/add_on/scriptfile/scriptfilesystem.cpp           \
		$(SDK_BASE_PATH)/add_on/scriptbuilder/scriptbuilder.cpp           \
		$(SDK_BASE_PATH)/add_on/serializer/serializer.cpp                 \
		$(SDK_BASE_PATH)/add_on/contextmgr/contextmgr.cpp                 \
		$(SDK_BASE_PATH)/add_on/debugger/debugger.cpp                     \
		$(SDK_BASE_PATH)/add_on/weakref/weakref.cpp
	
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_CPPFLAGS += -I$(ANGELSCRIPT_INCLUDE)
LOCAL_SRC_FILES := $(addons)
LOCAL_PATH := .
LOCAL_ARM_MODE := arm
include $(BUILD_STATIC_LIBRARY)

# -----------------------------------------------------
# Build the test project
# -----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := AndroidTests

LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_CPPFLAGS += -I$(ANGELSCRIPT_INCLUDE)
LOCAL_SRC_FILES := $(wildcard $(SDK_BASE_PATH)/tests/test_feature/source/*.cpp)
LOCAL_PATH := .
LOCAL_LDLIBS := -llog

LOCAL_STATIC_LIBRARIES := libangelscript libaddons
LOCAL_ARM_MODE := arm
include $(BUILD_SHARED_LIBRARY)