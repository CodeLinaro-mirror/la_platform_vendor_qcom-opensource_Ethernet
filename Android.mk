
ifneq (,$(filter $(QCOM_BOARD_PLATFORMS),$(TARGET_BOARD_PLATFORM)))
ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

commonSources :=

# the dlkm
DLKM_DIR   := device/qcom/common/dlkm

#include $(CLEAR_VARS)
LOCAL_MODULE      := DWC_ETH_QOS.ko
LOCAL_MODULE_TAGS := optional
include $(DLKM_DIR)/AndroidKernelModule.mk

include $(CLEAR_VARS)
LOCAL_MODULE_CLASS := data
LOCAL_MODULE := ntn_config.ini
LOCAL_SRC_FILES := config.ini
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)
include $(BUILD_PREBUILT)

endif
endif
