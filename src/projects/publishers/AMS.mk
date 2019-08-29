LOCAL_PATH := $(call get_local_path)
include $(DEFAULT_VARIABLES)

LOCAL_TARGET := publishers

LOCAL_SOURCE_FILES := $(LOCAL_SOURCE_FILES) \
    $(call get_sub_source_list,hls) \
    $(call get_sub_source_list,dash) \
	$(call get_sub_source_list,cmaf)

LOCAL_HEADER_FILES := $(LOCAL_HEADER_FILES) \
    $(call get_sub_header_list,hls) \
	$(call get_sub_header_list,dash) \
	$(call get_sub_header_list,cmaf)

include $(BUILD_STATIC_LIBRARY)