add_subdirectory(types)
add_subdirectory(datacollection)
add_subdirectory(datadecoding)
add_subdirectory(datainspection)
add_subdirectory(datamanager)
if(FWE_FEATURE_CUSTOM_DATA_SOURCE)
    add_subdirectory(custom)
endif()
