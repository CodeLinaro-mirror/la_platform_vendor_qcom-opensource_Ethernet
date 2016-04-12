ifneq (,$(filter $(QCOM_BOARD_PLATFORMS),$(TARGET_BOARD_PLATFORM)))
ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

commonSources :=

LOCAL_COPY_HEADERS_TO := neutrino
LOCAL_COPY_HEADERS += DWC_ETH_QOS_desc.h DWC_ETH_QOS_drv.h DWC_ETH_QOS_ethtool.h DWC_ETH_QOS_pci.h DWC_ETH_QOS_yapphdr.h DWC_ETH_QOS_yheader.h DWC_ETH_QOS_yregacc.h
include $(BUILD_COPY_HEADERS)

# the dlkm
DLKM_DIR   := device/qcom/common/dlkm

#include $(CLEAR_VARS)
LOCAL_MODULE      := DWC_ETH_QOS.ko
LOCAL_MODULE_TAGS := debug
include $(DLKM_DIR)/AndroidKernelModule.mk

endif
endif
